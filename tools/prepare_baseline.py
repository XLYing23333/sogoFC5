#!/usr/bin/env python3
"""Extract the pinned Debian payload without running maintenance scripts."""

import argparse
import hashlib
import json
import shutil
import subprocess
import tempfile
from pathlib import Path

PROJECT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    spec = json.loads((PROJECT / "vendor/baseline.json").read_text())
    if hashlib.sha256(args.package.read_bytes()).hexdigest() != spec["sha256"]:
        parser.error("package SHA-256 mismatch")
    metadata = subprocess.check_output(["dpkg-deb", "-f", str(args.package),
                                        "Package", "Version", "Architecture"], text=True)
    expected = {"Package": spec["package"], "Version": spec["version"], "Architecture": spec["architecture"]}
    actual = dict(line.split(": ", 1) for line in metadata.strip().splitlines())
    if actual != expected:
        parser.error("package identity mismatch")
    dest = args.destination.absolute()
    if dest.exists() or dest.is_symlink():
        parser.error("destination must not exist")
    dest.parent.mkdir(parents=True, exist_ok=True)
    stage = Path(tempfile.mkdtemp(prefix=".sogou-fc4-", dir=dest.parent))
    try:
        subprocess.run(["dpkg-deb", "-x", str(args.package.absolute()), str(stage)], check=True)
        for relative, expected_hash in spec["binaries"].items():
            if hashlib.sha256((stage / relative).read_bytes()).hexdigest() != expected_hash:
                raise ValueError("extracted binary SHA-256 mismatch: " + relative)
        # Refuse a concurrently created destination rather than merge its contents.
        dest.mkdir()
        for child in stage.iterdir():
            child.rename(dest / child.name)
        stage.rmdir()
    except BaseException:
        shutil.rmtree(stage, ignore_errors=True)
        raise
    print(json.dumps(dict(package=spec["package"], version=spec["version"],
                         sha256=spec["sha256"], maintainer_scripts_executed=False)))


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Create a deterministic archive of the ABI sources and static evidence."""

import argparse
import gzip
import hashlib
import io
import tarfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INCLUDE = ("CMakeLists.txt", "VERSION", "README.md", "README-CN.md", "NOTICE.md", ".gitignore", ".gitattributes", "src", "tests",
           "tools", "packaging", "themes", "assets", "docs", "abi", "scripts",
           "vendor/baseline.json", "vendor/README.md")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    if args.output.resolve().is_relative_to(ROOT):
        relative = args.output.resolve().relative_to(ROOT)
        if not relative.parts or relative.parts[0] not in {"dist", "build"}:
            parser.error("in-project archive output must be inside dist/ or build/")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("wb") as raw, gzip.GzipFile(filename="", mode="wb", fileobj=raw, mtime=0) as gz:
        with tarfile.open(fileobj=gz, mode="w", format=tarfile.USTAR_FORMAT) as archive:
            paths = []
            for name in INCLUDE:
                entry = ROOT / name
                if not entry.exists():
                    raise FileNotFoundError(name)
                paths.extend(entry.rglob("*") if entry.is_dir() else [entry])
            for path in sorted(paths):
                relative = path.relative_to(ROOT)
                if not path.is_file() or "__pycache__" in relative.parts or path.suffix == ".pyc":
                    continue
                data = path.read_bytes()
                info = tarfile.TarInfo("sogoFC5/" + relative.as_posix())
                mode = 0o755 if path.stat().st_mode & 0o111 else 0o644
                info.size, info.mode, info.mtime = len(data), mode, 0
                archive.addfile(info, io.BytesIO(data))
    digest = hashlib.sha256(args.output.read_bytes()).hexdigest()
    args.output.with_name(args.output.name + ".sha256").write_text(digest + "  " + args.output.name + "\n")
    print(digest)


if __name__ == "__main__":
    main()

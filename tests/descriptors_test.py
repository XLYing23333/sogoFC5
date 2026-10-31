import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from descriptors import extract, fields, parse_descriptor, proto_source, varint


class DescriptorsTest(unittest.TestCase):
    def test_embedded_schema(self):
        location = ROOT / "abi/4.2.1.145"
        report = json.loads((location / "analysis.json").read_text())
        for name, count in [("ImeClient.proto", 40), ("ImeServer.proto", 33)]:
            with self.subTest(name=name):
                record = report["protocol"][name]
                raw = (location / (name + ".descriptor.bin")).read_bytes()
                self.assertEqual(hashlib.sha256(raw).hexdigest(), record["frontend"]["sha256"])
                parsed = parse_descriptor(raw)
                self.assertEqual(parsed, record["schema"])
                self.assertEqual(len(parsed["messages"][0]["fields"]), count)
                self.assertEqual(proto_source(parsed), (location / name).read_text())
                again, offset, descriptor = extract(b"prefix\0" + raw + b"\0suffix", name)
                self.assertEqual(again, parsed)
                self.assertEqual(offset["file_offset"], 7)
                self.assertEqual(descriptor, raw)

    def test_truncated_data(self):
        for data in [b"\x80", b"\x12\x80", b"\x12\x04abc", b"\x09abcd", b"\x0dabc",
                     b"\x00", b"\x0b", b"\x08" + b"\xff" * 10 + b"\x01"]:
            with self.subTest(data=data), self.assertRaises(ValueError):
                fields(data)
        with self.assertRaises(ValueError):
            varint(b"\xff" * 9 + b"\x02", 0)

    def test_scalar_boundaries(self):
        self.assertEqual(varint(b"\xff" * 9 + b"\x01", 0), ((1 << 64) - 1, 10))
        self.assertEqual(fields(b"\x08\x00\x10\x01\x18\xac\x02"),
                         [(1, 0, 0), (2, 0, 1), (3, 0, 300)])

    def test_ambiguous_descriptors(self):
        raw = (ROOT / "abi/4.2.1.145/ImeClient.proto.descriptor.bin").read_bytes()
        with self.assertRaises(ValueError):
            extract(raw + b"\0" + raw + b"\0", "ImeClient.proto")
        with self.assertRaises(ValueError):
            extract(raw, "ImeClient.proto")

    def test_protoc_compilation(self):
        # protoc is an independent schema parser; no vendor code is loaded.
        import shutil
        if not shutil.which("protoc"):
            self.skipTest("protoc unavailable")
        location = ROOT / "abi/4.2.1.145"
        with tempfile.TemporaryDirectory() as folder:
            subprocess.run(["protoc", "-I", str(location), "--cpp_out=" + folder,
                            str(location / "ImeClient.proto"), str(location / "ImeServer.proto")],
                           check=True, capture_output=True)
            for name in ["ImeClient.pb.h", "ImeClient.pb.cc", "ImeServer.pb.h", "ImeServer.pb.cc"]:
                self.assertTrue((Path(folder) / name).is_file())


if __name__ == "__main__":
    unittest.main()

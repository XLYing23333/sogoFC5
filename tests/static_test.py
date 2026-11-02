import json
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from analyze import analyze, SHELL, FRONTEND, IME, VTABLE, TARGETS
from elf import Elf


def main():
    baseline = Path(sys.argv[1])
    with tempfile.TemporaryDirectory() as temp:
        report = analyze(baseline, Path(temp))
        expected = json.loads((ROOT / "abi/4.2.1.145/analysis.json").read_text())
        assert report == expected, "static evidence changed"
        for relative in (ROOT / "abi/4.2.1.145").rglob("*"):
            if relative.is_file():
                key = relative.relative_to(ROOT / "abi/4.2.1.145")
                assert relative.read_bytes() == (Path(temp) / key).read_bytes(), key
        assert not report["vendor_code_executed"]
        assert not report["engine_conversion_verified"]
        assert not report["desktop_verified"]
        symbols = {s["name"] for s in Elf(baseline / SHELL).symbols() if s["defined"]}
        assert {"GetSogouShell", "Release", "GetSogouShellWrapper"} <= symbols
        service=Elf(baseline/"opt/sogoupinyin/files/bin/sogoupinyin-service")
        window=[s for s in service.symbols() if s["name"]=="_ZTVN6n_sgxx10MainWindowE" and s["defined"]]
        assert len(window)==1 and window[0]["size"]==512
        ime_imports={s["name"] for s in Elf(baseline/IME).symbols() if not s["defined"]}
        assert "_ZTVN6n_sgxx10MainWindowE" in ime_imports
        imports = report["binaries"][FRONTEND]["relevant_imports"]
        assert "FcitxInstanceCommitString" in imports
        assert "XTestFakeKeyEvent" in imports
        shell = Elf(baseline / SHELL)
        environment=Elf(baseline/"opt/sogoupinyin/files/lib/libSogouWbShell.so")
        exports={s["name"]:s for s in environment.symbols() if s["defined"]}
        assert exports["BOOL_PadNumAsSelect"]["size"]==8
        assert exports["_ZN5t_env12SetValueBoolEPK10t_envEntryIbEb"]["address"]==0x1ad098
        assert "_ZNK5t_env12GetValueBoolEPK10t_envEntryIbE" in exports
        # The native reset writes a wchar terminator through the same result
        # getter used by the conversion path; it does not free the buffer.
        # The regular and fsync wrappers call the same save core with distinct
        # flags. The diagnostic int output is initialized by the wrapper.
        assert shell.read(0x10d8e3,2)==b"\x31\xf6"
        assert shell.read(0x10d9c7,5)==b"\xbe\x01\x00\x00\x00"
        assert report["learning_contract"]["sync_save"]==0x10d960
        assert shell.read(0x435217,6)==b"\xc7\x00\x00\x00\x00\x00"
        assert shell.read(0x438f24,6)==b"\x48\x05\x58\x14\x00\x00"
        assert shell.pointer(0x9ecc10+13*8)==0xa1b0e
        assert shell.pointer(0x9ecc10+14*8)==0xa1b54
        for address in (0xa1b4d,0xa1b93):
            assert shell.read(address,5)==b"\xb8\x01\x00\x00\x00", "PC paging must not be treated as a boundary-status query"
        for i, target in enumerate(TARGETS):
            assert shell.pointer(VTABLE + i * 8) == target
        try:
            shell.read(0xffffffffffffffff, 8)
        except ValueError:
            pass
        else:
            raise AssertionError("unmapped address accepted")
    print("Pinned ELF hashes, 37 vtable entries, 27 caller offsets, and 73 protocol fields verified statically.")


if __name__ == "__main__":
    main()

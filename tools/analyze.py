#!/usr/bin/env python3
"""Verify a pinned FC4 package and emit read-only ABI and descriptor evidence."""

import argparse
import hashlib
import json
import re
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

from descriptors import extract, proto_source
from elf import Elf

PROJECT = Path(__file__).resolve().parents[1]
SHELL = "opt/sogoupinyin/files/lib/libSogouShell.so"
IME = "opt/sogoupinyin/files/lib/libSogouIme.so"
FRONTEND = "usr/lib/x86_64-linux-gnu/fcitx/fcitx-sogoupinyin.so"
SERVICE = "opt/sogoupinyin/files/bin/sogoupinyin-service"
VTABLE = 0x9ec750
TARGETS = [0x8a2e6, 0x8a4da, 0x89c12, 0x8c2d4, 0x8a500, 0x8a81e,
           0x8ad42, 0x8adc0, 0x8ae02, 0x8ae3e, 0x8ae88, 0x8aed2,
           0x8af1c, 0x8af68, 0x8b03c, 0x8b07e, 0x8b0c2, 0x8b106,
           0x8b14c, 0x8b192, 0x8b1d8, 0x8b454, 0x8b62e, 0x8b6c0,
           0x8b830, 0x8ba36, 0x8ba88, 0x8bad6, 0x8bb22, 0x8bb6e,
           0x8bbca, 0x8bc24, 0x8bc78, 0x8afaa, 0x8afee, 0x8bcc6, 0x8b7de]
# Names derive from callers; unclassified slots remain opaque.
METHODS = {
    4: ("initialize_paths", 0x481d00, "bool(void*, const char*, const char*, const char*, const char*, const char*)"),
    5: ("active", 0x2f78ce, "bool(void*, int32_t)"),
    6: ("insert_char", 0x2f7a4b, "bool(void*, int32_t)"),
    7: ("back", 0x2f7be9, "bool(void*)"),
    8: ("clear", 0x2f7dcd, "void(void*)"),
    9: ("select", 0x2f7fbe, "void(void*, int32_t)"),
    10: ("select_pinyin", 0x2f801e, "void(void*, int32_t)"),
    11: ("focus_candidate", 0x2f8080, "opaque"),
    12: ("word_prediction", 0x2f8106, "opaque"),
    13: ("page_down", 0x2f8170, "bool(void*)"),
    14: ("page_up", 0x2f81fd, "bool(void*)"),
    15: ("pinyin_list", 0x2f9a94, "object*(void*)"),
    16: ("candidate_list", 0x2f98ff, "object*(void*)"),
    17: ("composition_text", 0x2fa052, "object*(void*)"),
    18: ("auxiliary_text", 0x2fa0ad, "object*(void*)"),
    19: ("result_text", 0x2f9987, "object*(void*)"),
    21: ("set_string_parameter", 0x2f9010, "opaque"),
    22: ("extra_text", 0x2f945c, "object*(void*, uint32_t)"),
    24: ("reload_config", 0x2f9073, "opaque"),
    25: ("set_cursor", 0x2f9215, "bool(void*, bool, int32_t)"),
    26: ("get_cursor", 0x2f9271, "bool(void*, int32_t*)"),
    27: ("need_insert", 0x2f92f0, "bool(void*, int32_t)"),
    28: ("integer_info", 0x2f9368, "int32_t(void*, uint32_t)"),
    29: ("integer_list", 0x2f93cb, "opaque C++ aggregate return"),
    33: ("associate_result", 0x2f7013, "opaque"),
    34: ("cell_dictionary_info", 0x2f6c3d, "opaque"),
    36: ("beautify_handwriting_path", 0x2f6e0c, "opaque")
}


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def assembly(path, start, end):
    result = subprocess.run(["objdump", "-d", "-C", "--no-show-raw-insn",
                             f"--start-address={start}", f"--stop-address={end}", str(path)],
                            check=True, text=True, capture_output=True)
    # File headers are excluded so generated evidence has no workspace paths.
    return "\n".join(line for line in result.stdout.splitlines()
                     if re.match(r"\s*[0-9a-f]+[: ]", line)) + "\n"


def analyze(root, output, package=None):
    baseline = json.loads((PROJECT / "vendor/baseline.json").read_text())
    if package and digest(package) != baseline["sha256"]:
        raise ValueError("package SHA-256 mismatch")
    images = {}
    binaries = {}
    for relative, expected in baseline["binaries"].items():
        path = root / relative
        if digest(path) != expected:
            raise ValueError("binary SHA-256 mismatch: " + relative)
        image = images[relative] = Elf(path)
        symbols = image.symbols()
        relevant = re.compile(r"^(GetSogou|ReleaseSogou|GetSGAuth|ReleaseSGAuth|ime$|ABI_VERSION$|"
                              r"_ZN(?:10t_sogouIme|14t_sogouImeBase|17SogouShellWrapper)|"
                              r"BOOL_PadNumAsSelect$|_ZN5t_env12SetValueBool|_ZNK5t_env12GetValueBool|"
                              r"Fcitx|dbus_|XTest|XGrab|XQueryKeymap|socket$|sendmsg|recvmsg|shm_|mq_|sem_)")
        binaries[relative] = dict(sha256=expected, **image.dynamic(),
                                  defined_symbol_count=sum(s["defined"] for s in symbols),
                                  undefined_symbol_count=sum(not s["defined"] for s in symbols),
                                  relevant_exports=[s for s in symbols if s["defined"] and relevant.search(s["name"])],
                                  relevant_imports=[s["name"] for s in symbols if not s["defined"] and relevant.search(s["name"])],
                                  debug_info=".debug_info" in image.named)
    shell = images[SHELL]
    slots = []
    for index, expected in enumerate(TARGETS):
        target = shell.pointer(VTABLE + index * 8)
        if target != expected or not shell.executable(target):
            raise ValueError(f"shell slot {index} mismatch")
        entry = dict(slot=index, byte_offset=index * 8, target=target, status="static")
        if index in METHODS:
            name, caller, signature = METHODS[index]
            caller_image = root / IME
            evidence = assembly(caller_image, caller, caller + 12)
            # The compiler spells +0x80 as subtraction of its negative value.
            needle = "$0x" + format(index * 8, "x") + ",%rax"
            if needle not in evidence and not (index == 16 and "$0xffffffffffffff80,%rax" in evidence):
                raise ValueError("caller offset mismatch: " + name)
            entry.update(name=name, signature=signature, caller_binary=IME,
                         caller_address=caller, caller_instruction=evidence.strip())
        slots.append(entry)
    if shell.pointer(VTABLE + len(TARGETS) * 8) != 0:
        raise ValueError("unexpected end of shell vtable")
    layout=ET.parse(root/"opt/sogoupinyin/files/share/resources/skin/InputMode.xml")
    if layout.find("./layout[@name='Keyboard_pcpy']").get("value")!="19":
        raise ValueError("PC pinyin layout mismatch")
    pc_slots={6:0xa175e,16:0xa369e,17:0xa36b0,19:0xa36d4,25:0xa3fca,26:0xa3fe2,28:0xa4006}
    for index,target in pc_slots.items():
        if shell.pointer(0x9ecc10+index*8)!=target:
            raise ValueError("PC shell slot mismatch")
    if shell.pointer(0x9ec1f0)!=0x7cdae or shell.pointer(0x9ec1f8)!=0x7cd90:
        raise ValueError("output string interface mismatch")
    for table,index,target in [(0x9fc310,25,0x410d3e),(0x9fd910,7,0x438f18),(0x9fd910,31,0x4351f4)]:
        if shell.pointer(table+index*8)!=target:
            raise ValueError("core output interface mismatch")
    output.mkdir(parents=True, exist_ok=True)
    protocol = {}
    for name in ("ImeClient.proto", "ImeServer.proto"):
        front, front_loc, raw = extract(images[FRONTEND].data, name)
        server, server_loc, other = extract(images[SERVICE].data, name)
        if raw != other or front != server:
            raise ValueError("frontend/service descriptor mismatch: " + name)
        (output / name).write_text(proto_source(front))
        (output / (name + ".descriptor.bin")).write_bytes(raw)
        protocol[name] = dict(schema=front, frontend=front_loc, service=server_loc,
                              equality="byte-identical")
    evidence = output / "evidence"
    evidence.mkdir(exist_ok=True)
    for name, relative, start, end in [
        ("shell-constructor", SHELL, 0x8a1f2, 0x8a22d),
        ("path-initialization", IME, 0x481cc5, 0x481d27),
        ("insert-char", IME, 0x2f7a3e, 0x2f7a69),
        ("candidate-reading", IME, 0x2f98f2, 0x2f9a87),
        ("composition-reading", IME, 0x2fa045, 0x2fa09b),
        ("result-reading", IME, 0x2f997a, 0x2f99cc),
        ("get-cursor", IME, 0x2f9264, 0x2f9298),
        ("base-factory-alias", IME, 0x2fb940, 0x2fb96c),
        ("pc-key-encoding", SHELL, 0xa175e, 0xa181e),
        ("pc-key-press-release", SHELL, 0xa181e, 0xa1948),
        ("pc-result-refresh", SHELL, 0xa2ff8, 0xa306c),
        ("shell-result-forward", SHELL, 0x8b192, 0x8b1d8),
        ("pc-core-binding", SHELL, 0xa0c52, 0xa0c78),
        ("pc-result-object", SHELL, 0xa36d4, 0xa36e6),
        ("pc-result-source", SHELL, 0xa3391, 0xa3474),
        ("core-result-query", SHELL, 0x410d3e, 0x410d90),
        ("core-result-buffer", SHELL, 0x438f18, 0x438f2c),
        ("core-transient-result-reset", SHELL, 0x4351f4, 0x43523a),
        ("keypad-selection-gate", SHELL, 0x426223, 0x426267),
        ("native-input-environment", SHELL, 0x422af6, 0x422b6b),
        ("native-key-character-packing", SHELL, 0xa0ad0, 0xa0b0e),
        ("native-bool-setting", "opt/sogoupinyin/files/lib/libSogouWbShell.so", 0x1ad098, 0x1ad0ce),
        ("result-string-assign", SHELL, 0x7cd5c, 0x7cd90),
        ("keyboard-event-state", SHELL, 0x9e868, 0x9e8a0),
        ("authorization-initialization", IME, 0x37acf2, 0x37ad28)]:
        (evidence / (name + ".asm")).write_text(assembly(root / relative, start, end))
    report = dict(schema=1, baseline=baseline, vendor_code_executed=False,
                  engine_conversion_verified=False, desktop_verified=False,
                  binaries=binaries, shell_vtable_address=VTABLE, shell_slots=slots,
                  pc_keyboard_contract=dict(layout=19,vtable=0x9ecc10,press_flag=0x80000000,
                      virtual_key_A=65,virtual_key_Z=90,slots={str(k):v for k,v in pc_slots.items()},keyboard_state=0x9e868,
                      string_vtable=0x9ec1f0,string_assign=0x7cd5c),
                  result_buffer_contract=dict(shell_child_offset=0x38,pc_core_offset=0x130,
                      core_vtable=0x9fc310,query_slot=25,query=0x410d3e,
                      composition_vtable=0x9fd910,buffer_slot=7,buffer_query=0x438f18,
                      buffer_offset=0x1458,transient_reset=0x4351f4,
                      reset_width=4,clear_scope="core result terminator and converted UTF-8 output only"),
                  keypad_contract=dict(first_virtual_key=0x60,last_virtual_key=0x69,
                      environment_factory=0x422af6,selection_setting="BOOL_PadNumAsSelect",
                      selection_enabled=False,conversion_runtime_verified=False),
                  protocol=protocol)
    (output / "analysis.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    print(json.dumps(dict(version=baseline["version"], binaries=len(binaries),
                         shell_slots=len(slots), protocol_fields=[
                             sum(len(m["fields"]) for m in p["schema"]["messages"])
                             for p in protocol.values()],
                         vendor_code_executed=False), ensure_ascii=False))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path, help="extracted official package root")
    parser.add_argument("output", type=Path)
    parser.add_argument("--package", type=Path)
    args = parser.parse_args()
    analyze(args.root, args.output, args.package)


if __name__ == "__main__":
    main()

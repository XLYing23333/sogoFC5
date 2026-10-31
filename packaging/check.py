"""Validate a Debian artifact without installing it or executing vendor code."""
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
import configparser
import struct
from pathlib import Path

source=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(source/"tools"))
from elf import Elf


def main():
    package=Path(sys.argv[1])
    version=subprocess.check_output(["dpkg-deb","-f",str(package),"Version"],text=True).strip()
    assert version==(source/"VERSION").read_text().strip()
    with tempfile.TemporaryDirectory(prefix="sogo-direct-check-") as folder:
        stage=Path(folder)
        subprocess.run(["dpkg-deb","-x",str(package),folder],check=True)
        module=stage/"usr/lib/x86_64-linux-gnu/fcitx5/libsogo-fc5-direct.so"
        engine=stage/"usr/lib/sogo-fc5-direct/sogo-fc5-direct-engine"
        vendor=stage/"usr/lib/sogo-fc5-direct/vendor/files"
        baseline=json.loads((source/"vendor/baseline.json").read_text())
        for relative in ["opt/sogoupinyin/files/lib/libSogouIme.so","opt/sogoupinyin/files/lib/libSogouShell.so","opt/sogoupinyin/files/lib/libSogouWbShell.so"]:
            name=Path(relative).name
            assert hashlib.sha256((vendor/"lib"/name).read_bytes()).hexdigest()==baseline["binaries"][relative]
        for path in [module,engine]:
            dynamic=Elf(path).dynamic()
            assert not dynamic["rpath"] and not dynamic["runpath"],str(path)
        dependencies=Elf(module).dynamic()["needed"]
        assert not any("Sogou" in name or "cpis" in name or "Qt5" in name or "fcitx-core" in name for name in dependencies)
        assert not (vendor/"bin").exists() and not (vendor/"lib/qt5").exists()
        assert not (stage/"usr/share/applications").exists()
        for group in ["addon","inputmethod"]:
            item=configparser.ConfigParser(interpolation=None);item.optionxform=str
            item.read(stage/"usr/share/fcitx5"/group/"sogo-fc5-direct.conf")
            section=item["Addon" if group=="addon" else "InputMethod"]
            assert section["Name"]==section["Name[zh_CN]"]=="Sogo-FC5"
            if group=="addon":assert section["Version"]==version
            else:assert section["Icon"]=="/usr/share/pixmaps/sogo-fc5.png"
        logo=stage/"usr/share/pixmaps/sogo-fc5.png"
        original=source/"assets/SogoFC5.png"
        logo_spec=json.loads((source/"assets/logo.json").read_text())
        data=logo.read_bytes()
        assert data==original.read_bytes()
        assert hashlib.sha256(data).hexdigest()==logo_spec["sha256"]
        assert data[:8]==b"\x89PNG\r\n\x1a\n"
        assert struct.unpack(">II",data[16:24])==(logo_spec["width"],logo_spec["height"])
        assert not (stage/"usr/share/icons/hicolor/scalable/apps/sogou-direct.svg").exists()
        module_bytes=module.read_bytes()
        assert b"sogo-direct-system-palette" not in module_bytes
        assert "跟随系统主题".encode() not in module_bytes
        for path in stage.rglob("*"):
            assert "cpis" not in path.name.lower()
            assert not path.name.endswith(".py")
            if not path.is_symlink():assert not path.stat().st_mode&0o002,str(path)
        for name in ["sogo-direct-dark-mode","sogo-direct-welight-mode"]:
            root=stage/"usr/share/fcitx5/themes"/name
            for asset in ["theme.conf","panel.svg","highlight.svg","prev.svg","next.svg"]:assert (root/asset).is_file()
            theme=configparser.ConfigParser(interpolation=None);theme.optionxform=str;theme.read(root/"theme.conf")
            margin=theme["InputPanel/Highlight/Margin"]
            expected=(6,5) if name.endswith("dark-mode") else (3,3)
            assert tuple(int(margin[key]) for key in ["Left","Right","Top","Bottom"])==(expected[0],expected[0],expected[1],expected[1])
        # The native engine is checked only in file-hash mode. No dlopen of
        # vendor code, X11 connection or private D-Bus is involved.
        result=subprocess.check_output([str(engine),"--check","--vendor-root",str(vendor)],text=True)
        assert json.loads(result)=={"vendor_hashes":True,"vendor_code_executed":False}
        environment=dict(os.environ,LD_LIBRARY_PATH=str(vendor/"lib"))
        # This shared object reproduces the worker's exported compatibility
        # boundary and the complete vendor dependency group. ldd -r resolves
        # relocations without running constructors or engine initialization.
        contract=stage/"loader-contract.so"
        subprocess.run([os.environ.get("CXX","c++"),"-std=c++17","-shared","-fPIC",
                        str(source/"src/runtime_guards.cpp"),"-Wl,--no-as-needed",
                        str(vendor/"lib/libSogouIme.so"),"-o",str(contract)],check=True)
        for path in [module,engine,contract]:
            result=subprocess.run(["ldd","-r",str(path)],env=environment,text=True,capture_output=True,check=True)
            output=result.stdout+result.stderr
            assert "not found" not in output and "undefined symbol:" not in output,(path.name,output)
        exports=subprocess.check_output(["nm","-D","--defined-only",str(engine)],text=True)
        for symbol in ["__cxa_throw_bad_array_new_length","_ZTVN6n_sgxx10MainWindowE",
                       "_ZN6n_sgxx10MainWindowC1EP7QWidgetPNS_16t_wndTopImplBaseE",
                       "_ZN6n_sgxx10MainWindowC2EP7QWidgetPNS_16t_wndTopImplBaseE"]:
            assert symbol in exports,symbol
        print(json.dumps({"version":version,"structure":True,"linking":True,
                          "relocations":True,"engine_hashes":True,"logo_original_bytes":True,
                          "vendor_code_executed":False,"desktop_verified":False}))


if __name__=="__main__":main()

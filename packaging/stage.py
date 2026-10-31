"""Stage a private official engine payload and Fcitx5 integration resources."""
import json
import os
import shutil
import sys
from pathlib import Path

source=Path(__file__).resolve().parents[1]
original=Path(sys.argv[1])
stage=Path(sys.argv[2])
files=original/"opt/sogoupinyin/files"
target=stage/"usr/lib/sogo-fc5-direct/vendor/files"
target.mkdir(parents=True,exist_ok=True)
for name in ("lib","share",".license"):
    shutil.copytree(files/name,target/name,symlinks=True)
# System Qt5 supplies the runtime ABI. The old bundled Qt stack includes old
# private platform plugins and is not part of this package's search path.
shutil.rmtree(target/"lib/qt5")
for name in ("libFcitxQt5DBusAddons.so.1",):
    (target/"lib"/name).unlink(missing_ok=True)
for folder in (target,target/"lib",target/"share"):
    folder.chmod(0o755)
for path in target.rglob("*"):
    if not path.is_symlink():path.chmod(0o755 if path.is_dir() else 0o644)
for directory,name,input in [("addon","sogo-fc5-direct.conf","addon.conf"),
                             ("inputmethod","sogo-fc5-direct.conf","inputmethod.conf")]:
    dest=stage/"usr/share/fcitx5"/directory/name
    dest.parent.mkdir(parents=True,exist_ok=True)
    shutil.copyfile(source/"packaging"/input,dest)
doc=stage/"usr/share/doc/sogo-fc5";doc.mkdir(parents=True,exist_ok=True)
for name in ("README.md","README-CN.md","NOTICE.md","docs/USAGE.md","docs/ABI.md","vendor/baseline.json"):
    shutil.copyfile(source/name,doc/Path(name).name)
shutil.copyfile(files/"share/doc/license.html",doc/"sogou-license.html")
bin=stage/"usr/bin";bin.mkdir(parents=True,exist_ok=True)
shutil.copyfile(source/"packaging/status",bin/"sogo-fc5-direct-status")
(bin/"sogo-fc5-direct-status").chmod(0o755)
icons=stage/"usr/share/pixmaps";icons.mkdir(parents=True,exist_ok=True)
shutil.copyfile(source/"assets/SogoFC5.png",icons/"sogo-fc5.png")
(icons/"sogo-fc5.png").chmod(0o644)
themes=stage/"usr/share/fcitx5/themes"
for path in (source/"themes").iterdir():
    if path.is_dir():shutil.copytree(path,themes/path.name)
print(json.dumps({"official_engine":"4.2.1.145","desktop_frontend":"fcitx5","cpis":False,"vendor_executables":False}))

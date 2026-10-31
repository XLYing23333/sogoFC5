#!/bin/sh
set -eu
umask 022
export SOURCE_DATE_EPOCH=0
export DPKG_DEB_THREADS_MAX=1
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    printf '%s\n' 'Usage: build-deb.sh official-sogoupinyin.deb output.deb [fcitx5-sdk-root]' >&2
    exit 2
fi
source_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
package=$(realpath -- "$1")
output=$(realpath -m -- "$2")
build_root=$(mktemp -d "${SOGO_BUILD_PARENT:-/tmp}/sogo-direct-build.XXXXXX")
trap 'rm -rf -- "$build_root"' EXIT HUP INT TERM
python3 "$source_dir/tools/prepare_baseline.py" "$package" "$build_root/original"
if [ "$#" = 3 ]; then
    sdk=$(realpath -- "$3")
    cmake -S "$source_dir" -B "$build_root/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DSOGO_FC4_ROOT="$build_root/original" -DSOGO_SDK_ROOT="$sdk" -DCMAKE_PREFIX_PATH="$sdk/usr"
else
    cmake -S "$source_dir" -B "$build_root/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DSOGO_FC4_ROOT="$build_root/original"
fi
cmake --build "$build_root/build" --parallel "${SOGO_JOBS:-2}"
ctest --test-dir "$build_root/build" --output-on-failure
DESTDIR="$build_root/stage" cmake --install "$build_root/build" --prefix /usr
python3 "$source_dir/packaging/stage.py" "$build_root/original" "$build_root/stage"
mkdir -p "$build_root/stage/DEBIAN" "$(dirname -- "$output")"
cp "$source_dir/packaging/control" "$build_root/stage/DEBIAN/control"
find "$build_root/stage" -exec touch -h -d '@0' {} +
dpkg-deb --root-owner-group --uniform-compression -Zxz -z6 --build "$build_root/stage" "$output"
python3 "$source_dir/packaging/check.py" "$output"
python3 - "$output" <<'PY'
from pathlib import Path
import hashlib,sys
p=Path(sys.argv[1]);value=hashlib.sha256(p.read_bytes()).hexdigest()
p.with_name(p.name+'.sha256').write_text(value+'  '+p.name+'\n')
print(value)
PY

#!/bin/sh
# Run from the source root on FreeBSD. Uses an isolated test home.
set -eu
if [ "$(uname -s)" != FreeBSD ]; then
    echo "Run this script on FreeBSD with the build dependencies installed." >&2
    exit 1
fi
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_PREFIX_PATH=/usr/local -DBUILD_TESTS=ON \
    -DHYPRFM_ENABLE_PCH=OFF -DHYPRFM_ENABLE_UNITY_BUILD=OFF
cmake --build build --parallel "$(sysctl -n hw.ncpu)"
test_root=$(mktemp -d -t hyprfm-tests)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM
mkdir -p "$test_root/home" "$test_root/config" "$test_root/cache" "$test_root/data"
env HOME="$test_root/home" XDG_CONFIG_HOME="$test_root/config" \
    XDG_CACHE_HOME="$test_root/cache" XDG_DATA_HOME="$test_root/data" \
    QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
    dbus-run-session -- ctest --test-dir build --output-on-failure -E tst_instance_launch

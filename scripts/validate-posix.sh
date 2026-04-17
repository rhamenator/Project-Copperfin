#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
build_dir=${COPPERFIN_BUILD_DIR:-"$repo_root/build"}
build_type=${COPPERFIN_BUILD_TYPE:-Release}

if [ -f "$repo_root/.codex-venv/bin/activate" ]; then
    # shellcheck disable=SC1091
    . "$repo_root/.codex-venv/bin/activate"
fi

if [ ! -f "$build_dir/CMakeCache.txt" ]; then
    if command -v ninja >/dev/null 2>&1; then
        cmake -S "$repo_root" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type" -G Ninja
    else
        cmake -S "$repo_root" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type"
    fi
fi

cmake --build "$build_dir" "$@"
ctest --test-dir "$build_dir" --output-on-failure

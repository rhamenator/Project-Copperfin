#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
build_dir=${COPPERFIN_BUILD_DIR:-"$repo_root/build"}
build_type=${COPPERFIN_BUILD_TYPE:-Release}
build_jobs=${COPPERFIN_BUILD_JOBS:-2}

case $build_jobs in
    ''|*[!0-9]*)
        echo "validate-posix.sh: COPPERFIN_BUILD_JOBS must be a positive integer" >&2
        exit 2
        ;;
esac
if [ "$build_jobs" -lt 1 ]; then
    echo "validate-posix.sh: COPPERFIN_BUILD_JOBS must be a positive integer" >&2
    exit 2
fi

venv_dir="$repo_root/.codex-venv"
venv_cfg="$venv_dir/pyvenv.cfg"

# A venv is tied to the exact Python minor version that created it. If the
# system's python3 has since moved to a different minor version (e.g. a
# routine OS update), the venv's internal paths go stale and it silently
# shadows a working system cmake/ctest with a broken pair. Detect that drift
# and rebuild rather than limping along with a broken venv.
if [ -f "$venv_dir/bin/activate" ] && [ -f "$venv_cfg" ] && command -v python3 >/dev/null 2>&1; then
    venv_python_version=$(sed -n 's/^version *= *//p' "$venv_cfg" | head -n1)
    current_python_version=$(python3 --version 2>&1 | awk '{print $2}')
    venv_python_minor=${venv_python_version%.*}
    current_python_minor=${current_python_version%.*}
    if [ -n "$venv_python_minor" ] && [ -n "$current_python_minor" ] && [ "$venv_python_minor" != "$current_python_minor" ]; then
        echo "validate-posix.sh: .codex-venv was built for Python $venv_python_minor but python3 is now $current_python_minor; rebuilding .codex-venv" >&2
        rm -rf "$venv_dir"
        if python3 -m venv "$venv_dir" && "$venv_dir/bin/pip" install --quiet cmake; then
            echo "validate-posix.sh: rebuilt .codex-venv for Python $current_python_minor" >&2
        else
            echo "validate-posix.sh: failed to rebuild .codex-venv; falling back to system cmake/ctest" >&2
            rm -rf "$venv_dir"
        fi
    fi
fi

if [ -f "$venv_dir/bin/activate" ]; then
    # shellcheck disable=SC1091
    . "$venv_dir/bin/activate"
fi

configure_build() {
    if [ "${1:-}" = "fresh" ] && command -v ninja >/dev/null 2>&1; then
        cmake -S "$repo_root" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type" -G Ninja
    else
        cmake -S "$repo_root" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type"
    fi
}

cached_build_type=
if [ -f "$build_dir/CMakeCache.txt" ]; then
    cached_build_type=$(sed -n 's/^CMAKE_BUILD_TYPE:[^=]*=//p' "$build_dir/CMakeCache.txt" | head -n1)
fi

if [ ! -f "$build_dir/CMakeCache.txt" ]; then
    configure_build fresh
elif [ "$cached_build_type" != "$build_type" ]; then
    echo "validate-posix.sh: cached CMAKE_BUILD_TYPE=${cached_build_type:-<unset>} does not match requested ${build_type}; reconfiguring" >&2
    configure_build
fi

cmake --build "$build_dir" --parallel "$build_jobs" "$@"
ctest --test-dir "$build_dir" --output-on-failure --timeout 180 --parallel "$build_jobs"

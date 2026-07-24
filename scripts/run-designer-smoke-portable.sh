#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
configuration=${COPPERFIN_MANAGED_BUILD_CONFIGURATION:-Release}
display_mode=${COPPERFIN_UI_DISPLAY_MODE:-auto}
skip_build=${COPPERFIN_HEADLESS_SKIP_BUILD:-0}
smoke_project="$repo_root/vsix/Copperfin.DesignerSmokeTests/Copperfin.DesignerSmokeTests.csproj"
smoke_exe="$repo_root/vsix/Copperfin.DesignerSmokeTests/bin/$configuration/net472/Copperfin.DesignerSmokeTests.exe"
smoke_args=
list_tests=0

usage() {
    cat <<'EOF'
Usage: run-designer-smoke-portable.sh [runner options] [smoke-test options]

Runner options:
  --configuration <name>  Managed build configuration (default: Release)
  --display-mode <mode>   auto, xvfb, existing, or offscreen (listing only)
  --no-build              Use the existing DesignerSmoke executable
  --help                  Show this help

Smoke-test options are passed through:
  --list-tests, --filter <substring>, --exact <test-name>

Environment:
  COPPERFIN_MANAGED_BUILD_CONFIGURATION, COPPERFIN_HEADLESS_SKIP_BUILD,
  COPPERFIN_UI_DISPLAY_MODE
EOF
}

append_arg() {
    if [ -n "$smoke_args" ]; then
        smoke_args="$smoke_args
$1"
    else
        smoke_args="$1"
    fi
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --configuration)
            [ "$#" -ge 2 ] || { echo "run-designer-smoke-portable.sh: --configuration requires a value" >&2; exit 2; }
            configuration=$2
            smoke_exe="$repo_root/vsix/Copperfin.DesignerSmokeTests/bin/$configuration/net472/Copperfin.DesignerSmokeTests.exe"
            shift 2
            ;;
        --display-mode)
            [ "$#" -ge 2 ] || { echo "run-designer-smoke-portable.sh: --display-mode requires a value" >&2; exit 2; }
            display_mode=$2
            shift 2
            ;;
        --no-build)
            skip_build=1
            shift
            ;;
        --help)
            usage
            exit 0
            ;;
        --list-tests)
            list_tests=1
            append_arg "$1"
            shift
            ;;
        --filter|--exact)
            [ "$#" -ge 2 ] || { echo "run-designer-smoke-portable.sh: $1 requires a value" >&2; exit 2; }
            append_arg "$1"
            append_arg "$2"
            shift 2
            ;;
        --)
            shift
            while [ "$#" -gt 0 ]; do
                append_arg "$1"
                shift
            done
            ;;
        *)
            append_arg "$1"
            shift
            ;;
    esac
done

case "$display_mode" in
    auto|xvfb|existing|offscreen) ;;
    *)
        echo "run-designer-smoke-portable.sh: unsupported display mode '$display_mode' (use auto, xvfb, existing, or offscreen)" >&2
        exit 2
        ;;
esac

if ! command -v dotnet >/dev/null 2>&1; then
    echo "run-designer-smoke-portable.sh: dotnet is required to build DesignerSmokeTests" >&2
    exit 2
fi

if ! command -v mono >/dev/null 2>&1; then
    echo "run-designer-smoke-portable.sh: mono is required to run the net472 WinForms smoke executable" >&2
    exit 2
fi

if [ "$skip_build" != "1" ]; then
    dotnet build "$smoke_project" \
        --configuration "$configuration" \
        -p:EnableWindowsTargeting=true \
        --nologo
fi

if [ ! -f "$smoke_exe" ]; then
    echo "run-designer-smoke-portable.sh: smoke executable was not found: $smoke_exe" >&2
    exit 2
fi

set_smoke_args() {
    set --
    if [ -n "$smoke_args" ]; then
        old_ifs=$IFS
        IFS='
'
        # Each selector is stored on its own line. Selector values may contain
        # spaces, so split only on newlines rather than using an array extension.
        set -- $smoke_args
        IFS=$old_ifs
    fi
    SMOKE_ARG_COUNT=$#
    SMOKE_ARGS="$*"
}

run_smoke() {
    set_smoke_args
    if [ "$SMOKE_ARG_COUNT" -eq 0 ]; then
        env MONO_MWF_OFFSCREEN=0 mono "$smoke_exe"
    else
        # Reconstruct the positional list using newline-only splitting from the
        # caller's selector buffer before invoking Mono.
        old_ifs=$IFS
        IFS='
'
        set -- $smoke_args
        IFS=$old_ifs
        env MONO_MWF_OFFSCREEN=0 mono "$smoke_exe" "$@"
    fi
}

run_listing() {
    old_ifs=$IFS
    IFS='
'
    set -- $smoke_args
    IFS=$old_ifs
    env MONO_MWF_OFFSCREEN=1 mono "$smoke_exe" "$@"
}

if [ "$list_tests" = "1" ] || [ "$display_mode" = "offscreen" ]; then
    if [ "$list_tests" != "1" ]; then
        echo "run-designer-smoke-portable.sh: offscreen mode is supported only with --list-tests" >&2
        exit 2
    fi
    run_listing
    exit 0
fi

run_with_xvfb() {
    if ! command -v xvfb-run >/dev/null 2>&1; then
        echo "run-designer-smoke-portable.sh: xvfb-run is required for virtual-display UI tests" >&2
        echo "run-designer-smoke-portable.sh: install Xvfb (Linux) or provide an existing X display (macOS/XQuartz)" >&2
        exit 2
    fi
    old_ifs=$IFS
    IFS='
'
    set -- $smoke_args
    IFS=$old_ifs
    xvfb-run \
        --auto-servernum \
        --server-args="-screen 0 1600x1200x24 -nolisten tcp" \
        env MONO_MWF_OFFSCREEN=0 mono "$smoke_exe" "$@"
}

run_with_existing_display() {
    if [ -z "${DISPLAY:-}" ]; then
        echo "run-designer-smoke-portable.sh: an existing display is required; set DISPLAY or use --display-mode xvfb" >&2
        exit 2
    fi
    run_smoke
}

case "$display_mode" in
    xvfb)
        run_with_xvfb
        ;;
    existing)
        run_with_existing_display
        ;;
    auto)
        if [ -n "${DISPLAY:-}" ]; then
            run_with_existing_display
        elif command -v xvfb-run >/dev/null 2>&1; then
            run_with_xvfb
        else
            echo "run-designer-smoke-portable.sh: no usable display found for WinForms UI tests" >&2
            echo "run-designer-smoke-portable.sh: Linux: install Xvfb; macOS: start XQuartz and export DISPLAY; or use --list-tests" >&2
            exit 2
        fi
        ;;
esac

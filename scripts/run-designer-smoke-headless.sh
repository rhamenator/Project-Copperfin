#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
configuration=${COPPERFIN_MANAGED_BUILD_CONFIGURATION:-Release}
smoke_project="$repo_root/vsix/Copperfin.DesignerSmokeTests/Copperfin.DesignerSmokeTests.csproj"
smoke_exe="$repo_root/vsix/Copperfin.DesignerSmokeTests/bin/$configuration/net472/Copperfin.DesignerSmokeTests.exe"

if ! command -v dotnet >/dev/null 2>&1; then
    echo "run-designer-smoke-headless.sh: dotnet is required to build DesignerSmokeTests" >&2
    exit 2
fi

if ! command -v mono >/dev/null 2>&1; then
    echo "run-designer-smoke-headless.sh: mono is required to run the net472 WinForms smoke executable" >&2
    exit 2
fi

if [ "${COPPERFIN_HEADLESS_SKIP_BUILD:-0}" != "1" ]; then
    dotnet build "$smoke_project" \
        --configuration "$configuration" \
        -p:EnableWindowsTargeting=true \
        --nologo
fi

if [ ! -f "$smoke_exe" ]; then
    echo "run-designer-smoke-headless.sh: smoke executable was not found: $smoke_exe" >&2
    exit 2
fi

if command -v xvfb-run >/dev/null 2>&1; then
    exec xvfb-run \
        --auto-servernum \
        --server-args="-screen 0 1600x1200x24 -nolisten tcp" \
        env MONO_MWF_OFFSCREEN=0 mono "$smoke_exe" "$@"
fi

for argument in "$@"; do
    if [ "$argument" = "--list-tests" ]; then
        # Listing does not initialize WinForms, so it remains useful without Xvfb.
        unset DISPLAY
        exec env MONO_MWF_OFFSCREEN=1 mono "$smoke_exe" "$@"
    fi
done

echo "run-designer-smoke-headless.sh: xvfb-run is required for WinForms UI tests" >&2
echo "run-designer-smoke-headless.sh: install Xvfb or run on a host with a virtual X display; Mono offscreen mode alone cannot initialize this target" >&2
exit 2

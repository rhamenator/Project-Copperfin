#!/usr/bin/env bash
set -u
WINEDEBUG=-all wine /home/rich/temp/copperfin-vfp9-wine-probe/vfp9.exe -t -cZ:\\home\\rich\\.codex\\worktrees\\getfont-size-numeric-5611\\Project-Copperfin\\tests\\fixtures\\vfp9-getfont-size-numeric\\probe.fpw >build/getfont-native/wine.stdout 2>build/getfont-native/wine.stderr &
task_vfp_pid=$!
trap 'kill -TERM "$task_vfp_pid" 2>/dev/null || true; wait "$task_vfp_pid" 2>/dev/null || true' EXIT
task_last_window=''
for ((task_tick=0;task_tick<500;task_tick++)); do
    if ((task_tick==30)); then
        xdotool search --onlyvisible --name '.*' getwindowname 2>/dev/null
        import -window root build/getfont-native/diagnostic.png
    fi
    if ! kill -0 "$task_vfp_pid" 2>/dev/null; then
        wait "$task_vfp_pid"
        task_rc=$?
        trap - EXIT
        exit "$task_rc"
    fi
    task_window=$(xdotool search --onlyvisible --name '^Font$' 2>/dev/null | head -1)
    if [[ -n "$task_window" && "$task_window" != "$task_last_window" && -f build/getfont-native/probe.out ]]; then
        task_action=$(rg '^BEGIN\|' build/getfont-native/probe.out | tail -1 | cut -d'|' -f3 | tr -d '\r')
        if [[ "$task_action" == Return || "$task_action" == Escape ]]; then
            printf 'DIALOG|%s|%s\n' "$task_window" "$task_action"
            printf 'SOURCE|%s\n' "$(rg '^BEGIN\|' build/getfont-native/probe.out | tail -1 | tr -d '\r')"
            import -window "$task_window" "build/getfont-native/dialog-$task_window.png"
            xdotool key --window "$task_window" "$task_action"
            task_last_window=$task_window
        fi
    fi
    sleep 0.1
done
xdotool search --onlyvisible --name '.*' getwindowname 2>/dev/null
exit 124

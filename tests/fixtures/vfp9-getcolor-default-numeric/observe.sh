#!/usr/bin/env bash
# Recovery only: run on a task-owned private Xvfb display, never user desktop.
set -u
WINEDEBUG=-all wine /home/rich/temp/copperfin-vfp9-wine-probe/vfp9.exe -t -cZ:\\home\\rich\\.codex\\worktrees\\getcolor-default-numeric-5611\\Project-Copperfin\\tests\\fixtures\\vfp9-getcolor-default-numeric\\probe.fpw >build/getcolor-native/wine.stdout 2>build/getcolor-native/wine.stderr &
task_vfp_pid=$!
trap 'kill -TERM "$task_vfp_pid" 2>/dev/null || true; wait "$task_vfp_pid" 2>/dev/null || true' EXIT
task_last_window=''
task_last_source=''
task_attempts=0
for ((task_tick=0;task_tick<550;task_tick++)); do
    if ! kill -0 "$task_vfp_pid" 2>/dev/null; then
        wait "$task_vfp_pid"
        task_rc=$?
        trap - EXIT
        exit "$task_rc"
    fi
    task_window=$(xdotool search --onlyvisible --name '^Color$' 2>/dev/null | head -1)
    if [[ -n "$task_window" && -f build/getcolor-native/probe.out ]]; then
        task_source=$(tail -1 build/getcolor-native/probe.out | tr -d '\r')
        if [[ "$task_source" != BEGIN\|* ]]; then
            sleep 0.1
            continue
        fi
        if [[ "$task_window|$task_source" != "$task_last_window|$task_last_source" ]]; then
            task_attempts=0
        fi
        task_action=$(printf '%s' "$task_source" | cut -d'|' -f3)
        if ((task_attempts<5)) && [[ "$task_action" == Return || "$task_action" == Escape ]]; then
            printf 'DIALOG|%s|%s|attempt=%s\nSOURCE|%s\n' "$task_window" "$task_action" "$task_attempts" "$task_source"
            if [[ "$task_source" == 'BEGIN|GETCOLOR(255.9)|Return' || "$task_source" == 'BEGIN|GETCOLOR(-4294967041)|Return' ]]; then
                import -window "$task_window" "build/getcolor-native/dialog-$task_window.png"
            fi
            sleep 0.15
            xdotool key --window "$task_window" "$task_action"
            task_last_window=$task_window
            task_last_source=$task_source
            ((task_attempts+=1))
        fi
    fi
    sleep 0.1
done
xdotool search --onlyvisible --name '.*' getwindowname 2>/dev/null
exit 124

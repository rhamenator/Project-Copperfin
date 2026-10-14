#!/usr/bin/env bash
# FUTURE fixed95 only, after completed BOTH original gate AND bounded wiring.
# RQ-CF-PRG-ADDLISTITEM-COLUMN-NUMERIC-001 / VR-5611-ADDLISTITEM-COLUMN-NUMERIC-001/002.
set -u
cd /home/rich/.codex/worktrees/addlistitem-column-numeric-5611/Project-Copperfin || exit 2
evidence=build/addlistitem-column-native
node - "$evidence" > "$evidence/fixed-preflight.log" 2> "$evidence/fixed-preflight.stderr" <<'JS'
const fs=require('fs'),assert=require('assert'),crypto=require('crypto'),path=require('path');
const evidence=process.argv[2],read=f=>fs.readFileSync(path.join(evidence,f),'utf8');
assert.strictEqual(read('originals-complete-audit.status').trim(),'0','BOTH original95 gate before wiring');
const gate=read('originals-complete-audit.jsonl').trim().split('\n').map(l=>JSON.parse(l)).at(-1);
assert.strictEqual(gate.complete,true);assert.strictEqual(gate.identicalSelectedFullFailures,true);
const parse=raw=>raw.trim().split('\n').map(l=>{const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m);return [m[1],m[2]];});
const original=parse(read('frozen-original-inputs.sha256')),fixed=parse(read('fixed-inputs.sha256'));
assert.strictEqual(original.length,259);assert.strictEqual(fixed.length,259);assert.strictEqual(new Set(fixed.map(e=>e[1])).size,259);
assert.deepStrictEqual(fixed.map(e=>e[1]),original.map(e=>e[1]));
assert.deepStrictEqual(fixed.filter((e,i)=>e[0]!==original[i][0]).map(e=>e[1]),['src/runtime/prg_engine_runtime_surface_object_methods.inl'],'only bounded caller migration');
for(const [hash,file] of fixed)assert.strictEqual(crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex'),hash,file);
console.log('Completed original gate and exact changed-caller/unchanged258/current259 preflight verified. No hostile invocation in this driver.');
JS
result=$?
printf '%s\n' "$result" > "$evidence/fixed-preflight.status"
[ "$result" = 0 ] || exit 3
export TMPDIR=/home/rich/.codex/worktrees/addlistitem-column-numeric-5611/Project-Copperfin/build/addlistitem-column-native/tmp
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
targets=(test_prg_engine_addlistitem_column_numeric test_prg_engine_additem_column_numeric test_prg_engine_additem_numeric test_prg_engine_addlistitem_numeric test_prg_engine_removelistitem_numeric test_prg_engine_removeitem_numeric test_prg_engine_topitemid_numeric test_prg_engine_topindex_numeric test_prg_engine_listitemid_numeric test_prg_engine_listindex_numeric test_prg_engine_list_index_numeric test_prg_engine_listitem_numeric test_prg_engine_itemidtoindex_numeric test_prg_engine_indextoitemid_numeric test_prg_engine_selectedid_numeric test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior test_prg_engine_int64_exact_expression test_prg_engine_runtime_surface_functions test_prg_engine_runtime_surface_itemid_selectors)
regex='^(test_native_test_isolation_contract|test_prg_engine_((addlistitem_column|additem_column|additem|addlistitem|removelistitem|removeitem|topitemid|topindex|listitemid|listindex|list_index|listitem|itemidtoindex|indextoitemid|selectedid|selected_index|itemdata_index|getcolor_default)_numeric(_.*)?|numeric_behavior|int64_exact_expression|runtime_surface_functions|runtime_surface_itemid_selectors))$'
run_compiler() {
 local name="$1" build="build/addlistitem-column-$1" result
 sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/fixed-$name-inputs-before.log" || return 4
 # Reuse the original compiler/configuration/flags; do not reconfigure tests.
 nice -n 10 ionice -c 2 -n 7 cmake --build "$build" --parallel 1 --target "${targets[@]}" > "$evidence/fixed-compile-$name.log" 2>&1
 result=$?
 printf '%s\n' "$result" > "$evidence/fixed-compile-$name.status"
 [ "$result" = 0 ] || return 5
 ctest --test-dir "$build" -N -R "$regex" > "$evidence/fixed-$name-test-list.log" 2>&1 || return 6
 rg -q 'Total Tests: 95$' "$evidence/fixed-$name-test-list.log" || return 6
 ctest --test-dir "$build" --show-only=json-v1 -R "$regex" > "$evidence/fixed-$name-test-metadata.json" 2> "$evidence/fixed-$name-test-metadata.stderr" || return 6
 node tests/fixtures/vfp9-addlistitem-column-numeric/audit-test-metadata.cjs "$evidence/fixed-$name-test-metadata.json" > "$evidence/fixed-$name-metadata-audit.jsonl" 2> "$evidence/fixed-$name-metadata-audit.stderr" || return 6
 nice -n 10 ionice -c 2 -n 7 ctest --test-dir "$build" -V -R "$regex" --parallel 1 --timeout 700 > "$evidence/fixed-$name.log" 2>&1
 result=$?
 printf '%s\n' "$result" > "$evidence/fixed-$name.status"
 date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/fixed-$name.completed-at"
 sha256sum "$evidence/fixed-$name.log" > "$evidence/fixed-$name.log.sha256"
 sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/fixed-$name-inputs-after.log" || return 7
 printf '%s fixed95 completed with actual CTest status %s\n' "$name" "$result"
 return "$result"
}
run_compiler gcc
gcc_status=$?
run_compiler clang
clang_status=$?
[ "$gcc_status" = 0 ] && [ "$clang_status" = 0 ] || exit 8
printf 'Both fixed95 complete; full raw/counter/source audit still required. Hostile182 and preservation304 are separate gates.\n'

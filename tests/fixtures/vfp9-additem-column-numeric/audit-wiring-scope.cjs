// Structural scope/dataflow audit, not a native oracle or replacement for tests.
const fs=require('fs'),crypto=require('crypto');
const p='src/runtime/prg_engine_runtime_surface_object_methods.inl';
const oldFull=fs.readFileSync('build/additem-column-native/initial-source/'+p,'utf8');
const nowFull=fs.readFileSync(p,'utf8');let failures=0;
const marker='    if (normalized_method_name != "additem") {';
const old=oldFull.slice(oldFull.indexOf(marker)),now=nowFull.slice(nowFull.indexOf(marker));
const prefixEqual=oldFull.slice(0,oldFull.indexOf(marker))===nowFull.slice(0,nowFull.indexOf(marker));
if(!prefixEqual)++failures;
console.log(JSON.stringify({name:'Entire preceding module (other methods)',unchanged:prefixEqual}));
const between=(s,start,end)=>s.slice(s.indexOf(start),s.indexOf(end,s.indexOf(start)));
const compare=(name,start,end)=>{const a=between(old,start,end),b=between(now,start,end);
 const unchanged=a.length>0&&a===b;if(!unchanged)++failures;
 console.log(JSON.stringify({name,unchanged,SHA256:crypto.createHash('sha256').update(b).digest('hex')}));};
// Compare the exact relocated predicate, excluding the new column block.
const start='    const auto row_source_type = runtime_object.properties.find("rowsourcetype");';
const end='        runtime_object.list_rows.size() == runtime_object.collection_items.size();';
const originalPredicate=between(old,start,end)+end,currentPredicate=between(now,start,end)+end;
const predicateEqual=originalPredicate===currentPredicate;
if(!predicateEqual)++failures;
console.log(JSON.stringify({name:'Precise unbound predicate bytes',unchanged:predicateEqual}));
compare('Shipped second-position arm and shared insertion-slot resolver call',
 '    std::optional<std::size_t> insert_slot;',
 '    const PrgValue inserted_item = make_string_value(value_as_string(arguments[0]));');
compare('Candidate and shared automatic-ID allocation',
 '    const PrgValue inserted_item = make_string_value(value_as_string(arguments[0]));',
 '    std::vector<PrgValue> inserted_row;');
compare('Shared insertion/synchronization/sort/return tail',
 '    runtime_object.list_rows.insert(',
 'std::optional<NativeListControlCellReference> parse_native_list_control_list_member_cell(');
const fallback=between(old,'    long long requested_column = 1LL;','    // RQ-CF-PRG-ADDITEM-POSITION-NUMERIC-001')
 .split('    if (arguments.size() >= 3U) {')[1];
const newFallback=between(now,'    } else if (arguments.size() >= 3U) {','    std::optional<std::size_t> insert_slot;')
 .split('    } else if (arguments.size() >= 3U) {')[1];
const fallbackEqual=fallback.trimEnd()===newFallback.trimEnd();
if(!fallbackEqual)++failures;
console.log(JSON.stringify({name:'Fallback third-column llround/return body (trailing separator whitespace excluded)',unchanged:fallbackEqual}));
const selected=between(now,'    const bool ordinary_position =','    std::optional<std::size_t> insert_slot;');
const checks=['arguments.size() == 3U','arguments[1].kind == PrgValueKind::number',
 'std::isfinite(arguments[1].number_value)','std::trunc(arguments[1].number_value)',
 'checked_truncated_numeric_to_int64(arguments[1].number_value).has_value()',
 'checked_additem_position_argument(arguments[1], NumericBehavior::copperfin,',
 'ordinary_position && materialized_unbound','!arguments[0].is_null',
 'arguments[0].kind == PrgValueKind::string','!arguments[2].is_null',
 'arguments[2].kind == PrgValueKind::number','arguments[2].kind == PrgValueKind::int64',
 'arguments[2].kind == PrgValueKind::uint64',
 'checked_additem_column_argument(arguments[2], behavior)',
 'if (!column.has_value())','Runtime.Prg.Expression.Error.InvalidArgument','11',
 'static_cast<long long>(*column)'];
for(const check of checks)if(!selected.includes(check))++failures;
const prep=between(now,'    std::vector<PrgValue> inserted_row;','    runtime_object.list_rows.insert(');
if(!prep.includes('requested_column == 0LL ? 1LL : requested_column')||
 !prep.includes('if (requested_column > 0LL)')||
 prep.indexOf('if (requested_column > 0LL)')>prep.indexOf('requested_column - 1LL'))++failures;
console.log(JSON.stringify({failures,note:'Byte attribution plus selected guard/zero-index dataflow. Both fixed90, hostile542, preservation296, contracts and reviews remain required; no global atomicity or native shared-state qualification.'}));
process.exitCode=failures?1:0;

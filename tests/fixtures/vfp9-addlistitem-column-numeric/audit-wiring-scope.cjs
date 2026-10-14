// RQ-CF-PRG-ADDLISTITEM-COLUMN-NUMERIC-001 / VR-5611-ADDLISTITEM-COLUMN-NUMERIC-001/002.
// Structural preservation corroboration, NOT a behavioral/native oracle.
const fs=require('fs'),assert=require('assert'),cp=require('child_process'),crypto=require('crypto');
const file='src/runtime/prg_engine_runtime_surface_object_methods.inl';
const base='5e1b259a65f909fb4c5ce3ae30cb48e869098778';
const original=cp.execFileSync('git',['show',base+':'+file],{encoding:'utf8'});
const originalAdmission=`        long long requested_column = 1LL;
        if (arguments.size() >= 3U) {
            requested_column = std::llround(value_as_number(arguments[2]));
            if (requested_column < 1LL) {
                return make_number_value(0.0);
            }
        }

`;
const originalWrite=`        auto& row = runtime_object.list_rows[*slot];
        if (row.size() < static_cast<std::size_t>(requested_column)) {
            row.resize(static_cast<std::size_t>(requested_column), make_string_value(""));
        }
        row[static_cast<std::size_t>(requested_column - 1LL)] = inserted_item;
`;
function audit(current){
 const methodStart=current.indexOf('    if (normalized_method_name == "addlistitem") {');
 const methodEnd=current.indexOf('    if (normalized_method_name == "clear") {',methodStart);
 assert(methodStart>=0&&methodEnd>methodStart);
 const method=current.slice(methodStart,methodEnd);
 const start=method.indexOf('        long long requested_column = 1LL;');
 const end=method.indexOf('        const PrgValue inserted_item =',start);
 const admission=method.slice(start,end);
 assert(start>=0&&end>start);
 for(const token of [
  'bool admitted_no_write = false;',
  'arguments.size() == 3U && materialized_unbound &&',
  '!arguments[0].is_null && arguments[0].kind == PrgValueKind::string &&',
  '!supplied_column.is_null &&',
  'supplied_column.kind == PrgValueKind::number ||',
  'supplied_column.kind == PrgValueKind::int64 ||',
  'supplied_column.kind == PrgValueKind::uint64)',
  '!supplied_id.is_null && supplied_id.kind == PrgValueKind::number &&',
  'std::isfinite(supplied_id.number_value) &&',
  'supplied_id.number_value >= 1.0 && supplied_id.number_value <= 32767.0 &&',
  'std::trunc(supplied_id.number_value) == supplied_id.number_value &&',
  'static_cast<long long>(supplied_id.number_value)).has_value()) {',
  'const auto column = checked_addlistitem_column_argument(supplied_column, behavior);',
  'if (!column.has_value()) {',
  'throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.InvalidArgument"), 11);',
  'admitted_no_write = *column == 0U;',
  'requested_column = std::llround(value_as_number(arguments[2]));',
  'if (requested_column < 1LL && !admitted_no_write) {'
 ])assert(admission.includes(token),'bounded admission '+token);
 for(const buffer of ['collection_item_keys','list_rows','list_item_data','list_selected'])
  assert(admission.includes('runtime_object.'+buffer+'.size() == runtime_object.collection_items.size()'),'coherent '+buffer);
 for(const token of ['row_source_type == runtime_object.properties.end()',
  '!row_source_type->second.is_null','row_source_type->second.number_value == 0.0',
  'row_source_type->second.int64_value == 0','row_source_type->second.uint64_value == 0U',
  'control_source == runtime_object.properties.end()', '!control_source->second.is_null',
  'control_source->second.kind == PrgValueKind::string',
  'trim_copy(control_source->second.string_value).empty()'])assert(admission.includes(token),token);
 assert(admission.indexOf('std::isfinite(supplied_id.number_value)')<admission.indexOf('static_cast<long long>(supplied_id.number_value)'),'ID proof before cast');
 assert(!/row\.resize|list_rows\[|properties\[/.test(admission),'no selected-row mutation in admission');
 assert.strictEqual((current.match(/checked_addlistitem_column_argument\(/g)||[]).length,1,'one bounded call');
 const writeStart=method.indexOf('        // Engaged0 is an admitted existing-ID no-write');
 const writeEnd=method.indexOf('        sync_native_list_control_primary_state_from_rows(runtime_object);',writeStart);
 assert(writeStart>end&&writeEnd>writeStart);
 const protectedWrite=method.slice(writeStart,writeEnd);
 const expectedWrite=`        // Engaged0 is an admitted existing-ID no-write, NEVER zero-1 indexing
        // or argument-driven resize; retain the original shared success work.
        if (!admitted_no_write) {
${originalWrite.trimEnd().split('\n').map(l=>'    '+l).join('\n')}
        }
`;
 assert.strictEqual(protectedWrite,expectedWrite,'exact protected resize/index write');
 const restored=current.slice(0,methodStart)+method.slice(0,start)+originalAdmission+
  method.slice(end,writeStart)+originalWrite+method.slice(writeEnd)+current.slice(methodEnd);
 assert.strictEqual(restored,original,'all other caller bytes, shipped ID/caption/fallback/shared-success methods unchanged');
 return {base,callerSHA256:crypto.createHash('sha256').update(current).digest('hex'),
  onlyTwoBoundedCallerRegions:true,unchangedOtherCallerBytes:true,ordinaryIdProofBeforeCast:true,
  protectedNoWrite:true,note:'Structural corroboration only; complete runtime/native/source gates remain independent.'};
}
const current=fs.readFileSync(file,'utf8');
if(process.argv[2]==='--selftest'){
 audit(current);
 const mutations=[['arguments.size() == 3U && materialized_unbound &&','arguments.size() >= 3U && materialized_unbound &&'],
  ['!supplied_column.is_null &&','true &&'],['std::isfinite(supplied_id.number_value) &&','true &&'],
  ['supplied_id.number_value <= 32767.0','supplied_id.number_value <= 65537.0'],
  ['std::trunc(supplied_id.number_value) == supplied_id.number_value &&','true &&'],
  ['runtime_object.list_selected.size() == runtime_object.collection_items.size()','true'],
  ['admitted_no_write = *column == 0U;','admitted_no_write = false;'],
  ['if (!admitted_no_write) {','if (true) {'],
  ['row[static_cast<std::size_t>(requested_column - 1LL)] = inserted_item;','row[0] = inserted_item;'],
  ['runtime_object.properties["newitemid"] = make_number_value(static_cast<double>(item_id));','runtime_object.properties["newitemid"] = make_number_value(0.0);']];
 for(const [before,after] of mutations){assert(current.includes(before));assert.throws(()=>audit(current.replace(before,after)));}
 console.log(JSON.stringify({syntheticOnly:true,positive:1,rejectedMutations:mutations.length}));
}else{assert.strictEqual(process.argv.length,2);console.log(JSON.stringify(audit(current)));}

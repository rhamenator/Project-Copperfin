## Current revision and scope

Observed while preparing non-native excluded-route preservation for the admitted
#5611/#6776 AddItem numeric-conversion slice. Current main:
`87b0fd854431381db977740e6d6af3a5b3b967ed`.

This is a separate, unadmitted gap. The active slice is limited to coherent
materialized-unbound RowSourceType=0 controls; this report does not authorize
a RowSourceType=1 repair, allocator repair (#7176), or expansion of that slice.

## Public reproduction

Run via PrgRuntimeSession using the existing public C++ test-support session
options, a private working directory, and this PRG:

```foxpro
SET NUMERICBEHAVIOR TO VFP9
SET DATASESSION TO 1
SET NUMERICBEHAVIOR TO COPPERFIN
PUBLIC o, vResult
TRY
    o = CREATEOBJECT('ListBox')
    o.ColumnCount = 2
    o.RowSourceType = 1
    o.RowSource = 'first,second,third'
    o.Requery()
    o.ListIndex = 2
    o.TopIndex = 2
    vResult = o.AddItem('candidate', 1.9)
CATCH TO problem
    ? problem.ErrorNo
ENDTRY
RETURN
```

The retained exact generated reproduction has additional before/after state
capture around the same call. Its first case is ListBox/COPPERFIN/session1/
RowSourceType1/argument1.9. No claim is made that all other classes, modes,
arguments or configurations were executed: the process died on this first case.

## Actual evidence

- Fresh dedicated slice GNU O1g0 runtime libraries, with the new distinct numeric
  helper still UNUSED (only declaration/definition), compile diagnostic0 but
  runtime exits139/SIGSEGV before a captured state row or DONE marker.
- GDB reproduces SIGSEGV in std::vector<std::string>::_M_insert_rval, with
  invoke_native_list_control_method immediately below it. GDB exit0 is a
  debugger invocation result, NOT program success.
- Independently linked the same diagnostic to all nine retained signed-head
  `2fb9b804552690212c9d97ce550012a27b8e22e7` GNU libraries: compile0 and GDB
  reproduces the same SIGSEGV/stack. Relevant complete src/include and
  test-support trees of that signed head versus current origin/main compare
  byte-identical (git diff status0). All nine retained library hashes verify
  unchanged afterwards. This is strong current-main attribution, not a claim
  of a second pristine whole-main rebuild.
- TRY/CATCH did not contain the process crash. No installed-VFP parity, exact
  cause, successful insertion semantics, leak or release qualification is
  claimed from this diagnostic.
- Source, exact PRG, full raw driver/stdout/stderr/status, GDB stacks, tree
  comparison and before/after library/source hashes are retained under the
  AddItem slice's tests/fixtures/vfp9-additem-numeric and ignored build evidence.

## Desired investigation boundary

Investigate why the RowSourceType1 materialized insertion reaches invalid
stable-key-vector insertion and provide safe runtime containment/appropriate
behavior with independently recovered requirements. Root cause is not yet
established. Keep this separate from selected coherent unbound numeric policy.

The failed 312-case preservation attempt remains failed, not a pass. Its
RowSourceType1 planned cases will be explicitly deferred to this issue; other
excluded-route preservation can be measured separately without erasing the crash.

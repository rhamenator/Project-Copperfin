# AddListItem second-argument numeric recovery (#5611/#6776)

## Authority and evidence boundary

Pre-code recovery on main/base `06d7b54107b5b22b20300108e1e19ef01357fb59`,
branch `fix/addlistitem-numeric-5611`. Both owner-authored parents remain OPEN
with exact `agent-approved` metadata. No helper, acceptance assertions,
test/configuration registration, production migration, PR or push exists yet.

Installed Visual FoxPro 09.00.0000.7423 was invoked through the existing
private Wine prefix, not a local VM. No VM start, clone, backup-reset completion,
credential access, installation, decompilation or disassembly occurred.
Runner `/home/rich/bin/vfp9-probe` SHA256:
`1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06`.
Native probe sources are observational capture, NOT acceptance assertions.
Copperfin/helper output is not an allowed requirement source.

Completely read shipped Microsoft/VFP help:

- AddListItem: 771 lines, SHA256
  `f4a11214d63557230094ea37f9cef305e2c12485fd8e590f133f72ae281d69e8`,
  original `/home/rich/temp/vfp9-help-len/html/317977bd-c3b4-4bce-8cfd-f376f6f3d4bf.htm`.
  Integer unique item ID, maximum32767, omitted column1, RowSourceType0.
- BINTOC: 564 lines, SHA256
  `3708551c6edd118a2c10fa2afc3392ac2dfd19c325ba5d064e25639ecc17fc80`,
  original `/home/rich/temp/vfp9-help-len/html/c3b28578-7b4f-487f-9192-fb619ff83ad6.htm`.
  B interprets Double; R/S flags are additive. Hex below is the measured
  BINTOC(v,'BRS') byte sequence, not a claim of otherwise undocumented byte order.
  Proprietary HTML is not copied here.

## Retained paired observations

Every listed first/repeat runner and bytewise cmp status is0, stderr empty,
and both full raw captures end with the specified DONE count.
Raw stdout/result files retain original CRLF bytes without normalization.
`evidence.sha256` covers source, full raw, statuses and supporting evidence;
generated FXP files remain only in the ignored build. Original absolute
output paths remain in native sources; use a separately named copy to rerun
elsewhere, never rewrite the retained source or raw.

| Pair / source | Calls per run | First full raw SHA256 |
| --- | ---: | --- |
| native / independent-probe.prg | 440 | 2bd7678d5bf6d57d3e25d902f6145a3286cef83602bed165fe6e91b12bc87d18 |
| boundary / boundary-probe.prg | 256 | be4bbe4a1dd928ae9564705d14e54b8adf301476d833eff290fad156b1ba520f |
| configuration / configuration-probe.prg | 96 | 57b39f69b7cb48805b9c1524a64a8630c965fd0bf5c88796404a80ae4982d7bb |
| precision / precision-probe.prg (evaluation only) | 6 | 276cd6493e405305e3e73a993f6d24703bd680103430a68444263f1e4068fa3d |
| precision-bits / precision-bits-probe.prg (evaluation only) | 6 | 9b82d2391924146e6e8f053a2eb16c849e6de3a31b9d1d271f1145bb08380859 |
| binary-neighbor / binary-neighbor-probe.prg | 70 | b03c7183da3a863ea5a8bcc71178c17d283af03a3075ac42b2995f0545061298 |
| no-touch-neighbor / no-touch-neighbor-probe.prg | 70 | b03c7183da3a863ea5a8bcc71178c17d283af03a3075ac42b2995f0545061298 |
| context / context-probe.prg | 24 | 0bf6e0d71adc587988de4b0a79f0ff601faeb0a2d6506eee60ad8f9c02e57b74 |
| concat-context / concat-context-probe.prg | 30 | 59bb6ce8b2608955db90ab4927a09ecf940449f5c89c3ba712a7bb12f3aa2854 |
| seed-context / seed-context-probe.prg | 30 | 3211352ec633211a13b04e1cdcb68ab9be76a1c099faa5f01c1ee4aa74e7f661 |
| boundary-postbits / boundary-postbits-probe.prg | 256 | 88755b8ce2a364f15d5fe63e324c31f1e76122208507d42829be4fe54e4aa084 |

Native/configuration snapshots explicitly distinguish setup/evaluation/invocation.
RST/binding are configured BEFORE population (separate setter lifecycle
gap#7171). Shapes: empty; dense1/2/3; sparse11/22/33; high-ID32767.
Captured count, stable IDs, two-column cells, ItemData, selection, focus,
Value/DisplayValue, NewIndex/NewItemID and ListBox top pair.

Capture audit `audit-native-capture.cjs` inspects932 consumer calls from
native/boundary/configuration/binary-neighbor/no-touch-neighbor:
360 rejected with error11 and byte-identical full before/after state,
462 append successes,110 existing-ID updates, failures0. Exact recheck
matches the original audit JSONL SHA256
`4c63ef5b2e030a404a666a93d26e8cccbd60e32c665595cbc28af978c61f4e7b`.
This validates capture consistency, not implementation acceptance or one
guessed native rule; other context probes are not included in that count.

## Precision attribution and correction

Decimal source0.9999999999999999 serializes as exact1 (3FF0000000000000).
Arithmetic1-2^(-52) and1-2^(-53) serialize below1
(3FEFFFFFFFFFFFFE / 3FEFFFFFFFFFFFFF), despite comparison-equality TRUE and
less-than FALSE. Both consumers reject11. Upper next-below32768 serializes
40DFFFFFFFFFFFFF and admits32767, despite equality to32768 TRUE.
Comparison flags alone are NOT binary identity measurements.

My earlier call-context-discrepancy interpretation was incorrect:
all sixteen original full-boundary near-one calls reject11. Removing only
the added postcallBRS field from the fresh full boundary-postbits output
makes all258 lines equal to the original. There is NO demonstrated
call-context discrepancy, parser/FPU/type/cache diagnosis, or production fix.
Both source/raw histories remain unchanged. Issue#7174 correction:
https://github.com/rhamenator/Project-Copperfin/issues/7174#issuecomment-6143310467
The independently valid older precision-attribution gap remains separate;
do not edit earlier RemoveListItem docs/code as part of this slice.

## Governing requirement, architecture and planned verification

RQ-CF-PRG-ADDLISTITEM-ID-NUMERIC-001 parents: admitted owner#5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, native/help evidence above, active
HZ-runtime-crash-01/HZ-data-corruption-01 containment.

Bound only supplied second Numeric/int64/uint64 ID with nonNULL Character
first item, exactly two arguments/omitted third column, coherent materialized-
unbound native ListBox/ComboBox, default or exact-typed RST0 and trimmed-empty
binding. Native ordinary finite admission truncates towardzero and admits
ID1..32767 (not display-row); invalid/nonfinite produces11 before mutation.
Explicit VFP9 mode projects BOTH signs to defined low32 before that domain:
positive/negative large aliases independently recover IDs1/2/3/11/32767.
Default finite/lossless exact policy, exact64 precision, NaN containment and
C++ binary-neighbor/subnormal behavior are named-parent derivations, not
claimed native exact64/NaN syntax measurements. Exact-typed config is derived
from owner extension policy plus native default/zero/space/tab bindings.

Planned architecture: distinct `checked_addlistitem_id_argument` UNUSED
through both complete original suites; only afterwards migrate the selected
column1 Numeric/exact arm in
`src/runtime/prg_engine_runtime_surface_object_methods.inl`.
Existing current-session mode, insertion path and successful state/return
remain unchanged. Other first/second types, omitted ID, explicit columns/extras,
binding/otherRST, lifecycle/callbacks/sorting/parser/other consumers excluded.

Native existing-ID calls successfully update first-column caption; current
main no-ops. Focused separate OPEN UNADMITTED gap#7173:
https://github.com/rhamenator/Project-Copperfin/issues/7173
Preserve that existing branch; no caption-update repair or full duplicate
native-parity claim. Native identity/count/retained-cell observations still
inform conversion routing; excluded current behavior needs preservation
diagnostics, not assertions falsely labeled native parity.

VR-5611-ADDLISTITEM-ID-NUMERIC-001 (planned): independent literal direct helper
table for both modes, finite/fraction/domain/wrap/huge/binary/nonfinite/exact64;
stable IDs rather than rows. VR-5611-ADDLISTITEM-ID-NUMERIC-002 (planned):
both classes/modes/session isolation, empty/dense/sparse/high-ID, config/type/
column exclusions, locale11 and full rejected-state assertions; retained
NON-NATIVE before/after diagnostics for excluded shared behavior.
Freeze all inputs before BOTH complete original GNU and Clang
ASan/UBSan/float-cast-overflow suites (all prior75 subjects retained).
Audit all failures and all before/after hashes before wiring, then repeat
both full fixed suites, scope comparisons and documentation contracts.
Signed exact-head post-push, actual clean Claude-FIRST/authorized Codex
review, all11 required green/resolved conversations remain integration gates.

DQ/DV-5611-ADDLISTITEM-ID-NUMERIC-001 (planned, NOT completed): procedural delta
from unchecked rounding to documented checked stable-ID admission; medium
misuse from confusing IDs/rows, comparison flags/bits, config-before-population,
source evaluation/consumer invocation and current/native duplicate branches.
Walkthrough must verify those distinctions plus every source/hash/result link.
Rollback incorrect docs using reviewed revision; notify linked slice/issue and
retain superseding correction. Active HZ-doc-command-01 remains open.
No independent-human/high-hazard/persistence/whole-call atomicity/leak-
warningfree/performance/system/release/certification or parent/hazard closure.

## NON-NATIVE current-main diagnostic

`current-main-duplicate.cpp` links nine retained signed656 GNU libraries;
relevant src/include/test-support trees match exact main06d7 (empty retained
diff), all9 before/after library hashes unchanged. Compile/run0, both classes
sparse11/22/33: error0/return0/count3/IDs/cells/payload unchanged, unlike native
caption update. Raw SHA256
`d7c5db791e34cff2890f0375d2a1e0e4e0980ee06caba3343492072e3a140cc7`.
This is a diagnostic, NOT a pristine whole-main build or requirement source.
Source/log/raw/status/hash retained; executable remains in ignored build.

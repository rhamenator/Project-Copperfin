# Literal-metadata sanitizer execution repair

RQ-CF-PRG-NUMERIC-LITERAL-METADATA-VERIFY-001 derives from the owner's Numeric
compatibility policy, RQ-CF-PRG-NUMERIC-LITERAL-METADATA-001/#5611/#6776 and its
requirement to verify actual instrumented runtime execution. Current main
ff41cfa99aa048014578211022f32979fe09b1ad builds both new literal test targets
in `TARGETS['sanitizer-pr']`, but the separate CTest regex in the same driver's
`main` omits them. Therefore prior hosted sanitizer PASS does not establish
that these tests executed. This is a verification-wiring defect, not a Numeric
payload/metadata algorithm or a new product-policy choice.

Before repair, add an independent AST-based regression that examines actual
`command.extend(['-R', ...])` arguments without executing the driver or any
child process. It must fail for the omitted targets, require all built
sanitizer tests to be selected and check exact literal-test names/anchors.
Freeze original driver/test hashes and actual failure. Then repair only the
PR execution filter, register the read-only portable contract, and add small
always-retained sanitizer execution receipts so exact-head success can be
audited by test name rather than inferred from compilation or wrapper exit.
Do not weaken any existing assertion, sanitizer flag, bound or selection.

VR-5611-NUMERIC-LITERAL-METADATA-VERIFY-001: original fail -> unchanged test
fixed pass; CMake/CTest registration/isolation contracts; actual hosted Clang
ASan/UBSan CTest receipt proving both literal tests ran/passed, exact head/merge
parents, clean review, strict required checks and resolved conversations.
Local selector tests are not instrumented runtime evidence. No complete
Numeric/list compatibility, hazard or release closure is claimed.

DQ-DV-5611-NUMERIC-LITERAL-METADATA-VERIFY-001 procedural delta: withdraw the
prior overbroad execution claim, repair build-versus-run selection, retain
execution receipts, then resume the preserved unary slice. Misuse severity
MEDIUM: accepting compilation as behavior verification risks missed numeric
data-integrity regressions. Walkthrough must show original omission, unchanged
assertions detecting it and named execution records after repair. Developer
self-review/automated evidence is not independent-human or release assurance.
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 status/severity stay
unchanged. Rollback removes this repair coherently and reopens the verification
gap; never restore a claim of execution without evidence. Notify the owner and
correct durable/public records if a claimed result is wrong.

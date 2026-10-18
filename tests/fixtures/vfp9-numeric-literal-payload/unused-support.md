# Unused literal-payload support stage

`RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001`, #7206 under admitted
#5611/#6776/#7189. Native/public [contract](contract.md) signed e2e59d020;
actual ORIGINAL signed f530857ff before this support. [Original verification](original-verification.md)
retains2,432 payload failures with descriptor/evaluation/control0. No original
native expected/header/public assertion was changed to introduce support.

`src/runtime/prg_numeric_literal_payload.h` implements only the recovered
bounded unsigned plain-literal algorithm and is initially **unproduced**:
`prg_engine.cpp`/`prg_engine_expression.inl` do not yet include or call it.
Shape validation uses the existing complete bounded classifier, not its display
caps as computational precision. No canonicalization, digit truncation, epsilon,
per-spelling branch, allocation, unchecked narrowing, shared invariant parser
change or source-grammar extension. All intermediate magnitudes fit binary64
under the256-byte bound; final finite guard remains fail-closed.

Volatile local temporaries deliberately enforce significant binary64 storage
rounding between multiply/add/divide steps and prohibit contraction across them;
they are not concurrency synchronization. The observed native environment is
nearest rounding (control word027F). Unsupported/non-nearest host rounding is
explicitly unspecified, not guessed: helper returns no correction and never
changes caller mode. General native non-default floating-environment recovery
remains an obligation, not full-environment parity. This derives from the
contract's recorded environment and HZ-data-corruption-01 fail-closed constraint.

Independent native header supplies458 unsigned helper inputs (245 first profile
+213 held-out, including calibration overlaps). The public regression still
checks ALL1,193 expression rows, including signed/IIF contexts. GNU -O2/Werror/
`-ffp-contract=fast` compile/run0; Clang -O3/Werror/contract-fast with explicit
ASan/UBSan/float-cast-overflow/no-recovery compile/run0. Each run reports458
selected/failures0; compiler/run stderr empty. Unsupported shape/NUL/non-ASCII/
257-byte/large-input guards and scoped FE_UPWARD unspecified/preserve/restore
controls pass. Exact source/call flags and actual receipts are retained under
host `build/literal-payload-native/unused-helper/`, copied to unused-evidence/.

Register the new pure name in normal CMake, serial scoped-environment isolation
(rounding test restores state), both hosted build/filter paths and exact-name
selector inspection3/3. Registration is not actual hosted execution. The
expanded support/registration freeze is separate from the earlier public freeze:
public source/header/native contract remain byte-identical, while adding this
new test legitimately changes registration files. Freeze expanded inputs before
hooking the producer; preserve them for SAME verification.

Next requires one bounded producer connection and normal configured CMake
SAME/broader verification, actual named hosted execution, exact-head clean
review, strict green checks and resolved conversations before merge. Coordinate
any runtime build with audit/DAW first; pure support is not runtime acceptance.
Keep #6776 modes/checked conversions/allocation limits, exact64/Currency,
unmigrated producers, display and retained division/ListItem work unchanged.

`DQ-DV-7206-DECIMAL-PAYLOAD-CONTRACT-001` remains MEDIUM: development self-review
walkthrough ties original failure -> unused native helper -> explicit rounding
boundaries/unknown-environment guard -> unchanged future public SAME. Not
independent-human/system/release assurance. Rollback before hookup removes only
unproduced support/test registrations while preserving raw/native/ORIGINAL
evidence; after hookup remove that connection coherently as in the contract.
No issue/parent closure or general Numeric parity claim.

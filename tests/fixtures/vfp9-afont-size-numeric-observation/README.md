# Installed VFP9 AFONT Numeric-size observations

Recovered 2026-10-05 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the existing local `vfp9-probe` wrapper without modification.
`afont.prg` is complete clean-room source; `afont.out` retains VERSION,
the selected installed font and all 54 calls. The probe uses local arrays
only; no persistent tables, font installation, system changes or VM were used.

The selected native font was `@Droid Sans Fallback`. This scalable font returns
Logical true for every probed Numeric size. Converted -1 is distinguishable:
the array contains Numeric -1 instead of Logical true. In particular -1.5
and -1.9 select that sentinel, while negative sub-units do not. Positive
4294967295/4294967295.9 and negative -4294967297/-4294967297.9 select the
same sentinel. These observations support truncation and signed-low-32-bit
aliases; do not impose a nonnegative size domain. Most other sizes, including
huge/infinite inputs, have indistinguishable results for this font: their
exact converted values are not independently proven by these output rows.

The shipped Microsoft VFP9 SP2 [AFONT reference](https://www.vfphelp.com/help/_5WN12PPCM.htm)
describes -1 size enumeration, Logical results, scalable fonts and the fourth
size-versus-character-set flag. Return/array/font availability parity is
outside this selected conversion slice, as are second/third argument type
admission and the fourth flag. Existing issues #3252, #6955, #6958 and #6963
retain those boundaries; no duplicate issue or expanded production fix is
needed. Copperfin's current host-aware font-size list is not the native
requirement source and must not be relabeled full VFP parity.

## Selected conversion requirement and remaining verification

Parent `RQ-CF-PRG-NUMERIC-BEHAVIOR-001` and owner-approved #5611/#6776
authorize a bounded checked third-argument conversion at main
`3aa571eaad1dd169b97491c5c3bd260882a2d4a2`. Proposed requirement
`RQ-CF-PRG-AFONT-SIZE-NUMERIC-001` admits finite Numeric values that truncate
into signed int32 in default COPPERFIN, including negative values. Only
explicit VFP9 retains the shared signed-low-32-bit conversion model. That
model's huge/infinite/NaN-to-zero behavior is derived shared-policy continuity,
not an independently distinguished AFONT native result. Exact int64/uint64
must avoid a floating round trip. Preserve other current coercions through
checked half-away rounding without fixing the separate type-admission issues.
Rejection must raise localized catchable error 11 before font enumeration or
array mutation (`HZ-runtime-crash-01` / `HZ-data-corruption-01`).

Native recovery is complete; runtime migration and regression/sanitizer
verification are not yet implemented. Next add exact helper assertions for
the distinguishable -1 boundary, both signed-int32 edges, adjacent doubles,
exact extended integers, NaN/infinities and coercion controls. Add public PRG
rows comparing admitted sizes to canonical inputs, and catchable rejection
rows that preserve array shape/content. Preserve current font-result behavior
instead of claiming the fixed size list reproduces native enumeration.
Run numeric-behavior, neighboring array and string/math suites normally and
under Clang ASan/UBSan/float-cast-overflow, sequentially across build directories
because the numeric suite owns a shared process-local scratch path. Retain
fail-before/pass-after results and docs/32 reverse traceability before a PR.

`VR-5611-AFONT-NATIVE-001`: all 54 expressions complete in the installed probe;
retained source/output confirms fractional -1 and both-direction wrap sentinels.
To reproduce, run a local installed-VFP9 probe wrapper on `afont.prg` from
this directory and compare with `afont.out`, allowing the selected font name
and host/font-specific results to differ. Do not install/change fonts to force
this output. This recipe does not authorize system maintenance or VM changes.

`DQ-5611-AFONT-SCOPE-001` requires separating observed conversion evidence,
derived safety policy, unfinished runtime verification and known result/type/
flag gaps. `DV-5611-AFONT-SCOPE-001`: maintainer-authorized agent self-review
compared the complete source/output, shipped reference, selected dispatch and
the four admitted existing issue scopes. This is not independent human review.
Misuse severity is medium: treating an identical font result as proof of every
converted integer could admit incorrect aliases. Walkthrough: -1.9 and wrapped
-1 aliases are observable; huge-size exact conversion remains derived, and
current Copperfin Logical/array parity remains unclaimed. Rollback of this
preparation is removing its fixture/continuation record; an implementation
correction must retain tests and notify affected users in changelog/release
notes. No production code changes or completion claim occur in this step.

# PRMBAR numeric query requirement and verification

Authority: open owner-authored, `agent-approved` #5611/#6776/#5868/#7096,
revalidated on 2026-10-08 before issue bodies were read. The selected slice is
the second-argument Numeric/exact-integer conversion in the existing PRMBAR
prompt-query callback; GETBAR and native missing-target error publication
remain separate bounded follow-ons. This fixture does not complete #7096.

## Independent installed VFP9 evidence

The unchanged source ran three times, strictly serially, in the installed
Visual FoxPro 9 SP2 `09.00.0000.7423` under Wine using the existing
`/home/rich/bin/vfp9-probe` harness. All three 125-line outputs compare
byte-for-byte equal. Each run queries 40 operands against independently
seeded singleton RELATIVE popups with bar identifiers 1, 2, and 2147483647.
No popup activation, VM boot, installer, disk clone or large allocation occurs.
Each query retains the known integer prompt, count and bar-identity controls.

- Source SHA-256:
  `851a59a1a9b41df70c7b0f0f24a6164e707e553fc44a3211ac0739f564844123`.
- Output SHA-256 (probe.out and each of run1.out/run2.out/run3.out):
  `a60ac1778ec9f0fc65eb2736a0b1ac562615fc809e8e72004e392c4e6056c5cd`.
- Fractional 1.4/1.5/1.9 select bar1; 2.9 selects bar2;
  2147483647.9 selects MAX. Positive4294967297 and negative4294967295
  select bar1; negative4294967294 selects bar2. This query is not the
  setter's positive-wide saturation rule.
- Other selected operands raise catchable1612 in these singleton controls,
  without altering their known prompt/count/identity. This is observed
  missing/nonpositive/converted-absence behavior, not full type/system-menu
  admission evidence or a requirement to reproduce undefined native casts.

The separately retained #7096 Access365 installed-COM probe independently
observes fractional truncation and missing-target1612 for existing RELATIVE
popups. Its source/output hashes are
`8c9089282ddcf81a50f1110840c39e98f009b27127cdbfe1d768c55522d51393` /
`e9bf4bb4f88a093ebc95c0cc7d59b44bd8e45e912562cae3fe6887f03767b722`.
That audit owns its files; this slice only reads them as provenance. The new
wide-operand observations are Wine-native evidence, not a new Windows VM run
or GUI/platform/release qualification.

## Governing requirement, mapped before migration

`RQ-CF-PRG-PRMBAR-NUMERIC-001` derives default finite truncation into1..INT32_MAX,
exact-integer precision and nonfinite containment from owner-admitted parent
`RQ-CF-PRG-NUMERIC-BEHAVIOR-001` (#5611/#6776). Explicit VFP9 Numeric/exact
integers use independently observed both-sign signed-low32 positive aliases,
never the setter's positive-wide saturation. Unsafe/absent targets retain this
bounded slice's existing empty-string fallback; native1612 remains a recorded
#7096 gap, not recovered error parity. Other already-accepted coercions keep
checked signed64 half-away rounding, with oversized values contained.

Shipped Microsoft PRMBAR help, topic `f9cf6b9c-9685-42e0-be3a-c68395b47f4f`,
was read from the installed help extraction. It specifies Character return,
inactive popup support, access-key/disabled-marker stripping and separator
empty strings. Its ordinal/count-bound wording does not explain independently
observed sparse RELATIVE bar identifiers: singleton MAX returns its prompt
despite count1. Native singleton controls govern identifier conversion here;
GETBAR position/order policy and system menus are excluded. Preserve existing
lookup, prefix normalization, selected session, cursor, skip/mark state and
registered callbacks without broadening this slice to type/GUI compatibility.

Architecture: `checked_prmbar_number_argument` delegates the independently
matched checked MRKBAR helper; only PRMBAR's existing selected-session callback
has migrated. Reverse requirement links are in the helper/callback/tests;
the durable matrix row is `defined` for this bounded conversion requirement,
not full PRMBAR/#7096 acceptance or release qualification.

## Verification and development assurance

`VR-5611-PRMBAR-NUMERIC-001/002`: frozen96direct/372guarded/12inactive
normalization cases and three old neighbors. Unchanged original GCC and Clang
callbacks produce exactly the same110 selected identity failure lines (102
guarded plus8 inactive), zero unrelated/direct/state/neighbor failures, totals
4.29s/4.47s respectively and no sanitizer diagnostics. The full original logs,
hashes and configuration are retained in [baseline-audit.md](baseline-audit.md).
Original logs retain their literal trailing space after `got ` for empty
prompts; the fixture-local whitespace attribute preserves these raw bytes.
Fixed GNU13/13 PASS537.34s (PRMBAR4.53s/neighbors0.11s, shared MRKBAR/SKPBAR,
older Numeric/localization and seven repository contracts); Clang21.1.8
ASan/UBSan/float-cast-overflow4/4 PASS13.93s (PRMBAR4.49s/neighbors0.09s/shared
queries), no sanitizer diagnostics/detect_leaks0. Full checksummed logs in the
baseline audit retain actual longer old-Numeric/traceability durations rather
than replacing them or guessing their cause. No helper/assertion/CMake edits
between original and fixed runs; only selected callback migrated. Builds and
runtime tests strictly serial/private TMPDIR; no VM, cloning or installer use.
Broader qualification and exact-head independent review remain hosted PR gates.

`DQ/DV-5611-PRMBAR-NUMERIC-001`: medium bounded documentation-misuse severity.
Procedural delta is safe default conversion versus explicit legacy aliases,
not native missing-target errors or setter saturation. Misreading this as
full menu/error compatibility could select a wrong label/action; relevant
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 controls are checked
conversion, identity/state guards, explicit residual gaps and narrow rollback.
Development implementation-maintainer self-review and automated walkthrough
completed against exact fixed source/helper/test hashes in the audit, under
the owner-directed development process; not independent human review or
high-hazard sign-off. Walkthrough:1.9 selects one;2.9 selects two;MAX.9 selects
cap;0.9 stays empty; positive4294967297/negative4294967295 select one only under
VFP9, and negative4294967294 selects two only under VFP9. Opposite-origin
session2 cases, both disabled-state seeds, mark/cursor/count/registered actions,
inactive access/disabled/separator/internal-marker prompts and explicit cleanup
all pass without query-state mutation. Direct nonfinite/exact64/adjacent-domain
controls pass; old neighbors and shared query helpers remain intact.
General hazards remain active at their registered severity; this bounded
medium-misuse analysis neither downgrades them nor closes a safety issue.
Rollback will revert only this helper/callback/docs/tests; preserve
other Numeric work and publish corrected scope/evidence in the same PR and
release fragment if any documented conversion proves wrong. Bounded local
verification complete; all required hosted checks, exact-head Claude or
owner-authorized Codex review and resolved conversations remain merge gates.
No full native-error/type/GUI/system-menu/leak/platform/release claim.

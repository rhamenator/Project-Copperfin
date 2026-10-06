# Installed VFP9 SET EPOCH contract-recovery gap

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
with the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
epoch.prg is complete clean-room source; epoch.out retains all ten output lines.
Two fresh final processes match byte-for-byte. No table, SQL/network/backend,
VM, installer, proprietary binary inspection or system change was used.
Generated FXP is ignored and retained only as local scratch while this slice
is paused.

## Evidence and selected boundary

The selected slice is only SET EPOCH TO Numeric/exact-integer conversion under
owner-approved #5611/#6776. RQ-CF-PRG-SET-EPOCH-CONTRACT-001 is a recovery gap,
not an implemented EPOCH compatibility requirement. Parent requirements are
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.

The exact retained program observes:

- All three macro-executed forms (SET EPOCH TO 1950, SET EPOCH 1950 and
  omitted SET EPOCH TO) raise catchable error 36.
- The same three EXECSCRIPT forms raise error 10 in the final program.
  This is execution-context evidence, not a universal EPOCH error-number claim.
- SET('EPOCH') raises error 231.
- The valid control SET CENTURY TO 19 ROLLOVER 50 reports 19/50 through
  SET('CENTURY',1|2), and YEAR(CTOD('01/01/49|50')) reports 2049/1950.
  These controls distinguish real century/rollover state; they do not establish
  all CENTURY validation, numeric conversion or parser contracts.

An initial direct-command probe failed compilation with error 36 before
exercising any numeric operand. That discarded attempt supplies no numeric
domain, fraction, alias, omission or consumer matrix. The final dynamic
command fixture above replaces it.

Open owner-authored, agent-approved #3698 already records the nonexistent
VFP SET EPOCH mechanism and the actual SET CENTURY ... ROLLOVER contract.
Its owner-authored 2026-09-15 comment offers removal or an intentional
Copperfin extension mode. Fresh observations support that conflict; no
duplicate issue or implementation admission label was created.

Copperfin main 0c7b209ebbfce42a02d935fffa9e8e8cd7b749d8 currently clamps EPOCH
through evaluate_set_integer_value(1950,1,9999), and date/time parsers consume
that EPOCH state. Those implementation/test values are not allowed requirement
sources. No production code, numeric regression expectation, locale catalog,
CENTURY implementation or parser has changed in this checkpoint.

## Reproduction and identity

From repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-set-epoch-numeric-observation/epoch.prg
sha256sum tests/fixtures/vfp9-set-epoch-numeric-observation/epoch.prg tests/fixtures/vfp9-set-epoch-numeric-observation/epoch.out
```

Source SHA-256: a01e1cc70e6c9e23a8ac5b0d07e5d5e1c556c019c50b225f5f2c56b65b3d2c99.
Output SHA-256: 59e68b19e9b1e8c9690e869a378b046fe68d2ec6a9f23d0ad60a10063500e759.
VR-5611-SET-EPOCH-CONTRACT-001: two final fresh native outputs are identical;
macro/query errors and valid CENTURY/control values are retained above.
Hash checks and git diff checks verify exact source/output retention. No
Copperfin normal/sanitizer execution or full native numeric parity is claimed:
implementation is blocked before selecting a product contract.
All 149 changelog fragments validate and the live-channel integrity check passes.

## Owner decision and continuation

Option A: defer this numeric EPOCH slice to #3698 and explicitly authorize
continuation with another bounded #5611 site. This avoids strengthening a
non-VFP interface, but leaves its existing conversion pending that workstream.

Option B: retain EPOCH as an intentional Copperfin-only extension. Approve its
mode/exposure, valid year range, omitted/default value, fraction conversion and
localized rejection contract first. This preserves deliberate Copperfin use,
but needs a product specification and extension-mode decision.

Removal in the real CENTURY workstream is another #3698 disposition: it aligns
the interface with VFP9 but can break programs using Copperfin's current EPOCH.
No alternative has been silently selected. CENTURY/parser changes would be a
separate admitted slice, not an expansion of this numeric checkpoint.

## Documentation assurance

DQ-5611-SET-EPOCH-CONTRACT-001 requires that failed native recovery is not
presented as a recovered conversion contract, that exact source/output and
owner-dependent alternatives are retained, and that the active slice pauses
before production or test-policy changes.

DV-5611-SET-EPOCH-CONTRACT-001: maintainer-authorized agent self-review on
2026-10-06 compared two final fresh outputs and identities, checked scoped
Copperfin dispatch/consumer references against exact main, and walked through
the macro/query failures plus the valid CENTURY control. This is not
independent-human review. Procedural delta is evidence-only recovery and
decision gating; no production operational procedure changed.

Misuse severity is low for this checkpoint: the principal risk is treating
existing implementation as native evidence. HZ-runtime-crash-01 and
HZ-data-corruption-01 motivate defined conversion and an approved date window;
their mitigations remain pending product selection, not claimed implemented.

Walkthrough: execute the retained fixture, observe error 36 for each macro
EPOCH form and 231 for its query, then observe CENTURY 19/50 and years
2049/1950. Stop before inventing an EPOCH numeric-domain expectation.
Rollback: discard or revert only this evidence checkpoint through version
control, retaining native observations as provenance; no persistent data or
system repair is needed. If guidance proves incorrect, notify maintainers
with exact revision and fixture identities, correct evidence/requirements
together, and repeat the native walkthrough.

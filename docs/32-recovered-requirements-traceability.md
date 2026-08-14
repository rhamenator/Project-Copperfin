# Requirements And Verification Traceability

## Purpose

This document is the durable bidirectional requirements-to-architecture-to-code-
to-verification matrix. It implements the Requirements Recovery Principle in
`docs/01-product-charter.md` without treating Copperfin's existing behavior as
its own requirement source. The historical filename remains stable because
recovered compatibility requirements are one important subset of this matrix.

Allowed requirement evidence is limited to observed behavior from a real,
installed VFP9 environment, shipped Microsoft/VFP documentation, explicit
repository-owner product policy, and registered known-bug or crash exceptions
in `docs/27-known-vfp9-bug-exceptions.md`.
Decompiled or disassembled VFP binaries are prohibited inputs under
`docs/07-clean-room-rules.md`.

Existing Copperfin implementation and tests may confirm or contradict a
requirement, but may not originate one. A derived requirement must identify its
parent product requirement or hazard. When allowed evidence and implementation
disagree, use `gap` or `exception`; never rewrite the requirement to describe
whatever the code happens to do.

## Assurance Boundary

Copperfin's DO-178C-inspired discipline is adapted to a general-purpose
C++/.NET platform. This matrix is not evidence of formal DO-178C compliance,
an assigned software level, certification, or suitability for a particular
safety-critical use. Integrators retain the responsibilities stated in the
product charter.

Every behavior-changing slice must add or cite a governing row and preserve
both directions of traceability. Higher-hazard or broader-reach work must also
record misuse and boundary analysis, rollback, applicable `HZ-*` controls,
focused and broader verification, retained results, and release-evidence
disposition. Unknown relationships remain explicit gaps rather than inferred
coverage.

## Status Values

- `defined`: an explicit owner product requirement or a derived requirement has
  its allowed source/parent plus mapped architecture, code, tests, and retained
  verification evidence.
- `recovered`: the requirement has allowed source evidence and mapped code and
  test evidence.
- `gap`: allowed source evidence exists, but code or test evidence is missing or
  contradicts the requirement. Track implementation in a separate prompt-sized
  issue.
- `exception`: the behavior is intentionally different and has an applied
  `KBX-*` entry in `docs/27-known-vfp9-bug-exceptions.md`.

## Product And Derived Requirements Matrix

| Requirement ID | Parent / allowed source | Testable requirement | Architecture / code | Focused and broader verification | Retained result | Exceptions / hazards | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `RQ-CF-AGENT-001` | Explicit repository-owner product policy; `docs/01-product-charter.md` security-first decision driver | Provider authentication shall not grant local workspace authority. Agent modes shall expose only their exact declared capabilities; unknown modes shall fail closed. Unrestricted local authority shall require explicit feature opt-in, nondefault native permission, trusted product UI, available content-free audit, the exact current warning, and affirmative consent, and shall never elevate privileges. | `docs/64-workspace-agent-access-policy.md`; `include/copperfin/security/workspace_agent_policy.h`; `src/security/workspace_agent_policy.cpp`; `src/security/security_model.cpp`; localization catalogs | `tests/test_workspace_agent_policy.cpp`; broader native policy/platform/localization/isolation selection; Windows, Ubuntu, and macOS protected native execution | `docs/safety/traceability-report-2026-08-12-workspace-agent-access-policy.md` records focused, mutation, sanitizer, review, and protected exact-head evidence | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01`; mutable executor, real sandbox, provider adapter, session revocation, and activation UI remain explicit implementation gaps | `defined` |
| `RQ-CF-AGENT-002` | Derived from `RQ-CF-AGENT-001` and the trusted-host architecture boundary | Before mutable activation exists, the native Studio host shall expose a versioned, read-only policy descriptor whose modes, capabilities, permission/UI/audit gates, warning identity, provider-auth separation, and no-elevation invariant derive from the governing policy. The descriptor shall not activate an agent or manufacture trusted-UI consent, and mixed, duplicate, reordered, or activation-like arguments shall fail without partial output. | `docs/64-workspace-agent-access-policy.md`; `apps/copperfin_studio_host/studio_host_main_workspace_agent_policy.cpp`; shared Studio-host CLI dispatch | `tests/test_studio_host_workspace_agent_policy.cpp`; policy/platform/localization/isolation tests; protected Windows, Ubuntu, and macOS native execution | `docs/safety/traceability-report-2026-08-12-workspace-agent-access-policy.md`; protected exact-head runs `31666059568`, `31666059576`, `31666059579`, `31666059522`, and `31666057944` | `HZ-system-failure-01`; `HZ-data-corruption-01`; descriptor-only boundary leaves activation, session revocation, audit commits, executor, and sandbox unimplemented | `defined` |
| `RQ-CF-AGENT-003` | Derived from `RQ-CF-AGENT-001`, `RQ-CF-AGENT-002`, and the managed-client trust boundary | Before managed activation exists, the shared Studio/VSIX consumer shall invoke only the versioned descriptor JSON grammar and fail closed for missing, unknown, wrongly typed, stale, duplicated, aliased, activation-capable, authority-expanding, provider-authorizing, or privilege-elevating input. It shall expose no activation or execution method. | `docs/64-workspace-agent-access-policy.md`; `vsix/Copperfin.VisualStudio/CopperfinWorkspaceAgentPolicyClient.cs`; `vsix/Copperfin.VisualStudio/CopperfinWorkspaceAgentPolicyModels.cs`; `vsix/Copperfin.VisualStudio/CopperfinStudioHostBridge.cs` | `vsix/Copperfin.DesignerSmokeTests/Program.WorkspaceAgentPolicy.01.cs`; exact-head managed Linux/Mono execution; exact-head Windows native, VSIX, managed, Studio, and Designer execution | `docs/safety/traceability-report-2026-08-12-workspace-agent-access-policy.md`; exact-head Linux run `31677215316` and Windows run `31677215577` with retained artifact digests | `HZ-system-failure-01`; `HZ-data-corruption-01`; read-only consumer leaves activation, audit commits, session revocation, executor, sandbox, provider, and trusted UI unimplemented | `defined` |
| `RQ-CF-AGENT-004` | Derived from `RQ-CF-AGENT-001`, `RQ-CF-AGENT-002`, `RQ-CF-AGENT-003`, and the trusted product-UI boundary | Before mutable activation exists, standalone Studio shall provide a localized, accessible, read-only preview of the validated descriptor. It shall default to advisory, display exactly the admitted modes/capabilities, use the admitted current warning identity to select catalog-owned warning prose, and expose only a Close action. Mode selection shall change display only. Host loading shall run outside the UI thread and suppress duplicate requests while pending. Raw parser, process, host-output, and host-supplied warning text shall remain available to diagnostics but shall not become trusted user-facing prose; actionable messages and warnings shall be selected from fixed localized product text by stable diagnostic code or versioned identity. The preview shall not grant permission, record consent, authenticate a provider, start or stop a session, activate an agent, or execute a tool. | `docs/64-workspace-agent-access-policy.md`; `vsix/Copperfin.Studio/StudioMainForm.cs`; `vsix/Copperfin.Studio/StudioWorkspaceAgentPolicyDialog.cs`; `vsix/Copperfin.VisualStudio/CopperfinLocalization.cs` | `vsix/Copperfin.DesignerSmokeTests/Program.WorkspaceAgentPolicy.02.cs`; warning-free managed compile; exact product/test head `4d76b3277` under Linux Mono/Xvfb and Windows Studio/Designer execution | `docs/safety/traceability-report-2026-08-12-workspace-agent-access-policy.md`; Linux run `31687794634` and Windows run `31687794715` with retained artifact digests | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01`; preview deliberately leaves activation, consent, audit commits, stop/revocation, executor, sandbox, and provider/session UI unimplemented | `defined` |
| `RQ-CF-REL-001` | Explicit repository-owner release-evidence policy; `docs/01-product-charter.md` Requirements Recovery Principle | An exact-candidate validation manifest shall use a versioned schema and the closed release-evidence status vocabulary. It shall distinguish artifact construction/static checks from installer and VSIX lifecycle execution; separate platform-signing, localization-review, and real-installed-VFP9 evidence; and shall report unperformed evidence as `NOT_RUN` rather than `PASS`. | `scripts/assemble-rc-candidate.py`; `docs/contracts/rc-validation-manifest-v2.schema.json`; `.github/workflows/rc-candidate-assembly.yml`; `docs/35-rc1-evaluation-guide.md` | assembler self-test; Draft 2020-12 schema validation; RC workflow contract; focused document/community/safety contracts; protected exact-head execution required | `docs/safety/traceability-report-2026-08-13-rc-evidence-truth.md`; local focused results recorded there; hosted exact-head result pending | `HZ-system-failure-01`; `HZ-doc-command-01`; actual installer/VSIX lifecycle, platform signing, qualified linguistic review, and current installed-VFP9 evidence remain explicitly separate | `gap` |
| `RQ-CF-REL-002` | Explicit repository-owner installer-lifecycle and release-evidence policy; derived from `RQ-CF-REL-001` and `HZ-system-failure-01` | The Windows NSIS producer shall install silently into a unique runner-owned fresh root; verify the installed Studio tree and locale catalogs; run a bounded installed executable smoke; perform a same-version maintenance reinstall without inventory/hash drift or duplicate uninstall registration; silently uninstall; and fail if the installation root or matching uninstall registration remains. Registry inspection shall fail closed on read errors and identify the exact configured CPack uninstall key even if its values were cleared. Evidence shall bind to the exact installer SHA-256. Same-version maintenance shall not be reported as prior-version upgrade. | `.github/workflows/build-installers.yml`; `CMakeLists.txt`; `scripts/test-windows-installer-lifecycle.ps1`; `scripts/assemble-rc-candidate.py`; `docs/contracts/rc-validation-manifest-v3.schema.json` | `tests/run_windows_installer_lifecycle_contract_check.cmake`; assembler self-test; exact-head Windows hosted lifecycle run `31702317708` | `docs/safety/traceability-report-2026-08-13-windows-installer-lifecycle.md`; exact-head `2c38492c1` corrected result bound to installer SHA-256 `f77217c135ee223746f876b672b1a98366b1ba44ff38a94184e58f9fa408dcc6` | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01`; root is unique and installer-owned, child processes are bounded, residue is detected rather than deleted, and previous-version upgrade remains `NOT_RUN` | `defined` |
| `RQ-CF-REL-003` | Explicit repository-owner VSIX-lifecycle and release-evidence policy; derived from `RQ-CF-REL-001` and `HZ-system-failure-01` | The Windows VSIX producer shall select one ephemeral-runner Visual Studio instance with the required editor component; install the exact VSIX into that instance; verify extension identity, version, installed payload, package load, opening a runner-owned PRG and one registered Copperfin command outside the source checkout; uninstall the exact extension; and fail on both matching manifest residue and the exact captured installed-extension directory. The registered-command proof shall identify an actual pane, tab, or window rather than an identically labelled menu item or button. Installer and IDE processes shall be bounded. Same-version reinstall, previous-version upgrade, and disablement shall remain `NOT_RUN` until directly exercised. Evidence shall bind to the exact VSIX SHA-256. | `.github/workflows/build-vsix.yml`; `scripts/test-windows-vsix-lifecycle.ps1`; `scripts/assemble-rc-candidate.py`; `docs/contracts/rc-validation-manifest-v3.schema.json` | helper self-test; `tests/run_windows_vsix_lifecycle_contract_check.cmake`; assembler self-test; exact-head hosted Windows lifecycle run `31770453468` | `docs/safety/traceability-report-2026-08-13-windows-vsix-lifecycle.md`; run `31711406714` is digest-bound but not admissible after review found insufficient PRG-open and successful-load proof; runs `31718662961`, `31719897147`, `31721057446`, `31722176750`, `31723497158`, `31724537954`, `31725371043`, `31726161585`, `31727207629`, `31728514403`, `31729613650`, `31730718257`, `31732427063`, `31733561020`, `31734701283`, `31736024834`, `31738142144`, `31739502436`, `31740807424`, `31744711631`, `31745965381`, `31747101809`, `31748419105`, `31751652834`, `31753021409`, `31754211038`, `31755148469`, `31756018725`, `31757029866`, `31758122162`, `31759758376`, `31761029410`, `31762937993`, `31763800302`, `31764703993`, `31765250426`, and compile-only run `31766391236` are retained negative command-activation/readiness evidence; exact-head run `31770453468` passed the full lifecycle at `26df3a63b` with actual `ControlType.Pane` command-surface proof, exact captured installed-directory absence, and VSIX SHA-256 `39bda85037bb3605c13c11418cb987e0128022ad891e0d8d7a9e7fb5e059e3b9`; runs `31731652063`, `31737231439`, `31742224692`, `31742937078`, `31743866527`, `31749596542`, and `31750359174` are retained bounded installer/prime-process scheduling evidence | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01`; ephemeral runner and exact instance/extension IDs bind mutation scope, the separately bounded installer retains operation diagnostics and re-inventories after failure before cleanup, the bounded registration-prime process must prove exact installed-PkgDef import before the separate evidence IDE and requests a normal close before bounded process-tree termination, bounded PID-bound DTE automation verifies the runner-owned solution and PRG paths and raises the exact product command identity without synthetic input or a test-extension command cache, cleanup constrains external processes while preserving primary and cleanup failures separately, pane/document/package proof remains separately mandatory, no user/project data is used, and unexecuted operations remain explicit | `defined` |

## Recovered Compatibility Requirements Matrix

| LLR ID | Recovered low-level requirement | Allowed source evidence | Code | Tests | Verification | Status | Issue |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `LLR-VFP-NUM-001` | PRG numeric source and calculation text shall use a period as the decimal point independently of the host C/C++ locale or the VFP display-point setting. Comma-decimal text shall not be silently accepted as the same numeric literal. Machine numeric text routed back into parser/runtime contracts shall remain period-decimal and ungrouped. | Shipped VFP9 SP2 Help: [Numeric Data Type](https://vfphelp.com/vfp9/html/f0945c58-08e4-46fc-a15b-e1714a064d91.htm) defines numeric values using digits, an optional sign, and a decimal point. [SET POINT Command](https://www.vfphelp.com/help/html/ab6ea03e-d7f8-4ddb-b2e2-56755efd8857.htm) states that `SET POINT` changes display but calculations must use a period. | `include/copperfin/platform/invariant_numeric.h`; `src/platform/invariant_numeric.cpp`; `src/runtime/prg_engine_expression.inl` (`parse_number`) | `tests/test_prg_engine_parser_classes.cpp` (`test_invariant_numeric_parser_preserves_vfp_decimal_contract`, comma-decimal preprocessor locale test); `tests/test_prg_engine_data_io_save_restore.cpp` (`test_restore_from_parses_numeric_values_invariantly`) | macOS: both CTest targets under `C`, `en_US.UTF-8`, `pt_BR.UTF-8`, and `de_DE.UTF-8` (8/8). Linux seq1419: source/evidence mapping pass plus both targets under default, `C`, and `en_US.utf8` (6/6); `pt_BR`/`de_DE` were not installed and macOS supplies those executions. | `recovered` | closed `#4896` |
| `LLR-VFP-NUM-002` | `SET POINT` shall control the decimal-point character used to display numeric and Currency expressions. Its default shall be period; argument-free `SET POINT TO` shall reset period; and the setting shall be scoped to the current data session. Display punctuation shall not change calculation syntax or invariant machine-readable Currency text. | Mounted shipped VFP9 `dv_foxhelp.chm` (10,870,662 bytes; SHA-256 `abaa86e7623bb00e8bd9323cf2d8e162013598e35d9492557a3ddd1c2cf13e79`), topic `html/ab6ea03e-d7f8-4ddb-b2e2-56755efd8857.htm`, title `SET POINT Command`. The topic states the numeric/Currency display scope, period default and reset, calculation-period boundary, and current-data-session scope. | `src/runtime/prg_engine_dispatch.inl` (`SET POINT`, current-session state, runtime print); `src/runtime/prg_engine_string_functions.cpp` (`format_value_for_display`); `src/runtime/prg_engine_string_function_helpers.inl` (`apply_numeric_picture_symbols`); `src/runtime/prg_engine_helpers.cpp` (`value_as_string` exact Currency text) | `tests/test_prg_engine_string_math_functions.cpp` (direct Currency display, invariant stringification, no-picture `TRANSFORM()`, default/explicit/reset/session state); `tests/test_prg_engine_control_flow_control_flow_basics.cpp` (numeric and Currency runtime print) | Linux: both mapped targets pass under the default locale and explicit `C` (4/4); independent review seq1438 verified the gap and correction. Windows run `31275114419` passed 322/322. Exact head `0af4807ca` macOS run `31281606203` passed the full suite and both targets under `C`, `en_US.UTF-8`, `pt_BR.UTF-8`, and `de_DE.UTF-8` (8/8 focused executions). | `recovered` | `#4897`, `#4913`; implementation `#4914` |
| `LLR-CF-ASSURANCE-001` | During solo-maintainer development, safety-documentation changes classified `none`, `low`, or `medium` shall accept explicit maintainer self-review plus applicable automated verification without claiming independence. Changes classified `high` or `catastrophic` shall require a second qualified human reviewer before closure, evidenced by a structured sign-off comment authored by that reviewer's GitHub account. Completed-project or first-stable-release readiness retains its independent-human-review gate. | Explicit repository-owner policy, 2026-08-14; `docs/DO-178C-ASSURANCE-POLICY.md`; `docs/RELEASE-READINESS-REVIEW.md` | `agents.md`; `.github/ISSUE_TEMPLATE/safety-critical-documentation-change.yml`; `scripts/validate-safety-traceability.ps1`; `docs/safety/triage-rubric.md` | `tests/run_safety_traceability_workflow_contract_check.cmake`; self-review, approved-independent, same-author, unattested, incomplete, negated-result, placeholder-reviewer, and issue-form-heading fixtures under `tests/fixtures/safety_traceability_*review*_issues.json`; legacy independent-review fixtures | Focused safety-traceability workflow contract proves structured low-severity self-review, rejects high-severity self-review, author-as-independent-reviewer, and unattested login claims, accepts approved high-severity review only with a distinct reviewer-authored sign-off carrying qualification and verification evidence, rejects negated legacy results and incomplete/placeholder evidence, preserves GitHub issue-form and authenticated legacy evidence compatibility, and retains mapping and hostile-input validation | `recovered` | repository-owner directive, 2026-08-14 |

## LLR-VFP-NUM-001 Evidence Boundary

The shipped help directly establishes two facts: VFP numeric/calculation syntax
uses a decimal point, and changing the displayed point does not change the
period required in calculations. The host-locale isolation clause is the
portable implementation consequence of those facts: Copperfin must not let the
embedding C/C++ locale reinterpret the VFP token grammar.

This row does **not** require displayed numeric text to ignore `SET POINT`,
`SET SYSFORMATS`, or other VFP formatting state. It also does not claim that all
string-to-number functions share source-literal syntax. Those surfaces require
separate recovered requirements and evidence.

The shared double parser requires full input consumption, rejects leading
whitespace, malformed signs, comma decimals, trailing text, overflow, and
nonfinite values unless a consuming binary-field contract explicitly opts in.
On Apple libc++, where floating-point `std::from_chars` may be unavailable, the
fallback stream is explicitly imbued with `std::locale::classic()` and uses
`std::noskipws`, preserving the same invariant contract.

Independent Linux review at channel sequence 1419 fetched and checked both
shipped-help pages, verified every mapped code and test location, and ran the
actual `test_prg_engine_parser_classes` and `test_prg_engine_data_io` targets.
The latter is the CMake target containing
`test_prg_engine_data_io_save_restore.cpp`; the source shard is not a standalone
test target. The Linux host's missing `pt_BR` and `de_DE` locales are disclosed
rather than inferred as executions. Linux source review confirmed the mapped
comma-decimal parser test constructs its locale facet in process, while the
macOS matrix supplies both installed locale runs.

## LLR-VFP-NUM-002 Evidence Boundary

The requirement text is recovered from the mounted shipped CHM, not from
Copperfin behavior. No binary was decompiled or disassembled. A temporary
current-head probe was used only to compare the recovered requirement with the
implementation: it produced `1.234,5000` for a numeric value but invariant
`1234.5000` for Currency under comma `POINT` and period `SEPARATOR`, proving a
display-path gap before #4914 changed product code.

The correction formats `value_as_string()` output for Currency through the
existing display-symbol seam. That source text is produced directly from the
signed scaled `int64` magnitude and four decimal digits, so display formatting
does not convert through `double`. The same seam serves runtime print and
no-picture `TRANSFORM()`. It deliberately does not add the `SET CURRENCY`
symbol: symbol presence and position remain picture/output-surface behavior,
outside this recovered `SET POINT` requirement.

`value_as_string(Currency)`, calculation parsing, SAVE/RESTORE, manifests,
JSON, and other machine-readable paths remain invariant period-decimal under
`LLR-VFP-NUM-001`. The focused regressions separately prove default period,
explicit punctuation, argument-free reset, per-data-session isolation, exact
four-place Currency digits, and unchanged invariant stringification.

The macOS locale evidence runs the two mapped targets from the shared native
validation action after the full suite. It is guarded by the invariant
`macos` platform input so Linux and Windows behavior is unchanged, while the
platform workflows remain shell-free thin callers. Exact run `31281606203`
checked out `0af4807ca` and passed all eight focused executions.

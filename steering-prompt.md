You're working in Project-Copperfin. A prior assessment confirmed the repo is
directionally aligned with the intended strategy (clean-room, in-situ, VFP9-anchored,
stepwise modernization with real working code in src/vfp, apps/copperfin_build_host,
apps/copperfin_runtime_host) but found one structural gap and a few hygiene issues.
Fix them. Ask me before closing/relabeling any GitHub issue or doing anything
destructive — everything else you can implement directly.

1. Version-targeting decision (docs/01-product-charter.md, docs/03-compatibility-and-migration.md):
   No code or doc currently distinguishes VFP 3-6 from VFP 7-9 as compile-time targets —
   everything is implicitly anchored on VFP 9. Add an explicit statement: v1 ships
   VFP 9 as the sole tested binary-compat target; VFP 6/7/8 is best-effort/untested
   via the shared DBF/CDX readers, not a differentiated target. Do not build a real
   version dial unless I ask for it — just make the current scope honest and explicit.

2. Add a non-goal to docs/01-product-charter.md's "Non-Goals For Version 1" list:
   FoxBASE, FoxPro 1.x, and FoxPro 2.x binary-format support is explicitly out of
   scope. (Code already has zero binary support for these — src/vfp/dbf_header.cpp
   only uses "FoxBASE" as a diagnostic label. This just codifies what's already true.)

3. .NET/polyglot maturity disclosure: docs/01, docs/10-dotnet-interop.md, and
   docs/19-polyglot-and-ai-subprojects.md describe .NET interop and Python/C#
   polyglot support as blanket, version-agnostic capabilities. In reality the
   .NET launcher is a generated C# stub invoked via process spawn
   (src/runtime/runtime_pipeline.cpp), C# "transpilation" writes a file that's
   never executed, and Python has no runtime hook at all. Add a short, honest
   "interop maturity" note to README.md and docs/10/docs/19 stating this plainly,
   and propose (don't implement yet) whether .NET/polyglot work should require a
   user-selected modernization target before being exposed.

4. Triage scope-creep issues #137, #138, #140, #141 (cross-database/Access/VBA
   migration bridge — no FoxPro/VFP connection). List each with a recommendation
   (close as out-of-charter / re-scope / keep with written rationale) and show me
   before taking action.

5. Mechanical cleanup:
   - issues.txt is UTF-16 and unreadable by standard tooling — re-save as UTF-8.
   - remaining-work.md is a single unbounded 1700+ line changelog. Propose a split
     (e.g. by phase/lane) and show me the plan before restructuring it.

6. Report back: what you changed, what you're proposing for my review (items 4 and
   the remaining-work.md split), and confirm none of the must-fix release-blocking
   issues from the prior assessment (#23, #24, #25, #27, #30) were touched by this
   pass — this is a docs/hygiene pass, not a parity-closure slice.

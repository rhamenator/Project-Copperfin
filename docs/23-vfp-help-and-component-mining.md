# VFP Help And Component Mining

Copperfin now treats the installed VFP help set, local extracted source trees, and selective binary/component inspection as a layered parity input stack.

## Source Order

Use sources in this order:

1. [`C:\Program Files (x86)\Microsoft Visual FoxPro 9\dv_foxhelp.chm`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/dv_foxhelp.chm)
2. [`C:\Program Files (x86)\Microsoft Visual FoxPro 9\foxtools.chm`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/foxtools.chm)
3. [`C:\vDosPlus\FPD26\FOXHELP.DBF`](C:/vDosPlus/FPD26/FOXHELP.DBF)
4. [`E:\VFPSource`](E:/VFPSource)
5. Microsoft Learn previous-version references
6. binary/component inspection only when the shipped help and source do not explain the behavior

## Generated Help Artifacts

Refresh the local help indexes with:

```powershell
powershell -ExecutionPolicy Bypass -File E:\Project-Copperfin\scripts\export-vfp-chm-topic-index.ps1
```

Generated outputs:

- [`docs/generated/vfp-chm-index-summary.json`](E:/Project-Copperfin/docs/generated/vfp-chm-index-summary.json)
- [`docs/generated/vfp-chm-command-topics.json`](E:/Project-Copperfin/docs/generated/vfp-chm-command-topics.json)
- [`docs/generated/vfp-foxtools-topics.json`](E:/Project-Copperfin/docs/generated/vfp-foxtools-topics.json)
- [`docs/generated/vfp-chm-topic-manifest.json`](E:/Project-Copperfin/docs/generated/vfp-chm-topic-manifest.json)

Those files provide offline keyword-to-topic mappings for:

- core VFP commands and functions from `dv_foxhelp.chm`
- `foxtools` API-library functions such as `RegFn()`, `RegFn32()`, and `CallFn()`

## High-Value Topics

The next runtime/help-mining targets should stay close to the deepest remaining parity gaps:

- work areas, sessions, indexing, and cursor state
- SQL pass-through and remote cursor behavior
- `ON ERROR`, fault handling, and debugger context
- OLE and automation behavior
- report/listener/report-output behavior
- `foxtools` DLL-call bridge topics

## Phase A Runtime Mapping Updates (2026-05-12)

Recent shipped runtime slices refined the help-mining focus for remaining A3 residuals:

- `#93`: selected SQL/result-cursor `COPY TO ... TYPE JSON FIELDS ...` parity now has focused coverage; keep mining help references around field-projection behavior and selected-cursor state preservation.
- `#94`: `ALTER TABLE ... ROLLBACK` pointer/schema restoration now has focused coverage; keep mining structural-operation rollback semantics (`ALTER TABLE`, `PACK MEMO`, transaction boundaries).
- `#99` and `#101`: `RELEASE ALL` shadow-lifetime and headless `INPUT`/`ACCEPT` local-target behavior now have focused coverage; use help references to validate remaining edge semantics.

Next help-mining target for the active runtime queue:

- `#100` field-transfer residuals, especially `SCATTER/GATHER` field-selection semantics (`FIELDS LIKE` / `FIELDS EXCEPT`) and transfer-path filtering behavior across local/remote cursor surfaces.
- 2026-05-12 note: focused `SCATTER/GATHER FROM array` wildcard/exclusion coverage is now in place (`SCATTER FIELDS LIKE N* TO <array>` and `GATHER FROM <array> FIELDS EXCEPT AGE`); help mining for `#100` should now prioritize any remaining transfer-path differences outside this filtered array path.
- 2026-05-12 note: `#92` support lane now has explicit optimizer contract coverage (`test_prg_engine_index_seek_optimization`), so help/source mining for order/seek residuals should prioritize semantic behavior gaps over planner metadata stability.

Especially high-signal local files and components:

- [`C:\Program Files (x86)\Microsoft Visual FoxPro 9\genmenu.prg`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/genmenu.prg)
- [`C:\Program Files (x86)\Microsoft Visual FoxPro 9\Ffc\_menu.vcx`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/Ffc/_menu.vcx)
- [`C:\Program Files (x86)\Microsoft Visual FoxPro 9\Ffc\_reportlistener.vcx`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/Ffc/_reportlistener.vcx)
- [`C:\Program Files (x86)\Microsoft Visual FoxPro 9\Ffc\_reports.vcx`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/Ffc/_reports.vcx)
- [`C:\Program Files (x86)\Microsoft Visual FoxPro 9\Toolbox\_toolbox.vcx`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/Toolbox/_toolbox.vcx)
- [`E:\VFPSource\DataExplorer`](E:/VFPSource/DataExplorer)
- [`E:\VFPSource\ReportBuilder`](E:/VFPSource/ReportBuilder)
- [`E:\VFPSource\ReportOutput`](E:/VFPSource/ReportOutput)
- [`E:\VFPSource\ReportPreview`](E:/VFPSource/ReportPreview)
- [`E:\VFPSource\foxref`](E:/VFPSource/foxref)
- [`E:\VFPSource\toolbox`](E:/VFPSource/toolbox)
- [`E:\VFPSource\builders`](E:/VFPSource/builders)

## Binary And Component Insight

Binary inspection is still useful, but mainly for boundaries that the help and source trees do not explain.

Current local findings:

- [`C:\Program Files (x86)\Microsoft Visual FoxPro 9\vfp9.exe`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/vfp9.exe) is a 32-bit native Windows GUI host.
- [`C:\Program Files (x86)\Common Files\Microsoft Shared\VFP\vfp9r.dll`](C:/Program%20Files%20(x86)/Common%20Files/Microsoft%20Shared/VFP/vfp9r.dll) and [`C:\Program Files (x86)\Common Files\Microsoft Shared\VFP\vfp9t.dll`](C:/Program%20Files%20(x86)/Common%20Files/Microsoft%20Shared/VFP/vfp9t.dll) expose a small COM/runtime-facing export surface rather than a broad procedural API.
- [`C:\Program Files (x86)\Microsoft Visual FoxPro 9\foxtools.fll`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/foxtools.fll) points to an extension-style dispatch boundary instead of a large native export table.
- many product features are delivered through `.app` components such as [`ReportBuilder.app`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/ReportBuilder.app), [`objectbrowser.app`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/objectbrowser.app), [`taskpane.app`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/taskpane.app), [`builder.app`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/builder.app), and [`wizard.app`](C:/Program%20Files%20(x86)/Microsoft%20Visual%20FoxPro%209/wizard.app)

That means the best reverse-engineering targets are:

- `.app` container/loading behavior
- runtime split and hosting differences between `vfp9r.dll` and `vfp9t.dll`
- `foxtools.fll` dispatch ABI

Everything else should prefer shipped docs/source first.

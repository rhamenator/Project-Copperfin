# VFP Language Reference Coverage

This document anchors Copperfin's command/runtime backlog to the official Visual FoxPro reference instead of memory.

## Primary Sources

- Microsoft Learn language index:
  - <https://learn.microsoft.com/en-us/previous-versions/visualstudio/foxpro/dd5f4hyy(v=vs.71)>
- Microsoft Learn general reference:
  - <https://learn.microsoft.com/en-us/previous-versions/visualstudio/foxpro/h9f898ae(v=vs.71)>
- Microsoft Learn VFP home page:
  - <https://learn.microsoft.com/en-us/previous-versions/visualstudio/foxpro/mt490117(v=msdn.10)>
- Local fallback when Learn coverage is incomplete or too terse:
  - [`C:\vDosPlus\FPD26\FOXHELP.DBF`](C:/vDosPlus/FPD26/FOXHELP.DBF)
  - companion memo file in the same directory

## Generated Inventory

The generated command inventory lives here:

- [`docs/generated/vfp-language-reference-summary.json`](E:/Project-Copperfin/docs/generated/vfp-language-reference-summary.json)
- [`docs/generated/vfp-language-reference-commands.json`](E:/Project-Copperfin/docs/generated/vfp-language-reference-commands.json)
- [`docs/generated/vfp-language-reference-commands.txt`](E:/Project-Copperfin/docs/generated/vfp-language-reference-commands.txt)

Refresh it with:

```powershell
powershell -ExecutionPolicy Bypass -File E:\Project-Copperfin\scripts\export-vfp-reference-index.ps1
```

Current official index snapshot from the Learn language-reference page:

- `429` command entries
- `413` function entries
- `323` property entries
- `83` method entries
- `72` system variable entries
- `69` event entries
- `22` object entries
- `4` preprocessor-directive entries
- `3` operator entries

## Current Copperfin Runtime Surface

The native runtime/parser currently has first-pass support for these command families:

- `DO`, `DO FORM`, `REPORT FORM`, `LABEL FORM`
- `ACTIVATE POPUP`, `ACTIVATE MENU`, `RELEASE POPUP`, `RELEASE MENU`
- `RETURN`
- `IF/ELSE/ENDIF`
- `FOR/ENDFOR`
- `READ EVENTS`, `CLEAR EVENTS`
- `SET ORDER TO TAG`
- `SEEK`
- `GO` / `GOTO`
- `SKIP`
- `SELECT`
- `USE`, `USE IN`, `USE AGAIN`
- `SET DATASESSION TO`
- `SET DEFAULT TO`
- `SET`
- `ON ERROR`
- `PUBLIC`
- `LOCAL`

The expression/function layer currently has first-pass support for these VFP-facing built-ins:

- `SELECT()`, `ALIAS()`, `USED()`, `DBF()`, `FCOUNT()`
- `RECCOUNT()`, `RECNO()`, `FOUND()`, `EOF()`, `BOF()`
- `SQLCONNECT()`, `SQLSTRINGCONNECT()`, `SQLEXEC()`, `SQLDISCONNECT()`
- `CREATEOBJECT()`, `GETOBJECT()`
- utility coverage already used by shipped tests and bootstrap paths, including `AT()`, `RAT()`, `SUBSTR()`, `ALLTRIM()`, `STR()`, `CHR()`, `FILE()`, `SYS()`, `MESSAGE()`, `ERROR()`, `VERSION()`, `ON()`, and `MESSAGEBOX()`

## Immediate Runtime Backlog Derived From The Official Reference

The official command inventory is much larger than the current runtime. The deepest next command groups should stay focused on data/runtime compatibility first:

### Work Areas, Sessions, And Indexed Data

- finish alias/work-area edge cases across `SELECT`, `USE`, and nested data sessions
- deepen order/search behavior beyond the first `SET ORDER TO TAG` / `SEEK` / `FOUND()` pass
- add adjacent data-navigation and data-search commands where VFP developers expect them to work together
- use the command inventory to pull the next search/index/data-session commands in families instead of one at a time

### Local Table Mutation And Query Flow

- add the table-editing and table-browse commands needed for real legacy business applications
- add the command families around filtering, locating, scanning, replacing, appending, deleting, and recalling records
- keep runtime semantics ahead of shell/designer work so the same engine can power both Visual Studio and the standalone IDE

### SQL Pass-Through And Remote Cursor Behavior

- add the broader SQL pass-through surface around the existing `SQLCONNECT` / `SQLEXEC` / `SQLDISCONNECT` slice
- bring remote cursor semantics closer to local cursor expectations where VFP does so
- use the general reference and local help corpus when Learn pages list a command but do not capture enough behavioral detail

### Automation And Compatibility Utilities

- expand the OLE/COM compatibility surface around the current `CREATEOBJECT()` / `GETOBJECT()` slice
- keep host stability and debugger fault containment mandatory for automation failures

## How To Use The References

- Use the Learn language-reference index to decide what command/function families exist and to avoid missing major areas.
- Use the Learn general reference for file structures, table/index storage, and other low-level behavior.
- Use installed VFP 9 help and local source trees for product-era behavior and sample patterns.
- Use `FOXHELP.DBF` and its memo file as a fallback source for older semantics when Learn pages are missing, gated, or too shallow.

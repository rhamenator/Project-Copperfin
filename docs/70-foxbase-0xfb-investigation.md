# FoxBASE `0xFB` Version Byte: Origin Investigation

Research for issue #5528 (parent #1076, #5517; related #5483, #5525, #5526,
#5527). This documents where the widely-repeated "`0xFB` = FoxBASE" claim
actually comes from, so a future session doesn't re-derive this or waste
time chasing a specimen that may never have existed. Per
`docs/07-clean-room-rules.md`, everything cited here is public
documentation, community-maintained reference material, or independently
observed real product behavior.

## Background

`DbfHeader::format_family()` classifies both `0x02` and `0xFB` as
`DbfFormatFamily::foxbase`. `0x02`'s physical layout (8-byte main header,
fixed 521-byte descriptor allocation, 16-byte descriptors) was verified
against a real MIT-licensed fixture during #5483. `0xFB`'s layout was
never independently verified against any real file -- #5526 restricted
`IMPORT DATABASE ... TYPE XBASE` to reject it for exactly that reason.
This investigation traces where the `0xFB` claim itself originates and
whether it can be trusted at all.

## Attempts to obtain a real specimen

- A synthetic file built from an LLM-generated script (Google Gemini) was
  offered as a "real `0xFB` file." Inspection of the script showed it
  simply wrote the standard, already-well-understood 32-byte dBASE
  III-style header/descriptor layout with the version byte set to
  `0xFB` -- an unsourced assumption, not evidence. Using it to "verify"
  the importer would have been circular: the script's assumption and
  Copperfin's pre-existing default-layout fallback happened to agree with
  each other, which proves nothing about what real `0xFB` files (if any
  exist) actually look like.
- Real FoxBASE+ 2.10 (run by the repository owner on real period-accurate
  software, not an emulator reconstruction) was used to `CREATE` a table
  following the same schema this investigation needed to stress a
  non-zero decimal-count field. It wrote version byte `0x03` -- standard
  dBASE III, not `0xFB`. This doesn't verify `0xFB` directly, but it does
  rule out FoxBASE+ 2.10's default `CREATE` path as *a* source of `0xFB`
  files, and independently confirmed the (already known) `0x03` dBASE III
  layout end-to-end, including a live round-trip through
  `import_xbase_table_to_vfp_native()` against genuine external output --
  the first real-world (non-fixture, non-synthetic) validation of that
  importer.

## Tracing the `0xFB` claim back through secondary sources

Multiple "DBF format encyclopedia" sites state `0xFB` = FoxBASE as a bare
fact with no citation:

- `dbf2002.com`'s DBF format page lists `0xFB FoxBASE` with no source and
  no discussion.
- A community C# library (`DbfDataReader`, `DbfHeader.cs`) lists
  `0xfb -> "FoxPro without memo file"` -- a *different* claim than
  `dbf2002.com`'s, suggesting independent (re-)guessing rather than a
  shared authoritative source. Its own maintainers flagged this in
  [issue #35](https://github.com/yellowfeather/DbfDataReader/issues/35),
  which reproduces the entry as `"0xFB FoxBASE (?)"` -- with an explicit
  question mark carried over from wherever they copied it from.
- A community blog post (dev.to, "dBase: Parsing a Binary File Format
  with Raku") cites both `dbf2002.com` and the `DbfDataReader` source as
  its references, calling the GitHub source "the more complete table."

### The root source

`DbfDataReader`'s code comments cite
<https://www.clicketyclick.dk/databases/xbase/format/dbf.html> (Erik
Bachmann's independently compiled xBase format reference -- a detailed,
long-standing site with per-byte bit-mask breakdowns and careful
distinctions between similar products, e.g. Flagship's `.dbv`/`.dbt`
variants, Clipper SIX's SMT memo driver, and VISUAL OBJECTS's NTX-Clipper
variant). Its actual table entry for this value is:

> `FBh (11111011)` -- **`"FoxPro ???"`**

The original compiler of this reference table marked `0xFB` as an
acknowledged unknown -- a value they'd apparently seen referenced
somewhere, without ever determining what it actually meant. As this table
was copied across the web over roughly two decades, that honest
uncertainty marker was silently dropped at each hop: `dbf2002.com` turned
it into a bare "FoxBASE" fact; `DbfDataReader` relabeled it "FoxPro
without memo file" (a plausible-sounding guess, likely pattern-matched
against the neighboring, genuinely-documented `0xF5` = "FoxPro w. memo").

### A fabricated citation, checked and rejected

During this investigation, a different AI assistant (Microsoft
Copilot/365) claimed a real, source-cited specification --
[nikolaygekht/codebase-net-port's `claude/specs/DBF-FORMAT.md`](https://github.com/nikolaygekht/codebase-net-port/blob/main/claude/specs/DBF-FORMAT.md),
part of an AI-assisted .NET port of Sequiter's CodeBase engine --
"explicitly documents `0xFB`" and traces it to CodeBase's S4FOX
Visual-FoxPro-compatible build. This was checked directly against the
actual document rather than trusted:

- The document is genuinely well-sourced: hundreds of specific citations
  into the real CodeBase C source (`D4OPEN.C:2144-2224`,
  `D4CREATE.C:1475`, etc.), released under LGPL v3 by M-P Systems
  Services in 2018.
- Its version-byte table matches Copperfin's own already-verified bytes
  exactly: `0x30`/`0x31` (VFP), `0xF5` (FoxPro 2.x with memo), `0x03`
  (FoxPro 2.x without memo -- CodeBase's own classification of this byte,
  independent corroboration for Copperfin's already-shipped `0x03`
  handling), `0x83` (dBase III+ with memo), `0x8B` (dBase IV with memo).
- **It contains no `0xFB` entry anywhere.** A full search of the document
  found none. Its own documented fallback behavior for unrecognized
  values is explicit: *"Any other value (`0x03`, `0xF5`, `0x83`, ...) --
  treated as a FoxPro 2.5-level file"* -- with no `0xFB` special case in
  that logic either.

Copilot's claim does not match the source it cited. Treated as a
fabricated/hallucinated citation, not evidence.

### Independent corroboration that no real specimen exists

- A long-established, comprehensive xBase format reference
  (`fship.com/dbfspecs.txt`) enumerates many version bytes (`0x03`,
  `0x04`, `0x05`, `0x83`, `0x8B`, `0x8E`, `0xF5`, and others) but has no
  `0xFB` entry.
- Microsoft's own official archived Visual FoxPro documentation
  (<https://learn.microsoft.com/en-us/previous-versions/visualstudio/foxpro/st4a0s68(v=vs.80)>,
  fetched and quoted verbatim, not paraphrased) has no `0xFB` entry
  either. Its nearest value is `FoxPro 2.x (or earlier) with memo: 0xFA`
  -- one hex digit different from `0xFB`, a plausible origin for the
  entire myth as a simple transcription typo somewhere in the secondary
  literature. (Note: this same archived page's other "with memo" values
  -- `0x8A`, `0xCA`, `0xF4` -- also don't match the values Copperfin has
  independently verified against real fixtures, `0x83`/`0x8B`/`0xF5`, so
  this specific archived page evidently carries its own transcription
  errors and should not be treated as more authoritative than empirical
  fixture verification where the two conflict.)
- Multiple independent community discussions over the years (an
  Experts-Exchange thread, an MSDN forum thread) state plainly that
  nobody has ever found or described a real file with the `0xFB`
  signature in practice.

## Decision

No primary source, official documentation, or source-code-derived
specification has ever confirmed `0xFB`'s physical layout -- and the
balance of evidence suggests it may not correspond to any real,
distinct format that was ever actually produced. However:

- Every source that ventured an actual guess at `0xFB`'s meaning (rather
  than reproducing the honest "???") converged on the same lineage as
  the already-verified `0x03` byte: FoxBASE/FoxPro-2.x-without-memo.
- No source, real or hypothetical, has ever contradicted that guess.
- Roughly two decades of active xBase community cataloging has not
  turned up a counterexample.

Repository owner decided (see issue #5528) that this convergent evidence
is sufficient to treat `0xFB` as supported, using the same physical
layout already verified for `0x03`: sequential field packing after the
deletion marker, no trusted on-disk field offset, `dbase_iii`-style memo
conventions (moot in practice, since the FoxBASE family never reports a
memo file). This is implemented in `dbf_read_layout()`
(`src/vfp/dbf_table.cpp`) and `import_xbase_table_to_vfp_native()`
(`src/vfp/dbf_import.cpp`).

This is a **documented best-effort choice**, not a verified fact. If a
genuine `0xFB`-signed file ever surfaces and its real layout turns out to
differ, this decision -- and the `RQ-CF-LEGACY-003`/`RQ-CF-MIGRATION-003`
traceability rows citing it -- will need to be revisited.

## Sources

- <https://www.clicketyclick.dk/databases/xbase/format/dbf.html> (root
  source; the original "FoxPro ???" entry)
- <https://github.com/yellowfeather/DbfDataReader/blob/main/src/DbfDataReader/DbfHeader.cs>
  and <https://github.com/yellowfeather/DbfDataReader/issues/35>
- <https://dev.to/uzluisf/dbase-parsing-a-binary-file-format-with-raku-2fm6>
- <https://www.dbf2002.com/dbf-file-format.html>
- <https://www.fship.com/dbfspecs.txt>
- <https://learn.microsoft.com/en-us/previous-versions/visualstudio/foxpro/st4a0s68(v=vs.80)>
  (official archived Microsoft VFP documentation)
- <https://github.com/nikolaygekht/codebase-net-port/blob/main/claude/specs/DBF-FORMAT.md>
  (genuine, source-cited CodeBase specification; checked and found to
  *not* support the `0xFB` claim attributed to it)

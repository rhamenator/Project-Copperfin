# CDX Index Write Format Notes

Written for issue #5534 (index-rebuild half, the sibling of #5534's
already-shipped relation-inference half, docs/74). Documents the
real-VFP9-verified byte format used by `create_vfp_cdx_single_tag_index_file()`
in `src/vfp/cdx_writer.cpp`.

## Why this needed real VFP9, not just documentation

Every existing CDX-consuming module in this codebase (`index_probe.cpp`,
`cdx_header.cpp`) is explicitly "header-probe-only, no B-tree
materialization" (docs/13). No community reverse-engineering source
(unlike `mdbtools` for the Access/Jet work) documents VFP's CDX B-tree
*leaf-node key-storage format* at the byte level -- this had to be
determined from scratch, against real VFP9 output, per this project's
clean-room discipline.

## Methodology

Using the local `copperfin-vfp9-win11` VM (real VFP9 9.0.00.7423) via a
non-interactive `vfp9.exe -c<config.fpw>` harness (matching the
established pattern from `tests/fixtures/vfp9_descending_observation.prg`,
launched via a Scheduled Task running in the interactive console
session -- a plain SSH-launched process runs in an isolated window
station and cannot be seen or interacted with, which blocked several
initial attempts before this was diagnosed):

1. **Read-side investigation.** Built small (5-record) single-tag
   ascending CDX indexes via real `INDEX ON ... TAG ...`, retrieved the
   resulting `.cdx`/`.dbf` files, and hand-analyzed their bytes against
   known record content (two fixtures: one with keys sharing no prefix,
   one with keys sharing increasing prefixes with their sorted
   predecessor -- to isolate the front-compression scheme).
2. **Write-side verification.** Reimplemented the derived format in a
   standalone Python prototype, used it to hand-construct a *new* CDX
   file (for data VFP9 never itself indexed), placed it next to a
   VFP9-created (but not VFP9-indexed) table, and confirmed **real VFP9
   opens it, resolves `TAGCOUNT()`/`KEY()`, walks records in the correct
   order, and `SEEK()`s correctly** -- the strongest form of verification
   available: real VFP9 accepting a file this project's own tooling
   built from independently-derived understanding, not merely re-reading
   VFP9's own output.

None of the real `.cdx`/`.dbf` fixtures generated during this
investigation are committed to the repository, per this project's
established practice for real-fixture evidence.

## Verified format (single tag, single leaf page, ascending, character key)

### File layout

| Page | Offset | Content |
| --- | --- | --- |
| 0 | 0 | File header |
| 1 | 512 | Unused (all zero) |
| 2 | 1024 | Tag-table page (always `root_node_offset`) |
| 3 | 1536 | Tag statistics page |
| 4 | 2048 | Key-expression page |
| 5 | 2560 | Leaf data page |

Every page is 512 bytes, matching `cdx_header.h`'s existing
`page_size = 512` assumption.

### File header (page 0)

Byte-identical across every single-tag fixture generated in this
investigation, independent of record count or key content:

- `[0:4)` `root_node_offset` = `0x00000400` (1024, page 2) -- LE u32.
- `[4:8)` `next_free_node_offset` = 0.
- `[8:12)` unknown, always 0.
- `[12:14)` `key_length_hint` (matches `cdx_header.cpp`'s own
  `header.raw_words[6]`) = the tag's key length, e.g. 10 for `C(10)`.
- `[14:16)` `key_pool_length_hint` = `0x01E0` (480 = 512 − 32) in every
  fixture; likely a per-page usable-space hint, not independently
  re-derived (copied verbatim -- this project's discipline against
  guessing applies to unverified fields as much as to unverified
  formats).
- `[16:496)` all zero.
- `[496:512)` = `00 00 00 00 00 00 00 00 01 00 01 00 00 00 01 00`,
  constant across every single-tag fixture. Not independently derived;
  copied verbatim. Plausibly tag-count-related (all fixtures had exactly
  one tag) -- unverified for a multi-tag file, explicitly out of scope
  for this slice.

### Tag-table page (page 2)

Byte-identical across every single-tag fixture with the same tag name
and key expression, independent of record count or key content:

```
[0:2)   0x0003
[2:4)   0x0001
[4:8)   0xFFFFFFFF (no left sibling)
[8:12)  0xFFFFFFFF (no right sibling)
[12:14) 0x01DE (478) -- not independently derived, copied verbatim
[14:16) 0xFFFF
[16:20) 0x00000F0F
[20:24) 0x03040410 (as four separate bytes: 10 04 04 03)
[24:28) target_page_offset (LE u32) -- the tag's own header page
        (page 3 in every single-leaf-page fixture: 0x00000600 = 1536)
[28:32) 0x00000030 (48) -- not independently derived, copied verbatim
[32:496) zero
tag name, uppercased, right-aligned to the page end with no fixed slot
        width -- CONFIRMED via two additional real fixtures with
        different name lengths: "IDX1" (4 chars) occupies exactly
        bytes [508:512), "SHORTIX"/"CKEYTAG" (7 chars) occupy exactly
        [505:512). The name occupies `[512 - len(name):512)`.
```

### Tag statistics page (page 3)

```
[0:2)   0x0A00 (10) -- appears identical to key_length_hint
[2:12)  zero
[12:14) key_length (10)
[14:16) 0x0160 (352) -- not independently derived, copied verbatim
[16:20) total record/key count (LE u32) -- confirmed exact:
        matched declared record count (5, and separately 260) in
        every fixture checked
[20:24) same value repeated
[24:28) NOT independently derived. Observed 11 (5 records, zero
        compression), 7 (5 records, with compression), 259 (260
        records, split tree) -- no formula found relating these to
        record count or compression ratio within this session's effort
        budget. The write-side verification test (above) used this
        page copied VERBATIM from an unrelated fixture with the same
        record count and confirmed real VFP9 still opened the file
        and produced correct results -- this field is evidently not
        strictly validated for basic open/seek/traverse operations,
        though its true meaning (and whether it matters for some
        other operation, e.g. appending a new key) is unverified.
[28:32) similarly not independently derived (observed 1, 0, 0)
[32:36) similarly not independently derived (observed 2, 3, 0)
[496:512) present only in the single-leaf-page case (absent/zero once
        the tree spans multiple pages): `00 00 00 00 00 00 00 00 05 00
        01 00 00 00 05 00`. CONFIRMED a true constant for this slice's
        scope: identical across three real fixtures spanning different
        record counts (5, 5, 3) and different key content/compression
        ratios. Two earlier theories were tested and ruled out by this
        same evidence: "tracks the leaf's last entry's own control
        byte" (ruled out -- two fixtures with the same record count but
        different last-entry control bytes, 0x50 vs 0x51, had identical
        footers) and "tracks record_count" (ruled out -- changing record
        count from 5 to 3 left the footer unchanged). Not independently
        explained beyond being a verified constant for this scope.
```

### Key-expression page (page 4)

The tag's key expression, as originally typed (not case-normalized),
starting at byte 0, NUL-padded:

```
[0:N) key expression text, e.g. "cKey" for `INDEX ON cKey TAG cKeyTag`
[N:512) zero
```

### Leaf data page (page 5, single-leaf-page case only)

```
[0:2)   0x0007 -- not independently derived, copied verbatim
[2:4)   num_entries (LE u16)
[4:8)   0xFFFFFFFF (no left sibling leaf)
[8:12)  0xFFFFFFFF (no right sibling leaf)
[12:14) free space remaining in this page (LE u16) -- CONFIRMED EXACT:
        512 - 24 (this header) - 2*num_entries (entry array) -
        (bytes actually used by the compressed key text below).
        Verified against two fixtures with different compression
        ratios (453 and 463 bytes free respectively).
[14:16) 0x00FF -- not independently derived, copied verbatim
[16:20) 0x00000F0F -- constant
[20:24) 0x02040408, i.e. bytes (08 04 04 02) -- constant across every
        single-leaf-page fixture with key_length=10; NOT verified to
        be independent of key_length (only one key_length was tested)
[24: ...) entry array, one 2-byte entry per key, in ASCENDING key
        order:
          byte 0: record number (0-255 only -- see "Known gaps" below)
          byte 1: control byte = (full_key_length << 4) | shared_prefix_length_with_previous_entry
[... : ...] 8 bytes of unexplained footer immediately after the entry
        array in both single-leaf fixtures (offset 34-41 relative to
        page start, i.e. entry_array_end+0 through +7): observed
        "00 00 SS 00 CC 00 00 00" where SS is NOT `num_entries` or any
        other value this session could derive, and CC matches the
        LAST entry's own control byte. Left as zero in the write-side
        verification test; real VFP9 still opened, traversed, and
        `SEEK()`ed the file correctly with this left as zero, meaning
        it is evidently not strictly validated for those operations.
[X:512) compressed key text, ordered in REVERSE entry order (the last
        entry's own trailing/differing suffix nearest the START of
        this block, the first entry's own trailing/differing suffix
        nearest the page END) -- CONFIRMED EXACT, byte-for-byte,
        against two real fixtures with different compression patterns,
        and independently re-derived and confirmed via the write-side
        verification test.
```

### Front-compression algorithm (the key discovery)

For each key, in ascending sort order, compare it to the *previous* key
in that same order (the first key has no previous key, so its shared
prefix length is 0):

```
shared_prefix_length = length of the longest common prefix between
                        this key and the previous key (in sorted order)
trailing_text         = this key's own characters from shared_prefix_length
                        to the end (i.e. the part NOT shared with the
                        previous key)
control_byte           = (this_key's_full_length << 4) | shared_prefix_length
```

The entry array stores `(record_number, control_byte)` pairs in
ascending order at a fixed offset from the leaf page start; the
`trailing_text` strings are concatenated in *reverse* entry order and
written backward from the page end. A reader reconstructs each key by
taking `shared_prefix_length` characters from the *previously
reconstructed* key (walking the entries in order) and appending that
entry's own `trailing_text`.

This was independently verified with two real-VFP9-generated fixtures
(one exercising zero compression, one exercising a range of
shared-prefix lengths from 0 to 4) and confirmed byte-exact by
reconstructing both real fixtures' leaf pages from scratch using this
algorithm.

## A second, critical discovery: the DBF `has_production_index` flag

Real VFP9 refuses to resolve `SET ORDER TO TAG <name> OF <file.cdx>`
against a table whose own DBF header does not have `table_flags`
(`dbf_header.cpp`'s existing `DbfHeader::has_production_index()`, byte
28, bit `0x01`) set -- **even when the CDX file is explicitly named**,
not relying on VFP's automatic same-basename production-index
association. This was discovered when the write-side verification test
initially failed with `"Variable 'CKEYTAG' is not found."` even when
using an *unmodified, already-proven-working* real CDX file against a
different (but otherwise well-formed) table -- isolating the cause to
this one DBF header bit, not anything about the CDX's own bytes.

This codebase's own DBF writers (`dbf_table.cpp`) currently always write
`table_flags = 0`. Any CDX-writing code must also set this bit on the
associated table's DBF header, or the resulting file pair will not be
usable by real VFP9 despite being byte-correct CDX content.

## Known gaps (explicit follow-up work, not attempted in this slice)

- **Multi-page trees (leaf splitting).** A 260-record experiment
  confirmed VFP9 splits a tag's data across multiple leaf pages once one
  page is full, introducing an internal/branch node (page 2's own
  target changes from pointing directly at leaf-shaped content to a
  branch page holding full-width, uncompressed *separator keys* and
  child-page pointers) whose exact format was not fully derived.
- **Record numbers beyond 255.** Real evidence (a leaf page in the
  260-record split fixture) shows entries switch from 2-byte
  `(record_byte, control_byte)` to a wider encoding (observed:
  `(record_lo, record_hi, control_byte)`, i.e. 3 bytes, when a page's
  entries reference record numbers beyond what one byte can hold) --
  confirmed present in real output but the exact trigger condition and
  resulting control-byte position were not independently re-derived or
  write-side-verified. `create_vfp_cdx_single_tag_index_file()` fails
  closed (a structured error) rather than guessing when any record
  number exceeds 255.
- **Multiple tags per CDX**, **descending order**, **`UNIQUE`
  indexes**, **`FOR` clauses**, and **non-character key types**
  (numeric, date, logical) are all unimplemented and fail closed.
- **Individual keys longer than 15 bytes.** The front-compression
  control byte packs a key's own trimmed length into a 4-bit nibble
  (`(length << 4) | shared_prefix_length`), so any single key longer
  than 15 bytes cannot be represented at all -- this was not exercised
  by either real fixture (both used short keys) and was only noticed
  once test-writing considered a realistic `C(20)`-style field.
  `create_vfp_cdx_single_tag_index_file()` fails closed
  (`Vfp.CdxWriter.Error.KeyLengthUnsupported`) rather than silently
  truncating the length nibble into a corrupt leaf page.
- **Page 3's and the leaf footer's exact unresolved fields** (see
  above) -- copied from a same-shaped real fixture rather than
  independently derived; the write-side verification test found this
  sufficient for open/traverse/`SEEK()`, but other operations (e.g.
  `APPEND`ing a new key to an existing index) were not tested and may
  depend on these fields more strictly.

## PR review follow-up (PR #5552)

Real-fixture-verified byte format aside, PR #5552's review round found
several implementation-level robustness gaps in `create_vfp_cdx_single_
tag_index_file()` unrelated to the format itself, all now fixed:

- **DBF flag set before the CDX was known-durable.** The
  `has_production_index` flag was being set before the CDX file was
  written at all, so a write failure left the table falsely claiming a
  structural index it didn't have. The writer now stages the CDX
  content in a temporary sibling file, `rename()`s it over the real
  destination only once every byte is confirmed written, and sets the
  DBF flag only after that succeeds.
- **Non-atomic overwrite of an existing CDX.** Opening the destination
  directly with `std::ios::trunc` destroyed any existing usable CDX
  before the new content was confirmed durable. The same staged
  temp-file-then-rename fix addresses this too.
- **Tag names of 481-512 bytes overwrote the tag-table page's own
  header fields.** The old bound (`<= 512`) didn't account for the
  32 bytes of required header fields at the start of that page; the
  writer now caps tag names at 480 bytes.
- **Oversized key expressions were silently truncated.** An expression
  of 512+ bytes lost everything past the page boundary (and exactly
  512 bytes left no NUL terminator) instead of failing closed.
- **`key_length` was never validated.** A zero or over-one-page value
  produces a file this codebase's own reader can't parse at all
  (`cdx_header.cpp`'s tag-directory scan requires `0 < key_length <=
  page_size`); now rejected up front.
- **Record numbers above 255 could not actually be rejected.**
  `CdxIndexEntry::record_number` was already `std::uint8_t`, so a
  caller converting a record number of 256 before constructing the
  struct got a silently wrapped 0 -- the documented "fails closed above
  255" behavior was unenforceable because the out-of-range information
  was already gone by the time this function saw it. The field is now
  a wider `std::uint32_t`, validated explicitly before narrowing.

## Relationship to #5534's own two-piece framing

This is the index-*rebuild* half #5534's own text anticipated as
"meaningfully bigger" than the relation-inference half (docs/74,
already shipped). Given the real, hard-won scope of even the
narrowest defensible slice (a working single-tag/single-leaf-page/
ascending/character-key writer, real-VFP9-verified), the remaining
gaps above are substantial enough that this document explicitly frames
them as separate follow-up issues rather than an oversight.

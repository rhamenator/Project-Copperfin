# DBF `_NullFlags` Field Format Notes

Governing requirement recovery for issue #6047 (nullable DBF writes silently
lose NULL). Recovered from observed behavior in a real, locally installed
VFP9 9.0.00.7423 instance (Wine prefix `~/copperfin-wine-vfp9`), per
`docs/07-clean-room-rules.md` -- no decompiled binaries were used. This
supersedes guesswork: an earlier draft of this work assumed "a blank
`APPEND BLANK`ed record has every nullable field NULL by default," which
these probes directly disprove (see Finding 3).

## Method

Two headless, non-interactive VFP9 runs (`SCREEN=OFF`, `RESOURCE=OFF`,
`COMMAND=DO <script>.prg` in a `.fpw` config, launched as `vfp9.exe
-c<config.fpw>` under Wine) each `CREATE TABLE`d a small fixture with a mix
of nullable and non-nullable fields, populated a few records with different
NULL patterns via ordinary `REPLACE ... WITH .NULL.`, then read the created
table's own raw bytes back with `FOPEN`/`FREAD`/`ASC` and wrote a decimal
byte dump via `STRTOFILE` for offline analysis. No column of this
investigation depended on any prior claim about the format; every finding
below was read directly from produced bytes.

## Finding 1: field-descriptor flags byte

Each 32-byte field descriptor's byte offset 18 (0x12) is a flags byte.
Confirmed bit `0x02` set for every field declared with a `NULL` clause, and
`0x00` for a field with no `NULL` clause (or an explicit `NOT NULL`).
Offsets 0-10 (name), 11 (type), 12-15 (LE u32 field-data offset), 16
(length), 17 (decimal count) were already documented/implemented
elsewhere in this codebase (`src/vfp/dbf_table.cpp`); offset 18 previously
had no reader or writer anywhere in the codebase and was always emitted as
`0x00`.

## Finding 2: the hidden `_NullFlags` field

When a table has one or more nullable fields, VFP9 appends one additional,
hidden field descriptor as the *last* descriptor, before the `0x0D`
terminator:

- Name: literally `_NullFlags` (11-byte name field, null-padded).
- Type byte: `0x30` (ASCII `'0'`).
- Field-data offset: immediately after the last real field's data (i.e. it
  occupies real record bytes, like any other field).
- Length: `ceil(nullable_field_count / 8)` bytes -- one bit per nullable
  field. Not probed beyond 4 nullable fields (fits one byte in both
  fixtures); a future probe should confirm a >8-nullable-field table
  correctly produces a >1-byte bitmap field before this is asserted as
  proven for that case.
- Decimal-count byte: `0x00`.
- Its own flags byte (offset 18): `0x05` (`0x01 | 0x04`). Bit `0x02`
  (nullable) is *not* set on the bitmap field's own descriptor -- the
  bitmap itself is never NULL. `0x01` and `0x04` were not independently
  decoded by this investigation (plausibly "system column" and "binary
  column" respectively, matching Microsoft's documented DBF field-flag
  bits, but not confirmed here -- do not cite `0x01`/`0x04` individually as
  verified without a further probe isolating each bit).

## Finding 3: bit assignment and per-record semantics

Each `_NullFlags` byte is a plain bitmap, **one bit per nullable field
only** (a non-nullable field consumes no bit and is invisible to this
mechanism entirely), assigned **in field declaration order, LSB-first**:
the first-declared nullable field is bit 0 (value 1) of the first bitmap
byte, the second-declared nullable field is bit 1 (value 2), and so on,
spanning into a second byte only past the eighth nullable field (not
directly observed; inferred from the single-byte-suffices case and VFP's
general bitmap conventions -- flagged as unconfirmed above).

Verified directly:

- A field explicitly set via `REPLACE fld WITH .NULL.` has its bit set to
  1 and its underlying storage bytes blanked (space-filled for
  Character/Numeric/Date; the pre-existing blank-fill convention this
  codebase's writer already uses for other "null token" cases).
  `_NullFlags` byte `0b010` (decimal 2) for three nullable fields A/B/C
  declared in that order meant exactly "B is null, A and C are not."
- A field explicitly set to a real value has its bit cleared to 0 and its
  storage bytes written normally, exactly like a non-nullable field of the
  same type.
- **`APPEND BLANK` alone, with no subsequent `REPLACE` at all, produces a
  `_NullFlags` byte of `0`** -- every nullable field starts as an ordinary
  ("empty"/zero/blank) non-null value, identical to how a non-nullable
  field starts. NULL is never the implicit default for a freshly appended
  record; it only occurs when a field is explicitly assigned `.NULL.`
  (this directly disproves the "blank record = every nullable field NULL"
  assumption raised, but not verified, during this issue's initial
  research).
- The mechanism is uniform across Character, Logical, Date, and Numeric
  (including one with a nonzero decimal count) fields in the same table --
  the bit position and blanking behavior did not vary by field type in
  either probe.

## Implementation guidance

This mechanism is intentionally described independently of any specific
implementation. `src/vfp/dbf_table.cpp`'s writer/reader must reproduce
Findings 1-3 exactly: emit the flags byte and hidden field on create,
compute/apply the correct bit per nullable field (not per field) on
read/write, and never assume a blank record implies NULL. See
`docs/32-recovered-requirements-traceability.md` for the requirement row
covering the storage-layer slice of #6047; the runtime-value-representation
and `VARTYPE`/`EMPTY`/`NVL`/`EVL`/aggregate-semantics slices remain a
separate, not-yet-scheduled follow-up (see that issue's own comments for
the split rationale) and are out of scope for the requirement this
document supports.

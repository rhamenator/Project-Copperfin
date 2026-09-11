# Access `SaveAsText` Design-Text Format Notes

Written for #5477 (parent #138, related #5478). Documents the real-
Access-verified text grammar `parse_access_saveastext_design()`
(`src/vfp/access_saveastext_design.cpp`) parses, following on from
`docs/78-access-forms-reports-vba-storage-reconnaissance.md`'s own
2026-09-11 update, which found that `Application.SaveAsText` -- not raw
Jet/ACE binary parsing -- is the tractable path to Access form/report
structural inspection.

## Methodology

Using the local `copperfin-access365-win11` VM (a clone of the
project's existing Windows VM, built specifically so Microsoft Access
365 could be installed without COM-registration conflicts with anything
else) via SSH + PowerShell COM automation
(`CreateObject("Access.Application")`, `.Visible = $false`,
`OpenCurrentDatabase()`, `SaveAsText()`):

1. Exported several real forms and one real report from a real Jet3
   fixture (`Order Entry1.mdb`, a generic order-entry sample
   application, not personal data) and inspected the resulting text
   byte-for-byte to derive the grammar below.
2. Cross-checked specific uncertain points (string escaping, long-value
   line wrapping) against additional real exports rather than guessing --
   the 2026-09-10 investigation phase's own "doubled double-quote"
   escaping inference (recorded in `docs/78`) turned out to be **wrong**
   once checked against a real fixture with an actually-escaped value;
   this document only records what was directly verified.
3. None of the real `.txt` exports are committed to the repository, nor
   is the source `.mdb` fixture -- matching this project's established
   practice for real-fixture evidence (#5549/#5551's own precedent).
   The committed regression tests (`tests/test_vfp_assets.cpp`) use
   hand-built synthetic text matching this documented grammar instead.

## Grammar

```
Version =<integer>
VersionRequired =<integer>
Checksum =<integer>
Begin <Form|Report>
    <block body>
End
[CodeBehindForm
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
<VBA source, verbatim, to end of file>]
```

- The three header lines are fixed, in this exact order, confirmed
  identical across every fixture checked (a form and a report).
- The root block's type is always exactly `Form` or `Report` in the
  fixtures this slice's own scope covers -- macros, queries, and
  standalone modules have their own different `SaveAsText` shapes (a
  standalone module, `acModule`, is *just* its plain VBA source with no
  `Begin`/`End` structure or header at all -- confirmed via a real
  export -- and is explicitly out of this parser's scope; see #5478 for
  VBA extraction generally).

### Block body

A block's body (`<block body>` above, and recursively within any nested
control) is a sequence of:

- **Property assignments**: `Name =<value>` or, for an inherited-default
  property Access's serializer does not write a concrete value for,
  `Name = NotDefault` (note the surrounding spaces specifically for this
  marker -- confirmed consistent across every fixture checked; a plain
  literal value never has a space before its own `=`).
- **A typed child control**: `Begin <ControlType>` ... `End`, e.g.
  `Begin Label`, `Begin CommandButton`, `Begin Section`, `Begin TextBox`,
  `Begin ComboBox`, `Begin Subform`, `Begin BreakHeader`. This slice does
  not enumerate a fixed set of control-type names -- whatever type name
  follows `Begin` is recorded as-is (real fixtures observed: `Label`,
  `Rectangle`, `Image`, `CommandButton`, `Section`, `TextBox`, `ListBox`,
  `ComboBox`, `CheckBox`, `OptionButton`, `OptionGroup`,
  `BoundObjectFrame`, `UnboundObjectFrame`, `ToggleButton`, `Tab`,
  `FormHeader`, `PageHeader`, `Subform`, `BreakLevel`, `BreakHeader`,
  `Line`).
- **An anonymous "children container"**: a bare `Begin` (no type name)
  ... `End` wrapping a sequence of further typed child-control blocks --
  confirmed via real fixtures to hold nothing else. The parser flattens
  this into the parent control's own `children` list directly rather
  than nesting a synthetic wrapper node, since the wrapper itself
  carries no information.
- **A blob-valued property**: `Name = Begin` ... `End`, where the value
  is one or more raw lines the parser does not interpret (real fixtures
  observed: `GUID` — one line, a `0x`-prefixed hex literal; `NameMap`
  and `RecSrcDt` — multiple comma-continued hex lines). Not decoded in
  this slice: not needed for #5477's own acceptance criteria (control
  hierarchy, control types, basic properties), and, per this project's
  clean-room discipline, not guessed at without a documented format to
  verify against.

A line is unambiguously one of these three kinds by its own shape alone,
independent of indentation (which is present in every real fixture but
cosmetic, not load-bearing): a line whose first token is exactly `Begin`
opens a block (typed or anonymous); a bare `End` closes the innermost
open block; anything else must be a property assignment (`Name=value`)
or the parse fails closed.

### String values, escaping, and line-wrapping

A property value beginning with `"` is a string. Two escape sequences
are confirmed by real fixture evidence:

- `\"` decodes to a literal `"` -- confirmed via a real combo-box
  `BaseInfo`/`ColumnInfo` property value, which is itself Access's own
  semicolon-delimited row-source descriptor and is consequently full of
  embedded, escaped quotes (e.g. a real value decodes to
  `"SELECT ... "; "PrimaryKey"` when unescaped).
- `\\` decodes to a literal `\` -- confirmed via a real `Picture`
  property holding a Windows path
  (`"C:\\Program Files\\...\\ordproc.gif"`).

A long string value can wrap across multiple physical lines with **no
explicit continuation marker** other than structural position: the
first line reads `Name ="<chunk>"` as normal, and each following line
that is *itself* nothing but another quoted chunk (`"<chunk>"`, with no
property name or `=` before it) is a continuation. The parser detects
this structurally (a line starting with a bare `"` rather than an
identifier) rather than assuming a fixed wrap-column width -- a real
fixture's own wrapped `BaseInfo` value splits mid-word at column 80 in
one observed case, but this is not treated as a guaranteed constant.
Continuation chunks are concatenated (still escaped) in order, then the
whole result is unescaped once.

### Code-behind

After the root block's own closing `End`, real fixtures for BOTH a form
and a report show a literal `CodeBehindForm` marker line (used verbatim
for reports too -- Access does not rename it) followed by four
`Attribute VB_...` lines (the class-module attributes VBA gives every
form/report code-behind module) and then the object's own VBA source,
verbatim, to end of file. `parse_access_saveastext_design()` captures
everything from immediately after the `CodeBehindForm` line to EOF as
`AccessDesignParseResult::code_behind`, including the `Attribute` lines,
without interpreting them -- VBA semantics are #5478's own scope, not
this parser's.

A design with no code-behind at all was not observed in a real fixture
during this investigation; the parser treats the complete absence of
anything after the root `End` as valid (empty `code_behind`, not an
error), since nothing in the confirmed grammar requires a
`CodeBehindForm` section to always be present.

## Known gaps (explicit, not attempted in this slice)

- **Blob properties are not decoded** (`GUID`, `NameMap`, `RecSrcDt`
  observed) -- captured as opaque raw lines only. `NameMap` in
  particular is a real, structured binary format (it encodes property-
  to-name-index bindings used elsewhere in the same design text), but
  decoding it is not needed for #5477's acceptance criteria and was not
  investigated further.
- **Non-Form/Report `SaveAsText` shapes are out of scope for this
  parser**: a standalone module (`acModule`) has no `Begin`/`End`
  structure at all (just plain VBA source with no header); macros and
  queries were not exported or inspected during this investigation and
  may have their own distinct shapes.
- **The full real-world control-type vocabulary is not independently
  enumerated** -- this parser accepts any type name following `Begin`
  rather than validating against a fixed list, so an unfamiliar or
  future Access control type would still parse structurally (this is a
  deliberate permissive design choice, not an oversight: rejecting an
  unrecognized-but-well-formed control type would make this parser
  brittle against Access versions/control types this investigation
  did not happen to exercise).
- **Producing the input text itself** (invoking `Application.SaveAsText`
  via COM automation against a real `.mdb`/`.accdb`) is a separate
  concern this module deliberately does not perform -- see `docs/78`'s
  own "Implications for implementation architecture" section for the
  planned split (a small external automation helper produces the text;
  this parser only consumes it).

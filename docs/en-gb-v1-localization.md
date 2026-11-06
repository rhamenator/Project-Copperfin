# en-GB v1 catalog status

The en-GB catalog has all 3,146 en-US source keys and the same named
placeholders. Its 149 changed values use UK spellings for user-facing prose
and display labels (including licence as a noun, colour, centre, catalogue,
behaviour, initialise, analyse, localisation, and authorised). The remaining
2,997 values are currently identical to the en-US source. Many are valid
shared English; their per-entry status is still untriaged rather than presumed
reviewed. The inventory marks en-GB `unreviewed` until the release gate is met.

`docs/localization-reviews/en-GB.json` records source and translation SHA-256,
separate translation and review status, and review provenance for every key.
Claude Sonnet 5.5 reviewed the 149 changed entries in two read-only passes.
It initially held 60 identifier-shaped labels. Source inspection established
that CLI flags and VFP property identifiers are separate literals or fields:
for example, the displayed `fill-colour` label is passed alongside the
unchanged `--fill-color` option, and `VisualAssetEditor.PropertyLabel.FillColor`
is passed alongside the unchanged `FillColor` property name. Claude then
accepted the group with that supplied evidence. The exact hashes, model
sessions, decisions, and the limitation that its second pass used group-level
consumer evidence are retained in
`docs/localization-reviews/2026-11-03-en-gb-claude.json`.

An automated audit confirmed every key, named placeholder, CLI switch,
`*Color` VFP property token, and `PRG`/`VFP`/`COM`/`Copperfin` token in changed
values remained unchanged. Managed catalog lookup rendered British licence,
colour, and analyse strings from the shared catalog; the managed language
service suite and generator parity check passed locally. The normal hosted
native/managed/installer gates and release UI captures are still pending.

The [UK Government Digital Service licensing guide](https://gds-way.digital.cabinet-office.gov.uk/manuals/licensing.html)
distinguishes the noun *licence* from the verb *license*. The
[Transport for London editorial guide](https://tfl.gov.uk/info-for/suppliers-and-contractors/digital-design-toolkit/editorial-style-guide)
uses UK spelling while retaining *program* for computer programs. Microsoft's
[localization overview](https://learn.microsoft.com/en-gb/globalization/localization/localization-overview)
identifies US-to-UK English as localization and calls for in-context review.
These sources support the spelling policy; they do not certify every Copperfin
entry, layout, cultural fit, or user context. No native-speaker or human
cultural certification is claimed.

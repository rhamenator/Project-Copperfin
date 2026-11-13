# Post-v1 locale catalog roadmap (#7075)

This is a versioned **candidate inventory**, not an install manifest or language
selector. Its machine-readable source is
[`post-v1-locale-registry.json`](contracts/post-v1-locale-registry.json). On the
`c650c965b` baseline, Copperfin has four catalog directories: source `en-US`,
unqualified `es-419` and `pt-BR`, and test-only `qps-ploc`. The separate v1
localization work owns the addition and completion of `en-GB` and corrections
to those existing catalogs. Registry status is a baseline snapshot and must
be reconciled with that work before any release inventory consumes it.

## Identity and inheritance

Use canonical BCP 47 language, optional script, and region subtags. Validate
new subtags against the [IANA Language Subtag Registry](https://www.iana.org/assignments/language-subtags-tags-extensions)
and use [CLDR locale guidance](https://www.unicode.org/reports/tr35/) for
likely script and region, layout and locale data. Do not infer that CLDR
fallback is safe for product prose. `zh-Hans` and `zh-Hant`, and `sr-Cyrl`
and `sr-Latn`, remain distinct identities. `es-419` uses the numeric UN M.49
Latin America region. `fil-PH` is the planned Filipino tag. `qps-ploc` is a
test-only internal tag, not an IANA production language.

`preferredParent` records **editorial intent** only. It does not change
runtime fallback. A regional child may inherit a parent's entry only after a
reviewer approves that exact phrase in the child context. Missing or
unqualified parents resolve to the source fallback under the separately
tested runtime policy; the product must not call that a translated child.
For example, `es-MX` may use approved `es-419` phrases, while `es-ES` and
`pt-PT` start without automatic inheritance from Latin American Spanish or
Brazilian Portuguese. `zh-Hant-HK` may inherit reviewed `zh-Hant-TW` entries
but never simplified Chinese. Arabic region variants require individual
terminology review even when they inherit a generic Arabic base. Alias handling
such as OS-provided `zh-CN` to `zh-Hans-CN` must be implemented and tested in
the managed/native resolver before it is advertised; the registry does not
silently rewrite requests.

## Phased delivery

| Phase | Candidate scope | Gate before advertising a pack |
| --- | --- | --- |
| v1 dependency | `en-US`, `es-419`, `pt-BR`, `en-GB`; `qps-ploc` remains test-only | The other localization chat's content, leakage, fallback, CI and packaging work; independent release decision. |
| Western/regional | English, Spanish, Portuguese variants and Western/Northern European languages | Locale-aware selection, reviewed regional inheritance, complete per-key ledger, package and UI evidence. |
| European scripts | Central/Eastern European, Cyrillic, Greek and Baltic candidates | Font coverage, keyboard input, line breaking and native terminology review. Serbian scripts require separate records. |
| CJK | Japanese, Korean, simplified Chinese and two traditional Chinese regions | Shaping/font and glyph fallback, IME/composition, line breaking, accessibility, HiDPI and supported-platform screenshots. |
| RTL | Generic Arabic plus separately reviewed Egyptian, Saudi and Moroccan variants; Hebrew, Persian and Urdu | Bidi layout and isolation in mixed source code/path/number text, keyboard, shaping, mirroring and accessibility. |
| South/Southeast and further | Indic scripts, Thai, Vietnamese, Indonesian, Malay, Filipino, Turkish, Swahili regions and Afrikaans | Script-specific shaping/input/line-break tests and regional wording reviews. |

These are work batches, not promised release dates or a single all-language
release. Additional minority and regional candidates can be added through
the same schema after a reviewer and scope decision. SoundCurrent EQ and
Studio each contain 34 Qt TS locale identities (English plus 33 non-English)
on local reference commits `ab1f6f9204b3779e869b9d9339280f76daf71bd9`
and `1e7771ebf1c6ef39365bb788eb295b35f4d89e35`, respectively. Their catalogs show useful breadth and
separate scripts/Portuguese regions; their documented status still has no
native-speaker review. No Copperfin string is copied from them.

## Measurement and evidence

Each locale needs a ledger conforming to
[`locale-entry-evidence-v1.schema.json`](contracts/locale-entry-evidence-v1.schema.json),
keyed by the exact `en-US` source revision, catalog revision and source-key
hash. An entry records its translation and review status independently.
The schema is a post-v1 planning vocabulary, not the live #7073 ledger
vocabulary; an integration slice must define and test an explicit mapping
before either format is converted or consumed operationally. JSON Schema
cannot enforce uniqueness of one object property across an array, so every
consumer must also enforce the schema's `x-copperfin-uniqueEntryKey` annotation
and reject duplicate `key` values.
The current registry's `required-per-key` value is a **requirement to create
that ledger**, not a claim that one exists. A source-string change invalidates
the matching review until rechecked. Inherited text counts as inherited,
not independently translated. AI and independent reference checks may
suggest corrections, but are advisory and cannot fill the human review field.
Do not send confidential text to an outside model without provider/content
authorization.

Report these numbers separately at each exact source/artifact revision:

1. Source keys and candidate keys; missing, inherited, copied-source and
   nonblank translated entry counts, with denominator and explicit justified
   invariant terms.
2. Placeholder/plural/Unicode/markup/accelerator checks and rendered English
   leakage by surface. A key count alone cannot pass this gate.
3. Human linguistic, terminology, cultural and regional review counts; reviewer
   identity, qualification, date, evidence link and unresolved risk.
4. Runtime/UI accessibility, expansion, fonts, IME, bidi, installer/package,
   native/managed/generated fallback and OS-platform pass counts, bound to
   artifact hashes. The release matrix names any untested platform.

For a pack to become production selectable, its exact revision needs full
required-key coverage, a narrow justified invariant-text list, zero unexplained
English leakage, passing structural and machine-contract checks, qualified
human review of the actual wording, and supported-platform runtime/UI/package
evidence. A reviewer may explicitly accept a documented residual risk; a
missing review is not acceptance. The owner can personally verify English;
other languages need qualified human reviewers. The live v1 localization task
may surface a release-policy conflict if those reviewers are unavailable.

## Integration dependencies and bounded next slices

This registry deliberately does not change current selection or packaging.
Integrate it only after #7071 managed identity/external catalogs, #7072
inventory-driven package QA, #7073 translation debt/per-entry verification,
and #7074 Spanish fallback policy are reconciled with v1. Preserve catalog
trust boundaries (#5737, #5656, #7017), lookup/cache behavior (#6387),
generated C# behavior (#6137), English FoxPro/VFP syntax, diagnostic IDs,
CLI switches, JSON fields and package identifiers. Next PRs should each cover
one family: resolve aliases/selection and test fallback first, add source-bound
entry ledgers, then add regional text and qualification evidence. No locale
becomes selectable merely by adding a registry row.

# v1 locale review status (#7073)

The `es-419` and `pt-BR` catalogs have complete key presence (3,146 entries each)
but remain unreviewed language packs. `docs/localization-reviews/<locale>.json`
records, for every key, the SHA-256 of the en-US source and current translation,
source status, translation status, and independent review status/provenance.
`scripts/check_locale_review_status.py` validates those snapshots in normal
CTest. Its `--release` mode fails while unchanged values are untriaged or any
entry lacks independent review; it is currently expected to fail. A source or
translation edit invalidates the associated recorded evidence until that entry
is explicitly updated. The normal check reports unresolved entries instead of
mistaking populated keys for translated or verified text.

Current status after localizing four documented IntelliSense examples in each
language (2026-11-03):

| Locale | Populated | Changed translation candidates | Unchanged/untriaged | Exact invariant exceptions | Independent AI reviewed | Human reviewed |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| es-419 | 3,146 | 2,925 | 218 | 3 | 4 | 0 |
| pt-BR | 3,146 | 2,923 | 220 | 3 | 4 | 0 |

A changed candidate is **not** a verified translation. Equal source text is
untriaged unless its exact key and source value appear in
`docs/localization-invariant-exceptions.json` with a specific reason. That
list currently covers only VFP application filenames and one Visual FoxPro
field-type label. FoxPro/VFP command tokens, `COM`, and `Copperfin` remain
unchanged inside translated prose as code/product identifiers; whole prose
entries containing those words are not exempt.

Initial glossary for the corrected IntelliSense descriptions:

| Source concept | es-419 candidate | pt-BR candidate | Invariant syntax |
| --- | --- | --- | --- |
| command | comando | comando | `USE IN`, `SET` |
| keyword | palabra clave | palavra-chave | `IN` |
| work area | área de trabajo | área de trabalho | VFP alias names |
| free table | tabla libre | tabela livre | table/cursor identifiers |
| runtime state | estado de ejecución | estado de execução | `SAFETY`, `DELETED`, `EXCLUSIVE`, `TALK` |
| automation object | objeto de automatización | objeto de automação | `COM`, `Copperfin` |

The terminology approach follows Microsoft's [language resources](https://learn.microsoft.com/en-us/globalization/reference/microsoft-language-resources)
and [terminology management guidance](https://learn.microsoft.com/en-us/globalization/localization/managing-terminology),
which distinguish translated terminology from deliberate do-not-translate
terms. Microsoft's [Spanish](https://learn.microsoft.com/es-es/contribute/content/text-formatting-guidelines)
and [Brazilian Portuguese](https://learn.microsoft.com/pt-br/contribute/content/text-formatting-guidelines)
technical formatting guidance treats code keywords as invariant. These sources
support the process and token boundary; they do not certify the wording of
individual Copperfin sentences or regional cultural suitability.

The corrected examples have local managed IntelliSense completion and Quick
Info tests. Claude Sonnet 5.5 independently reviewed all eight corrected
entries in three read-only passes; the third accepted clearer
CreateObject wording after review feedback. Exact source/translation hashes, session provenance,
decisions and residual uncertainty are retained in
`docs/localization-reviews/2026-11-03-intellisense-claude.json`. No native
speaker or qualified human has reviewed either complete
pack. Phrasing, technical terminology, regional variation, tone, formality,
accessibility, layout and screenshots remain open. Independent AI advice may
be recorded per entry with exact provenance, but cannot be described as human
cultural certification. The owner accepted independent AI review with disclosed
limits as the v1.0 linguistic gate on 2026-11-03. Every entry still needs that
review, and no language pack is currently production-ready. The review must
disclose residual uncertainty and must not be represented as native-speaker or
human cultural certification.

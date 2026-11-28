# First-pass European locale drafts

These files preserve in-progress translation and independent AI review work.
They are **not packaged catalogs or release evidence**. Each batch contains 40
source keys in `en-US` catalog order. A `revise` verdict remains unresolved even
when the reviewer suggested replacement wording; only an exact accepted final
value may enter a release ledger.

| Locale | First-pass strings | Independently accepted | Needs revision |
| --- | ---: | ---: | ---: |
| `de-DE` | 1,080 of 3,146 | 1,027 | 53 |
| `fr-FR` | 1,000 of 3,146 | 907 | 93 |
| `it-IT` | 640 of 3,146 | 512 | 128 |
| `es-ES` | 480 of 3,146 | 439 | 41 |
| `pt-PT` | 480 of 3,146 | 434 | 46 |

`de-DE` batches 000–006 and `fr-FR` batches 000–004 were authored with Claude
Sonnet; later batches and the Italian start were authored by separate Codex
instances while Claude was rate limited. Different Codex instances reviewed the
German, French, Italian, Spain Spanish, and Portugal Portuguese drafts.
Run `python3 scripts/check_locale_drafts.py` to check source order, placeholders,
exact review hashes, and protected syntax for accepted entries. Revised entries
may retain reported syntax defects until corrected and reviewed again. Complete
catalog coverage, regional language review, installed package checks, and
rendered UI tests remain open.

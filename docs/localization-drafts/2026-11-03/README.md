# First-pass European locale drafts

These files preserve in-progress translation and independent AI review work.
They are **not packaged catalogs or release evidence**. Each batch contains 40
source keys in `en-US` catalog order. A `revise` verdict remains unresolved even
when the reviewer suggested replacement wording; only an exact accepted final
value may enter a release ledger.

| Locale | First-pass strings | Independently accepted | Needs revision |
| --- | ---: | ---: | ---: |
| `de-DE` | 760 of 3,146 | 728 | 32 |
| `fr-FR` | 680 of 3,146 | 615 | 65 |
| `it-IT` | 320 of 3,146 | 295 | 25 |
| `es-ES` | 160 of 3,146 | 156 | 4 |
| `pt-PT` | 160 of 3,146 | 158 | 2 |

`de-DE` batches 000–006 and `fr-FR` batches 000–004 were authored with Claude
Sonnet; later batches and the Italian start were authored by separate Codex
instances while Claude was rate limited. Different Codex instances reviewed the
German, French, Italian, Spain Spanish, and Portugal Portuguese drafts.
Placeholder and protected syntax checks run on the
drafts; complete catalog coverage, regional language review, installed package
checks, and rendered UI tests remain open.

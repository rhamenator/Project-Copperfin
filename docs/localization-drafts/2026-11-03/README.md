# First-pass European locale drafts

These files preserve in-progress translation and independent AI review work.
They are **not packaged catalogs or release evidence**. Each batch contains 40
source keys in `en-US` catalog order. A `revise` verdict remains unresolved even
when the reviewer suggested replacement wording; only an exact accepted final
value may enter a release ledger.

| Locale | First-pass strings | Independently accepted | Needs revision |
| --- | ---: | ---: | ---: |
| `de-DE` | 440 of 3,146 | 431 | 9 |
| `fr-FR` | 360 of 3,146 | 331 | 29 |
| `it-IT` | 160 of 3,146 | 150 | 10 |

`de-DE` batches 000–006 and `fr-FR` batches 000–004 were authored with Claude
Sonnet; later batches and the Italian start were authored by separate Codex
instances while Claude was rate limited. Different Codex instances reviewed the
German, French, and Italian drafts. Placeholder and protected syntax checks run on the
drafts; complete catalog coverage, regional language review, installed package
checks, and rendered UI tests remain open.

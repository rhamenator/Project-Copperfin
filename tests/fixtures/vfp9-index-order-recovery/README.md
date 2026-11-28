# VFP9 CDX order recovery fixture

This synthetic three-record DBF and its structural CDX were written on the
owner's Windows 11 Pro `copperfin-vfp9-win11` VM through the installed
`VisualFoxPro.Application` COM automation server on 2026-11-27. The capture
program is `capture.prg`; the recorded native results are `vfp9-result.txt`.
No customer data is present.

The table's physical insertion order is `ALPHA`, `CHARLIE`, `BRAVO`. VFP9
created ascending tag `ASCTAG` and descending tag `DESCTAG`. Native VFP9
reported `ORDER() = DESCTAG`, scanned `CHARLIE`, `BRAVO`, `ALPHA` in that
order, and successfully sought `CHARLIE`; selecting `ASCTAG` scanned from
`ALPHA`. This fixture supplies real-format evidence for CDX tag-name parsing
and ordered navigation, not a claim that Copperfin has been replayed against
this exact pair yet.

SHA-256:

| File | SHA-256 |
| --- | --- |
| `indexed_demo.cdx` | `9ca9c39ab716aab76fbd2c5d1af7cd38d37a30fdd65d9df97872a38fb447e946` |
| `indexed_demo.dbf` | `d5b0b494ef81ba25d9e0a19283eab1d54419e574a567b1521599a7e7a0a25f65` |
| `vfp9-result.txt` | `cb63114b9d055d672245ab96fed973161dd148a184cde1ca352be1a35b5f5c67` |

The fixture supports the existing owner-approved index issues [#5618](https://github.com/rhamenator/Project-Copperfin/issues/5618), [#5619](https://github.com/rhamenator/Project-Copperfin/issues/5619), and [#6334](https://github.com/rhamenator/Project-Copperfin/issues/6334). It should be used to add focused regression tests after the observed Copperfin failures are reproduced against the selected source revision.

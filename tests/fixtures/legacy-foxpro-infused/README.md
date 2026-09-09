# FoxBASE/FoxPro compatibility fixtures

These binary DBF/FPT fixtures are unmodified copies from Keith Morrison's
[`infused/dbf`](https://github.com/infused/dbf) repository, commit
`6b6547384439fd009815d20112b22c58eee83503` (2026-08-21). They are licensed
under the MIT License; the upstream license text is retained as `LICENSE`.

They are test data only, not source-code inputs. Copperfin's layout and memo
decoding were independently derived from published DBF/FoxPro file-format
documentation and checked against these artifacts:

- `dbase_02.dbf`: FoxBASE (version `0x02`), no memo file, 8-byte main
  header, fixed 16-byte field descriptors.
- `dbase_f5.dbf` + `dbase_f5.fpt`: FoxPro with memo (version `0xf5`).

SHA-256 checksums at import:

```
aef6c148dc190924b7bf2257f7b162c6dd28b4f7fed200a18342d6a19ed47998  dbase_02.dbf
8774af86ae4cd499237aeb026dc6f00d45cd36c7c29bb41acc3ca02fc7a9140c  dbase_f5.dbf
39c0f129b47f7ee2a1d18703340e226c14bff04bf440b645a88ef0447d167aa2  dbase_f5.fpt
```

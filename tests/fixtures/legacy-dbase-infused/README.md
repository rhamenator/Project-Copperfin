# dBASE compatibility fixtures

These binary DBF/DBT fixtures are unmodified copies from Keith Morrison's
[`infused/dbf`](https://github.com/infused/dbf) repository, commit
`6b6547384439fd009815d20112b22c58eee83503` (2026-08-20). They are licensed
under the MIT License; the upstream license text is retained as `LICENSE`.

They are test data only, not source-code inputs. Copperfin's layout and memo
decoding were independently derived from the public dBASE file-format
documentation and checked against these artifacts:

- `dbase_03.dbf`: dBASE III (version `0x03`), fixed fields.
- `dbase_83.dbf` + `dbase_83.dbt`: dBASE III memo (version `0x83`).
- `dbase_8b.dbf` + `dbase_8b.dbt`: dBASE IV memo (version `0x8b`).
- `dbase_8c.dbf`: dBASE Level 7 memo-flag variant (version `0x8c`).

SHA-256 checksums at import:

```
cb0c742eb0da68f4ab004e5dc30cb0168ad040baf3ea8d4e8e8c9f6b1d730ea4  dbase_03.dbf
022bd66f945205e3636a6d5680a33aa041544ac22301e1ef8a73c6cc88827909  dbase_83.dbf
2265e7a30df4a9919c558b7e4e2e73d3be4c939f4ee875d0ac1bce36670f966e  dbase_83.dbt
3ea7306667131fb2a42fb201e33e3b25a2c0452169c6a890970f348bc65981ee  dbase_8b.dbf
b83882869e4f19c066986b02052e6a21de5e340221d514a4628c718686005bfd  dbase_8b.dbt
38426af0a7b757c400aa8bbb35f74d1fb4947c20e431c6b61d884ea6076a7b0c  dbase_8c.dbf
```

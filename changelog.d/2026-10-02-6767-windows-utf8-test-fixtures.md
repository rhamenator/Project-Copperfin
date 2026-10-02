- 2026-10-02: Three Windows native tests that had failed on every `main` run (#6767) now
  pass, verified on a Windows 11 VM. `test_prg_engine_file_command_operands` and
  `test_vfp_assets` built their UTF-8 file names by joining a narrow `std::string` onto a
  path, which Windows reads in the ANSI code page (`é.tmp` became `Ã©.tmp`, and a 240-byte
  name overflowed MAX_PATH); they now go through `path_from_utf8_string` and
  `path_to_utf8_string`. `test_staged_import_publish` read the staged file with
  `std::ifstream`, which Windows refuses while the staged identity handle (DELETE access,
  FILE_SHARE_READ) is open; the test now reads with `FILE_SHARE_DELETE`. No runtime or
  product code changed, and the replacement-write protection itself was confirmed intact.

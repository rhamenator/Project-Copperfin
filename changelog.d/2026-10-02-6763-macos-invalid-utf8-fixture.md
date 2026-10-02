- 2026-10-02: `test_prg_engine_file_command_operands` no longer fails on macOS for the
  #6583 invalid-UTF-8 wildcard-capture case (#6763). APFS rejects a file name that is not
  valid UTF-8, so the fixture could not be created there and COPY FILE correctly reported a
  missing source. The case now skips when the filesystem refuses the fixture name and still
  runs wherever the name can exist.

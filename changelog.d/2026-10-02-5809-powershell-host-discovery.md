- 2026-10-02: The test build now validates the discovered PowerShell host during
  CMake configuration. On hosted macOS runners `find_program()` returned a
  duplicated, nonexistent app-bundle path, so `test_access_saveastext_export`
  failed at run time and kept the macOS lane red (#5809). A host that does not
  resolve to a regular file is replaced by a real macOS bundle or install binary
  when one exists, and otherwise the PowerShell-dependent tests are omitted with
  a configure-time diagnostic. The code under test and its containment and digest
  checks are unchanged.

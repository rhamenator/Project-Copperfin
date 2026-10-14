# Selection-only manifest path correction

The initial hash command reported:
sha256sum: src/runtime/prg_engine_runtime_surface_itemid_selectors.inl: No such file or directory

It produced only six entries, saved as initial-partial-core-inputs.sha256.
The subsequent checksum0 covered those SIX entries only, not the planned
seven-source inventory. The first hash command's exit status was not separately
recorded; do not turn the final shell success into its success or a complete
seven-input qualification. No test/build/native run was active or interrupted.

Read-only rg discovery and the preceding slice's actual manifest identify
src/runtime/prg_engine_runtime_surface_list_selector_helpers.inl. Correct only
that inventory path and regenerate the complete seven-source manifest/check;
source files themselves remain unchanged against HEAD. This is selection
inventory, not a frozen original phase or completed requirement recovery.

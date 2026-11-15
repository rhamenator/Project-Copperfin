- 2026-11-14: Make the Access SaveAsText design-file parser reject FIFOs
  and symlink-to-FIFO targets through descriptor-bound regular-file admission
  instead of blocking while it waits for a writer (#7347).

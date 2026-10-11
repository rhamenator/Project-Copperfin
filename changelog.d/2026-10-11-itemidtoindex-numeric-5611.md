- 2026-10-11: ItemIDToIndex Numeric/exact item-ID READ conversion is under bounded
  requirements recovery for #5611/#6776. Independent installed VFP9/help
  evidence distinguishes sparse item IDs from row positions, truncation,
  both-sign low32 aliases and huge/Infinity0. Default signed32 rejection,
  exact64 and NaN containment are parent-derived. The operation-specific
  helper stayed unused through both complete frozen GNU/Clang originals;
  bounded method/raw-selector migration now follows that evidence. Fixed GNU
  34/34 and Clang sanitizer 34/34 pass; both pass six unchanged preservation
  groups. Bounded development documentation walkthrough is complete;
  five documentation contracts pass. PR #7150 merged into main as c2073149c
  after clean exact-head authorized Codex fallback review, all 11 required
  checks passing, and no unresolved review conversations. Exact-head signed
  post-push verification passes 34/34; Linux/macOS native validation passes
  544/544. Optional Windows native validation completed successfully, 545/545,
  on the same exact head after merge; it was pending at merge, not retroactively
  green. Its full raw/watch evidence is retained recoverably with the build.
  Full evidence is retained; the completed
  worktree/branches are removed and its build is recoverable in Trash.
  Unsupported type gap #7149,
  writes/ID creation/lifecycle and generic parser remain separate.

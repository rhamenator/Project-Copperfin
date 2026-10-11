- 2026-10-11: ItemIDToIndex Numeric/exact item-ID READ conversion is under bounded
  requirements recovery for #5611/#6776. Independent installed VFP9/help
  evidence distinguishes sparse item IDs from row positions, truncation,
  both-sign low32 aliases and huge/Infinity0. Default signed32 rejection,
  exact64 and NaN containment are parent-derived. The operation-specific
  helper stayed unused through both complete frozen GNU/Clang originals;
  bounded method/raw-selector migration now follows that evidence. Fixed GNU
  34/34 and Clang sanitizer 34/34 pass; both pass six unchanged preservation
  groups. Bounded development documentation walkthrough is complete;
  five documentation contracts pass; exact-head external acceptance remains pending.
  Unsupported type gap #7149,
  writes/ID creation/lifecycle and generic parser remain separate.

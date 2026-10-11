- 2026-10-11: Check native ComboBox/ListBox IndexToItemID Numeric/exact
  row-index read requirements under #5611/#6776 from independent installed
  VFP9 and shipped help. Distinguish sparse item identity from row position,
  signed missing-row zero from rejected conversions, and explicit VFP9's
  both-sign low32 aliases from default checked signed32/exact64 policy.
  Wire bounded method and raw property-first reads after both full original
  runs, preserving frozen inputs and phase evidence. GNU29/29, Clang sanitizer
  29/29 and six unchanged preservation groups per compiler pass; bounded
  development documentation walkthrough/self-review and five documentation
  contracts pass. PR #7148 merged after all 11 required checks passed, a clean
  exact-head authorized Codex review followed Claude-first routing, and all
  conversations were resolved. Optional broad native CI was still running at
  merge; all three late native jobs subsequently passed at the exact head
  (Linux/macOS 539 tests each, Windows 540), with full raw evidence retained.
  Unsupported
  type admission, writes, lifecycle and parent completion remain separate.

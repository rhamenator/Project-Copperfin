# ListItem supplied-column modeled preservation plan

RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 /
VR-5611-LISTITEM-COLUMN-NUMERIC-002.

This is a NON-NATIVE before/after preservation gate, never a compatibility
oracle. The baseline product revision is the retained signed pre-wire commit
`1558300d4f113153b0406ba7191ba259b08ccbee`; the fixed revision must be the
exact candidate head. One frozen harness is compiled independently against
each revision's same-compiler runtime libraries. Acceptance requires complete
capture framing and whole-byte equality for GNU-to-GNU and Clang-to-Clang.
Cross-compiler equality is diagnostic only.

The harness must cover both ListBox and ComboBox, COPPERFIN and VFP9 modes, and
two isolated data sessions. It captures return/error plus row IDs, primary and
secondary cells, payload, selection/focus/top/display/value state, binding and
NumericBehavior/session context for every call. The excluded-scope cases are:

- one-argument implicit-column reads and three-argument/unsupported arities;
- unsupported second selectors: NULL, Character, Logical, Currency, Date and
  DateTime;
- first-selector conversions that precede the new second-column adapter:
  fractional, zero, negative, missing, Character, Logical, NULL and exact64;
- bound/non-default configurations retained outside the admitted boundary:
  RowSourceType 2 and 5, non-empty ControlSource and non-exact-zero RST values;
- missing positive columns, ordinary integral columns and the existing
  first-ID/implicit-column controls needed to detect accidental route changes.

The new behavior's safe-public92 and hostile56 tables are excluded because
their native/derived expectations already have separate frozen gates. The
preservation capture may not add requirements, repair excluded routes, weaken
the original/fixed106 evidence, or treat current output as parity. Both source
trees, the frozen harness, compile commands, linked library inventories,
statuses, stderr, complete raw outputs and before/after hashes must be retained.
Only after both SAME-compiler comparisons and independent framing audits return
zero may development contracts and broader qualification proceed.

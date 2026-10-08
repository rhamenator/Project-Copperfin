# DEFINE BAR numeric identifiers (#5611/#6776)

RQ-CF-PRG-DEFINE-BAR-NUMERIC-001 derives from admitted
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and installed VFP9 09.00.0000.7423.
Scope: the existing parsed Numeric-literal identifier of define_bar_command,
not broader expression/type admission, popup metadata/ordinal filling,
activation, query, selection, skip or mark conversion.

Requirements mapped BEFORE production conversion changes:

- COPPERFIN: truncate finite Numeric identifiers toward zero; admit only
  converted 1..INT32_MAX; reject every other value with localized catchable167
  before evaluating PROMPT or inserting/replacing a bar.
- VFP9: finite inputs at or above 2^32 saturate to INT32_MAX, NOT low32 small
  aliases. Below 2^32 use defined legacy low-signed32 conversion, then reject
  results below1. Ordinary fractions therefore truncate; negative finite
  -4294967295/-4294967294 alias1/2 only in VFP9 mode. Positive 2^31 and
  2^32-1 reject167; positive 2^32/2^32+1/1E20/1E300 replace INT32_MAX.
- Reject NaN and either infinity in BOTH modes (owner-derived safety boundary,
  not a native nonfinite claim); no unsafe floating/integral cast or llround.
- Preserve mode/session/popup/prompt state on rejection, current sparse-map
  allocation semantics, ordinary prompt evaluation and same-key replacement.
  No new ordinal allocation or arbitrary host bar-count cap is introduced.

Allowed native source: probe.prg/probe.out, fresh RELATIVE popup seeded with
bars1/2/INT32_MAX for each operand, so every attempt holds at most FOUR bars.
No activation/UI interaction, huge native ordinal allocation or backend use.
Count, small prompts and the seeded maximum prompt independently distinguish
small alias, large replacement and invalid no-mutation; GETBAR is an additional
identifier observation, NOT the sole identity oracle. Complete final transcript
is 67 lines/32 operands including four native syntax/type errors10. Those
non-Numeric inputs remain outside this conversion-only slice.

Native investigation caution: two original uninstrumented non-RELATIVE attempts
timed out25s with empty buffered output, not complete evidence. The third
instrumented nonrelative-attempt.prg.txt stopped at BEGIN|32768; retained partial
output proves no completion/cleanup and is NOT a passing/native upper-limit
claim. In that private process, ordinal32767 materialized32767 bars. The
unchanged wrapper's existing SLEEP zero control then completed successfully.
Subsequent final recovery uses RELATIVE and bounded sparse state; never repeat
the incomplete attempt (retained as non-runnable .prg.txt source). Its source SHA256
c8b0c637394d6c8369a1ae00f51cbc9b1e7850bf685f1e6b33a977aa80e3aba7;
original partial byte hash b3fadbc636a5202f7306090de37d2dab3a1a9ef77f9c500febc5ea8ef02d4e70;
the retained normalized line transcription has SHA256
3f9f185b2725462dd78cff53b37bf3f3b9e071da1bb8f3f5a61565b9c017f7c1.
This partial transcription is not claimed byte-identical native evidence.
No VM/template/system/ODBC/environment maintenance was performed.

Final source eeefd67a0a61f3994d2ef328b1bf1bb1eb0758e12dbf656a29be77a6680710e7;
output 6f9ac3b0bffc7078c6411653a92902f4a3f7f3881d529dbcf5e66c924b8d23a6.
Three final unchanged serial runs completed with matching source/output hashes.
The separately completed bounded nonrelative-safe fixture observes counts20
and3 for ordinal versus RELATIVE bars1/2/20, with identical prompts and cleanup.
That existing popup-metadata/count gap remains outside this slice under #6152;
#6225 retains broader expression admission. #5868 covers other bar commands
too and remains unfinished; its expectation to reject ordinary fractions is
contradicted by these installed observations and is not used as an oracle.
VR-5611-DEFINE-BAR-NUMERIC-001: 68 independent helper checks, 56 fresh PRG
both-mode guarded operands, eight four-locale metadata cases and four added
inline-PROMPT session/captured-mode cases. Invalid admission precedes PROMPT.
VR-5611-DEFINE-BAR-NUMERIC-002: original-dispatch negative, focused fixed
GCC/Clang ASan/UBSan/float-cast-overflow, old popup/list/control neighbors,
older Numeric/localization and repository contracts. Final Clang focused2/2
passes38.08s (numeric37.62s/neighbors0.44s), ASan/UBSan/float-cast-overflow,
detect_leaks0/abort_on_error1/halt_on_error1, no diagnostics. Earlier fixed
GCC4/4 passes101.82s (localization14.40s/older Numeric86.24s/numeric0.84s/
neighbors0.10s); that run preceded the four additional session cases and is
not claimed as their verification. Final GCC two focused tests plus seven
repository contracts pass9/9,369.91s (numeric0.80s/neighbors0.10s/safety357.32s):
locale installation, agent intake, DCO, changelog assembler/178 valid fragments,
native isolation and safety traceability. This final run includes all four
additional session cases; the requirement is defined with these retained results.

HZ-runtime-crash-01: checked numeric conversion before any host cast.
HZ-data-corruption-01: wide IDs cannot accidentally replace small bars by
compiler-dependent wrapping; invalid input cannot evaluate a mutating prompt.
HZ-doc-command-01: explicit modes/units/bounds/context and non-RELATIVE warning.
Misuse severity medium. DQ-5611-DEFINE-BAR-NUMERIC-001 requires these boundaries;
DV-5611-DEFINE-BAR-NUMERIC-001 requires development maintainer self-review/
walkthrough and automated results, not independent human review.
Procedural delta: checked admission replaces one llround-to-map-key site.
Rollback: revert this isolated helper/dispatch/test/docs delta; field notice
describes truncation/error167/default range and opt-in legacy alias/saturation.
No independent-human, full native type/GUI, leak, platform or release assurance.

Completed development maintainer self-review/walkthrough under owner
authorization (NOT independent human evidence): checked the isolated helper,
single dispatch conversion, localized167 and unchanged parser/menu boundaries
against the retained native controls and admitted parent policy. The executed
both-mode walkthrough verifies 0.5 and2147483648 reject167 without PROMPT or
map mutation;1.9 replaces bar1 once; default wide4294967297 rejects while
VFP9 replaces the seeded maximum, never bar1; negative4294967295 aliases1
only in VFP9. Sparse counts and seeded small/MAX prompts establish identity
independently of GETBAR. Four additional cases verify admission captures the
origin's mode before inline PROMPT changes to session2 with the opposite mode;
callback-selected session2 remains selected, its popup receives the captured
identifier and the origin popup remains intact. Invalid default wide input
never enters the callback. Four-locale ERROR/AERROR/message and guard checks,
release/reset and the two unchanged menu/list neighbors passed the focused
sanitizer. Reviewed the non-RELATIVE warning, incomplete transcription limits,
bounded completed native controls and rollback/field-notice wording. These
are development containment checks, not independent or full type/GUI/native
ordinal behavior, resource-limit, all-platform or release qualification.

Fixed dispatcher SHA256:
24fd72a7e528f76c42817ace86c5baacc8672efe41010badb5a078d38d38c60b;
helper11321c87539d849f25e28461bbd8a8b8fb2fa60848976d9fc60f1b8cc60fb424;
final test e40b6ebbd2fe4cfeffcd6610cc3dc95240cbfb614289b247d448af6ff60799a7.
Final Clang CTest log SHA256:
2bc529ff0e608dde81610f5879e68cc5687b895fe63863ce9fe83243ca3156aa.
Earlier fixed GCC CTest log SHA256:
e46042a8c3d502348b6919b532ff56c0a92dcc76c2738f6bfba9d47cb6a509ba.
Final GCC focused/contracts CTest log SHA256:
cf2ae676910891c0c815bcaf5207d51013a4890ec14e27a08416438692a2b137.
See baseline-audit.txt for original-dispatch150 selected failures per compiler,
source/log identities and reconstruction without the four later-added cases.

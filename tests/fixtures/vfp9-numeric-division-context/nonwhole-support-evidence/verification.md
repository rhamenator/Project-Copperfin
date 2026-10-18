# Unused non-whole helper checks

Requirement RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001; frozen tests/oracle
2165024f7941a39bbee4a223fe76cc422af258ce BEFORE unused support
dda3f633cf38b4169139ba0fedd709369d027982. No runtime caller changed.

Both audit and DAW cleared the narrow serialized window. Initial high I/O
deferred launch; after decay, both actual compiler/run pairs sampled full
avg10=0.00% before/after (60s retains the historical spike). Window released
immediately. No native/Wine/VM, runtime archive rebuild or broader build here.

Actual commands, from the active worktree (output paths under
`build/nonwhole-support/`):

```
nice -n 10 c++ -std=c++20 -O0 -g0 -Wall -Wextra -Wpedantic -Werror -Iinclude tests/test_prg_numeric_nonwhole_division_helper.cpp -o build/nonwhole-support/gnu-helper
nice -n 10 build/nonwhole-support/gnu-helper
nice -n 10 clang++ -std=c++20 -O1 -g0 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all -fno-omit-frame-pointer -Iinclude tests/test_prg_numeric_nonwhole_division_helper.cpp -o build/nonwhole-support/clang-helper
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 nice -n 10 build/nonwhole-support/clang-helper
```

GNU15.2 and Clang21.1.8 actual compile0/run0, all stderr empty. Both stdout
report68 complete native roots/314 syntax nodes, all640 older native rows and
bounded-domain controls passed. No tests skipped, no tolerance/metadata-only
check. This is two explicit pure-unit executions, not CTest/hosted execution or
full Numeric parity. Native-only direct header reconstruction0; four actual
sanitizer-selection contract tests0. Hash manifest binds support, direct tests,
native headers and unchanged expanded public sources. Raw statuses, logs,
timestamps and pressure samples retained here. Executables stay host-side.

LOW DQ/DV recovery delta and parent hazards remain as in the timing contract.
Walkthrough: native-only roots -> signed test freeze -> unused support -> exact
direct results/older inventory -> separate bounded hook/SAME. Development
self-review/automation is not independent-human or release qualification.
The expanded ORIGINAL remains metadata-only fail; unchanged SAME, broader,
actual selected hosted tests, clean exact-head review, strict checks and resolved
conversations are still required. #7208 remains unfixed and outside this slice.

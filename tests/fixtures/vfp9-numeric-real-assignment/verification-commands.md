# Reproduction commands and local instrumentation

Run from the slice worktree. Evidence logs/statuses under evidence/ correspond
to these commands; standalone outputs are rebuildable, not retained binaries.
No VM/native installation mutation and no untrusted downloaded code executed.

```
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -Iinclude tests/test_prg_numeric_real_assignment_helper.cpp -o build/numeric-real-assignment-native/helper-gnu
build/numeric-real-assignment-native/helper-gnu
clang++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all -Iinclude tests/test_prg_numeric_real_assignment_helper.cpp -o build/numeric-real-assignment-native/helper-clang
build/numeric-real-assignment-native/helper-clang
clang++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsyntax-only -Iinclude tests/test_prg_engine_numeric_real_assignment.cpp
cmake -S . -B build/numeric-real-assignment-focused -DCMAKE_BUILD_TYPE=Debug '-DCMAKE_CXX_FLAGS_DEBUG=-O0 -g0' -DCOPPERFIN_BUILD_TESTS=ON
node tests/fixtures/vfp9-numeric-real-assignment/audit-real-assignment.cjs tests/fixtures/vfp9-numeric-real-assignment/captures
node tests/fixtures/vfp9-numeric-real-assignment/audit-expectations.cjs
node tests/fixtures/vfp9-numeric-real-assignment/audit-original.cjs tests/fixtures/vfp9-numeric-real-assignment/evidence
```

Build the exact test names enumerated in final-ctest.log before CTest. The
initial premature run did not wait for that build and remains a failed
attempt, not a product failure or acceptance result. Retained compiler
versions and actual CMake logs identify the GNU runtime build. Explicit
sanitizer flags above concern the standalone Clang helper only; they are
not a claim that the GNU runtime build or any hosted lane used those flags.
Native probe runner provenance and byte-identical paired captures are
recorded separately in captures/. Raw active-field audit never uses the
production helper. No assertions/timeouts were changed between attempts.

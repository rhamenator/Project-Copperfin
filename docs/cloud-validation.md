# Cloud validation guide

Project-Copperfin is public. Use standard GitHub-hosted runners for broad or
expensive automated evidence. Keep local work to editing, navigation, small
reproductions, and focused compile/test loops.

## Existing coverage and added lanes

The native Linux GCC, Windows MSVC, and macOS Clang workflows already run the
full CTest suite. Other existing workflows cover hosted Windows deep tests,
managed UI, VSIX, generated launchers, installers, security, traceability,
and release assembly. They remain the platform and package qualification
routes. The `Cloud Defect Hunt` workflow adds independent Linux jobs:

| Lane | Pull request | Nightly | Evidence |
| --- | --- | --- | --- |
| `sanitizer` | Selected runtime, package, DBF, and migration tests under Clang ASan/UBSan | Full native CTest inventory under Clang ASan/UBSan | Memory and undefined behavior diagnostics |
| `fuzz` | 30-second libFuzzer campaign against the DBF header parser | 600-second campaign | Seed, libFuzzer log, crash input on failure |
| `stress` | Two repetitions of existing task, database lifecycle, buffering, and data-I/O sequences | Twelve repetitions | CTest failure sequence; this repeats fixed state sequences, not randomized scheduling |
| `migration` | Seeded 1,000-row dBASE III to VFP-native round trip plus DBF/staged-import tests | Seeded 20,000-row round trip plus those tests | Seed, row count, file sizes, peak child RSS |

The fuzz lane presently has one real coverage-guided target: DBF header
parsing. Its seed corpus is copied to a writable artifact directory before
each campaign. Passing corpus additions are deleted. A crash preserves the
small reproducer and metadata for 14 days. No PRG, JSON, SQL, CDX, FPT, or
package fuzz coverage is claimed. The migration stress lane tests one-table
C/N field fidelity at scale; it does not claim NULL/deleted fidelity or
streaming-memory bounds. The peak RSS observation is a diagnostic, not a
memory budget gate.

The full native suites already compile with `-Wall -Wextra -Wpedantic` or
MSVC `/W4 /permissive-`. This change adds no clang-tidy, C# analyzer, or
PowerShell analyzer gate. ThreadSanitizer is not enabled: it requires a
separate, stable run of the runtime's thread-sensitive tests before its
findings could be made a reliable gate. ASan and UBSan are combined with
abort-on-error; neither is mixed with TSan. The byte-for-byte vendored
Ed25519 reference verifier alone excludes UBSan `shift-base` instrumentation
for its signed-limb arithmetic; ASan and other UBSan checks remain active.
The first full run found and led to correction of a POSIX package-content
file-descriptor parsing lifetime bug (`RQ-CF-CONTAINMENT-001`), and its
runtime-pipeline regression is now in the PR sanitizer selection.

## Dispatch and validation ladder

1. Run the smallest relevant local test while editing.
2. Push a branch, then dispatch a targeted cloud lane if deeper evidence is
   needed: `gh workflow run cloud-defect-hunt.yml --ref <branch> -f lane=fuzz
   -f profile=nightly -f seconds=600`. Manual inputs are bounded by the
   driver: fuzz seconds 1–1800, stress repetitions 1–30, rows 1–50,000.
3. Use ordinary PR checks for the hosted Linux, Windows, macOS, and new
   defect-hunt lanes. Nightly jobs run independently; manual `all` dispatch
   selects all four jobs.
4. Use Native Release Readiness and the existing RC candidate assembly path
   for platform, installer, VSIX, managed, security, and release evidence.
   Review the defect-hunt run for the candidate commit separately; the RC
   assembly workflow does not currently require it.

Use `gh run watch <run-id> --exit-status` or `gh pr checks <pr> --watch` to
wait. Inspect the failing job and its artifact after the blocking command
returns. Do not spend model turns polling an active run.

Hosted Windows is the normal automated Windows environment. Use the local
Windows VM for VFP9-dependent differential evidence, interactive Visual
Studio/VSIX testing, persistent installed components, or COM servers absent
from hosted runners. Hosted macOS is the automated modern macOS environment;
its CI tests are not evidence of manual GUI or hardware testing.

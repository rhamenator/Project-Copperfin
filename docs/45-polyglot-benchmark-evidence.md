# Representative Polyglot Benchmark Evidence

## Purpose

`run_polyglot_benchmark()` supplies the repeatable evidence boundary that feeds
the advisory route-impact evaluator. It executes caller-owned representative
workloads through exactly three named implementation layers:

- `direct-cpp`: the in-process C++ implementation
- `cpp-dotnet-wrapper`: Copperfin artifact admission, request serialization,
  bounded process execution, response validation, and parity comparison
- `csharp-service`: direct invocation of the same local Native AOT endpoint
  boundary, bypassing the Copperfin wrapper

The third layer is a local one-request endpoint boundary. It is not evidence of
a persistent or remote service, which Copperfin does not currently ship.

## Runner Contract

The request fixes warmup and measured iteration counts, one or more named
workloads with expected payloads, route availability/security/compatibility
facts, and the existing route-impact policy. Hard ceilings allow at most 100
warmup iterations, 10,000 measured iterations, and 100 workloads. Duplicate or
missing route classes, empty workload fields, overflow, invalid measurements,
and callback exceptions fail before any recommendation is usable.

Warmups exercise readiness but are excluded from metrics; any warmup execution
or parity failure invalidates the run. Measured observations
record invocation success, exact payload parity, wall-clock latency, startup
time, and peak resident memory. Aggregation uses nearest-rank p95, integer
throughput over total measured latency, maximum observed memory, and exact
failure/parity counts. Unavailable routes are retained with zero samples and
are never invoked or populated with synthetic timings.

`run_polyglot_benchmark()` owns no route registry, artifact, network client, or
promotion function. It passes measurements to `evaluate_polyglot_route_impact()`;
operators must separately review any recommendation and change routing through
the existing migration workflow.

## Memory Semantics

Windows bounded-process results expose a peak only when the owned Job Object
query succeeds; that value includes descendants. Generic POSIX bounded-process
results deliberately mark peak memory unavailable and leave the value zero.
`wait4()` cannot supply a trustworthy post-`exec` child peak because the
forked child's high-water mark can already contain the launcher's copy-on-write
resident set.

For this controlled cross-platform benchmark, the Native AOT candidate receives
an explicit opt-in environment flag and reports its own `PeakWorkingSet64` in
KiB on the otherwise-unused diagnostic channel. The harness accepts only the
exact metric prefix, a positive base-10 integer, and one terminating newline;
a missing or malformed metric fails that observation. Normal candidate calls
do not emit the metric. The current direct C++ sample performs no route-specific
allocation and therefore records zero additional KiB. These are route-impact
figures, not total host memory or a universal language comparison.

## Representative Workload

`test_polyglot_dotnet_candidate` runs positive, negative, and zero signed
64-bit additions. After one warmup iteration per workload and route, it records
three measured iterations per workload (nine samples per route). The C++/.NET
wrapper and direct endpoint paths use the same admitted Native AOT artifact but
different call layers, so their timing series are independently observed. Every
sample must execute successfully and exactly match the expected payload before
the advisory result is accepted.

The test prints a single `COPPERFIN_POLYGLOT_BENCHMARK_V1=` JSON payload for
evidence capture. A checked-in evidence document adds the exact source commit,
host/toolchain metadata, UTC capture time, policy, and limitations under the
versioned schema in `docs/contracts/polyglot-benchmark-result-v1.schema.json`.
Timing values are host-specific observations and must not be generalized.

The first checked-in result was captured from exact signed source commit
`cf4cd9573103096d915a211c9cd95aae413cb68c` on Linux x86_64. All 27 measured
executions completed with exact parity and no failures. The host-specific
advisory order was direct C++, local C# endpoint boundary, then the full
C++/.NET wrapper. The result and its explicit limitations are stored in
`docs/contracts/polyglot-benchmark-result-v1.json`; no route was changed.

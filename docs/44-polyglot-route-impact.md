# Polyglot Route Impact Recommendation

## Purpose

`evaluate_polyglot_route_impact()` is the deterministic advisory boundary for
choosing among the three implementation classes adopted by the .NET parity
roadmap:

- direct C++
- a C++ wrapper into an admitted .NET process
- a C# wrapper or service

The evaluator consumes measurements that a benchmark owner has already
captured for one canonical capability. It does not run a benchmark, invoke an
artifact, edit the route registry, or promote traffic. Its result is evidence
for the existing human-reviewed migration playbook, never an automatic route
change.

## Evidence Contract

One request contains a canonical capability ID, one policy, and exactly one
measurement for each implementation class. The policy declares:

- minimum sample count
- maximum p95 latency in microseconds
- minimum throughput per second
- maximum peak memory in KiB
- maximum p95 startup time in milliseconds
- the maximum permitted security-exposure profile
- integer latency, throughput, memory, and startup weights totaling 100

Each measurement records runtime availability, explicit security approval,
security profile, contract compatibility, sample/failure/parity-mismatch
counts, and the four impact metrics. Metric values have a fixed
`1,000,000,000,000` arithmetic ceiling. Counts and measurements are integer
machine data; display text and locale do not participate in the decision.

## Eligibility And Ranking

The evaluator applies hard gates before ranking. In order, a route must have an
available runtime, explicit security approval, a permitted security profile,
contract compatibility, enough samples, zero execution failures, zero parity
mismatches, and measurements inside every declared budget. Each rejected route
retains one stable `polyglot.impact.*` reason.

For each eligible route, the four measurements are normalized against their
budgets in integer basis points. Latency, memory, and startup use
`measurement / maximum`; throughput uses `minimum / measurement`, so a lower
weighted score is better in every dimension. Exact score ties use the stable
order direct C++, C++/.NET wrapper, then C# service. Input vector order cannot
change the output. The lowest score is preferred; remaining eligible routes
form the ordered fallback chain.

This score compares routes only under the capability-specific policy. It is not
a universal claim that one implementation language is faster, safer, or more
appropriate than another.

## Fail-Closed Behavior

Invalid identity, policy, route cardinality, duplicate routes, impossible
counts, and excessive metrics reject the complete request. When every route is
ineligible, the result is `polyglot.impact.no_eligible_route`,
`recommendation_ready` is false, the fallback chain is empty, and the preferred
field retains its direct-C++/native default. Operators must keep the live route
unchanged; insufficient evidence is not permission to promote.

## Related Evidence

The representative workload runner, result schema, capture procedure, and
measurement limitations are defined in
[`45-polyglot-benchmark-evidence.md`](45-polyglot-benchmark-evidence.md). Neither
component replaces artifact admission, parity comparison, route execution,
telemetry, signing, or human review. Those remain separate reviewed boundaries.

## Verification

`test_polyglot_route_impact` covers measured route and fallback selection,
input-order independence, every eligibility gate, deterministic ties,
fail-closed native/no-promotion behavior, and malformed request/policy/evidence
rejection. Fresh local Release coverage passes with the adjacent route registry,
route execution, migration telemetry, and isolation contracts (`5/5`); the
focused target repeats `20/20`. Clang 21 ASan/UBSan with leak detection passes
the focused target, and focused static analysis is clean. The test is audited
portable, parallel-safe, filesystem-free, environment-free, process-free,
network-free, and sample-free.

Exact signed product/test head `2f21378eb` passes Linux Native
`31477286106` and macOS Native `31477287811` at `338/338`, Windows Native
`31477289323` at `337/337`, and both `test_polyglot_route_impact` and
`test_native_test_isolation_contract` on every host. The macOS run additionally
passes both SET POINT display targets under `C`, `en_US.UTF-8`, `pt_BR.UTF-8`,
and `de_DE.UTF-8` (`8/8`). All eleven PR-triggered checks pass at that exact
head.

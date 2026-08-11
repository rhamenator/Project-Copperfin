# Engine Concurrency Policy

This document defines the runtime invariants for Copperfin's in-memory coordination surface:

- `SPAWN`
- `AWAIT`
- `CFTASKSTATUS()`, `CFTASKCANCEL()`, `CFTASKRESULT()`, and `CFTASKOUTPUT()`
- `CFPOLYGLOTDISPATCH()`
- `YIELD`
- `ENTER CRITICAL`
- `EXIT CRITICAL`
- lock retry/backoff under record or file-lock contention

The goal is deterministic runtime behavior with explicit deadlock avoidance, while staying compatible with the practical coordination expectations FoxPro/VFP developers bring to Copperfin.

## Scope

This policy applies to the native execution engine and to any runtime surface that:

- acquires a named critical section
- waits for another task to complete
- sleeps for time-based backoff
- retries a lock acquisition after contention
- introduces any new blocking behavior in engine code

## Canonical Rules

### 1. Critical sections are named, normalized, and in-memory

- `ENTER CRITICAL <name>` acquires an engine-managed in-memory mutex identified by the normalized section name.
- `ENTER CRITICAL` without a name uses the implicit `default` section.
- Re-entering the same normalized section from the same worker is allowed.

These sections are an engine coordination construct. They are not exposed as bitwise monitor primitives and they are not intended to mirror .NET's low-level synchronization API shape.

### 2. Nested cross-section acquires must follow ascending normalized-name order

- If a worker already holds section `alpha`, it may then enter `beta`.
- If a worker already holds section `beta`, it must not then enter `alpha`.
- Violations must fail fast with deterministic diagnostics instead of waiting indefinitely.

### 3. Exit critical sections in strict LIFO order

If nested `ENTER CRITICAL` operations are active, `EXIT CRITICAL` must target the most recent section first.

Engine diagnostic contract:

- event category: `runtime.critical.order_violation`
- fault text includes the held section and requested section

This rule prevents stack corruption and unlock races caused by out-of-order unwinds and keeps lock-state deterministic for deadlock prevention.

### 4. No blocking while any critical section is held

Once a worker holds at least one critical section, it must not perform an operation that can block on:

- another task's completion
- time-based sleep/backoff
- lock contention retry/backoff
- any future engine wait surface with equivalent semantics

Current enforced examples:

- `AWAIT`
- positive-duration `SLEEP`
- `CFPOLYGLOTDISPATCH()`, because the host callback may wait for bounded
  external execution
- lock retry/backoff reached from `RLOCK()`, `FLOCK()`, `LOCK()`, or mutation paths such as `REPLACE`, `APPEND BLANK`, `DELETE`, and `RECALL` when contention would otherwise trigger retry waits

Engine diagnostic contract:

- event category: `runtime.critical.blocking_violation`
- fault text includes the rejected blocking operation and held section name

Fast failure is intentional. The engine must not convert a critical section into a hidden wait state, because that would make deadlocks dependent on scheduling order and external contention timing.

### 4. Non-blocking cooperative operations remain allowed

- `YIELD` is allowed while a critical section is held because it does not wait on external completion, time, or lock ownership.
- The `CFTASK*()` supervision functions are allowed while a critical section is
  held. They use a zero-duration readiness probe or an atomic cancellation
  request and never join a worker. `AWAIT` remains the blocking join.
- A trusted polyglot runtime-host callback retains only immutable PRG request
  data and the read-only cancellation probe. It serializes same-capability
  artifact-admission revalidation/launch state internally; it never calls back
  into mutable PRG state. `CFTASKCANCEL()` therefore remains cooperative and
  governed by the configured bridge cancellation policy.
- Pure computation, local state updates, and other non-blocking operations are allowed.

The policy is specifically about blocking behavior, not about banning all coordination-aware statements inside critical sections.

## Implementation Obligations

Any new or refactored engine path that can block must:

1. detect whether a critical section is currently held
2. fail before entering the blocking wait or retry loop
3. emit deterministic diagnostics through the shared runtime event stream
4. add focused regression coverage proving the fast-fail behavior

In code, blocking paths should route through the shared critical-section policy helper rather than open-coding ad hoc checks.

## Compatibility Position

FoxPro/VFP does not define this exact engine coordination surface, so Copperfin must choose deterministic rules where legacy behavior is silent.

The compatibility objective is therefore:

- preserve practical developer expectations for named in-memory coordination
- avoid surprising managed-runtime-only synchronization idioms
- make deadlock prevention explicit and testable
- keep runtime diagnostics stable enough for debugger and host tooling

## Deadlock Prevention Summary

Copperfin prevents critical-section deadlocks with these hard rules:

1. all nested section acquires use one global name order
2. exits must unwind in strict LIFO order
3. no worker may block while holding a section

If both rules continue to hold, the engine avoids the classic circular-wait pattern that causes deadlocks in multi-worker code.

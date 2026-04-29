# Database Federation And Query Translation

## Goal

Copperfin should let developers keep using xBase-style commands and FoxPro-style SQL while talking to more than legacy DBF files.

That means:

- first-class legacy xBase storage
- first-class relational connectors
- modern document/JSON connectors
- modern vector and semantic-search connectors

## Core Principle

The trusted Copperfin core should normalize intent first.

Examples:

- `BROWSE`
- `SEEK`
- `SCAN`
- `REPLACE`
- FoxPro-style `SELECT`
- report/query/filter intent from designers

That normalized intent should then flow into deterministic connector translators where possible.

## Current Implementation Snapshot

This area is no longer purely future-looking:

- the platform already has a deterministic Fox SQL translation lane for relational backends
- the runtime host already has a deterministic federation execution-planning lane that materializes backend/target/sql plans
- SQL pass-through and remote-cursor behavior are implemented inside the runtime for the current local/synthetic cursor model

What is still missing is the live end-to-end connector layer: actual backend execution integration, broader provider/session behavior, and deeper non-relational connector slices.

## Translation Tiers

### Relational

These are the easiest and should be deterministic-first:

- SQLite
- PostgreSQL
- SQL Server
- Oracle

Strategy:

- parse FoxPro-style SQL into a Copperfin query AST
- lower that AST into target-dialect SQL
- keep behavior explainable and auditable
- keep the current shipped planner/translator path deterministic even as live connector execution is added

AI is not required for this path.

### Document And JSON

These are more dynamic:

- MongoDB-style stores
- JSON-first APIs
- sparse or nested document systems

Strategy:

- map browse/filter/select intent into projections, paths, and document predicates
- expose discovered schema hints back into Data Explorer
- use AI assistance only as an optional planner for ambiguous reshaping or nested-document synthesis

### Vector And Semantic Stores

These need additional operators beyond classic SQL:

- vector similarity search
- hybrid search
- embedding lookups
- semantic ranking

Strategy:

- add explicit Copperfin semantic operators rather than pretending they are plain SQL
- lower those operators into deterministic vector-search templates where possible
- allow optional AI planning for query drafting, explanation, or semantic retrieval shaping

## AI Rule

AI should help where the target store is dynamic or semantically rich, but it should not be required for the ordinary path.

The design rule is:

- deterministic translator first
- optional policy-controlled AI planner second
- never make the trusted runtime depend on a remote model just to execute a straightforward relational query

## Why This Matters

This is how Copperfin goes beyond VFP 9 while staying credible:

- legacy apps keep an xBase-native home
- modern apps can use relational, document, and vector data
- AI becomes an optional accelerator, not a hidden runtime dependency

## Near-Term Completion Criteria

This document should be considered "meaningfully complete" only when Copperfin has:

- live relational connector execution behind the current translator/planner surface
- clearer capability/behavior contracts for provider-backed sessions and cursors
- deeper documentation for how local xBase intent maps onto relational and non-relational targets

import subprocess
import json
import re
import sys

repo = "rhamenator/Project-Copperfin"

# 1. Fetch current issues to check for existing titles
print("Fetching current issues...")
cmd = ["gh", "issue", "list", "-R", repo, "--limit", "300", "--state", "all", "--json", "number,title,url"]
res = subprocess.run(cmd, capture_output=True, text=True, check=True)
issues = json.loads(res.stdout, strict=False)

existing_titles = {iss["title"].strip() for iss in issues}
print(f"Found {len(existing_titles)} existing issues.")

# Exact titles to create
titles = [
    "Polyglot Contract v1: define schema, versioning, and compatibility rules",
    "Dispatch Registry: implement capability route-state lifecycle (off/shadow/canary/on/retire-legacy)",
    "Bridge Invocation Semantics: timeout, cancellation propagation, and fallback matrix",
    "Shadow Mode Parity Comparator: diff native vs candidate outputs with mismatch telemetry",
    "Polyglot Telemetry & Diagnostics: route decisions, fallback reasons, and migration outcomes",
    "Developer Migration Playbook: leaf-unit-first replacement workflow and promotion gates"
]

issue_bodies = {
    "Polyglot Contract v1: define schema, versioning, and compatibility rules": """## Summary
Canonical contract artifact for capability id, typed envelopes, execution budget, fallback, observability fields.
Child of #4700.

## Scope
Contract document + machine-readable schema + validator tests.

## Acceptance Criteria
Schema validates required fields; semantic version compatibility policy documented; examples for success/error envelopes.

## Non-Goals
Runtime routing implementation.

## Traceability
Follow-up to #4700.""",

    "Dispatch Registry: implement capability route-state lifecycle (off/shadow/canary/on/retire-legacy)": """## Summary
Route each capability by state with deterministic selection behavior.
Child of #4700.

## Scope
Registry model, config loading, route decision function, unit tests.

## Acceptance Criteria
All five states test-covered; invalid state/config errors explicit; default safe state off when config absent.

## Non-Goals
Bridge invocation mechanics.

## Traceability
Child of #4700.""",

    "Bridge Invocation Semantics: timeout, cancellation propagation, and fallback matrix": """## Summary
Implement bridge call semantics aligned with runtime concurrency/cancel behavior from #272.
Child of #4700.

## Scope
Timeout handling, cancellation token propagation, fallback policy application.

## Acceptance Criteria
Tests for timeout/cancel/failure classes map to configured fallback; deterministic error mapping emitted.

## Non-Goals
Shadow parity comparator UX.

## Traceability
References #272 and #4700.""",

    "Shadow Mode Parity Comparator: diff native vs candidate outputs with mismatch telemetry": """## Summary
Support shadow routing by executing both paths and comparing outputs while returning native.
Child of #4700.

## Scope
Comparator rules, mismatch categories, tolerance knobs, telemetry events.

## Acceptance Criteria
Parity pass/fail metrics emitted; mismatch samples include capability id + reason; no behavior change to caller in shadow mode.

## Non-Goals
Canary traffic policy.

## Traceability
Child of #4700.""",

    "Polyglot Telemetry & Diagnostics: route decisions, fallback reasons, and migration outcomes": """## Summary
Standardize event taxonomy for migration visibility and operability.
Child of #4700.

## Scope
Event names/fields, diagnostics formatting, integration with existing runtime event streams.

## Acceptance Criteria
Route decision + fallback + parity mismatch + latency outcomes observable in tests; docs include event contract table.

## Non-Goals
Policy engine redesign.

## Traceability
Child of #4700.""",

    "Developer Migration Playbook: leaf-unit-first replacement workflow and promotion gates": """## Summary
Step-by-step process teams can execute from off to retire-legacy.
Child of #4700.

## Scope
Workflow guide, checklists, sample capability migration, CI parity gate criteria.

## Acceptance Criteria
Docs include rollback procedure, promotion thresholds, required evidence for each state transition.

## Non-Goals
Automatic migration tooling.

## Traceability
Child of #4700."""
}

created_issues = []

for t in titles:
    body = issue_bodies[t]
    # Check if title exists
    final_title = t
    if final_title in existing_titles:
        final_title = t + " (slice)"
        print(f"Title already exists, renaming to: {final_title}")
    
    print(f"Creating issue: {final_title}")
    create_cmd = [
        "gh", "issue", "create",
        "-R", repo,
        "--title", final_title,
        "--body", body
    ]
    res_create = subprocess.run(create_cmd, capture_output=True, text=True, check=True)
    out_url = res_create.stdout.strip()
    print(f"Created: {out_url}")
    
    # Extract issue number from url
    num = out_url.split("/")[-1]
    created_issues.append({
        "number": num,
        "title": final_title,
        "url": out_url
    })

# Now post comment to issue #4700
comment_body = "Child issue plan (recommended order):\n"
for idx, iss in enumerate(created_issues, 1):
    comment_body += f"{idx}. {iss['url']}\n"

print("Posting comment to #4700...")
comment_cmd = [
    "gh", "issue", "comment", "4700",
    "-R", repo,
    "--body", comment_body
]
res_comment = subprocess.run(comment_cmd, capture_output=True, text=True, check=True)
comment_url = res_comment.stdout.strip()
print(f"Comment posted: {comment_url}")

print("\n--- RESULTS ---")
for iss in created_issues:
    print(f"Issue Number: {iss['number']}")
    print(f"Title: {iss['title']}")
    print(f"URL: {iss['url']}\n")

print(f"Comment URL: {comment_url}")


// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

namespace copperfin::security {

// Test-only fault-injection seam for WorkspaceAgentSessionController::stop()'s
// exception-safety guarantee (issue #5401): stop() calls this exactly once,
// after transition_ has been set to stopping but before the revocation-lease
// wait and before it clears the active session, and throws whatever the hook
// throws. It exists because the real trigger for that guarantee --
// allocation failure or an OS mutex/condition-variable primitive throwing --
// cannot be reproduced deterministically through the controller's ordinary
// public API. Production code must never set this hook; it defaults to
// nullptr and is a no-op call site (checked-and-skipped) when unset, so it
// has no effect on production behavior.
//
// This hook is a single process-global variable shared by every
// WorkspaceAgentSessionController instance, not scoped per-instance or
// per-call -- safe only because the test suite that uses it runs every test
// sequentially on one thread. It must not be set while any thread could
// concurrently call stop() on any controller instance, and tests must clear
// it (set nullptr) immediately after use, before another test or thread
// calls stop().
void set_workspace_agent_session_stop_test_only_throw_hook_for_testing(
    void (*hook)());

}  // namespace copperfin::security

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

namespace copperfin::security {

// Test-only fault-injection seam for WorkspaceAgentSessionController::stop()'s
// exception-safety guarantee (issue #5401): stop() calls this exactly once,
// immediately before it clears the active session, and throws whatever the
// hook throws. It exists because the real trigger for that guarantee --
// allocation failure or an OS mutex/condition-variable primitive throwing --
// cannot be reproduced deterministically through the controller's ordinary
// public API. Production code must never set this hook; it defaults to
// nullptr and is a no-op call site (checked-and-skipped) when unset, so it
// has no effect on production behavior. Setting it affects every
// WorkspaceAgentSessionController instance in the process, so tests must
// clear it (set nullptr) immediately after use, before another test or
// thread calls stop().
void set_workspace_agent_session_stop_test_only_throw_hook_for_testing(
    void (*hook)());

}  // namespace copperfin::security

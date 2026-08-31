// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#if defined(COPPERFIN_ENABLE_WORKSPACE_AGENT_SESSION_TEST_HOOKS)

namespace copperfin::security {

// Test-only fault-injection seam for WorkspaceAgentSessionController::stop()'s
// exception-safety guarantee (issue #5401): stop() calls this exactly once,
// after transition_ has been set to stopping but before the revocation-lease
// wait and before it clears the active session, and throws whatever the hook
// throws. It exists because the real trigger for that guarantee --
// allocation failure or an OS mutex/condition-variable primitive throwing --
// cannot be reproduced deterministically through the controller's ordinary
// public API.
//
// This entire seam -- the atomic hook variable, the check-and-fire call site
// in stop(), and this setter -- only exists when
// COPPERFIN_ENABLE_WORKSPACE_AGENT_SESSION_TEST_HOOKS is defined, which
// CMakeLists.txt does only for the cf_security library variant that
// COPPERFIN_BUILD_TESTS links tests against (matching this codebase's
// existing convention for test-only hooks, e.g.
// COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS). A production build never
// defines the macro, so the symbol this header declares does not exist in a
// production binary; an adversarial review of PR #5416 found the first
// version of this header had no such gate and shipped the setter in every
// production binary that links cf_security.
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

#endif  // COPPERFIN_ENABLE_WORKSPACE_AGENT_SESSION_TEST_HOOKS

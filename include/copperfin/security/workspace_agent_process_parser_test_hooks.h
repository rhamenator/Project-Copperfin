// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#if defined(COPPERFIN_ENABLE_WORKSPACE_AGENT_PROCESS_PARSER_TEST_HOOKS)

namespace copperfin::security {

// Test-only seam for the trusted-executable read-then-verify path shared by
// capture_binding() and authorize_windows() (issue #5421 round-2 review):
// both call this hook, if set, immediately before reading the checked
// executable's bytes, letting a test deterministically interleave a
// filesystem change (e.g. adding a hard link) between the pre-read
// containment check and the read -- the exact window a real attacker would
// need, and the exact window that was silently left unguarded once already
// in this file's initial migration to the handle-based read primitive
// (issue #5409/#5420). This cannot be reproduced deterministically through
// the public API otherwise, since both functions perform the check and the
// read within one synchronous call with no exposed seam between them.
//
// This entire seam -- the atomic hook variable, the check-and-fire call
// site, and this setter -- only exists when
// COPPERFIN_ENABLE_WORKSPACE_AGENT_PROCESS_PARSER_TEST_HOOKS is defined,
// which CMakeLists.txt does only for the cf_security library variant that
// COPPERFIN_BUILD_TESTS links tests against, matching this codebase's
// existing test-only-hook convention (e.g.
// COPPERFIN_ENABLE_WORKSPACE_AGENT_SESSION_TEST_HOOKS). A production build
// never defines the macro, so the symbol this header declares does not
// exist in a production binary.
//
// Single-shot and process-global, not scoped per-call -- safe only because
// the test suite that uses it runs every test sequentially on one thread.
// Tests must clear it (set nullptr) immediately after use, before another
// test relies on it being unset.
void set_workspace_agent_process_parser_pre_read_test_hook_for_testing(
    void (*hook)());

}  // namespace copperfin::security

#endif  // COPPERFIN_ENABLE_WORKSPACE_AGENT_PROCESS_PARSER_TEST_HOOKS

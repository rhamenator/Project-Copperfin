// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#if defined(COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS)

namespace copperfin::security {

// Test-only seam for authorize_external_process()'s Windows path (issue
// #5430): fired after the pre-verification file identity is captured but
// before the final post-verification identity read, letting a test
// deterministically rename/replace the resolved executable inside the
// window between the signature/publisher checks and the identity
// comparison that closes it -- the exact race a real attacker would need,
// and one that cannot be reproduced deterministically through the public
// API otherwise since the whole authorization runs within one synchronous
// call with no exposed seam between its independent path-string opens.
//
// This entire seam -- the atomic hook variable, the check-and-fire call
// site, and this setter -- only exists when
// COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS is defined, which
// CMakeLists.txt does only for the cf_security library variant that
// COPPERFIN_BUILD_TESTS links tests against, matching this codebase's
// existing test-only-hook convention (e.g.
// COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS). A
// production build never defines the macro, so the symbol this header
// declares does not exist in a production binary.
//
// Single-shot and process-global, not scoped per-call -- safe only because
// the test suite that uses it runs every test sequentially on one thread.
// Tests must clear it (set nullptr) immediately after use, before another
// test relies on it being unset.
void set_external_process_policy_pre_identity_check_test_hook_for_testing(
    void (*hook)());

}  // namespace copperfin::security

#endif  // COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS

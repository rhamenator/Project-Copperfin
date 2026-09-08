// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#if defined(COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS)

#include <string>

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

struct AuthenticodeSignatureProbeResult {
    bool trusted = false;
    std::string signer_display_name;
};

#if defined(_WIN32)
// Test-only seam exposing the raw Authenticode verification step
// (verify_authenticode_signature(), internal to external_process_policy.cpp)
// in isolation, independent of allowed_path_roots/allowed_publishers/PATH
// resolution. ExternalProcessAuthorizationResult deliberately does not
// expose the verified signer name -- production callers only need the
// allow/deny decision -- so there is otherwise no way for a test to learn
// a real signed binary's actual signer display name. Hardcoding an
// assumed name instead is exactly the mistake that let #5429's publisher
// string go unverified until #5450's CI kept failing (dotnet.exe's real
// signer name on GitHub Actions Windows runners is ".NET", not "Microsoft
// Corporation"). Tests should discover the real name via this hook first,
// then use it, rather than assume it.
[[nodiscard]] AuthenticodeSignatureProbeResult
verify_authenticode_signature_for_testing(const std::string& path);
#endif  // _WIN32

}  // namespace copperfin::security

#endif  // COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS

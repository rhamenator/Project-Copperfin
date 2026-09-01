// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#if defined(COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS)

namespace copperfin::platform {

// Test-only seam for admit_polyglot_supporting_artifact()'s post-read path
// re-walk (issue #5427, added after adversarial review found the
// handle-based read alone does not guarantee resolved_path_ still resolves
// to the object that was read -- see issue #5426 for the identical gap and
// fix in a sibling file). Fired immediately before that re-walk, letting a
// test deterministically rename/replace the artifact between the read and
// the re-walk -- the exact window a real attacker would need, and one that
// cannot be reproduced deterministically through the public API otherwise
// since the read and the re-walk happen within one synchronous call with
// no exposed seam between them.
//
// This entire seam -- the atomic hook variable, the check-and-fire call
// site, and this setter -- only exists when
// COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS is
// defined, which CMakeLists.txt does only for the cf_platform_profile
// library variant that COPPERFIN_BUILD_TESTS links tests against, matching
// this codebase's existing test-only-hook convention (e.g.
// COPPERFIN_ENABLE_WORKSPACE_AGENT_PROCESS_PARSER_TEST_HOOKS). A production
// build never defines the macro, so the symbol this header declares does
// not exist in a production binary.
//
// Single-shot and process-global, not scoped per-call -- safe only because
// the test suite that uses it runs every test sequentially on one thread.
// Tests must clear it (set nullptr) immediately after use, before another
// test relies on it being unset.
void set_polyglot_supporting_artifact_admission_post_read_test_hook_for_testing(
    void (*hook)());

}  // namespace copperfin::platform

#endif  // COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"
#include "copperfin/security/physical_path_containment.h"

#include <array>
#include <atomic>

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
#include "runtime_pipeline_test_hooks.h"
#endif

#if defined(_WIN32)
#include <windows.h>
#endif

namespace copperfin::runtime {

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
namespace {
// See runtime_pipeline_test_hooks.h. Single-shot, same-thread: fired
// synchronously from admit_launcher_artifact(), never from a background
// thread, so relaxed ordering is sufficient.
std::atomic<void (*)()> launcher_artifact_post_read_test_hook{nullptr};
}  // namespace

namespace test_hooks {
void set_launcher_artifact_post_read_test_hook(void (*hook)()) {
    launcher_artifact_post_read_test_hook.store(hook, std::memory_order_relaxed);
}
}  // namespace test_hooks
#endif

namespace runtime_pipeline_detail {
namespace {

constexpr std::string_view kGeneratedLauncherInternalPrefix = "Copperfin.GeneratedLauncher.";
#if defined(_WIN32)
constexpr std::string_view kGeneratedLauncherAppHost = "Copperfin.GeneratedLauncher.apphost.exe";
#endif
constexpr std::string_view kGeneratedLauncherDll = "Copperfin.GeneratedLauncher.dll";
constexpr std::string_view kGeneratedLauncherDeps = "Copperfin.GeneratedLauncher.deps.json";
constexpr std::string_view kGeneratedLauncherRuntimeConfig = "Copperfin.GeneratedLauncher.runtimeconfig.json";
constexpr std::string_view kGeneratedLauncherPdb = "Copperfin.GeneratedLauncher.pdb";

struct LauncherArtifactSpec {
    std::filesystem::path path;
    RuntimeLauncherArtifactRole role = RuntimeLauncherArtifactRole::runtime_required;
    bool required = true;
};

bool is_internal_generated_launcher_name(const std::string& file_name) {
    const std::string folded_name = lowercase_copy(file_name);
    const std::string folded_prefix = lowercase_copy(std::string(kGeneratedLauncherInternalPrefix));
    return folded_name.starts_with(folded_prefix);
}

bool file_names_equal_case_insensitive(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
#if defined(_WIN32)
    const std::wstring left_name = left.filename().native();
    const std::wstring right_name = right.filename().native();
    return CompareStringOrdinal(
               left_name.c_str(),
               -1,
               right_name.c_str(),
               -1,
               TRUE) == CSTR_EQUAL;
#else
    return lowercase_copy(copperfin::platform::path_to_utf8_string(left.filename())) ==
        lowercase_copy(copperfin::platform::path_to_utf8_string(right.filename()));
#endif
}

bool launcher_file_names_equal_case_insensitive(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
    return file_names_equal_case_insensitive(left, right);
}

std::vector<std::filesystem::path> casefold_package_entries(
    const std::filesystem::path& package_root,
    const std::filesystem::path& expected_file_name,
    std::string& error) {
    std::vector<std::filesystem::path> matches;
    const std::string expected_name = copperfin::platform::path_to_utf8_string(expected_file_name);
    std::error_code iterator_error;
    for (std::filesystem::directory_iterator it(package_root, iterator_error), end;
         it != end;
         it.increment(iterator_error)) {
        if (iterator_error) {
            error = runtime_text(
                "Runtime.Package.Error.LauncherArtifactNotDirectRegularFile",
                {{"path", expected_name}});
            return {};
        }
        if (launcher_file_names_equal_case_insensitive(
                it->path().filename(),
                expected_file_name)) {
            matches.push_back(it->path());
        }
    }
    if (iterator_error) {
        error = runtime_text(
            "Runtime.Package.Error.LauncherArtifactNotDirectRegularFile",
            {{"path", expected_name}});
        return {};
    }
    return matches;
}

// Deliberately fail-fast, no retry, on every rejection path below:
// parent-path mismatch, a directory-iteration error, ambiguous match,
// missing, containment-denied, read-failed, the post-read rename/
// replace rejection, and digest-computation failure -- decided in
// issue #5435 rather than added silently. Two reasons:
//
//   1. This runs at package build time, not in a production execution
//      path -- a transient race with another legitimate tool touching
//      package_root (an overlapping rebuild, an editor autosave, an AV
//      scan) fails the whole package build, and a human re-running the
//      build is the existing, sufficient recovery path for any build
//      failure. Not attacker-adjacent, so the cost of not auto-recovering
//      is low.
//   2. Auto-retrying specifically the rename/replace rejection (#5426's
//      round-2 fix) would work against the reason that check exists: it
//      would give a TOCTOU attacker's swap window more read attempts to
//      land outside it, and a "succeeded on retry" outcome would erase
//      the audit signal that a rename/replace was ever observed. That
//      signal is LauncherArtifactRenamedDuringRead, a diagnostic code
//      distinct from the generic containment/read-failure paths' code,
//      kept that way specifically so this isn't masked (issue #5435,
//      round 2). Blind retry can't tell a benign race from an
//      attacker's second attempt.
//
// If build-time flakiness from this function becomes an observed
// operational problem, a bounded, code-aware retry (e.g. retry only the
// ambiguous-match and missing-file cases, never the rename/replace
// rejection) would be the way to revisit this, not a blanket retry.
bool admit_launcher_artifact(
    const LauncherArtifactSpec& spec,
    const std::filesystem::path& package_root,
    std::vector<RuntimeLauncherArtifact>& inventory,
    std::string& error) {
    const std::string expected_name = copperfin::platform::path_to_utf8_string(spec.path.filename());
    if (spec.path.parent_path().lexically_normal() != package_root.lexically_normal()) {
        error = runtime_text(
            "Runtime.Package.Error.LauncherArtifactNotDirectRegularFile",
            {{"path", expected_name}});
        return false;
    }

    const auto matches = casefold_package_entries(package_root, spec.path.filename(), error);
    if (!error.empty()) {
        return false;
    }
    if (matches.size() > 1U) {
        error = runtime_text(
            "Runtime.Package.Error.LauncherArtifactAmbiguous",
            {{"path", expected_name}});
        return false;
    }
    const auto exact = std::find_if(matches.begin(), matches.end(), [&](const std::filesystem::path& match) {
        return copperfin::platform::path_to_utf8_string(match.filename()) == expected_name;
    });
    if (exact == matches.end()) {
        if (!spec.required && matches.empty()) {
            return true;
        }
        error = runtime_text(
            "Runtime.Package.Error.LauncherArtifactMissing",
            {{"path", expected_name}});
        return false;
    }

    // Atomic check-and-open primitive (issue #5409/#5420/#5426): the read
    // below is bound to the exact object this walk verified, never reopened
    // by path string. This call site has no link_count-dependent trust
    // invariant (unlike workspace_agent_process_parser.cpp's #5421
    // migration) -- the digest computed here is only ever stored in the
    // build-time launcher-artifact inventory, never compared against a
    // captured identity's link_count downstream -- so no additional
    // post-read link_count re-check is needed beyond the handle-based
    // read's own content_equal() freshness check.
    auto handle = security::inspect_and_open_physically_contained_path(*exact, package_root);
    const auto& containment = handle.result();
    if (!containment.allowed) {
        error = runtime_text(
            "Runtime.Package.Error.LauncherArtifactNotDirectRegularFile",
            {{"path", expected_name}});
        return false;
    }
    // read_physically_contained_file_snapshot_from_handle() alone guarantees
    // the bytes read are exactly what the walk verified, but makes no claim
    // about whether the path still resolves to that same object once this
    // call returns -- this inventory entry is keyed by expected_name, and a
    // rename/replace of the artifact during the read would otherwise let a
    // digest for an object the package no longer actually ships under that
    // name be silently recorded. The
    // _and_revalidate_path() variant closes that gap with an independent
    // post-read re-walk (issue #5434, consolidating what used to be a
    // hand-rolled re-walk here and in polyglot_supporting_artifact_admission.cpp;
    // originally found by adversarial review on PR #5428).
    void (*post_read_hook)() = nullptr;
#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
    post_read_hook =
        launcher_artifact_post_read_test_hook.exchange(nullptr, std::memory_order_relaxed);
#endif
    const auto snapshot =
        security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
            handle, package_root, post_read_hook);
    if (!snapshot.ok) {
        // Distinct from LauncherArtifactNotDirectRegularFile (used by the
        // containment-denied branch above) when the failure is specifically
        // a rename/replace observed mid-read, so this outcome is
        // identifiable in logs rather than folded into the generic
        // containment diagnostic (found by adversarial review on PR
        // #5444/issue #5435).
        error = runtime_text(
            snapshot.failure == security::PhysicalPathContainmentFailure::identity_changed
                ? "Runtime.Package.Error.LauncherArtifactRenamedDuringRead"
                : "Runtime.Package.Error.LauncherArtifactNotDirectRegularFile",
            {{"path", expected_name}});
        return false;
    }
    const auto digest = security::sha256_hex_for_text(snapshot.bytes);
    if (!digest.ok) {
        error = digest.error;
        return false;
    }

    inventory.push_back({
        .package_relative_path = expected_name,
        .role = spec.role,
        .sha256 = digest.hex_digest
    });
    return true;
}

void append_plan_owned_direct_file_name(
    std::vector<std::string>& names,
    const std::filesystem::path& package_root,
    const std::string& path_text) {
    if (path_text.empty()) {
        return;
    }
    const std::filesystem::path path = copperfin::platform::path_from_utf8_string(path_text);
    if (path.parent_path().lexically_normal() != package_root.lexically_normal()) {
        return;
    }
    names.push_back(copperfin::platform::path_to_utf8_string(path.filename()));
}

void append_plan_owned_direct_file_names(
    std::vector<std::string>& names,
    const RuntimePackagePlan& plan,
    const std::filesystem::path& package_root) {
    const std::array<const std::string*, 17U> paths{
        &plan.manifest_path,
        &plan.debug_manifest_path,
        &plan.ast_manifest_path,
        &plan.ir_manifest_path,
        &plan.transpiled_csharp_path,
        &plan.launcher_project_path,
        &plan.launcher_source_path,
        &plan.module_definition_path,
        &plan.native_wrapper_source_path,
        &plan.native_wrapper_cmake_path,
        &plan.native_wrapper_build_script_path,
        &plan.native_wrapper_build_powershell_path,
        &plan.library_api_manifest_path,
        &plan.fll_api_manifest_path,
        &plan.fxp_token_manifest_path,
        &plan.app_archive_manifest_path,
        &plan.runtime_host_destination_path
    };
    for (const auto* path : paths) {
        append_plan_owned_direct_file_name(names, package_root, *path);
    }
    append_plan_owned_direct_file_name(names, package_root, plan.audit_log_path);
}

}  // namespace

std::string_view launcher_artifact_role_name(const RuntimeLauncherArtifactRole role) {
    switch (role) {
        case RuntimeLauncherArtifactRole::public_apphost:
            return "public_apphost";
        case RuntimeLauncherArtifactRole::runtime_required:
            return "runtime_required";
        case RuntimeLauncherArtifactRole::debug_optional:
            return "debug_optional";
    }
    return "runtime_required";
}

bool validate_public_output_artifact_name(
    const RuntimePackagePlan& plan,
    std::string& error) {
    error.clear();
    if (plan.package_root.empty() || plan.launcher_output_path.empty()) {
        return true;
    }

    const std::filesystem::path output_name =
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path);
    std::vector<std::filesystem::path> reserved_names;
    if (is_native_host_output_kind(plan.output_kind) || is_library_output_kind(plan.output_kind)) {
        reserved_names.emplace_back(
            copperfin::platform::path_from_utf8_string(plan.runtime_host_destination_path));
    }
    if (plan.emit_dotnet_launcher) {
#if defined(_WIN32)
        reserved_names.emplace_back(kGeneratedLauncherAppHost);
#endif
        reserved_names.emplace_back(kGeneratedLauncherDll);
        reserved_names.emplace_back(kGeneratedLauncherDeps);
        reserved_names.emplace_back(kGeneratedLauncherRuntimeConfig);
        reserved_names.emplace_back(kGeneratedLauncherPdb);
    }

    for (const auto& reserved_name : reserved_names) {
        if (!reserved_name.empty() &&
            file_names_equal_case_insensitive(output_name, reserved_name)) {
            error = runtime_text(
                "Runtime.Package.Error.OutputNameReserved",
                {
                    {"outputName", copperfin::platform::path_to_utf8_string(output_name.filename())},
                    {"reservedName", copperfin::platform::path_to_utf8_string(reserved_name.filename())}
                });
            return false;
        }
    }
    return true;
}

bool inventory_generated_launcher_artifacts(
    const RuntimePackagePlan& plan,
    std::vector<RuntimeLauncherArtifact>& inventory,
    std::string& error) {
    const std::filesystem::path package_root =
        copperfin::platform::path_from_utf8_string(plan.package_root);
    const std::vector<LauncherArtifactSpec> specs{
        {
            .path = copperfin::platform::path_from_utf8_string(plan.launcher_output_path),
            .role = RuntimeLauncherArtifactRole::public_apphost,
            .required = true
        },
#if defined(_WIN32)
        {
            .path = package_root / kGeneratedLauncherAppHost,
            .role = RuntimeLauncherArtifactRole::runtime_required,
            .required = true
        },
#endif
        {
            .path = package_root / kGeneratedLauncherDll,
            .role = RuntimeLauncherArtifactRole::runtime_required,
            .required = true
        },
        {
            .path = package_root / kGeneratedLauncherDeps,
            .role = RuntimeLauncherArtifactRole::runtime_required,
            .required = true
        },
        {
            .path = package_root / kGeneratedLauncherRuntimeConfig,
            .role = RuntimeLauncherArtifactRole::runtime_required,
            .required = true
        },
        {
            .path = package_root / kGeneratedLauncherPdb,
            .role = RuntimeLauncherArtifactRole::debug_optional,
            .required = false
        }
    };

    inventory.clear();
    for (const auto& spec : specs) {
        if (!admit_launcher_artifact(spec, package_root, inventory, error)) {
            inventory.clear();
            return false;
        }
    }

    std::vector<std::string> allowed_names;
    allowed_names.reserve(specs.size());
    for (const auto& spec : specs) {
        allowed_names.push_back(copperfin::platform::path_to_utf8_string(spec.path.filename()));
    }
    append_plan_owned_direct_file_names(allowed_names, plan, package_root);
    std::error_code iterator_error;
    for (std::filesystem::directory_iterator it(package_root, iterator_error), end;
         it != end;
         it.increment(iterator_error)) {
        if (iterator_error) {
            break;
        }
        const std::string file_name = copperfin::platform::path_to_utf8_string(it->path().filename());
        if (!is_internal_generated_launcher_name(file_name)) {
            continue;
        }
        if (std::find(allowed_names.begin(), allowed_names.end(), file_name) != allowed_names.end()) {
            continue;
        }
        error = runtime_text(
            "Runtime.Package.Error.LauncherArtifactUnexpected",
            {{"path", file_name}});
        inventory.clear();
        return false;
    }
    if (iterator_error) {
        error = runtime_text(
            "Runtime.Package.Error.LauncherArtifactNotDirectRegularFile",
            {{"path", copperfin::platform::path_to_utf8_string(package_root.filename())}});
        inventory.clear();
        return false;
    }

    return true;
}

bool is_launcher_owned_digest(
    const RuntimeArtifactDigest& digest,
    const RuntimePackagePlan& plan) {
    const std::filesystem::path path = copperfin::platform::path_from_utf8_string(digest.path);
    const std::filesystem::path package_root =
        copperfin::platform::path_from_utf8_string(plan.package_root);
    bool parent_is_package_root =
        path.parent_path().lexically_normal() == package_root.lexically_normal();
    if (!parent_is_package_root) {
        std::error_code equivalent_error;
        parent_is_package_root = std::filesystem::equivalent(
            path.parent_path(),
            package_root,
            equivalent_error);
    }
    if (!parent_is_package_root) {
        return false;
    }
    return launcher_file_names_equal_case_insensitive(
               path.filename(),
               copperfin::platform::path_from_utf8_string(plan.launcher_output_path).filename()) ||
        is_internal_generated_launcher_name(copperfin::platform::path_to_utf8_string(path.filename()));
}

}  // namespace runtime_pipeline_detail
}  // namespace copperfin::runtime

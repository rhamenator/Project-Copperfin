// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"
#include "copperfin/security/physical_path_containment.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace copperfin::runtime::runtime_pipeline_detail {
namespace {

constexpr std::string_view kGeneratedLauncherInternalPrefix = "Copperfin.GeneratedLauncher.";
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

bool launcher_file_names_equal_case_insensitive(
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
    return lowercase_copy(left.filename().string()) ==
        lowercase_copy(right.filename().string());
#endif
}

std::vector<std::filesystem::path> casefold_package_entries(
    const std::filesystem::path& package_root,
    const std::filesystem::path& expected_file_name,
    std::string& error) {
    std::vector<std::filesystem::path> matches;
    const std::string expected_name = expected_file_name.generic_string();
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

bool admit_launcher_artifact(
    const LauncherArtifactSpec& spec,
    const std::filesystem::path& package_root,
    std::vector<RuntimeLauncherArtifact>& inventory,
    std::string& error) {
    const std::string expected_name = spec.path.filename().generic_string();
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
        return match.filename().generic_string() == expected_name;
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

    const auto containment = security::inspect_physical_path_containment(*exact, package_root);
    if (!containment.allowed) {
        error = runtime_text(
            "Runtime.Package.Error.LauncherArtifactNotDirectRegularFile",
            {{"path", expected_name}});
        return false;
    }
    const auto snapshot = security::read_physically_contained_file_snapshot(containment, package_root);
    if (!snapshot.ok) {
        error = runtime_text(
            "Runtime.Package.Error.LauncherArtifactNotDirectRegularFile",
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

bool inventory_generated_launcher_artifacts(
    const RuntimePackagePlan& plan,
    std::vector<RuntimeLauncherArtifact>& inventory,
    std::string& error) {
    const std::filesystem::path package_root(plan.package_root);
    const std::vector<LauncherArtifactSpec> specs{
        {
            .path = std::filesystem::path(plan.launcher_output_path),
            .role = RuntimeLauncherArtifactRole::public_apphost,
            .required = true
        },
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
        allowed_names.push_back(spec.path.filename().generic_string());
    }
    std::error_code iterator_error;
    for (std::filesystem::directory_iterator it(package_root, iterator_error), end;
         it != end;
         it.increment(iterator_error)) {
        if (iterator_error) {
            break;
        }
        const std::string file_name = it->path().filename().generic_string();
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
            {{"path", package_root.filename().generic_string()}});
        inventory.clear();
        return false;
    }

    return true;
}

bool is_launcher_owned_digest(
    const RuntimeArtifactDigest& digest,
    const RuntimePackagePlan& plan) {
    const std::filesystem::path path(digest.path);
    const std::filesystem::path package_root(plan.package_root);
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
               std::filesystem::path(plan.launcher_output_path).filename()) ||
        is_internal_generated_launcher_name(path.filename().string());
}

}  // namespace copperfin::runtime::runtime_pipeline_detail

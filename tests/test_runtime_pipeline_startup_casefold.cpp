// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_support.h"

namespace cf_test_runtime_pipeline {
namespace {

struct StartupFamily {
    std::string primary_extension;
    std::string companion_extension;
    std::string type_title;
};

const std::vector<StartupFamily>& startup_families() {
    static const std::vector<StartupFamily> families{
        {".PRG", {}, "Program"},
        {".SCX", ".SCT", "Form"},
        {".VCX", ".VCT", "Class Library"},
        {".FRX", ".FRT", "Report"},
        {".LBX", ".LBT", "Label"},
        {".MNX", ".MNT", "Menu"}};
    return families;
}

std::string lowercase_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

copperfin::studio::StudioProjectWorkspace startup_workspace(
    const std::filesystem::path& project_dir,
    const std::filesystem::path& output_dir,
    const std::string& project_title,
    const std::string& startup_path,
    const std::string& type_title) {
    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = project_title;
    workspace.home_directory = copperfin::platform::path_to_utf8_string(project_dir);
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = project_title;
    workspace.build_plan.output_path = copperfin::platform::path_to_utf8_string(
        output_dir / (project_title + ".exe"));
    workspace.build_plan.startup_item = startup_path;
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {{
        .record_index = 1U,
        .name = startup_path,
        .relative_path = startup_path,
        .type_title = type_title}};
    return workspace;
}

copperfin::runtime::RuntimePackagePlan create_startup_plan(
    const copperfin::studio::StudioDocumentModel& document,
    const copperfin::studio::StudioProjectWorkspace& workspace,
    const std::filesystem::path& output_dir) {
    return copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        copperfin::platform::path_to_utf8_string(output_dir),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);
}

copperfin::runtime::RuntimeMaterializeResult materialize_startup_plan(
    const copperfin::runtime::RuntimePackagePlan& plan,
    const std::filesystem::path& runtime_host) {
    return copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        copperfin::platform::path_to_utf8_string(runtime_host));
}

bool has_exact_directory_entry(
    const std::filesystem::path& directory,
    const std::string& file_name) {
    std::error_code error;
    for (std::filesystem::directory_iterator iterator(directory, error), end;
         iterator != end;
         iterator.increment(error)) {
        if (error) {
            return false;
        }
        if (copperfin::platform::path_to_utf8_string(iterator->path().filename()) == file_name) {
            return true;
        }
    }
    return false;
}

}  // namespace

void test_casefold_startup_paths_preserve_actual_spelling_for_all_mvp_families() {
    namespace fs = std::filesystem;

    std::size_t index = 0U;
    for (const auto& family : startup_families()) {
        const fs::path temp_root =
            fs::temp_directory_path() /
            ("copperfin_runtime_pipeline_casefold_startup_" + std::to_string(index));
        const fs::path project_dir = temp_root / "project";
        const fs::path actual_dir = project_dir / "Sources" / "Launchers";
        const fs::path output_dir = temp_root / "output";
        const fs::path runtime_host = runtime_host_fixture_path(temp_root);
        const std::string primary_name = "Start" + family.primary_extension;
        const std::string recorded_name =
            "sources/LAUNCHERS\\start" +
            lowercase_ascii(std::filesystem::path(primary_name).extension().string());
        const std::string project_title = "CasefoldStartup" + std::to_string(index);
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(actual_dir);

        const fs::path primary_source = actual_dir / primary_name;
        write_text(primary_source, "startup-primary-" + std::to_string(index));
        fs::path companion_source;
        if (!family.companion_extension.empty()) {
            companion_source = actual_dir / ("START" + family.companion_extension);
            write_text(companion_source, "startup-companion-" + std::to_string(index));
        }
        write_text(runtime_host, "runtime-host");

        copperfin::studio::StudioDocumentModel document;
        document.path = (project_dir / (project_title + ".pjx")).string();
        const auto workspace = startup_workspace(
            project_dir,
            output_dir,
            project_title,
            recorded_name,
            family.type_title);
        const auto plan = create_startup_plan(document, workspace, output_dir);

        const std::string expected_relative = "Sources/Launchers/" + primary_name;
        const fs::path expected_staged = fs::path(plan.content_root) / expected_relative;
        expect(plan.ok, "#3953: casefold startup plan should be created for " + family.type_title);
        expect(plan.assets.size() == 1U,
               "#3953: casefold startup plan should retain one asset for " + family.type_title);
        if (plan.assets.size() == 1U) {
            expect(paths_refer_to_same_filesystem_entry(
                       plan.assets.front().source_path, primary_source),
                   "#3953: startup source should preserve actual spelling for " + family.type_title);
            expect(plan.assets.front().relative_path == expected_relative,
                   "#3953: staged relative path should preserve actual spelling for " + family.type_title);
            expect(plan.assets.front().source_resolution_error.empty(),
                   "#3953: unique casefold startup resolution should not report ambiguity for " + family.type_title);
        }
        expect(plan.startup_source_path == expected_staged.lexically_normal().string(),
               "#3953: runtime startup identity should preserve actual spelling for " + family.type_title);
        expect(paths_refer_to_same_filesystem_entry(
                   plan.debug_plan.startup_source_path, primary_source),
               "#3953: debug startup identity should preserve source spelling for " + family.type_title);
        expect(plan.debug_plan.supports_breakpoints,
               "#3953: resolved startup should retain debug support for " + family.type_title);

        const auto result = materialize_startup_plan(plan, runtime_host);
        expect_materialization(result, "#3953: uniquely casefolded startup should materialize for " + family.type_title);
        if (result.ok) {
            const std::string runtime_manifest = read_text(result.plan.manifest_path);
            const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
            expect(fs::exists(expected_staged),
                   "#3953: resolved startup primary should be staged for " + family.type_title);
            expect(
                runtime_manifest.find("asset=1|" + expected_relative + "|") != std::string::npos,
                "#3953: runtime asset identity should preserve actual spelling for " + family.type_title);
            expect(
                manifest_value_for_key(runtime_manifest, "startup_source") ==
                    quote_manifest_value(expected_staged.lexically_normal().string()),
                "#3953: runtime manifest startup path should preserve actual spelling for " + family.type_title);
            expect(paths_refer_to_same_filesystem_entry(
                       decode_manifest_value(
                           manifest_value_for_key(debug_manifest, "startup_source")),
                       primary_source),
                "#3953: debug manifest startup path should preserve actual spelling for " + family.type_title);
            if (!family.companion_extension.empty()) {
                const fs::path staged_companion =
                    fs::path(result.plan.content_root) /
                    fs::path(expected_relative).parent_path() /
                    companion_source.filename();
                expect(has_exact_directory_entry(
                           staged_companion.parent_path(),
                           companion_source.filename().string()),
                       "#3953: required startup companion should preserve actual spelling for " + family.type_title);
                const auto companion_digest = std::find_if(
                    result.plan.extension_payload_digests.begin(),
                    result.plan.extension_payload_digests.end(),
                    [&](const copperfin::runtime::RuntimeArtifactDigest& digest) {
                        return fs::path(digest.path).filename().string() ==
                            companion_source.filename().string();
                    });
                expect(companion_digest != result.plan.extension_payload_digests.end(),
                       "#3953: companion digest should preserve actual spelling for " + family.type_title);
                if (companion_digest != result.plan.extension_payload_digests.end()) {
                    expect(
                        runtime_manifest.find(
                            "extension_payload=" + quote_manifest_value(companion_digest->path) + "|") !=
                            std::string::npos,
                        "#3953: companion manifest identity should preserve actual spelling for " + family.type_title);
                }
            }
        }

        fs::remove_all(temp_root, ignored);
        ++index;
    }
}

void test_exact_startup_path_wins_over_casefold_siblings() {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_pipeline_exact_startup_precedence";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir / "Exact");
    fs::create_directories(project_dir / "EXACT");
    write_text(project_dir / "Exact" / "main.prg", "exact-startup");
    write_text(project_dir / "Exact" / "MAIN.PRG", "casefold-sibling");
    write_text(project_dir / "EXACT" / "main.prg", "directory-sibling");
    write_text(runtime_host, "runtime-host");

    const bool supports_case_distinct_entries =
        has_exact_directory_entry(project_dir, "Exact") &&
        has_exact_directory_entry(project_dir, "EXACT") &&
        has_exact_directory_entry(project_dir / "Exact", "main.prg") &&
        has_exact_directory_entry(project_dir / "Exact", "MAIN.PRG");
    if (!supports_case_distinct_entries) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "exact_startup.pjx").string();
    const auto workspace = startup_workspace(
        project_dir,
        output_dir,
        "ExactStartup",
        "Exact/main.prg",
        "Program");
    const auto plan = create_startup_plan(document, workspace, output_dir);

    expect(plan.assets.size() == 1U &&
               paths_refer_to_same_filesystem_entry(
                   plan.assets.front().source_path, project_dir / "Exact" / "main.prg"),
           "#3953: exact startup components should win over casefold siblings");
    const auto result = materialize_startup_plan(plan, runtime_host);
        expect_materialization(result, "#3953: exact startup path should materialize despite casefold siblings");
    if (result.ok) {
        expect(read_text(fs::path(result.plan.content_root) / "Exact" / "main.prg") == "exact-startup",
               "#3953: exact startup path should stage the exact source bytes");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ambiguous_casefold_startup_path_fails_closed() {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_pipeline_ambiguous_startup_path";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir / "Forms");
    fs::create_directories(project_dir / "FORMS");
    write_text(project_dir / "Forms" / "Start.SCX", "first-form");
    write_text(project_dir / "Forms" / "START.SCT", "first-form-memo");
    write_text(project_dir / "FORMS" / "Start.SCX", "second-form");
    write_text(project_dir / "FORMS" / "START.SCT", "second-form-memo");
    write_text(runtime_host, "runtime-host");

    if (!has_exact_directory_entry(project_dir, "Forms") ||
        !has_exact_directory_entry(project_dir, "FORMS")) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "ambiguous_startup.pjx").string();
    const std::string recorded_path = "forms\\start.scx";
    const auto workspace = startup_workspace(
        project_dir,
        output_dir,
        "AmbiguousStartup",
        recorded_path,
        "Form");
    const auto plan = create_startup_plan(document, workspace, output_dir);
    const std::string requested_path = (project_dir / "forms" / "start.scx").string();
    const std::string expected_error = runtime_pipeline_english_catalog().translate(
        "Runtime.Package.Error.AmbiguousProjectAssetPath",
        {{"path", requested_path}});

    expect(plan.assets.size() == 1U &&
               plan.assets.front().source_resolution_error == expected_error,
           "#3953: ambiguous startup path should be retained as a localized plan error");
    expect(!plan.debug_plan.supports_breakpoints,
           "#3953: ambiguous startup path should not advertise debug support");
    const auto result = materialize_startup_plan(plan, runtime_host);
    expect(!result.ok, "#3953: ambiguous casefold startup path should fail package materialization");
    expect(result.error == expected_error,
           "#3953: ambiguous startup failure should preserve the localized diagnostic");
    expect(!fs::exists(plan.package_root),
           "#3953: ambiguous startup failure should not commit a package root");

    fs::remove_all(temp_root, ignored);
}

void test_startup_resolution_preserves_parent_tail_and_name_fallbacks() {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_pipeline_startup_fallbacks";
    const fs::path source_root = temp_root / "VFPSource";
    const fs::path project_dir = source_root / "addlabel";
    const fs::path shared_dir = source_root / "Wizards" / "wzcommon";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(shared_dir);
    write_text(shared_dir / "REGISTRY.VCX", "shared-class-library");
    write_text(shared_dir / "REGISTRY.VCT", "shared-class-library-memo");
    write_text(project_dir / "MAIN.PRG", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "fallbacks.pjx").string();

    auto parent_workspace = startup_workspace(
        project_dir,
        output_dir,
        "ParentTailStartup",
        R"(..\wzcommon\registry.vcx)",
        "Class Library");
    const auto parent_plan = create_startup_plan(document, parent_workspace, output_dir);
    const fs::path expected_parent_source = shared_dir / "REGISTRY.VCX";
    const fs::path expected_parent_staged =
        fs::path(parent_plan.content_root) / "Wizards" / "wzcommon" / "REGISTRY.VCX";

    expect(parent_plan.assets.size() == 1U &&
               paths_refer_to_same_filesystem_entry(
                   parent_plan.assets.front().source_path, expected_parent_source),
           "#3953: selected startup should retain the existing parent-relative tail fallback");
    expect(parent_plan.startup_source_path == expected_parent_staged.string(),
           "#3953: parent-relative startup fallback should preserve actual staged spelling");
    const auto parent_result = materialize_startup_plan(parent_plan, runtime_host);
    expect(parent_result.ok,
           "#3953: uniquely resolved parent-relative startup fallback should materialize");
    if (parent_result.ok) {
        expect(fs::exists(expected_parent_staged),
               "#3953: parent-relative startup fallback should stage the primary");
        expect(fs::exists(expected_parent_staged.parent_path() / "REGISTRY.VCT"),
               "#3953: parent-relative startup fallback should stage its required sidecar");
    }

    auto name_workspace = startup_workspace(
        project_dir,
        output_dir,
        "NameFallbackStartup",
        "stale/path.prg",
        "Program");
    name_workspace.entries.front().name = "MAIN.PRG";
    const auto name_plan = create_startup_plan(document, name_workspace, output_dir);
    const fs::path expected_name_source = project_dir / "MAIN.PRG";
    const fs::path expected_name_staged = fs::path(name_plan.content_root) / "MAIN.PRG";

    expect(name_plan.assets.size() == 1U &&
               paths_refer_to_same_filesystem_entry(
                   name_plan.assets.front().source_path, expected_name_source),
           "#3953: selected startup should retain the existing entry-name fallback");
    expect(name_plan.startup_source_path == expected_name_staged.string(),
           "#3953: entry-name startup fallback should preserve actual staged spelling");
    const auto name_result = materialize_startup_plan(name_plan, runtime_host);
    expect(name_result.ok,
           "#3953: uniquely resolved entry-name startup fallback should materialize");
    if (name_result.ok) {
        expect(read_text(expected_name_staged) == "RETURN\n",
               "#3953: entry-name startup fallback should stage the resolved source bytes");
    }

    fs::remove_all(temp_root, ignored);
}

void test_absolute_project_item_paths_never_rebind_to_project_decoys() {
    namespace fs = std::filesystem;

    struct MissingAbsoluteCase {
        std::string label;
        std::string recorded_path;
        std::string decoy_name;
        std::string type_title;
        std::string companion_extension;
    };

    const fs::path shared_root =
        fs::temp_directory_path() / "copperfin_runtime_pipeline_absolute_project_items";
    std::error_code ignored;
    fs::remove_all(shared_root, ignored);
    fs::create_directories(shared_root);

    const std::vector<MissingAbsoluteCase> missing_cases{
        {
            "POSIX PRG",
            (shared_root / "missing-posix" / "startup.prg").string(),
            "startup.prg",
            "Program",
            {}
        },
        {
            "Windows PRG",
            R"(Q:\CopperfinMissingAbsolute\Sources\startup.prg)",
            "startup.prg",
            "Program",
            {}
        },
        {
            "UNC PRG",
            R"(\\tmp\copperfin_runtime_pipeline_absolute_project_items\unc-alias\startup.prg)",
            "startup.prg",
            "Program",
            {}
        },
        {
            "POSIX FRX",
            (shared_root / "missing-posix" / "invoice.frx").string(),
            "invoice.frx",
            "Report",
            ".frt"
        },
        {
            "Windows LBX",
            R"(Q:\CopperfinMissingAbsolute\Labels\customer.lbx)",
            "customer.lbx",
            "Label",
            ".lbt"
        }
    };

    std::size_t case_index = 0U;
    for (const auto& missing_case : missing_cases) {
        const fs::path temp_root = shared_root / ("missing-" + std::to_string(case_index));
        const fs::path project_dir = temp_root / "project";
        const fs::path output_dir = temp_root / "output";
        const fs::path runtime_host = runtime_host_fixture_path(temp_root);
        const fs::path decoy_path = project_dir / missing_case.decoy_name;
        fs::create_directories(project_dir);
        write_text(decoy_path, "project-decoy-bytes");
        if (!missing_case.companion_extension.empty()) {
            fs::path decoy_companion = decoy_path;
            decoy_companion.replace_extension(missing_case.companion_extension);
            write_text(decoy_companion, "project-decoy-companion-bytes");
        }
        write_text(runtime_host, "runtime-host");

        const fs::path original_working_directory = fs::current_path();
        bool changed_working_directory = false;
#if !defined(_WIN32)
        if (missing_case.recorded_path.size() >= 2U &&
            missing_case.recorded_path[0U] == '\\' &&
            missing_case.recorded_path[1U] == '\\') {
            std::string normalized_host_alias = missing_case.recorded_path;
            std::replace(
                normalized_host_alias.begin(),
                normalized_host_alias.end(),
                '\\',
                '/');
            const fs::path host_alias_decoy(normalized_host_alias);
            fs::create_directories(host_alias_decoy.parent_path());
            write_text(host_alias_decoy, "unc-host-alias-decoy-bytes");
        }
        if (missing_case.recorded_path.size() >= 3U &&
            missing_case.recorded_path[1U] == ':') {
            const fs::path cwd_decoy_root = temp_root / "cwd-decoy";
            std::string normalized_cwd_decoy = missing_case.recorded_path;
            std::replace(
                normalized_cwd_decoy.begin(),
                normalized_cwd_decoy.end(),
                '\\',
                '/');
            const fs::path cwd_decoy_path = cwd_decoy_root / fs::path(normalized_cwd_decoy);
            fs::create_directories(cwd_decoy_path.parent_path());
            write_text(cwd_decoy_path, "cwd-drive-decoy-bytes");
            if (!missing_case.companion_extension.empty()) {
                fs::path cwd_decoy_companion = cwd_decoy_path;
                cwd_decoy_companion.replace_extension(missing_case.companion_extension);
                write_text(cwd_decoy_companion, "cwd-drive-decoy-companion-bytes");
            }
            fs::current_path(cwd_decoy_root, ignored);
            changed_working_directory = !ignored;
        }
#endif

        copperfin::studio::StudioDocumentModel document;
        document.path = (project_dir / "absolute_items.pjx").string();
        auto workspace = startup_workspace(
            project_dir,
            output_dir,
            "MissingAbsolute" + std::to_string(case_index),
            missing_case.recorded_path,
            missing_case.type_title);
        workspace.entries[0U].name = missing_case.decoy_name;

        const auto plan = create_startup_plan(document, workspace, output_dir);
        if (changed_working_directory) {
            fs::current_path(original_working_directory, ignored);
        }
        std::string normalized_recorded_path = missing_case.recorded_path;
        std::replace(
            normalized_recorded_path.begin(),
            normalized_recorded_path.end(),
            '\\',
            '/');
        fs::path expected_unresolved(normalized_recorded_path);
#if !defined(_WIN32)
        if (normalized_recorded_path.rfind("//", 0U) != 0U) {
            expected_unresolved = expected_unresolved.lexically_normal();
        }
#else
        expected_unresolved = expected_unresolved.lexically_normal();
#endif
        expect(plan.assets.size() == 1U,
               "#3991: " + missing_case.label + " missing-absolute plan should retain one startup asset");
        if (plan.assets.size() == 1U) {
            expect(plan.assets[0U].source_path == expected_unresolved.string() &&
                       !plan.assets[0U].exists,
                   "#3991: " + missing_case.label +
                       " missing absolute should remain unresolved instead of binding the decoy");
            expect(plan.assets[0U].source_path != decoy_path.string(),
                   "#3991: " + missing_case.label + " plan must not adopt the project decoy path");
        }
        expect(plan.debug_plan.startup_source_path == expected_unresolved.string() &&
                   !plan.debug_plan.supports_breakpoints,
               "#3991: " + missing_case.label +
                   " debug metadata should retain unresolved absolute provenance");

        const auto security_profile = copperfin::security::default_native_security_profile();
        const auto extensibility_profile = copperfin::platform::default_extensibility_profile();
        const std::string runtime_manifest = copperfin::runtime::build_runtime_manifest_text(
            plan, security_profile, extensibility_profile);
        const std::string debug_manifest = copperfin::runtime::build_debug_manifest_text(
            plan, security_profile, extensibility_profile);
        expect(runtime_manifest.find(decoy_path.string()) == std::string::npos &&
                   debug_manifest.find(decoy_path.string()) == std::string::npos &&
                   runtime_manifest.find("project-decoy-bytes") == std::string::npos &&
                   debug_manifest.find("project-decoy-bytes") == std::string::npos,
               "#3991: " + missing_case.label +
                   " decoy path and bytes must not enter manifest contracts");

        const auto result = materialize_startup_plan(plan, runtime_host);
        const std::string expected_error = runtime_pipeline_english_catalog().translate(
            "Runtime.Package.Error.SourceFileMissing",
            {{"path", expected_unresolved.string()}});
        expect(!result.ok && result.error == expected_error,
               "#3991: " + missing_case.label +
                   " missing absolute should fail through the localized source diagnostic");
        expect(!fs::exists(plan.package_root),
               "#3991: " + missing_case.label +
                   " missing absolute failure should not commit package content");

        ++case_index;
    }

    const fs::path valid_root = shared_root / "valid";
    const fs::path project_dir = valid_root / "project";
    const fs::path external_dir = valid_root / "external";
    const fs::path output_dir = valid_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(valid_root);
    const fs::path decoy_path = project_dir / "startup.prg";
    const fs::path absolute_source = external_dir / "startup.prg";
    fs::create_directories(project_dir);
    fs::create_directories(external_dir);
    write_text(decoy_path, "project-decoy-bytes");
    write_text(absolute_source, "authoritative-absolute-bytes");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "valid_absolute.pjx").string();
    auto workspace = startup_workspace(
        project_dir,
        output_dir,
        "ValidAbsolute",
        absolute_source.string(),
        "Program");
    workspace.entries[0U].name = "startup.prg";

    const auto plan = create_startup_plan(document, workspace, output_dir);
    expect(plan.assets.size() == 1U && plan.assets[0U].exists &&
               paths_refer_to_same_filesystem_entry(
                   plan.assets[0U].source_path, absolute_source) &&
               paths_refer_to_same_filesystem_entry(
                   plan.debug_plan.startup_source_path, absolute_source),
           "#3991: an existing absolute source should preserve authoritative provenance");
    const auto result = materialize_startup_plan(plan, runtime_host);
    expect_materialization(result, "#3991: an existing absolute source should still materialize");
    if (result.ok && !result.plan.assets.empty()) {
        expect(read_text(result.plan.assets[0U].staged_path) == "authoritative-absolute-bytes",
               "#3991: valid absolute materialization should stage source bytes, not decoy bytes");
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(debug_manifest.find(quote_manifest_value(absolute_source.string())) != std::string::npos &&
                   debug_manifest.find(quote_manifest_value(decoy_path.string())) == std::string::npos,
               "#3991: valid absolute debug metadata should retain only authoritative provenance");
    }

    fs::remove_all(shared_root, ignored);
}

void test_missing_startup_primary_fails_for_all_mvp_families() {
    namespace fs = std::filesystem;

    std::size_t index = 0U;
    for (const auto& family : startup_families()) {
        const fs::path temp_root =
            fs::temp_directory_path() /
            ("copperfin_runtime_pipeline_missing_startup_" + std::to_string(index));
        const fs::path project_dir = temp_root / "project";
        const fs::path output_dir = temp_root / "output";
        const fs::path runtime_host = runtime_host_fixture_path(temp_root);
        const std::string startup_name = "Missing" + family.primary_extension;
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(project_dir);
        write_text(runtime_host, "runtime-host");

        copperfin::studio::StudioDocumentModel document;
        document.path = (project_dir / ("missing_" + std::to_string(index) + ".pjx")).string();
        const auto workspace = startup_workspace(
            project_dir,
            output_dir,
            "MissingStartup" + std::to_string(index),
            startup_name,
            family.type_title);
        const auto plan = create_startup_plan(document, workspace, output_dir);
        const auto result = materialize_startup_plan(plan, runtime_host);
        const std::string expected_error = runtime_pipeline_english_catalog().translate(
            "Runtime.Package.Error.SourceFileMissing",
            {{"path", (project_dir / startup_name).string()}});

        expect(!result.ok, "#3953: missing startup primary should fail for " + family.type_title);
        expect(result.error == expected_error,
               "#3953: missing startup primary should use the localized path diagnostic for " + family.type_title);
        expect(!plan.debug_plan.supports_breakpoints,
               "#3953: missing startup primary should disable debug support for " + family.type_title);

        fs::remove_all(temp_root, ignored);
        ++index;
    }
}

void test_missing_required_startup_sidecar_fails_for_all_xasset_families() {
    namespace fs = std::filesystem;

    std::size_t index = 0U;
    for (const auto& family : startup_families()) {
        if (family.companion_extension.empty()) {
            continue;
        }

        const fs::path temp_root =
            fs::temp_directory_path() /
            ("copperfin_runtime_pipeline_missing_startup_sidecar_" + std::to_string(index));
        const fs::path project_dir = temp_root / "project";
        const fs::path output_dir = temp_root / "output";
        const fs::path runtime_host = runtime_host_fixture_path(temp_root);
        const std::string startup_name = "Start" + family.primary_extension;
        const fs::path primary_source = project_dir / startup_name;
        fs::path expected_companion = primary_source;
        expected_companion.replace_extension(lowercase_ascii(family.companion_extension));
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(project_dir);
        write_text(primary_source, "startup-primary");
        write_text(runtime_host, "runtime-host");

        copperfin::studio::StudioDocumentModel document;
        document.path = (project_dir / ("missing_sidecar_" + std::to_string(index) + ".pjx")).string();
        const auto workspace = startup_workspace(
            project_dir,
            output_dir,
            "MissingSidecar" + std::to_string(index),
            startup_name,
            family.type_title);
        const auto plan = create_startup_plan(document, workspace, output_dir);
        const auto result = materialize_startup_plan(plan, runtime_host);
        fs::path diagnostic_companion = expected_companion;
        if (!plan.assets.empty() && !plan.assets.front().source_path.empty()) {
            diagnostic_companion = fs::path(plan.assets.front().source_path);
            diagnostic_companion.replace_extension(lowercase_ascii(family.companion_extension));
        }
        const std::string expected_error = runtime_pipeline_english_catalog().translate(
            "Runtime.Package.Error.SourceFileMissing",
            {{"path", diagnostic_companion.string()}});

        expect(!result.ok, "#3953: missing required sidecar should fail for " + family.type_title);
        expect(result.error == expected_error,
               "#3953: missing required sidecar should use the localized path diagnostic for " + family.type_title);
        expect(!fs::exists(plan.package_root),
               "#3953: missing required sidecar should not commit a package for " + family.type_title);

        fs::remove_all(temp_root, ignored);
        ++index;
    }
}

void test_unicode_runtime_package_paths_preserve_source_and_manifest_contracts() {
    namespace fs = std::filesystem;
    std::string unicode_directory = "copperfin_runtime_pipeline_";
    unicode_directory += "\xE9\xA0\xB9\xE7\x9B\xAE";
    std::string unicode_source_name = "caf";
    unicode_source_name += "\xC3\xA9.prg";

    const fs::path temp_root =
        fs::temp_directory_path() / copperfin::platform::path_from_utf8_string(unicode_directory);
    const fs::path project_dir = temp_root / "project";
    const fs::path source_dir = project_dir / "Sources";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    const fs::path source_path = source_dir / copperfin::platform::path_from_utf8_string(unicode_source_name);
    const std::string startup_relative =
        "Sources/" + copperfin::platform::path_to_utf8_string(source_path.filename());
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_dir);
    write_text(source_path, "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = copperfin::platform::path_to_utf8_string(project_dir / "Unicode.pjx");
    const auto workspace = startup_workspace(
        project_dir,
        output_dir,
        "Unicode",
        startup_relative,
        "Program");
    const auto plan = create_startup_plan(document, workspace, output_dir);

    expect(plan.ok, "#3873: Unicode runtime package plan should be created");
    expect(plan.assets.size() == 1U, "#3873: Unicode runtime package should retain its source asset");
    if (plan.assets.size() == 1U) {
        expect(paths_refer_to_same_filesystem_entry(
                   plan.assets.front().source_path, source_path),
               "#3873: Unicode runtime source path should preserve UTF-8 spelling");
        const auto source_digest = copperfin::security::sha256_hex_for_file(
            copperfin::platform::path_to_utf8_string(source_path));
        expect(source_digest.ok,
               "#3873: Unicode runtime source should be readable by the security hash boundary "
                   "(error=" + source_digest.error + ")");
    }

    const auto result = materialize_startup_plan(plan, runtime_host);
    expect_materialization(result, "#3873: Unicode runtime package should materialize");
    if (result.ok) {
        expect(
            fs::exists(copperfin::platform::path_from_utf8_string(result.plan.runtime_host_destination_path)) &&
                read_text(copperfin::platform::path_from_utf8_string(result.plan.runtime_host_destination_path)) ==
                    "runtime-host",
            "#4249: Unicode runtime-host source paths should stage the runtime host payload");
        const auto source_digest = copperfin::security::sha256_hex_for_file(
            copperfin::platform::path_to_utf8_string(source_path));
        expect(result.plan.assets.size() == 1U && source_digest.ok &&
                   result.plan.assets.front().sha256 == source_digest.hex_digest,
               "#3873: materialized Unicode runtime source should retain its security digest");
        const std::string runtime_manifest = read_text(
            copperfin::platform::path_from_utf8_string(result.plan.manifest_path));
        const std::string debug_manifest = read_text(
            copperfin::platform::path_from_utf8_string(result.plan.debug_manifest_path));
        expect(runtime_manifest.find(copperfin::platform::path_to_utf8_string(source_path.filename())) !=
                   std::string::npos,
               "#3873: runtime manifest should retain the Unicode source name");
        expect(paths_refer_to_same_filesystem_entry(
                   decode_manifest_value(
                       manifest_value_for_key(debug_manifest, "startup_source")),
                   source_path),
               "#3873: debug manifest should retain the Unicode source path");
        expect(fs::exists(copperfin::platform::path_from_utf8_string(result.plan.startup_source_path)),
               "#3873: Unicode startup source should be staged at the manifest path");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_runtime_pipeline

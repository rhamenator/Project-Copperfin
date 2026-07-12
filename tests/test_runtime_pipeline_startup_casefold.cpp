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
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = project_title;
    workspace.build_plan.output_path = (output_dir / (project_title + ".exe")).string();
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
        output_dir.string(),
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
        runtime_host.string());
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
        if (iterator->path().filename().string() == file_name) {
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
            expect(plan.assets.front().source_path == primary_source.lexically_normal().string(),
                   "#3953: startup source should preserve actual spelling for " + family.type_title);
            expect(plan.assets.front().relative_path == expected_relative,
                   "#3953: staged relative path should preserve actual spelling for " + family.type_title);
            expect(plan.assets.front().source_resolution_error.empty(),
                   "#3953: unique casefold startup resolution should not report ambiguity for " + family.type_title);
        }
        expect(plan.startup_source_path == expected_staged.lexically_normal().string(),
               "#3953: runtime startup identity should preserve actual spelling for " + family.type_title);
        expect(plan.debug_plan.startup_source_path == primary_source.lexically_normal().string(),
               "#3953: debug startup identity should preserve source spelling for " + family.type_title);
        expect(plan.debug_plan.supports_breakpoints,
               "#3953: resolved startup should retain debug support for " + family.type_title);

        const auto result = materialize_startup_plan(plan, runtime_host);
        expect(result.ok, "#3953: uniquely casefolded startup should materialize for " + family.type_title);
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
            expect(
                manifest_value_for_key(debug_manifest, "startup_source") ==
                    quote_manifest_value(primary_source.lexically_normal().string()),
                "#3953: debug manifest startup path should preserve actual spelling for " + family.type_title);
            if (!family.companion_extension.empty()) {
                const fs::path staged_companion = expected_staged.parent_path() / companion_source.filename();
                expect(fs::exists(staged_companion),
                       "#3953: required startup companion should preserve actual spelling for " + family.type_title);
                expect(
                    runtime_manifest.find(
                        "extension_payload=" + quote_manifest_value(staged_companion.string()) + "|") !=
                        std::string::npos,
                    "#3953: companion manifest identity should preserve actual spelling for " + family.type_title);
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
               plan.assets.front().source_path == (project_dir / "Exact" / "main.prg").string(),
           "#3953: exact startup components should win over casefold siblings");
    const auto result = materialize_startup_plan(plan, runtime_host);
    expect(result.ok, "#3953: exact startup path should materialize despite casefold siblings");
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
               parent_plan.assets.front().source_path == expected_parent_source.string(),
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
               name_plan.assets.front().source_path == expected_name_source.string(),
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
        const std::string expected_error = runtime_pipeline_english_catalog().translate(
            "Runtime.Package.Error.SourceFileMissing",
            {{"path", expected_companion.string()}});

        expect(!result.ok, "#3953: missing required sidecar should fail for " + family.type_title);
        expect(result.error == expected_error,
               "#3953: missing required sidecar should use the localized path diagnostic for " + family.type_title);
        expect(!fs::exists(plan.package_root),
               "#3953: missing required sidecar should not commit a package for " + family.type_title);

        fs::remove_all(temp_root, ignored);
        ++index;
    }
}

}  // namespace cf_test_runtime_pipeline

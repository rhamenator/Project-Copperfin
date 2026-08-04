// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_runtime_pipeline_support.h"
#include "runtime_pipeline_support.h"
#include "../src/runtime/runtime_pipeline_test_hooks.h"

#include <cerrno>
#include <thread>

#if defined(_WIN32)
#include <direct.h>
#endif

namespace cf_test_runtime_pipeline {
namespace {

namespace fs = std::filesystem;

#if defined(_WIN32)
std::string lowercase_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}
#endif

void change_current_directory(const fs::path& next) {
#if defined(_WIN32)
    if (::_wchdir(next.c_str()) != 0) {
        throw fs::filesystem_error(
            "unable to change current directory",
            next,
            std::error_code(errno, std::generic_category()));
    }
#else
    fs::current_path(next);
#endif
}

struct ScopedCurrentDirectory {
    fs::path previous;

    explicit ScopedCurrentDirectory(const fs::path& next)
        : previous(fs::current_path()) {
        change_current_directory(next);
    }

    ~ScopedCurrentDirectory() {
#if defined(_WIN32)
        (void)::_wchdir(previous.c_str());
#else
        std::error_code ignored;
        fs::current_path(previous, ignored);
#endif
    }
};

copperfin::studio::StudioProjectWorkspace package_workspace(
    const fs::path& project_dir,
    const fs::path& output_dir,
    const std::vector<copperfin::studio::StudioProjectEntry>& entries) {
    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ContainedAssets";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.output_path = (output_dir / "ContainedAssets.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = entries;
    return workspace;
}

const copperfin::runtime::RuntimePackageAsset* asset_by_record(
    const copperfin::runtime::RuntimePackagePlan& plan,
    const std::size_t record_index) {
    const auto found = std::find_if(
        plan.assets.begin(),
        plan.assets.end(),
        [&](const copperfin::runtime::RuntimePackageAsset& asset) {
            return asset.record_index == record_index;
        });
    return found == plan.assets.end() ? nullptr : &*found;
}

#if defined(_WIN32)
void run_windows_drive_relative_case(
    const fs::path& project_parent,
    const std::string& identity,
    const std::string& primary_extension,
    const std::string& companion_extension,
    const std::string& type_title,
    const bool expect_different_drive) {
    const fs::path user_profile(getenv_value("USERPROFILE"));
    expect(!user_profile.empty(),
           "#4065: Windows drive-relative validation requires USERPROFILE");
    if (user_profile.empty()) {
        return;
    }
    const fs::path source_drive = user_profile.root_path();
    expect(!source_drive.empty(),
           "#4065: Windows drive-relative validation requires a profile drive");
    if (source_drive.empty()) {
        return;
    }
    const fs::path source_temp_parent = user_profile / "AppData" / "Local" / "Temp";
    const fs::path temp_root = source_temp_parent /
        (expect_different_drive ? "cf4065d" : "cf4065s");
    const fs::path source_current = temp_root / "s";
    const fs::path external_root = temp_root / "e";
    const fs::path external_target =
        external_root / ("payload" + primary_extension);
    fs::path external_companion = external_target;
    external_companion.replace_extension(companion_extension);
    const fs::path tail = external_target.lexically_relative(source_drive);
    const fs::path source_target = source_current / tail;
    fs::path source_companion = source_target;
    source_companion.replace_extension(companion_extension);
    const fs::path project_dir = project_parent / "p";
    const fs::path output_dir = project_parent / "o";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);

    expect(source_target.native().size() > 64U,
           "#4065: Windows drive-relative fixture should exercise full-path buffer growth");

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::remove_all(project_dir, ignored);
    fs::remove_all(output_dir, ignored);
    fs::create_directories(source_target.parent_path());
    fs::create_directories(external_target.parent_path());
    fs::create_directories(project_dir);
    write_text(source_target, "drive-current-source");
    write_text(source_companion, "drive-current-companion");
    write_text(external_target, "external-sentinel");
    write_text(external_companion, "external-companion-sentinel");
    write_text(project_dir / "main.prg", "RETURN\n");
    const std::string missing_name = "missing_payload" + primary_extension;
    write_text(project_dir / missing_name, "project-decoy");
    write_text(runtime_host, "runtime-host");

    const std::string source_root_name = source_drive.root_name().string();
    const std::string drive_relative = source_root_name + tail.generic_string();
    const fs::path missing_tail = fs::path("m") / missing_name;
    const std::string missing_drive_relative =
        source_root_name + missing_tail.generic_string();
    const bool different_drive =
        lowercase_ascii(project_dir.root_name().string()) !=
        lowercase_ascii(source_root_name);
    expect(different_drive == expect_different_drive,
           "#4065: Windows drive-relative fixture should use the requested drive relationship");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "contained_assets.pjx").string();
    const auto workspace = package_workspace(
        project_dir,
        output_dir,
        {
            {.record_index = 1U,
             .name = "main.prg",
             .relative_path = "main.prg",
             .type_title = "Program"},
            {.record_index = 2U,
             .name = "payload" + primary_extension,
             .relative_path = drive_relative,
             .type_title = type_title},
            {.record_index = 3U,
             .name = missing_name,
             .relative_path = missing_drive_relative,
             .type_title = type_title}
        });

    {
        ScopedCurrentDirectory current_directory(source_current);
        if (expect_different_drive) {
            change_current_directory(project_dir);
        }
        const auto plan = copperfin::runtime::create_runtime_package_plan(
            document,
            workspace,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            output_dir.string(),
            copperfin::runtime::BuildConfiguration::debug,
            false,
            false);
        const auto* asset = asset_by_record(plan, 2U);
        expect(plan.ok && asset != nullptr && asset->exists,
               "#4065: Windows drive-relative source should resolve from the drive current directory");
        if (asset != nullptr) {
            expect(fs::path(asset->source_path).is_absolute(),
                   "#4065: an existing drive-relative source should be frozen to an absolute identity");
            expect(read_text(asset->source_path) == "drive-current-source",
                   "#4065: Windows drive-relative source must not rebind to the PJX directory");
            expect(asset->relative_path == tail.generic_string(),
                   "#4065: package asset identity should remove the Windows drive designator");
            expect(fs::path(asset->staged_path) ==
                       (fs::path(plan.content_root) / tail).lexically_normal(),
                   "#4065: drive-relative package destination should remain under content_root");
        }
        const auto* missing_asset = asset_by_record(plan, 3U);
        expect(missing_asset != nullptr && !missing_asset->exists,
               "#4065: an unresolved drive-relative source should remain unavailable");
        if (missing_asset != nullptr) {
            expect(
                missing_asset->source_path ==
                    fs::path(missing_drive_relative).lexically_normal().string() &&
                    fs::path(missing_asset->source_path) != project_dir / missing_name,
                "#4065: an unresolved drive-relative source must not rebind to a PJX-local decoy");
        }

        change_current_directory(source_drive);
        if (expect_different_drive) {
            change_current_directory(project_dir);
        }
        const auto result = copperfin::runtime::materialize_runtime_package(
            plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            runtime_host.string());
        expect_materialization(result, "#4065: contained Windows drive-relative asset should materialize");
        expect(read_text(external_target) == "external-sentinel",
               "#4065: Windows drive-relative staging must not overwrite the drive-root target");
        expect(read_text(external_companion) == "external-companion-sentinel",
               "#4065: drive-relative companion staging must not overwrite the drive-root target");
        const auto* materialized_asset = asset_by_record(result.plan, 2U);
        const fs::path staged_primary = fs::path(result.plan.content_root) / tail;
        fs::path staged_companion = staged_primary;
        staged_companion.replace_extension(companion_extension);
        const std::string staged_primary_bytes = read_text(staged_primary);
        const std::string staged_companion_bytes = read_text(staged_companion);
        std::ostringstream failure_context;
        failure_context
            << " [fixture=" << identity
            << ", copied="
            << (materialized_asset != nullptr && materialized_asset->copied ? "true" : "false")
            << ", source="
            << (materialized_asset != nullptr ? materialized_asset->source_path : "<missing>")
            << ", staged="
            << (materialized_asset != nullptr ? materialized_asset->staged_path : "<missing>")
            << ", primaryBytes=" << staged_primary_bytes
            << ", companionBytes=" << staged_companion_bytes;
        for (const auto& warning : result.plan.warnings) {
            failure_context << ", warning=" << warning;
        }
        failure_context << ']';
        expect(staged_primary_bytes == "drive-current-source",
               "#4065: Windows drive-relative bytes should be staged inside content_root" +
                   failure_context.str());
        expect(staged_companion_bytes == "drive-current-companion",
               "#4065: drive-relative companion bytes should be staged inside content_root" +
                   failure_context.str());
    }

    fs::remove_all(project_dir, ignored);
    fs::remove_all(output_dir, ignored);
    fs::remove_all(temp_root, ignored);
}
#endif

}  // namespace

void test_fd_backed_binary_reads_accept_direct_descriptor_paths() {
#if defined(_WIN32)
    return;
#else
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() /
        ("copperfin_fd_backed_binary_read_" +
         std::to_string(static_cast<long long>(::getpid())));
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path source = root / "payload.bin";
    std::string expected;
    expected.push_back('\0');
    expected += "Copperfin";
    expected.push_back(static_cast<char>(0xFF));
    expected += "payload";
    {
        std::ofstream output(source, std::ios::binary);
        output.write(expected.data(), static_cast<std::streamsize>(expected.size()));
    }

    const int descriptor = ::open(source.c_str(), O_RDONLY | O_CLOEXEC);
    expect(descriptor >= 0, "#4371: direct FD fixture should open");
    if (descriptor >= 0) {
        const fs::path fd_path = fs::exists("/dev/fd")
            ? fs::path("/dev/fd") / std::to_string(descriptor)
            : fs::path("/proc/self/fd") / std::to_string(descriptor);
        std::string error;
        const std::string actual = copperfin::runtime::read_binary_file(fd_path, error);
        expect(actual == expected && error.empty(),
               "#4371: direct POSIX FD paths should support binary reads");
        (void)::close(descriptor);
    }
    fs::remove_all(root, ignored);
#endif
}

void test_drive_relative_asset_paths_use_contained_package_identity() {
#if defined(_WIN32)
    const fs::path user_profile(getenv_value("USERPROFILE"));
    expect(!user_profile.empty(),
           "#4065: Windows drive-relative validation requires USERPROFILE");
    if (user_profile.empty()) {
        return;
    }
    const fs::path same_drive_parent =
        user_profile / "AppData" / "Local" / "Temp" /
        "cf4065p";
    run_windows_drive_relative_case(
        same_drive_parent,
        "same_drive_report",
        ".frx",
        ".frt",
        "Report",
        false);

    const fs::path mapping_target =
        user_profile / "AppData" / "Local" / "Temp" /
        "cf4065m";
    std::error_code ignored;
    fs::remove_all(mapping_target, ignored);
    fs::create_directories(mapping_target);
    fs::path mapped_drive_root;
    const bool mapped = create_windows_drive_mapping(mapping_target, mapped_drive_root);
    expect(mapped,
           "#4065: different-drive validation should create a temporary drive mapping");
    if (mapped) {
        const fs::path different_drive_parent = mapped_drive_root / "p";
        run_windows_drive_relative_case(
            different_drive_parent,
            "different_drive_label",
            ".lbx",
            ".lbt",
            "Label",
            true);
        expect(remove_windows_drive_mapping(mapping_target, mapped_drive_root),
               "#4065: different-drive validation should remove its drive mapping");
    }
    fs::remove_all(mapping_target, ignored);
#else
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_drive_relative_host_neutral";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(project_dir / "payload.prg", "project-decoy");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "contained_assets.pjx").string();
    const auto workspace = package_workspace(
        project_dir,
        output_dir,
        {
            {.record_index = 1U,
             .name = "main.prg",
             .relative_path = "main.prg",
             .type_title = "Program"},
            {.record_index = 2U,
             .name = "payload.prg",
             .relative_path = "D:assets/payload.prg",
             .type_title = "Program"}
        });
    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);
    const auto* asset = asset_by_record(plan, 2U);
    expect(asset != nullptr && !asset->exists &&
               asset->source_path == "D:assets/payload.prg" &&
               asset->relative_path == "assets/payload.prg",
           "#4065: foreign Windows drive-relative identity should lose its drive designator");
    if (asset != nullptr) {
        expect(fs::path(asset->staged_path) ==
                   (fs::path(plan.content_root) / "assets" / "payload.prg").lexically_normal(),
               "#4065: host-neutral planning should keep a drive-relative destination contained");
    }
    fs::remove_all(temp_root, ignored);
#endif
}

void test_materialization_rejects_external_asset_destinations() {
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_external_asset_destination";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path external_target = temp_root / "external" / "sentinel.prg";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(external_target.parent_path());
    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(external_target, "external-sentinel");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "contained_assets.pjx").string();
    const auto workspace = package_workspace(
        project_dir,
        output_dir,
        {{.record_index = 1U,
          .name = "main.prg",
          .relative_path = "main.prg",
          .type_title = "Program"}});
    auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);
    expect(plan.ok && plan.assets.size() == 1U,
           "#4065: external-destination fixture plan should be available");
    if (plan.assets.size() == 1U) {
        plan.assets[0].relative_path = external_target.string();
        plan.assets[0].staged_path = external_target.string();
        plan.startup_source_path = external_target.string();
    }
    fs::create_directories(plan.package_root);
    write_text(fs::path(plan.package_root) / "previous-package.txt", "previous-package");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());
    expect(!result.ok,
           "#4065: materialization must re-admit and reject an external asset destination");
    expect(read_text(external_target) == "external-sentinel",
           "#4065: rejected external asset destination must preserve sentinel bytes");
    expect(read_text(fs::path(plan.package_root) / "previous-package.txt") ==
               "previous-package" &&
               std::distance(
                   fs::directory_iterator(plan.package_root),
                   fs::directory_iterator()) == 1,
           "#4065: rejected external asset destination should restore the exact previous package");

    fs::remove_all(temp_root, ignored);
}

void test_optional_rejected_asset_identity_stays_out_of_manifests() {
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_optional_external_asset_identity";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path external_target = temp_root / "external" / "sentinel.prg";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(external_target.parent_path());
    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(project_dir / "helper.prg", "RETURN\n");
    write_text(external_target, "external-sentinel");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "contained_assets.pjx").string();
    const auto workspace = package_workspace(
        project_dir,
        output_dir,
        {
            {.record_index = 1U,
             .name = "main.prg",
             .relative_path = "main.prg",
             .type_title = "Program"},
            {.record_index = 2U,
             .name = "helper.prg",
             .relative_path = "helper.prg",
             .type_title = "Program"}
        });
    auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);
    expect(plan.ok && plan.assets.size() == 2U,
           "#4065: optional rejected-asset fixture plan should be available");
    if (plan.assets.size() == 2U) {
        plan.assets[1].relative_path = external_target.string();
        plan.assets[1].staged_path = external_target.string();
        plan.assets[1].copied = true;
        plan.assets[1].sha256 = "stale-asset-digest";
    }
    constexpr std::string_view stale_extension_digest = "stale-extension-digest";
    constexpr std::string_view stale_data_digest = "stale-data-digest";
    plan.extension_payload_digests.push_back({
        .path = (fs::path(plan.content_root) / "stale-extension.dll").string(),
        .sha256 = std::string(stale_extension_digest)
    });
    plan.writable_data_payload_digests.push_back({
        .path = (fs::path(plan.content_root) / "stale-data.fpt").string(),
        .sha256 = std::string(stale_data_digest)
    });

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());
    expect_materialization(result,
           "#4065: a rejected optional destination should remain a package warning");
    const auto* rejected_asset = asset_by_record(result.plan, 2U);
    expect(rejected_asset != nullptr && rejected_asset->exists &&
               !rejected_asset->copied &&
               rejected_asset->relative_path == "record_2.asset" &&
               rejected_asset->staged_path.empty() && rejected_asset->sha256.empty(),
           "#4065: rejected optional asset should retain no unsafe package identity");
    expect(std::none_of(
               result.plan.extension_payload_digests.begin(),
               result.plan.extension_payload_digests.end(),
               [&](const copperfin::runtime::RuntimeArtifactDigest& digest) {
                   return digest.sha256 == stale_extension_digest;
               }) &&
               std::none_of(
                   result.plan.writable_data_payload_digests.begin(),
                   result.plan.writable_data_payload_digests.end(),
                   [&](const copperfin::runtime::RuntimeArtifactDigest& digest) {
                       return digest.sha256 == stale_data_digest;
                   }),
           "#4065: fresh materialization should discard stale payload digests");
    expect(read_text(external_target) == "external-sentinel",
           "#4065: rejected optional destination must preserve external bytes");
    if (result.ok) {
        const std::string raw_external_identity = external_target.string();
        const std::string escaped_external_identity =
            quote_manifest_value(raw_external_identity);
        const auto runtime_assets =
            lines_with_prefix(read_text(result.plan.manifest_path), "asset=");
        const auto debug_assets =
            lines_with_prefix(read_text(result.plan.debug_manifest_path), "asset=");
        const auto identity_is_safe = [&](const std::string& line) {
            return line.find(raw_external_identity) == std::string::npos &&
                line.find(escaped_external_identity) == std::string::npos;
        };
        expect(std::all_of(
                   runtime_assets.begin(),
                   runtime_assets.end(),
                   identity_is_safe) &&
                   std::all_of(
                       debug_assets.begin(),
                       debug_assets.end(),
                       identity_is_safe),
               "#4065: rejected optional identity must not enter manifest asset records");
        const std::string ast_manifest = read_text(result.plan.ast_manifest_path);
        const std::string ir_manifest = read_text(result.plan.ir_manifest_path);
        expect(identity_is_safe(ast_manifest) && identity_is_safe(ir_manifest) &&
                   ast_manifest.find("record_2.asset") == std::string::npos &&
                   ir_manifest.find("record_2.asset") == std::string::npos,
               "#4065: rejected optional identity must not enter AST or IR JSON contracts");
    }

    fs::remove_all(temp_root, ignored);
}

void test_materialization_rejects_external_content_root() {
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_external_content_root";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path external_root = temp_root / "external-content";
    const fs::path external_target = external_root / "main.prg";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(external_root);
    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(external_target, "external-sentinel");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "contained_assets.pjx").string();
    const auto workspace = package_workspace(
        project_dir,
        output_dir,
        {{.record_index = 1U,
          .name = "main.prg",
          .relative_path = "main.prg",
          .type_title = "Program"}});
    auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);
    fs::create_directories(plan.package_root);
    write_text(fs::path(plan.package_root) / "previous-package.txt", "previous-package");
    plan.content_root = external_root.string();
    plan.assets[0].staged_path = external_target.string();
    plan.startup_source_path = external_target.string();

    ScopedEnvironmentVariable locale("COPPERFIN_LOCALE", "en-US");
    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());
    expect(!result.ok,
           "#4065: materialization must reject a content root outside package_root");
    expect(result.error ==
               runtime_pipeline_english_catalog().translate(
                   "Runtime.Package.Error.ContentRootRejected",
                   {{"path", external_root.string()}}),
           "#4065: external content root should emit the localized root diagnostic");
    expect(read_text(external_target) == "external-sentinel",
           "#4065: rejected external content root must preserve external bytes");
    expect(read_text(fs::path(plan.package_root) / "previous-package.txt") ==
               "previous-package" &&
               std::distance(
                   fs::directory_iterator(plan.package_root),
                   fs::directory_iterator()) == 1,
           "#4065: external-content-root rejection should restore the exact previous package");

    fs::remove_all(temp_root, ignored);
}

void test_package_content_copy_rejects_indirect_parent() {
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_indirect_package_content_parent";
    const fs::path content_root = temp_root / "package" / "content";
    const fs::path external_root = temp_root / "external";
    const fs::path external_sentinel = external_root / "sentinel.txt";
    const fs::path source = temp_root / "source.prg";
    const fs::path redirect = content_root / "redirect";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    {
        const fs::path missing_package = temp_root / "missing-package";
        ScopedEnvironmentVariable locale("COPPERFIN_LOCALE", "en-US");
        std::string error;
        const bool prepared =
            copperfin::runtime::runtime_pipeline_detail::prepare_package_content_root(
                missing_package,
                missing_package / "content",
                error);
        expect(!prepared &&
                   error == runtime_pipeline_english_catalog().translate(
                       "Runtime.Package.Error.CreateContentRootFailed"),
               "#4065: an unavailable package root should retain the operational diagnostic");
    }
    fs::create_directories(content_root);
    fs::create_directories(external_root);
    write_text(external_sentinel, "external-sentinel");
    write_text(source, "package-source");

    {
        ScopedEnvironmentVariable locale("COPPERFIN_LOCALE", "en-US");
        const std::vector<std::string> rejected_paths = {
            "D:payload.prg",
            "payload.prg:stream",
            "CON.txt",
            std::string("com") + "\xC2\xB9",
            std::string("lpt") + "\xC2\xB2.txt"
        };
        for (const std::string& rejected_path : rejected_paths) {
            fs::path rejected_destination;
            std::string error;
                const bool copied =
                    copperfin::runtime::runtime_pipeline_detail::copy_file_to_package_content(
                        source,
                        temp_root / "package",
                        content_root,
                        copperfin::platform::path_from_utf8_string(rejected_path),
                        rejected_destination,
                        error);
            expect(!copied && rejected_destination.empty(),
                   "#4065: package content admission should reject Windows path aliases");
        }
    }

#if defined(_WIN32)
    {
        const fs::path case_alias_root = temp_root / "package" / "CONTENT";
        fs::path destination;
        std::string error;
        const bool copied =
            copperfin::runtime::runtime_pipeline_detail::copy_file_to_package_content(
                source,
                temp_root / "package",
                case_alias_root,
                "case-alias.prg",
                destination,
                error);
        expect(copied && read_text(content_root / "case-alias.prg") == "package-source",
               "#4065: Windows content-root aliases should compare case-insensitively");
        fs::remove(content_root / "case-alias.prg", ignored);
    }
    const bool redirect_created = create_windows_junction(redirect, external_root);
#else
    std::error_code link_error;
    fs::create_directory_symlink(external_root, redirect, link_error);
    const bool redirect_created = !link_error;
#endif
    expect(redirect_created,
           "#4065: physical-containment validation should create an indirect parent fixture");
    if (redirect_created) {
        ScopedEnvironmentVariable locale("COPPERFIN_LOCALE", "en-US");
        fs::path destination;
        std::string error;
        const bool copied =
            copperfin::runtime::runtime_pipeline_detail::copy_file_to_package_content(
                source,
                temp_root / "package",
                content_root,
                fs::path("redirect") / "payload.prg",
                destination,
                error);
        expect(!copied,
               "#4065: package content copy should reject a redirected parent");
        expect(error ==
                   runtime_pipeline_english_catalog().translate(
                       "Runtime.Package.Error.ContentDestinationRejected",
                       {{"path", (content_root / "redirect" / "payload.prg").string()}}),
               "#4065: redirected-parent rejection should use the localized diagnostic");
        expect(read_text(external_sentinel) == "external-sentinel" &&
                   !fs::exists(external_root / "payload.prg"),
               "#4065: redirected package staging must not modify the external directory");
    }

    fs::remove_all(temp_root, ignored);
}

void test_package_content_copy_rejects_hard_link_destination() {
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_hard_link_package_content_destination";
    const fs::path package_root = temp_root / "package";
    const fs::path content_root = package_root / "content";
    const fs::path external_target = temp_root / "external.prg";
    const fs::path destination = content_root / "payload.prg";
    const fs::path source = temp_root / "source.prg";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);
    write_text(external_target, "external-sentinel");
    write_text(source, "package-source");
    fs::create_hard_link(external_target, destination, ignored);
    expect(!ignored,
           "#4065: hard-link destination validation should create its fixture");
    if (!ignored) {
        ScopedEnvironmentVariable locale("COPPERFIN_LOCALE", "en-US");
        fs::path admitted_destination;
        std::string error;
        const bool copied =
            copperfin::runtime::runtime_pipeline_detail::copy_file_to_package_content(
                source,
                package_root,
                content_root,
                "payload.prg",
                admitted_destination,
                error);
        expect(!copied,
               "#4065: package content copy should reject a hard-link destination");
        expect(read_text(external_target) == "external-sentinel",
               "#4065: rejected hard-link staging must preserve external bytes");
    }

    fs::remove_all(temp_root, ignored);
}

void test_windows_nested_package_parent_rebind_fails_closed() {
#if defined(_WIN32)
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_nested_package_parent_rebind";
    const fs::path package_root = temp_root / "package";
    const fs::path content_root = package_root / "content";
    const fs::path nested_parent = content_root / "safe" / "nested";
    const fs::path moved_parent = temp_root / "safe-moved";
    const fs::path external_root = temp_root / "external";
    const fs::path external_sentinel = external_root / "sentinel.txt";
    const fs::path source = temp_root / "source.prg";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(nested_parent);
    fs::create_directories(external_root);
    write_text(external_sentinel, "external-sentinel");
    write_text(source, "package-source");

    copperfin::runtime::test_hooks::arm_package_content_parent_open_pause();
    bool copied = false;
    fs::path destination;
    std::string error;
    std::thread writer([&] {
        copied = copperfin::runtime::runtime_pipeline_detail::copy_file_to_package_content(
            source,
            package_root,
            content_root,
            fs::path("safe") / "nested" / "payload.prg",
            destination,
            error);
    });
    const bool pause_entered =
        copperfin::runtime::test_hooks::wait_for_package_content_parent_open_pause();
    expect(pause_entered,
           "#4340: nested-parent race fixture should reach the post-validation barrier");
    if (pause_entered) {
        fs::rename(content_root / "safe", moved_parent, ignored);
        expect(!ignored,
               "#4340: nested-parent race fixture should move the validated parent");
        if (!ignored) {
            const bool junction_created = create_windows_junction(
                content_root / "safe",
                external_root);
            expect(junction_created,
                   "#4340: nested-parent race fixture should install a replacement junction");
        }
    }
    copperfin::runtime::test_hooks::release_package_content_parent_open_pause();
    writer.join();
    expect(!copied,
           "#4340: a replaced nested package parent should fail closed");
    expect(read_text(external_sentinel) == "external-sentinel" &&
               !fs::exists(external_root / "nested" / "payload.prg"),
           "#4340: nested-parent replacement must not modify external files");
    fs::remove_all(temp_root, ignored);
#else
    return;
#endif
}

void test_relative_output_root_preserves_plan_path_contract() {
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_relative_output_content_paths";
    const fs::path project_dir = temp_root / "project";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    {
        ScopedCurrentDirectory current_directory(temp_root);
        copperfin::studio::StudioDocumentModel document;
        document.path = (project_dir / "contained_assets.pjx").string();
        const auto workspace = package_workspace(
            project_dir,
            "relative-output",
            {{.record_index = 1U,
              .name = "main.prg",
              .relative_path = "main.prg",
              .type_title = "Program"}});
        const auto plan = copperfin::runtime::create_runtime_package_plan(
            document,
            workspace,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            "relative-output",
            copperfin::runtime::BuildConfiguration::debug,
            false,
            false);
        expect(!fs::path(plan.content_root).is_absolute(),
               "#4065: relative-output fixture should retain a relative content root");
        const auto result = copperfin::runtime::materialize_runtime_package(
            plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            runtime_host.string());
        expect_materialization(result,
               "#4065: package staging should support a relative output root");
        const fs::path expected_staged =
            (fs::path(plan.content_root) / "main.prg").lexically_normal();
        expect(result.ok && !result.plan.assets.empty() &&
                   result.plan.assets[0].staged_path == expected_staged.string() &&
                   result.plan.startup_source_path == expected_staged.string() &&
                   !fs::path(result.plan.assets[0].staged_path).is_absolute(),
               "#4065: contained copy should preserve relative staged-path contracts");
        if (result.ok) {
            const std::string manifest = read_text(result.plan.manifest_path);
            expect(manifest.find(
                       "startup_source=" + quote_manifest_value(expected_staged.string())) !=
                       std::string::npos,
                   "#4065: runtime manifest should preserve relative startup-source spelling");
        }
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_runtime_pipeline

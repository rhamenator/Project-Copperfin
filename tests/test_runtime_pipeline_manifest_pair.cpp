// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_pipeline_support.h"

#include "../src/runtime/runtime_pipeline_support.h"

#include "runtime_pipeline_test_hooks.h"
#include "runtime_pipeline_manifest_pair_io.h"

#if !defined(_WIN32)
#include <fcntl.h>
#include <unistd.h>
#endif

#include <thread>

namespace cf_test_runtime_pipeline {
namespace {

namespace fs = std::filesystem;

struct ManifestPairFixture {
    fs::path root;
    fs::path runtime_host;
    copperfin::runtime::RuntimePackagePlan plan;
    std::string runtime_before;
    std::string debug_before;
    bool ready = false;
};

ManifestPairFixture prepare_manifest_pair_fixture(const std::string& identity) {
    ManifestPairFixture fixture;
    fixture.root = fs::temp_directory_path() / ("copperfin_manifest_pair_" + identity);
    const fs::path project_dir = fixture.root / "project";
    const fs::path output_dir = fixture.root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(fixture.root);
    fixture.runtime_host = runtime_host;
    std::error_code ignored;
    fs::remove_all(fixture.root, ignored);
    fs::create_directories(project_dir);
    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "manifest_pair.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ManifestPair";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "ManifestPair";
    workspace.build_plan.output_path =
        (output_dir / "Copperfin.GeneratedLauncher.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U,
         .name = "main.prg",
         .relative_path = "main.prg",
         .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);
    expect(plan.ok, "#4056: manifest-pair fixture plan should be created");
    if (!plan.ok) {
        return fixture;
    }

    const auto materialized = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());
    expect(materialized.ok, "#4056: manifest-pair fixture package should materialize");
    if (!materialized.ok) {
        return fixture;
    }

    fixture.plan = materialized.plan;
    const fs::path package_root(fixture.plan.package_root);
    write_text(fixture.plan.launcher_output_path, "published-launcher");
    write_text(package_root / "Copperfin.GeneratedLauncher.dll", "launcher-dll");
    write_text(package_root / "Copperfin.GeneratedLauncher.deps.json", "launcher-deps");
    write_text(
        package_root / "Copperfin.GeneratedLauncher.runtimeconfig.json",
        "launcher-runtimeconfig");
    fixture.runtime_before = read_text(fixture.plan.manifest_path);
    fixture.debug_before = read_text(fixture.plan.debug_manifest_path);
    fixture.ready = true;
    return fixture;
}

bool has_manifest_pair_transaction_artifacts(const fs::path& package_root) {
    std::error_code error;
    for (fs::directory_iterator it(package_root, error), end; !error && it != end;
         it.increment(error)) {
        if (it->path().filename().string().find(".copperfin-manifest-pair") !=
            std::string::npos) {
            return true;
        }
    }
    return error || false;
}

copperfin::runtime::RuntimeBuildResult finalize_manifest_pair(
    const copperfin::runtime::RuntimePackagePlan& plan) {
    return copperfin::runtime::finalize_runtime_package_primary_output(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile());
}

void remove_fixture(const ManifestPairFixture& fixture) {
    std::error_code ignored;
    fs::remove_all(fixture.root, ignored);
}

}  // namespace

void test_manifest_pair_finalization_rejects_redirected_destinations() {
    auto fixture = prepare_manifest_pair_fixture("redirected");
    if (!fixture.ready) {
        remove_fixture(fixture);
        return;
    }

    const fs::path manifest_path(fixture.plan.manifest_path);
    const fs::path external_manifest = fixture.root / "external.cfmanifest";
    write_text(external_manifest, "external-original");
    std::error_code link_error;
    fs::remove(manifest_path, link_error);
    link_error.clear();
    fs::create_symlink(external_manifest, manifest_path, link_error);
    if (!link_error) {
        const auto redirected = finalize_manifest_pair(fixture.plan);
        expect(!redirected.ok,
               "#4056: finalization should reject a redirected runtime manifest leaf");
        expect(read_text(external_manifest) == "external-original",
               "#4056: redirected finalization must not modify the external target");
        expect(read_text(fixture.plan.debug_manifest_path) == fixture.debug_before,
               "#4056: redirected finalization must preserve the debug manifest");
        fs::remove(manifest_path, link_error);
    }
    write_text(manifest_path, fixture.runtime_before);

#if defined(_WIN32)
    const fs::path reparse_target = fixture.root / "windows-reparse-target";
    fs::create_directories(reparse_target);
    write_text(reparse_target / "sentinel.txt", "external-reparse-sentinel");
    fs::remove(manifest_path, link_error);
    const bool junction_created = create_windows_junction(manifest_path, reparse_target);
    expect(junction_created,
           "#4056: Windows validation must create a deterministic reparse fixture");
    if (junction_created) {
        const auto redirected_reparse = finalize_manifest_pair(fixture.plan);
        expect(!redirected_reparse.ok,
               "#4056: finalization should reject a Windows reparse-backed manifest leaf");
        expect(read_text(reparse_target / "sentinel.txt") ==
                   "external-reparse-sentinel" &&
                   read_text(fixture.plan.debug_manifest_path) == fixture.debug_before,
               "#4056: Windows reparse rejection must preserve external data and the debug manifest");
        fs::remove(manifest_path, link_error);
    }
    write_text(manifest_path, fixture.runtime_before);
#endif

    const fs::path debug_path(fixture.plan.debug_manifest_path);
    const fs::path external_debug = fixture.root / "external.cfdebug";
    write_text(external_debug, "external-debug-original");
    link_error.clear();
    fs::remove(debug_path, link_error);
    link_error.clear();
    fs::create_symlink(external_debug, debug_path, link_error);
    if (!link_error) {
        const auto redirected_debug = finalize_manifest_pair(fixture.plan);
        expect(!redirected_debug.ok,
               "#4056: finalization should reject a redirected debug manifest leaf");
        expect(read_text(external_debug) == "external-debug-original" &&
                   read_text(fixture.plan.manifest_path) == fixture.runtime_before,
               "#4056: redirected debug finalization must preserve both reachable files");
        fs::remove(debug_path, link_error);
    }
    write_text(debug_path, fixture.debug_before);

    const fs::path external_destination = fixture.root / "mutated.cfmanifest";
    write_text(external_destination, "external-plan-target");
    auto external_plan = fixture.plan;
    external_plan.manifest_path = external_destination.string();
    const auto outside = finalize_manifest_pair(external_plan);
    expect(!outside.ok,
           "#4056: finalization should reject a manifest destination outside the package root");
    expect(read_text(external_destination) == "external-plan-target" &&
               read_text(fixture.plan.debug_manifest_path) == fixture.debug_before,
           "#4056: external-plan rejection must preserve both reachable files");

    const fs::path external_directory = fixture.root / "external-directory";
    const fs::path redirected_directory = fs::path(fixture.plan.package_root) / "redirected";
    fs::create_directories(external_directory);
    link_error.clear();
    fs::create_directory_symlink(external_directory, redirected_directory, link_error);
    if (!link_error) {
        const fs::path nested_external = external_directory / "app.cfmanifest";
        write_text(nested_external, "nested-external-target");
        auto component_plan = fixture.plan;
        component_plan.manifest_path =
            (redirected_directory / "app.cfmanifest").string();
        const auto redirected_component = finalize_manifest_pair(component_plan);
        expect(!redirected_component.ok,
               "#4056: finalization should reject an indirect component below the package root");
        expect(read_text(nested_external) == "nested-external-target",
               "#4056: component rejection must not modify its external target");
    }

    const fs::path unowned_stage =
        fixture.plan.manifest_path + std::string(".copperfin-manifest-pair-next");
    write_text(unowned_stage, "unowned-stage-collision");
    const auto unowned_collision = finalize_manifest_pair(fixture.plan);
    expect(!unowned_collision.ok,
           "#4056: finalization should reject an unowned reserved-path collision");
    expect(read_text(unowned_stage) == "unowned-stage-collision" &&
               read_text(fixture.plan.manifest_path) == fixture.runtime_before &&
               read_text(fixture.plan.debug_manifest_path) == fixture.debug_before,
           "#4056: collision rejection should preserve the reserved entry and old pair");
    fs::remove(unowned_stage, link_error);

    const fs::path unowned_backup =
        fixture.plan.debug_manifest_path +
        std::string(".copperfin-manifest-pair-previous");
    write_text(unowned_backup, "unowned-backup-collision");
    const auto backup_collision = finalize_manifest_pair(fixture.plan);
    expect(!backup_collision.ok &&
               read_text(unowned_backup) == "unowned-backup-collision",
           "#4056: finalization should preserve and reject an unowned backup collision");
    fs::remove(unowned_backup, link_error);

    const fs::path foreign_marker =
        fs::path(fixture.plan.package_root) / ".copperfin-manifest-pair.transaction";
    write_text(foreign_marker, "foreign-transaction-marker\n");
    const auto marker_collision = finalize_manifest_pair(fixture.plan);
    expect(!marker_collision.ok &&
               read_text(foreign_marker) == "foreign-transaction-marker\n" &&
               read_text(fixture.plan.manifest_path) == fixture.runtime_before &&
               read_text(fixture.plan.debug_manifest_path) == fixture.debug_before,
           "#4056: finalization should preserve and reject a foreign transaction journal");
    fs::remove(foreign_marker, link_error);

    expect(!has_manifest_pair_transaction_artifacts(fixture.plan.package_root),
           "#4056: path-admission failures should not leave transaction artifacts");
    remove_fixture(fixture);
}

void test_manifest_pair_directory_stays_pinned_and_never_overwrites() {
    const fs::path root =
        fs::temp_directory_path() / "copperfin_manifest_pair_pinned_root";
    const fs::path moved_root = root.string() + "-moved";
    const fs::path external_root = root.string() + "-external";
    std::error_code error;
    fs::remove_all(root, error);
    fs::remove_all(moved_root, error);
    fs::remove_all(external_root, error);
    fs::create_directories(root);
    fs::create_directories(external_root);

#if !defined(_WIN32)
    const int parent_descriptor = ::open(
        root.parent_path().c_str(),
        O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    expect(parent_descriptor >= 0,
           "#4257: descriptor-backed manifest-pair fixture should open its parent");
    if (parent_descriptor >= 0) {
        const fs::path fd_root = fs::path("/dev/fd") /
            std::to_string(parent_descriptor) /
            root.filename();
        {
            copperfin::runtime::runtime_pipeline_detail::ManifestPairDirectory directory;
            const bool acquired = directory.acquire(fd_root, "fd-backed-pinned-root-regression");
            expect(acquired,
                   "#4257: manifest-pair acquisition should walk a descriptor-backed package path");
            if (acquired) {
                fs::rename(root, moved_root, error);
                expect(!error,
                       "#4257: descriptor-backed fixture should rename its visible package path");
                fs::create_directory_symlink(external_root, root, error);
                expect(!error,
                       "#4257: descriptor-backed fixture should replace its visible path");
                expect(
                    directory.create_direct_file_and_flush("fd-source", "fd-generation") &&
                        read_text(moved_root / "fd-source") == "fd-generation" &&
                        !fs::exists(external_root / "fd-source"),
                    "#4257: descriptor-backed manifest-pair writes must stay in the admitted package root");
            }
        }
        (void)::close(parent_descriptor);
        fs::remove_all(root, error);
        fs::remove_all(moved_root, error);
        fs::remove_all(external_root, error);
        fs::create_directories(root);
        fs::create_directories(external_root);
    }
#endif

    {
        copperfin::runtime::runtime_pipeline_detail::ManifestPairDirectory directory;
        const bool acquired = directory.acquire(root, "pinned-root-regression");
        expect(acquired, "#4056: direct-file test should pin the package root");
        if (!acquired) {
            return;
        }

#if defined(_WIN32)
        fs::rename(root, moved_root, error);
        expect(
            static_cast<bool>(error),
            "#4056: Windows package-root handle should prevent path rebinding");
        error.clear();
        const fs::path active_root = root;
#else
        fs::rename(root, moved_root, error);
        expect(!error, "#4056: POSIX root-rebinding fixture should rename the visible path");
        fs::create_directory_symlink(external_root, root, error);
        expect(!error, "#4056: POSIX root-rebinding fixture should replace the visible path");
        const fs::path active_root = moved_root;
#endif

        expect(
            directory.create_direct_file_and_flush("source", "source-generation") &&
                read_text(active_root / "source") == "source-generation" &&
                !fs::exists(external_root / "source"),
            "#4056: pinned directory writes must stay in the originally admitted package root");
        write_text(active_root / "destination", "existing-destination");
        expect(
            !directory.move_direct_file_no_replace("source", "destination") &&
                read_text(active_root / "source") == "source-generation" &&
                read_text(active_root / "destination") == "existing-destination",
            "#4056: direct-file promotion must never replace an existing destination");
        expect(
            directory.remove_direct_file("source") &&
                directory.remove_direct_file("destination"),
            "#4056: pinned direct-file cleanup should remove owned regular leaves");
    }

    fs::remove_all(root, error);
    fs::remove_all(moved_root, error);
    fs::remove_all(external_root, error);
}

void test_manifest_pair_finalization_rolls_back_failed_promotions() {
    using copperfin::runtime::test_hooks::ManifestPairPromotionFault;
    for (const auto& scenario : {
             std::pair{std::string("first"), ManifestPairPromotionFault::before_first_promotion},
             std::pair{std::string("second"), ManifestPairPromotionFault::before_second_promotion}}) {
        auto fixture = prepare_manifest_pair_fixture("rollback_" + scenario.first);
        if (!fixture.ready) {
            remove_fixture(fixture);
            continue;
        }

        copperfin::runtime::test_hooks::set_manifest_pair_promotion_fault(scenario.second);
        const auto failed = finalize_manifest_pair(fixture.plan);
        copperfin::runtime::test_hooks::set_manifest_pair_promotion_fault(
            ManifestPairPromotionFault::none);
        expect(!failed.ok,
               "#4056: injected " + scenario.first + " promotion failure should fail finalization");
        expect(read_text(fixture.plan.manifest_path) == fixture.runtime_before &&
                   read_text(fixture.plan.debug_manifest_path) == fixture.debug_before,
               "#4056: " + scenario.first +
                   " promotion failure should restore the exact previous manifest pair");
        expect(!has_manifest_pair_transaction_artifacts(fixture.plan.package_root),
               "#4056: " + scenario.first +
                   " promotion rollback should remove all transaction artifacts");

        const auto retry = finalize_manifest_pair(fixture.plan);
        expect(retry.ok,
               "#4056: finalization should remain retryable after " + scenario.first +
                   " promotion rollback");
        remove_fixture(fixture);
    }
}

void test_manifest_pair_finalization_recovers_stale_transactions() {
    auto fixture = prepare_manifest_pair_fixture("stale");
    if (!fixture.ready) {
        remove_fixture(fixture);
        return;
    }

    std::string seed_error;
    const bool seeded = copperfin::runtime::test_hooks::seed_stale_manifest_pair_transaction(
        fixture.plan,
        "stale-runtime-generation\n",
        "stale-debug-generation\n",
        seed_error);
    expect(seeded, "#4056: stale transaction fixture should be seeded: " + seed_error);
    expect(has_manifest_pair_transaction_artifacts(fixture.plan.package_root),
           "#4056: stale transaction fixture should leave owned artifacts");

#if defined(_WIN32)
    {
        const fs::path package_root(fixture.plan.package_root);
        std::string alias_leaf = package_root.filename().string();
        for (char& character : alias_leaf) {
            if (character >= 'A' && character <= 'Z') {
                character = static_cast<char>(character - 'A' + 'a');
            } else if (character >= 'a' && character <= 'z') {
                character = static_cast<char>(character - 'a' + 'A');
            }
        }
        const fs::path alias_root = package_root.parent_path() / alias_leaf;
        auto alias_plan = fixture.plan;
        alias_plan.package_root = alias_root.string();
        const auto rebase_package_path = [&](std::string& value) {
            if (value.empty()) {
                return;
            }
            const fs::path path(value);
            if (path == package_root) {
                value = alias_root.string();
                return;
            }
            const fs::path relative = path.lexically_relative(package_root);
            if (relative.empty() || relative == "." || relative.is_absolute() ||
                relative.begin()->string() == "..") {
                return;
            }
            value = (alias_root / relative).string();
        };
        for (std::string* path : {
                 &alias_plan.content_root,
                 &alias_plan.manifest_path,
                 &alias_plan.debug_manifest_path,
                 &alias_plan.ast_manifest_path,
                 &alias_plan.ir_manifest_path,
                 &alias_plan.transpiled_csharp_path,
                 &alias_plan.launcher_project_path,
                 &alias_plan.launcher_source_path,
                 &alias_plan.launcher_output_path,
                 &alias_plan.module_definition_path,
                 &alias_plan.native_wrapper_source_path,
                 &alias_plan.native_wrapper_cmake_path,
                 &alias_plan.native_wrapper_build_script_path,
                 &alias_plan.native_wrapper_build_powershell_path,
                 &alias_plan.library_api_manifest_path,
                 &alias_plan.fll_api_manifest_path,
                 &alias_plan.fxp_token_manifest_path,
                 &alias_plan.app_archive_manifest_path,
                 &alias_plan.runtime_host_destination_path,
                 &alias_plan.audit_log_path}) {
            rebase_package_path(*path);
        }

        const std::string canonical_root =
            copperfin::runtime::runtime_pipeline_detail::canonical_casefolded_path_identity(
                package_root);
        const std::string canonical_alias_root =
            copperfin::runtime::runtime_pipeline_detail::canonical_casefolded_path_identity(
                alias_root);
        expect(canonical_root == canonical_alias_root,
               "#4098: Windows casing-only package aliases should share a canonical root identity");

        const auto identity_input_for = [&](const fs::path& root) {
            return copperfin::runtime::runtime_pipeline_detail::canonical_casefolded_path_identity(
                       root) + '\0' +
                copperfin::runtime::runtime_pipeline_detail::canonical_casefolded_path_identity(
                    root / fs::path(fixture.plan.manifest_path).filename()) + '\0' +
                copperfin::runtime::runtime_pipeline_detail::canonical_casefolded_path_identity(
                    root / fs::path(fixture.plan.debug_manifest_path).filename());
        };
        const std::string identity_input = identity_input_for(package_root);
        expect(identity_input == identity_input_for(alias_root),
               "#4098: Windows casing-only aliases should share the full manifest-pair identity input");
        const auto identity_hash = copperfin::security::sha256_hex_for_text(identity_input);
        expect(identity_hash.ok,
               "#4098: Windows manifest-pair alias fixture should derive its identity hash");
        if (identity_hash.ok) {
            {
                copperfin::runtime::runtime_pipeline_detail::ManifestPairDirectory first_directory;
                expect(first_directory.acquire(package_root, identity_hash.hex_digest),
                       "#4098: Windows manifest-pair fixture should acquire the first alias lock");
                bool alias_acquired = false;
                std::thread alias_attempt([&] {
                    copperfin::runtime::runtime_pipeline_detail::ManifestPairDirectory alias_directory;
                    alias_acquired = alias_directory.acquire(alias_root, identity_hash.hex_digest);
                });
                alias_attempt.join();
                expect(!alias_acquired,
                       "#4098: Windows casing-only aliases should serialize concurrent manifest-pair finalization");
            }
        }

        const auto recovered_alias = finalize_manifest_pair(alias_plan);
        expect(recovered_alias.ok,
               "#4098: Windows casing-only alias should recover the stale manifest-pair journal: " +
                   recovered_alias.error);
        expect(!has_manifest_pair_transaction_artifacts(alias_root),
               "#4098: Windows casing-only stale recovery should remove manifest-pair artifacts");
    }
#endif

    const auto recovered = finalize_manifest_pair(fixture.plan);
    expect(recovered.ok,
           "#4056: finalization should recover and replace an owned stale transaction: " +
               recovered.error);
    expect(!has_manifest_pair_transaction_artifacts(fixture.plan.package_root),
           "#4056: stale transaction recovery should remove every temporary artifact");
    if (recovered.ok) {
        const std::string runtime_manifest = read_text(recovered.plan.manifest_path);
        const std::string debug_manifest = read_text(recovered.plan.debug_manifest_path);
        expect(runtime_manifest.find("manifest_version=3") != std::string::npos &&
                   debug_manifest.find("debug_manifest_version=3") != std::string::npos &&
                   debug_manifest.find("primary_output_materialized=true") != std::string::npos,
               "#4056: recovery should publish current invariant manifest schemas and status fields");
        expect(
            runtime_manifest.find(
                "package_root=" + quote_manifest_value(recovered.plan.package_root)) !=
                std::string::npos &&
                runtime_manifest.find(
                    "startup_item=" + quote_manifest_value(recovered.plan.startup_item)) !=
                    std::string::npos &&
                debug_manifest.find(
                    "project_path=" + quote_manifest_value(recovered.plan.project_path)) !=
                    std::string::npos &&
                debug_manifest.find(
                    "working_directory=" +
                    quote_manifest_value(recovered.plan.debug_plan.working_directory)) !=
                    std::string::npos,
            "#4056: stale recovery should preserve live-plan path and startup identities");
        expect(runtime_manifest != "stale-runtime-generation\n" &&
                   debug_manifest != "stale-debug-generation\n",
               "#4056: recovery must not publish abandoned staged generations");
    }

    remove_fixture(fixture);
}

void test_materialize_cleanup_warning_rewrites_manifest_pair_atomically() {
    auto fixture = prepare_manifest_pair_fixture("cleanup_warning");
    if (!fixture.ready) {
        remove_fixture(fixture);
        return;
    }

    copperfin::runtime::test_hooks::force_package_backup_cleanup_warning_once();
    const auto rematerialized = copperfin::runtime::materialize_runtime_package(
        fixture.plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        fixture.runtime_host.string());
    expect(rematerialized.ok,
           "#4056: cleanup-warning materialization should preserve a valid package");
    if (rematerialized.ok) {
        const std::string runtime_manifest = read_text(rematerialized.plan.manifest_path);
        const std::string debug_manifest = read_text(rematerialized.plan.debug_manifest_path);
        expect(!rematerialized.plan.warnings.empty() &&
                   runtime_manifest.find("warning=") != std::string::npos &&
                   debug_manifest.find("warning=") != std::string::npos,
               "#4056: cleanup warnings should be published to both manifest generations");
        expect(runtime_manifest.find("manifest_version=3") != std::string::npos &&
                   debug_manifest.find("debug_manifest_version=3") != std::string::npos &&
                   debug_manifest.find("primary_output_materialized=true") != std::string::npos,
               "#4056: cleanup-warning rewrites should preserve current schemas and status");
        expect(!has_manifest_pair_transaction_artifacts(rematerialized.plan.package_root),
               "#4056: cleanup-warning rewrites should remove manifest-pair artifacts");
    }

    copperfin::runtime::test_hooks::force_package_backup_cleanup_warning_once();
    copperfin::runtime::test_hooks::set_manifest_pair_promotion_fault(
        copperfin::runtime::test_hooks::ManifestPairPromotionFault::before_first_promotion);
    const auto live_package_with_stale_manifests =
        copperfin::runtime::materialize_runtime_package(
            fixture.plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            fixture.runtime_host.string());
    copperfin::runtime::test_hooks::set_manifest_pair_promotion_fault(
        copperfin::runtime::test_hooks::ManifestPairPromotionFault::none);
    expect(live_package_with_stale_manifests.ok,
           "#4096: a post-commit manifest rewrite failure should remain warning-only");
    expect(live_package_with_stale_manifests.error.empty(),
           "#4096: warning-only manifest rewrite failure should not expose a materialization error");
    expect(
        live_package_with_stale_manifests.ok &&
            fs::exists(fs::path(live_package_with_stale_manifests.plan.package_root) /
                       "content" / "main.prg"),
        "#4096: warning-only manifest rewrite failure should leave the new package live");
    expect(
        !live_package_with_stale_manifests.plan.warnings.empty() &&
            live_package_with_stale_manifests.plan.warnings.back().find(
                "manifest") != std::string::npos,
        "#4096: warning-only manifest rewrite failure should identify the stale manifest pair");
    expect(!has_manifest_pair_transaction_artifacts(
               live_package_with_stale_manifests.plan.package_root),
           "#4096: failed manifest rewrite recovery should leave no transaction artifacts");

    remove_fixture(fixture);
}

void test_write_text_file_reports_close_failure() {
#if defined(__linux__)
    if (!fs::exists("/dev/full")) {
        return;
    }

    std::string error;
    const bool result = copperfin::runtime::runtime_pipeline_detail::write_text_file(
        fs::path("/dev/full"),
        "close-time failure",
        error);
    expect(!result,
           "#4304: text-file writes should fail when the destination reports a close-time write error");
    expect(!error.empty(),
           "#4304: close-time text-file failures should preserve a localized write diagnostic");
#endif
}

}  // namespace cf_test_runtime_pipeline

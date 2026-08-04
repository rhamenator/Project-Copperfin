// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_pipeline_support.h"

#include "../src/runtime/runtime_pipeline_test_hooks.h"

#include <map>
#include <thread>

namespace cf_test_runtime_pipeline {
namespace {

using PackageSnapshot = std::map<std::string, std::string>;

PackageSnapshot snapshot_package_files(const std::filesystem::path& package_root) {
    PackageSnapshot snapshot;
    std::error_code error;
    for (std::filesystem::recursive_directory_iterator iterator(package_root, error), end;
         iterator != end && !error;
         iterator.increment(error)) {
        if (!iterator->is_regular_file(error)) {
            continue;
        }
        const std::string relative_path =
            iterator->path().lexically_relative(package_root).generic_string();
        snapshot[relative_path] = read_text(iterator->path());
    }
    expect(!error, "package snapshot should enumerate the complete generated tree");
    return snapshot;
}

copperfin::runtime::RuntimePackagePlan create_rematerialization_plan(
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

copperfin::runtime::RuntimeMaterializeResult materialize_rematerialization_plan(
    const copperfin::runtime::RuntimePackagePlan& plan,
    const std::filesystem::path& runtime_host) {
    return copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());
}

}  // namespace

void test_repeated_materialization_replaces_generated_package_transactionally() {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_pipeline_rematerialization";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    const fs::path unrelated_output = output_dir / "caller-owned.txt";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);

    write_text(project_dir / "startup.scx", "form-v1");
    write_text(project_dir / "startup.sct", "form-memo-v1");
    write_text(project_dir / "old_name.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");
    write_text(unrelated_output, "caller-owned");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "rematerialize.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "RematerializeDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.output_path = (output_dir / "RematerializeDemo.app").string();
    workspace.build_plan.output_kind = "app";
    workspace.build_plan.startup_item = "startup.scx";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "startup.scx", .relative_path = "startup.scx", .type_title = "Form"},
        {.record_index = 2U, .name = "old_name.prg", .relative_path = "old_name.prg", .type_title = "Program"}
    };

    const auto first_plan = create_rematerialization_plan(document, workspace, output_dir);
    const auto first_result = materialize_rematerialization_plan(first_plan, runtime_host);
    expect(first_result.ok, "initial rematerialization fixture package should materialize");
    if (!first_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path package_root(first_result.plan.package_root);
    const fs::path content_root(first_result.plan.content_root);
    expect(fs::exists(content_root / "startup.sct"),
           "initial package should stage the startup form sidecar");
    expect(fs::exists(content_root / "old_name.prg"),
           "initial package should stage the original named asset");

    fs::remove(project_dir / "old_name.prg", ignored);
    const auto stale_plan_result = materialize_rematerialization_plan(first_result.plan, runtime_host);
    expect(stale_plan_result.ok,
           "re-materializing a returned plan should tolerate a removed optional asset");
    const auto stale_asset = std::find_if(
        stale_plan_result.plan.assets.begin(),
        stale_plan_result.plan.assets.end(),
        [](const copperfin::runtime::RuntimePackageAsset& asset) {
            return asset.relative_path == "old_name.prg";
        });
    expect(stale_asset != stale_plan_result.plan.assets.end() &&
               !stale_asset->copied &&
               stale_asset->sha256.empty(),
           "re-materializing a returned plan must clear stale optional-asset copy and digest state");

    write_text(project_dir / "old_name.prg", "RETURN\n");
    const auto restored_optional_result = materialize_rematerialization_plan(stale_plan_result.plan, runtime_host);
    expect(restored_optional_result.ok,
           "re-materializing after an optional source is restored should succeed");
    const auto restored_optional_asset = std::find_if(
        restored_optional_result.plan.assets.begin(),
        restored_optional_result.plan.assets.end(),
        [](const copperfin::runtime::RuntimePackageAsset& asset) {
            return asset.relative_path == "old_name.prg";
        });
    expect(restored_optional_result.plan.warnings.empty(),
           "re-materializing after an optional source is restored should discard stale materialization warnings");
    expect(restored_optional_asset != restored_optional_result.plan.assets.end() &&
               restored_optional_asset->copied && !restored_optional_asset->staged_path.empty() &&
               !restored_optional_asset->sha256.empty(),
           "re-materializing after an optional source is restored should rebuild its stage and digest state");

    auto excluded_asset_plan = restored_optional_result.plan;
    const auto excluded_asset = std::find_if(
        excluded_asset_plan.assets.begin(),
        excluded_asset_plan.assets.end(),
        [](const copperfin::runtime::RuntimePackageAsset& asset) {
            return asset.relative_path == "old_name.prg";
        });
    if (excluded_asset != excluded_asset_plan.assets.end()) {
        excluded_asset->excluded = true;
        excluded_asset->staged_path = (temp_root / "stale-stage.prg").string();
        excluded_asset->copied = true;
        excluded_asset->sha256 = "stale-asset-digest";
    }
    const auto excluded_asset_result = materialize_rematerialization_plan(excluded_asset_plan, runtime_host);
    expect(excluded_asset_result.ok,
           "re-materializing an excluded optional source should succeed");
    const auto rematerialized_excluded_asset = std::find_if(
        excluded_asset_result.plan.assets.begin(),
        excluded_asset_result.plan.assets.end(),
        [](const copperfin::runtime::RuntimePackageAsset& asset) {
            return asset.relative_path == "old_name.prg";
        });
    expect(rematerialized_excluded_asset != excluded_asset_result.plan.assets.end() &&
               !rematerialized_excluded_asset->copied &&
               rematerialized_excluded_asset->staged_path.empty() &&
               rematerialized_excluded_asset->sha256.empty(),
           "re-materializing an excluded optional source should discard stale stage, copy, and digest state");

    auto library_workspace = workspace;
    library_workspace.build_plan.output_kind = "dll";
    library_workspace.build_plan.output_path = (output_dir / "RematerializeLibrary.dll").string();
    const auto library_plan = create_rematerialization_plan(document, library_workspace, output_dir);
    const auto initial_library_result = materialize_rematerialization_plan(library_plan, runtime_host);
    expect(initial_library_result.ok,
           "initial library rematerialization fixture package should materialize");

    auto library_rematerialization_plan = initial_library_result.plan;
    library_rematerialization_plan.primary_output_materialized = true;
    library_rematerialization_plan.runtime_host_sha256 = "stale-runtime-host-digest";
    library_rematerialization_plan.compiler_contract_digests.push_back({
        .path = (temp_root / "stale-wrapper.cpp").string(),
        .sha256 = "stale-compiler-digest"
    });
    library_rematerialization_plan.launcher_artifacts.push_back({
        .package_relative_path = "stale-launcher.exe",
        .role = copperfin::runtime::RuntimeLauncherArtifactRole::public_apphost,
        .sha256 = "stale-launcher-digest"
    });
    const auto library_rematerialization_result =
        materialize_rematerialization_plan(library_rematerialization_plan, runtime_host);
    expect(library_rematerialization_result.ok,
           "re-materializing a returned library plan should succeed: " +
               library_rematerialization_result.error);
    expect(!library_rematerialization_result.plan.primary_output_materialized &&
               library_rematerialization_result.plan.runtime_host_sha256 != "stale-runtime-host-digest" &&
               library_rematerialization_result.plan.launcher_artifacts.empty() &&
               std::none_of(
                   library_rematerialization_result.plan.compiler_contract_digests.begin(),
                   library_rematerialization_result.plan.compiler_contract_digests.end(),
                   [](const copperfin::runtime::RuntimeArtifactDigest& digest) {
                       return digest.sha256 == "stale-compiler-digest";
                   }),
           "library rematerialization should rebuild primary output, runtime-host, compiler, and launcher-derived state");
    if (library_rematerialization_result.ok) {
        const auto library_build_result = copperfin::runtime::build_runtime_package_primary_output(
            library_rematerialization_result.plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile());
        expect(library_build_result.ok,
               "library rematerialization should finalize its deferred native wrapper transaction: " +
                   library_build_result.error);
    }
    fs::remove(project_dir / "startup.sct", ignored);
    write_text(project_dir / "new_name.prg", "RETURN\n");
    workspace.entries = {
        {.record_index = 1U, .name = "startup.scx", .relative_path = "startup.scx", .type_title = "Form"},
        {.record_index = 2U, .name = "new_name.prg", .relative_path = "new_name.prg", .type_title = "Program"}
    };

    const auto second_plan = create_rematerialization_plan(document, workspace, output_dir);
    const PackageSnapshot package_before_missing_sidecar = snapshot_package_files(package_root);
    const auto missing_sidecar_result = materialize_rematerialization_plan(second_plan, runtime_host);
    expect(!missing_sidecar_result.ok,
           "repeated materialization should reject a missing required startup sidecar");
    expect(snapshot_package_files(package_root) == package_before_missing_sidecar,
           "missing startup sidecar failure should preserve the prior generated package");

    write_text(project_dir / "startup.sct", "form-memo-v2");
    const auto restored_plan = create_rematerialization_plan(document, workspace, output_dir);
    const auto second_result = materialize_rematerialization_plan(restored_plan, runtime_host);
    expect(second_result.ok, "repeated materialization should replace the prior generated package");
    if (!second_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    expect(read_text(content_root / "startup.sct") == "form-memo-v2",
           "repeated materialization should replace the restored startup sidecar");
    expect(!fs::exists(content_root / "old_name.prg"),
           "repeated materialization should remove an asset renamed out of the project");
    expect(fs::exists(content_root / "new_name.prg"),
           "repeated materialization should stage the renamed asset under its current name");
    expect(read_text(unrelated_output) == "caller-owned",
           "package replacement should preserve caller-owned siblings in the output root");

    const std::string archive_manifest = read_text(second_result.plan.app_archive_manifest_path);
    const std::string archive_output = read_text(second_result.plan.launcher_output_path);
    expect(archive_manifest.find("startup.sct") != std::string::npos &&
               archive_output.find("startup.sct") != std::string::npos,
           "repeated APP materialization should archive the restored required sidecar");
    expect(archive_manifest.find("old_name.prg") == std::string::npos &&
               archive_output.find("old_name.prg") == std::string::npos,
           "repeated APP materialization should not re-archive a renamed-away asset");
    expect(archive_manifest.find("new_name.prg") != std::string::npos &&
               archive_output.find("new_name.prg") != std::string::npos,
           "repeated APP materialization should archive the current renamed asset");

    fs::remove(project_dir / "startup.scx", ignored);
    const auto missing_required_plan = create_rematerialization_plan(document, workspace, output_dir);
    const auto missing_required_asset = std::find_if(
        missing_required_plan.assets.begin(),
        missing_required_plan.assets.end(),
        [](const copperfin::runtime::RuntimePackageAsset& asset) {
            return asset.required_for_runtime;
        });
    expect(
        missing_required_asset != missing_required_plan.assets.end() &&
            !missing_required_asset->exists,
        "#4099: the freshness fixture should plan the required startup source as absent");
    write_text(project_dir / "startup.scx", "form-restored-without-replan");
    const auto restored_required_result =
        materialize_rematerialization_plan(missing_required_plan, runtime_host);
    expect(
        restored_required_result.ok,
        "#4099: restoring a required startup source should allow the same plan to materialize");
    const auto restored_required_asset = std::find_if(
        restored_required_result.plan.assets.begin(),
        restored_required_result.plan.assets.end(),
        [](const copperfin::runtime::RuntimePackageAsset& asset) {
            return asset.required_for_runtime;
        });
    expect(
        restored_required_asset != restored_required_result.plan.assets.end() &&
            restored_required_asset->exists && restored_required_asset->copied,
        "#4099: same-plan required-source restoration should refresh existence before copying");
    expect(
        read_text(fs::path(restored_required_result.plan.content_root) / "startup.scx") ==
            "form-restored-without-replan",
        "#4099: same-plan required-source restoration should stage the restored bytes");

    const PackageSnapshot last_known_good = snapshot_package_files(package_root);
    write_text(project_dir / "startup.scx", "form-v3");
    write_text(project_dir / "new_name.prg", "? 'changed'\nRETURN\n");
    auto failing_plan = create_rematerialization_plan(document, workspace, output_dir);
    failing_plan.ast_manifest_path = failing_plan.content_root;
    const auto failed_result = materialize_rematerialization_plan(failing_plan, runtime_host);
    expect(!failed_result.ok, "late package materialization failure should be reported");
    expect(snapshot_package_files(package_root) == last_known_good,
           "late package materialization failure should restore the exact prior package bytes");
    expect(!fs::exists(package_root.string() + ".copperfin-previous"),
           "successful rollback should consume the package recovery directory");
    expect(!fs::exists(package_root.string() + ".copperfin-materializing"),
           "successful rollback should consume the package transaction marker");

    const fs::path interrupted_backup = package_root.string() + ".copperfin-previous";
    const fs::path interrupted_marker = package_root.string() + ".copperfin-materializing";
    const fs::path interrupted_owner = interrupted_backup.string() + ".owner";
    const std::string transaction_identity =
        "copperfin_package_transaction=1\npackage_root=" +
        package_root.lexically_normal().generic_string() + "\n";
    fs::rename(package_root, interrupted_backup, ignored);
    expect(!ignored, "interrupted-rematerialization fixture should preserve the prior package as a backup");
    write_text(interrupted_owner, transaction_identity);
    fs::create_directories(content_root);
    write_text(content_root / "partial.tmp", "partial-package");
    write_text(interrupted_marker, transaction_identity);

    const auto interrupted_failure = materialize_rematerialization_plan(failing_plan, runtime_host);
    expect(!interrupted_failure.ok,
           "failed retry after interrupted materialization should preserve the recovery package");
    expect(snapshot_package_files(package_root) == last_known_good,
           "interrupted failed materialization should restore the exact last known-good package");
    expect(!fs::exists(interrupted_backup) && !fs::exists(interrupted_owner),
           "interrupted failed materialization rollback should consume recovery metadata");
    expect(!fs::exists(interrupted_marker),
           "interrupted failed materialization rollback should consume the transaction marker");
    expect(!fs::exists(content_root / "partial.tmp"),
           "interrupted failed materialization should remove the partial generated tree");

    fs::create_directories(interrupted_backup);
    write_text(interrupted_backup / "caller-owned.txt", "reserved-name-collision");
    const auto unowned_backup_collision = materialize_rematerialization_plan(second_plan, runtime_host);
    expect(!unowned_backup_collision.ok,
           "unowned reserved-name backup collision should fail without deleting caller data");
    expect(snapshot_package_files(package_root) == last_known_good &&
               read_text(interrupted_backup / "caller-owned.txt") == "reserved-name-collision",
           "unowned reserved-name backup collision should preserve both package and caller sibling");
    fs::remove_all(interrupted_backup, ignored);

    write_text(interrupted_owner, "caller-owned-owner");
    const auto unowned_owner_collision = materialize_rematerialization_plan(second_plan, runtime_host);
    expect(!unowned_owner_collision.ok,
           "unowned reserved-name backup owner should fail without deleting caller data");
    expect(snapshot_package_files(package_root) == last_known_good &&
               read_text(interrupted_owner) == "caller-owned-owner",
           "unowned backup-owner collision should preserve both package and caller sibling");
    fs::remove(interrupted_owner, ignored);

    write_text(interrupted_marker, "caller-owned-marker");
    const auto unowned_marker_collision = materialize_rematerialization_plan(second_plan, runtime_host);
    expect(!unowned_marker_collision.ok,
           "unowned reserved-name transaction marker should fail without deleting caller data");
    expect(snapshot_package_files(package_root) == last_known_good &&
               read_text(interrupted_marker) == "caller-owned-marker",
           "unowned reserved-name marker collision should preserve both package and caller sibling");
    fs::remove(interrupted_marker, ignored);

    const fs::path dangling_target = temp_root / "dangling-marker-target";
    fs::create_symlink(dangling_target, interrupted_marker, ignored);
    if (!ignored) {
        const auto dangling_marker_collision = materialize_rematerialization_plan(second_plan, runtime_host);
        expect(!dangling_marker_collision.ok,
               "dangling transaction-marker symlink should fail before following the target");
        expect(snapshot_package_files(package_root) == last_known_good &&
                   fs::is_symlink(interrupted_marker) &&
                   !fs::exists(dangling_target),
               "dangling marker collision should preserve package, symlink, and external target state");
        fs::remove(interrupted_marker, ignored);
    }

    fs::create_directories(interrupted_backup);
    write_text(interrupted_owner, transaction_identity);
    write_text(interrupted_backup / "cleanup-pending.tmp", "stale-backup");
    const auto cleanup_retry_failure = materialize_rematerialization_plan(failing_plan, runtime_host);
    expect(!cleanup_retry_failure.ok,
           "failed retry with a stale cleanup backup should still report the injected package failure");
    expect(snapshot_package_files(package_root) == last_known_good,
           "stale cleanup backup handling should preserve the current last known-good package");
    expect(!fs::exists(interrupted_backup) &&
               !fs::exists(interrupted_owner) &&
               !fs::exists(interrupted_marker),
           "stale cleanup retry rollback should leave no transaction artifacts");
    expect(read_text(unrelated_output) == "caller-owned",
           "failed and interrupted package replacement should preserve output-root siblings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_concurrent_materialization_is_serialized_per_package_root(
    const std::filesystem::path& executable_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_pipeline_concurrent_materialization";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);
    write_text(project_dir / "startup.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "concurrent.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ConcurrentMaterialization";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.output_path = (output_dir / "ConcurrentMaterialization.app").string();
    workspace.build_plan.output_kind = "app";
    workspace.build_plan.startup_item = "startup.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "startup.prg", .relative_path = "startup.prg", .type_title = "Program"}
    };

    const auto plan = create_rematerialization_plan(document, workspace, output_dir);
#if !defined(_WIN32)
    const fs::path lock_path = fs::path(plan.package_root).string() + ".copperfin-lock";
    write_text(lock_path, "caller-owned-lock\n");
    const auto caller_owned_lock_result = materialize_rematerialization_plan(plan, runtime_host);
    expect(!caller_owned_lock_result.ok,
           "a caller-owned package lock collision should fail before package mutation");
    expect(read_text(lock_path) == "caller-owned-lock\n",
           "a caller-owned package lock collision should preserve its bytes");
    fs::remove(lock_path, ignored);
#endif
    copperfin::runtime::test_hooks::arm_package_materialization_pause_after_begin();

    copperfin::runtime::RuntimeMaterializeResult first_result;
    std::thread first([&] {
        first_result = materialize_rematerialization_plan(plan, runtime_host);
    });
    const bool pause_entered =
        copperfin::runtime::test_hooks::wait_for_package_materialization_pause();
    expect(pause_entered,
           "the first materialization should reach the deterministic transaction overlap barrier");
    if (!pause_entered) {
        copperfin::runtime::test_hooks::release_package_materialization_pause();
        first.join();
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto second_result = materialize_rematerialization_plan(plan, runtime_host);
    expect(!second_result.ok,
           "a concurrent materialization of one package root should fail without entering the active transaction");

    const fs::path coordination_root = temp_root / "coordination";
    const fs::path config_path = coordination_root / "config.txt";
    const fs::path ready_path = coordination_root / "child.ready";
    const fs::path go_path = coordination_root / "child.go";
    const fs::path result_path = coordination_root / "child.result";
    fs::create_directories(coordination_root);
    write_text(
        config_path,
        copperfin::test_support::path_to_utf8_string(project_dir) + "\n" +
            copperfin::test_support::path_to_utf8_string(output_dir) + "\n" +
            copperfin::test_support::path_to_utf8_string(runtime_host) + "\n");
    const int child_exit = run_materialization_lock_probe_process(
        executable_path,
        config_path,
        ready_path,
        go_path,
        result_path);
    expect(child_exit == 0,
           "the cross-process materialization probe should exit cleanly");
    expect(read_text(result_path) == "busy\n",
           "a separate process should fail at the package-root lock before mutating the active transaction");

    copperfin::runtime::test_hooks::release_package_materialization_pause();
    first.join();
    expect(first_result.ok,
           "the transaction that acquired the package-root lock should complete normally");

    const fs::path package_root(first_result.plan.package_root);
    expect(fs::exists(package_root / "content" / "startup.prg"),
           "the winning materialization should retain its complete package content");
    expect(!fs::exists(package_root.string() + ".copperfin-previous") &&
               !fs::exists(package_root.string() + ".copperfin-materializing"),
           "concurrent materialization rejection should leave no transaction recovery artifacts");
#if !defined(_WIN32)
    const std::string lock_contents = read_text(package_root.string() + ".copperfin-lock");
    expect(lock_contents.rfind("copperfin_package_lock=1\nidentity=", 0U) == 0U &&
               lock_contents.back() == '\n',
           "the POSIX package lock should retain only Copperfin ownership bytes");
#endif

    const fs::path recovery_ready_path = coordination_root / "recovery.ready";
    const fs::path recovery_go_path = coordination_root / "recovery.go";
    const fs::path recovery_result_path = coordination_root / "recovery.result";
    const int recovery_exit = run_materialization_lock_probe_process(
        executable_path,
        config_path,
        recovery_ready_path,
        recovery_go_path,
        recovery_result_path);
    expect(recovery_exit == 0 && read_text(recovery_result_path) == "ok\n",
           "a later process should reacquire a valid package lock after the original owner exits");

    fs::remove_all(temp_root, ignored);
}

void test_package_transaction_rejects_rebound_output_parent() {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_pipeline_rebound_parent";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path moved_output_dir = temp_root / "output-moved";
    const fs::path external_output_dir = temp_root / "external-output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);
    fs::create_directories(external_output_dir);
    write_text(project_dir / "startup.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "rebound.pjx").string();
    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ReboundParent";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.output_path =
        (output_dir / "ReboundParent.app").string();
    workspace.build_plan.output_kind = "app";
    workspace.build_plan.startup_item = "startup.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U,
         .name = "startup.prg",
         .relative_path = "startup.prg",
         .type_title = "Program"}
    };
    const auto plan = create_rematerialization_plan(document, workspace, output_dir);
    const auto initial_result = materialize_rematerialization_plan(plan, runtime_host);
    expect(initial_result.ok,
           "the parent-rebinding fixture should first create a known-good package");
    if (!initial_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }
    const PackageSnapshot known_good = snapshot_package_files(plan.package_root);

    copperfin::runtime::test_hooks::arm_package_materialization_pause_after_begin();
    copperfin::runtime::RuntimeMaterializeResult result;
    std::thread materialization([&] {
        result = materialize_rematerialization_plan(plan, runtime_host);
    });
    const bool pause_entered =
        copperfin::runtime::test_hooks::wait_for_package_materialization_pause();
    expect(pause_entered,
           "the parent-rebinding transaction should reach its deterministic barrier");
    if (!pause_entered) {
        copperfin::runtime::test_hooks::release_package_materialization_pause();
        materialization.join();
        fs::remove_all(temp_root, ignored);
        return;
    }

    std::error_code rebind_error;
#if defined(_WIN32)
    fs::rename(output_dir, moved_output_dir, rebind_error);
    expect(static_cast<bool>(rebind_error),
           "the Windows parent directory pin should prevent rebinding the admitted output root");
    rebind_error.clear();
#else
    fs::rename(output_dir, moved_output_dir, rebind_error);
    expect(!rebind_error,
           "the POSIX parent-rebinding fixture should move the admitted output root");
    if (!rebind_error) {
        fs::create_directory_symlink(external_output_dir, output_dir, rebind_error);
        expect(!rebind_error,
               "the POSIX parent-rebinding fixture should install an external output symlink");
    }
#endif

    copperfin::runtime::test_hooks::release_package_materialization_pause();
    materialization.join();

#if defined(_WIN32)
    expect_materialization(result,
           "the Windows transaction should continue after the blocked parent-rebind attempt");
    expect(result.ok && fs::exists(fs::path(result.plan.package_root) / "content" / "startup.prg"),
           "the Windows parent-rebind guard should preserve package materialization");
#else
    expect(!result.ok,
           "the POSIX transaction should reject a rebound output parent before mutation");
    expect(!fs::exists(external_output_dir / "ReboundParent") &&
               !fs::exists(external_output_dir / "ReboundParent.copperfin-materializing"),
           "a rejected POSIX parent rebind must not modify the external output tree");
    fs::remove(output_dir, ignored);
    fs::rename(moved_output_dir, output_dir, ignored);
    expect(snapshot_package_files(plan.package_root) == known_good,
           "a rejected POSIX parent rebind must restore the last known-good package");
#endif

    fs::remove_all(temp_root, ignored);
}

void test_package_content_root_remains_pinned_during_asset_writes() {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_pipeline_rebound_content";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path moved_content_dir = temp_root / "content-moved";
    const fs::path external_content_dir = temp_root / "external-content";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);
    fs::create_directories(external_content_dir);
    write_text(project_dir / "startup.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "rebound-content.pjx").string();
    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ReboundContent";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.output_path =
        (output_dir / "ReboundContent.app").string();
    workspace.build_plan.output_kind = "app";
    workspace.build_plan.startup_item = "startup.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U,
         .name = "startup.prg",
         .relative_path = "startup.prg",
         .type_title = "Program"}
    };
    const auto plan = create_rematerialization_plan(document, workspace, output_dir);
    const auto initial_result = materialize_rematerialization_plan(plan, runtime_host);
    expect(initial_result.ok,
           "the content-rebinding fixture should first create a known-good package");
    if (!initial_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }
    const PackageSnapshot known_good = snapshot_package_files(plan.package_root);

    copperfin::runtime::test_hooks::
        arm_package_content_materialization_pause_before_first_asset();
    copperfin::runtime::RuntimeMaterializeResult result;
    std::thread materialization([&] {
        result = materialize_rematerialization_plan(plan, runtime_host);
    });
    const bool pause_entered =
        copperfin::runtime::test_hooks::wait_for_package_content_materialization_pause();
    expect(pause_entered,
           "the content-rebinding transaction should reach its content-root barrier");
    if (!pause_entered) {
        copperfin::runtime::test_hooks::release_package_content_materialization_pause();
        materialization.join();
        fs::remove_all(temp_root, ignored);
        return;
    }

    std::error_code rebind_error;
#if defined(_WIN32)
    fs::rename(
        fs::path(plan.content_root),
        moved_content_dir,
        rebind_error);
    expect(static_cast<bool>(rebind_error),
           "the Windows content-directory pin should prevent rebinding the admitted content root");
#else
    fs::rename(fs::path(plan.content_root), moved_content_dir, rebind_error);
    expect(!rebind_error,
           "the POSIX content-rebinding fixture should move the admitted content root");
    if (!rebind_error) {
        fs::create_directory_symlink(
            external_content_dir,
            fs::path(plan.content_root),
            rebind_error);
        expect(!rebind_error,
               "the POSIX content-rebinding fixture should install an external content symlink");
    }
#endif

    copperfin::runtime::test_hooks::release_package_content_materialization_pause();
    materialization.join();

#if defined(_WIN32)
    expect_materialization(result,
           "the Windows transaction should continue after the blocked content rebind attempt");
    expect(
        result.ok && fs::exists(fs::path(result.plan.package_root) / "content" / "startup.prg"),
        "the Windows content-rebind guard should preserve package materialization");
#else
    expect_materialization(result,
           "the POSIX transaction should write through the pinned content directory");
    expect(!fs::exists(external_content_dir / "startup.prg"),
           "a POSIX content rebind must not redirect staged bytes to the external directory");
    fs::remove(fs::path(plan.content_root), ignored);
    fs::rename(moved_content_dir, fs::path(plan.content_root), ignored);
    expect(
        result.ok && fs::exists(fs::path(result.plan.package_root) / "content" / "startup.prg"),
        "restoring the original content name should expose the completed package");
#endif

    const fs::path external_leaf = temp_root / "external-leaf.prg";
    write_text(external_leaf, "external-leaf-sentinel");
    copperfin::runtime::test_hooks::
        arm_package_content_materialization_pause_before_first_asset();
    copperfin::runtime::RuntimeMaterializeResult leaf_result;
    std::thread leaf_materialization([&] {
        leaf_result = materialize_rematerialization_plan(plan, runtime_host);
    });
    const bool leaf_pause_entered =
        copperfin::runtime::test_hooks::wait_for_package_content_materialization_pause();
    expect(leaf_pause_entered,
           "the hard-link leaf transaction should reach its content-root barrier");
    if (leaf_pause_entered) {
        std::error_code hard_link_error;
        fs::create_hard_link(
            external_leaf,
            fs::path(plan.package_root) / "content" / "startup.prg",
            hard_link_error);
        expect(!hard_link_error,
               "the hard-link leaf fixture should create its external alias");
        copperfin::runtime::test_hooks::release_package_content_materialization_pause();
    } else {
        copperfin::runtime::test_hooks::release_package_content_materialization_pause();
    }
    leaf_materialization.join();
    expect(!leaf_result.ok,
           "a hard-link destination should be rejected before package bytes are replaced");
    expect(read_text(external_leaf) == "external-leaf-sentinel",
           "hard-link rejection should preserve the external destination bytes");
    expect(snapshot_package_files(plan.package_root) == known_good,
           "hard-link rejection should restore the last known-good package");

#if defined(_WIN32)
    {
        const fs::path package_root(plan.package_root);
        std::string alias_leaf = package_root.filename().string();
        for (char& character : alias_leaf) {
            if (character >= 'A' && character <= 'Z') {
                character = static_cast<char>(character - 'A' + 'a');
            } else if (character >= 'a' && character <= 'z') {
                character = static_cast<char>(character - 'a' + 'A');
            }
        }
        const fs::path alias_package_root = package_root.parent_path() / alias_leaf;
        const fs::path interrupted_backup = alias_package_root.string() + ".copperfin-previous";
        const fs::path interrupted_marker = alias_package_root.string() + ".copperfin-materializing";
        const fs::path interrupted_owner = interrupted_backup.string() + ".owner";
        std::string transaction_identity =
            package_root.lexically_normal().generic_string();
        transaction_identity =
            "copperfin_package_transaction=1\npackage_root=" +
            transaction_identity + "\n";

        fs::rename(package_root, interrupted_backup, ignored);
        expect(!ignored,
               "#4097: Windows casing-alias fixture should create an interrupted backup");
        if (!ignored) {
            write_text(interrupted_owner, transaction_identity);
            write_text(interrupted_marker, transaction_identity);
            auto recovery_plan = plan;
            recovery_plan.package_root = alias_package_root.string();
            const auto recovery_result =
                materialize_rematerialization_plan(recovery_plan, runtime_host);
            expect(recovery_result.ok,
                   "#4097: Windows casing-only package alias should recover its owned transaction");
            expect(fs::exists(alias_package_root / "content" / "startup.prg") &&
                       read_text(alias_package_root / "content" / "startup.prg") == "RETURN\n",
                   "#4097: Windows casing-only package alias should materialize the recovered package");
            expect(!fs::exists(interrupted_backup) &&
                       !fs::exists(interrupted_owner) &&
                       !fs::exists(interrupted_marker),
                   "#4097: Windows casing-only recovery should consume transaction artifacts");
        }
    }
#endif

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_runtime_pipeline

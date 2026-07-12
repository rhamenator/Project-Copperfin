// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_support.h"

#include <map>

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

    fs::remove(project_dir / "startup.sct", ignored);
    fs::remove(project_dir / "old_name.prg", ignored);
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

}  // namespace cf_test_runtime_pipeline

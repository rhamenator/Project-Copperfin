// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_nudges_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_nudge_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto nudge_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--nudge-object",
            "--nudge-mode", "both",
            "--delta-hpos", "25",
            "--delta-vpos", "-100",
            "--nudge-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (nudge_process.exit_code != 0) {
        std::cerr << "studio host report object nudge stdout:\n" << nudge_process.stdout_text << "\n";
        std::cerr << "studio host report object nudge stderr:\n" << nudge_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(nudge_process.exit_code == 0,
           "#1463: report layout object nudge should exit successfully");
    expect(visual_object_property(report_path, "field-guid", "HPOS") == "1225" &&
               visual_object_property(report_path, "field-guid", "VPOS") == "2500",
           "#1463: report layout object nudge should mutate FRX HPOS and VPOS fields");
    expect_contains(nudge_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1463: nudged report object JSON should retain selected-object availability");
    expect_contains(nudge_process.stdout_text, "\"left\": 1225",
                    "#1463: nudged report object JSON should expose updated left coordinates");
    expect_contains(nudge_process.stdout_text, "\"top\": 2500",
                    "#1463: nudged report object JSON should expose updated top coordinates");
    expect_contains(nudge_process.stdout_text, "\"right\": 5225",
                    "#1463: nudged report object JSON should recompute right-edge coordinates");
    expect_contains(nudge_process.stdout_text, "\"bottom\": 2950",
                    "#1463: nudged report object JSON should recompute bottom-edge coordinates");
    expect_contains(nudge_process.stdout_text, "\"sectionRelativeTop\": 500",
                    "#1463: nudged report object JSON should recompute section-relative top coordinates");
    expect_contains(nudge_process.stdout_text, "\"sectionRelativeBottom\": 950",
                    "#1463: nudged report object JSON should recompute section-relative bottom coordinates");
    expect_contains(nudge_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1463: nudged report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_aligns_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_align_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto align_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "label-guid",
            "--align-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (align_process.exit_code != 0) {
        std::cerr << "studio host report object align stdout:\n" << align_process.stdout_text << "\n";
        std::cerr << "studio host report object align stderr:\n" << align_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(align_process.exit_code == 0,
           "#1464: report layout object alignment should exit successfully");
    expect(visual_object_property(report_path, "field-guid", "HPOS") == "900" &&
               visual_object_property(report_path, "field-guid", "VPOS") == "2600",
           "#1464: report layout object left alignment should mutate FRX HPOS and preserve VPOS");
    expect(visual_object_property(report_path, "label-guid", "HPOS") == "900",
           "#1464: report layout object alignment should preserve anchor geometry");
    expect_contains(align_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1464: aligned report object JSON should retain selected-object availability");
    expect_contains(align_process.stdout_text, "\"left\": 900",
                    "#1464: aligned report object JSON should expose updated left coordinates");
    expect_contains(align_process.stdout_text, "\"top\": 2600",
                    "#1464: aligned report object JSON should preserve top coordinates");
    expect_contains(align_process.stdout_text, "\"right\": 4900",
                    "#1464: aligned report object JSON should recompute right-edge coordinates");
    expect_contains(align_process.stdout_text, "\"bottom\": 3050",
                    "#1464: aligned report object JSON should preserve bottom-edge coordinates");
    expect_contains(align_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1464: aligned report object JSON should preserve section-relative top coordinates");
    expect_contains(align_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1464: aligned report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_resizes_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_resize_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto resize_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--resize-object",
            "--resize-mode", "size",
            "--anchor-unique-id", "label-guid",
            "--resize-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (resize_process.exit_code != 0) {
        std::cerr << "studio host report object resize stdout:\n" << resize_process.stdout_text << "\n";
        std::cerr << "studio host report object resize stderr:\n" << resize_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(resize_process.exit_code == 0,
           "#1465: report layout object resize should exit successfully");
    expect(visual_object_property(report_path, "field-guid", "WIDTH") == "1800" &&
               visual_object_property(report_path, "field-guid", "HEIGHT") == "350",
           "#1465: report layout object size resize should mutate FRX WIDTH and HEIGHT fields");
    expect(visual_object_property(report_path, "field-guid", "HPOS") == "1200" &&
               visual_object_property(report_path, "field-guid", "VPOS") == "2600",
           "#1465: report layout object size resize should preserve FRX HPOS and VPOS fields");
    expect_contains(resize_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1465: resized report object JSON should retain selected-object availability");
    expect_contains(resize_process.stdout_text, "\"left\": 1200",
                    "#1465: resized report object JSON should preserve left coordinates");
    expect_contains(resize_process.stdout_text, "\"top\": 2600",
                    "#1465: resized report object JSON should preserve top coordinates");
    expect_contains(resize_process.stdout_text, "\"width\": 1800",
                    "#1465: resized report object JSON should expose updated width");
    expect_contains(resize_process.stdout_text, "\"height\": 350",
                    "#1465: resized report object JSON should expose updated height");
    expect_contains(resize_process.stdout_text, "\"right\": 3000",
                    "#1465: resized report object JSON should recompute right-edge coordinates");
    expect_contains(resize_process.stdout_text, "\"bottom\": 2950",
                    "#1465: resized report object JSON should recompute bottom-edge coordinates");
    expect_contains(resize_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1465: resized report object JSON should preserve section-relative top coordinates");
    expect_contains(resize_process.stdout_text, "\"sectionRelativeBottom\": 950",
                    "#1465: resized report object JSON should recompute section-relative bottom coordinates");
    expect_contains(resize_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1465: resized report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_snaps_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_snap_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto snap_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--snap-object",
            "--snap-mode", "both",
            "--grid-width", "700",
            "--grid-height", "750",
            "--snap-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (snap_process.exit_code != 0) {
        std::cerr << "studio host report object snap stdout:\n" << snap_process.stdout_text << "\n";
        std::cerr << "studio host report object snap stderr:\n" << snap_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(snap_process.exit_code == 0,
           "#1466: report layout object snap should exit successfully");
    expect(visual_object_property(report_path, "field-guid", "HPOS") == "1400" &&
               visual_object_property(report_path, "field-guid", "VPOS") == "2250",
           "#1466: report layout object snap should mutate FRX HPOS and VPOS fields");
    expect_contains(snap_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1466: snapped report object JSON should retain selected-object availability");
    expect_contains(snap_process.stdout_text, "\"left\": 1400",
                    "#1466: snapped report object JSON should expose updated left coordinates");
    expect_contains(snap_process.stdout_text, "\"top\": 2250",
                    "#1466: snapped report object JSON should expose updated top coordinates");
    expect_contains(snap_process.stdout_text, "\"right\": 5400",
                    "#1466: snapped report object JSON should recompute right-edge coordinates");
    expect_contains(snap_process.stdout_text, "\"bottom\": 2700",
                    "#1466: snapped report object JSON should recompute bottom-edge coordinates");
    expect_contains(snap_process.stdout_text, "\"sectionRelativeTop\": 250",
                    "#1466: snapped report object JSON should recompute section-relative top coordinates");
    expect_contains(snap_process.stdout_text, "\"sectionRelativeBottom\": 700",
                    "#1466: snapped report object JSON should recompute section-relative bottom coordinates");
    expect_contains(snap_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1466: snapped report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_delete_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--delete-object",
            "--unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host report object delete stdout:\n" << delete_process.stdout_text << "\n";
        std::cerr << "studio host report object delete stderr:\n" << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0,
           "#1467: report layout object delete should exit successfully");
    expect(visual_object_deleted(report_path, "field-guid"),
           "#1467: report layout object delete should mark the FRX object record deleted");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1467: deleted report object JSON should move the object into deleted report objects");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1467: deleted selected report object JSON should remain available");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1467: deleted report objects should preserve containing-section availability");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1467: deleted report objects should serialize containing-section JSON");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1467: deleted report object selections should still classify as report objects");
    expect_contains(delete_process.stdout_text, "\"recordIndex\": 3",
                    "#1467: deleted selected report object JSON should preserve selected record indexes");
    expect_contains(delete_process.stdout_text, "\"deleted\": true",
                    "#1467: deleted selected report object JSON should expose deleted state");
    expect_contains(delete_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1467: deleted report object JSON should preserve containing section ids");
    expect_contains(delete_process.stdout_text, "\"containingSectionRecordIndex\": 2",
                    "#1467: deleted report object JSON should preserve containing section record indexes");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_restore_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);
    const auto seed_delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "field-guid",
        .deleted = true
    });
    expect(seed_delete_result.ok && visual_object_deleted(report_path, "field-guid"),
           "#1468: report layout restore fixture should start with a deleted report object");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--restore-object",
            "--unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host report object restore stdout:\n" << restore_process.stdout_text << "\n";
        std::cerr << "studio host report object restore stderr:\n" << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0,
           "#1468: report layout object restore should exit successfully");
    expect(!visual_object_deleted(report_path, "field-guid"),
           "#1468: report layout object restore should clear the FRX object deleted flag");
    expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1468: restored report object JSON should move the object out of deleted report objects");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1468: restored selected report object JSON should remain available");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1468: restored report objects should advertise containing-section availability");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1468: restored report objects should serialize containing-section JSON");
    expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1468: restored report object selections should classify as report objects");
    expect_contains(restore_process.stdout_text, "\"recordIndex\": 3",
                    "#1468: restored selected report object JSON should preserve selected record indexes");
    expect_contains(restore_process.stdout_text, "\"deleted\": false",
                    "#1468: restored selected report object JSON should expose live state");
    expect_contains(restore_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1468: restored report object JSON should expose containing section ids again");
    expect_contains(restore_process.stdout_text, "\"containingSectionRecordIndex\": 2",
                    "#1468: restored report object JSON should expose containing section record indexes again");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_distribute_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_distribution_json(report_path);

    const auto distribute_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--distribute-object",
            "--distribution-mode", "horizontal",
            "--distribute-target-unique-id", "left-field-guid",
            "--distribute-target-unique-id", "middle-field-guid",
            "--distribute-target-unique-id", "right-field-guid",
            "--json"
        },
        temp_root);

    if (distribute_process.exit_code != 0) {
        std::cerr << "studio host report object distribute stdout:\n" << distribute_process.stdout_text << "\n";
        std::cerr << "studio host report object distribute stderr:\n" << distribute_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(distribute_process.exit_code == 0,
           "#1469: report layout object distribution should exit successfully");
    expect(visual_object_property(report_path, "left-field-guid", "HPOS") == "100" &&
               visual_object_property(report_path, "middle-field-guid", "HPOS") == "400" &&
               visual_object_property(report_path, "right-field-guid", "HPOS") == "700",
           "#1469: report layout object distribution should evenly position the middle FRX object");
    expect_contains(distribute_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1469: distributed report object JSON should retain selected-object availability");
    expect_contains(distribute_process.stdout_text, "\"left\": 400",
                    "#1469: distributed report object JSON should expose updated left coordinates");
    expect_contains(distribute_process.stdout_text, "\"right\": 450",
                    "#1469: distributed report object JSON should recompute right-edge coordinates");
    expect_contains(distribute_process.stdout_text, "\"sectionObjectIndex\": 1",
                    "#1469: distributed report object JSON should preserve sorted section object order");
    expect_contains(distribute_process.stdout_text, "\"sectionObjectCount\": 3",
                    "#1469: distributed report object JSON should expose containing section object counts");
    expect_contains(distribute_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    "#1469: distributed report object JSON should preserve containing section metadata");
    expect_contains(distribute_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1469: distributed report object JSON should keep selected containing-section availability");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_reorder_json(report_path);

    const auto reorder_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--reorder-object",
            "--unique-id", "right-field-guid",
            "--placement", "before",
            "--target-unique-id", "left-field-guid",
            "--json"
        },
        temp_root);

    if (reorder_process.exit_code != 0) {
        std::cerr << "studio host report object reorder stdout:\n" << reorder_process.stdout_text << "\n";
        std::cerr << "studio host report object reorder stderr:\n" << reorder_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(reorder_process.exit_code == 0,
           "#1470: report layout object reorder should exit successfully");
    expect(visual_object_order(report_path) == "right-field-guid,left-field-guid,middle-field-guid",
           "#1470: report layout object reorder should update physical FRX record order");
    expect_contains_in_order(
        reorder_process.stdout_text,
        {
            "\"sectionObjectIndex\": 0",
            "\"expression\": \"right.value\"",
            "\"sectionObjectIndex\": 1",
            "\"expression\": \"left.value\"",
            "\"sectionObjectIndex\": 2",
            "\"expression\": \"middle.value\""
        },
        "#1470: report layout JSON should serialize tied-geometry section objects in reordered record order");
    expect_contains(reorder_process.stdout_text, "\"sectionObjectCount\": 3",
                    "#1470: reordered report object JSON should expose containing section object counts");
    expect_contains(reorder_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    "#1470: reordered report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const std::size_t before_count = visual_object_count(report_path);

    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--duplicate-object",
            "--unique-id", "middle-field-guid",
            "--new-unique-id", "middle-copy-guid",
            "--json"
        },
        temp_root);

    if (duplicate_process.exit_code != 0) {
        std::cerr << "studio host report object duplicate stdout:\n" << duplicate_process.stdout_text << "\n";
        std::cerr << "studio host report object duplicate stderr:\n" << duplicate_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(duplicate_process.exit_code == 0,
           "#1471: report layout object duplicate should exit successfully");
    expect(visual_object_count(report_path) == before_count + 1U,
           "#1471: report layout object duplicate should append one FRX object record");
    expect(visual_object_exists(report_path, "middle-copy-guid"),
           "#1471: report layout object duplicate should persist replacement unique ids");
    expect(visual_object_order(report_path) == "left-field-guid,middle-field-guid,right-field-guid,middle-copy-guid",
           "#1471: report layout object duplicate should append the copied FRX object after existing layout objects");
    expect_contains_in_order(
        duplicate_process.stdout_text,
        {
            "\"recordIndex\": 5",
            "\"containingSectionId\": \"detail_1\"",
            "\"sectionObjectIndex\": 3",
            "\"sectionObjectCount\": 4",
            "\"expression\": \"middle.value\""
        },
        "#1471: report layout JSON should expose the duplicated object in refreshed section membership");
    expect_contains(duplicate_process.stdout_text, "\"sectionObjectCount\": 4",
                    "#1471: duplicated report object JSON should refresh containing section object counts");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_report_layout_object_identity_by_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const std::size_t before_count = visual_object_count(report_path);

    const auto rename_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--rename-object",
            "--unique-id", "right-field-guid",
            "--new-unique-id", "renamed-right-guid",
            "--json"
        },
        temp_root);

    if (rename_process.exit_code != 0) {
        std::cerr << "studio host report object rename stdout:\n" << rename_process.stdout_text << "\n";
        std::cerr << "studio host report object rename stderr:\n" << rename_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(rename_process.exit_code == 0,
           "#1472: report layout object rename should exit successfully");
    expect(visual_object_count(report_path) == before_count,
           "#1472: report layout object rename should preserve FRX object count");
    expect(!visual_object_exists(report_path, "right-field-guid"),
           "#1472: report layout object rename should remove the old unique id");
    expect(visual_object_exists(report_path, "renamed-right-guid"),
           "#1472: report layout object rename should persist replacement unique ids");
    expect(visual_object_order(report_path) == "left-field-guid,middle-field-guid,renamed-right-guid",
           "#1472: report layout object rename should preserve physical FRX object order");
    expect_contains_in_order(
        rename_process.stdout_text,
        {
            "\"recordIndex\": 4",
            "\"containingSectionId\": \"detail_1\"",
            "\"sectionObjectIndex\": 2",
            "\"sectionObjectCount\": 3",
            "\"expression\": \"right.value\""
        },
        "#1472: report layout JSON should keep the renamed object in refreshed section membership");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_expression_update = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "EXPR",
                "--property-value", "customer.contact",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1529: report/label layout object expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.contact",
               "#1529: report/label layout object expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1529: report/label layout object expression update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1529: report/label layout object expression update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1529: report/label layout object expression update should preserve object selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.contact\"",
                "\"expression\": \"customer.contact\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1529: report/label layout object expression update should refresh selected object expression metadata");
    };

    run_expression_update(temp_root / "expression_update.frx", "expression_update.frx", "report");
    run_expression_update(temp_root / "expression_update.lbx", "expression_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_expression_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_expression_update = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "EXPR",
                "--property-value", "customer.contact",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1630: report/label layout object stable expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.contact",
               "#1630: report/label layout object stable expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1630: report/label layout object stable expression update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1630: label layout object stable expression update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1630: report/label layout object stable expression update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1630: report/label layout object stable expression update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1630: report/label layout object stable expression update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.contact\"",
                "\"expression\": \"customer.contact\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1630: report/label layout object stable expression update should refresh selected object expression metadata");
    };

    run_expression_update(temp_root / "expression_update_stable.frx",
                          "expression_update_stable.frx",
                          "report");
    run_expression_update(temp_root / "expression_update_stable.lbx",
                          "expression_update_stable.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_expression_update = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1601: deleted report/label layout object expression fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "EXPR",
                "--property-value", "customer.deleted_contact",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1601: deleted report/label layout object expression update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1601: deleted report/label layout object expression update should preserve deleted state");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.deleted_contact",
               "#1601: deleted report/label layout object expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1601: deleted report/label layout object expression update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1601: deleted report/label layout object expression update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1601: deleted report/label layout object expression update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1601: deleted report/label layout object expression update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1601: deleted report/label layout object expression update should preserve containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1601: deleted report/label layout object expression update should serialize containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"objectKind\": \"label\"",
                "\"title\": \"customer.deleted_contact\"",
                "\"expression\": \"customer.deleted_contact\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1601: deleted report/label layout object expression update should refresh deleted-object expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"objectKind\": \"label\"",
                "\"title\": \"customer.deleted_contact\"",
                "\"expression\": \"customer.deleted_contact\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1601: deleted report/label layout object expression update should refresh selected deleted-object metadata");
        expect_not_contains(update_process.stdout_text, "\"expression\": \"\\\"Deleted label\\\"\"",
                            "#1601: deleted report/label layout object expression update should not leak stale expression values");
    };

    run_deleted_expression_update(temp_root / "deleted_expression_update.frx",
                                  "deleted_expression_update.frx",
                                  "report");
    run_deleted_expression_update(temp_root / "deleted_expression_update.lbx",
                                  "deleted_expression_update.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_expression_clear = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1558: report/label layout object expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1558: report/label layout object expression clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1558: report/label layout object expression clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1558: report/label layout object expression clear should preserve object selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"field-guid\"",
                "\"titleFieldIndex\": 9",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1558: report/label layout object expression clear should refresh selected object expression metadata");
        expect_not_contains(clear_process.stdout_text, "\"expression\": \"customer.company\"",
                            "#1558: report/label layout object expression clear should not leak stale expression values");
    };

    run_expression_clear(temp_root / "expression_clear.frx", "expression_clear.frx", "report");
    run_expression_clear(temp_root / "expression_clear.lbx", "expression_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_expression_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_expression_clear = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1631: report/label layout object stable expression clear should exit successfully");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"EXPR\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 2",
                        "#1631: report/label layout object stable expression clear should blank the EXPR memo field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1631: report/label layout object stable expression clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1631: label layout object stable expression clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1631: report/label layout object stable expression clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1631: report/label layout object stable expression clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1631: report/label layout object stable expression clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"field-guid\"",
                "\"titleFieldIndex\": 9",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1631: report/label layout object stable expression clear should refresh selected object expression metadata");
        expect_not_contains(clear_process.stdout_text, "\"expression\": \"customer.company\"",
                            "#1631: report/label layout object stable expression clear should not leak stale expression values");
    };

    run_expression_clear(temp_root / "expression_clear_stable.frx",
                         "expression_clear_stable.frx",
                         "report");
    run_expression_clear(temp_root / "expression_clear_stable.lbx",
                         "expression_clear_stable.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_expression_clear = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1602: deleted report/label layout object expression clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1602: deleted report/label layout object expression clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1602: deleted report/label layout object expression clear should preserve deleted state");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1602: deleted report/label layout object expression clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1602: deleted report/label layout object expression clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1602: deleted report/label layout object expression clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1602: deleted report/label layout object expression clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1602: deleted report/label layout object expression clear should preserve containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1602: deleted report/label layout object expression clear should serialize containing-section metadata");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"EXPR\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 2",
                        "#1602: deleted report/label layout object expression clear should blank the EXPR memo property");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"objectKind\": \"label\"",
                "\"title\": \"Record 6\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1602: deleted report/label layout object expression clear should refresh deleted-object expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"objectKind\": \"label\"",
                "\"title\": \"Record 6\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1602: deleted report/label layout object expression clear should refresh selected deleted-object metadata");
        expect_not_contains(clear_process.stdout_text, "\"expression\": \"\\\"Deleted label\\\"\"",
                            "#1602: deleted report/label layout object expression clear should not leak stale expression values");
    };

    run_deleted_expression_clear(temp_root / "deleted_expression_clear.frx",
                                 "deleted_expression_clear.frx",
                                 "report");
    run_deleted_expression_clear(temp_root / "deleted_expression_clear.lbx",
                                 "deleted_expression_clear.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_unplaced_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1614: restore edited deleted layout object as unplaced fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "6",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted unplaced " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted unplaced " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1614: deleted report/label layout object unplaced pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1614: deleted report/label layout object unplaced pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "-300");
        set_deleted_geometry("VPOS", "9000");
        set_deleted_geometry("HEIGHT", "700");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "6",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited deleted layout unplaced restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited deleted layout unplaced restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1614: edited deleted report/label layout object unplaced restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1614: edited deleted report/label layout object unplaced restore should clear deleted state");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1614: edited deleted report/label layout object unplaced restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1892: label edited deleted layout object unplaced restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1614: edited deleted report/label layout object unplaced restore should keep preview bounds available");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1614: edited deleted report/label layout object unplaced restore should expand preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1892: edited deleted report/label layout object unplaced restore should preserve preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1614: edited deleted report/label layout object unplaced restore should preserve preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1614: edited deleted report/label layout object unplaced restore should expand preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 5500",
                        "#1614: edited deleted report/label layout object unplaced restore should expand preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 9700",
                        "#1614: edited deleted report/label layout object unplaced restore should expand preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1892: edited deleted report/label layout object unplaced restore should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1614: edited deleted report/label layout object unplaced restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1614: edited deleted report/label layout object unplaced restore should not count out-of-band restored objects as placed");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1614: edited deleted report/label layout object unplaced restore should add restored objects to unplaced counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1614: edited deleted report/label layout object unplaced restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1614: edited deleted report/label layout object unplaced restore should not fabricate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1614: edited deleted report/label layout object unplaced restore should serialize null containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 1200",
                "\"right\": 900",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1614: edited deleted report/label layout object unplaced restore should refresh selected unplaced geometry without section metadata");
    };

    run_deleted_unplaced_restore(temp_root / "deleted_restore_unplaced.frx",
                                 "deleted_restore_unplaced.frx",
                                 "report");
    run_deleted_unplaced_restore(temp_root / "deleted_restore_unplaced.lbx",
                                 "deleted_restore_unplaced.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_unplaced_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_unplaced_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1653: stable restore edited deleted layout object as unplaced fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-label-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted unplaced " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted unplaced " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1653: stable deleted report/label layout object unplaced pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1653: stable deleted report/label layout object unplaced pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "-300");
        set_deleted_geometry("VPOS", "9000");
        set_deleted_geometry("HEIGHT", "700");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "deleted-label-guid",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable edited deleted layout unplaced restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable edited deleted layout unplaced restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1653: stable edited deleted report/label layout object unplaced restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1653: stable edited deleted report/label layout object unplaced restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && !left_property.record_deleted &&
                   left_property.value == "-300" &&
                   top_property.ok && top_property.exists && !top_property.record_deleted &&
                   top_property.value == "9000" &&
                   height_property.ok && height_property.exists && !height_property.record_deleted &&
                   height_property.value == "700",
               "#1653: stable edited deleted report/label layout object unplaced restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1653: stable edited deleted report/label layout object unplaced restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1653: label stable edited deleted layout object unplaced restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1653: stable edited deleted report/label layout object unplaced restore should keep preview bounds available");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1653: stable edited deleted report/label layout object unplaced restore should expand preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1893: stable edited deleted report/label layout object unplaced restore should preserve preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1653: stable edited deleted report/label layout object unplaced restore should preserve preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1653: stable edited deleted report/label layout object unplaced restore should expand preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 5500",
                        "#1653: stable edited deleted report/label layout object unplaced restore should expand preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 9700",
                        "#1653: stable edited deleted report/label layout object unplaced restore should expand preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1893: stable edited deleted report/label layout object unplaced restore should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1653: stable edited deleted report/label layout object unplaced restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1653: stable edited deleted report/label layout object unplaced restore should not count out-of-band restored objects as placed");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1653: stable edited deleted report/label layout object unplaced restore should add restored objects to unplaced counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1653: stable edited deleted report/label layout object unplaced restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1653: stable edited deleted report/label layout object unplaced restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1653: stable edited deleted report/label layout object unplaced restore should not fabricate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1653: stable edited deleted report/label layout object unplaced restore should serialize null containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 1200",
                "\"right\": 900",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1653: stable edited deleted report/label layout object unplaced restore should refresh selected unplaced geometry without section metadata");
    };

    run_deleted_unplaced_restore(temp_root / "deleted_restore_unplaced_stable.frx",
                                 "deleted_restore_unplaced_stable.frx",
                                 "report");
    run_deleted_unplaced_restore(temp_root / "deleted_restore_unplaced_stable.lbx",
                                 "deleted_restore_unplaced_stable.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

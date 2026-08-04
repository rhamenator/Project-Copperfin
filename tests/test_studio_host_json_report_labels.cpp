// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_label_layout_parity(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto summary_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--json"},
        temp_root);

    if (summary_process.exit_code != 0) {
        std::cerr << "studio host label layout summary stdout:\n"
                  << summary_process.stdout_text << "\n";
        std::cerr << "studio host label layout summary stderr:\n"
                  << summary_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(summary_process.exit_code == 0,
           "#1501: unselected label layout summary JSON should exit successfully");
    expect_contains(summary_process.stdout_text, "\"reportLayout\": {",
                    "#1501: unselected label documents should expose report-layout JSON");
    expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                    "#1501: unselected label layout JSON should retain label identity");
    expect_contains(summary_process.stdout_text, "\"documentTitle\": \"mailing.lbx\"",
                    "#1501: unselected label layout JSON should preserve label document titles");
    expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1516: label layout JSON should expose preview bounds availability");
    expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1516: label layout JSON should expose shared right preview bounds");
    expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1516: label layout JSON should expose shared bottom preview bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1524: label layout JSON should expose deleted preview bounds availability");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1524: label layout JSON should expose deleted preview left bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1524: label layout JSON should expose deleted preview top bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1524: label layout JSON should expose deleted preview right bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1524: label layout JSON should expose deleted preview bottom bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1524: label layout JSON should expose deleted preview width");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1524: label layout JSON should expose deleted preview height");
    expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 3",
                    "#1516: label layout JSON should summarize live placed and unplaced object counts");
    expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 2",
                    "#1522: label layout JSON should summarize section-contained live object counts");
    expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                    "#1523: label layout JSON should summarize deleted objects still inside section bands");
    expect_contains(summary_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    "#1523: label layout JSON should not fabricate deleted unplaced object counts");
    expect_contains(summary_process.stdout_text, "\"objectKindCount\": 3",
                    "#1519: label layout JSON should summarize live object-kind count buckets");
    expect_contains(summary_process.stdout_text, "\"objectKindCounts\": [",
                    "#1519: label layout JSON should expose live object-kind count summaries");
    expect_contains(summary_process.stdout_text, "{\"kind\": \"field\", \"count\": 1}",
                    "#1519: label layout JSON should count live field objects");
    expect_contains(summary_process.stdout_text, "{\"kind\": \"label\", \"count\": 1}",
                    "#1519: label layout JSON should count live label objects");
    expect_contains(summary_process.stdout_text, "{\"kind\": \"line\", \"count\": 1}",
                    "#1519: label layout JSON should count live unplaced line objects");
    expect_contains(summary_process.stdout_text, "\"unplacedObjectKindCount\": 1",
                    "#1519: label layout JSON should summarize unplaced object-kind count buckets");
    expect_contains(summary_process.stdout_text, "\"unplacedObjectKindCounts\": [\n        {\"kind\": \"line\", \"count\": 1}\n      ]",
                    "#1519: label layout JSON should count unplaced line objects");
    expect_contains(summary_process.stdout_text, "\"deletedObjectKindCount\": 1",
                    "#1519: label layout JSON should summarize deleted object-kind count buckets");
    expect_contains(summary_process.stdout_text, "\"deletedObjectKindCounts\": [\n        {\"kind\": \"label\", \"count\": 1}\n      ]",
                    "#1519: label layout JSON should count deleted label objects");
    expect_contains(summary_process.stdout_text, "\"sectionKindCount\": 2",
                    "#1520: label layout JSON should summarize live section band-kind buckets");
    expect_contains(summary_process.stdout_text, "\"sectionKindCounts\": [\n        {\"kind\": \"detail\", \"count\": 1},\n        {\"kind\": \"page_header\", \"count\": 1}\n      ]",
                    "#1520: label layout JSON should count live detail and page-header sections");
    expect_contains(summary_process.stdout_text, "\"deletedSectionKindCount\": 0",
                    "#1520: label layout JSON should not fabricate deleted section band-kind buckets");
    expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 7000",
                    "#1521: label layout JSON should summarize live section heights");
    expect_contains(summary_process.stdout_text, "\"deletedSectionHeightTotal\": 0",
                    "#1521: label layout JSON should not fabricate deleted section heights");
    expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                    "#1517: label layout JSON should expose page setup summary availability");
    expect_contains(summary_process.stdout_text, "\"orientationCode\": 0",
                    "#1517: label layout JSON should expose orientation codes");
    expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 1",
                    "#1517: label layout JSON should expose paper-size codes");
    expect_contains(summary_process.stdout_text, "\"topMargin\": 10",
                    "#1517: label layout JSON should expose top margins");
    expect_contains(summary_process.stdout_text, "\"bottomMargin\": 20",
                    "#1517: label layout JSON should expose bottom margins");
    expect_contains(summary_process.stdout_text, "\"gridVertical\": 4",
                    "#1517: label layout JSON should expose vertical grid spacing");
    expect_contains(summary_process.stdout_text, "\"gridHorizontal\": 8",
                    "#1517: label layout JSON should expose horizontal grid spacing");
    expect_contains(summary_process.stdout_text, "\"settingCount\": 6",
                    "#1501: unselected label layout JSON should summarize live settings");
    expect_contains(summary_process.stdout_text, "\"sectionCount\": 2",
                    "#1501: unselected label layout JSON should summarize live sections");
    expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1501: unselected label layout JSON should summarize deleted objects");
    expect_contains(summary_process.stdout_text, "\"unplacedObjectCount\": 1",
                    "#1501: unselected label layout JSON should summarize unplaced objects");
    expect_contains(summary_process.stdout_text, "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"0\"",
                    "#1501: unselected label layout JSON should expose memo-line setting provenance");
    expect_contains(summary_process.stdout_text, "\"title\": \"Record 5\"",
                    "#1501: unselected label layout JSON should preserve synthesized unplaced-object titles");
    expect_contains(summary_process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                    "#1501: unselected label layout JSON should not fabricate report-selection availability");
    expect_contains(summary_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                    "#1501: unselected label layout JSON should expose the none report-selection kind");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_nudges_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_nudge_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto nudge_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--nudge-object",
            "--nudge-mode", "both",
            "--delta-hpos", "50",
            "--delta-vpos", "-200",
            "--nudge-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (nudge_process.exit_code != 0) {
        std::cerr << "studio host label object nudge stdout:\n" << nudge_process.stdout_text << "\n";
        std::cerr << "studio host label object nudge stderr:\n" << nudge_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(nudge_process.exit_code == 0,
           "#1482: label layout object nudge should exit successfully");
    expect(visual_object_property(label_path, "field-guid", "HPOS") == "1250" &&
               visual_object_property(label_path, "field-guid", "VPOS") == "2400",
           "#1482: label layout object nudge should mutate LBX HPOS and VPOS fields");
    expect_contains(nudge_process.stdout_text, "\"isLabel\": true",
                    "#1482: nudged label layout JSON should retain label identity");
    expect_contains(nudge_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1482: nudged label object JSON should retain selected-object availability");
    expect_contains(nudge_process.stdout_text, "\"left\": 1250",
                    "#1482: nudged label object JSON should expose updated left coordinates");
    expect_contains(nudge_process.stdout_text, "\"top\": 2400",
                    "#1482: nudged label object JSON should expose updated top coordinates");
    expect_contains(nudge_process.stdout_text, "\"right\": 5250",
                    "#1482: nudged label object JSON should recompute right-edge coordinates");
    expect_contains(nudge_process.stdout_text, "\"bottom\": 2850",
                    "#1482: nudged label object JSON should recompute bottom-edge coordinates");
    expect_contains(nudge_process.stdout_text, "\"sectionRelativeTop\": 400",
                    "#1482: nudged label object JSON should recompute section-relative top coordinates");
    expect_contains(nudge_process.stdout_text, "\"sectionRelativeBottom\": 850",
                    "#1482: nudged label object JSON should recompute section-relative bottom coordinates");
    expect_contains(nudge_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1482: nudged label object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_aligns_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_align_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto align_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "label-guid",
            "--align-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (align_process.exit_code != 0) {
        std::cerr << "studio host label object align stdout:\n" << align_process.stdout_text << "\n";
        std::cerr << "studio host label object align stderr:\n" << align_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(align_process.exit_code == 0,
           "#1483: label layout object alignment should exit successfully");
    expect(visual_object_property(label_path, "field-guid", "HPOS") == "900" &&
               visual_object_property(label_path, "field-guid", "VPOS") == "2600",
           "#1483: label layout object left alignment should mutate LBX HPOS and preserve VPOS");
    expect(visual_object_property(label_path, "label-guid", "HPOS") == "900",
           "#1483: label layout object alignment should preserve anchor geometry");
    expect_contains(align_process.stdout_text, "\"isLabel\": true",
                    "#1483: aligned label layout JSON should retain label identity");
    expect_contains(align_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1483: aligned label object JSON should retain selected-object availability");
    expect_contains(align_process.stdout_text, "\"left\": 900",
                    "#1483: aligned label object JSON should expose updated left coordinates");
    expect_contains(align_process.stdout_text, "\"top\": 2600",
                    "#1483: aligned label object JSON should preserve top coordinates");
    expect_contains(align_process.stdout_text, "\"right\": 4900",
                    "#1483: aligned label object JSON should recompute right-edge coordinates");
    expect_contains(align_process.stdout_text, "\"bottom\": 3050",
                    "#1483: aligned label object JSON should preserve bottom-edge coordinates");
    expect_contains(align_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1483: aligned label object JSON should preserve section-relative top coordinates");
    expect_contains(align_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1483: aligned label object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_resizes_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_resize_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto resize_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--resize-object",
            "--resize-mode", "size",
            "--anchor-unique-id", "label-guid",
            "--resize-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (resize_process.exit_code != 0) {
        std::cerr << "studio host label object resize stdout:\n" << resize_process.stdout_text << "\n";
        std::cerr << "studio host label object resize stderr:\n" << resize_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(resize_process.exit_code == 0,
           "#1484: label layout object resize should exit successfully");
    expect(visual_object_property(label_path, "field-guid", "WIDTH") == "1800" &&
               visual_object_property(label_path, "field-guid", "HEIGHT") == "350",
           "#1484: label layout object size resize should mutate LBX WIDTH and HEIGHT fields");
    expect(visual_object_property(label_path, "field-guid", "HPOS") == "1200" &&
               visual_object_property(label_path, "field-guid", "VPOS") == "2600",
           "#1484: label layout object size resize should preserve LBX HPOS and VPOS fields");
    expect_contains(resize_process.stdout_text, "\"isLabel\": true",
                    "#1484: resized label layout JSON should retain label identity");
    expect_contains(resize_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1484: resized label object JSON should retain selected-object availability");
    expect_contains(resize_process.stdout_text, "\"left\": 1200",
                    "#1484: resized label object JSON should preserve left coordinates");
    expect_contains(resize_process.stdout_text, "\"top\": 2600",
                    "#1484: resized label object JSON should preserve top coordinates");
    expect_contains(resize_process.stdout_text, "\"width\": 1800",
                    "#1484: resized label object JSON should expose updated width");
    expect_contains(resize_process.stdout_text, "\"height\": 350",
                    "#1484: resized label object JSON should expose updated height");
    expect_contains(resize_process.stdout_text, "\"right\": 3000",
                    "#1484: resized label object JSON should recompute right-edge coordinates");
    expect_contains(resize_process.stdout_text, "\"bottom\": 2950",
                    "#1484: resized label object JSON should recompute bottom-edge coordinates");
    expect_contains(resize_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1484: resized label object JSON should preserve section-relative top coordinates");
    expect_contains(resize_process.stdout_text, "\"sectionRelativeBottom\": 950",
                    "#1484: resized label object JSON should recompute section-relative bottom coordinates");
    expect_contains(resize_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1484: resized label object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_snaps_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_snap_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto snap_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
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
        std::cerr << "studio host label object snap stdout:\n" << snap_process.stdout_text << "\n";
        std::cerr << "studio host label object snap stderr:\n" << snap_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(snap_process.exit_code == 0,
           "#1485: label layout object snap should exit successfully");
    expect(visual_object_property(label_path, "field-guid", "HPOS") == "1400" &&
               visual_object_property(label_path, "field-guid", "VPOS") == "2250",
           "#1485: label layout object snap should mutate LBX HPOS and VPOS fields");
    expect(visual_object_property(label_path, "field-guid", "WIDTH") == "4000" &&
               visual_object_property(label_path, "field-guid", "HEIGHT") == "450",
           "#1485: label layout object snap should preserve LBX WIDTH and HEIGHT fields");
    expect_contains(snap_process.stdout_text, "\"isLabel\": true",
                    "#1485: snapped label layout JSON should retain label identity");
    expect_contains(snap_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1485: snapped label object JSON should retain selected-object availability");
    expect_contains(snap_process.stdout_text, "\"left\": 1400",
                    "#1485: snapped label object JSON should expose updated left coordinates");
    expect_contains(snap_process.stdout_text, "\"top\": 2250",
                    "#1485: snapped label object JSON should expose updated top coordinates");
    expect_contains(snap_process.stdout_text, "\"width\": 4000",
                    "#1485: snapped label object JSON should preserve width");
    expect_contains(snap_process.stdout_text, "\"height\": 450",
                    "#1485: snapped label object JSON should preserve height");
    expect_contains(snap_process.stdout_text, "\"right\": 5400",
                    "#1485: snapped label object JSON should recompute right-edge coordinates");
    expect_contains(snap_process.stdout_text, "\"bottom\": 2700",
                    "#1485: snapped label object JSON should recompute bottom-edge coordinates");
    expect_contains(snap_process.stdout_text, "\"sectionRelativeTop\": 250",
                    "#1485: snapped label object JSON should recompute section-relative top coordinates");
    expect_contains(snap_process.stdout_text, "\"sectionRelativeBottom\": 700",
                    "#1485: snapped label object JSON should recompute section-relative bottom coordinates");
    expect_contains(snap_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1485: snapped label object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_delete_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--delete-object",
            "--unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host label object delete stdout:\n" << delete_process.stdout_text << "\n";
        std::cerr << "studio host label object delete stderr:\n" << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0,
           "#1486: label layout object delete should exit successfully");
    expect(visual_object_deleted(label_path, "field-guid"),
           "#1486: label layout object delete should mark the LBX object record deleted");
    expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                    "#1486: deleted label layout JSON should retain label identity");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1486: deleted label object JSON should move the object into deleted label objects");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1486: deleted selected label object JSON should remain available");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1486: deleted label objects should preserve containing-section availability");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1486: deleted label objects should serialize containing-section JSON");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1486: deleted label object selections should still classify as report objects");
    expect_contains(delete_process.stdout_text, "\"recordIndex\": 3",
                    "#1486: deleted selected label object JSON should preserve selected record indexes");
    expect_contains(delete_process.stdout_text, "\"deleted\": true",
                    "#1486: deleted selected label object JSON should expose deleted state");
    expect_contains(delete_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1486: deleted label object JSON should preserve containing section ids");
    expect_contains(delete_process.stdout_text, "\"containingSectionRecordIndex\": 2",
                    "#1486: deleted label object JSON should preserve containing section record indexes");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_restore_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);
    const auto seed_delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = label_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "field-guid",
        .deleted = true
    });
    expect(seed_delete_result.ok && visual_object_deleted(label_path, "field-guid"),
           "#1487: label layout restore fixture should start with a deleted label object");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--restore-object",
            "--unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host label object restore stdout:\n" << restore_process.stdout_text << "\n";
        std::cerr << "studio host label object restore stderr:\n" << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0,
           "#1487: label layout object restore should exit successfully");
    expect(!visual_object_deleted(label_path, "field-guid"),
           "#1487: label layout object restore should clear the LBX object deleted flag");
    expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                    "#1487: restored label layout JSON should retain label identity");
    expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1487: restored label object JSON should move the object out of deleted label objects");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1487: restored selected label object JSON should remain available");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1487: restored label objects should advertise containing-section availability");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1487: restored label objects should serialize containing-section JSON");
    expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1487: restored label object selections should classify as report objects");
    expect_contains(restore_process.stdout_text, "\"recordIndex\": 3",
                    "#1487: restored selected label object JSON should preserve selected record indexes");
    expect_contains(restore_process.stdout_text, "\"deleted\": false",
                    "#1487: restored selected label object JSON should expose live state");
    expect_contains(restore_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1487: restored label object JSON should expose containing section ids again");
    expect_contains(restore_process.stdout_text, "\"containingSectionRecordIndex\": 2",
                    "#1487: restored label object JSON should expose containing section record indexes again");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_distribute_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_distribution_json(label_path);

    const auto distribute_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
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
        std::cerr << "studio host label object distribute stdout:\n" << distribute_process.stdout_text << "\n";
        std::cerr << "studio host label object distribute stderr:\n" << distribute_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(distribute_process.exit_code == 0,
           "#1488: label layout object distribution should exit successfully");
    expect(visual_object_property(label_path, "left-field-guid", "HPOS") == "100" &&
               visual_object_property(label_path, "middle-field-guid", "HPOS") == "400" &&
               visual_object_property(label_path, "right-field-guid", "HPOS") == "700",
           "#1488: label layout object distribution should evenly position the middle LBX object");
    expect(visual_object_property(label_path, "middle-field-guid", "VPOS") == "2600" &&
               visual_object_property(label_path, "middle-field-guid", "WIDTH") == "50" &&
               visual_object_property(label_path, "middle-field-guid", "HEIGHT") == "200",
           "#1488: label layout object horizontal distribution should preserve LBX vertical geometry and size");
    expect_contains(distribute_process.stdout_text, "\"isLabel\": true",
                    "#1488: distributed label layout JSON should retain label identity");
    expect_contains(distribute_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1488: distributed label object JSON should retain selected-object availability");
    expect_contains(distribute_process.stdout_text, "\"left\": 400",
                    "#1488: distributed label object JSON should expose updated left coordinates");
    expect_contains(distribute_process.stdout_text, "\"right\": 450",
                    "#1488: distributed label object JSON should recompute right-edge coordinates");
    expect_contains(distribute_process.stdout_text, "\"sectionObjectIndex\": 1",
                    "#1488: distributed label object JSON should preserve sorted section object order");
    expect_contains(distribute_process.stdout_text, "\"sectionObjectCount\": 3",
                    "#1488: distributed label object JSON should expose containing section object counts");
    expect_contains(distribute_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    "#1488: distributed label object JSON should preserve containing section metadata");
    expect_contains(distribute_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1488: distributed label object JSON should keep selected containing-section availability");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_reorder_json(label_path);

    const auto reorder_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--reorder-object",
            "--unique-id", "right-field-guid",
            "--placement", "before",
            "--target-unique-id", "left-field-guid",
            "--json"
        },
        temp_root);

    if (reorder_process.exit_code != 0) {
        std::cerr << "studio host label object reorder stdout:\n" << reorder_process.stdout_text << "\n";
        std::cerr << "studio host label object reorder stderr:\n" << reorder_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(reorder_process.exit_code == 0,
           "#1489: label layout object reorder should exit successfully");
    expect(visual_object_order(label_path) == "right-field-guid,left-field-guid,middle-field-guid",
           "#1489: label layout object reorder should update physical LBX record order");
    expect_contains(reorder_process.stdout_text, "\"isLabel\": true",
                    "#1489: reordered label layout JSON should retain label identity");
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
        "#1489: label layout JSON should serialize tied-geometry section objects in reordered record order");
    expect_contains(reorder_process.stdout_text, "\"sectionObjectCount\": 3",
                    "#1489: reordered label object JSON should expose containing section object counts");
    expect_contains(reorder_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    "#1489: reordered label object JSON should preserve containing section metadata");
    expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1489: reordered label object JSON should keep selected containing-section availability");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_reorder_json(label_path);
    const std::size_t before_count = visual_object_count(label_path);

    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--duplicate-object",
            "--unique-id", "middle-field-guid",
            "--new-unique-id", "middle-copy-guid",
            "--json"
        },
        temp_root);

    if (duplicate_process.exit_code != 0) {
        std::cerr << "studio host label object duplicate stdout:\n" << duplicate_process.stdout_text << "\n";
        std::cerr << "studio host label object duplicate stderr:\n" << duplicate_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(duplicate_process.exit_code == 0,
           "#1490: label layout object duplicate should exit successfully");
    expect(visual_object_count(label_path) == before_count + 1U,
           "#1490: label layout object duplicate should append one LBX object record");
    expect(visual_object_exists(label_path, "middle-copy-guid"),
           "#1490: label layout object duplicate should persist replacement unique ids");
    expect(visual_object_order(label_path) == "left-field-guid,middle-field-guid,right-field-guid,middle-copy-guid",
           "#1490: label layout object duplicate should append the copied LBX object after existing layout objects");
    expect_contains(duplicate_process.stdout_text, "\"isLabel\": true",
                    "#1490: duplicated label layout JSON should retain label identity");
    expect_contains_in_order(
        duplicate_process.stdout_text,
        {
            "\"recordIndex\": 5",
            "\"containingSectionId\": \"detail_1\"",
            "\"sectionObjectIndex\": 3",
            "\"sectionObjectCount\": 4",
            "\"expression\": \"middle.value\""
        },
        "#1490: label layout JSON should expose the duplicated object in refreshed section membership");
    expect_contains(duplicate_process.stdout_text, "\"sectionObjectCount\": 4",
                    "#1490: duplicated label object JSON should refresh containing section object counts");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_label_layout_object_identity_by_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_reorder_json(label_path);
    const std::size_t before_count = visual_object_count(label_path);

    const auto rename_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--rename-object",
            "--unique-id", "right-field-guid",
            "--new-unique-id", "renamed-right-guid",
            "--json"
        },
        temp_root);

    if (rename_process.exit_code != 0) {
        std::cerr << "studio host label object rename stdout:\n" << rename_process.stdout_text << "\n";
        std::cerr << "studio host label object rename stderr:\n" << rename_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(rename_process.exit_code == 0,
           "#1491: label layout object rename should exit successfully");
    expect(visual_object_count(label_path) == before_count,
           "#1491: label layout object rename should preserve LBX object count");
    expect(!visual_object_exists(label_path, "right-field-guid"),
           "#1491: label layout object rename should remove the old unique id");
    expect(visual_object_exists(label_path, "renamed-right-guid"),
           "#1491: label layout object rename should persist replacement unique ids");
    expect(visual_object_order(label_path) == "left-field-guid,middle-field-guid,renamed-right-guid",
           "#1491: label layout object rename should preserve physical LBX object order");
    expect_contains(rename_process.stdout_text, "\"isLabel\": true",
                    "#1491: renamed label layout JSON should retain label identity");
    expect_contains_in_order(
        rename_process.stdout_text,
        {
            "\"recordIndex\": 4",
            "\"containingSectionId\": \"detail_1\"",
            "\"sectionObjectIndex\": 2",
            "\"sectionObjectCount\": 3",
            "\"expression\": \"right.value\""
        },
        "#1491: label layout JSON should keep the renamed object in refreshed section membership");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_label_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    test_studio_host_json_updates_report_layout_object_expressions_by_record_selection(studio_host_path);
}

void test_studio_host_json_updates_label_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path) {
    test_studio_host_json_updates_report_layout_object_expression_by_stable_selection(studio_host_path);
}

void test_studio_host_json_updates_deleted_label_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    test_studio_host_json_updates_deleted_report_layout_object_expressions_by_record_selection(studio_host_path);
}

void test_studio_host_json_clears_label_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    test_studio_host_json_clears_report_layout_object_expressions_by_record_selection(studio_host_path);
}

void test_studio_host_json_clears_label_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path) {
    test_studio_host_json_clears_report_layout_object_expression_by_stable_selection(studio_host_path);
}

void test_studio_host_json_clears_deleted_label_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    test_studio_host_json_clears_deleted_report_layout_object_expressions_by_record_selection(studio_host_path);
}

void test_studio_host_json_restores_edited_deleted_label_layout_object_as_unplaced_by_record_selection(
    const std::string& studio_host_path) {
    test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_record_selection(
        studio_host_path);
}

void test_studio_host_json_restores_edited_deleted_label_layout_object_as_unplaced_by_stable_selection(
    const std::string& studio_host_path) {
    test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_stable_selection(
        studio_host_path);
}

}  // namespace cf_test_studio_host_json

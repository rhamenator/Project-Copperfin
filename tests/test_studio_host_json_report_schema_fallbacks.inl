#if !defined(COPPERFIN_REPORT_SCHEMA_FALLBACK_SKIP_HOST_SMOKE)
void test_studio_host_json_defaults_missing_report_object_objcode_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_object_objcode_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_object_objcode_layout = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_missing_object_objcode_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing object OBJCODE summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing object OBJCODE summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1729: missing object OBJCODE schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1729: missing object OBJCODE layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1729: missing object OBJCODE label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1729: missing object OBJCODE live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 300",
                        "#1729: missing object OBJCODE live preview top should come from the object");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 390",
                        "#1729: missing object OBJCODE live preview bottom should come from the object");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1729: missing object OBJCODE deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 620",
                        "#1729: missing object OBJCODE deleted preview top should come from the deleted object");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 740",
                        "#1729: missing object OBJCODE deleted preview bottom should come from the deleted object");
        expect_missing_object_objcode_preview_bounds(
            summary_process.stdout_text,
            "#2343: missing object OBJCODE summary JSON");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1729: missing object OBJCODE layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1729: missing object OBJCODE layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 0",
                        "#1729: missing object OBJCODE layouts should not fabricate live section membership");
        expect_contains(summary_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1729: missing object OBJCODE layouts should count sectionless live objects");
        expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 0",
                        "#1729: missing object OBJCODE layouts should not fabricate deleted section membership");
        expect_contains(summary_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                        "#1729: missing object OBJCODE layouts should count sectionless deleted objects");
        expect_contains(summary_process.stdout_text, "\"objectKindCounts\": [\n        {\"kind\": \"field\", \"count\": 1}\n      ]",
                        "#1729: missing live object OBJCODE layouts should preserve OBJTYPE-derived kinds");
        expect_contains(summary_process.stdout_text, "\"deletedObjectKindCounts\": [\n        {\"kind\": \"label\", \"count\": 1}\n      ]",
                        "#1729: missing deleted object OBJCODE layouts should preserve OBJTYPE-derived kinds");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"unplacedObjects\": [",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"objectTypeCode\": 8",
                "\"objectTypeFieldIndex\": 0",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.name\"",
                "\"titleFieldIndex\": 1",
                "\"expression\": \"customer.name\"",
                "\"expressionFieldIndex\": 1",
                "\"left\": 120",
                "\"top\": 300",
                "\"width\": 700",
                "\"right\": 820",
                "\"height\": 90",
                "\"bottom\": 390"
            },
            "#1729: missing live object OBJCODE should serialize default object-code provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"objectTypeCode\": 5",
                "\"objectTypeFieldIndex\": 0",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"objectKind\": \"label\"",
                "\"title\": \"\\\"Deleted no objcode\\\"\"",
                "\"titleFieldIndex\": 1",
                "\"expression\": \"\\\"Deleted no objcode\\\"\"",
                "\"expressionFieldIndex\": 1",
                "\"left\": 260",
                "\"top\": 620",
                "\"width\": 500",
                "\"right\": 760",
                "\"height\": 120",
                "\"bottom\": 740"
            },
            "#1729: missing deleted object OBJCODE should serialize default object-code provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1729: missing object OBJCODE live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1729: missing object OBJCODE live selection should advertise selected objects");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1729: missing object OBJCODE live selection should expose object selection kind");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1729: missing object OBJCODE live selection should not fabricate containing sections");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1729: missing object OBJCODE live selection should serialize null containing sections");
        expect_missing_object_objcode_preview_bounds(
            live_process.stdout_text,
            "#2316: selected missing object OBJCODE live JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionObjectIndex\": null",
                "\"objectTypeCode\": 8",
                "\"objectTypeFieldIndex\": 0",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.name\"",
                "\"titleFieldIndex\": 1",
                "\"expression\": \"customer.name\"",
                "\"expressionFieldIndex\": 1",
                "\"left\": 120",
                "\"top\": 300",
                "\"width\": 700",
                "\"right\": 820",
                "\"height\": 90",
                "\"bottom\": 390"
            },
            "#1729: missing live object OBJCODE selection should expose default object-code metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1729: missing object OBJCODE deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1729: missing object OBJCODE deleted selection should advertise selected objects");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1729: missing object OBJCODE deleted selection should expose object selection kind");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1729: missing object OBJCODE deleted selection should not fabricate containing sections");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1729: missing object OBJCODE deleted selection should serialize null containing sections");
        expect_missing_object_objcode_preview_bounds(
            deleted_process.stdout_text,
            "#2316: selected missing object OBJCODE deleted JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionObjectIndex\": null",
                "\"objectTypeCode\": 5",
                "\"objectTypeFieldIndex\": 0",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"objectKind\": \"label\"",
                "\"title\": \"\\\"Deleted no objcode\\\"\"",
                "\"titleFieldIndex\": 1",
                "\"expression\": \"\\\"Deleted no objcode\\\"\"",
                "\"expressionFieldIndex\": 1",
                "\"left\": 260",
                "\"top\": 620",
                "\"width\": 500",
                "\"right\": 760",
                "\"height\": 120",
                "\"bottom\": 740"
            },
            "#1729: missing deleted object OBJCODE selection should expose default object-code metadata");
    };

    run_missing_object_objcode_layout(temp_root / "missing_object_objcode.frx",
                                      "missing_object_objcode.frx",
                                      "report");
    run_missing_object_objcode_layout(temp_root / "missing_object_objcode.lbx",
                                      "missing_object_objcode.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_REPORT_UNRESOLVED_MEMO_SKIP_HOST_SMOKE)
void test_studio_host_json_suppresses_unresolved_deleted_report_object_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_deleted_report_object_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_unresolved_deleted_object_memo_layout = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_unresolved_deleted_object_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved deleted object memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved deleted object memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1738: unresolved deleted object memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1738: unresolved deleted object memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1738: unresolved deleted object memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1738: unresolved deleted object memo layouts should not fabricate live objects");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1738: unresolved deleted object memo layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1738: unresolved deleted object memo layouts should preserve deleted placed-object counts");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 1\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2",
                "\"left\": 1200",
                "\"top\": 2600",
                "\"width\": 4000",
                "\"height\": 450",
                "\"highlightCount\": 0"
            },
            "#1738: unresolved deleted object memo summary should suppress expression/highlight text while preserving object metadata");
        expect_unresolved_deleted_object_memo_preview_bounds(
            summary_process.stdout_text,
            "#2334: unresolved deleted object memo summary JSON");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1738: unresolved deleted object memo placeholders should not leak into summary JSON");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1738: unresolved deleted object memo selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1738: unresolved deleted object memo selection should advertise selected objects");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1738: unresolved deleted object memo selection should expose object selection kind");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1738: unresolved deleted object memo selection should preserve containing-section metadata");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_0\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 1\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2",
                "\"left\": 1200",
                "\"top\": 2600",
                "\"width\": 4000",
                "\"height\": 450",
                "\"highlightCount\": 0"
            },
            "#1738: unresolved deleted object memo selection should suppress expression/highlight text while preserving object metadata");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_0\"",
                "\"title\": \"Detail\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#1738: unresolved deleted object memo selection should expose containing detail-band metadata");
        expect_unresolved_deleted_object_memo_preview_bounds(
            deleted_process.stdout_text,
            "#2334: selected unresolved deleted object memo JSON");
        expect_not_contains(deleted_process.stdout_text, "<memo block",
                            "#1738: unresolved deleted object memo placeholders should not leak into selection JSON");
    };

    run_unresolved_deleted_object_memo_layout(temp_root / "unresolved_deleted_object_memo.frx",
                                              "unresolved_deleted_object_memo.frx",
                                              "report");
    run_unresolved_deleted_object_memo_layout(temp_root / "unresolved_deleted_object_memo.lbx",
                                              "unresolved_deleted_object_memo.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_suppresses_unresolved_unplaced_report_object_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_unplaced_report_object_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_unresolved_unplaced_object_memo_layout = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_synthetic_report_table_for_unresolved_unplaced_object_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved unplaced object memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved unplaced object memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1739: unresolved unplaced object memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1739: unresolved unplaced object memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1739: unresolved unplaced object memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1739: unresolved unplaced object memo layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 0",
                        "#1739: unresolved unplaced object memo layouts should not fabricate placed objects");
        expect_contains(summary_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1739: unresolved unplaced object memo layouts should preserve unplaced object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1739: unresolved unplaced object memo layouts should not fabricate deleted objects");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"unplacedObjects\": [",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 0\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2",
                "\"left\": 1200",
                "\"top\": 2600",
                "\"width\": 4000",
                "\"height\": 450",
                "\"highlightCount\": 0"
            },
            "#1739: unresolved unplaced object memo summary should suppress expression/highlight text while preserving object metadata");
        expect_unresolved_unplaced_object_memo_preview_bounds(
            summary_process.stdout_text,
            "#2335: unresolved unplaced object memo summary JSON");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1739: unresolved unplaced object memo placeholders should not leak into summary JSON");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(object_process.exit_code == 0,
               "#1739: unresolved unplaced object memo selection should keep inspection non-failing");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1739: unresolved unplaced object memo selection should advertise selected objects");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1739: unresolved unplaced object memo selection should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1739: unresolved unplaced object memo selection should not fabricate containing sections");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1739: unresolved unplaced object memo selection should serialize null containing sections");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 0\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2",
                "\"left\": 1200",
                "\"top\": 2600",
                "\"width\": 4000",
                "\"height\": 450",
                "\"highlightCount\": 0"
            },
            "#1739: unresolved unplaced object memo selection should suppress expression/highlight text while preserving object metadata");
        expect_unresolved_unplaced_object_memo_preview_bounds(
            object_process.stdout_text,
            "#2335: selected unresolved unplaced object memo JSON");
        expect_not_contains(object_process.stdout_text, "<memo block",
                            "#1739: unresolved unplaced object memo placeholders should not leak into selection JSON");
    };

    run_unresolved_unplaced_object_memo_layout(temp_root / "unresolved_unplaced_object_memo.frx",
                                               "unresolved_unplaced_object_memo.frx",
                                               "report");
    run_unresolved_unplaced_object_memo_layout(temp_root / "unresolved_unplaced_object_memo.lbx",
                                               "unresolved_unplaced_object_memo.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_REPORT_SCHEMA_FALLBACK_SKIP_HOST_SMOKE)
void test_studio_host_json_preserves_report_objects_without_expr_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_object_expr_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_object_expr_layout = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_missing_object_expr_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing object EXPR summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing object EXPR summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1725: missing object EXPR schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1725: missing object EXPR layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1725: missing object EXPR label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1725: missing object EXPR live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 200",
                        "#1725: missing object EXPR live preview top should include the section band");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 1200",
                        "#1725: missing object EXPR live preview bottom should include the section band");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1725: missing object EXPR deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 260",
                        "#1725: missing object EXPR deleted preview left should come from the deleted object");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 740",
                        "#1725: missing object EXPR deleted preview bottom should come from the deleted object");
        expect_missing_object_expr_preview_bounds(
            summary_process.stdout_text,
            "#2339: missing object EXPR summary JSON");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1725: missing object EXPR layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1725: missing object EXPR layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1725: missing object EXPR layouts should keep live object section membership");
        expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1725: missing object EXPR layouts should preserve deleted placed-object counts");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1725: missing object EXPR layouts should preserve the containing live section");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1725: missing object EXPR layouts should not fabricate deleted sections");
        expect_contains(summary_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                        "#1725: missing object EXPR layouts should not fabricate deleted unplaced objects");
        expect_contains(summary_process.stdout_text, "\"objectKindCounts\": [\n        {\"kind\": \"field\", \"count\": 1}\n      ]",
                        "#1725: missing live object EXPR layouts should preserve object-kind counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectKindCounts\": [\n        {\"kind\": \"label\", \"count\": 1}\n      ]",
                        "#1725: missing deleted object EXPR layouts should preserve object-kind counts");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"missing-expr-detail-section-guid\"",
                "\"objects\": [",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"containingSectionId\": \"missing-expr-detail-section-guid\"",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"missing-expr-live-object-guid\"",
                "\"titleFieldIndex\": 6",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 120",
                "\"top\": 300",
                "\"width\": 700",
                "\"right\": 820",
                "\"height\": 90",
                "\"bottom\": 390",
                "\"highlightCount\": 0"
            },
            "#1725: missing live object EXPR layouts should serialize null expression provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"missing-expr-detail-section-guid\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 420",
                "\"sectionRelativeBottom\": 540",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"title\": \"missing-expr-deleted-object-guid\"",
                "\"titleFieldIndex\": 6",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 260",
                "\"top\": 620",
                "\"width\": 500",
                "\"right\": 760",
                "\"height\": 120",
                "\"bottom\": 740",
                "\"highlightCount\": 0"
            },
            "#1725: missing deleted object EXPR layouts should serialize null expression provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1725: missing object EXPR live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1725: missing object EXPR live selection should advertise selected objects");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1725: missing object EXPR live selection should expose object selection kind");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1725: missing object EXPR live selection should expose containing-section metadata");
        expect_missing_object_expr_preview_bounds(
            live_process.stdout_text,
            "#2339: selected missing live object EXPR JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"containingSectionId\": \"missing-expr-detail-section-guid\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 190",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"missing-expr-live-object-guid\"",
                "\"titleFieldIndex\": 6",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 120",
                "\"top\": 300",
                "\"width\": 700",
                "\"right\": 820",
                "\"height\": 90",
                "\"bottom\": 390",
                "\"highlightCount\": 0"
            },
            "#1725: missing live object EXPR selection should expose object metadata with null expression provenance");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"missing-expr-detail-section-guid\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"top\": 200",
                "\"height\": 1000",
                "\"bottom\": 1200"
            },
            "#1725: missing live object EXPR selection should expose containing section metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1725: missing object EXPR deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1725: missing object EXPR deleted selection should advertise selected objects");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1725: missing object EXPR deleted selection should expose object selection kind");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1725: missing object EXPR deleted selection should preserve containing sections");
        expect_missing_object_expr_preview_bounds(
            deleted_process.stdout_text,
            "#2339: selected missing deleted object EXPR JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"missing-expr-detail-section-guid\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 420",
                "\"sectionRelativeBottom\": 540",
                "\"sectionObjectIndex\": 0",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"title\": \"missing-expr-deleted-object-guid\"",
                "\"titleFieldIndex\": 6",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 260",
                "\"top\": 620",
                "\"width\": 500",
                "\"right\": 760",
                "\"height\": 120",
                "\"bottom\": 740",
                "\"highlightCount\": 0"
            },
            "#1725: missing deleted object EXPR selection should expose object metadata with null expression provenance");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"missing-expr-detail-section-guid\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"top\": 200",
                "\"height\": 1000",
                "\"bottom\": 1200"
            },
            "#1725: missing deleted object EXPR selection should expose containing section metadata");
    };

    run_missing_object_expr_layout(temp_root / "missing_object_expr.frx",
                                   "missing_object_expr.frx",
                                   "report");
    run_missing_object_expr_layout(temp_root / "missing_object_expr.lbx",
                                   "missing_object_expr.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_synthesizes_report_object_titles_without_title_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_object_title_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_object_title_layout = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_missing_object_title_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing object title summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing object title summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1726: missing object title schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1726: missing object title layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1726: missing object title label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1726: missing object title layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1726: missing object title layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1726: missing object title layouts should keep live object section membership");
        expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1726: missing object title layouts should preserve deleted placed-object counts");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1726: missing object title layouts should preserve the containing live section");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1726: missing object title layouts should not fabricate deleted sections");
        expect_contains(summary_process.stdout_text, "\"objectKindCounts\": [\n        {\"kind\": \"field\", \"count\": 1}\n      ]",
                        "#1726: missing live object title layouts should preserve object-kind counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectKindCounts\": [\n        {\"kind\": \"label\", \"count\": 1}\n      ]",
                        "#1726: missing deleted object title layouts should preserve object-kind counts");
        expect_missing_object_title_preview_bounds(
            summary_process.stdout_text,
            "#2340: missing object title summary JSON");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"detail_0\"",
                "\"objects\": [",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_0\"",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 1\"",
                "\"titleFieldIndex\": null",
                "\"titleMemoBlockNumber\": 0",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 140",
                "\"top\": 220",
                "\"width\": 420",
                "\"right\": 560",
                "\"height\": 80",
                "\"bottom\": 300"
            },
            "#1726: missing live object title layouts should synthesize titles with null provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_0\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 400",
                "\"sectionRelativeBottom\": 510",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"title\": \"Record 2\"",
                "\"titleFieldIndex\": null",
                "\"titleMemoBlockNumber\": 0",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 360",
                "\"top\": 500",
                "\"width\": 380",
                "\"right\": 740",
                "\"height\": 110",
                "\"bottom\": 610"
            },
            "#1726: missing deleted object title layouts should synthesize titles with null provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1726: missing object title live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1726: missing object title live selection should advertise selected objects");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1726: missing object title live selection should expose object selection kind");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1726: missing object title live selection should expose containing-section metadata");
        expect_missing_object_title_preview_bounds(
            live_process.stdout_text,
            "#2340: selected missing live object title JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_0\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 120",
                "\"sectionRelativeBottom\": 200",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 1\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"left\": 140",
                "\"top\": 220",
                "\"width\": 420",
                "\"right\": 560",
                "\"height\": 80",
                "\"bottom\": 300"
            },
            "#1726: missing live object title selection should expose synthesized title metadata");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_0\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"top\": 100",
                "\"height\": 800",
                "\"bottom\": 900"
            },
            "#1726: missing live object title selection should expose containing section metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1726: missing object title deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1726: missing object title deleted selection should advertise selected objects");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1726: missing object title deleted selection should expose object selection kind");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1726: missing object title deleted selection should preserve containing sections");
        expect_missing_object_title_preview_bounds(
            deleted_process.stdout_text,
            "#2340: selected missing deleted object title JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_0\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 400",
                "\"sectionRelativeBottom\": 510",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"title\": \"Record 2\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"left\": 360",
                "\"top\": 500",
                "\"width\": 380",
                "\"right\": 740",
                "\"height\": 110",
                "\"bottom\": 610"
            },
            "#1726: missing deleted object title selection should expose synthesized title metadata");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_0\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"top\": 100",
                "\"height\": 800",
                "\"bottom\": 900"
            },
            "#1726: missing deleted object title selection should expose containing section metadata");
    };

    run_missing_object_title_layout(temp_root / "missing_object_title.frx",
                                    "missing_object_title.frx",
                                    "report");
    run_missing_object_title_layout(temp_root / "missing_object_title.lbx",
                                    "missing_object_title.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

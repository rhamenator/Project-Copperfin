#include "copperfin/studio/report_layout.h"

#include "copperfin/localization/localization.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

copperfin::vfp::DbfRecordValue value(std::string name, std::string display_value, std::uint32_t memo_block_number = 0U) {
    return {
        .field_name = std::move(name),
        .field_type = 'C',
        .is_null = false,
        .display_value = std::move(display_value),
        .memo_block_number = memo_block_number
    };
}

void test_build_report_layout_groups_band_objects() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "synthetic.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=0\r\nPAPERSIZE=1", 9U),
                value("TOPMARGIN", "10"),
                value("BOTMARGIN", "12")
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "1", 41U),
                value("VPOS", "0.000", 42U),
                value("HEIGHT", "2000.000", 43U)
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "4"),
                value("VPOS", "2000.000"),
                value("HEIGHT", "5000.000")
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "8", 301U),
                value("OBJCODE", "0", 302U),
                value("EXPR", "customer.company", 31U),
                value("HPOS", "1200.000", 303U),
                value("VPOS", "2600.000", 304U),
                value("WIDTH", "4000.000", 305U),
                value("HEIGHT", "450.000", 306U),
                value("FONTFACE", "Segoe UI", 12U),
                value("FONTSIZE", "10")
            }
        },
        {
            .record_index = 4U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "5"),
                value("EXPR", "\"Invoice\"", 32U),
                value("HPOS", "900.000"),
                value("VPOS", "100.000"),
                value("WIDTH", "1800.000"),
                value("HEIGHT", "350.000")
            }
        },
        {
            .record_index = 5U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "6"),
                value("HPOS", "50.000"),
                value("VPOS", "8000.000"),
                value("WIDTH", "100.000"),
                value("HEIGHT", "100.000")
            }
        },
        {
            .record_index = 6U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "5", 601U),
                value("EXPR", "\"Deleted label\"", 33U),
                value("HPOS", "1000.000", 603U),
                value("VPOS", "2600.000", 604U),
                value("WIDTH", "1200.000", 605U),
                value("HEIGHT", "300.000", 606U)
            }
        },
        {
            .record_index = 7U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "8", 81U),
                value("VPOS", "9000.000", 82U),
                value("HEIGHT", "700.000", 83U)
            }
        },
        {
            .record_index = 8U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "DELETEDSETTING=1", 19U),
                value("TOPMARGIN", "99")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "report layout should be available for report assets");
    expect(!layout.is_label, "report layout should distinguish reports from labels");
    expect(layout.document_title == "synthetic.frx", "#728: report layout document titles should mirror Studio display names");
    expect(layout.document_title_field_index == copperfin::studio::StudioReportMissingFieldIndex,
        "#728: display-name report titles should use missing DBF field provenance");
    expect(layout.document_title_memo_block_number == 0U,
        "#728: display-name report titles should expose memo block zero");
    expect(layout.sections.size() == 2U, "report layout should detect two sections");
    expect(layout.settings.size() >= 2U, "report layout should parse root settings");
    expect(layout.sections[0].band_kind == "page_header", "first section should decode the page header band");
    expect(layout.sections[1].band_kind == "detail", "second section should decode the detail band");
    expect(layout.sections[0].title == "Page Header", "#2489: default report section titles should preserve en-US prose");
    expect(layout.sections[1].title == "Detail", "#2489: default detail section titles should preserve en-US prose");
    expect(layout.sections[0].section_index == 0U, "#1460: report sections should carry zero-based order");
    expect(layout.sections[0].section_count == 2U, "#1460: report sections should carry live section counts");
    expect(layout.sections[1].section_index == 1U, "#1460: later report sections should carry sorted order");
    expect(layout.sections[1].section_count == 2U, "#1460: later report sections should carry live section counts");
    expect(layout.sections[0].bottom == 2000, "#1461: report sections should carry bottom-edge coordinates");
    expect(layout.sections[1].bottom == 7000, "#1461: later report sections should carry bottom-edge coordinates");
    expect(layout.sections[0].id == "page_header_1", "#729: report section ids should remain synthesized from band and record");
    expect(layout.sections[0].id_field_index == copperfin::studio::StudioReportMissingFieldIndex,
        "#729: synthesized report section ids should use missing DBF field provenance");
    expect(layout.sections[0].id_memo_block_number == 0U,
        "#729: synthesized report section ids should expose memo block zero");
    expect(layout.sections[0].objcode_code == 1, "#666: report sections should preserve raw OBJCODE values");
    expect(layout.sections[0].objcode_field_index == 1U, "#664: report section should preserve OBJCODE field provenance");
    expect(layout.sections[0].objcode_memo_block_number == 41U, "#722: report sections should preserve OBJCODE memo block provenance");
    expect(layout.sections[0].title_field_index == 1U, "#682: report section title provenance should retain OBJCODE field ordinal");
    expect(layout.sections[0].title_memo_block_number == 41U, "#722: report section titles should inherit OBJCODE memo block provenance");
    expect(layout.sections[0].band_kind_field_index == 1U, "#682: report section band-kind provenance should retain OBJCODE field ordinal");
    expect(layout.sections[0].band_kind_memo_block_number == 41U, "#722: report section band kinds should inherit OBJCODE memo block provenance");
    expect(layout.sections[0].top_field_index == 2U, "#664: report section should preserve VPOS field provenance");
    expect(layout.sections[0].top_memo_block_number == 42U, "#722: report section top geometry should preserve VPOS memo block provenance");
    expect(layout.sections[0].height_field_index == 3U, "#664: report section should preserve HEIGHT field provenance");
    expect(layout.sections[0].height_memo_block_number == 43U, "#722: report section height geometry should preserve HEIGHT memo block provenance");
    expect(layout.sections[0].objects.size() == 1U, "page header should capture its label object");
    if (!layout.sections[0].objects.empty()) {
        expect(layout.sections[0].objects[0].objtype_field_index == 0U, "#674: present report object fields should keep DBF field provenance");
        expect(layout.sections[0].objects[0].objtype_memo_block_number == 0U,
            "#723: live report object OBJTYPE without memo provenance should expose block zero");
        expect(layout.sections[0].objects[0].object_kind_memo_block_number == 0U,
            "#723: report object kind should inherit the OBJTYPE memo block provenance");
        expect(layout.sections[0].objects[0].objcode_field_index == copperfin::studio::StudioReportMissingFieldIndex,
            "#674: missing report object fields should not masquerade as DBF field zero");
        expect(layout.sections[0].objects[0].objcode_memo_block_number == 0U,
            "#723: missing report object OBJCODE should expose memo block zero");
        expect(layout.sections[0].objects[0].title == "\"Invoice\"", "#675: report layout object titles should keep existing EXPR fallback");
        expect(layout.sections[0].objects[0].title_field_index == 1U,
            "#675: report layout object title provenance should retain selected EXPR field ordinal");
        expect(layout.sections[0].objects[0].title_memo_block_number == 32U,
            "#716: report layout object titles should inherit selected EXPR memo block provenance");
    }
    expect(layout.sections[1].objects.size() == 1U, "detail section should capture its field object");
    expect(layout.sections[1].objects[0].object_kind == "field", "detail object should retain its type");
    expect(layout.sections[1].objects[0].containing_section_id == "detail_2",
        "#1458: section-contained report objects should carry containing section ids");
    expect(layout.sections[1].objects[0].containing_section_record_index == 2U,
        "#1458: section-contained report objects should carry containing section record indexes");
    expect(layout.sections[1].objects[0].section_relative_top == 600,
        "#1458: section-contained report objects should carry top coordinates relative to their band");
    expect(layout.sections[1].objects[0].section_relative_bottom == 1050,
        "#1461: section-contained report objects should carry bottom coordinates relative to their band");
    expect(layout.sections[1].objects[0].section_object_index == 0U,
        "#1459: section-contained report objects should carry zero-based order within their section");
    expect(layout.sections[1].objects[0].section_object_count == 1U,
        "#1459: section-contained report objects should carry containing section object counts");
    expect(layout.sections[1].objects[0].object_kind_field_index == 0U, "#682: report object kind provenance should retain OBJTYPE field ordinal");
    expect(layout.sections[1].objects[0].object_kind_memo_block_number == 301U,
        "#723: report object kind should inherit OBJTYPE memo block provenance");
    expect(layout.sections[1].objects[0].objtype_code == 8, "#666: layout objects should preserve raw OBJTYPE values");
    expect(layout.sections[1].objects[0].objcode_code == 0, "#666: layout objects should preserve raw OBJCODE values");
    expect(layout.sections[1].objects[0].expression == "customer.company", "detail object should surface its expression");
    expect(layout.sections[1].objects[0].objtype_field_index == 0U, "#665: layout objects should preserve OBJTYPE field provenance");
    expect(layout.sections[1].objects[0].objtype_memo_block_number == 301U,
        "#723: layout objects should preserve OBJTYPE memo block provenance");
    expect(layout.sections[1].objects[0].objcode_field_index == 1U, "#666: layout objects should preserve OBJCODE field provenance");
    expect(layout.sections[1].objects[0].objcode_memo_block_number == 302U,
        "#723: layout objects should preserve OBJCODE memo block provenance");
    expect(layout.sections[1].objects[0].title_field_index == 2U, "#675: detail object title provenance should retain selected EXPR field ordinal");
    expect(layout.sections[1].objects[0].title_memo_block_number == 31U, "#716: detail object title should inherit EXPR memo block provenance");
    expect(layout.sections[1].objects[0].expression_field_index == 2U, "#665: layout objects should preserve EXPR field provenance");
    expect(layout.sections[1].objects[0].expression_memo_block_number == 31U, "#716: layout object expressions should retain EXPR memo block provenance");
    expect(layout.sections[1].objects[0].left_field_index == 3U, "#665: layout objects should preserve HPOS field provenance");
    expect(layout.sections[1].objects[0].left_memo_block_number == 303U,
        "#723: layout objects should preserve HPOS memo block provenance");
    expect(layout.sections[1].objects[0].top_field_index == 4U, "#665: layout objects should preserve VPOS field provenance");
    expect(layout.sections[1].objects[0].top_memo_block_number == 304U,
        "#723: layout objects should preserve VPOS memo block provenance");
    expect(layout.sections[1].objects[0].width_field_index == 5U, "#665: layout objects should preserve WIDTH field provenance");
    expect(layout.sections[1].objects[0].width_memo_block_number == 305U,
        "#723: layout objects should preserve WIDTH memo block provenance");
    expect(layout.sections[1].objects[0].right == 5200,
        "#1462: report layout objects should carry right-edge coordinates");
    expect(layout.sections[1].objects[0].height_field_index == 6U, "#665: layout objects should preserve HEIGHT field provenance");
    expect(layout.sections[1].objects[0].height_memo_block_number == 306U,
        "#723: layout objects should preserve HEIGHT memo block provenance");
    expect(layout.sections[1].objects[0].bottom == 3050, "#1461: report layout objects should carry bottom-edge coordinates");
    const auto orientation = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "ORIENTATION";
    });
    expect(orientation != layout.settings.end(), "#661: report settings should include parsed EXPR settings");
    if (orientation != layout.settings.end()) {
        expect(orientation->record_index == 0U, "#661: parsed EXPR settings should retain their source record index");
        expect(orientation->field_index == 2U, "#661: parsed EXPR settings should retain the source EXPR field ordinal");
        expect(orientation->source_line_index == 0U, "#676: parsed EXPR settings should retain their source memo line index");
        expect(orientation->memo_block_number == 9U, "#713: parsed EXPR settings should inherit the source EXPR memo block");
    }
    const auto paper_size = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "PAPERSIZE";
    });
    expect(paper_size != layout.settings.end(), "#676: report settings should include later parsed EXPR settings");
    if (paper_size != layout.settings.end()) {
        expect(paper_size->source_line_index == 1U, "#676: later EXPR settings should retain their source memo line index");
        expect(paper_size->memo_block_number == 9U, "#713: later EXPR settings should inherit the source EXPR memo block");
    }
    const auto top_margin = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "TOPMARGIN";
    });
    expect(top_margin != layout.settings.end(), "#661: report settings should include direct numeric settings");
    if (top_margin != layout.settings.end()) {
        expect(top_margin->record_index == 0U, "#661: direct settings should retain their source record index");
        expect(top_margin->field_index == 3U, "#661: direct settings should retain their DBF field ordinal");
        expect(top_margin->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
            "#676: direct settings should not masquerade as parsed memo-line settings");
        expect(top_margin->memo_block_number == 0U, "#713: direct non-memo settings should expose memo block zero");
    }
    const auto fontface = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "FONTFACE";
        });
    expect(fontface != layout.sections[1].objects[0].highlights.end(), "#661: layout highlights should include FONTFACE");
    if (fontface != layout.sections[1].objects[0].highlights.end()) {
        expect(fontface->record_index == 3U, "#661: layout highlights should retain source record index");
        expect(fontface->field_index == 7U, "#661: layout highlights should retain DBF field ordinal");
        expect(fontface->memo_block_number == 12U, "#713: memo-backed highlights should preserve source memo block provenance");
    }
    expect(layout.unplaced_objects.size() == 1U, "#675: object without matching section should remain unplaced");
    if (!layout.unplaced_objects.empty()) {
        expect(layout.unplaced_objects[0].title == "Record 5", "#675: untitled report layout object should keep synthetic title fallback");
        expect(layout.unplaced_objects[0].containing_section_id.empty(),
            "#1458: unplaced report objects should not fabricate containing section ids");
        expect(layout.unplaced_objects[0].containing_section_record_index == copperfin::studio::StudioReportMissingRecordIndex,
            "#1458: unplaced report objects should expose missing containing section record indexes");
        expect(layout.unplaced_objects[0].section_relative_top == 0,
            "#1458: unplaced report objects should use zero relative section top");
        expect(layout.unplaced_objects[0].section_relative_bottom == 0,
            "#1461: unplaced report objects should use zero relative section bottom");
        expect(layout.unplaced_objects[0].section_object_index == copperfin::studio::StudioReportMissingRecordIndex,
            "#1459: unplaced report objects should expose missing section object indexes");
        expect(layout.unplaced_objects[0].section_object_count == 0U,
            "#1459: unplaced report objects should expose zero section object counts");
        expect(layout.unplaced_objects[0].title_field_index == copperfin::studio::StudioReportMissingFieldIndex,
            "#675: synthesized report layout object title should use the missing-field sentinel");
        expect(layout.unplaced_objects[0].title_memo_block_number == 0U,
            "#716: synthesized report layout object titles should expose memo block zero");
        expect(layout.unplaced_objects[0].bottom == 8100,
            "#1461: unplaced report objects should still carry absolute bottom-edge coordinates");
        expect(layout.unplaced_objects[0].right == 150,
            "#1462: unplaced report objects should still carry right-edge coordinates");
    }
    expect(layout.deleted_objects.size() == 1U, "#689: deleted report layout objects should be preserved separately");
    if (!layout.deleted_objects.empty()) {
        expect(layout.deleted_objects[0].deleted, "#689: deleted report layout object snapshots should retain deleted state");
        expect(layout.deleted_objects[0].containing_section_id.empty(),
            "#1458: deleted report objects should not fabricate containing section ids");
        expect(layout.deleted_objects[0].containing_section_record_index == copperfin::studio::StudioReportMissingRecordIndex,
            "#1458: deleted report objects should expose missing containing section record indexes");
        expect(layout.deleted_objects[0].section_object_index == copperfin::studio::StudioReportMissingRecordIndex,
            "#1459: deleted report objects should expose missing section object indexes");
        expect(layout.deleted_objects[0].section_object_count == 0U,
            "#1459: deleted report objects should expose zero section object counts");
        expect(layout.deleted_objects[0].section_relative_bottom == 0,
            "#1461: deleted report objects should not fabricate relative bottom-edge coordinates");
        expect(layout.deleted_objects[0].title == "\"Deleted label\"", "#689: deleted report layout objects should retain title metadata");
        expect(layout.deleted_objects[0].title_field_index == 1U,
            "#689: deleted report layout objects should retain title provenance");
        expect(layout.deleted_objects[0].title_memo_block_number == 33U,
            "#716: deleted report layout object titles should retain memo block provenance");
        expect(layout.deleted_objects[0].objtype_memo_block_number == 601U,
            "#723: deleted report layout objects should retain OBJTYPE memo block provenance");
        expect(layout.deleted_objects[0].object_kind_memo_block_number == 601U,
            "#723: deleted report layout object kind should inherit OBJTYPE memo block provenance");
        expect(layout.deleted_objects[0].objcode_memo_block_number == 0U,
            "#723: deleted report layout objects with missing OBJCODE should expose block zero");
        expect(layout.deleted_objects[0].left_memo_block_number == 603U,
            "#723: deleted report layout objects should retain HPOS memo block provenance");
        expect(layout.deleted_objects[0].top_memo_block_number == 604U,
            "#723: deleted report layout objects should retain VPOS memo block provenance");
        expect(layout.deleted_objects[0].width_memo_block_number == 605U,
            "#723: deleted report layout objects should retain WIDTH memo block provenance");
        expect(layout.deleted_objects[0].height_memo_block_number == 606U,
            "#723: deleted report layout objects should retain HEIGHT memo block provenance");
        expect(layout.deleted_objects[0].bottom == 2900,
            "#1461: deleted report objects should still carry absolute bottom-edge coordinates");
        expect(layout.deleted_objects[0].right == 2200,
            "#1462: deleted report objects should still carry right-edge coordinates");
    }
    expect(layout.deleted_sections.size() == 1U, "#690: deleted report sections should be preserved separately");
    if (!layout.deleted_sections.empty()) {
        expect(layout.deleted_sections[0].deleted, "#690: deleted report section snapshots should retain deleted state");
        expect(layout.deleted_sections[0].title == "Summary", "#2489: default deleted section titles should preserve en-US prose");
        expect(layout.deleted_sections[0].section_index == copperfin::studio::StudioReportMissingRecordIndex,
            "#1460: deleted report sections should expose missing section order");
        expect(layout.deleted_sections[0].section_count == 0U,
            "#1460: deleted report sections should expose zero live section counts");
        expect(layout.deleted_sections[0].band_kind == "summary", "#690: deleted report sections should retain band metadata");
        expect(layout.deleted_sections[0].id_field_index == copperfin::studio::StudioReportMissingFieldIndex,
            "#729: deleted synthesized section ids should use missing DBF field provenance");
        expect(layout.deleted_sections[0].id_memo_block_number == 0U,
            "#729: deleted synthesized section ids should expose memo block zero");
        expect(layout.deleted_sections[0].objcode_field_index == 1U,
            "#690: deleted report sections should retain OBJCODE provenance");
        expect(layout.deleted_sections[0].objcode_memo_block_number == 81U,
            "#722: deleted report sections should retain OBJCODE memo block provenance");
        expect(layout.deleted_sections[0].top_memo_block_number == 82U,
            "#722: deleted report sections should retain VPOS memo block provenance");
        expect(layout.deleted_sections[0].height_memo_block_number == 83U,
            "#722: deleted report sections should retain HEIGHT memo block provenance");
        expect(layout.deleted_sections[0].bottom == 9700,
            "#1461: deleted report sections should still carry bottom-edge coordinates");
    }
    const auto deleted_setting = std::find_if(layout.deleted_settings.begin(), layout.deleted_settings.end(), [](const auto& setting) {
        return setting.name == "DELETEDSETTING";
    });
    expect(deleted_setting != layout.deleted_settings.end(), "#691: deleted report root settings should be preserved separately");
    if (deleted_setting != layout.deleted_settings.end()) {
        expect(deleted_setting->record_index == 8U, "#691: deleted report root settings should retain record provenance");
        expect(deleted_setting->field_index == 2U, "#691: deleted report root settings should retain EXPR field provenance");
        expect(deleted_setting->source_line_index == 0U, "#691: deleted report root settings should retain memo-line provenance");
        expect(deleted_setting->memo_block_number == 19U, "#713: deleted parsed EXPR settings should retain memo block provenance");
        expect(deleted_setting->value == "1", "#691: deleted report root settings should retain parsed value text");
    }
    const auto deleted_top_margin = std::find_if(layout.deleted_settings.begin(), layout.deleted_settings.end(), [](const auto& setting) {
        return setting.name == "TOPMARGIN";
    });
    expect(deleted_top_margin != layout.deleted_settings.end(), "#691: deleted report root direct settings should be preserved separately");
    if (deleted_top_margin != layout.deleted_settings.end()) {
        expect(deleted_top_margin->record_index == 8U, "#691: deleted direct settings should retain record provenance");
        expect(deleted_top_margin->field_index == 3U, "#691: deleted direct settings should retain DBF field provenance");
        expect(deleted_top_margin->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
            "#691: deleted direct settings should not masquerade as memo-line settings");
        expect(deleted_top_margin->memo_block_number == 0U, "#713: deleted direct non-memo settings should expose memo block zero");
    }
    const auto live_deleted_setting = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "DELETEDSETTING";
    });
    expect(live_deleted_setting == layout.settings.end(), "#691: deleted report root settings should not mix into live settings");
}

void test_build_report_layout_localizes_section_titles_without_localizing_band_kinds() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "localized.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "10"),
                value("VPOS", "0.000"),
                value("HEIGHT", "500.000")
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "99"),
                value("VPOS", "500.000"),
                value("HEIGHT", "500.000")
            }
        }
    };

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto layout = copperfin::studio::build_report_layout(document, pseudo_catalog);

    expect(layout.sections.size() == 2U, "#2489: pseudo-localized report layout should preserve section detection");
    if (layout.sections.size() == 2U) {
        expect(layout.sections[0].band_kind == "detail_footer",
            "#2489: pseudo-localized report layout should preserve invariant detail-footer band_kind");
        expect(layout.sections[1].band_kind == "other",
            "#2489: pseudo-localized report layout should preserve invariant fallback band_kind");
        expect(layout.sections[0].title.find("[!! ") != std::string::npos,
            "#2489: pseudo-localized detail-footer title should route through the catalog");
        expect(layout.sections[1].title.find("[!! ") != std::string::npos,
            "#2489: pseudo-localized other-band title should route through the catalog");
        expect(layout.sections[0].title.find("Detail Footer") == std::string::npos,
            "#2489: pseudo-localized detail-footer title should not fall back to raw English prose");
        expect(layout.sections[1].title.find("Other Band") == std::string::npos,
            "#2489: pseudo-localized other-band title should not fall back to raw English prose");
    }
}

void test_build_report_layout_suppresses_unresolved_memo_placeholders() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "memo-placeholder.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "<memo block 30>"),
                value("TOPMARGIN", "<memo block 31>")
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "8"),
                value("EXPR", "<memo block 32>", 32U),
                value("HPOS", "100.000"),
                value("VPOS", "100.000"),
                value("WIDTH", "500.000"),
                value("HEIGHT", "100.000"),
                value("FONTFACE", "<memo block 33>")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.settings.empty(), "#695: unresolved report root memo placeholders should not become settings");
    expect(layout.unplaced_objects.size() == 1U, "#695: unresolved report memo placeholders should not prevent object capture");
    if (!layout.unplaced_objects.empty()) {
        const auto& object = layout.unplaced_objects[0];
        expect(object.title == "Record 1", "#695: unresolved memo object titles should use the synthetic fallback");
        expect(object.title_field_index == copperfin::studio::StudioReportMissingFieldIndex,
            "#695: synthetic titles should not masquerade as unresolved memo field provenance");
        expect(object.expression.empty(), "#695: unresolved memo expressions should not become active object expressions");
        expect(object.expression_field_index == 1U,
            "#695: unresolved memo expression fields should retain source field provenance");
        expect(object.expression_memo_block_number == 32U,
            "#716: unresolved memo expression fields should retain source memo block provenance");
        expect(object.title_memo_block_number == 0U,
            "#716: synthesized titles from unresolved expressions should expose memo block zero");
        const auto expr_highlight = std::find_if(object.highlights.begin(), object.highlights.end(), [](const auto& highlight) {
            return highlight.name == "EXPR";
        });
        expect(expr_highlight == object.highlights.end(), "#695: unresolved memo expressions should not become highlights");
        const auto font_highlight = std::find_if(object.highlights.begin(), object.highlights.end(), [](const auto& highlight) {
            return highlight.name == "FONTFACE";
        });
        expect(font_highlight == object.highlights.end(), "#695: unresolved memo font fields should not become highlights");
    }
}

void test_build_report_layout_carries_group_section_expressions() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "grouped.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53")
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "3"),
                value("EXPR", "customer.country", 71U),
                value("VPOS", "0.000"),
                value("HEIGHT", "600.000")
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "4"),
                value("VPOS", "600.000"),
                value("HEIGHT", "3000.000")
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "5"),
                value("EXPR", "customer.country", 72U),
                value("VPOS", "3600.000"),
                value("HEIGHT", "500.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.sections.size() == 3U, "#1566: report layout should preserve grouped section rows");
    if (layout.sections.size() == 3U) {
        expect(layout.sections[0].band_kind == "group_header", "#1566: first group section should decode as a group header");
        expect(layout.sections[0].expression == "customer.country",
            "#1566: group header sections should expose the grouping expression");
        expect(layout.sections[0].expression_field_index == 2U,
            "#1566: group header expressions should retain EXPR field provenance");
        expect(layout.sections[0].expression_memo_block_number == 71U,
            "#1566: group header expressions should retain EXPR memo block provenance");
        expect(layout.sections[1].band_kind == "detail", "#1566: detail sections should remain ordered between group bands");
        expect(layout.sections[1].expression.empty(), "#1566: detail sections should not fabricate group expressions");
        expect(layout.sections[1].expression_field_index == copperfin::studio::StudioReportMissingFieldIndex,
            "#1566: expression-less sections should expose missing field provenance");
        expect(layout.sections[2].band_kind == "group_footer", "#1566: final group section should decode as a group footer");
        expect(layout.sections[2].expression == "customer.country",
            "#1566: group footer sections should expose the grouping expression");
        expect(layout.sections[2].expression_field_index == 2U,
            "#1566: group footer expressions should retain EXPR field provenance");
        expect(layout.sections[2].expression_memo_block_number == 72U,
            "#1566: group footer expressions should retain EXPR memo block provenance");
    }
}

void test_build_report_layout_reports_missing_title_provenance_when_unavailable() {
    copperfin::studio::StudioDocumentModel label_document;
    label_document.display_name = "mailing.lbx";
    label_document.kind = copperfin::studio::StudioAssetKind::label;
    label_document.table_preview_available = true;

    const auto label_layout = copperfin::studio::build_report_layout(label_document);
    expect(label_layout.available, "#728: label assets should still produce report layouts");
    expect(label_layout.is_label, "#728: label layouts should retain label classification");
    expect(label_layout.document_title == "mailing.lbx", "#728: label document titles should mirror Studio display names");
    expect(label_layout.document_title_field_index == copperfin::studio::StudioReportMissingFieldIndex,
        "#728: display-name label titles should use missing DBF field provenance");
    expect(label_layout.document_title_memo_block_number == 0U,
        "#728: display-name label titles should expose memo block zero");

    copperfin::studio::StudioDocumentModel document;
    document.display_name = "customer.scx";
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(!layout.available, "#728: non-report assets should not produce report layouts");
    expect(layout.document_title.empty(), "#728: unavailable report layouts should keep empty document titles");
    expect(layout.document_title_field_index == copperfin::studio::StudioReportMissingFieldIndex,
        "#728: unavailable report layouts should retain missing document-title field provenance");
    expect(layout.document_title_memo_block_number == 0U,
        "#728: unavailable report layouts should expose document-title memo block zero");
}

void test_build_report_layout_includes_direct_orientation_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "orientation.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "PAPERSIZE=1\r\nTOPMARGIN=10\r\nBOTMARGIN=20\r\nGRIDV=4\r\nGRIDH=8", 44U),
                value("ORIENTATION", "2")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#1544: report layout should be available for direct orientation settings");
    expect(layout.page_setup_available, "#1544: direct orientation should preserve page setup availability");
    expect(layout.orientation_available, "#1544: direct orientation should mark orientation metadata available");
    expect(layout.orientation_code == 2, "#1544: direct orientation should refresh orientation codes");
    expect(layout.paper_size_available && layout.paper_size_code == 1,
        "#1544: direct orientation should preserve memo-derived paper size");
    expect(layout.top_margin_available && layout.top_margin == 10,
        "#1544: direct orientation should preserve memo-derived top margins");
    expect(layout.bottom_margin_available && layout.bottom_margin == 20,
        "#1544: direct orientation should preserve memo-derived bottom margins");
    expect(layout.grid_vertical_available && layout.grid_vertical == 4,
        "#1544: direct orientation should preserve memo-derived vertical grid spacing");
    expect(layout.grid_horizontal_available && layout.grid_horizontal == 8,
        "#1544: direct orientation should preserve memo-derived horizontal grid spacing");
    expect(layout.settings.size() == 6U, "#1544: direct orientation should preserve root setting counts");

    const auto orientation = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "ORIENTATION";
    });
    expect(orientation != layout.settings.end(), "#1544: direct orientation should appear in root settings");
    if (orientation != layout.settings.end()) {
        expect(orientation->record_index == 0U, "#1544: direct orientation should retain source record provenance");
        expect(orientation->field_index == 3U, "#1544: direct orientation should retain DBF field provenance");
        expect(orientation->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
            "#1544: direct orientation should not masquerade as a memo-line setting");
        expect(orientation->memo_block_number == 0U,
            "#1544: direct orientation should expose memo block zero for non-memo fields");
        expect(orientation->value == "2", "#1544: direct orientation should preserve the field value text");
    }
}

void test_build_report_layout_includes_direct_paper_size_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "paper-size.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=0\r\nTOPMARGIN=10\r\nBOTMARGIN=20\r\nGRIDV=4\r\nGRIDH=8", 45U),
                value("PAPERSIZE", "9")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#1545: report layout should be available for direct paper-size settings");
    expect(layout.page_setup_available, "#1545: direct paper size should preserve page setup availability");
    expect(layout.paper_size_available, "#1545: direct paper size should mark paper-size metadata available");
    expect(layout.paper_size_code == 9, "#1545: direct paper size should refresh paper-size codes");
    expect(layout.orientation_available && layout.orientation_code == 0,
        "#1545: direct paper size should preserve memo-derived orientation");
    expect(layout.top_margin_available && layout.top_margin == 10,
        "#1545: direct paper size should preserve memo-derived top margins");
    expect(layout.bottom_margin_available && layout.bottom_margin == 20,
        "#1545: direct paper size should preserve memo-derived bottom margins");
    expect(layout.grid_vertical_available && layout.grid_vertical == 4,
        "#1545: direct paper size should preserve memo-derived vertical grid spacing");
    expect(layout.grid_horizontal_available && layout.grid_horizontal == 8,
        "#1545: direct paper size should preserve memo-derived horizontal grid spacing");
    expect(layout.settings.size() == 6U, "#1545: direct paper size should preserve root setting counts");

    const auto paper_size = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "PAPERSIZE";
    });
    expect(paper_size != layout.settings.end(), "#1545: direct paper size should appear in root settings");
    if (paper_size != layout.settings.end()) {
        expect(paper_size->record_index == 0U, "#1545: direct paper size should retain source record provenance");
        expect(paper_size->field_index == 3U, "#1545: direct paper size should retain DBF field provenance");
        expect(paper_size->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
            "#1545: direct paper size should not masquerade as a memo-line setting");
        expect(paper_size->memo_block_number == 0U,
            "#1545: direct paper size should expose memo block zero for non-memo fields");
        expect(paper_size->value == "9", "#1545: direct paper size should preserve the field value text");
    }
}

}  // namespace

int main() {
    test_build_report_layout_groups_band_objects();
    test_build_report_layout_localizes_section_titles_without_localizing_band_kinds();
    test_build_report_layout_suppresses_unresolved_memo_placeholders();
    test_build_report_layout_carries_group_section_expressions();
    test_build_report_layout_reports_missing_title_provenance_when_unavailable();
    test_build_report_layout_includes_direct_orientation_settings();
    test_build_report_layout_includes_direct_paper_size_settings();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

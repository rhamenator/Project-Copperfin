// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/studio/report_layout.h"

#include "copperfin/localization/localization.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
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
        expect(layout.deleted_objects[0].containing_section_id == "detail_2",
            "#2689: deleted report objects inside live sections should expose containing section ids");
        expect(layout.deleted_objects[0].containing_section_record_index == 2U,
            "#2689: deleted report objects inside live sections should expose containing section record indexes");
        expect(layout.deleted_objects[0].section_object_index == 0U,
            "#2689: deleted report objects inside live sections should expose section-local deleted-object indexes");
        expect(layout.deleted_objects[0].section_object_count == 1U,
            "#2689: deleted report objects inside live sections should expose section-local deleted-object counts");
        expect(layout.deleted_objects[0].section_relative_top == 600,
            "#2689: deleted report objects inside live sections should expose relative top coordinates");
        expect(layout.deleted_objects[0].section_relative_bottom == 900,
            "#2689: deleted report objects inside live sections should expose relative bottom-edge coordinates");
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

void test_report_layout_section_catalog_entries_cover_placeholder_locales() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> keys = {
        "Studio.ReportLayout.Section.Title",
        "Studio.ReportLayout.Section.PageHeader",
        "Studio.ReportLayout.Section.ColumnHeader",
        "Studio.ReportLayout.Section.GroupHeader",
        "Studio.ReportLayout.Section.Detail",
        "Studio.ReportLayout.Section.GroupFooter",
        "Studio.ReportLayout.Section.ColumnFooter",
        "Studio.ReportLayout.Section.PageFooter",
        "Studio.ReportLayout.Section.Summary",
        "Studio.ReportLayout.Section.DetailHeader",
        "Studio.ReportLayout.Section.DetailFooter",
        "Studio.ReportLayout.Section.OtherBand",
        "Studio.ReportLayout.Fallback.RecordTitle"};

    expect(
        english_catalog.translate("Studio.ReportLayout.Section.PageHeader") == "Page Header",
        "#2610: report layout page-header title should remain catalog-backed in en-US");
    expect(
        spanish_catalog.translate("Studio.ReportLayout.Section.PageHeader") == "Encabezado de pagina",
        "#2610: es-419 report layout page-header title should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ReportLayout.Section.DetailFooter") == "Pie de detalle",
        "#2610: es-419 report layout detail-footer title should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ReportLayout.Section.ColumnFooter") == "Rodape da coluna",
        "#2610: pt-BR report layout column-footer title should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ReportLayout.Section.OtherBand") == "Outra banda",
        "#2610: pt-BR report layout fallback band title should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ReportLayout.Fallback.RecordTitle") == "Registro {recordIndex}",
        "#2652: es-419 report-layout fallback record title should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ReportLayout.Fallback.RecordTitle") == "Registro {recordIndex}",
        "#2652: pt-BR report-layout fallback record title should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.ReportLayout.Section.Summary") ==
            copperfin::localization::pseudo_localize("Summary"),
        "#2610: qps-ploc report layout summary title should resolve through the pseudo-localization transform");

    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", keys) == 0U,
        "#2610: es-419 should define every remaining Studio.ReportLayout.Section localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", keys) == 0U,
        "#2610: pt-BR should define every remaining Studio.ReportLayout.Section localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", keys) == 0U,
        "#2610: qps-ploc should define every remaining Studio.ReportLayout.Section localization key");
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
        const auto spanish_catalog =
            copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "es-419");
        const auto portuguese_catalog =
            copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "pt-BR");
        const auto pseudo_catalog =
            copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "qps-ploc");
        const auto spanish_layout = copperfin::studio::build_report_layout(document, spanish_catalog);
        const auto portuguese_layout = copperfin::studio::build_report_layout(document, portuguese_catalog);
        expect(
            spanish_layout.unplaced_objects.size() == 1U &&
                spanish_layout.unplaced_objects[0].title == "Registro 1",
            "#2652: es-419 fallback report object title should flow through the report-layout model");
        expect(
            portuguese_layout.unplaced_objects.size() == 1U &&
                portuguese_layout.unplaced_objects[0].title == "Registro 1",
            "#2652: pt-BR fallback report object title should flow through the report-layout model");
        const auto pseudo_layout = copperfin::studio::build_report_layout(document, pseudo_catalog);
        expect(
            pseudo_layout.unplaced_objects.size() == 1U,
            "#2498: pseudo-localized fallback title should preserve unplaced object capture");
        if (!pseudo_layout.unplaced_objects.empty()) {
            const auto& pseudo_object = pseudo_layout.unplaced_objects[0];
            expect(
                pseudo_object.record_index == 1U,
                "#2498: pseudo-localized fallback title should preserve object record index");
            expect(
                pseudo_object.title.find("[!! ") != std::string::npos,
                "#2498: fallback object title should route through pseudo-localization");
            expect(
                pseudo_object.title.find("Record 1") == std::string::npos,
                "#2498: pseudo-localized fallback object title should not fall back to raw English prose");
            expect(
                pseudo_object.title.find("1") != std::string::npos,
                "#2498: fallback object title should preserve the named recordIndex placeholder value");
            expect(
                pseudo_object.object_kind == object.object_kind,
                "#2498: pseudo-localized fallback title should preserve object kind");
            expect(
                pseudo_object.title_field_index == object.title_field_index,
                "#2498: pseudo-localized fallback title should preserve title field provenance");
            expect(
                pseudo_object.title_memo_block_number == object.title_memo_block_number,
                "#2498: pseudo-localized fallback title should preserve title memo provenance");
            expect(
                pseudo_object.left == object.left && pseudo_object.top == object.top &&
                    pseudo_object.width == object.width && pseudo_object.height == object.height,
                "#2498: pseudo-localized fallback title should preserve object geometry");
        }
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

void test_build_report_layout_carries_nested_group_section_ordering() {
    const auto run_nested_group_layout = [](copperfin::studio::StudioAssetKind kind,
                                            const std::string& display_name,
                                            bool expect_label) {
        copperfin::studio::StudioDocumentModel document;
        document.display_name = display_name;
        document.kind = kind;
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
                    value("EXPR", "customer.region", 81U),
                    value("VPOS", "0.000"),
                    value("HEIGHT", "400.000")
                }
            },
            {
                .record_index = 2U,
                .deleted = false,
                .values = {
                    value("OBJTYPE", "9"),
                    value("OBJCODE", "3"),
                    value("EXPR", "customer.country", 82U),
                    value("VPOS", "400.000"),
                    value("HEIGHT", "300.000")
                }
            },
            {
                .record_index = 3U,
                .deleted = false,
                .values = {
                    value("OBJTYPE", "9"),
                    value("OBJCODE", "4"),
                    value("VPOS", "700.000"),
                    value("HEIGHT", "2200.000")
                }
            },
            {
                .record_index = 4U,
                .deleted = false,
                .values = {
                    value("OBJTYPE", "9"),
                    value("OBJCODE", "5"),
                    value("EXPR", "customer.country", 83U),
                    value("VPOS", "2900.000"),
                    value("HEIGHT", "250.000")
                }
            },
            {
                .record_index = 5U,
                .deleted = false,
                .values = {
                    value("OBJTYPE", "9"),
                    value("OBJCODE", "5"),
                    value("EXPR", "customer.region", 84U),
                    value("VPOS", "3150.000"),
                    value("HEIGHT", "350.000")
                }
            }
        };

        const auto layout = copperfin::studio::build_report_layout(document);
        expect(layout.available, "#2680: nested grouped report/label layouts should remain available");
        expect(layout.is_label == expect_label,
               "#2680: nested grouped report/label layouts should preserve label identity");
        expect(layout.sections.size() == 5U,
               "#2680: nested grouped report/label layouts should preserve both group levels around detail");
        expect(layout.preview_bounds_available && layout.preview_bounds_top == 0 && layout.preview_bounds_bottom == 3500,
               "#2680: nested grouped report/label layouts should preserve full section preview bounds");
        if (layout.sections.size() == 5U) {
            expect(layout.sections[0].band_kind == "group_header" &&
                       layout.sections[0].expression == "customer.region" &&
                       layout.sections[0].section_index == 0U &&
                       layout.sections[0].section_count == 5U,
                   "#2680: outer group headers should remain first with their grouping expression");
            expect(layout.sections[1].band_kind == "group_header" &&
                       layout.sections[1].expression == "customer.country" &&
                       layout.sections[1].section_index == 1U &&
                       layout.sections[1].section_count == 5U,
                   "#2680: inner group headers should remain ordered inside the outer group");
            expect(layout.sections[2].band_kind == "detail" &&
                       layout.sections[2].expression.empty() &&
                       layout.sections[2].section_index == 2U,
                   "#2680: detail sections should stay between nested group headers and footers");
            expect(layout.sections[3].band_kind == "group_footer" &&
                       layout.sections[3].expression == "customer.country" &&
                       layout.sections[3].section_index == 3U,
                   "#2680: inner group footers should remain ahead of the outer footer");
            expect(layout.sections[4].band_kind == "group_footer" &&
                       layout.sections[4].expression == "customer.region" &&
                       layout.sections[4].section_index == 4U,
                   "#2680: outer group footers should remain last with their grouping expression");
            expect(layout.sections[0].expression_memo_block_number == 81U &&
                       layout.sections[1].expression_memo_block_number == 82U &&
                       layout.sections[3].expression_memo_block_number == 83U &&
                       layout.sections[4].expression_memo_block_number == 84U,
                   "#2680: nested group sections should retain distinct EXPR memo provenance");
        }
    };

    run_nested_group_layout(copperfin::studio::StudioAssetKind::report, "nested_grouped.frx", false);
    run_nested_group_layout(copperfin::studio::StudioAssetKind::label, "nested_grouped.lbx", true);
}

void test_build_report_layout_summarizes_groupings() {
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
                value("EXPR", "customer.country", 91U),
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
                value("EXPR", "customer.country", 92U),
                value("VPOS", "3600.000"),
                value("HEIGHT", "500.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.groupings.size() == 1U, "#2685: grouped report layouts should expose one grouping summary");
    expect(layout.sections.size() == 3U, "#2686: grouped report layouts should preserve three live sections");
    if (layout.groupings.size() == 1U) {
        const auto& grouping = layout.groupings[0];
        expect(grouping.grouping_index == 0U, "#2685: grouped report layouts should number groupings from zero");
        expect(grouping.nesting_depth == 0U, "#2685: top-level groupings should expose zero nesting depth");
        expect(grouping.expression == "customer.country",
               "#2685: grouping summaries should expose the grouping expression");
        expect(grouping.expression_field_index == 2U,
               "#2685: grouping summaries should retain grouping-expression field provenance");
        expect(grouping.expression_memo_block_number == 91U,
               "#2685: grouping summaries should prefer header expression memo provenance");
        expect(grouping.header_section_id == "group_header_1" && grouping.header_record_index == 1U &&
                   !grouping.header_deleted,
               "#2685: grouping summaries should expose live group-header identity");
        expect(grouping.footer_section_id == "group_footer_3" && grouping.footer_record_index == 3U &&
                   !grouping.footer_deleted,
               "#2685: grouping summaries should expose live group-footer identity");
    }
    if (layout.sections.size() == 3U) {
        const auto& header = layout.sections[0];
        expect(header.grouping_context_available,
               "#2686: live group headers should expose grouping context");
        expect(header.grouping_index == 0U && header.grouping_nesting_depth == 0U,
               "#2686: top-level group headers should expose grouping index and depth");
        expect(header.grouping_role == "header",
               "#2686: live group headers should expose header role");
        expect(header.grouping_partner_section_id == "group_footer_3" &&
                   header.grouping_partner_record_index == 3U &&
                   !header.grouping_partner_deleted,
               "#2686: live group headers should expose live footer partner identity");

        const auto& detail = layout.sections[1];
        expect(!detail.grouping_context_available,
               "#2686: non-group detail sections should keep grouping context unavailable");
        expect(detail.grouping_index == copperfin::studio::StudioReportMissingRecordIndex &&
                   detail.grouping_partner_record_index == copperfin::studio::StudioReportMissingRecordIndex,
               "#2686: non-group detail sections should keep grouping identifiers absent");

        const auto& footer = layout.sections[2];
        expect(footer.grouping_context_available,
               "#2686: live group footers should expose grouping context");
        expect(footer.grouping_index == 0U && footer.grouping_nesting_depth == 0U,
               "#2686: top-level group footers should expose grouping index and depth");
        expect(footer.grouping_role == "footer",
               "#2686: live group footers should expose footer role");
        expect(footer.grouping_partner_section_id == "group_header_1" &&
                   footer.grouping_partner_record_index == 1U &&
                   !footer.grouping_partner_deleted,
               "#2686: live group footers should expose live header partner identity");
    }
}

void test_build_report_layout_resolves_grouping_expression_for_blank_footer() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "grouped_blank_footer.frx";
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
                value("EXPR", "customer.country", 95U),
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
                value("VPOS", "3600.000"),
                value("HEIGHT", "500.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.sections.size() == 3U,
           "#2687: grouped report layouts with blank footer EXPR should preserve all live sections");
    if (layout.sections.size() == 3U) {
        const auto& header = layout.sections[0];
        expect(header.grouping_expression == "customer.country" &&
                   header.grouping_expression_field_index == 2U &&
                   header.grouping_expression_memo_block_number == 95U,
               "#2687: group headers should expose resolved grouping-expression provenance");

        const auto& footer = layout.sections[2];
        expect(footer.expression.empty(),
               "#2687: blank group footers should preserve empty direct EXPR payloads");
        expect(footer.expression_field_index == copperfin::studio::StudioReportMissingFieldIndex &&
                   footer.expression_memo_block_number == 0U,
               "#2687: blank group footers should preserve missing direct EXPR provenance");
        expect(footer.grouping_context_available,
               "#2687: blank group footers should still expose grouping context");
        expect(footer.grouping_expression == "customer.country",
               "#2687: blank group footers should expose the resolved grouping expression from the paired header");
        expect(footer.grouping_expression_field_index == 2U &&
                   footer.grouping_expression_memo_block_number == 95U,
               "#2687: blank group footers should expose resolved grouping-expression provenance from the paired header");
    }
}

void test_build_report_layout_resolves_grouping_expression_for_deleted_blank_footer() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "grouped_deleted_blank_footer.frx";
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
                value("EXPR", "customer.country", 96U),
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
            .deleted = true,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "5"),
                value("VPOS", "3600.000"),
                value("HEIGHT", "500.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.deleted_sections.size() == 1U,
           "#2687: grouped report layouts with deleted blank footer EXPR should preserve one deleted section");
    if (layout.deleted_sections.size() == 1U) {
        const auto& deleted_footer = layout.deleted_sections[0];
        expect(deleted_footer.expression.empty(),
               "#2687: deleted blank group footers should preserve empty direct EXPR payloads");
        expect(deleted_footer.expression_field_index == copperfin::studio::StudioReportMissingFieldIndex &&
                   deleted_footer.expression_memo_block_number == 0U,
               "#2687: deleted blank group footers should preserve missing direct EXPR provenance");
        expect(deleted_footer.grouping_context_available,
               "#2687: deleted blank group footers should still expose grouping context");
        expect(deleted_footer.grouping_expression == "customer.country",
               "#2687: deleted blank group footers should expose the resolved grouping expression from the paired header");
        expect(deleted_footer.grouping_expression_field_index == 2U &&
                   deleted_footer.grouping_expression_memo_block_number == 96U,
               "#2687: deleted blank group footers should expose resolved grouping-expression provenance from the paired header");
    }
}

void test_build_report_layout_preserves_group_pairing_when_group_header_moves_below_footer() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "group_header_moved.frx";
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
                value("EXPR", "customer.country", 111U),
                value("VPOS", "500.000"),
                value("HEIGHT", "600.000"),
                value("UNIQUEID", "country-header-guid")
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "4"),
                value("VPOS", "1200.000"),
                value("HEIGHT", "2000.000")
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "5"),
                value("EXPR", "customer.country", 112U),
                value("VPOS", "0.000"),
                value("HEIGHT", "500.000"),
                value("UNIQUEID", "country-footer-guid")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.groupings.size() == 1U,
           "#3058: moved group headers should keep one grouping summary");
    expect(layout.sections.size() == 3U,
           "#3058: moved group headers should preserve live section counts");
    if (layout.groupings.size() == 1U) {
        const auto& grouping = layout.groupings[0];
        expect(grouping.grouping_index == 0U,
               "#3058: moved group headers should preserve zero-based grouping indexes");
        expect(grouping.expression == "customer.country",
               "#3058: moved group headers should preserve grouping expressions");
        expect(grouping.header_section_id == "country-header-guid" &&
                   grouping.header_record_index == 1U &&
                   !grouping.header_deleted,
               "#3058: moved group headers should keep the live header in the original grouping pair");
        expect(grouping.footer_section_id == "country-footer-guid" &&
                   grouping.footer_record_index == 3U &&
                   !grouping.footer_deleted,
               "#3058: moved group headers should keep the live footer in the original grouping pair");
    }

    if (layout.sections.size() == 3U) {
        const auto header = std::find_if(layout.sections.begin(), layout.sections.end(), [](const auto& section) {
            return section.record_index == 1U;
        });
        expect(header != layout.sections.end() &&
                   header->grouping_context_available &&
                   header->grouping_index == 0U &&
                   header->grouping_role == "header" &&
                   header->grouping_partner_section_id == "country-footer-guid" &&
                   header->grouping_partner_record_index == 3U &&
                   !header->grouping_partner_deleted,
               "#3058: moved group headers should preserve header grouping context and footer partner identity");

        const auto footer = std::find_if(layout.sections.begin(), layout.sections.end(), [](const auto& section) {
            return section.record_index == 3U;
        });
        expect(footer != layout.sections.end() &&
                   footer->grouping_context_available &&
                   footer->grouping_index == 0U &&
                   footer->grouping_role == "footer" &&
                   footer->grouping_partner_section_id == "country-header-guid" &&
                   footer->grouping_partner_record_index == 1U &&
                   !footer->grouping_partner_deleted,
               "#3058: moved group headers should preserve footer grouping context and header partner identity");
    }
}

void test_build_report_layout_counts_deleted_objects_per_section() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "section-deleted-objects.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=0")
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "4"),
                value("VPOS", "0"),
                value("HEIGHT", "1000")
            }
        },
        {
            .record_index = 2U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "8"),
                value("VPOS", "2000"),
                value("HEIGHT", "500")
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "8"),
                value("OBJCODE", "0"),
                value("EXPR", "detail.value"),
                value("HPOS", "100"),
                value("VPOS", "200"),
                value("WIDTH", "400"),
                value("HEIGHT", "100")
            }
        },
        {
            .record_index = 4U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "5"),
                value("EXPR", "\"Deleted detail\""),
                value("HPOS", "150"),
                value("VPOS", "300"),
                value("WIDTH", "200"),
                value("HEIGHT", "100")
            }
        },
        {
            .record_index = 5U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "5"),
                value("EXPR", "\"Deleted summary\""),
                value("HPOS", "200"),
                value("VPOS", "2100"),
                value("WIDTH", "250"),
                value("HEIGHT", "100")
            }
        },
        {
            .record_index = 6U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "6"),
                value("HPOS", "50"),
                value("VPOS", "5000"),
                value("WIDTH", "100"),
                value("HEIGHT", "100")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.sections.size() == 1U, "#2688: live section-deleted-object layout should keep one live section");
    expect(layout.deleted_sections.size() == 1U,
           "#2688: live section-deleted-object layout should keep one deleted section");
    expect(layout.deleted_placed_object_count == 2U,
           "#2688: section-deleted-object layout should count placed deleted objects across live and deleted sections");
    expect(layout.deleted_unplaced_object_count == 1U,
           "#2688: section-deleted-object layout should keep deleted unplaced objects separate");
    if (layout.sections.size() == 1U) {
        expect(layout.sections[0].objects.size() == 1U,
               "#2688: live section-deleted-object layout should preserve live section membership");
        expect(layout.sections[0].deleted_object_count == 1U,
               "#2688: live sections should count deleted placed objects inside their geometry");
    }
    if (layout.deleted_sections.size() == 1U) {
        expect(layout.deleted_sections[0].objects.empty(),
               "#2688: deleted sections should keep live object membership empty");
        expect(layout.deleted_sections[0].deleted_object_count == 1U,
               "#2688: deleted sections should count deleted placed objects inside their geometry");
    }
    expect(layout.deleted_objects.size() == 3U,
           "#2689: section-deleted-object layout should preserve deleted live-section, deleted-section, and unplaced objects");
    if (layout.deleted_objects.size() == 3U) {
        const auto& deleted_live_section_object = layout.deleted_objects[0];
        expect(deleted_live_section_object.containing_section_id == "detail_1" &&
                   deleted_live_section_object.containing_section_record_index == 1U,
               "#2689: deleted objects inside live sections should expose containing live-section identity");
        expect(deleted_live_section_object.section_relative_top == 300 &&
                   deleted_live_section_object.section_relative_bottom == 400,
               "#2689: deleted objects inside live sections should expose relative geometry");
        expect(deleted_live_section_object.section_object_index == 0U &&
                   deleted_live_section_object.section_object_count == 1U,
               "#2689: deleted objects inside live sections should expose section-local deleted-object order");

        const auto& deleted_deleted_section_object = layout.deleted_objects[1];
        expect(deleted_deleted_section_object.containing_section_id == "summary_2" &&
                   deleted_deleted_section_object.containing_section_record_index == 2U,
               "#2689: deleted objects inside deleted sections should expose containing deleted-section identity");
        expect(deleted_deleted_section_object.section_relative_top == 100 &&
                   deleted_deleted_section_object.section_relative_bottom == 200,
               "#2689: deleted objects inside deleted sections should expose deleted section-relative geometry");
        expect(deleted_deleted_section_object.section_object_index == 0U &&
                   deleted_deleted_section_object.section_object_count == 1U,
               "#2689: deleted objects inside deleted sections should expose section-local deleted-object order");

        const auto& deleted_unplaced_object = layout.deleted_objects[2];
        expect(deleted_unplaced_object.containing_section_id.empty() &&
                   deleted_unplaced_object.containing_section_record_index ==
                       copperfin::studio::StudioReportMissingRecordIndex,
               "#2689: deleted unplaced objects should keep containing-section identity absent");
        expect(deleted_unplaced_object.section_object_index ==
                   copperfin::studio::StudioReportMissingRecordIndex &&
                   deleted_unplaced_object.section_object_count == 0U,
               "#2689: deleted unplaced objects should keep section-local deleted-object order absent");
    }
}

void test_build_report_layout_preserves_live_objects_in_deleted_sections() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "live-objects-in-deleted-sections.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=0")
            }
        },
        {
            .record_index = 1U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "1"),
                value("VPOS", "0"),
                value("HEIGHT", "2000")
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "4"),
                value("VPOS", "2000"),
                value("HEIGHT", "5000")
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "5"),
                value("EXPR", "\"Deleted-section label\""),
                value("HPOS", "900"),
                value("VPOS", "100"),
                value("WIDTH", "1800"),
                value("HEIGHT", "350")
            }
        },
        {
            .record_index = 4U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "8"),
                value("OBJCODE", "0"),
                value("EXPR", "detail.value"),
                value("HPOS", "1200"),
                value("VPOS", "2600"),
                value("WIDTH", "4000"),
                value("HEIGHT", "450")
            }
        },
        {
            .record_index = 5U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "6"),
                value("HPOS", "50"),
                value("VPOS", "8000"),
                value("WIDTH", "100"),
                value("HEIGHT", "100")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.sections.size() == 1U && layout.deleted_sections.size() == 1U,
           "#2690: live-object deleted-section layouts should preserve both live and deleted section partitions");
    expect(layout.live_object_count == 3U,
           "#2690: live-object deleted-section layouts should still count all live objects");
    expect(layout.placed_object_count == 2U,
           "#2690: live objects inside deleted sections should remain placed");
    expect(layout.unplaced_objects.size() == 1U,
           "#2690: only true strays should remain unplaced when a section is deleted");
    expect(layout.preview_bounds_available &&
               layout.preview_bounds_left == 0 &&
               layout.preview_bounds_top == 100 &&
               layout.preview_bounds_right == 5200 &&
               layout.preview_bounds_bottom == 8100,
           "#3684: live objects inside deleted sections should still expand live preview bounds");
    expect(layout.deleted_preview_bounds_available &&
               layout.deleted_preview_bounds_left == 0 &&
               layout.deleted_preview_bounds_top == 0 &&
               layout.deleted_preview_bounds_right == 0 &&
               layout.deleted_preview_bounds_bottom == 2000,
           "#3684: live objects inside deleted sections should not contaminate deleted preview bounds");
    if (layout.deleted_sections.size() == 1U) {
        expect(layout.deleted_sections[0].objects.size() == 1U,
               "#2690: deleted sections should preserve live object membership when geometry still belongs to the band");
        if (!layout.deleted_sections[0].objects.empty()) {
            const auto& object = layout.deleted_sections[0].objects[0];
            expect(object.containing_section_id == "page_header_1" &&
                       object.containing_section_record_index == 1U,
                   "#2690: live objects inside deleted sections should expose containing deleted-section identity");
            expect(object.section_relative_top == 100 &&
                       object.section_relative_bottom == 450,
                   "#2690: live objects inside deleted sections should expose deleted section-relative geometry");
            expect(object.section_object_index == 0U &&
                       object.section_object_count == 1U,
                   "#2690: live objects inside deleted sections should expose deleted section-local object order");
        }
    }
}

void test_build_report_layout_keeps_tall_objects_in_the_section_where_they_begin() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "tall_object.frx";
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
                value("OBJCODE", "1"),
                value("VPOS", "0"),
                value("HEIGHT", "100")
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "4"),
                value("VPOS", "100"),
                value("HEIGHT", "100")
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "5"),
                value("EXPR", "\"Tall object\""),
                value("HPOS", "25"),
                value("VPOS", "50"),
                value("WIDTH", "75"),
                value("HEIGHT", "200")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#3671: report layout should stay available for tall-object section tests");
    expect(layout.sections.size() == 2U, "#3671: tall-object section tests should keep both live sections");
    expect(layout.sections[0].objects.size() == 1U,
           "#3671: a tall object should remain attached to the section containing its top edge");
    if (!layout.sections[0].objects.empty()) {
        expect(layout.sections[0].objects[0].containing_section_id == "page_header_1",
               "#3671: tall objects should keep the containing section where their top edge begins");
        expect(layout.sections[0].objects[0].section_relative_top == 50,
               "#3671: tall objects should keep a non-negative top relative to their starting section");
        expect(layout.sections[0].objects[0].section_relative_bottom == 250,
               "#3671: tall objects should measure their bottom relative to the section where they start");
    }
    expect(layout.sections[1].objects.empty(),
           "#3671: later overlapping sections should not steal tall objects whose top edge begins earlier");
}

void test_build_report_layout_summarizes_nested_mixed_state_groupings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "nested_deleted_grouped.frx";
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
                value("EXPR", "customer.region", 101U),
                value("VPOS", "0.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "3"),
                value("EXPR", "customer.country", 102U),
                value("VPOS", "400.000"),
                value("HEIGHT", "300.000")
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "4"),
                value("VPOS", "700.000"),
                value("HEIGHT", "2200.000")
            }
        },
        {
            .record_index = 4U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "5"),
                value("EXPR", "customer.country", 103U),
                value("VPOS", "2900.000"),
                value("HEIGHT", "250.000")
            }
        },
        {
            .record_index = 5U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "5"),
                value("EXPR", "customer.region", 104U),
                value("VPOS", "3150.000"),
                value("HEIGHT", "350.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.groupings.size() == 2U,
           "#2685: nested mixed-state grouped layouts should expose both grouping levels");
    expect(layout.sections.size() == 4U && layout.deleted_sections.size() == 1U,
           "#2686: nested mixed-state grouped layouts should preserve live and deleted section partitions");
    if (layout.groupings.size() == 2U) {
        const auto& outer_group = layout.groupings[0];
        expect(outer_group.grouping_index == 0U && outer_group.nesting_depth == 0U,
               "#2685: outer grouping summaries should stay first at depth zero");
        expect(outer_group.expression == "customer.region" &&
                   outer_group.header_section_id == "group_header_1" &&
                   outer_group.footer_section_id == "group_footer_5" &&
                   !outer_group.header_deleted &&
                   !outer_group.footer_deleted,
               "#2685: outer grouping summaries should pair live outer group bands");

        const auto& inner_group = layout.groupings[1];
        expect(inner_group.grouping_index == 1U && inner_group.nesting_depth == 1U,
               "#2685: inner grouping summaries should expose nested depth");
        expect(inner_group.expression == "customer.country",
               "#2685: inner grouping summaries should preserve the inner grouping expression");
        expect(inner_group.expression_field_index == 2U &&
                   inner_group.expression_memo_block_number == 102U,
               "#2685: inner grouping summaries should prefer live header expression provenance");
        expect(inner_group.header_section_id == "group_header_2" &&
                   inner_group.header_record_index == 2U &&
                   !inner_group.header_deleted,
               "#2685: inner grouping summaries should expose the live inner group-header identity");
        expect(inner_group.footer_section_id == "group_footer_4" &&
                   inner_group.footer_record_index == 4U &&
                   inner_group.footer_deleted,
               "#2685: inner grouping summaries should expose deleted inner group-footers without losing the grouping pair");
    }
    if (layout.sections.size() == 4U && layout.deleted_sections.size() == 1U) {
        const auto& outer_header = layout.sections[0];
        expect(outer_header.grouping_context_available &&
                   outer_header.grouping_index == 0U &&
                   outer_header.grouping_nesting_depth == 0U &&
                   outer_header.grouping_role == "header",
               "#2686: live outer group headers should expose top-level grouping context");
        expect(outer_header.grouping_partner_section_id == "group_footer_5" &&
                   outer_header.grouping_partner_record_index == 5U &&
                   !outer_header.grouping_partner_deleted,
               "#2686: live outer group headers should point at the live outer footer");

        const auto& inner_header = layout.sections[1];
        expect(inner_header.grouping_context_available &&
                   inner_header.grouping_index == 1U &&
                   inner_header.grouping_nesting_depth == 1U &&
                   inner_header.grouping_role == "header",
               "#2686: live inner group headers should expose nested grouping context");
        expect(inner_header.grouping_partner_section_id == "group_footer_4" &&
                   inner_header.grouping_partner_record_index == 4U &&
                   inner_header.grouping_partner_deleted,
               "#2686: live inner group headers should point at the deleted nested footer");

        const auto& detail = layout.sections[2];
        expect(!detail.grouping_context_available,
               "#2686: nested detail sections should keep grouping context unavailable");

        const auto& outer_footer = layout.sections[3];
        expect(outer_footer.grouping_context_available &&
                   outer_footer.grouping_index == 0U &&
                   outer_footer.grouping_nesting_depth == 0U &&
                   outer_footer.grouping_role == "footer",
               "#2686: live outer group footers should expose top-level grouping context");
        expect(outer_footer.grouping_partner_section_id == "group_header_1" &&
                   outer_footer.grouping_partner_record_index == 1U &&
                   !outer_footer.grouping_partner_deleted,
               "#2686: live outer group footers should point back to the live outer header");

        const auto& deleted_inner_footer = layout.deleted_sections[0];
        expect(deleted_inner_footer.grouping_context_available &&
                   deleted_inner_footer.grouping_index == 1U &&
                   deleted_inner_footer.grouping_nesting_depth == 1U &&
                   deleted_inner_footer.grouping_role == "footer",
               "#2686: deleted inner group footers should expose nested grouping context");
        expect(deleted_inner_footer.grouping_partner_section_id == "group_header_2" &&
                   deleted_inner_footer.grouping_partner_record_index == 2U &&
                   !deleted_inner_footer.grouping_partner_deleted,
               "#2686: deleted inner group footers should point back to the live inner header");
        expect(deleted_inner_footer.grouping_expression == "customer.country" &&
                   deleted_inner_footer.grouping_expression_field_index == 2U &&
                   deleted_inner_footer.grouping_expression_memo_block_number == 102U,
               "#2687: deleted inner group footers should expose resolved grouping-expression provenance from the paired header");
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

void test_build_report_layout_includes_direct_side_margin_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "side-margins.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=0\r\nPAPERSIZE=1\r\nTOPMARGIN=10\r\nBOTMARGIN=20\r\nGRIDV=4\r\nGRIDH=8", 46U),
                value("LEFTMARGIN", "15"),
                value("RIGHTMARGIN", "25")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#3742: report layout should be available for direct side-margin settings");
    expect(layout.page_setup_available, "#3742: direct side margins should preserve page setup availability");
    expect(layout.left_margin_available, "#3742: direct left margin should mark left-margin metadata available");
    expect(layout.left_margin == 15, "#3742: direct left margin should refresh left-margin values");
    expect(layout.right_margin_available, "#3742: direct right margin should mark right-margin metadata available");
    expect(layout.right_margin == 25, "#3742: direct right margin should refresh right-margin values");
    expect(layout.orientation_available && layout.orientation_code == 0,
        "#3742: direct side margins should preserve memo-derived orientation");
    expect(layout.paper_size_available && layout.paper_size_code == 1,
        "#3742: direct side margins should preserve memo-derived paper size");
    expect(layout.top_margin_available && layout.top_margin == 10,
        "#3742: direct side margins should preserve memo-derived top margins");
    expect(layout.bottom_margin_available && layout.bottom_margin == 20,
        "#3742: direct side margins should preserve memo-derived bottom margins");
    expect(layout.grid_vertical_available && layout.grid_vertical == 4,
        "#3742: direct side margins should preserve memo-derived vertical grid spacing");
    expect(layout.grid_horizontal_available && layout.grid_horizontal == 8,
        "#3742: direct side margins should preserve memo-derived horizontal grid spacing");
    expect(layout.settings.size() == 8U, "#3742: direct side margins should preserve root setting counts");

    const auto left_margin = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "LEFTMARGIN";
    });
    expect(left_margin != layout.settings.end(), "#3742: direct left margin should appear in root settings");
    if (left_margin != layout.settings.end()) {
        expect(left_margin->record_index == 0U, "#3742: direct left margin should retain source record provenance");
        expect(left_margin->field_index == 3U, "#3742: direct left margin should retain DBF field provenance");
        expect(left_margin->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
            "#3742: direct left margin should not masquerade as a memo-line setting");
        expect(left_margin->memo_block_number == 0U,
            "#3742: direct left margin should expose memo block zero for non-memo fields");
        expect(left_margin->value == "15", "#3742: direct left margin should preserve the field value text");
    }

    const auto right_margin = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "RIGHTMARGIN";
    });
    expect(right_margin != layout.settings.end(), "#3742: direct right margin should appear in root settings");
    if (right_margin != layout.settings.end()) {
        expect(right_margin->record_index == 0U, "#3742: direct right margin should retain source record provenance");
        expect(right_margin->field_index == 4U, "#3742: direct right margin should retain DBF field provenance");
        expect(right_margin->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
            "#3742: direct right margin should not masquerade as a memo-line setting");
        expect(right_margin->memo_block_number == 0U,
            "#3742: direct right margin should expose memo block zero for non-memo fields");
        expect(right_margin->value == "25", "#3742: direct right margin should preserve the field value text");
    }
}

void test_build_report_layout_summarizes_paper_dimensions() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "paper-dimensions.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR",
                      "ORIENTATION=0\r\nPAPERSIZE=1\r\nPAPERLENGTH=2794\r\nPAPERWIDTH=2159\r\nTOPMARGIN=10\r\nBOTMARGIN=20\r\nGRIDV=4\r\nGRIDH=8",
                      47U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#3744: report layout should be available for paper-dimension settings");
    expect(layout.page_setup_available, "#3744: paper-dimension settings should preserve page setup availability");
    expect(layout.orientation_available && layout.orientation_code == 0,
        "#3744: paper-dimension settings should preserve memo-derived orientation");
    expect(layout.paper_size_available && layout.paper_size_code == 1,
        "#3744: paper-dimension settings should preserve memo-derived paper size");
    expect(layout.paper_length_available && layout.paper_length == 2794,
        "#3744: paper-dimension settings should expose memo-derived paper length");
    expect(layout.paper_width_available && layout.paper_width == 2159,
        "#3744: paper-dimension settings should expose memo-derived paper width");
    expect(layout.top_margin_available && layout.top_margin == 10,
        "#3744: paper-dimension settings should preserve memo-derived top margins");
    expect(layout.bottom_margin_available && layout.bottom_margin == 20,
        "#3744: paper-dimension settings should preserve memo-derived bottom margins");
    expect(layout.grid_vertical_available && layout.grid_vertical == 4,
        "#3744: paper-dimension settings should preserve memo-derived vertical grid spacing");
    expect(layout.grid_horizontal_available && layout.grid_horizontal == 8,
        "#3744: paper-dimension settings should preserve memo-derived horizontal grid spacing");
    expect(layout.settings.size() == 8U, "#3744: paper-dimension settings should preserve root setting counts");

    const auto paper_length = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "PAPERLENGTH";
    });
    expect(paper_length != layout.settings.end(), "#3744: paper length should appear in root settings");
    if (paper_length != layout.settings.end()) {
        expect(paper_length->record_index == 0U, "#3744: paper length should retain source record provenance");
        expect(paper_length->field_index == 2U, "#3744: paper length should retain EXPR field provenance");
        expect(paper_length->source_line_index == 2U, "#3744: paper length should retain memo line provenance");
        expect(paper_length->memo_block_number == 47U, "#3744: paper length should retain EXPR memo provenance");
        expect(paper_length->value == "2794", "#3744: paper length should preserve the field value text");
    }

    const auto paper_width = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "PAPERWIDTH";
    });
    expect(paper_width != layout.settings.end(), "#3744: paper width should appear in root settings");
    if (paper_width != layout.settings.end()) {
        expect(paper_width->record_index == 0U, "#3744: paper width should retain source record provenance");
        expect(paper_width->field_index == 2U, "#3744: paper width should retain EXPR field provenance");
        expect(paper_width->source_line_index == 3U, "#3744: paper width should retain memo line provenance");
        expect(paper_width->memo_block_number == 47U, "#3744: paper width should retain EXPR memo provenance");
        expect(paper_width->value == "2159", "#3744: paper width should preserve the field value text");
    }
}

void test_build_report_layout_preserves_band_unique_ids_for_section_identity() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "detail-ids.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "9"),
                value("VPOS", "0.000"),
                value("HEIGHT", "300.000"),
                value("UNIQUEID", "detail-header-guid")
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "10"),
                value("VPOS", "300.000"),
                value("HEIGHT", "250.000"),
                value("UNIQUEID", "detail-footer-guid")
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "5"),
                value("EXPR", "\"Header label\"", 51U),
                value("HPOS", "100.000"),
                value("VPOS", "50.000"),
                value("WIDTH", "700.000"),
                value("HEIGHT", "120.000"),
                value("UNIQUEID", "detail-header-label-guid")
            }
        },
        {
            .record_index = 3U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "10"),
                value("VPOS", "550.000"),
                value("HEIGHT", "200.000"),
                value("UNIQUEID", "deleted-detail-footer-guid")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#2727: report layout should be available for section UNIQUEID fixtures");
    expect(layout.sections.size() == 2U, "#2727: live detail header/footer sections should be detected");
    expect(layout.deleted_sections.size() == 1U, "#2727: deleted detail footer sections should be detected");
    expect(layout.sections[0].id == "detail-header-guid",
        "#2727: live detail-header sections should preserve UNIQUEID values as section ids");
    expect(layout.sections[0].id_field_index == 4U,
        "#2727: live detail-header section ids should retain UNIQUEID field provenance");
    expect(layout.sections[0].id_memo_block_number == 0U,
        "#2727: live detail-header section ids should expose memo block zero for character UNIQUEID fields");
    expect(layout.sections[1].id == "detail-footer-guid",
        "#2727: live detail-footer sections should preserve UNIQUEID values as section ids");
    expect(layout.deleted_sections[0].id == "deleted-detail-footer-guid",
        "#2727: deleted detail-footer sections should preserve UNIQUEID values as section ids");
    expect(layout.deleted_sections[0].id_field_index == 4U,
        "#2727: deleted detail-footer section ids should retain UNIQUEID field provenance");
    expect(layout.sections[0].objects.size() == 1U,
        "#2727: detail-header sections should continue to own placed objects");
    if (!layout.sections[0].objects.empty()) {
        expect(layout.sections[0].objects[0].containing_section_id == "detail-header-guid",
            "#2727: placed report objects should expose stable containing-section ids from band UNIQUEID values");
        expect(layout.sections[0].objects[0].containing_section_record_index == 0U,
            "#2727: placed report objects should preserve containing-section record indexes");
    }
}

}  // namespace

int main() {
    test_build_report_layout_groups_band_objects();
    test_build_report_layout_localizes_section_titles_without_localizing_band_kinds();
    test_report_layout_section_catalog_entries_cover_placeholder_locales();
    test_build_report_layout_suppresses_unresolved_memo_placeholders();
    test_build_report_layout_carries_group_section_expressions();
    test_build_report_layout_carries_nested_group_section_ordering();
    test_build_report_layout_summarizes_groupings();
    test_build_report_layout_resolves_grouping_expression_for_blank_footer();
    test_build_report_layout_resolves_grouping_expression_for_deleted_blank_footer();
    test_build_report_layout_preserves_group_pairing_when_group_header_moves_below_footer();
    test_build_report_layout_counts_deleted_objects_per_section();
    test_build_report_layout_preserves_live_objects_in_deleted_sections();
    test_build_report_layout_keeps_tall_objects_in_the_section_where_they_begin();
    test_build_report_layout_summarizes_nested_mixed_state_groupings();
    test_build_report_layout_reports_missing_title_provenance_when_unavailable();
    test_build_report_layout_includes_direct_orientation_settings();
    test_build_report_layout_includes_direct_paper_size_settings();
    test_build_report_layout_includes_direct_side_margin_settings();
    test_build_report_layout_summarizes_paper_dimensions();
    test_build_report_layout_preserves_band_unique_ids_for_section_identity();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

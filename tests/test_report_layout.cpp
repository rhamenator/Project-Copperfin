#include "copperfin/studio/report_layout.h"

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

copperfin::vfp::DbfRecordValue value(std::string name, std::string display_value) {
    return {
        .field_name = std::move(name),
        .field_type = 'C',
        .is_null = false,
        .display_value = std::move(display_value)
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
                value("EXPR", "ORIENTATION=0\r\nPAPERSIZE=1"),
                value("TOPMARGIN", "10"),
                value("BOTMARGIN", "12")
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "1"),
                value("VPOS", "0.000"),
                value("HEIGHT", "2000.000")
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
                value("OBJTYPE", "8"),
                value("OBJCODE", "0"),
                value("EXPR", "customer.company"),
                value("HPOS", "1200.000"),
                value("VPOS", "2600.000"),
                value("WIDTH", "4000.000"),
                value("HEIGHT", "450.000"),
                value("FONTFACE", "Segoe UI"),
                value("FONTSIZE", "10")
            }
        },
        {
            .record_index = 4U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "5"),
                value("EXPR", "\"Invoice\""),
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
                value("OBJTYPE", "5"),
                value("EXPR", "\"Deleted label\""),
                value("HPOS", "1000.000"),
                value("VPOS", "2600.000"),
                value("WIDTH", "1200.000"),
                value("HEIGHT", "300.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "report layout should be available for report assets");
    expect(!layout.is_label, "report layout should distinguish reports from labels");
    expect(layout.sections.size() == 2U, "report layout should detect two sections");
    expect(layout.settings.size() >= 2U, "report layout should parse root settings");
    expect(layout.sections[0].band_kind == "page_header", "first section should decode the page header band");
    expect(layout.sections[1].band_kind == "detail", "second section should decode the detail band");
    expect(layout.sections[0].objcode_code == 1, "#666: report sections should preserve raw OBJCODE values");
    expect(layout.sections[0].objcode_field_index == 1U, "#664: report section should preserve OBJCODE field provenance");
    expect(layout.sections[0].title_field_index == 1U, "#682: report section title provenance should retain OBJCODE field ordinal");
    expect(layout.sections[0].band_kind_field_index == 1U, "#682: report section band-kind provenance should retain OBJCODE field ordinal");
    expect(layout.sections[0].top_field_index == 2U, "#664: report section should preserve VPOS field provenance");
    expect(layout.sections[0].height_field_index == 3U, "#664: report section should preserve HEIGHT field provenance");
    expect(layout.sections[0].objects.size() == 1U, "page header should capture its label object");
    if (!layout.sections[0].objects.empty()) {
        expect(layout.sections[0].objects[0].objtype_field_index == 0U, "#674: present report object fields should keep DBF field provenance");
        expect(layout.sections[0].objects[0].objcode_field_index == copperfin::studio::StudioReportMissingFieldIndex,
            "#674: missing report object fields should not masquerade as DBF field zero");
        expect(layout.sections[0].objects[0].title == "\"Invoice\"", "#675: report layout object titles should keep existing EXPR fallback");
        expect(layout.sections[0].objects[0].title_field_index == 1U,
            "#675: report layout object title provenance should retain selected EXPR field ordinal");
    }
    expect(layout.sections[1].objects.size() == 1U, "detail section should capture its field object");
    expect(layout.sections[1].objects[0].object_kind == "field", "detail object should retain its type");
    expect(layout.sections[1].objects[0].object_kind_field_index == 0U, "#682: report object kind provenance should retain OBJTYPE field ordinal");
    expect(layout.sections[1].objects[0].objtype_code == 8, "#666: layout objects should preserve raw OBJTYPE values");
    expect(layout.sections[1].objects[0].objcode_code == 0, "#666: layout objects should preserve raw OBJCODE values");
    expect(layout.sections[1].objects[0].expression == "customer.company", "detail object should surface its expression");
    expect(layout.sections[1].objects[0].objtype_field_index == 0U, "#665: layout objects should preserve OBJTYPE field provenance");
    expect(layout.sections[1].objects[0].objcode_field_index == 1U, "#666: layout objects should preserve OBJCODE field provenance");
    expect(layout.sections[1].objects[0].title_field_index == 2U, "#675: detail object title provenance should retain selected EXPR field ordinal");
    expect(layout.sections[1].objects[0].expression_field_index == 2U, "#665: layout objects should preserve EXPR field provenance");
    expect(layout.sections[1].objects[0].left_field_index == 3U, "#665: layout objects should preserve HPOS field provenance");
    expect(layout.sections[1].objects[0].top_field_index == 4U, "#665: layout objects should preserve VPOS field provenance");
    expect(layout.sections[1].objects[0].width_field_index == 5U, "#665: layout objects should preserve WIDTH field provenance");
    expect(layout.sections[1].objects[0].height_field_index == 6U, "#665: layout objects should preserve HEIGHT field provenance");
    const auto orientation = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "ORIENTATION";
    });
    expect(orientation != layout.settings.end(), "#661: report settings should include parsed EXPR settings");
    if (orientation != layout.settings.end()) {
        expect(orientation->record_index == 0U, "#661: parsed EXPR settings should retain their source record index");
        expect(orientation->field_index == 2U, "#661: parsed EXPR settings should retain the source EXPR field ordinal");
        expect(orientation->source_line_index == 0U, "#676: parsed EXPR settings should retain their source memo line index");
    }
    const auto paper_size = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "PAPERSIZE";
    });
    expect(paper_size != layout.settings.end(), "#676: report settings should include later parsed EXPR settings");
    if (paper_size != layout.settings.end()) {
        expect(paper_size->source_line_index == 1U, "#676: later EXPR settings should retain their source memo line index");
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
    }
    expect(layout.unplaced_objects.size() == 1U, "#675: object without matching section should remain unplaced");
    if (!layout.unplaced_objects.empty()) {
        expect(layout.unplaced_objects[0].title == "Record 5", "#675: untitled report layout object should keep synthetic title fallback");
        expect(layout.unplaced_objects[0].title_field_index == copperfin::studio::StudioReportMissingFieldIndex,
            "#675: synthesized report layout object title should use the missing-field sentinel");
    }
    expect(layout.deleted_objects.size() == 1U, "#689: deleted report layout objects should be preserved separately");
    if (!layout.deleted_objects.empty()) {
        expect(layout.deleted_objects[0].deleted, "#689: deleted report layout object snapshots should retain deleted state");
        expect(layout.deleted_objects[0].title == "\"Deleted label\"", "#689: deleted report layout objects should retain title metadata");
        expect(layout.deleted_objects[0].title_field_index == 1U,
            "#689: deleted report layout objects should retain title provenance");
    }
}

}  // namespace

int main() {
    test_build_report_layout_groups_band_objects();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_report_layout_support.h"
#include "test_environment_support.h"

namespace cf_test_report_layout {

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
                value("BOTMARGIN", "12"),
                value("NAME", "", 77U)
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "1", 41U),
                value("VPOS", "0.000", 42U),
                value("HEIGHT", "2000.000", 43U),
                value("PAGEBREAK", "T", 44U),
                value("COLBREAK", "F", 45U),
                value("RESETPAGE", "T", 46U),
                value("EJECTBEFOR", "T", 47U),
                value("EJECTAFTER", "F", 48U),
                value("PLAIN", "T", 49U),
                value("TAG", "DO ENTRY", 50U),
                value("TAG2", "DO EXIT", 51U),
                value("COMMENT", "Band developer note", 52U),
                value("USER", "Band user comment", 53U),
                value("NOREPEAT", "T", 54U)
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "4"),
                value("VPOS", "2000.000"),
                value("HEIGHT", "5000.000"),
                value("PAGEBREAK", "", 44U),
                value("COLBREAK", "", 45U),
                value("RESETPAGE", "", 46U),
                value("EJECTBEFOR", "", 47U),
                value("EJECTAFTER", "", 48U),
                value("PLAIN", "", 49U),
                value("TAG", "", 50U),
                value("TAG2", "", 51U)
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
                value("FONTSIZE", "10"),
                value("SUPEXPR", "customer.company > 0", 313U),
                value("SUPGROUP", "6"),
                value("SUPALWAYS", "1"),
                value("SUPVALCHNG", "0"),
                value("SUPRPCOL", "0"),
                value("SUPOVFLOW", "0"),
                value("BOTTOM", "0"),
                value("TOP", "0"),
                value("FONTSTYLE", "5"),
                value("MODE", "1"),
                value("PICTURE", "@J", 314U),
                value("PENRED", "10", 315U),
                value("PENGREEN", "20", 316U),
                value("PENBLUE", "30", 317U),
                value("FILLRED", "40", 318U),
                value("FILLGREEN", "50", 319U),
                value("FILLBLUE", "60", 320U),
                value("COMMENT", "Object developer note", 321U),
                value("USER", "Object user comment", 322U),
                value("FLOAT", "T", 323U),
                value("NOREPEAT", "F", 324U),
                value("STRETCH", "T", 325U),
                value("STRETCHTOP", "F", 326U)
            }
        },
        {
            .record_index = 4U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "5"),
                value("EXPR", "\"Invoice\"", 32U),
                value("PICTURE", "@I", 37U),
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
                value("HEIGHT", "100.000"),
                value("FLOAT", "T", 501U),
                value("NOREPEAT", "F", 502U),
                value("STRETCH", "T", 503U),
                value("STRETCHTOP", "F", 504U)
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
                value("HEIGHT", "300.000", 606U),
                value("PENRED", "70", 607U),
                value("PENGREEN", "80", 608U),
                value("PENBLUE", "90", 609U),
                value("FLOAT", "F", 610U),
                value("NOREPEAT", "T", 611U),
                value("STRETCH", "F", 612U),
                value("STRETCHTOP", "T", 613U)
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
    expect(layout.document_title_field_index == 5U,
        "#4248: report document titles should retain the header NAME field provenance");
    expect(layout.document_title_memo_block_number == 77U,
        "#4248: report document titles should retain the header NAME memo provenance");
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
    expect(layout.sections[1].page_break.empty() &&
           layout.sections[1].page_break_field_index == copperfin::studio::StudioReportMissingFieldIndex &&
           layout.sections[1].column_break.empty() &&
           layout.sections[1].reset_page.empty() &&
           layout.sections[1].eject_before.empty() &&
           layout.sections[1].eject_after.empty() &&
           layout.sections[1].plain.empty() &&
           layout.sections[1].on_entry_expression.empty() &&
           layout.sections[1].on_exit_expression.empty() &&
           layout.sections[1].comment.empty() &&
           layout.sections[1].user_comment.empty() &&
           layout.sections[1].no_repeat.empty(),
        "#4531: blank section pagination flags should remain blank without fabricating provenance");
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
    expect(layout.sections[0].page_break == "T" && layout.sections[0].page_break_field_index == 4U &&
           layout.sections[0].page_break_memo_block_number == 44U,
        "#4531: report sections should preserve PAGEBREAK values and source provenance");
    expect(layout.sections[0].column_break == "F" && layout.sections[0].column_break_field_index == 5U &&
           layout.sections[0].column_break_memo_block_number == 45U,
        "#4531: report sections should preserve COLBREAK values and source provenance");
    expect(layout.sections[0].eject_before == "T" && layout.sections[0].eject_before_field_index == 7U &&
           layout.sections[0].eject_before_memo_block_number == 47U,
        "#4532: report sections should preserve EJECTBEFOR values and source provenance");
    expect(layout.sections[0].eject_after == "F" && layout.sections[0].eject_after_field_index == 8U &&
           layout.sections[0].eject_after_memo_block_number == 48U,
        "#4532: report sections should preserve EJECTAFTER values and source provenance");
    expect(layout.sections[0].plain == "T" && layout.sections[0].plain_field_index == 9U &&
           layout.sections[0].plain_memo_block_number == 49U,
        "#4532: report sections should preserve PLAIN values and source provenance");
    expect(layout.sections[0].on_entry_expression == "DO ENTRY" &&
           layout.sections[0].on_entry_expression_field_index == 10U &&
           layout.sections[0].on_entry_expression_memo_block_number == 50U,
        "#4533: report sections should preserve TAG entry expressions and source provenance");
    expect(layout.sections[0].on_exit_expression == "DO EXIT" &&
           layout.sections[0].on_exit_expression_field_index == 11U &&
           layout.sections[0].on_exit_expression_memo_block_number == 51U,
        "#4533: report sections should preserve TAG2 exit expressions and source provenance");
    expect(layout.sections[0].comment == "Band developer note" &&
           layout.sections[0].comment_field_index == 12U &&
           layout.sections[0].comment_memo_block_number == 52U,
        "#4544: report sections should preserve COMMENT values and memo provenance");
    expect(layout.sections[0].user_comment == "Band user comment" &&
           layout.sections[0].user_comment_field_index == 13U &&
           layout.sections[0].user_comment_memo_block_number == 53U,
        "#4545: report sections should preserve USER values and memo provenance");
    expect(layout.sections[0].no_repeat == "T" &&
           layout.sections[0].no_repeat_field_index == 14U &&
           layout.sections[0].no_repeat_memo_block_number == 54U,
        "#4546: report sections should preserve band NOREPEAT values and memo provenance");
    expect(layout.sections[0].reset_page == "T" && layout.sections[0].reset_page_field_index == 6U &&
           layout.sections[0].reset_page_memo_block_number == 46U,
        "#4531: report sections should preserve RESETPAGE values and source provenance");
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
        expect(layout.sections[0].objects[0].picture == "@I",
            "#4291: label objects should preserve PICTURE alignment values");
        expect(layout.sections[0].objects[0].picture_field_index == 2U,
            "#4291: label object PICTURE should preserve field provenance");
        expect(layout.sections[0].objects[0].picture_memo_block_number == 37U,
            "#4291: label object PICTURE should preserve memo provenance");
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
    const auto comment_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) { return highlight.name == "COMMENT"; });
    expect(comment_highlight != layout.sections[1].objects[0].highlights.end() &&
           comment_highlight->value == "Object developer note" &&
           comment_highlight->field_index == 26U &&
           comment_highlight->memo_block_number == 321U,
        "#4544: report objects should expose COMMENT highlights with memo provenance");
    const auto user_comment_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) { return highlight.name == "USER"; });
    expect(user_comment_highlight != layout.sections[1].objects[0].highlights.end() &&
           user_comment_highlight->value == "Object user comment" &&
           user_comment_highlight->field_index == 27U &&
           user_comment_highlight->memo_block_number == 322U,
        "#4545: report objects should expose USER highlights with memo provenance");
    const auto expect_object_flag_highlight = [&](const auto& object,
                                                  const char* name,
                                                  const char* expected_value,
                                                  std::size_t field_index,
                                                  std::uint32_t memo_block_number) {
        const auto highlight = std::find_if(
            object.highlights.begin(),
            object.highlights.end(),
            [&](const auto& candidate) { return candidate.name == name; });
        expect(highlight != object.highlights.end(),
               std::string("#4554: report layout object highlights should include ") + name);
        if (highlight != object.highlights.end()) {
            expect(highlight->value == expected_value &&
                       highlight->field_index == field_index &&
                       highlight->memo_block_number == memo_block_number,
                   std::string("#4554: report layout object highlight should preserve ") + name +
                       " value and provenance");
        }
    };
    expect_object_flag_highlight(layout.sections[1].objects[0], "FLOAT", "T", 28U, 323U);
    expect_object_flag_highlight(layout.sections[1].objects[0], "NOREPEAT", "F", 29U, 324U);
    expect_object_flag_highlight(layout.sections[1].objects[0], "STRETCH", "T", 30U, 325U);
    expect_object_flag_highlight(layout.sections[1].objects[0], "STRETCHTOP", "F", 31U, 326U);
    const auto print_when_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "SUPEXPR";
        });
    expect(print_when_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4510: report layout highlights should include the memo-backed SUPEXPR Print When expression");
    if (print_when_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(print_when_highlight->value == "customer.company > 0",
            "#4510: SUPEXPR highlights should preserve the Print When expression value");
        expect(print_when_highlight->field_index == 9U,
            "#4510: SUPEXPR highlights should retain the source DBF field ordinal");
        expect(print_when_highlight->memo_block_number == 313U,
            "#4510: SUPEXPR highlights should retain the source memo block provenance");
    }
    const auto print_when_group_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "SUPGROUP";
        });
    expect(print_when_group_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4511: report layout highlights should include the SUPGROUP group-change setting");
    if (print_when_group_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(print_when_group_highlight->value == "6" && print_when_group_highlight->field_index == 10U &&
               print_when_group_highlight->memo_block_number == 0U,
            "#4511: SUPGROUP highlights should preserve value and source field provenance");
    }
    const auto print_when_repeated_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "SUPALWAYS";
        });
    expect(print_when_repeated_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4512: report layout highlights should include the SUPALWAYS repeated-value setting");
    if (print_when_repeated_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(print_when_repeated_highlight->value == "1" && print_when_repeated_highlight->field_index == 11U &&
               print_when_repeated_highlight->memo_block_number == 0U,
            "#4512: SUPALWAYS highlights should preserve value and source field provenance");
    }
    const auto print_when_value_changes_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "SUPVALCHNG";
        });
    expect(print_when_value_changes_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4513: report layout highlights should include the SUPVALCHNG value-change setting");
    if (print_when_value_changes_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(print_when_value_changes_highlight->value == "0" && print_when_value_changes_highlight->field_index == 12U &&
               print_when_value_changes_highlight->memo_block_number == 0U,
            "#4513: SUPVALCHNG highlights should preserve value and source field provenance");
    }
    const auto print_when_page_column_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "SUPRPCOL";
        });
    expect(print_when_page_column_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4514: report layout highlights should include the SUPRPCOL page-column setting");
    if (print_when_page_column_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(print_when_page_column_highlight->value == "0" && print_when_page_column_highlight->field_index == 13U &&
               print_when_page_column_highlight->memo_block_number == 0U,
            "#4514: SUPRPCOL highlights should preserve value and source field provenance");
    }
    const auto print_when_overflow_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "SUPOVFLOW";
        });
    expect(print_when_overflow_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4515: report layout highlights should include the SUPOVFLOW overflow setting");
    if (print_when_overflow_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(print_when_overflow_highlight->value == "0" && print_when_overflow_highlight->field_index == 14U &&
               print_when_overflow_highlight->memo_block_number == 0U,
            "#4515: SUPOVFLOW highlights should preserve value and source field provenance");
    }
    const auto bottom_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "BOTTOM";
        });
    expect(bottom_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4516: report layout highlights should include the BOTTOM positioning flag");
    if (bottom_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(bottom_highlight->value == "0" && bottom_highlight->field_index == 15U &&
               bottom_highlight->memo_block_number == 0U,
            "#4516: BOTTOM highlights should preserve value and source field provenance");
    }
    const auto top_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "TOP";
        });
    expect(top_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4517: report layout highlights should include the TOP positioning flag");
    if (top_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(top_highlight->value == "0" && top_highlight->field_index == 16U &&
               top_highlight->memo_block_number == 0U,
            "#4517: TOP highlights should preserve value and source field provenance");
    }
    const auto font_style_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "FONTSTYLE";
        });
    expect(font_style_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4518: report layout highlights should include the FONTSTYLE font setting");
    if (font_style_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(font_style_highlight->value == "5" && font_style_highlight->field_index == 17U &&
               font_style_highlight->memo_block_number == 0U,
            "#4518: FONTSTYLE highlights should preserve value and source field provenance");
    }
    const auto mode_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "MODE";
        });
    expect(mode_highlight != layout.sections[1].objects[0].highlights.end(),
        "#4519: report layout highlights should include the MODE appearance setting");
    if (mode_highlight != layout.sections[1].objects[0].highlights.end()) {
        expect(mode_highlight->value == "1" && mode_highlight->field_index == 18U &&
               mode_highlight->memo_block_number == 0U,
            "#4519: MODE highlights should preserve value and source field provenance");
    }
    expect(layout.sections[1].objects[0].picture.empty() &&
           layout.sections[1].objects[0].picture_field_index == copperfin::studio::StudioReportMissingFieldIndex &&
           layout.sections[1].objects[0].picture_memo_block_number == 0U,
        "#4621: field-object PICTURE should remain raw property data instead of visual metadata");
    const auto field_picture_highlight = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "PICTURE";
        });
    expect(field_picture_highlight != layout.sections[1].objects[0].highlights.end() &&
           field_picture_highlight->value == "@J" && field_picture_highlight->field_index == 19U &&
           field_picture_highlight->memo_block_number == 314U,
        "#4621: field-object PICTURE should retain raw value and provenance in highlights");
    const auto pen_red = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "PENRED";
        });
    expect(pen_red != layout.sections[1].objects[0].highlights.end(),
        "#4534: report expression highlights should include PENRED");
    if (pen_red != layout.sections[1].objects[0].highlights.end()) {
        expect(pen_red->value == "10" && pen_red->field_index == 20U &&
               pen_red->memo_block_number == 315U,
            "#4534: PENRED should preserve value and source provenance");
    }
    const auto fill_blue = std::find_if(
        layout.sections[1].objects[0].highlights.begin(),
        layout.sections[1].objects[0].highlights.end(),
        [](const auto& highlight) {
            return highlight.name == "FILLBLUE";
        });
    expect(fill_blue != layout.sections[1].objects[0].highlights.end(),
        "#4534: report expression highlights should include FILLBLUE");
    if (fill_blue != layout.sections[1].objects[0].highlights.end()) {
        expect(fill_blue->value == "60" && fill_blue->field_index == 25U &&
               fill_blue->memo_block_number == 320U,
            "#4534: FILLBLUE should preserve value and source provenance");
    }
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
        expect_object_flag_highlight(layout.unplaced_objects[0], "FLOAT", "T", 5U, 501U);
        expect_object_flag_highlight(layout.unplaced_objects[0], "NOREPEAT", "F", 6U, 502U);
        expect_object_flag_highlight(layout.unplaced_objects[0], "STRETCH", "T", 7U, 503U);
        expect_object_flag_highlight(layout.unplaced_objects[0], "STRETCHTOP", "F", 8U, 504U);
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
        const auto deleted_pen_blue = std::find_if(
            layout.deleted_objects[0].highlights.begin(),
            layout.deleted_objects[0].highlights.end(),
            [](const auto& highlight) {
                return highlight.name == "PENBLUE";
            });
        expect(deleted_pen_blue != layout.deleted_objects[0].highlights.end(),
            "#4534: deleted label highlights should preserve PENBLUE");
        if (deleted_pen_blue != layout.deleted_objects[0].highlights.end()) {
            expect(deleted_pen_blue->value == "90" && deleted_pen_blue->field_index == 8U &&
                   deleted_pen_blue->memo_block_number == 609U,
                "#4534: deleted PENBLUE should preserve value and source provenance");
        }
        expect_object_flag_highlight(layout.deleted_objects[0], "FLOAT", "F", 9U, 610U);
        expect_object_flag_highlight(layout.deleted_objects[0], "NOREPEAT", "T", 10U, 611U);
        expect_object_flag_highlight(layout.deleted_objects[0], "STRETCH", "F", 11U, 612U);
        expect_object_flag_highlight(layout.deleted_objects[0], "STRETCHTOP", "T", 12U, 613U);
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

void test_report_variable_initial_value_tag_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "variable.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "18", 701U),
                value("OBJCODE", "0"),
                value("TAG", "customer.initial", 702U)
            }
        },
        {
            .record_index = 1U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "18", 703U),
                value("OBJCODE", "0"),
                value("TAG", "deleted.initial", 704U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "variable report layout should be available");
    expect(layout.unplaced_objects.size() == 1U,
           "variable report layout should preserve a live variable as an unplaced object");
    expect(layout.deleted_objects.size() == 1U,
           "variable report layout should preserve a deleted variable separately");

    const auto expect_tag = [](const auto& object, const char* expected_value, std::uint32_t memo_block, const char* state) {
        const auto tag = std::find_if(
            object.highlights.begin(),
            object.highlights.end(),
            [](const auto& highlight) { return highlight.name == "TAG"; });
        expect(tag != object.highlights.end(),
               std::string("#4556: ") + state + " report variable should expose TAG");
        if (tag != object.highlights.end()) {
            expect(tag->value == expected_value &&
                       tag->field_index == 2U &&
                       tag->memo_block_number == memo_block,
                   std::string("#4556: ") + state + " report variable TAG should preserve value and memo provenance");
        }
    };
    expect_tag(layout.unplaced_objects[0], "customer.initial", 702U, "live");
    expect_tag(layout.deleted_objects[0], "deleted.initial", 704U, "deleted");
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

void test_report_layout_default_catalog_refreshes_when_locale_changes() {
    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    locale_override.set("en-US");

    copperfin::studio::StudioDocumentModel document;
    document.display_name = "locale-refresh.frx";
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
        }
    };

    const auto english_layout = copperfin::studio::build_report_layout(document);
    locale_override.set("es-419");
    const auto spanish_layout = copperfin::studio::build_report_layout(document);
    locale_override.set("qps-ploc");
    const auto pseudo_layout = copperfin::studio::build_report_layout(document);

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    constexpr std::string_view title_key = "Studio.ReportLayout.Section.DetailFooter";
    expect(english_layout.sections.size() == 1U &&
               english_layout.sections[0].title == english_catalog.translate(title_key),
           "#4362: default report-layout titles should begin in en-US");
    expect(spanish_layout.sections.size() == 1U &&
               spanish_layout.sections[0].title == spanish_catalog.translate(title_key),
           "#4362: default report-layout titles should refresh to es-419");
    expect(pseudo_layout.sections.size() == 1U &&
               pseudo_layout.sections[0].title == pseudo_catalog.translate(title_key),
           "#4362: default report-layout titles should refresh to qps-ploc");
    expect(english_layout.sections.size() == 1U &&
               spanish_layout.sections.size() == 1U &&
               pseudo_layout.sections.size() == 1U &&
               english_layout.sections[0].band_kind == "detail_footer" &&
               spanish_layout.sections[0].band_kind == "detail_footer" &&
               pseudo_layout.sections[0].band_kind == "detail_footer",
           "#4362: locale refresh should preserve the invariant report band kind");
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



}  // namespace cf_test_report_layout

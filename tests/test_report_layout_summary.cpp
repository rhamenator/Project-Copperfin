// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

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

void test_build_report_layout_summarizes_color_and_copies() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "color-copies.frx";
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
                      "ORIENTATION=0\r\nPAPERSIZE=1\r\nGRIDV=4\r\nGRIDH=8\r\nCOLOR=0\r\nCOPIES=3",
                      48U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#3806: report layout should be available for COLOR/COPIES fixtures");
    expect(layout.page_setup_available, "#3806: COLOR/COPIES fixtures should preserve page setup availability");
    expect(layout.color_available && layout.color == 0,
           "#3806: COLOR fixtures should expose additive color summary metadata");
    expect(layout.copies_available && layout.copies == 3,
           "#3806: COPIES fixtures should expose additive copies summary metadata");
    expect(layout.orientation_available && layout.orientation_code == 0,
           "#3806: COLOR/COPIES fixtures should preserve memo-derived orientation");
    expect(layout.paper_size_available && layout.paper_size_code == 1,
           "#3806: COLOR/COPIES fixtures should preserve memo-derived paper size");
    expect(layout.grid_vertical_available && layout.grid_vertical == 4,
           "#3806: COLOR/COPIES fixtures should preserve memo-derived vertical grid spacing");
    expect(layout.grid_horizontal_available && layout.grid_horizontal == 8,
           "#3806: COLOR/COPIES fixtures should preserve memo-derived horizontal grid spacing");
    expect(layout.settings.size() == 6U, "#3806: COLOR/COPIES fixtures should preserve root setting counts");

    const auto color = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "COLOR";
    });
    expect(color != layout.settings.end(), "#3806: COLOR should appear in root settings");
    if (color != layout.settings.end()) {
        expect(color->record_index == 0U, "#3806: COLOR should retain source record provenance");
        expect(color->field_index == 2U, "#3806: COLOR should retain EXPR field provenance");
        expect(color->source_line_index == 4U, "#3806: COLOR should retain memo line provenance");
        expect(color->memo_block_number == 48U,
               "#3806: COLOR should expose EXPR memo provenance");
        expect(color->value == "0", "#3806: COLOR should preserve the field value text");
    }

    const auto copies = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "COPIES";
    });
    expect(copies != layout.settings.end(), "#3806: COPIES should appear in root settings");
    if (copies != layout.settings.end()) {
        expect(copies->record_index == 0U, "#3806: COPIES should retain source record provenance");
        expect(copies->field_index == 2U, "#3806: COPIES should retain EXPR field provenance");
        expect(copies->source_line_index == 5U, "#3806: COPIES should retain memo line provenance");
        expect(copies->memo_block_number == 48U,
               "#3806: COPIES should expose EXPR memo provenance");
        expect(copies->value == "3", "#3806: COPIES should preserve the field value text");
    }
}

void test_build_report_layout_summarizes_auxiliary_print_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "aux-print-settings.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value(
                    "EXPR",
                    "ORIENTATION=0\r\nPAPERSIZE=1\r\nGRIDV=4\r\nGRIDH=8\r\nDEFAULTSOURCE=15\r\nPRINTQUALITY=600\r\nYRESOLUTION=600\r\nTTOPTION=3\r\nASCII=9\r\nCOLLATE=1",
                    49U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#3818: report layout should be available for auxiliary print settings");
    expect(layout.page_setup_available, "#3818: auxiliary print settings should preserve page setup availability");
    expect(layout.orientation_available && layout.orientation_code == 0,
        "#3818: auxiliary print settings should preserve memo-derived orientation");
    expect(layout.paper_size_available && layout.paper_size_code == 1,
        "#3818: auxiliary print settings should preserve memo-derived paper size");
    expect(layout.grid_vertical_available && layout.grid_vertical == 4,
        "#3818: auxiliary print settings should preserve memo-derived vertical grid spacing");
    expect(layout.grid_horizontal_available && layout.grid_horizontal == 8,
        "#3818: auxiliary print settings should preserve memo-derived horizontal grid spacing");
    expect(layout.default_source_available && layout.default_source == 15,
        "#3818: auxiliary print settings should expose DEFAULTSOURCE summary values");
    expect(layout.print_quality_available && layout.print_quality == 600,
        "#3818: auxiliary print settings should expose PRINTQUALITY summary values");
    expect(layout.y_resolution_available && layout.y_resolution == 600,
        "#3818: auxiliary print settings should expose YRESOLUTION summary values");
    expect(layout.true_type_option_available && layout.true_type_option == 3,
        "#3818: auxiliary print settings should expose TTOPTION summary values");
    expect(layout.ascii_available && layout.ascii == 9,
        "#3818: auxiliary print settings should expose ASCII summary values");
    expect(layout.collate_available && layout.collate == 1,
        "#3818: auxiliary print settings should expose COLLATE summary values");
    expect(layout.settings.size() == 10U, "#3818: auxiliary print settings should preserve root setting counts");
}

void test_build_report_layout_falls_back_to_deleted_root_summary_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "deleted-root-summary.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value(
                    "EXPR",
                    "ORIENTATION=1\r\nPAPERSIZE=9\r\nPAPERLENGTH=2794\r\nPAPERWIDTH=2159\r\nBOTMARGIN=20\r\nGRIDV=4\r\nGRIDH=8\r\nCOLOR=0\r\nCOPIES=3\r\nDRIVER=HP LaserJet\r\nDEVICE=winspool\r\nOUTPUT=invoice.pdf\r\nDEFAULTSOURCE=15\r\nPRINTQUALITY=600\r\nYRESOLUTION=600\r\nTTOPTION=3\r\nASCII=9\r\nCOLLATE=1\r\nCOLS=2\r\nCOLWIDTH=3000\r\nCOLSPACING=250",
                    51U),
                value("TOPMARGIN", "10"),
                value("TAG", "customer.country", 52U),
                value("LEFTMARGIN", "15"),
                value("RIGHTMARGIN", "25")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#3815: deleted-only report layout should still be available");
    expect(layout.settings.empty(), "#3815: deleted-only report layout should keep live root settings empty");
    expect(layout.deleted_settings.size() == 25U,
        "#3815: deleted-only report layout should preserve deleted root setting counts");
    expect(layout.page_setup_available, "#3815: deleted root settings should surface page-setup availability");
    expect(layout.orientation_available && layout.orientation_code == 1,
        "#3815: deleted root settings should surface orientation summary values");
    expect(layout.paper_size_available && layout.paper_size_code == 9,
        "#3815: deleted root settings should surface paper-size summary values");
    expect(layout.paper_length_available && layout.paper_length == 2794,
        "#3815: deleted root settings should surface paper-length summary values");
    expect(layout.paper_width_available && layout.paper_width == 2159,
        "#3815: deleted root settings should surface paper-width summary values");
    expect(layout.top_margin_available && layout.top_margin == 10,
        "#3815: deleted root settings should surface top-margin summary values");
    expect(layout.bottom_margin_available && layout.bottom_margin == 20,
        "#3815: deleted root settings should surface bottom-margin summary values");
    expect(layout.left_margin_available && layout.left_margin == 15,
        "#3815: deleted root settings should surface left-margin summary values");
    expect(layout.right_margin_available && layout.right_margin == 25,
        "#3815: deleted root settings should surface right-margin summary values");
    expect(layout.grid_vertical_available && layout.grid_vertical == 4,
        "#3815: deleted root settings should surface vertical-grid summary values");
    expect(layout.grid_horizontal_available && layout.grid_horizontal == 8,
        "#3815: deleted root settings should surface horizontal-grid summary values");
    expect(layout.color_available && layout.color == 0,
        "#3815: deleted root settings should surface COLOR summary values");
    expect(layout.copies_available && layout.copies == 3,
        "#3815: deleted root settings should surface COPIES summary values");
    expect(layout.driver_available && layout.driver == "HP LaserJet",
        "#3815: deleted root settings should surface DRIVER summary values");
    expect(layout.device_available && layout.device == "winspool",
        "#3815: deleted root settings should surface DEVICE summary values");
    expect(layout.output_available && layout.output == "invoice.pdf",
        "#3815: deleted root settings should surface OUTPUT summary values");
    expect(layout.default_source_available && layout.default_source == 15,
        "#3818: deleted root settings should surface DEFAULTSOURCE summary values");
    expect(layout.print_quality_available && layout.print_quality == 600,
        "#3818: deleted root settings should surface PRINTQUALITY summary values");
    expect(layout.y_resolution_available && layout.y_resolution == 600,
        "#3818: deleted root settings should surface YRESOLUTION summary values");
    expect(layout.true_type_option_available && layout.true_type_option == 3,
        "#3818: deleted root settings should surface TTOPTION summary values");
    expect(layout.ascii_available && layout.ascii == 9,
        "#3818: deleted root settings should surface ASCII summary values");
    expect(layout.collate_available && layout.collate == 1,
        "#3818: deleted root settings should surface COLLATE summary values");
    expect(layout.column_setup_available, "#3815: deleted root settings should surface column-setup availability");
    expect(layout.column_count_available && layout.column_count == 2,
        "#3815: deleted root settings should surface column-count summary values");
    expect(layout.column_width_available && layout.column_width == 3000,
        "#3815: deleted root settings should surface column-width summary values");
    expect(layout.column_spacing_available && layout.column_spacing == 250,
        "#3815: deleted root settings should surface column-spacing summary values");
    expect(layout.sort_expression_available && layout.sort_expression == "customer.country",
        "#3815: deleted root settings should surface TAG-derived sort summary values");
}

void test_build_report_layout_prefers_live_root_summary_settings_over_deleted_fallback() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "live-root-preferred.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=0\r\nPAPERSIZE=1", 53U),
                value("TOPMARGIN", "10")
            }
        },
        {
            .record_index = 1U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "PAPERLENGTH=2794\r\nPAPERWIDTH=2159\r\nDEFAULTSOURCE=15\r\nPRINTQUALITY=600\r\nYRESOLUTION=600\r\nTTOPTION=3\r\nASCII=9\r\nCOLLATE=1\r\nCOLS=2\r\nCOLWIDTH=3000\r\nCOLSPACING=250", 54U),
                value("TOPMARGIN", "99"),
                value("TAG", "customer.country", 55U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#3815: mixed live/deleted report layout should be available");
    expect(layout.settings.size() == 3U, "#3815: mixed live/deleted report layout should keep live root settings intact");
    expect(layout.deleted_settings.size() == 13U,
        "#3815: mixed live/deleted report layout should keep deleted root settings separate");
    expect(layout.page_setup_available, "#3815: live root settings should still surface page-setup availability");
    expect(layout.orientation_available && layout.orientation_code == 0,
        "#3815: live root settings should win for orientation summary values");
    expect(layout.paper_size_available && layout.paper_size_code == 1,
        "#3815: live root settings should win for paper-size summary values");
    expect(layout.top_margin_available && layout.top_margin == 10,
        "#3815: live root settings should win for direct top-margin summary values");
    expect(!layout.paper_length_available && layout.paper_length == 0,
        "#3815: deleted root paper length should not backfill live-root summary gaps");
    expect(!layout.paper_width_available && layout.paper_width == 0,
        "#3815: deleted root paper width should not backfill live-root summary gaps");
    expect(!layout.column_setup_available, "#3815: deleted root column settings should not backfill live-root summary gaps");
    expect(!layout.column_count_available && layout.column_count == 0,
        "#3815: deleted root column counts should not backfill live-root summary gaps");
    expect(!layout.column_width_available && layout.column_width == 0,
        "#3815: deleted root column widths should not backfill live-root summary gaps");
    expect(!layout.column_spacing_available && layout.column_spacing == 0,
        "#3815: deleted root column spacing should not backfill live-root summary gaps");
    expect(!layout.sort_expression_available && layout.sort_expression.empty(),
        "#3815: deleted root TAG should not backfill live-root sort summary gaps");
    expect(!layout.default_source_available && layout.default_source == 0,
        "#3818: deleted root DEFAULTSOURCE should not backfill live-root summary gaps");
    expect(!layout.print_quality_available && layout.print_quality == 0,
        "#3818: deleted root PRINTQUALITY should not backfill live-root summary gaps");
    expect(!layout.y_resolution_available && layout.y_resolution == 0,
        "#3818: deleted root YRESOLUTION should not backfill live-root summary gaps");
    expect(!layout.true_type_option_available && layout.true_type_option == 0,
        "#3818: deleted root TTOPTION should not backfill live-root summary gaps");
    expect(!layout.ascii_available && layout.ascii == 0,
        "#3818: deleted root ASCII should not backfill live-root summary gaps");
    expect(!layout.collate_available && layout.collate == 0,
        "#3818: deleted root COLLATE should not backfill live-root summary gaps");
}

void test_build_report_layout_summarizes_root_sort_expression() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "sort-expression.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=1\r\nPAPERSIZE=9", 48U),
                value("TAG", "customer.country", 49U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#3745: report layout should be available for root sort settings");
    expect(layout.page_setup_available, "#3745: root sort settings should preserve page setup availability");
    expect(layout.orientation_available && layout.orientation_code == 1,
        "#3745: root sort settings should preserve memo-derived orientation");
    expect(layout.paper_size_available && layout.paper_size_code == 9,
        "#3745: root sort settings should preserve memo-derived paper size");
    expect(layout.sort_expression_available,
        "#3745: root sort settings should expose sort-expression availability");
    expect(layout.sort_expression == "customer.country",
        "#3745: root sort settings should expose the root sort expression");
    expect(layout.settings.size() == 3U, "#3745: root sort settings should preserve root setting counts");

    const auto tag = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "TAG";
    });
    expect(tag != layout.settings.end(), "#3745: root sort settings should appear in root settings");
    if (tag != layout.settings.end()) {
        expect(tag->record_index == 0U, "#3745: root sort settings should retain source record provenance");
        expect(tag->field_index == 3U, "#3745: root sort settings should retain DBF field provenance");
        expect(tag->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
            "#3745: root sort settings should not masquerade as memo-line settings");
        expect(tag->memo_block_number == 49U,
            "#3745: root sort settings should expose direct TAG memo provenance");
        expect(tag->value == "customer.country",
            "#3745: root sort settings should preserve the TAG field value text");
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



}  // namespace cf_test_report_layout

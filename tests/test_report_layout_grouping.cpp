// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

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
               layout.deleted_preview_bounds_right == 2700 &&
               layout.deleted_preview_bounds_bottom == 2000,
           "#3840: live objects retained inside deleted sections should expand deleted recovery preview bounds");
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



}  // namespace cf_test_report_layout

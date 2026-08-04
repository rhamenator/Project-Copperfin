// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "visual_asset_editor_support.h"

namespace copperfin::vfp {
VisualAssetEditResult set_visual_object_lock_columns(const VisualObjectLockColumnsRequest& request) {
    if (request.lock_columns < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("LockColumns")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "LockColumns",
        visual_asset_text("VisualAssetEditor.PropertyLabel.LockColumns"),
        std::to_string(request.lock_columns));
}

VisualAssetEditResult set_visual_object_lock_columns_left(const VisualObjectLockColumnsLeftRequest& request) {
    if (request.lock_columns_left < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("LockColumnsLeft")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "LockColumnsLeft",
        visual_asset_text("VisualAssetEditor.PropertyLabel.LockColumnsLeft"),
        std::to_string(request.lock_columns_left));
}

VisualAssetEditResult set_visual_object_record_source(const VisualObjectRecordSourceRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "RecordSource",
        visual_asset_text("VisualAssetEditor.PropertyLabel.RecordSource"),
        request.record_source);
}

VisualAssetEditResult set_visual_object_link_master(const VisualObjectLinkMasterRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "LinkMaster",
        visual_asset_text("VisualAssetEditor.PropertyLabel.LinkMaster"),
        request.link_master);
}

VisualAssetEditResult set_visual_object_initial_selected_alias(const VisualObjectInitialSelectedAliasRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "InitialSelectedAlias",
        visual_asset_text("VisualAssetEditor.PropertyLabel.InitialSelectedAlias"),
        request.initial_selected_alias);
}

VisualAssetEditResult set_visual_object_default_file_path(const VisualObjectDefaultFilePathRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "DefaultFilePath",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DefaultFilePath"),
        request.default_file_path);
}

VisualAssetEditResult set_visual_object_form_set_class(const VisualObjectFormSetClassRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "FormSetClass",
        visual_asset_text("VisualAssetEditor.PropertyLabel.FormSetClass"),
        request.form_set_class);
}

VisualAssetEditResult set_visual_object_record_source_type(const VisualObjectRecordSourceTypeRequest& request) {
    if (request.record_source_type < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("RecordSourceType")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "RecordSourceType",
        visual_asset_text("VisualAssetEditor.PropertyLabel.RecordSourceType"),
        std::to_string(request.record_source_type));
}

VisualAssetEditResult set_visual_object_partition(const VisualObjectPartitionRequest& request) {
    if (request.partition < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("Partition")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Partition",
        visual_asset_text("VisualAssetEditor.PropertyLabel.Partition"),
        std::to_string(request.partition));
}

VisualAssetEditResult set_visual_object_column_order(const VisualObjectColumnOrderRequest& request) {
    if (request.column_order < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ColumnOrder")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ColumnOrder",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ColumnOrder"),
        std::to_string(request.column_order));
}

VisualAssetEditResult set_visual_object_child_order(const VisualObjectChildOrderRequest& request) {
    if (request.child_order < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ChildOrder")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ChildOrder",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ChildOrder"),
        std::to_string(request.child_order));
}

VisualAssetEditResult set_visual_object_data_session(const VisualObjectDataSessionRequest& request) {
    if (request.data_session < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("DataSession")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DataSession",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DataSession"),
        std::to_string(request.data_session));
}

VisualAssetEditResult set_visual_object_row_source(const VisualObjectRowSourceRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "RowSource",
        visual_asset_text("VisualAssetEditor.PropertyLabel.RowSource"),
        request.row_source);
}

VisualAssetEditResult set_visual_object_row_source_type(const VisualObjectRowSourceTypeRequest& request) {
    if (request.row_source_type < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("RowSourceType")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "RowSourceType",
        visual_asset_text("VisualAssetEditor.PropertyLabel.RowSourceType"),
        std::to_string(request.row_source_type));
}

VisualAssetEditResult set_visual_object_bound_column(const VisualObjectBoundColumnRequest& request) {
    if (request.bound_column < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("BoundColumn")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BoundColumn",
        visual_asset_text("VisualAssetEditor.PropertyLabel.BoundColumn"),
        std::to_string(request.bound_column));
}

VisualAssetEditResult set_visual_object_button_count(const VisualObjectButtonCountRequest& request) {
    if (request.button_count < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ButtonCount")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ButtonCount",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ButtonCount"),
        std::to_string(request.button_count));
}

VisualAssetEditResult set_visual_object_column_count(const VisualObjectColumnCountRequest& request) {
    if (request.column_count < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ColumnCount")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ColumnCount",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ColumnCount"),
        std::to_string(request.column_count));
}

VisualAssetEditResult set_visual_object_column_widths(const VisualObjectColumnWidthsRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "ColumnWidths",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ColumnWidths"),
        request.column_widths);
}

VisualAssetEditResult set_visual_object_column_lines(const VisualObjectColumnLinesRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ColumnLines",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ColumnLines"),
        request.column_lines ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_integral_height(const VisualObjectIntegralHeightRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "IntegralHeight",
        visual_asset_text("VisualAssetEditor.PropertyLabel.IntegralHeight"),
        request.integral_height ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_incremental_search(const VisualObjectIncrementalSearchRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "IncrementalSearch",
        visual_asset_text("VisualAssetEditor.PropertyLabel.IncrementalSearch"),
        request.incremental_search ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_multi_select(const VisualObjectMultiSelectRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MultiSelect",
        visual_asset_text("VisualAssetEditor.PropertyLabel.MultiSelect"),
        request.multi_select ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_list_index(const VisualObjectListIndexRequest& request) {
    if (request.list_index < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("ListIndex")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ListIndex",
        visual_asset_text("VisualAssetEditor.PropertyLabel.ListIndex"),
        std::to_string(request.list_index));
}

VisualAssetEditResult set_visual_object_left_column(const VisualObjectLeftColumnRequest& request) {
    if (request.left_column < 0) {
        return {.ok = false, .error = visual_asset_property_non_negative_text("LeftColumn")};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "LeftColumn",
        visual_asset_text("VisualAssetEditor.PropertyLabel.LeftColumn"),
        std::to_string(request.left_column));
}

VisualAssetEditResult set_visual_object_display_value(const VisualObjectDisplayValueRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "DisplayValue",
        visual_asset_text("VisualAssetEditor.PropertyLabel.DisplayValue"),
        request.display_value);
}

}  // namespace copperfin::vfp

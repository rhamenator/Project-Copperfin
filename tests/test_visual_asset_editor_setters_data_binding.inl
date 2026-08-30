void test_set_visual_object_lock_columns_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#920",
        "lock_columns",
        "LockColumns",
        "LOCKCOLUMNS",
        "lock-columns",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_lock_columns({
                .path = path,
                .objects = objects,
                .lock_columns = value
            });
        });
}

void test_set_visual_object_lock_columns_left_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#921",
        "lock_columns_left",
        "LockColumnsLeft",
        "LOCKCOLUMNSLEFT",
        "lock-columns-left",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_lock_columns_left({
                .path = path,
                .objects = objects,
                .lock_columns_left = value
            });
        });
}

void test_set_visual_object_record_source_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#930",
        "record_source",
        "RecordSource",
        "RECORDSOURCE",
        "record-source",
        "customers",
        "orders",
        "states",
        "customers.active",
        "archive.records",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_record_source({
                .path = path,
                .objects = objects,
                .record_source = value
            });
        });
}

void test_set_visual_object_link_master_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#931",
        "link_master",
        "LinkMaster",
        "LINKMASTER",
        "link-master",
        "parent_form",
        "customers",
        "orders",
        "customers.active",
        "archive.parent",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_link_master({
                .path = path,
                .objects = objects,
                .link_master = value
            });
        });
}

void test_set_visual_object_initial_selected_alias_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#944",
        "initial_selected_alias",
        "InitialSelectedAlias",
        "INITIALSELECTEDALIAS",
        "initial-selected alias",
        "customers",
        "orders",
        "states",
        "customers.active",
        "archive.records",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_initial_selected_alias({
                .path = path,
                .objects = objects,
                .initial_selected_alias = value
            });
        });
}

void test_set_visual_object_default_file_path_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#945",
        "default_file_path",
        "DefaultFilePath",
        "DEFAULTFILEPATH",
        "default file path",
        "data",
        "reports",
        "forms",
        "c:\\data\\customers",
        "archive\\records",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_default_file_path({
                .path = path,
                .objects = objects,
                .default_file_path = value
            });
        });
}

void test_set_visual_object_form_set_class_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#946",
        "form_set_class",
        "FormSetClass",
        "FORMSETCLASS",
        "form-set class",
        "BaseFormSet",
        "CustomerFormSet",
        "OrderFormSet",
        "forms.customer_set",
        "archive.form_set",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_form_set_class({
                .path = path,
                .objects = objects,
                .form_set_class = value
            });
        });
}

void test_set_visual_object_record_source_type_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#929",
        "record_source_type",
        "RecordSourceType",
        "RECORDSOURCETYPE",
        "record-source-type",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_record_source_type({
                .path = path,
                .objects = objects,
                .record_source_type = value
            });
        });
}

void test_set_visual_object_partition_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#933",
        "partition",
        "Partition",
        "PARTITION",
        "partition",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_partition({
                .path = path,
                .objects = objects,
                .partition = value
            });
        });
}

void test_set_visual_object_column_order_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#936",
        "column_order",
        "ColumnOrder",
        "COLUMNORDER",
        "column-order",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_column_order({
                .path = path,
                .objects = objects,
                .column_order = value
            });
        });
}

void test_set_visual_object_child_order_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#938",
        "child_order",
        "ChildOrder",
        "CHILDORDER",
        "child-order",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_child_order({
                .path = path,
                .objects = objects,
                .child_order = value
            });
        });
}

void test_set_visual_object_data_session_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#916",
        "data_session",
        "DataSession",
        "DATASESSION",
        "data-session",
        1,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_data_session({
                .path = path,
                .objects = objects,
                .data_session = value
            });
        });
}

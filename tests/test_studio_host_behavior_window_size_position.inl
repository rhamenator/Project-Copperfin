void test_parse_launch_arguments_for_min_height_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--min-height-object",
        "--min-height", "640",
        "--min-height-target-object-name", "frmCustomer",
        "--min-height-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1156: launch contract should parse min-height-object requests");
    expect(result.request.min_height_object,
        "#1156: launch contract should detect --min-height-object");
    expect(result.request.min_height_available && result.request.min_height == 640,
        "#1156: min-height-object requests should carry min height value");
    expect(result.request.min_height_objects.size() == 2U,
        "#1156: min-height-object requests should collect min-height target selectors");
    if (result.request.min_height_objects.size() == 2U) {
        expect(result.request.min_height_objects[0].object_name == "frmCustomer" &&
                result.request.min_height_objects[0].unique_id.empty(),
            "#1156: min-height-object requests should parse target object-name selectors");
        expect(result.request.min_height_objects[1].object_name.empty() &&
                result.request.min_height_objects[1].unique_id == "two-guid",
            "#1156: min-height-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_min_height_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-height-object",
        "--min-height-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1156: launch contract should reject min-height-object requests without min height value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-height-object",
        "--min-height", "640"
    });
    expect(!missing_targets_result.ok,
        "#1156: launch contract should reject min-height-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-height-object",
        "--min-height", "wide",
        "--min-height-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1156: launch contract should reject non-integer min-height values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-height-object",
        "--min-height", "-1",
        "--min-height-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1156: launch contract should reject negative min-height values");
}

void test_parse_launch_arguments_rejects_min_height_object_ambiguity() {
    const auto min_height_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-height-object",
        "--allow-output-object",
        "--min-height", "640",
        "--min-height-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!min_height_allow_output_result.ok,
        "#1156: launch contract should reject simultaneous min-height-object and allow-output-object requests");

    const auto min_height_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-height-object",
        "--clear-property",
        "--property-name", "MinHeight",
        "--min-height", "640",
        "--min-height-target-unique-id", "one-guid"
    });
    expect(!min_height_property_result.ok,
        "#1156: launch contract should reject min-height-object combined with property commands");

    const auto stray_min_height_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-height", "640"
    });
    expect(!stray_min_height_result.ok,
        "#1156: launch contract should reject stray min-height arguments");
}

void test_parse_launch_arguments_for_min_width_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--min-width-object",
        "--min-width", "640",
        "--min-width-target-object-name", "frmCustomer",
        "--min-width-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1157: launch contract should parse min-width-object requests");
    expect(result.request.min_width_object,
        "#1157: launch contract should detect --min-width-object");
    expect(result.request.min_width_available && result.request.min_width == 640,
        "#1157: min-width-object requests should carry min width value");
    expect(result.request.min_width_objects.size() == 2U,
        "#1157: min-width-object requests should collect min-width target selectors");
    if (result.request.min_width_objects.size() == 2U) {
        expect(result.request.min_width_objects[0].object_name == "frmCustomer" &&
                result.request.min_width_objects[0].unique_id.empty(),
            "#1157: min-width-object requests should parse target object-name selectors");
        expect(result.request.min_width_objects[1].object_name.empty() &&
                result.request.min_width_objects[1].unique_id == "two-guid",
            "#1157: min-width-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_min_width_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-width-object",
        "--min-width-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1157: launch contract should reject min-width-object requests without min width value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-width-object",
        "--min-width", "640"
    });
    expect(!missing_targets_result.ok,
        "#1157: launch contract should reject min-width-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-width-object",
        "--min-width", "wide",
        "--min-width-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1157: launch contract should reject non-integer min-width values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-width-object",
        "--min-width", "-1",
        "--min-width-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1157: launch contract should reject negative min-width values");
}

void test_parse_launch_arguments_rejects_min_width_object_ambiguity() {
    const auto min_width_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-width-object",
        "--allow-output-object",
        "--min-width", "640",
        "--min-width-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!min_width_allow_output_result.ok,
        "#1157: launch contract should reject simultaneous min-width-object and allow-output-object requests");

    const auto min_width_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-width-object",
        "--clear-property",
        "--property-name", "MinWidth",
        "--min-width", "640",
        "--min-width-target-unique-id", "one-guid"
    });
    expect(!min_width_property_result.ok,
        "#1157: launch contract should reject min-width-object combined with property commands");

    const auto stray_min_width_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-width", "640"
    });
    expect(!stray_min_width_result.ok,
        "#1157: launch contract should reject stray min-width arguments");
}

void test_parse_launch_arguments_for_max_height_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--max-height-object",
        "--max-height", "640",
        "--max-height-target-object-name", "frmCustomer",
        "--max-height-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1151: launch contract should parse max-height-object requests");
    expect(result.request.max_height_object,
        "#1151: launch contract should detect --max-height-object");
    expect(result.request.max_height_available && result.request.max_height == 640,
        "#1151: max-height-object requests should carry max height value");
    expect(result.request.max_height_objects.size() == 2U,
        "#1151: max-height-object requests should collect max-height target selectors");
    if (result.request.max_height_objects.size() == 2U) {
        expect(result.request.max_height_objects[0].object_name == "frmCustomer" &&
                result.request.max_height_objects[0].unique_id.empty(),
            "#1151: max-height-object requests should parse target object-name selectors");
        expect(result.request.max_height_objects[1].object_name.empty() &&
                result.request.max_height_objects[1].unique_id == "two-guid",
            "#1151: max-height-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_max_height_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-height-object",
        "--max-height-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1151: launch contract should reject max-height-object requests without max height value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-height-object",
        "--max-height", "640"
    });
    expect(!missing_targets_result.ok,
        "#1151: launch contract should reject max-height-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-height-object",
        "--max-height", "tall",
        "--max-height-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1151: launch contract should reject non-integer max-height values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-height-object",
        "--max-height", "-1",
        "--max-height-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1151: launch contract should reject negative max-height values");
}

void test_parse_launch_arguments_rejects_max_height_object_ambiguity() {
    const auto max_height_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-height-object",
        "--allow-output-object",
        "--max-height", "640",
        "--max-height-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!max_height_allow_output_result.ok,
        "#1151: launch contract should reject simultaneous max-height-object and allow-output-object requests");

    const auto max_height_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-height-object",
        "--clear-property",
        "--property-name", "MaxHeight",
        "--max-height", "640",
        "--max-height-target-unique-id", "one-guid"
    });
    expect(!max_height_property_result.ok,
        "#1151: launch contract should reject max-height-object combined with property commands");

    const auto stray_max_height_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-height", "640"
    });
    expect(!stray_max_height_result.ok,
        "#1151: launch contract should reject stray max-height arguments");
}

void test_parse_launch_arguments_for_movable_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--movable-object",
        "--movable", "false",
        "--movable-target-object-name", "frmCustomer",
        "--movable-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1158: launch contract should parse movable-object requests");
    expect(result.request.movable_object,
        "#1158: launch contract should detect --movable-object");
    expect(result.request.movable_available && !result.request.movable,
        "#1158: movable-object requests should carry movable state");
    expect(result.request.movable_objects.size() == 2U,
        "#1158: movable-object requests should collect movable target selectors");
    if (result.request.movable_objects.size() == 2U) {
        expect(result.request.movable_objects[0].object_name == "frmCustomer" &&
                result.request.movable_objects[0].unique_id.empty(),
            "#1158: movable-object requests should parse target object-name selectors");
        expect(result.request.movable_objects[1].object_name.empty() &&
                result.request.movable_objects[1].unique_id == "two-guid",
            "#1158: movable-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_movable_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--movable-object",
        "--movable-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1158: launch contract should reject movable-object requests without movable state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--movable-object",
        "--movable", "false"
    });
    expect(!missing_targets_result.ok,
        "#1158: launch contract should reject movable-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--movable-object",
        "--movable", "sometimes",
        "--movable-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1158: launch contract should reject invalid movable boolean values");
}

void test_parse_launch_arguments_rejects_movable_object_ambiguity() {
    const auto movable_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--movable-object",
        "--allow-output-object",
        "--movable", "false",
        "--movable-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!movable_allow_output_result.ok,
        "#1158: launch contract should reject simultaneous movable-object and allow-output-object requests");

    const auto movable_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--movable-object",
        "--clear-property",
        "--property-name", "Movable",
        "--movable", "false",
        "--movable-target-unique-id", "one-guid"
    });
    expect(!movable_property_result.ok,
        "#1158: launch contract should reject movable-object combined with property commands");

    const auto stray_movable_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--movable", "false"
    });
    expect(!stray_movable_result.ok,
        "#1158: launch contract should reject stray movable arguments");
}

void test_parse_launch_arguments_for_half_height_caption_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--half-height-caption-object",
        "--half-height-caption", "false",
        "--half-height-caption-target-object-name", "frmCustomer",
        "--half-height-caption-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1159: launch contract should parse half-height-caption-object requests");
    expect(result.request.half_height_caption_object,
        "#1159: launch contract should detect --half-height-caption-object");
    expect(result.request.half_height_caption_available && !result.request.half_height_caption,
        "#1159: half-height-caption-object requests should carry half-height-caption state");
    expect(result.request.half_height_caption_objects.size() == 2U,
        "#1159: half-height-caption-object requests should collect half-height-caption target selectors");
    if (result.request.half_height_caption_objects.size() == 2U) {
        expect(result.request.half_height_caption_objects[0].object_name == "frmCustomer" &&
                result.request.half_height_caption_objects[0].unique_id.empty(),
            "#1159: half-height-caption-object requests should parse target object-name selectors");
        expect(result.request.half_height_caption_objects[1].object_name.empty() &&
                result.request.half_height_caption_objects[1].unique_id == "two-guid",
            "#1159: half-height-caption-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_half_height_caption_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--half-height-caption-object",
        "--half-height-caption-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1159: launch contract should reject half-height-caption-object requests without half-height-caption state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--half-height-caption-object",
        "--half-height-caption", "false"
    });
    expect(!missing_targets_result.ok,
        "#1159: launch contract should reject half-height-caption-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--half-height-caption-object",
        "--half-height-caption", "sometimes",
        "--half-height-caption-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1159: launch contract should reject invalid half-height-caption boolean values");
}

void test_parse_launch_arguments_rejects_half_height_caption_object_ambiguity() {
    const auto half_height_caption_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--half-height-caption-object",
        "--allow-output-object",
        "--half-height-caption", "false",
        "--half-height-caption-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!half_height_caption_allow_output_result.ok,
        "#1159: launch contract should reject simultaneous half-height-caption-object and allow-output-object requests");

    const auto half_height_caption_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--half-height-caption-object",
        "--clear-property",
        "--property-name", "HalfHeightCaption",
        "--half-height-caption", "false",
        "--half-height-caption-target-unique-id", "one-guid"
    });
    expect(!half_height_caption_property_result.ok,
        "#1159: launch contract should reject half-height-caption-object combined with property commands");

    const auto stray_half_height_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--half-height-caption", "false"
    });
    expect(!stray_half_height_caption_result.ok,
        "#1159: launch contract should reject stray half-height-caption arguments");
}

void test_parse_launch_arguments_for_mdi_form_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--mdi-form-object",
        "--mdi-form", "false",
        "--mdi-form-target-object-name", "frmCustomer",
        "--mdi-form-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1160: launch contract should parse mdi-form-object requests");
    expect(result.request.mdi_form_object,
        "#1160: launch contract should detect --mdi-form-object");
    expect(result.request.mdi_form_available && !result.request.mdi_form,
        "#1160: mdi-form-object requests should carry MDI-form state");
    expect(result.request.mdi_form_objects.size() == 2U,
        "#1160: mdi-form-object requests should collect MDI-form target selectors");
    if (result.request.mdi_form_objects.size() == 2U) {
        expect(result.request.mdi_form_objects[0].object_name == "frmCustomer" &&
                result.request.mdi_form_objects[0].unique_id.empty(),
            "#1160: mdi-form-object requests should parse target object-name selectors");
        expect(result.request.mdi_form_objects[1].object_name.empty() &&
                result.request.mdi_form_objects[1].unique_id == "two-guid",
            "#1160: mdi-form-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_mdi_form_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mdi-form-object",
        "--mdi-form-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1160: launch contract should reject mdi-form-object requests without MDI-form state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mdi-form-object",
        "--mdi-form", "false"
    });
    expect(!missing_targets_result.ok,
        "#1160: launch contract should reject mdi-form-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mdi-form-object",
        "--mdi-form", "sometimes",
        "--mdi-form-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1160: launch contract should reject invalid MDI-form boolean values");
}

void test_parse_launch_arguments_rejects_mdi_form_object_ambiguity() {
    const auto mdi_form_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mdi-form-object",
        "--allow-output-object",
        "--mdi-form", "false",
        "--mdi-form-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!mdi_form_allow_output_result.ok,
        "#1160: launch contract should reject simultaneous mdi-form-object and allow-output-object requests");

    const auto mdi_form_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mdi-form-object",
        "--clear-property",
        "--property-name", "MDIForm",
        "--mdi-form", "false",
        "--mdi-form-target-unique-id", "one-guid"
    });
    expect(!mdi_form_property_result.ok,
        "#1160: launch contract should reject mdi-form-object combined with property commands");

    const auto stray_mdi_form_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mdi-form", "false"
    });
    expect(!stray_mdi_form_result.ok,
        "#1160: launch contract should reject stray MDI-form arguments");
}

void test_parse_launch_arguments_for_max_width_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--max-width-object",
        "--max-width", "640",
        "--max-width-target-object-name", "frmCustomer",
        "--max-width-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1152: launch contract should parse max-width-object requests");
    expect(result.request.max_width_object,
        "#1152: launch contract should detect --max-width-object");
    expect(result.request.max_width_available && result.request.max_width == 640,
        "#1152: max-width-object requests should carry max width value");
    expect(result.request.max_width_objects.size() == 2U,
        "#1152: max-width-object requests should collect max-width target selectors");
    if (result.request.max_width_objects.size() == 2U) {
        expect(result.request.max_width_objects[0].object_name == "frmCustomer" &&
                result.request.max_width_objects[0].unique_id.empty(),
            "#1152: max-width-object requests should parse target object-name selectors");
        expect(result.request.max_width_objects[1].object_name.empty() &&
                result.request.max_width_objects[1].unique_id == "two-guid",
            "#1152: max-width-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_max_width_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-width-object",
        "--max-width-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1152: launch contract should reject max-width-object requests without max width value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-width-object",
        "--max-width", "640"
    });
    expect(!missing_targets_result.ok,
        "#1152: launch contract should reject max-width-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-width-object",
        "--max-width", "wide",
        "--max-width-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1152: launch contract should reject non-integer max-width values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-width-object",
        "--max-width", "-1",
        "--max-width-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1152: launch contract should reject negative max-width values");
}

void test_parse_launch_arguments_rejects_max_width_object_ambiguity() {
    const auto max_width_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-width-object",
        "--allow-output-object",
        "--max-width", "640",
        "--max-width-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!max_width_allow_output_result.ok,
        "#1152: launch contract should reject simultaneous max-width-object and allow-output-object requests");

    const auto max_width_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-width-object",
        "--clear-property",
        "--property-name", "MaxWidth",
        "--max-width", "640",
        "--max-width-target-unique-id", "one-guid"
    });
    expect(!max_width_property_result.ok,
        "#1152: launch contract should reject max-width-object combined with property commands");

    const auto stray_max_width_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-width", "640"
    });
    expect(!stray_max_width_result.ok,
        "#1152: launch contract should reject stray max-width arguments");
}

void test_parse_launch_arguments_for_max_left_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--max-left-object",
        "--max-left", "640",
        "--max-left-target-object-name", "frmCustomer",
        "--max-left-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1153: launch contract should parse max-left-object requests");
    expect(result.request.max_left_object,
        "#1153: launch contract should detect --max-left-object");
    expect(result.request.max_left_available && result.request.max_left == 640,
        "#1153: max-left-object requests should carry max left value");
    expect(result.request.max_left_objects.size() == 2U,
        "#1153: max-left-object requests should collect max-left target selectors");
    if (result.request.max_left_objects.size() == 2U) {
        expect(result.request.max_left_objects[0].object_name == "frmCustomer" &&
                result.request.max_left_objects[0].unique_id.empty(),
            "#1153: max-left-object requests should parse target object-name selectors");
        expect(result.request.max_left_objects[1].object_name.empty() &&
                result.request.max_left_objects[1].unique_id == "two-guid",
            "#1153: max-left-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_max_left_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-left-object",
        "--max-left-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1153: launch contract should reject max-left-object requests without max left value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-left-object",
        "--max-left", "640"
    });
    expect(!missing_targets_result.ok,
        "#1153: launch contract should reject max-left-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-left-object",
        "--max-left", "wide",
        "--max-left-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1153: launch contract should reject non-integer max-left values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-left-object",
        "--max-left", "-1",
        "--max-left-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1153: launch contract should reject negative max-left values");
}

void test_parse_launch_arguments_rejects_max_left_object_ambiguity() {
    const auto max_left_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-left-object",
        "--allow-output-object",
        "--max-left", "640",
        "--max-left-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!max_left_allow_output_result.ok,
        "#1153: launch contract should reject simultaneous max-left-object and allow-output-object requests");

    const auto max_left_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-left-object",
        "--clear-property",
        "--property-name", "MaxLeft",
        "--max-left", "640",
        "--max-left-target-unique-id", "one-guid"
    });
    expect(!max_left_property_result.ok,
        "#1153: launch contract should reject max-left-object combined with property commands");

    const auto stray_max_left_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-left", "640"
    });
    expect(!stray_max_left_result.ok,
        "#1153: launch contract should reject stray max-left arguments");
}

void test_parse_launch_arguments_for_max_top_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--max-top-object",
        "--max-top", "640",
        "--max-top-target-object-name", "frmCustomer",
        "--max-top-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1154: launch contract should parse max-top-object requests");
    expect(result.request.max_top_object,
        "#1154: launch contract should detect --max-top-object");
    expect(result.request.max_top_available && result.request.max_top == 640,
        "#1154: max-top-object requests should carry max top value");
    expect(result.request.max_top_objects.size() == 2U,
        "#1154: max-top-object requests should collect max-top target selectors");
    if (result.request.max_top_objects.size() == 2U) {
        expect(result.request.max_top_objects[0].object_name == "frmCustomer" &&
                result.request.max_top_objects[0].unique_id.empty(),
            "#1154: max-top-object requests should parse target object-name selectors");
        expect(result.request.max_top_objects[1].object_name.empty() &&
                result.request.max_top_objects[1].unique_id == "two-guid",
            "#1154: max-top-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_max_top_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-top-object",
        "--max-top-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1154: launch contract should reject max-top-object requests without max top value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-top-object",
        "--max-top", "640"
    });
    expect(!missing_targets_result.ok,
        "#1154: launch contract should reject max-top-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-top-object",
        "--max-top", "wide",
        "--max-top-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1154: launch contract should reject non-integer max-top values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-top-object",
        "--max-top", "-1",
        "--max-top-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1154: launch contract should reject negative max-top values");
}

void test_parse_launch_arguments_rejects_max_top_object_ambiguity() {
    const auto max_top_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-top-object",
        "--allow-output-object",
        "--max-top", "640",
        "--max-top-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!max_top_allow_output_result.ok,
        "#1154: launch contract should reject simultaneous max-top-object and allow-output-object requests");

    const auto max_top_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-top-object",
        "--clear-property",
        "--property-name", "MaxTop",
        "--max-top", "640",
        "--max-top-target-unique-id", "one-guid"
    });
    expect(!max_top_property_result.ok,
        "#1154: launch contract should reject max-top-object combined with property commands");

    const auto stray_max_top_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-top", "640"
    });
    expect(!stray_max_top_result.ok,
        "#1154: launch contract should reject stray max-top arguments");
}

void test_parse_launch_arguments_for_button_count_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--button-count-object",
        "--button-count", "3",
        "--button-count-target-object-name", "cmdSave",
        "--button-count-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1110: launch contract should parse button-count-object requests");
    expect(result.request.button_count_object,
        "#1110: launch contract should detect --button-count-object");
    expect(result.request.button_count_available && result.request.button_count == 3,
        "#1110: button-count-object requests should carry button-count value");
    expect(result.request.button_count_objects.size() == 2U,
        "#1110: button-count-object requests should collect button-count target selectors");
    if (result.request.button_count_objects.size() == 2U) {
        expect(result.request.button_count_objects[0].object_name == "cmdSave" &&
                result.request.button_count_objects[0].unique_id.empty(),
            "#1110: button-count-object requests should parse target object-name selectors");
        expect(result.request.button_count_objects[1].object_name.empty() &&
                result.request.button_count_objects[1].unique_id == "two-guid",
            "#1110: button-count-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_button_count_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--button-count-object",
        "--button-count-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1110: launch contract should reject button-count-object requests without button-count value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--button-count-object",
        "--button-count", "manual",
        "--button-count-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1110: launch contract should reject non-integer button-count values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--button-count-object",
        "--button-count", "-1",
        "--button-count-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1110: launch contract should reject negative button-count values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--button-count-object",
        "--button-count", "2"
    });
    expect(!missing_targets_result.ok,
        "#1110: launch contract should reject button-count-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_button_count_object_ambiguity() {
    const auto button_count_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--button-count-object",
        "--locked-object",
        "--button-count", "2",
        "--button-count-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!button_count_locked_result.ok,
        "#1110: launch contract should reject simultaneous button-count-object and locked-object requests");

    const auto button_count_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--button-count-object",
        "--clear-property",
        "--property-name", "ButtonCount",
        "--button-count", "2",
        "--button-count-target-unique-id", "one-guid"
    });
    expect(!button_count_property_result.ok,
        "#1110: launch contract should reject button-count-object combined with property commands");

    const auto stray_button_count_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--button-count", "2"
    });
    expect(!stray_button_count_result.ok,
        "#1110: launch contract should reject stray button-count arguments");
}

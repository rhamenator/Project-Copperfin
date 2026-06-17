#include "copperfin/studio/toolbox_creation.h"

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#define _getpid getpid
#endif
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::size_t object_count(const std::filesystem::path& table_path) {
    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    return list_result.ok ? list_result.objects.size() : 0U;
}

std::filesystem::path create_toolbox_fixture(const std::filesystem::path& temp_dir) {
    const std::filesystem::path table_path = temp_dir / "toolbox_create.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "CAPTION", .type = 'C', .length = 32U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "frmMain", "form-guid", "", "Form", "Form", "Main", ""},
        {"txt1", "txt1", "existing-textbox-guid", "frmMain", "TextBox", "TextBox", "Existing", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#1017: toolbox creation fixture should be writable");
    return table_path;
}

void test_toolbox_creation_maps_descriptors_and_defaults() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);

    const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Customer"},
            {.property_name = "PROPERTIES", .property_value = "ControlSource = \"customer.name\"\r\nLeft = 12\r\n"}
        }
    });

    expect(create_result.ok, "#1017: toolbox descriptor creates should succeed for known toolbox ids");
    expect(create_result.record_index == 2U,
        "#1017: toolbox descriptor creates should append the new object row");
    expect(create_result.object_name == "txt2" &&
            create_result.unique_id == "created-textbox-guid" &&
            create_result.parent_name == "frmMain",
        "#1017: toolbox descriptor creates should report generated identity metadata");

    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U,
        "#1017: toolbox descriptor creates should append exactly one object");
    if (list_result.ok && list_result.objects.size() == 3U) {
        const auto& created_object = list_result.objects[2];
        expect(created_object.object_name == "txt2",
            "#1017: created objects should carry generated object names");
        expect(created_object.unique_id == "created-textbox-guid",
            "#1017: created objects should carry unique ids");
        expect(created_object.parent_name == "frmMain",
            "#1017: created objects should carry parent names");
        expect(created_object.class_name == "TextBox",
            "#1017: created objects should carry descriptor class names");
        expect(created_object.baseclass_name == "TextBox",
            "#1017: created objects should carry descriptor baseclass names");
    }

    const auto caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "CAPTION"
    });
    expect(caption.ok && caption.exists && caption.value == "Customer",
        "#1017: toolbox descriptor creates should propagate caller-provided direct fields");

    const auto control_source = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(control_source.ok && control_source.exists && control_source.value == "\"customer.name\"",
        "#1017: toolbox descriptor creates should propagate caller-provided memo properties");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_respects_explicit_object_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_explicit_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);

    const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "commandbutton",
        .object_name = "cmdRun",
        .unique_id = "command-guid",
        .parent_name = "frmMain",
        .field_values = {}
    });

    expect(create_result.ok && create_result.object_name == "cmdRun",
        "#1017: explicit toolbox object names should take precedence over default name generation");

    const auto class_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "command-guid",
        .property_name = "CLASS"
    });
    expect(class_result.ok && class_result.exists && class_result.value == "CommandButton",
        "#1017: explicit-name creates should still map descriptor class metadata");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_rejects_unknown_toolbox_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_failure_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "missing-toolbox-item",
        .object_name = {},
        .unique_id = "should-not-exist",
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Should Not Exist"}
        }
    });

    expect(!create_result.ok,
        "#1017: unknown toolbox ids should fail cleanly");
    expect(create_result.object_name.empty() &&
            create_result.unique_id.empty() &&
            create_result.parent_name.empty(),
        "#1017: failed toolbox descriptor creates should not report stale identity metadata");
    expect(object_count(table_path) == before_count,
        "#1017: unknown toolbox ids should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace

int main() {
    test_toolbox_creation_maps_descriptors_and_defaults();
    test_toolbox_creation_respects_explicit_object_name();
    test_toolbox_creation_rejects_unknown_toolbox_without_mutation();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

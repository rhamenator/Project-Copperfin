#include "copperfin/runtime/xasset_methods.h"

#include <cstdlib>
#include <iostream>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

copperfin::vfp::DbfRecord make_record(
    std::size_t record_index,
    std::initializer_list<copperfin::vfp::DbfRecordValue> values) {
    copperfin::vfp::DbfRecord record;
    record.record_index = record_index;
    record.values.assign(values.begin(), values.end());
    return record;
}

void test_build_xasset_executable_model() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\demo.scx)";
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "Dataenvironment"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "dataenvironment"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE BeforeOpenTables\r\nSET DELETED ON\r\nENDPROC"}
        }),
        make_record(1, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "frmDemo"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "form"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Init\r\nSET DELETED OFF\r\nENDPROC"}
        }),
        make_record(2, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "pgfMain"},
            {.field_name = "PARENT", .field_type = 'M', .display_value = "frmDemo"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "pageframe"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Page2.Activate\r\nTHISFORM.Refresh\r\nENDPROC"}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "xAsset executable model should be created for forms");
    expect(model.runnable_startup, "form model should be runnable when startup methods exist");
    expect(model.root_object_path == "frmDemo", "root object path should identify the root form");
    expect(model.methods.size() == 3U, "all methods should be extracted");
    expect(model.startup_routines.size() == 2U, "startup should include data environment and form init methods");

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    expect(bootstrap.find("DO __cf_Dataenvironment_BeforeOpenTables") != std::string::npos, "bootstrap should call the data environment method");
    expect(bootstrap.find("DO __cf_frmDemo_Init") != std::string::npos, "bootstrap should call the form init method");
    expect(bootstrap.find("PROCEDURE __cf_frmDemo_pgfMain_Page2_Activate") != std::string::npos, "bootstrap should materialize nested object methods");
    expect(bootstrap.find("READ EVENTS") != std::string::npos, "bootstrap should optionally include READ EVENTS");
}

}  // namespace

int main() {
    test_build_xasset_executable_model();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

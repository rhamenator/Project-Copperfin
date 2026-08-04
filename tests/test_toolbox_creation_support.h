#pragma once

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/studio/toolbox_creation.h"

#include "copperfin/localization/localization.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_environment_support.h"

#include <algorithm>
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
#include <string_view>
#include <vector>


namespace copperfin::toolbox_creation_tests
{
inline int failures = 0;

inline void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

inline std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
}

inline std::size_t object_count(const std::filesystem::path& table_path) {
    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    return list_result.ok ? list_result.objects.size() : 0U;
}

inline bool has_field_value(
    const std::vector<copperfin::vfp::VisualObjectPropertyChange>& changes,
    const std::string& property_name,
    const std::string& property_value) {
    for (const auto& change : changes) {
        if (change.property_name == property_name && change.property_value == property_value) {
            return true;
        }
    }
    return false;
}

inline bool has_argument_pair(const std::vector<std::string>& arguments, const std::string& key, const std::string& value) {
    for (std::size_t index = 0U; (index + 1U) < arguments.size(); ++index) {
        if (arguments[index] == key && arguments[index + 1U] == value) {
            return true;
        }
    }
    return false;
}

inline bool has_argument(const std::vector<std::string>& arguments, const std::string& value) {
    return std::find(arguments.begin(), arguments.end(), value) != arguments.end();
}

inline const copperfin::studio::StudioToolboxObjectCreatePlanCatalogEntry* find_create_plan_entry(
    const std::vector<copperfin::studio::StudioToolboxObjectCreatePlanCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.toolbox_item.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

inline const copperfin::studio::StudioToolboxObjectCreateDispatchCatalogEntry* find_create_dispatch_entry(
    const std::vector<copperfin::studio::StudioToolboxObjectCreateDispatchCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.toolbox_item.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

inline const copperfin::studio::StudioToolboxObjectCreatePlan* find_create_batch_plan(
    const std::vector<copperfin::studio::StudioToolboxObjectCreatePlan>& plans,
    std::string_view id) {
    for (const auto& plan : plans) {
        if (plan.toolbox_item.id == id) {
            return &plan;
        }
    }
    return nullptr;
}

inline std::filesystem::path create_toolbox_fixture(const std::filesystem::path& temp_dir) {
    const std::filesystem::path table_path = temp_dir / "toolbox_create.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 64U},
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

}

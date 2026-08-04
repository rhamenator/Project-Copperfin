// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
std::size_t count_missing_locale_keys(
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

std::string build_nested_do_chain_script(std::size_t nested_routine_count) {
    std::ostringstream script;
    if (nested_routine_count == 0U) {
        script << "RETURN\n";
        return script.str();
    }

    script << "DO p1\n";
    script << "RETURN\n";
    for (std::size_t index = 1; index <= nested_routine_count; ++index) {
        script << "PROCEDURE p" << index << "\n";
        if (index < nested_routine_count) {
            script << "DO p" << (index + 1U) << "\n";
        }
        script << "RETURN\n";
    }

    return script.str();
}

}  // namespace cf_test_prg_engine_control_flow

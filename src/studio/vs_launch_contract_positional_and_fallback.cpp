// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_positional_and_fallback(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    [[maybe_unused]] const std::vector<std::string>& args,
    [[maybe_unused]] std::size_t& index,
    LaunchParseResult& result,
    [[maybe_unused]] std::string& parsed_argument_error) {
if (!argument.empty() && argument[0] == '-') {
            result = {.ok = false, .error = localized_unknown_argument(catalog, argument)}; return {true, true};
        }

if (result.request.path.empty()) {
            result.request.path = argument;
            return {true, false};
        }
    return {false, false};
}

}  // namespace copperfin::studio

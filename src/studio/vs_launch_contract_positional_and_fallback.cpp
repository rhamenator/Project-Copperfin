#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_positional_and_fallback(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error) {
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

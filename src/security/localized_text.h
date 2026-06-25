#pragma once

#include "copperfin/localization/localization.h"

#include <string>
#include <string_view>

namespace copperfin::security {

std::string security_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {});

}  // namespace copperfin::security

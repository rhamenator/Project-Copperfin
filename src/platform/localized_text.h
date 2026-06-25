#pragma once

#include "copperfin/localization/localization.h"

#include <string>
#include <string_view>

namespace copperfin::platform {

std::string platform_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {});

}  // namespace copperfin::platform

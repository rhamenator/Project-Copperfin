#pragma once

#include <string>

namespace copperfin::security {

struct ProcessHardeningStatus {
    bool applied = false;
    std::string message;
};

[[nodiscard]] ProcessHardeningStatus apply_default_process_hardening();

}  // namespace copperfin::security

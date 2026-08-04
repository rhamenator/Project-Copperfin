// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace copperfin::runtime {

class PrgCompatibilityError final : public std::runtime_error
{
public:
    PrgCompatibilityError(std::string message, int error_code)
        : std::runtime_error(std::move(message)),
          error_code_(error_code)
    {
    }

    [[nodiscard]] int error_code() const noexcept
    {
        return error_code_;
    }

private:
    int error_code_;
};

}  // namespace copperfin::runtime

// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace copperfin::platform {

inline constexpr std::string_view mcp_current_protocol_version = "2026-07-28";
inline constexpr std::string_view mcp_dbf_header_tool_name =
    "copperfin.parse_dbf_header";
inline constexpr std::size_t mcp_max_message_bytes = 64U * 1024U;

struct McpMessageResult final {
    bool has_response = false;
    std::string response;
};

class McpStdioSession final {
public:
    [[nodiscard]] McpMessageResult handle_message(std::string_view message);

private:
    bool legacy_initialized_ = false;
    bool legacy_ready_ = false;
    std::string legacy_protocol_version_;
};

}  // namespace copperfin::platform

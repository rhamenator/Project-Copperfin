// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/environment.h"
#include "copperfin/platform/json.h"
#include "copperfin/platform/mcp_host.h"
#include "copperfin/security/authorization.h"
#include "copperfin/security/process_hardening.h"
#include "copperfin/security/security_model.h"

#include <iostream>
#include <string>

namespace {

enum class BoundedLineStatus {
    line,
    too_large,
    end
};

bool mcp_role_is_authorized(std::string& role) {
    role = copperfin::platform::read_environment_variable_or_empty(
        "COPPERFIN_SECURITY_ROLE");
    if (role.empty()) {
        role = "developer";
    }
    return copperfin::security::role_has_permission(
        copperfin::security::default_native_security_profile(),
        role,
        "ai.mcp");
}

void audit_tool_call(
    const std::string_view request,
    const std::string_view response) {
    const auto method = copperfin::platform::select_json_value(request, "/method");
    if (!method.ok() || method.kind != copperfin::platform::JsonValueKind::string ||
        method.decoded_string != "tools/call") {
        return;
    }
    const auto name = copperfin::platform::select_json_value(request, "/params/name");
    const bool known_tool = name.ok() &&
        name.kind == copperfin::platform::JsonValueKind::string &&
        name.decoded_string == copperfin::platform::mcp_dbf_header_tool_name;
    const auto is_error = copperfin::platform::select_json_value(
        response, "/result/isError");
    const bool succeeded = is_error.ok() &&
        is_error.kind == copperfin::platform::JsonValueKind::boolean &&
        is_error.raw_json == "false";
    std::cerr << "copperfin.audit event=ai.mcp_invoked tool="
              << (known_tool ? "copperfin.parse_dbf_header" : "unknown")
              << " outcome=" << (succeeded ? "success" : "rejected") << '\n';
}

BoundedLineStatus read_bounded_line(std::istream& input, std::string& line) {
    line.clear();
    bool overflow = false;
    while (true) {
        const int next = input.get();
        if (next == std::char_traits<char>::eof()) {
            return line.empty() && !overflow ? BoundedLineStatus::end
                                              : (overflow ? BoundedLineStatus::too_large
                                                          : BoundedLineStatus::line);
        }
        if (next == '\n') {
            return overflow ? BoundedLineStatus::too_large : BoundedLineStatus::line;
        }
        if (next == '\r') {
            continue;
        }
        if (line.size() < copperfin::platform::mcp_max_message_bytes) {
            line.push_back(static_cast<char>(next));
        } else {
            overflow = true;
        }
    }
}

}  // namespace

int main() {
    const auto hardening = copperfin::security::apply_default_process_hardening();
    if (!hardening.applied && !hardening.message.empty()) {
        std::cerr << hardening.message << '\n';
    }
    std::string security_role;
    if (!mcp_role_is_authorized(security_role)) {
        std::cerr << "mcp.security.permission_denied permission=ai.mcp\n";
        return 7;
    }

    copperfin::platform::McpStdioSession session;
    std::string line;
    while (true) {
        const BoundedLineStatus status = read_bounded_line(std::cin, line);
        if (status == BoundedLineStatus::end) {
            return 0;
        }
        const auto result = status == BoundedLineStatus::too_large
            ? session.handle_message(std::string(
                  copperfin::platform::mcp_max_message_bytes + 1U, ' '))
            : session.handle_message(line);
        if (result.has_response) {
            std::cout << result.response << '\n';
            std::cout.flush();
            audit_tool_call(line, result.response);
        }
    }
}

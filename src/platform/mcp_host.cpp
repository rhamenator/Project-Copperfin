// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/mcp_host.h"

#include "copperfin/platform/json.h"
#include "copperfin/vfp/dbf_header.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <locale>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace copperfin::platform {

namespace {

#ifndef COPPERFIN_MCP_HOST_VERSION
#error "COPPERFIN_MCP_HOST_VERSION must be supplied by the build"
#endif

constexpr std::string_view server_name = "copperfin-mcp-host";
constexpr std::string_view server_version = COPPERFIN_MCP_HOST_VERSION;
constexpr std::array<std::string_view, 3U> supported_versions{
    mcp_current_protocol_version,
    "2025-11-25",
    "2025-06-18"
};
constexpr JsonDocumentLimits message_limits{
    .max_document_bytes = mcp_max_message_bytes,
    .max_nesting_depth = 32U,
    .max_value_count = 2048U
};

std::string server_info_json() {
    return "{\"name\":\"" + std::string(server_name) +
        "\",\"version\":\"" + std::string(server_version) + "\"}";
}

std::string result_meta_json() {
    return "{\"io.modelcontextprotocol/serverInfo\":" + server_info_json() + "}";
}

std::string response_with_result(
    const std::string_view id,
    const std::string_view result) {
    return "{\"jsonrpc\":\"2.0\",\"id\":" + std::string(id) +
        ",\"result\":" + std::string(result) + "}";
}

std::string response_with_error(
    const std::string_view id,
    const int code,
    const std::string_view message,
    const std::string_view data = {}) {
    std::string response = "{\"jsonrpc\":\"2.0\",\"id\":" +
        std::string(id.empty() ? "null" : id) +
        ",\"error\":{\"code\":" + std::to_string(code) +
        ",\"message\":\"" + json_escape_string(message) + "\"";
    if (!data.empty()) {
        response += ",\"data\":" + std::string(data);
    }
    response += "}}";
    return response;
}

std::optional<JsonSelectionResult> select_optional(
    const std::string_view document,
    const std::string_view pointer) {
    JsonSelectionResult selection = select_json_value(document, pointer, message_limits);
    if (selection.error == JsonSelectionError::value_not_found) {
        return std::nullopt;
    }
    return selection;
}

bool has_exact_members(
    const std::string_view document,
    const std::string_view pointer,
    const std::set<std::string>& required,
    const std::set<std::string>& optional = {}) {
    const JsonObjectMembersResult members =
        select_json_object_member_names(document, pointer, message_limits);
    if (!members.ok()) {
        return false;
    }
    std::set<std::string> actual(members.names.begin(), members.names.end());
    for (const auto& name : required) {
        if (!actual.erase(name)) {
            return false;
        }
    }
    for (const auto& name : optional) {
        actual.erase(name);
    }
    return actual.empty();
}

std::string supported_versions_json() {
    std::string json = "[";
    for (std::size_t index = 0U; index < supported_versions.size(); ++index) {
        if (index != 0U) {
            json += ',';
        }
        json += "\"" + std::string(supported_versions[index]) + "\"";
    }
    json += ']';
    return json;
}

bool is_supported_legacy_version(const std::string_view value) {
    return value == supported_versions[1] || value == supported_versions[2];
}

std::string unsupported_version_data(const std::string_view requested) {
    return "{\"supported\":" + supported_versions_json() +
        ",\"requested\":\"" + json_escape_string(requested) + "\"}";
}

std::string tool_definition_json() {
    return "{\"name\":\"" + std::string(mcp_dbf_header_tool_name) +
        "\",\"title\":\"Parse a DBF header\""
        ",\"description\":\"Decode one caller-supplied 32-byte DBF header without reading files or using a network.\""
        ",\"inputSchema\":{\"type\":\"object\",\"properties\":{"
        "\"headerHex\":{\"type\":\"string\",\"pattern\":\"^[0-9A-Fa-f]{64}$\","
        "\"description\":\"Exactly 32 DBF header bytes encoded as 64 hexadecimal characters.\"}},"
        "\"required\":[\"headerHex\"],\"additionalProperties\":false}"
        ",\"annotations\":{\"readOnlyHint\":true,\"destructiveHint\":false,"
        "\"idempotentHint\":true,\"openWorldHint\":false}}";
}

std::string discover_result() {
    return "{\"resultType\":\"complete\",\"supportedVersions\":" +
        supported_versions_json() +
        ",\"capabilities\":{\"tools\":{\"listChanged\":false}},"
        "\"_meta\":" + result_meta_json() +
        ",\"ttlMs\":3600000,\"cacheScope\":\"public\"}";
}

std::string tools_list_result(const bool modern) {
    std::string result = "{";
    if (modern) {
        result += "\"resultType\":\"complete\",";
    }
    result += "\"tools\":[" + tool_definition_json() + "]";
    if (modern) {
        result += ",\"ttlMs\":3600000,\"cacheScope\":\"public\",\"_meta\":" +
            result_meta_json();
    }
    result += '}';
    return result;
}

int hex_digit(const char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    if (value >= 'A' && value <= 'F') {
        return value - 'A' + 10;
    }
    return -1;
}

std::optional<std::vector<std::uint8_t>> decode_header_hex(
    const std::string_view value) {
    if (value.size() != 64U) {
        return std::nullopt;
    }
    std::vector<std::uint8_t> bytes(32U, 0U);
    for (std::size_t index = 0U; index < bytes.size(); ++index) {
        const int high = hex_digit(value[index * 2U]);
        const int low = hex_digit(value[index * 2U + 1U]);
        if (high < 0 || low < 0) {
            return std::nullopt;
        }
        bytes[index] = static_cast<std::uint8_t>((high << 4U) | low);
    }
    return bytes;
}

std::string tool_payload_error(const std::string_view error_code) {
    return "{\"schemaVersion\":1,\"errorCode\":\"" +
        json_escape_string(error_code) + "\"}";
}

std::string tool_result(
    const std::string_view payload,
    const bool is_error,
    const bool modern) {
    std::string result = "{";
    if (modern) {
        result += "\"resultType\":\"complete\",";
    }
    result += "\"content\":[{\"type\":\"text\",\"text\":\"" +
        json_escape_string(payload) + "\"}],\"structuredContent\":" +
        std::string(payload) + ",\"isError\":" + (is_error ? "true" : "false");
    if (modern) {
        result += ",\"_meta\":" + result_meta_json();
    }
    result += '}';
    return result;
}

std::string dbf_header_payload(const copperfin::vfp::DbfHeader& header) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << "{\"schemaVersion\":1"
           << ",\"version\":" << static_cast<unsigned int>(header.version)
           << ",\"lastUpdate\":\"" << json_escape_string(header.last_update_iso8601()) << "\""
           << ",\"recordCount\":" << header.record_count
           << ",\"headerLength\":" << header.header_length
           << ",\"recordLength\":" << header.record_length
           << ",\"tableFlags\":" << static_cast<unsigned int>(header.table_flags)
           << ",\"codePageMark\":" << static_cast<unsigned int>(header.code_page_mark)
           << ",\"codePage\":";
    const auto code_page = copperfin::vfp::dbf_code_page_from_mark(header.code_page_mark);
    if (code_page.has_value()) {
        stream << *code_page;
    } else {
        stream << "null";
    }
    stream << ",\"hasMemoFile\":" << (header.has_memo_file() ? "true" : "false")
           << ",\"hasProductionIndex\":" << (header.has_production_index() ? "true" : "false")
           << ",\"hasStructuralCdx\":" << (header.has_structural_cdx() ? "true" : "false")
           << ",\"hasDatabaseContainer\":" << (header.has_database_container() ? "true" : "false")
           << '}';
    return stream.str();
}

std::string call_tool_result(
    const std::string_view document,
    const bool modern) {
    const auto name = select_optional(document, "/params/name");
    const auto arguments = select_optional(document, "/params/arguments");
    if (!name.has_value() || !name->ok() || name->kind != JsonValueKind::string ||
        name->decoded_string != mcp_dbf_header_tool_name) {
        return {};
    }
    if (!arguments.has_value() || !arguments->ok() ||
        arguments->kind != JsonValueKind::object ||
        !has_exact_members(document, "/params/arguments", {"headerHex"})) {
        return tool_result(tool_payload_error("mcp.dbf_header.invalid_arguments"), true, modern);
    }
    const auto header_hex = select_optional(document, "/params/arguments/headerHex");
    if (!header_hex.has_value() || !header_hex->ok() ||
        header_hex->kind != JsonValueKind::string) {
        return tool_result(tool_payload_error("mcp.dbf_header.invalid_arguments"), true, modern);
    }
    const auto bytes = decode_header_hex(header_hex->decoded_string);
    if (!bytes.has_value()) {
        return tool_result(tool_payload_error("mcp.dbf_header.invalid_hex"), true, modern);
    }
    const auto parsed = copperfin::vfp::parse_dbf_header(*bytes);
    if (!parsed.ok) {
        return tool_result(tool_payload_error("mcp.dbf_header.invalid_values"), true, modern);
    }
    return tool_result(dbf_header_payload(parsed.header), false, modern);
}

}  // namespace

McpMessageResult McpStdioSession::handle_message(const std::string_view message) {
    if (message.empty() || message.size() > mcp_max_message_bytes) {
        return {true, response_with_error("null", -32700, "Parse error")};
    }
    const JsonSelectionResult root = select_json_value(message, {}, message_limits);
    if (!root.ok()) {
        return {true, response_with_error("null", -32700, "Parse error")};
    }
    if (root.kind != JsonValueKind::object) {
        return {true, response_with_error("null", -32600, "Invalid Request")};
    }

    const auto jsonrpc = select_optional(message, "/jsonrpc");
    const auto method = select_optional(message, "/method");
    const auto id = select_optional(message, "/id");
    const auto params = select_optional(message, "/params");
    const bool notification = !id.has_value();
    if (!jsonrpc.has_value() || !jsonrpc->ok() ||
        jsonrpc->kind != JsonValueKind::string || jsonrpc->decoded_string != "2.0" ||
        !method.has_value() || !method->ok() || method->kind != JsonValueKind::string ||
        (id.has_value() && (!id->ok() ||
            (id->kind != JsonValueKind::string && id->kind != JsonValueKind::number))) ||
        (params.has_value() && (!params->ok() || params->kind != JsonValueKind::object))) {
        return notification
            ? McpMessageResult{}
            : McpMessageResult{true, response_with_error(
                  id.has_value() && id->ok() ? id->raw_json : "null",
                  -32600,
                  "Invalid Request")};
    }
    const std::string response_id = id.has_value() ? id->raw_json : "null";

    if (method->decoded_string == "initialize") {
        legacy_initialized_ = false;
        legacy_ready_ = false;
        legacy_protocol_version_.clear();
        if (notification || !params.has_value()) {
            return notification ? McpMessageResult{} : McpMessageResult{
                true, response_with_error(response_id, -32602, "Invalid params")};
        }
        const auto version = select_optional(message, "/params/protocolVersion");
        const auto capabilities = select_optional(message, "/params/capabilities");
        const auto client_info = select_optional(message, "/params/clientInfo");
        const auto client_name = select_optional(message, "/params/clientInfo/name");
        const auto client_version = select_optional(message, "/params/clientInfo/version");
        if (!version.has_value() || !version->ok() || version->kind != JsonValueKind::string ||
            !capabilities.has_value() || !capabilities->ok() || capabilities->kind != JsonValueKind::object ||
            !client_info.has_value() || !client_info->ok() || client_info->kind != JsonValueKind::object ||
            !client_name.has_value() || !client_name->ok() || client_name->kind != JsonValueKind::string ||
            !client_version.has_value() || !client_version->ok() ||
            client_version->kind != JsonValueKind::string) {
            return {true, response_with_error(response_id, -32602, "Invalid params")};
        }
        if (version->decoded_string == mcp_current_protocol_version) {
            return {true, response_with_error(response_id, -32601, "Method not found")};
        }
        legacy_initialized_ = true;
        legacy_ready_ = false;
        legacy_protocol_version_ = is_supported_legacy_version(version->decoded_string)
            ? version->decoded_string
            : std::string(supported_versions[1]);
        const std::string result = "{\"protocolVersion\":\"" +
            json_escape_string(legacy_protocol_version_) +
            "\",\"capabilities\":{\"tools\":{\"listChanged\":false}},"
            "\"serverInfo\":" + server_info_json() + "}";
        return {true, response_with_result(response_id, result)};
    }

    if (method->decoded_string == "notifications/initialized") {
        if (notification && legacy_initialized_) {
            legacy_ready_ = true;
        }
        return {};
    }
    if (notification) {
        return {};
    }
    bool modern = false;
    const auto protocol_version =
        select_optional(message, "/params/_meta/io.modelcontextprotocol~1protocolVersion");
    const auto client_capabilities =
        select_optional(message, "/params/_meta/io.modelcontextprotocol~1clientCapabilities");
    if (protocol_version.has_value()) {
        if (!protocol_version->ok() || protocol_version->kind != JsonValueKind::string ||
            protocol_version->decoded_string != mcp_current_protocol_version) {
            const std::string requested = protocol_version->ok() &&
                    protocol_version->kind == JsonValueKind::string
                ? protocol_version->decoded_string
                : "";
            return {true, response_with_error(
                response_id,
                -32022,
                "Unsupported protocol version",
                unsupported_version_data(requested))};
        }
        if (!client_capabilities.has_value() || !client_capabilities->ok() ||
            client_capabilities->kind != JsonValueKind::object) {
            return {true, response_with_error(response_id, -32602, "Invalid params")};
        }
        modern = true;
    } else if (method->decoded_string == "server/discover" ||
               client_capabilities.has_value()) {
        return {true, response_with_error(response_id, -32602, "Invalid params")};
    } else if (!legacy_ready_ &&
               !(legacy_initialized_ && method->decoded_string == "ping")) {
        return {true, response_with_error(response_id, -32002, "Server not initialized")};
    }

    if (method->decoded_string == "server/discover") {
        if (!modern) {
            return {true, response_with_error(response_id, -32601, "Method not found")};
        }
        return {true, response_with_result(response_id, discover_result())};
    }
    if (method->decoded_string == "ping") {
        return {true, response_with_result(
            response_id,
            modern
                ? "{\"resultType\":\"complete\",\"_meta\":" + result_meta_json() + "}"
                : "{}")};
    }
    if (method->decoded_string == "tools/list") {
        const auto cursor = select_optional(message, "/params/cursor");
        if (cursor.has_value()) {
            return {true, response_with_error(response_id, -32602, "Invalid params")};
        }
        return {true, response_with_result(response_id, tools_list_result(modern))};
    }
    if (method->decoded_string == "tools/call") {
        if (!params.has_value()) {
            return {true, response_with_error(response_id, -32602, "Invalid params")};
        }
        const std::string result = call_tool_result(message, modern);
        if (result.empty()) {
            return {true, response_with_error(response_id, -32602, "Unknown tool")};
        }
        return {true, response_with_result(response_id, result)};
    }
    return {true, response_with_error(response_id, -32601, "Method not found")};
}

}  // namespace copperfin::platform

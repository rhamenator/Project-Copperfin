// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/json.h"
#include "copperfin/platform/mcp_host.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace {

void expect(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

std::string modern_request(
    const std::string_view id,
    const std::string_view method,
    const std::string_view extra_params = {}) {
    std::string params =
        "{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\","
        "\"io.modelcontextprotocol/clientCapabilities\":{}}";
    if (!extra_params.empty()) {
        params += ',';
        params += extra_params;
    }
    params += '}';
    return "{\"jsonrpc\":\"2.0\",\"id\":" + std::string(id) +
        ",\"method\":\"" + std::string(method) + "\",\"params\":" + params + '}';
}

void expect_json_value(
    const std::string& document,
    const std::string_view pointer,
    const std::string_view expected) {
    const auto selected = copperfin::platform::select_json_value(document, pointer);
    expect(selected.ok() && selected.raw_json == expected, pointer);
}

}  // namespace

int main() {
    using copperfin::platform::McpStdioSession;

    McpStdioSession modern;
    auto response = modern.handle_message("{");
    expect(response.has_response, "malformed JSON receives a response");
    expect_json_value(response.response, "/error/code", "-32700");

    response = modern.handle_message(std::string(
        copperfin::platform::mcp_max_message_bytes + 1U, ' '));
    expect_json_value(response.response, "/error/code", "-32700");

    response = modern.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":0,\"method\":\"tools/list\",\"params\":{}}" );
    expect_json_value(response.response, "/error/code", "-32002");

    response = modern.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":01,\"method\":\"server/discover\",\"params\":{}}" );
    expect_json_value(response.response, "/error/code", "-32700");

    response = modern.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":9,\"method\":\"server/discover\",\"params\":{}}" );
    expect_json_value(response.response, "/error/code", "-32602");

    response = modern.handle_message(modern_request("1", "server/discover"));
    expect_json_value(response.response, "/result/resultType", "\"complete\"");
    expect_json_value(response.response, "/result/supportedVersions/0", "\"2026-07-28\"");

    response = modern.handle_message(modern_request("\"list\"", "tools/list"));
    expect_json_value(response.response, "/id", "\"list\"");
    expect_json_value(response.response, "/result/tools/0/name", "\"copperfin.parse_dbf_header\"");
    expect_json_value(response.response, "/result/tools/0/annotations/readOnlyHint", "true");

    const std::string valid_header =
        "037e010201000000200001000000000000000000000000000000000001030000";
    response = modern.handle_message(modern_request(
        "3", "tools/call",
        "\"name\":\"copperfin.parse_dbf_header\",\"arguments\":{\"headerHex\":\"" +
            valid_header + "\"}"));
    expect_json_value(response.response, "/result/isError", "false");
    expect_json_value(response.response, "/result/structuredContent/recordCount", "1");
    expect_json_value(response.response, "/result/structuredContent/headerLength", "32");
    expect_json_value(response.response, "/result/structuredContent/codePage", "1252");

    response = modern.handle_message(modern_request(
        "4", "tools/call",
        "\"name\":\"copperfin.parse_dbf_header\",\"arguments\":{\"headerHex\":\"zz\"}"));
    expect_json_value(response.response, "/result/isError", "true");
    expect_json_value(response.response, "/result/structuredContent/errorCode",
        "\"mcp.dbf_header.invalid_hex\"");

    response = modern.handle_message(modern_request(
        "5", "tools/call",
        "\"name\":\"copperfin.parse_dbf_header\",\"arguments\":{\"headerHex\":\"" +
            valid_header + "\",\"extra\":true}"));
    expect_json_value(response.response, "/result/structuredContent/errorCode",
        "\"mcp.dbf_header.invalid_arguments\"");

    response = modern.handle_message(modern_request(
        "51", "tools/call",
        "\"name\":\"copperfin.unknown\",\"arguments\":{}"));
    expect_json_value(response.response, "/error/code", "-32602");

    response = modern.handle_message(modern_request("52", "unknown/method"));
    expect_json_value(response.response, "/error/code", "-32601");

    response = modern.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":6,\"method\":\"ping\",\"params\":{"
        "\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2099-01-01\","
        "\"io.modelcontextprotocol/clientCapabilities\":{}}}}" );
    expect_json_value(response.response, "/error/code", "-32022");

    response = modern.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":7,\"method\":\"initialize\",\"params\":{"
        "\"protocolVersion\":\"2026-07-28\",\"capabilities\":{},"
        "\"clientInfo\":{\"name\":\"test\",\"version\":\"1\"}}}" );
    expect_json_value(response.response, "/error/code", "-32601");

    response = modern.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":8,\"method\":\"initialize\",\"params\":{}}" );
    expect_json_value(response.response, "/error/code", "-32602");

    response = modern.handle_message(
        "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\",\"params\":{}}" );
    expect(!response.has_response, "notifications never receive responses");

    McpStdioSession legacy;
    response = legacy.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{"
        "\"protocolVersion\":\"2025-11-25\",\"capabilities\":{},"
        "\"clientInfo\":{\"name\":\"test\",\"version\":\"1\"}}}" );
    expect_json_value(response.response, "/result/protocolVersion", "\"2025-11-25\"");
    response = legacy.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":11,\"method\":\"ping\"}" );
    expect_json_value(response.response, "/result", "{}");
    response = legacy.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/list\",\"params\":{}}" );
    expect_json_value(response.response, "/error/code", "-32002");
    response = legacy.handle_message(
        "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\",\"params\":{}}" );
    expect(!response.has_response, "legacy initialized notification has no response");
    response = legacy.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/list\"}" );
    expect_json_value(response.response, "/result/tools/0/name", "\"copperfin.parse_dbf_header\"");
    expect(copperfin::platform::select_json_value(
        response.response, "/result/resultType").error ==
            copperfin::platform::JsonSelectionError::value_not_found,
        "legacy result omits current-only resultType");

    response = legacy.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":4,\"method\":\"tools/list\",\"method\":\"ping\",\"params\":{}}" );
    expect_json_value(response.response, "/error/code", "-32700");

    McpStdioSession older_legacy;
    response = older_legacy.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{"
        "\"protocolVersion\":\"2025-06-18\",\"capabilities\":{},"
        "\"clientInfo\":{\"name\":\"test\",\"version\":\"1\"}}}" );
    expect_json_value(response.response, "/result/protocolVersion", "\"2025-06-18\"");

    McpStdioSession negotiated_legacy;
    response = negotiated_legacy.handle_message(
        "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{"
        "\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},"
        "\"clientInfo\":{\"name\":\"test\",\"version\":\"1\"}}}" );
    expect_json_value(response.response, "/result/protocolVersion", "\"2025-11-25\"");

    std::cout << "MCP host tests passed\n";
    return 0;
}

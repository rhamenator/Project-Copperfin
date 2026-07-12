// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/security/audit_stream.h"

#include "copperfin/security/sha256.h"
#include "localized_text.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace copperfin::security {

namespace {

std::string now_utc_compact() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    return std::to_string(millis);
}

std::string escape_field(std::string value) {
    for (char& ch : value) {
        if (ch == '|' || ch == '\n' || ch == '\r') {
            ch = ' ';
        }
    }
    return value;
}

struct AuditTailReadResult {
    bool ok = false;
    std::string hash;
    std::string error;
};

std::vector<std::string> split_audit_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    while (std::getline(stream, token, '|')) {
        tokens.push_back(token);
    }
    return tokens;
}

bool is_sha256_hex(const std::string& value) {
    return value.size() == 64U &&
        std::all_of(value.begin(), value.end(), [](const unsigned char ch) {
            return std::isxdigit(ch) != 0;
        });
}

AuditTailReadResult read_last_hash(const std::string& log_path) {
    std::ifstream input(log_path, std::ios::binary);
    if (!input) {
        std::error_code status_error;
        const bool exists = std::filesystem::exists(log_path, status_error);
        if (!exists && !status_error) {
            return {
                .ok = true,
                .hash = "GENESIS",
                .error = {}};
        }
        return {
            .ok = false,
            .hash = {},
            .error = security_text("Security.Audit.Error.ReadExistingLogFailed")};
    }

    input.seekg(0, std::ios::end);
    const std::streampos end = input.tellg();
    if (end < 0) {
        return {
            .ok = false,
            .hash = {},
            .error = security_text("Security.Audit.Error.ReadExistingLogFailed")};
    }
    if (end == 0) {
        return {
            .ok = true,
            .hash = "GENESIS",
            .error = {}};
    }

    input.seekg(-1, std::ios::end);
    char final_byte = '\0';
    input.get(final_byte);
    if (!input.good()) {
        return {
            .ok = false,
            .hash = {},
            .error = security_text("Security.Audit.Error.ReadExistingLogFailed")};
    }
    input.clear();
    input.seekg(0, std::ios::beg);
    std::string line;
    std::string last_line;
    while (std::getline(input, line)) {
        if (!line.empty()) {
            last_line = line;
        }
    }
    if (input.bad()) {
        return {
            .ok = false,
            .hash = {},
            .error = security_text("Security.Audit.Error.ReadExistingLogFailed")};
    }

    if (last_line.empty()) {
        return {
            .ok = true,
            .hash = "GENESIS",
            .error = {}};
    }
    if (final_byte != '\n') {
        return {
            .ok = false,
            .hash = {},
            .error = security_text("Security.Audit.Error.InvalidExistingLogTail")};
    }

    const auto fields = split_audit_line(last_line);
    if (fields.size() != 5U ||
        fields[0].empty() ||
        (fields[3] != "GENESIS" && !is_sha256_hex(fields[3])) ||
        !is_sha256_hex(fields[4])) {
        return {
            .ok = false,
            .hash = {},
            .error = security_text("Security.Audit.Error.InvalidExistingLogTail")};
    }

    return {
        .ok = true,
        .hash = fields[4],
        .error = {}};
}

std::string compute_entry_hash(const std::string& timestamp,
                              const std::string& event_name,
                              const std::string& detail,
                              const std::string& previous_hash) {
    const std::string payload = timestamp + "|" + event_name + "|" + detail + "|" + previous_hash;
    const auto hash = sha256_hex_for_text(payload);
    return hash.ok ? hash.hex_digest : std::string{};
}

}  // namespace

AuditAppendResult append_immutable_audit_event(
    const std::string& log_path,
    const std::string& event_name,
    const std::string& detail) {
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(log_path).parent_path(), error);
    if (error) {
        return {.ok = false, .error = security_text("Security.Audit.Error.CreateLogDirectoryFailed"), .entry_hash = {}};
    }

    const auto tail = read_last_hash(log_path);
    if (!tail.ok) {
        return {.ok = false, .error = tail.error, .entry_hash = {}};
    }
    const std::string& previous_hash = tail.hash;
    const std::string timestamp = now_utc_compact();
    const std::string safe_event = escape_field(event_name);
    const std::string safe_detail = escape_field(detail);

    const std::string signed_payload = timestamp + "|" + safe_event + "|" + safe_detail + "|" + previous_hash;
    const auto hash = sha256_hex_for_text(signed_payload);
    if (!hash.ok) {
        return {.ok = false, .error = hash.error, .entry_hash = {}};
    }

    std::ofstream output(log_path, std::ios::app | std::ios::binary);
    if (!output) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }

    output << signed_payload << "|" << hash.hex_digest << "\n";
    if (!output.good()) {
        return {.ok = false, .error = security_text("Security.Audit.Error.AppendLogEntryFailed"), .entry_hash = {}};
    }

    return {.ok = true, .error = {}, .entry_hash = hash.hex_digest};
}

AuditChainVerifyResult verify_immutable_audit_chain(const std::string& log_path) {
    std::ifstream input(log_path, std::ios::binary);
    if (!input) {
        return {.ok = true, .error = {}, .entries = 0U};
    }

    std::string line;
    std::size_t line_number = 0U;
    std::string previous_hash = "GENESIS";
    std::size_t verified = 0U;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        ++line_number;

        const auto fields = split_audit_line(line);
        if (fields.size() != 5U) {
            return {.ok = false,
                    .error = security_text(
                        "Security.Audit.Error.MalformedLine",
                        {{"lineNumber", std::to_string(line_number)}}),
                    .entries = verified};
        }

        const auto& timestamp = fields[0];
        const auto& event_name = fields[1];
        const auto& detail = fields[2];
        const auto& expected_previous_hash = fields[3];
        const auto& observed_hash = fields[4];
        if (expected_previous_hash != previous_hash) {
            return {.ok = false,
                    .error = security_text(
                        "Security.Audit.Error.ChainBrokenPreviousHashMismatch",
                        {{"lineNumber", std::to_string(line_number)}}),
                    .entries = verified};
        }

        const auto calculated_hash = compute_entry_hash(timestamp, event_name, detail, expected_previous_hash);
        if (calculated_hash.empty()) {
            return {.ok = false,
                    .error = security_text(
                        "Security.Audit.Error.ComputeHashAtLineFailed",
                        {{"lineNumber", std::to_string(line_number)}}),
                    .entries = verified};
        }
        if (calculated_hash != observed_hash) {
            return {.ok = false,
                    .error = security_text(
                        "Security.Audit.Error.HashMismatchAtLine",
                        {{"lineNumber", std::to_string(line_number)}}),
                    .entries = verified};
        }

        previous_hash = observed_hash;
        ++verified;
    }

    return {.ok = true, .error = {}, .entries = verified};
}

}  // namespace copperfin::security

#include "copperfin/security/audit_stream.h"

#include "copperfin/security/sha256.h"
#include "localized_text.h"

#include <chrono>
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

std::string read_last_hash(const std::string& log_path) {
    std::ifstream input(log_path, std::ios::binary);
    if (!input) {
        return "GENESIS";
    }

    std::string line;
    std::string last_line;
    while (std::getline(input, line)) {
        if (!line.empty()) {
            last_line = line;
        }
    }

    if (last_line.empty()) {
        return "GENESIS";
    }

    const auto last_delimiter = last_line.rfind('|');
    if (last_delimiter == std::string::npos || last_delimiter + 1U >= last_line.size()) {
        return "GENESIS";
    }

    return last_line.substr(last_delimiter + 1U);
}

std::vector<std::string> split_audit_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    while (std::getline(stream, token, '|')) {
        tokens.push_back(token);
    }
    return tokens;
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

    const std::string previous_hash = read_last_hash(log_path);
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

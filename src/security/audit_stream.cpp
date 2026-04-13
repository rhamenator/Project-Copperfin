#include "copperfin/security/audit_stream.h"

#include "copperfin/security/sha256.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

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

}  // namespace

AuditAppendResult append_immutable_audit_event(
    const std::string& log_path,
    const std::string& event_name,
    const std::string& detail) {
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(log_path).parent_path(), error);
    if (error) {
        return {.ok = false, .error = "Unable to create audit log directory."};
    }

    const std::string previous_hash = read_last_hash(log_path);
    const std::string timestamp = now_utc_compact();
    const std::string safe_event = escape_field(event_name);
    const std::string safe_detail = escape_field(detail);

    const std::string signed_payload = timestamp + "|" + safe_event + "|" + safe_detail + "|" + previous_hash;
    const auto hash = sha256_hex_for_text(signed_payload);
    if (!hash.ok) {
        return {.ok = false, .error = hash.error};
    }

    std::ofstream output(log_path, std::ios::app | std::ios::binary);
    if (!output) {
        return {.ok = false, .error = "Unable to open audit log for append."};
    }

    output << signed_payload << "|" << hash.hex_digest << "\n";
    if (!output.good()) {
        return {.ok = false, .error = "Unable to append audit log entry."};
    }

    return {.ok = true, .entry_hash = hash.hex_digest};
}

}  // namespace copperfin::security

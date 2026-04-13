#pragma once

#include <string>

namespace copperfin::security {

struct AuditAppendResult {
    bool ok = false;
    std::string error;
    std::string entry_hash;
};

AuditAppendResult append_immutable_audit_event(
    const std::string& log_path,
    const std::string& event_name,
    const std::string& detail);

}  // namespace copperfin::security

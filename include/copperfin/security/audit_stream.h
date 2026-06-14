#pragma once

#include <string>

namespace copperfin::security {

struct AuditAppendResult {
    bool ok = false;
    std::string error;
    std::string entry_hash;
};

struct AuditChainVerifyResult {
    bool ok = false;
    std::string error;
    std::size_t entries = 0U;
};

AuditAppendResult append_immutable_audit_event(
    const std::string& log_path,
    const std::string& event_name,
    const std::string& detail);

AuditChainVerifyResult verify_immutable_audit_chain(const std::string& log_path);

}  // namespace copperfin::security

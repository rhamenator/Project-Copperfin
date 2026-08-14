// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
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

AuditAppendResult append_immutable_audit_event_to_contained_file(
    const std::string& log_path,
    const std::string& package_root,
    const std::string& event_name,
    const std::string& detail);

AuditAppendResult append_bounded_immutable_audit_event_to_contained_file(
    const std::string& log_path,
    const std::string& package_root,
    const std::string& event_name,
    const std::string& detail,
    std::size_t max_log_bytes);

AuditChainVerifyResult verify_immutable_audit_chain(const std::string& log_path);

}  // namespace copperfin::security

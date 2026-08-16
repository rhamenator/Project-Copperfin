// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/path.h"
#include "copperfin/security/audit_stream.h"
#include "copperfin/security/workspace_agent_audit_sink.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

namespace fs = std::filesystem;
using copperfin::security::WorkspaceAgentAccessMode;
using copperfin::security::WorkspaceAgentActivationRequest;
using copperfin::security::WorkspaceAgentSessionAuditEvent;
using copperfin::security::WorkspaceAgentSessionAuditFileSink;
using copperfin::security::WorkspaceAgentSessionController;

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

class ScopedTempRoot final {
public:
    explicit ScopedTempRoot(std::string label) {
        path_ = fs::temp_directory_path() /
            ("copperfin-workspace-agent-audit-" + std::move(label) + "-" +
             std::to_string(
                 std::chrono::steady_clock::now().time_since_epoch().count()));
        std::error_code error;
        fs::create_directories(path_, error);
        ready_ = !error;
    }

    ~ScopedTempRoot() {
        std::error_code ignored;
        fs::remove_all(path_, ignored);
    }

    [[nodiscard]] const fs::path& path() const {
        return path_;
    }

    [[nodiscard]] bool ready() const {
        return ready_;
    }

private:
    fs::path path_;
    bool ready_ = false;
};

WorkspaceAgentActivationRequest request_for(const WorkspaceAgentAccessMode mode) {
    return {
        .requested_mode = mode,
        .feature_enabled = true,
        .permission_granted = true,
        .trusted_product_ui = true,
        .audit_sink_available = true,
        .warning_presented = mode == WorkspaceAgentAccessMode::unrestricted_local,
        .warning_id = mode == WorkspaceAgentAccessMode::unrestricted_local
            ? copperfin::security::workspace_agent_unrestricted_warning_id
            : "",
        .user_confirmed = mode == WorkspaceAgentAccessMode::unrestricted_local};
}

std::string read_bytes(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

std::vector<std::string> split(const std::string& value, const char separator) {
    std::vector<std::string> fields;
    std::size_t begin = 0U;
    while (begin <= value.size()) {
        const std::size_t end = value.find(separator, begin);
        fields.push_back(value.substr(
            begin,
            end == std::string::npos ? std::string::npos : end - begin));
        if (end == std::string::npos) {
            break;
        }
        begin = end + 1U;
    }
    return fields;
}

std::vector<std::vector<std::string>> audit_lines(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::vector<std::vector<std::string>> lines;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty()) {
            lines.push_back(split(line, '|'));
        }
    }
    return lines;
}

void test_persistent_lifecycle_is_content_free_and_verifiable() {
    ScopedTempRoot root("lifecycle");
    expect(root.ready(), "RQ-CF-AGENT-006: lifecycle audit root should be created");
    if (!root.ready()) {
        return;
    }

    WorkspaceAgentSessionAuditFileSink file_sink(root.path(), "agent/session.log");
    expect(file_sink.ready() && file_sink.session_sink().commit != nullptr,
           "RQ-CF-AGENT-006: a contained persistent sink configuration should be ready");
    expect(file_sink.log_path() == root.path() / "agent/session.log",
           "RQ-CF-AGENT-006: the sink should expose only its normalized contained log path");

    WorkspaceAgentSessionController controller;
    const auto started = controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        file_sink.session_sink());
    expect(started.activated && started.audit_committed &&
               started.audit_receipt.size() == 64U && started.session.active,
           "RQ-CF-AGENT-006: durable start persistence should provide the authority receipt");
    const auto stopped = controller.stop(file_sink.session_sink());
    expect(stopped.revoked && stopped.audit_committed &&
               stopped.audit_receipt.size() == 64U && !stopped.session.active,
           "RQ-CF-AGENT-006: durable stop persistence should follow immediate revocation");

    WorkspaceAgentSessionController denied_controller;
    auto disabled = request_for(WorkspaceAgentAccessMode::unrestricted_local);
    disabled.feature_enabled = false;
    disabled.warning_id = "caller-content-must-not-persist";
    const auto denied = denied_controller.start(disabled, file_sink.session_sink());
    expect(!denied.activated && denied.audit_committed &&
               denied.audit_receipt.size() == 64U && !denied.session.active,
           "RQ-CF-AGENT-006: a policy denial should persist without creating authority");

    const auto verified = copperfin::security::verify_immutable_audit_chain(
        copperfin::platform::path_to_utf8_string(file_sink.log_path()));
    expect(verified.ok && verified.entries == 3U,
           "RQ-CF-AGENT-006: the persistent lifecycle log should retain a verifiable chain");

    const auto lines = audit_lines(file_sink.log_path());
    expect(lines.size() == 3U,
           "RQ-CF-AGENT-006: start, stop, and denial should each produce one record");
    if (lines.size() != 3U) {
        return;
    }
    for (const auto& line : lines) {
        expect(line.size() == 5U && line[1] == "workspace_agent.session.v1",
               "RQ-CF-AGENT-006: every persisted line should use the exact versioned event identity");
    }
    if (lines[0].size() == 5U && lines[1].size() == 5U && lines[2].size() == 5U) {
        const WorkspaceAgentSessionAuditEvent expected_start{
            .kind = copperfin::security::WorkspaceAgentSessionEventKind::start,
            .session_generation = 1U,
            .requested_mode = WorkspaceAgentAccessMode::workspace_sandbox,
            .effective_mode = WorkspaceAgentAccessMode::workspace_sandbox,
            .outcome = "allowed",
            .diagnostic_code = "workspace_agent.sandbox_allowed"};
        const WorkspaceAgentSessionAuditEvent expected_stop{
            .kind = copperfin::security::WorkspaceAgentSessionEventKind::stop,
            .session_generation = 1U,
            .requested_mode = WorkspaceAgentAccessMode::workspace_sandbox,
            .effective_mode = WorkspaceAgentAccessMode::advisory,
            .outcome = "revoked",
            .diagnostic_code = "workspace_agent.session_stopped"};
        const WorkspaceAgentSessionAuditEvent expected_denial{
            .kind = copperfin::security::WorkspaceAgentSessionEventKind::start,
            .session_generation = 1U,
            .requested_mode = WorkspaceAgentAccessMode::unrestricted_local,
            .effective_mode = WorkspaceAgentAccessMode::advisory,
            .outcome = "denied",
            .diagnostic_code = "workspace_agent.feature_disabled"};
        expect(lines[0][2] == serialize_workspace_agent_session_audit_event(expected_start) &&
                   lines[0][4] == started.audit_receipt,
               "RQ-CF-AGENT-006: the start record and returned receipt should bind exact bytes");
        expect(lines[1][2] == serialize_workspace_agent_session_audit_event(expected_stop) &&
                   lines[1][4] == stopped.audit_receipt,
               "RQ-CF-AGENT-006: the stop record and returned receipt should bind exact bytes");
        expect(lines[2][2] == serialize_workspace_agent_session_audit_event(expected_denial) &&
                   lines[2][4] == denied.audit_receipt,
               "RQ-CF-AGENT-006: denial persistence should retain only the admitted machine event");
    }

    const std::string persisted = read_bytes(file_sink.log_path());
    expect(persisted.find("caller-content-must-not-persist") == std::string::npos &&
               persisted.find(copperfin::platform::path_to_utf8_string(root.path())) ==
                   std::string::npos &&
               persisted.find("\"receipt\"") == std::string::npos &&
               persisted.find("\"prompt\"") == std::string::npos &&
               persisted.find("\"credential\"") == std::string::npos &&
               persisted.find("\"token\"") == std::string::npos,
           "RQ-CF-AGENT-006: persistent records must exclude warning, path, receipt, prompt, credential, and token content");
}

void test_direct_malformed_events_are_rejected_without_mutation() {
    ScopedTempRoot root("malformed");
    expect(root.ready(), "RQ-CF-AGENT-006: malformed-event audit root should be created");
    if (!root.ready()) {
        return;
    }
    WorkspaceAgentSessionAuditFileSink file_sink(root.path(), "session.log");
    const auto sink = file_sink.session_sink();
    const WorkspaceAgentSessionAuditEvent valid{
        .kind = copperfin::security::WorkspaceAgentSessionEventKind::start,
        .session_generation = 7U,
        .requested_mode = WorkspaceAgentAccessMode::advisory,
        .effective_mode = WorkspaceAgentAccessMode::advisory,
        .outcome = "allowed",
        .diagnostic_code = "workspace_agent.advisory_allowed"};
    const auto initial = sink.commit(valid, sink.context);
    expect(initial.ok, "RQ-CF-AGENT-006: malformed-event setup should persist one valid event");
    const WorkspaceAgentSessionAuditEvent cleanup_intent{
        .kind = copperfin::security::WorkspaceAgentSessionEventKind::
            layout_cleanup_intent,
        .session_generation = 7U,
        .requested_mode = WorkspaceAgentAccessMode::advisory,
        .effective_mode = WorkspaceAgentAccessMode::advisory,
        .outcome = "pending",
        .diagnostic_code = "workspace_agent.session_layout_cleanup_intent"};
    const WorkspaceAgentSessionAuditEvent cleanup_outcome{
        .kind = copperfin::security::WorkspaceAgentSessionEventKind::
            layout_cleanup_outcome,
        .session_generation = 7U,
        .requested_mode = WorkspaceAgentAccessMode::advisory,
        .effective_mode = WorkspaceAgentAccessMode::advisory,
        .outcome = "cleaned",
        .diagnostic_code =
            "workspace_agent.environment_session_layout_cleaned"};
    expect(sink.commit(cleanup_intent, sink.context).ok &&
               sink.commit(cleanup_outcome, sink.context).ok,
           "RQ-CF-AGENT-021: the durable sink must admit exact cleanup intent and outcome records");
    const WorkspaceAgentSessionAuditEvent process_intent{
        .schema_version = 2U,
        .kind = copperfin::security::WorkspaceAgentSessionEventKind::
            process_launch_intent,
        .session_generation = 7U,
        .requested_mode = WorkspaceAgentAccessMode::unrestricted_local,
        .effective_mode = WorkspaceAgentAccessMode::unrestricted_local,
        .process_instance_id = "0123456789abcdef0123456789abcdef",
        .operation_id = 9U,
        .outcome = "pending",
        .diagnostic_code = "workspace_agent.process_launch_intent"};
    const WorkspaceAgentSessionAuditEvent process_outcome{
        .schema_version = 2U,
        .kind = copperfin::security::WorkspaceAgentSessionEventKind::
            process_launch_outcome,
        .session_generation = 7U,
        .requested_mode = WorkspaceAgentAccessMode::unrestricted_local,
        .effective_mode = WorkspaceAgentAccessMode::unrestricted_local,
        .process_instance_id = "0123456789abcdef0123456789abcdef",
        .operation_id = 9U,
        .outcome = "exited",
        .diagnostic_code = "polyglot.process.exited"};
    expect(sink.commit(process_intent, sink.context).ok &&
               sink.commit(process_outcome, sink.context).ok,
           "RQ-CF-AGENT-028: the durable sink must admit exact correlated process intent and outcome records");
    const auto retained_lines = audit_lines(file_sink.log_path());
    expect(retained_lines.size() == 5U &&
               retained_lines[3].size() == 5U &&
               retained_lines[3][1] == "workspace_agent.process.v2" &&
               retained_lines[4].size() == 5U &&
               retained_lines[4][1] == "workspace_agent.process.v2" &&
               retained_lines[3][2] ==
                   serialize_workspace_agent_session_audit_event(process_intent) &&
               retained_lines[4][2] ==
                   serialize_workspace_agent_session_audit_event(process_outcome),
           "RQ-CF-AGENT-028: process audit records must retain the versioned content-free operation id contract");
    std::uint64_t diagnostic_operation_id = 10U;
    for (const std::string diagnostic : {
             "polyglot.process.executable_path_unsupported",
             "polyglot.process.working_directory_path_unsupported"}) {
        auto unsupported_intent = process_intent;
        auto unsupported_path = process_outcome;
        unsupported_intent.operation_id = diagnostic_operation_id;
        unsupported_path.operation_id = diagnostic_operation_id++;
        unsupported_path.outcome = "launch-failed";
        unsupported_path.diagnostic_code = diagnostic;
        expect(sink.commit(unsupported_intent, sink.context).ok &&
                   sink.commit(unsupported_path, sink.context).ok,
               "RQ-CF-AGENT-028: the durable sink must admit only fixed content-free device-path compatibility diagnostics");
    }
    const std::string before = read_bytes(file_sink.log_path());

    std::vector<WorkspaceAgentSessionAuditEvent> malformed;
    auto stale_schema = valid;
    stale_schema.schema_version = 2U;
    malformed.push_back(stale_schema);
    auto zero_generation = valid;
    zero_generation.session_generation = 0U;
    malformed.push_back(zero_generation);
    auto exhausted_generation = valid;
    exhausted_generation.session_generation =
        std::numeric_limits<std::uint64_t>::max();
    malformed.push_back(exhausted_generation);
    auto substituted_outcome = valid;
    substituted_outcome.outcome = "denied";
    malformed.push_back(substituted_outcome);
    auto capability_mismatch = valid;
    capability_mismatch.effective_mode = WorkspaceAgentAccessMode::workspace_sandbox;
    malformed.push_back(capability_mismatch);
    auto content_injection = valid;
    content_injection.diagnostic_code = "workspace_agent.advisory_allowed/secret/path/token";
    malformed.push_back(content_injection);
    auto impossible_warning_denial = valid;
    impossible_warning_denial.outcome = "denied";
    impossible_warning_denial.diagnostic_code = "workspace_agent.warning_required";
    malformed.push_back(impossible_warning_denial);
    auto invalid_kind = valid;
    invalid_kind.kind = static_cast<copperfin::security::WorkspaceAgentSessionEventKind>(99);
    malformed.push_back(invalid_kind);
    auto missing_operation = process_intent;
    missing_operation.operation_id = 0U;
    malformed.push_back(missing_operation);
    auto missing_process_instance = process_intent;
    missing_process_instance.process_instance_id.clear();
    malformed.push_back(missing_process_instance);
    auto zero_process_instance = process_intent;
    zero_process_instance.process_instance_id =
        "00000000000000000000000000000000";
    malformed.push_back(zero_process_instance);
    auto injected_process_instance = process_intent;
    injected_process_instance.process_instance_id =
        "0123456789abcdef/path-or-secret";
    malformed.push_back(injected_process_instance);
    auto injected_process_diagnostic = process_outcome;
    injected_process_diagnostic.diagnostic_code =
        "polyglot.process.exited/secret/path/token";
    malformed.push_back(injected_process_diagnostic);
    auto mismatched_process_mode = process_outcome;
    mismatched_process_mode.effective_mode = WorkspaceAgentAccessMode::workspace_sandbox;
    malformed.push_back(mismatched_process_mode);
    auto mismatched_process_outcome = process_outcome;
    mismatched_process_outcome.outcome = "timed-out";
    malformed.push_back(mismatched_process_outcome);
    auto launch_failure_as_denial = process_outcome;
    launch_failure_as_denial.outcome = "denied";
    launch_failure_as_denial.diagnostic_code = "polyglot.process.launch_failed";
    malformed.push_back(launch_failure_as_denial);

    for (const auto& event : malformed) {
        const auto result = sink.commit(event, sink.context);
        expect(!result.ok && result.receipt.empty(),
               "RQ-CF-AGENT-006: stale, mismatched, injected, or unknown events must fail closed");
    }
    expect(read_bytes(file_sink.log_path()) == before,
           "RQ-CF-AGENT-006: rejected events must not mutate the durable audit chain");
}

void test_configuration_containment_and_size_limit_fail_closed() {
    ScopedTempRoot root("boundaries");
    ScopedTempRoot outside("outside");
    expect(root.ready() && outside.ready(),
           "RQ-CF-AGENT-006: boundary audit roots should be created");
    if (!root.ready() || !outside.ready()) {
        return;
    }

    WorkspaceAgentSessionAuditFileSink traversal(root.path(), "../escape.log");
    WorkspaceAgentSessionAuditFileSink directory_leaf(root.path(), ".");
    WorkspaceAgentSessionAuditFileSink absolute(root.path(), outside.path() / "escape.log");
    WorkspaceAgentSessionAuditFileSink missing(root.path() / "missing", "events.log");
    WorkspaceAgentSessionAuditFileSink too_small(
        root.path(), "small.log", copperfin::security::workspace_agent_audit_min_log_bytes - 1U);
    WorkspaceAgentSessionAuditFileSink too_large(
        root.path(), "large.log", copperfin::security::workspace_agent_audit_max_log_bytes + 1U);
    expect(!traversal.ready() && traversal.session_sink().commit == nullptr &&
               !directory_leaf.ready() && directory_leaf.session_sink().commit == nullptr &&
               !absolute.ready() && absolute.session_sink().commit == nullptr &&
               !missing.ready() && missing.session_sink().commit == nullptr &&
               !too_small.ready() && too_small.session_sink().commit == nullptr &&
               !too_large.ready() && too_large.session_sink().commit == nullptr,
           "RQ-CF-AGENT-006: escaping, missing-root, and unsafe-size configurations must be inert");

    auto embedded_nul_name = fs::path("target.log").native();
    embedded_nul_name.push_back(fs::path::value_type{});
    embedded_nul_name += fs::path("different").native();
    WorkspaceAgentSessionAuditFileSink embedded_nul(
        root.path(), fs::path(embedded_nul_name));
    expect(!embedded_nul.ready() && embedded_nul.session_sink().commit == nullptr &&
               !fs::exists(root.path() / "target.log"),
           "RQ-CF-AGENT-006: embedded-NUL log paths must be inert before syscall truncation");
    auto direct_nul_name = (root.path() / "direct.log").native();
    direct_nul_name.push_back(fs::path::value_type{});
    direct_nul_name += fs::path("different").native();
    const auto direct_embedded_nul =
        copperfin::security::append_bounded_immutable_audit_event_to_contained_file(
            copperfin::platform::path_to_utf8_string(fs::path(direct_nul_name)),
            copperfin::platform::path_to_utf8_string(root.path()),
            "event",
            "detail",
            copperfin::security::workspace_agent_audit_min_log_bytes);
    expect(!direct_embedded_nul.ok && direct_embedded_nul.entry_hash.empty() &&
               !fs::exists(root.path() / "direct.log"),
           "RQ-CF-AGENT-006: the bounded writer must reject embedded-NUL paths directly");

    auto embedded_nul_root_name = root.path().native();
    embedded_nul_root_name.push_back(fs::path::value_type{});
    embedded_nul_root_name += fs::path("different").native();
    const fs::path embedded_nul_root(embedded_nul_root_name);
    WorkspaceAgentSessionAuditFileSink embedded_nul_storage_root(
        embedded_nul_root, "root-event.log");
    expect(!embedded_nul_storage_root.ready() &&
               embedded_nul_storage_root.session_sink().commit == nullptr &&
               !fs::exists(root.path() / "root-event.log"),
           "RQ-CF-AGENT-006: embedded-NUL storage roots must fail before canonicalization");
    const auto direct_embedded_nul_root =
        copperfin::security::append_bounded_immutable_audit_event_to_contained_file(
            copperfin::platform::path_to_utf8_string(root.path() / "root-direct.log"),
            copperfin::platform::path_to_utf8_string(embedded_nul_root),
            "event",
            "detail",
            copperfin::security::workspace_agent_audit_min_log_bytes);
    expect(!direct_embedded_nul_root.ok && direct_embedded_nul_root.entry_hash.empty() &&
               !fs::exists(root.path() / "root-direct.log"),
           "RQ-CF-AGENT-006: bounded-writer roots must reject NULs before canonicalization");

    auto cancelled_component_name = fs::path("discard").native();
    cancelled_component_name.push_back(fs::path::value_type{});
    cancelled_component_name += fs::path("suffix").native();
    const fs::path cancellable_log =
        root.path() / fs::path(cancelled_component_name) / ".." / "cancelled.log";
    const auto cancelled_embedded_nul =
        copperfin::security::append_bounded_immutable_audit_event_to_contained_file(
            copperfin::platform::path_to_utf8_string(cancellable_log),
            copperfin::platform::path_to_utf8_string(root.path()),
            "event",
            "detail",
            copperfin::security::workspace_agent_audit_min_log_bytes);
    expect(!cancelled_embedded_nul.ok && cancelled_embedded_nul.entry_hash.empty() &&
               !fs::exists(root.path() / "cancelled.log"),
           "RQ-CF-AGENT-006: NUL components must fail before lexical normalization can erase them");

    const fs::path oversized_parent = root.path() / "oversized" / "nested";
    const fs::path oversized_log = oversized_parent / "audit.log";
    const auto oversized =
        copperfin::security::append_bounded_immutable_audit_event_to_contained_file(
            copperfin::platform::path_to_utf8_string(oversized_log),
            copperfin::platform::path_to_utf8_string(root.path()),
            std::string(copperfin::security::workspace_agent_audit_min_log_bytes, 'E'),
            std::string(copperfin::security::workspace_agent_audit_min_log_bytes, 'D'),
            copperfin::security::workspace_agent_audit_min_log_bytes);
    expect(!oversized.ok && oversized.entry_hash.empty() &&
               !fs::exists(oversized_parent) && !fs::exists(oversized_log),
           "RQ-CF-AGENT-006: oversized direct inputs must fail before path mutation");

    const fs::path genesis_boundary_parent = root.path() / "genesis-boundary" / "nested";
    const fs::path genesis_boundary_log = genesis_boundary_parent / "audit.log";
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    const std::size_t timestamp_size = std::to_string(millis).size();
    const std::size_t boundary_max =
        copperfin::security::workspace_agent_audit_min_log_bytes;
    const std::size_t boundary_event_size = boundary_max - timestamp_size - 64U - 5U;
    const auto genesis_boundary =
        copperfin::security::append_bounded_immutable_audit_event_to_contained_file(
            copperfin::platform::path_to_utf8_string(genesis_boundary_log),
            copperfin::platform::path_to_utf8_string(root.path()),
            std::string(boundary_event_size, 'E'),
            {},
            boundary_max);
    expect(!genesis_boundary.ok && genesis_boundary.entry_hash.empty() &&
               !fs::exists(genesis_boundary_parent) && !fs::exists(genesis_boundary_log),
           "RQ-CF-AGENT-006: empty-chain GENESIS bytes must be preflighted before path mutation");

    const fs::path bounded_log = root.path() / "bounded.log";
    std::size_t prefilled_size = 0U;
    for (int index = 0; index < 8 && prefilled_size <
             copperfin::security::workspace_agent_audit_min_log_bytes; ++index) {
        const auto appended = copperfin::security::append_immutable_audit_event_to_contained_file(
            copperfin::platform::path_to_utf8_string(bounded_log),
            copperfin::platform::path_to_utf8_string(root.path()),
            "test.prefill",
            "bounded-prefill");
        expect(appended.ok, "RQ-CF-AGENT-006: size-bound fixture append should succeed");
        prefilled_size = read_bytes(bounded_log).size();
    }
    expect(prefilled_size >= copperfin::security::workspace_agent_audit_min_log_bytes,
           "RQ-CF-AGENT-006: size-bound fixture should reach the configured minimum");
    const std::string bounded_before = read_bytes(bounded_log);
    WorkspaceAgentSessionAuditFileSink bounded(root.path(), "bounded.log", prefilled_size);
    WorkspaceAgentSessionController bounded_controller;
    const auto bounded_start = bounded_controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        bounded.session_sink());
    expect(bounded.ready() && !bounded_start.activated && !bounded_start.audit_committed &&
               bounded_start.diagnostic_code == "workspace_agent.session_audit_commit_failed" &&
               !bounded_controller.snapshot().active &&
               read_bytes(bounded_log) == bounded_before,
           "RQ-CF-AGENT-006: a full bounded log must withhold authority and preserve existing bytes");

    const fs::path tampered_log = root.path() / "tampered.log";
    const auto clean_entry =
        copperfin::security::append_immutable_audit_event_to_contained_file(
            copperfin::platform::path_to_utf8_string(tampered_log),
            copperfin::platform::path_to_utf8_string(root.path()),
            "test.clean",
            "clean-detail");
    expect(clean_entry.ok, "RQ-CF-AGENT-006: tampered-chain fixture append should succeed");
    std::string tampered_before = read_bytes(tampered_log);
    const std::size_t detail_offset = tampered_before.find("clean-detail");
    expect(detail_offset != std::string::npos,
           "RQ-CF-AGENT-006: tampered-chain fixture should contain its detail");
    if (detail_offset != std::string::npos) {
        tampered_before[detail_offset] = 'X';
        std::ofstream output(tampered_log, std::ios::binary | std::ios::trunc);
        output << tampered_before;
    }
    const auto tampered_verification = copperfin::security::verify_immutable_audit_chain(
        copperfin::platform::path_to_utf8_string(tampered_log));
    WorkspaceAgentSessionAuditFileSink tampered(root.path(), "tampered.log");
    WorkspaceAgentSessionController tampered_controller;
    const auto tampered_start = tampered_controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        tampered.session_sink());
    expect(!tampered_verification.ok && !tampered_start.activated &&
               !tampered_start.audit_committed &&
               !tampered_controller.snapshot().active &&
               read_bytes(tampered_log) == tampered_before,
           "RQ-CF-AGENT-006: a well-shaped tampered chain must fail closed without mutation");

    const fs::path broken_log = root.path() / "broken.log";
    {
        std::ofstream output(broken_log, std::ios::binary | std::ios::trunc);
        output << "truncated-audit-tail";
    }
    const std::string broken_before = read_bytes(broken_log);
    WorkspaceAgentSessionAuditFileSink broken(root.path(), "broken.log");
    WorkspaceAgentSessionController broken_controller;
    const auto broken_start = broken_controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        broken.session_sink());
    expect(!broken_start.activated && !broken_start.audit_committed &&
               !broken_controller.snapshot().active &&
               read_bytes(broken_log) == broken_before,
           "RQ-CF-AGENT-006: a malformed existing chain must fail closed without mutation");

    std::error_code symlink_error;
    const fs::path linked_parent = root.path() / "linked";
    fs::create_directory_symlink(outside.path(), linked_parent, symlink_error);
    if (!symlink_error) {
        WorkspaceAgentSessionAuditFileSink linked(root.path(), "linked/events.log");
        WorkspaceAgentSessionController linked_controller;
        const auto linked_start = linked_controller.start(
            request_for(WorkspaceAgentAccessMode::workspace_sandbox),
            linked.session_sink());
        expect(!linked.ready() && !linked_start.activated && !linked_start.audit_committed &&
                   !linked_controller.snapshot().active &&
                   !fs::exists(outside.path() / "events.log"),
               "RQ-CF-AGENT-006: an intermediate link must not redirect persistent audit bytes");
    }

    std::error_code in_root_symlink_error;
    const fs::path in_root_target = root.path() / "real-audit-directory";
    fs::create_directories(in_root_target, in_root_symlink_error);
    const fs::path in_root_link = root.path() / "in-root-link";
    if (!in_root_symlink_error) {
        fs::create_directory_symlink(in_root_target, in_root_link, in_root_symlink_error);
    }
    if (!in_root_symlink_error) {
        WorkspaceAgentSessionAuditFileSink linked(root.path(), "in-root-link/events.log");
        WorkspaceAgentSessionController linked_controller;
        const auto linked_start = linked_controller.start(
            request_for(WorkspaceAgentAccessMode::workspace_sandbox),
            linked.session_sink());
        expect(!linked.ready() && !linked_start.activated && !linked_start.audit_committed &&
                   !linked_controller.snapshot().active &&
                   !fs::exists(in_root_target / "events.log"),
               "RQ-CF-AGENT-006: an in-root link must not be canonicalized into an admitted path");
    }

    std::error_code swapped_symlink_error;
    WorkspaceAgentSessionAuditFileSink swapped(root.path(), "swapped-link/events.log");
    const fs::path swapped_target = root.path() / "swapped-real-directory";
    fs::create_directories(swapped_target, swapped_symlink_error);
    if (!swapped_symlink_error) {
        fs::create_directory_symlink(
            swapped_target, root.path() / "swapped-link", swapped_symlink_error);
    }
    if (!swapped_symlink_error) {
        WorkspaceAgentSessionController swapped_controller;
        const auto swapped_start = swapped_controller.start(
            request_for(WorkspaceAgentAccessMode::workspace_sandbox),
            swapped.session_sink());
        expect(swapped.ready() && !swapped_start.activated &&
                   !swapped_start.audit_committed &&
                   !swapped_controller.snapshot().active &&
                   !fs::exists(swapped_target / "events.log"),
               "RQ-CF-AGENT-006: a post-construction in-root link swap must fail closed");
    }
}

}  // namespace

int main() {
    test_persistent_lifecycle_is_content_free_and_verifiable();
    test_direct_malformed_events_are_rejected_without_mutation();
    test_configuration_containment_and_size_limit_fail_closed();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

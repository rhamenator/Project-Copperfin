// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_session.h"

#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using copperfin::security::WorkspaceAgentAccessMode;
using copperfin::security::WorkspaceAgentActivationRequest;
using copperfin::security::WorkspaceAgentSessionAuditCommitResult;
using copperfin::security::WorkspaceAgentSessionAuditEvent;
using copperfin::security::WorkspaceAgentSessionAuditSink;
using copperfin::security::WorkspaceAgentSessionController;

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

WorkspaceAgentActivationRequest request_for(WorkspaceAgentAccessMode mode) {
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

enum class CommitBehavior {
    succeed,
    fail,
    empty_receipt,
    throw_exception
};

struct AuditContext {
    CommitBehavior behavior = CommitBehavior::succeed;
    std::vector<WorkspaceAgentSessionAuditEvent> events;
    WorkspaceAgentSessionController* controller = nullptr;
    bool require_revoked_during_callback = false;
    bool observed_revoked_during_callback = false;
    bool block_commit = false;
    bool commit_entered = false;
    bool release_commit = false;
    std::mutex commit_mutex;
    std::condition_variable commit_condition;
};

WorkspaceAgentSessionAuditCommitResult commit_event(
    const WorkspaceAgentSessionAuditEvent& event,
    void* opaque_context) {
    auto* context = static_cast<AuditContext*>(opaque_context);
    if (context == nullptr) {
        return {};
    }
    context->events.push_back(event);
    if (context->block_commit) {
        std::unique_lock lock(context->commit_mutex);
        context->commit_entered = true;
        context->commit_condition.notify_all();
        context->commit_condition.wait(lock, [context] {
            return context->release_commit;
        });
    }
    if (context->require_revoked_during_callback && context->controller != nullptr) {
        context->observed_revoked_during_callback = !context->controller->snapshot().active;
    }
    switch (context->behavior) {
        case CommitBehavior::succeed:
            return {
                .ok = true,
                .receipt = "receipt-" + std::to_string(context->events.size())};
        case CommitBehavior::fail:
            return {};
        case CommitBehavior::empty_receipt:
            return {.ok = true, .receipt = {}};
        case CommitBehavior::throw_exception:
            throw std::runtime_error("synthetic workspace-agent audit failure");
    }
    return {};
}

WorkspaceAgentSessionAuditSink sink_for(AuditContext& context) {
    return {.commit = commit_event, .context = &context};
}

void test_audited_session_lifecycle_and_capability_binding() {
    WorkspaceAgentSessionController controller;
    AuditContext audit;
    audit.controller = &controller;

    const auto started = controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        sink_for(audit));
    expect(started.activated && started.audit_committed &&
               started.audit_receipt == "receipt-1" &&
               started.diagnostic_code == "workspace_agent.sandbox_allowed",
           "RQ-CF-AGENT-005: an admitted session should require a committed audit receipt");
    expect(started.session.active && started.session.generation == 1U &&
               started.session.effective_mode == WorkspaceAgentAccessMode::workspace_sandbox &&
               started.session.capabilities.read_workspace_files &&
               started.session.capabilities.write_workspace_files &&
               started.session.capabilities.run_local_processes &&
               !started.session.capabilities.access_outside_workspace &&
               !started.session.capabilities.use_network &&
               !started.session.capabilities.elevate_privileges &&
               started.session.activation_audit_receipt == "receipt-1",
           "RQ-CF-AGENT-005: a session snapshot should bind only the admitted mode capabilities");
    expect(audit.events.size() == 1U && audit.events.front().outcome == "allowed" &&
               audit.events.front().diagnostic_code == "workspace_agent.sandbox_allowed",
           "RQ-CF-AGENT-005: start audit should be content-free and retain the policy outcome");

    const auto replacement = controller.start(
        request_for(WorkspaceAgentAccessMode::unrestricted_local),
        sink_for(audit));
    expect(!replacement.activated && replacement.audit_committed &&
               replacement.diagnostic_code == "workspace_agent.session_already_active" &&
               replacement.session.active && replacement.session.generation == 1U &&
               !replacement.session.capabilities.access_outside_workspace,
           "RQ-CF-AGENT-005: a second start must not replace or expand an active session");
    expect(audit.events.size() == 2U && audit.events.back().outcome == "denied" &&
               audit.events.back().requested_mode == WorkspaceAgentAccessMode::unrestricted_local,
           "RQ-CF-AGENT-005: a rejected replacement attempt should be audited");

    audit.require_revoked_during_callback = true;
    const auto stopped = controller.stop(sink_for(audit));
    expect(stopped.revoked && stopped.audit_committed &&
               stopped.diagnostic_code == "workspace_agent.session_stopped" &&
               !stopped.session.active && audit.observed_revoked_during_callback,
           "RQ-CF-AGENT-005: stop must revoke authority before the audit callback executes");
    const auto inactive = controller.snapshot();
    expect(!inactive.active && inactive.generation == 0U &&
               inactive.effective_mode == WorkspaceAgentAccessMode::advisory &&
               !inactive.capabilities.read_workspace_files &&
               inactive.activation_audit_receipt.empty(),
           "RQ-CF-AGENT-005: an inactive snapshot must expose no residual authority or receipt");

    const auto duplicate_stop = controller.stop(sink_for(audit));
    expect(!duplicate_stop.revoked && !duplicate_stop.audit_committed &&
               duplicate_stop.diagnostic_code == "workspace_agent.session_not_active" &&
               audit.events.size() == 3U,
           "RQ-CF-AGENT-005: stopping an inactive controller should be a side-effect-free misuse result");
}

void test_denials_are_audited_without_creating_authority() {
    WorkspaceAgentSessionController controller;
    AuditContext audit;
    auto disabled = request_for(WorkspaceAgentAccessMode::workspace_sandbox);
    disabled.feature_enabled = false;
    const auto denied = controller.start(disabled, sink_for(audit));
    expect(!denied.activated && denied.audit_committed &&
               denied.diagnostic_code == "workspace_agent.feature_disabled" &&
               !denied.session.active && audit.events.size() == 1U &&
               audit.events.front().outcome == "denied",
           "RQ-CF-AGENT-005: policy denials should commit content-free audit evidence without authority");

    auto missing_audit = request_for(WorkspaceAgentAccessMode::advisory);
    missing_audit.audit_sink_available = false;
    const auto policy_denied = controller.start(missing_audit, sink_for(audit));
    expect(!policy_denied.activated && policy_denied.audit_committed &&
               policy_denied.diagnostic_code == "workspace_agent.audit_unavailable" &&
               !controller.snapshot().active,
           "RQ-CF-AGENT-005: an available commit callback must not override policy's audit-availability denial");

    auto stale_warning = request_for(WorkspaceAgentAccessMode::unrestricted_local);
    stale_warning.warning_id = "workspace-agent.unrestricted-local.v0";
    const auto stale = controller.start(stale_warning, sink_for(audit));
    expect(!stale.activated && stale.audit_committed &&
               stale.diagnostic_code == "workspace_agent.warning_version_mismatch" &&
               !controller.snapshot().active,
           "RQ-CF-AGENT-005: a stale unrestricted warning must remain denied at session creation");
}

void test_audit_failures_withhold_start_but_cannot_extend_stop() {
    for (const auto behavior : {
             CommitBehavior::fail,
             CommitBehavior::empty_receipt,
             CommitBehavior::throw_exception}) {
        WorkspaceAgentSessionController controller;
        AuditContext audit;
        audit.behavior = behavior;
        const auto result = controller.start(
            request_for(WorkspaceAgentAccessMode::unrestricted_local),
            sink_for(audit));
        expect(!result.activated && !result.audit_committed && result.audit_receipt.empty() &&
                   result.diagnostic_code == "workspace_agent.session_audit_commit_failed" &&
                   !controller.snapshot().active,
               "RQ-CF-AGENT-005: failed, empty-receipt, and throwing start audit sinks must fail closed");
    }

    WorkspaceAgentSessionController absent_controller;
    const auto absent = absent_controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        {});
    expect(!absent.activated && !absent.audit_committed &&
               absent.diagnostic_code == "workspace_agent.session_audit_commit_failed" &&
               !absent_controller.snapshot().active,
           "RQ-CF-AGENT-005: an absent start audit sink must fail closed");

    WorkspaceAgentSessionController stop_controller;
    AuditContext successful;
    const auto started = stop_controller.start(
        request_for(WorkspaceAgentAccessMode::unrestricted_local),
        sink_for(successful));
    expect(started.activated, "RQ-CF-AGENT-005: stop-failure setup should activate a session");
    AuditContext failing_stop;
    failing_stop.behavior = CommitBehavior::throw_exception;
    const auto stopped = stop_controller.stop(sink_for(failing_stop));
    expect(stopped.revoked && !stopped.audit_committed &&
               stopped.diagnostic_code == "workspace_agent.session_stop_audit_commit_failed" &&
               !stop_controller.snapshot().active,
           "RQ-CF-AGENT-005: stop audit failure must remain visible without extending authority");
}

void test_overlapping_start_cannot_observe_or_replace_partial_authority() {
    WorkspaceAgentSessionController controller;
    AuditContext blocked_audit;
    blocked_audit.block_commit = true;
    copperfin::security::WorkspaceAgentSessionStartResult first_result;
    std::thread first_start([&controller, &blocked_audit, &first_result] {
        first_result = controller.start(
            request_for(WorkspaceAgentAccessMode::workspace_sandbox),
            sink_for(blocked_audit));
    });

    bool commit_entered = false;
    {
        std::unique_lock lock(blocked_audit.commit_mutex);
        commit_entered = blocked_audit.commit_condition.wait_for(
            lock,
            std::chrono::seconds(5),
            [&blocked_audit] { return blocked_audit.commit_entered; });
    }
    if (!commit_entered) {
        {
            std::lock_guard lock(blocked_audit.commit_mutex);
            blocked_audit.release_commit = true;
        }
        blocked_audit.commit_condition.notify_all();
        first_start.join();
        expect(false,
               "RQ-CF-AGENT-005: the blocking audit callback should begin within its test bound");
        return;
    }
    expect(!controller.snapshot().active,
           "RQ-CF-AGENT-005: authority must remain absent while its start audit is uncommitted");

    AuditContext overlapping_audit;
    const auto overlapping = controller.start(
        request_for(WorkspaceAgentAccessMode::unrestricted_local),
        sink_for(overlapping_audit));
    expect(!overlapping.activated && !overlapping.audit_committed &&
               overlapping.diagnostic_code ==
                   "workspace_agent.session_transition_in_progress" &&
               !overlapping.session.active && overlapping_audit.events.empty(),
           "RQ-CF-AGENT-005: an overlapping start must not replace, expand, or observe partial authority");

    {
        std::lock_guard lock(blocked_audit.commit_mutex);
        blocked_audit.release_commit = true;
    }
    blocked_audit.commit_condition.notify_all();
    first_start.join();

    expect(first_result.activated && first_result.audit_committed &&
               first_result.session.effective_mode ==
                   WorkspaceAgentAccessMode::workspace_sandbox &&
               !first_result.session.capabilities.access_outside_workspace,
           "RQ-CF-AGENT-005: the serialized first start should retain only its admitted capabilities");

    AuditContext stop_audit;
    expect(controller.stop(sink_for(stop_audit)).revoked,
           "RQ-CF-AGENT-005: concurrency test cleanup should revoke the admitted session");
}

void test_audit_serialization_is_stable_and_content_free() {
    const WorkspaceAgentSessionAuditEvent event{
        .kind = copperfin::security::WorkspaceAgentSessionEventKind::start,
        .session_generation = 42U,
        .requested_mode = WorkspaceAgentAccessMode::unrestricted_local,
        .effective_mode = WorkspaceAgentAccessMode::advisory,
        .outcome = "denied",
        .diagnostic_code = "workspace_agent.test\ncode"};
    const std::string serialized =
        copperfin::security::serialize_workspace_agent_session_audit_event(event);
    expect(
        serialized ==
            "{\"schema_version\":1,\"event\":\"start\",\"session_generation\":42,"
            "\"requested_mode\":\"unrestricted_local\",\"effective_mode\":\"advisory\","
            "\"outcome\":\"denied\",\"diagnostic_code\":\"workspace_agent.test\\ncode\"}",
        "RQ-CF-AGENT-005: session audit JSON should preserve its versioned machine contract");
    expect(serialized.find("prompt") == std::string::npos &&
               serialized.find("path") == std::string::npos &&
               serialized.find("credential") == std::string::npos &&
               serialized.find("receipt") == std::string::npos,
           "RQ-CF-AGENT-005: session audit events must not carry content, paths, credentials, or receipts");
}

}  // namespace

int main() {
    test_audited_session_lifecycle_and_capability_binding();
    test_denials_are_audited_without_creating_authority();
    test_audit_failures_withhold_start_but_cannot_extend_stop();
    test_overlapping_start_cannot_observe_or_replace_partial_authority();
    test_audit_serialization_is_stable_and_content_free();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

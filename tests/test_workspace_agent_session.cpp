// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_session.h"

#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <locale>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

using copperfin::security::WorkspaceAgentAccessMode;
using copperfin::security::WorkspaceAgentActivationRequest;
using copperfin::security::WorkspaceAgentSessionAuditCommitResult;
using copperfin::security::WorkspaceAgentSessionAuditEvent;
using copperfin::security::WorkspaceAgentSessionAuditSink;
using copperfin::security::WorkspaceAgentSessionController;
using copperfin::security::WorkspaceAgentToolPreflightRequest;

int failures = 0;

class GroupEveryDigit final : public std::numpunct<char> {
protected:
    char do_thousands_sep() const override {
        return ',';
    }

    std::string do_grouping() const override {
        return "\1";
    }
};

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

WorkspaceAgentToolPreflightRequest tool_request(
    std::uint64_t generation,
    std::string_view tool_id) {
    return {
        .session_generation = generation,
        .tool_id = std::string(tool_id)};
}

void test_tool_preflight_is_session_bound_and_fail_closed() {
    WorkspaceAgentSessionController controller;

    const auto invalid_schema = controller.preflight_tool_request({
        .schema_version = 2U,
        .session_generation = 1U,
        .tool_id = std::string(
            copperfin::security::workspace_agent_tool_workspace_inspect)});
    expect(!invalid_schema.allowed &&
               invalid_schema.diagnostic_code == "workspace_agent.tool_invalid_schema",
           "RQ-CF-AGENT-007: unknown tool preflight schemas must fail closed");
    const auto empty = controller.preflight_tool_request({
        .session_generation = 1U,
        .tool_id = {}});
    expect(!empty.allowed &&
               empty.diagnostic_code == "workspace_agent.tool_not_registered",
           "RQ-CF-AGENT-008: an empty tool id must not resolve a capability set");
    for (const std::string_view unknown : {
             "workspace.inspect.v2",
             "Workspace.inspect.v1",
             "provider.custom.v1"}) {
        const auto rejected = controller.preflight_tool_request(tool_request(1U, unknown));
        expect(!rejected.allowed && rejected.tool_id.empty() &&
                   rejected.diagnostic_code == "workspace_agent.tool_not_registered",
               "RQ-CF-AGENT-008: unknown and provider-defined tool ids must fail closed without reflection");
    }
    const auto inactive = controller.preflight_tool_request(tool_request(
        1U,
        copperfin::security::workspace_agent_tool_workspace_inspect));
    expect(!inactive.allowed &&
               inactive.diagnostic_code == "workspace_agent.session_not_active",
           "RQ-CF-AGENT-007: an inactive controller must grant no tool preflight");

    AuditContext sandbox_audit;
    const auto sandbox = controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        sink_for(sandbox_audit));
    expect(sandbox.activated,
           "RQ-CF-AGENT-007: sandbox fixture should establish audited authority");

    const auto admitted = controller.preflight_tool_request(tool_request(
        sandbox.session.generation,
        copperfin::security::workspace_agent_tool_workspace_run_process));
    expect(admitted.allowed &&
               admitted.session_generation == sandbox.session.generation &&
               admitted.effective_mode == WorkspaceAgentAccessMode::workspace_sandbox &&
               admitted.tool_id ==
                   copperfin::security::workspace_agent_tool_workspace_run_process &&
               admitted.diagnostic_code == "workspace_agent.tool_request_allowed",
           "RQ-CF-AGENT-008: sandbox preflight should use the registered complete capability set");

    const auto stale = controller.preflight_tool_request(tool_request(
        sandbox.session.generation + 1U,
        copperfin::security::workspace_agent_tool_workspace_inspect));
    expect(!stale.allowed &&
               stale.session_generation == 0U &&
               stale.effective_mode == WorkspaceAgentAccessMode::advisory &&
               stale.diagnostic_code == "workspace_agent.tool_stale_session",
           "RQ-CF-AGENT-007: a mismatched generation must not reuse or disclose current session authority");
    const auto zero_generation = controller.preflight_tool_request(tool_request(
        0U,
        copperfin::security::workspace_agent_tool_workspace_inspect));
    expect(!zero_generation.allowed &&
               zero_generation.diagnostic_code == "workspace_agent.tool_stale_session",
           "RQ-CF-AGENT-007: generation zero must not identify an active session");

    for (const std::string_view denied_tool : {
             copperfin::security::workspace_agent_tool_local_inspect,
             copperfin::security::workspace_agent_tool_local_apply_edit,
             copperfin::security::workspace_agent_tool_local_run_process,
             copperfin::security::workspace_agent_tool_network_request}) {
        const auto denied = controller.preflight_tool_request(tool_request(
            sandbox.session.generation,
            denied_tool));
        expect(!denied.allowed &&
                   denied.tool_id == denied_tool &&
                   denied.diagnostic_code == "workspace_agent.tool_capability_denied",
               "RQ-CF-AGENT-008: any unavailable registered capability must deny the whole tool");
    }

    AuditContext sandbox_stop;
    expect(controller.stop(sink_for(sandbox_stop)).revoked,
           "RQ-CF-AGENT-007: sandbox fixture should revoke cleanly");
    const auto after_stop = controller.preflight_tool_request(tool_request(
        sandbox.session.generation,
        copperfin::security::workspace_agent_tool_workspace_inspect));
    expect(!after_stop.allowed &&
               after_stop.diagnostic_code == "workspace_agent.session_not_active",
           "RQ-CF-AGENT-007: a previously admitted generation must fail after stop");
}

void test_registered_tool_preflight_mode_matrix() {
    for (const auto mode : {
             WorkspaceAgentAccessMode::advisory,
             WorkspaceAgentAccessMode::unrestricted_local}) {
        WorkspaceAgentSessionController controller;
        AuditContext start_audit;
        const auto started = controller.start(request_for(mode), sink_for(start_audit));
        expect(started.activated,
               "RQ-CF-AGENT-007: mode fixture should establish audited authority");

        const auto read = controller.preflight_tool_request(tool_request(
            started.session.generation,
            copperfin::security::workspace_agent_tool_workspace_inspect));
        expect(read.allowed == (mode == WorkspaceAgentAccessMode::unrestricted_local),
               "RQ-CF-AGENT-007: preflight must preserve the active mode's read capability");

        const auto broad = controller.preflight_tool_request(tool_request(
            started.session.generation,
            copperfin::security::workspace_agent_tool_local_run_process));
        expect(broad.allowed == (mode == WorkspaceAgentAccessMode::unrestricted_local),
               "RQ-CF-AGENT-008: only unrestricted mode may preflight a registered local broad tool");

        AuditContext stop_audit;
        expect(controller.stop(sink_for(stop_audit)).revoked,
               "RQ-CF-AGENT-007: mode fixture should revoke cleanly");
    }
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

void test_policy_exception_fails_closed_and_restores_transition() {
#if !defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path original_directory = fs::current_path();
    const fs::path removed_directory = fs::temp_directory_path() /
        ("copperfin-workspace-agent-session-" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::error_code filesystem_error;
    fs::create_directories(removed_directory, filesystem_error);
    if (filesystem_error) {
        expect(false, "RQ-CF-AGENT-005: policy-exception fixture directory should be created");
        return;
    }
    fs::current_path(removed_directory, filesystem_error);
    if (filesystem_error) {
        fs::remove_all(removed_directory, filesystem_error);
        expect(false, "RQ-CF-AGENT-005: policy-exception fixture should become the working directory");
        return;
    }

    const char* configured_locale = std::getenv("COPPERFIN_LOCALE_DIR");
    const bool had_configured_locale = configured_locale != nullptr;
    const std::string saved_locale = had_configured_locale ? configured_locale : "";
    unsetenv("COPPERFIN_LOCALE_DIR");
    filesystem_error.clear();
    fs::remove(removed_directory, filesystem_error);
    if (filesystem_error) {
        fs::current_path(original_directory, filesystem_error);
        fs::remove_all(removed_directory, filesystem_error);
        if (had_configured_locale) {
            setenv("COPPERFIN_LOCALE_DIR", saved_locale.c_str(), 1);
        }
        expect(false, "RQ-CF-AGENT-005: policy-exception fixture directory should be removable");
        return;
    }

    WorkspaceAgentSessionController controller;
    AuditContext failed_policy_audit;
    const auto denied = controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        sink_for(failed_policy_audit));

    filesystem_error.clear();
    fs::current_path(original_directory, filesystem_error);
    if (had_configured_locale) {
        setenv("COPPERFIN_LOCALE_DIR", saved_locale.c_str(), 1);
    }
    expect(!filesystem_error,
           "RQ-CF-AGENT-005: policy-exception fixture should restore the working directory");
    expect(!denied.activated && denied.audit_committed &&
               denied.diagnostic_code == "workspace_agent.policy_evaluation_failed" &&
               denied.policy_decision.diagnostic_code ==
                   "workspace_agent.policy_evaluation_failed" &&
               failed_policy_audit.events.size() == 1U &&
               failed_policy_audit.events.front().outcome == "denied" &&
               !controller.snapshot().active,
           "RQ-CF-AGENT-005: a throwing policy dependency should fail closed with an audited denial");

    AuditContext recovery_audit;
    const auto recovered = controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        sink_for(recovery_audit));
    expect(recovered.activated && recovered.audit_committed && recovered.session.active,
           "RQ-CF-AGENT-005: policy failure must restore the transition for a later valid start");
    AuditContext stop_audit;
    expect(controller.stop(sink_for(stop_audit)).revoked,
           "RQ-CF-AGENT-005: recovered session should remain immediately revocable");
#endif
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
    const auto during_start = controller.preflight_tool_request(tool_request(
        1U,
        copperfin::security::workspace_agent_tool_workspace_inspect));
    expect(!during_start.allowed &&
               during_start.diagnostic_code ==
                   "workspace_agent.session_transition_in_progress",
           "RQ-CF-AGENT-007: tool preflight must fail closed while session start is incomplete");

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
    stop_audit.block_commit = true;
    copperfin::security::WorkspaceAgentSessionStopResult stop_result;
    std::thread stop_thread([&controller, &stop_audit, &stop_result] {
        stop_result = controller.stop(sink_for(stop_audit));
    });
    bool stop_commit_entered = false;
    {
        std::unique_lock lock(stop_audit.commit_mutex);
        stop_commit_entered = stop_audit.commit_condition.wait_for(
            lock,
            std::chrono::seconds(5),
            [&stop_audit] { return stop_audit.commit_entered; });
    }
    if (!stop_commit_entered) {
        {
            std::lock_guard lock(stop_audit.commit_mutex);
            stop_audit.release_commit = true;
        }
        stop_audit.commit_condition.notify_all();
        stop_thread.join();
        expect(false,
               "RQ-CF-AGENT-007: the blocking stop audit should begin within its test bound");
        return;
    }
    const auto during_stop = controller.preflight_tool_request(tool_request(
        first_result.session.generation,
        copperfin::security::workspace_agent_tool_workspace_inspect));
    expect(!during_stop.allowed &&
               during_stop.diagnostic_code ==
                   "workspace_agent.session_transition_in_progress",
           "RQ-CF-AGENT-007: tool preflight must fail closed after authority is revoked during stop");
    {
        std::lock_guard lock(stop_audit.commit_mutex);
        stop_audit.release_commit = true;
    }
    stop_audit.commit_condition.notify_all();
    stop_thread.join();
    expect(stop_result.revoked,
           "RQ-CF-AGENT-005: concurrency test cleanup should revoke the admitted session");
}

void test_audit_serialization_is_stable_and_content_free() {
    const WorkspaceAgentSessionAuditEvent event{
        .kind = copperfin::security::WorkspaceAgentSessionEventKind::start,
        .session_generation = 1234567U,
        .requested_mode = WorkspaceAgentAccessMode::unrestricted_local,
        .effective_mode = WorkspaceAgentAccessMode::advisory,
        .outcome = "denied",
        .diagnostic_code = std::string("workspace_agent.test") + '\x7f'};
    const std::locale previous_locale = std::locale();
    std::locale::global(std::locale(previous_locale, new GroupEveryDigit));
    const std::string serialized =
        copperfin::security::serialize_workspace_agent_session_audit_event(event);
    std::locale::global(previous_locale);
    expect(
        serialized ==
            "{\"schema_version\":1,\"event\":\"start\",\"session_generation\":1234567,"
            "\"requested_mode\":\"unrestricted_local\",\"effective_mode\":\"advisory\","
            "\"outcome\":\"denied\",\"diagnostic_code\":\"workspace_agent.test\\u007f\"}",
        "RQ-CF-AGENT-005: session audit JSON should preserve its locale-independent machine contract");
    expect(serialized.find("prompt") == std::string::npos &&
               serialized.find("path") == std::string::npos &&
               serialized.find("credential") == std::string::npos &&
               serialized.find("receipt") == std::string::npos,
           "RQ-CF-AGENT-005: session audit events must not carry content, paths, credentials, or receipts");
}

void test_launch_revocation_lease_is_generation_bound_and_blocks_stop() {
    using copperfin::security::WorkspaceAgentSessionRevocationLease;
    static_assert(!std::is_copy_constructible_v<WorkspaceAgentSessionRevocationLease>);
    static_assert(!std::is_copy_assignable_v<WorkspaceAgentSessionRevocationLease>);
    static_assert(std::is_nothrow_move_constructible_v<WorkspaceAgentSessionRevocationLease>);

    WorkspaceAgentSessionController controller;
    const auto inactive =
        controller.acquire_process_launch_revocation_lease(1U);
    expect(!inactive.acquired && !inactive.lease.has_value() &&
               inactive.diagnostic_code == "workspace_agent.session_not_active",
           "RQ-CF-AGENT-022: inactive sessions must not create a revocation lease");

    AuditContext advisory_start_audit;
    const auto advisory = controller.start(
        request_for(WorkspaceAgentAccessMode::advisory),
        sink_for(advisory_start_audit));
    expect(advisory.activated,
           "RQ-CF-AGENT-022: advisory denial fixture should establish audited authority");
    const auto incapable = controller.acquire_process_launch_revocation_lease(
        advisory.session.generation);
    expect(!incapable.acquired && !incapable.lease.has_value() &&
               incapable.diagnostic_code ==
                   "workspace_agent.process_launch_lease_capability_denied",
           "RQ-CF-AGENT-022: a generation without process capability must not acquire a launch lease");
    AuditContext advisory_stop_audit;
    expect(controller.stop(sink_for(advisory_stop_audit)).revoked,
           "RQ-CF-AGENT-022: advisory denial fixture should revoke cleanly");

    AuditContext start_audit;
    const auto started = controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox),
        sink_for(start_audit));
    expect(started.activated,
           "RQ-CF-AGENT-022: lease fixture should establish audited authority");
    const auto stale = controller.acquire_process_launch_revocation_lease(
        started.session.generation + 1U);
    expect(!stale.acquired && !stale.lease.has_value() &&
               stale.diagnostic_code ==
                   "workspace_agent.process_launch_lease_stale_session",
           "RQ-CF-AGENT-022: a different generation must not acquire a lease");

    auto acquired = controller.acquire_process_launch_revocation_lease(
        started.session.generation);
    auto second = controller.acquire_process_launch_revocation_lease(
        started.session.generation);
    expect(acquired.acquired && acquired.lease.has_value() &&
               acquired.lease->valid() &&
               acquired.lease->session_generation() == started.session.generation &&
               acquired.diagnostic_code ==
                   "workspace_agent.process_launch_revocation_lease_acquired",
           "RQ-CF-AGENT-022: the active exact generation should acquire one move-only lease");
    expect(second.acquired && second.lease.has_value() && second.lease->valid(),
           "RQ-CF-AGENT-022: one generation may hold multiple short launch-boundary leases");

    AuditContext stop_audit;
    stop_audit.block_commit = true;
    copperfin::security::WorkspaceAgentSessionStopResult stop_result;
    std::thread stop_thread([&controller, &stop_audit, &stop_result] {
        stop_result = controller.stop(sink_for(stop_audit));
    });
    bool stop_transition_observed = false;
    const auto transition_deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < transition_deadline) {
        const auto during_stop = controller.preflight_tool_request(tool_request(
            started.session.generation,
            copperfin::security::workspace_agent_tool_workspace_inspect));
        if (during_stop.diagnostic_code ==
            "workspace_agent.session_transition_in_progress") {
            stop_transition_observed = true;
            break;
        }
        std::this_thread::yield();
    }
    expect(stop_transition_observed,
           "RQ-CF-AGENT-022: stop should enter its serialized transition while the lease is held");
    {
        std::unique_lock lock(stop_audit.commit_mutex);
        const bool stop_reached_audit = stop_audit.commit_condition.wait_for(
            lock,
            std::chrono::milliseconds(100),
            [&stop_audit] { return stop_audit.commit_entered; });
        expect(!stop_reached_audit,
               "RQ-CF-AGENT-022: stop must wait for an outstanding launch revocation lease before auditing revocation");
    }
    const auto still_denied =
        controller.revalidate_serialized_process_invocation_for_launch({}, {});
    expect(!still_denied.allowed &&
               still_denied.diagnostic_code ==
                   "workspace_agent.process_launch_revalidation_pinning_unavailable",
           "RQ-CF-AGENT-022: a revocation lease alone must not make the launch-promotion gate allow");

    acquired.lease.reset();
    {
        std::unique_lock lock(stop_audit.commit_mutex);
        const bool stop_reached_audit = stop_audit.commit_condition.wait_for(
            lock,
            std::chrono::milliseconds(100),
            [&stop_audit] { return stop_audit.commit_entered; });
        expect(!stop_reached_audit,
               "RQ-CF-AGENT-022: stop must wait until every outstanding lease is released");
    }
    second.lease.reset();
    bool stop_reached_audit = false;
    {
        std::unique_lock lock(stop_audit.commit_mutex);
        stop_reached_audit = stop_audit.commit_condition.wait_for(
            lock,
            std::chrono::seconds(5),
            [&stop_audit] { return stop_audit.commit_entered; });
        stop_audit.release_commit = true;
    }
    stop_audit.commit_condition.notify_all();
    stop_thread.join();
    expect(stop_reached_audit && stop_result.revoked &&
               !controller.snapshot().active,
           "RQ-CF-AGENT-022: releasing the lease must let stop revoke the session normally");
    expect(!acquired.lease.has_value(),
           "RQ-CF-AGENT-022: a released lease must retain no reusable authority");
}

}  // namespace

int main() {
    test_tool_preflight_is_session_bound_and_fail_closed();
    test_registered_tool_preflight_mode_matrix();
    test_audited_session_lifecycle_and_capability_binding();
    test_denials_are_audited_without_creating_authority();
    test_audit_failures_withhold_start_but_cannot_extend_stop();
    test_policy_exception_fails_closed_and_restores_transition();
    test_overlapping_start_cannot_observe_or_replace_partial_authority();
    test_audit_serialization_is_stable_and_content_free();
    test_launch_revocation_lease_is_generation_bound_and_blocks_stop();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

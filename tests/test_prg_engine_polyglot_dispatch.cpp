// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

namespace {

using namespace copperfin::test_support;

std::string formatted_global(
    const copperfin::runtime::RuntimePauseState& state,
    const std::string& name) {
    const auto found = state.globals.find(name);
    expect(found != state.globals.end(), name + " should be captured");
    return found == state.globals.end()
        ? std::string{}
        : copperfin::runtime::format_value(found->second);
}

void test_prg_polyglot_dispatch_validates_and_publishes_bounded_evidence() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_polyglot_dispatch";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path unavailable_path = temp_root / "unavailable.prg";
    write_text(
        unavailable_path,
        "cResult = CFPOLYGLOTDISPATCH('interop.invoice-v1', '{\"id\":1}', 7)\n"
        "cStatus = CFJSONGET(cResult, '/status')\n"
        "cReason = CFJSONGET(cResult, '/error_code')\n"
        "RETURN\n");
    auto unavailable_options = make_runtime_session_options(unavailable_path, temp_root);
    auto unavailable_session =
        copperfin::runtime::PrgRuntimeSession::create(unavailable_options);
    const auto unavailable_state = unavailable_session.run(
        copperfin::runtime::DebugResumeAction::continue_run);
    expect(unavailable_state.completed,
           "unconfigured PRG polyglot dispatch should fail closed without faulting");
    expect(formatted_global(unavailable_state, "cstatus") == "unavailable",
           "an unconfigured host dispatcher should report unavailable");
    expect(formatted_global(unavailable_state, "creason") ==
               "polyglot.prg.dispatch_unavailable",
           "an unconfigured host dispatcher should retain its invariant reason");

    const fs::path configured_path = temp_root / "configured.prg";
    write_text(
        configured_path,
        "cBadArity = CFPOLYGLOTDISPATCH()\n"
        "cBadCapability = CFPOLYGLOTDISPATCH('Bad Capability', '{}')\n"
        "cBadJson = CFPOLYGLOTDISPATCH('interop.invoice-v1', '[1]')\n"
        "cBadSample = CFPOLYGLOTDISPATCH('interop.invoice-v1', '{}', 100)\n"
        "cBadSampleType = CFPOLYGLOTDISPATCH('interop.invoice-v1', '{}', '7')\n"
        "ENTER CRITICAL routeguard\n"
        "cBlocked = CFPOLYGLOTDISPATCH('interop.invoice-v1', '{}', 7)\n"
        "EXIT CRITICAL routeguard\n"
        "cSuccess = CFPOLYGLOTDISPATCH('interop.invoice-v1', '{\"id\":9007199254740993}', 7)\n"
        "cSuccessStatus = CFJSONGET(cSuccess, '/status')\n"
        "cSuccessAuthority = CFJSONGET(cSuccess, '/authority')\n"
        "cSuccessRoute = CFJSONGET(cSuccess, '/route_selection')\n"
        "cSuccessExact = CFJSONGET(cSuccess, '/payload/id')\n"
        "nCandidateCalls = CFJSONGET(cSuccess, '/candidate_invocation_count')\n"
        "cInvalidHost = CFPOLYGLOTDISPATCH('interop.invalid-host-v1', '{}')\n"
        "cInvalidHostReason = CFJSONGET(cInvalidHost, '/error_code')\n"
        "cInvalidStatus = CFPOLYGLOTDISPATCH('interop.invalid-status-v1', '{}')\n"
        "cInvalidStatusReason = CFJSONGET(cInvalidStatus, '/error_code')\n"
        "cMalformedPayload = CFPOLYGLOTDISPATCH('interop.malformed-payload-v1', '{}')\n"
        "cMalformedPayloadReason = CFJSONGET(cMalformedPayload, '/error_code')\n"
        "cLargePayload = CFPOLYGLOTDISPATCH('interop.large-payload-v1', '{}')\n"
        "cLargePayloadReason = CFJSONGET(cLargePayload, '/error_code')\n"
        "cThrow = CFPOLYGLOTDISPATCH('interop.throw-v1', '{}')\n"
        "cThrowReason = CFJSONGET(cThrow, '/error_code')\n"
        "RETURN\n");

    std::mutex request_mutex;
    std::vector<copperfin::runtime::RuntimePolyglotDispatchRequest> requests;
    auto configured_options = make_runtime_session_options(configured_path, temp_root);
    configured_options.polyglot_dispatch_callback =
        [&](const copperfin::runtime::RuntimePolyglotDispatchRequest& request) {
            {
                std::lock_guard<std::mutex> lock(request_mutex);
                requests.push_back(request);
            }
            expect(request.cancellation_requested &&
                       !request.cancellation_requested(),
                   "dispatch should receive a live read-only cancellation probe");
            if (request.capability_id == "interop.throw-v1") {
                throw std::runtime_error("synthetic host failure");
            }
            if (request.capability_id == "interop.invalid-host-v1") {
                return copperfin::runtime::RuntimePolyglotDispatchResult{
                    .status = copperfin::runtime::RuntimePolyglotDispatchStatus::success,
                    .error_code = "polyglot.execution.candidate_success",
                    .authority = copperfin::runtime::RuntimePolyglotDispatchAuthority::none,
                    .selection = copperfin::runtime::RuntimePolyglotDispatchSelection::candidate,
                    .candidate_invocation_count = 1U,
                    .payload_json = "{}"};
            }
            if (request.capability_id == "interop.invalid-status-v1") {
                return copperfin::runtime::RuntimePolyglotDispatchResult{
                    .status = copperfin::runtime::RuntimePolyglotDispatchStatus::invalid_request,
                    .error_code = "polyglot.execution.invalid_request",
                    .authority = copperfin::runtime::RuntimePolyglotDispatchAuthority::candidate,
                    .selection = copperfin::runtime::RuntimePolyglotDispatchSelection::candidate,
                    .candidate_invocation_count = 1U,
                    .payload_json = "{}"};
            }
            if (request.capability_id == "interop.malformed-payload-v1") {
                return copperfin::runtime::RuntimePolyglotDispatchResult{
                    .status = copperfin::runtime::RuntimePolyglotDispatchStatus::success,
                    .error_code = "polyglot.execution.candidate_success",
                    .authority = copperfin::runtime::RuntimePolyglotDispatchAuthority::candidate,
                    .selection = copperfin::runtime::RuntimePolyglotDispatchSelection::candidate,
                    .candidate_invocation_count = 1U,
                    .payload_json = "{]"};
            }
            if (request.capability_id == "interop.large-payload-v1") {
                return copperfin::runtime::RuntimePolyglotDispatchResult{
                    .status = copperfin::runtime::RuntimePolyglotDispatchStatus::success,
                    .error_code = "polyglot.execution.candidate_success",
                    .authority = copperfin::runtime::RuntimePolyglotDispatchAuthority::candidate,
                    .selection = copperfin::runtime::RuntimePolyglotDispatchSelection::candidate,
                    .candidate_invocation_count = 1U,
                    .payload_json = "\"" + std::string(1024U * 1024U - 2U, 'x') + "\""};
            }
            return copperfin::runtime::RuntimePolyglotDispatchResult{
                .status = copperfin::runtime::RuntimePolyglotDispatchStatus::success,
                .error_code = "polyglot.execution.candidate_success",
                .authority = copperfin::runtime::RuntimePolyglotDispatchAuthority::candidate,
                .selection = copperfin::runtime::RuntimePolyglotDispatchSelection::candidate,
                .candidate_invocation_count = 1U,
                .payload_json = R"json({"id":9007199254740993,"message":"ok"})json"};
        };

    auto configured_session =
        copperfin::runtime::PrgRuntimeSession::create(configured_options);
    const auto configured_state = configured_session.run(
        copperfin::runtime::DebugResumeAction::continue_run);
    expect(configured_state.completed,
           "configured PRG polyglot dispatch should complete: " +
               configured_state.message);
    expect(formatted_global(configured_state, "csuccessstatus") == "success",
           "configured dispatch should expose success");
    expect(formatted_global(configured_state, "csuccessauthority") == "candidate",
           "configured dispatch should expose candidate authority");
    expect(formatted_global(configured_state, "csuccessroute") == "candidate",
           "configured dispatch should expose the selected route");
    expect(formatted_global(configured_state, "csuccessexact") ==
               "9007199254740993",
           "dispatch evidence should preserve exact JSON number spelling");
    expect(formatted_global(configured_state, "ncandidatecalls") == "1",
           "dispatch evidence should expose exact invocation counts");
    expect(formatted_global(configured_state, "cinvalidhostreason") ==
               "polyglot.prg.invalid_host_result",
           "internally inconsistent host evidence should fail closed");
    expect(formatted_global(configured_state, "cinvalidstatusreason") ==
               "polyglot.prg.invalid_host_result",
           "status and route evidence must describe one coherent outcome");
    expect(formatted_global(configured_state, "cmalformedpayloadreason") ==
               "polyglot.prg.invalid_host_result",
           "malformed host payload JSON should fail closed");
    expect(formatted_global(configured_state, "clargepayloadreason") ==
               "polyglot.prg.result_too_large",
           "a valid payload that overflows the evidence envelope should fail closed");
    expect(formatted_global(configured_state, "cthrowreason") ==
               "polyglot.prg.dispatch_exception",
           "host exceptions should be contained behind an invariant result");
    expect(formatted_global(configured_state, "cbadarity").find(
               "polyglot.prg.invalid_arguments") != std::string::npos,
           "invalid arity should fail before calling the host");
    expect(formatted_global(configured_state, "cbadcapability").find(
               "polyglot.prg.invalid_capability_id") != std::string::npos,
           "noncanonical capability IDs should fail before calling the host");
    expect(formatted_global(configured_state, "cbadjson").find(
               "polyglot.prg.invalid_arguments_json") != std::string::npos,
           "non-object JSON arguments should fail before calling the host");
    expect(formatted_global(configured_state, "cbadsample").find(
               "polyglot.prg.invalid_selection_sample") != std::string::npos &&
               formatted_global(configured_state, "cbadsampletype").find(
                   "polyglot.prg.invalid_selection_sample") != std::string::npos,
           "selection samples should require an exact numeric integer from 0 through 99");
    expect(formatted_global(configured_state, "cblocked").find(
               "polyglot.prg.blocked_in_critical_section") != std::string::npos,
           "potentially blocking dispatch should fail while a critical section is held");
    expect(requests.size() == 6U,
           "only validated, noncritical dispatches should reach the host callback");
    if (!requests.empty()) {
        expect(requests.front().capability_id == "interop.invoice-v1" &&
                   requests.front().arguments_json ==
                       R"json({"id":9007199254740993})json" &&
                   requests.front().selection_sample == 7U,
               "the callback should receive exact capability, JSON, and sample data");
    }
    expect(has_runtime_event(
               configured_state.events,
               "runtime.critical.blocking_violation",
               "operation=CFPOLYGLOTDISPATCH section=routeguard"),
           "critical-section rejection should use the centralized blocking-policy event");
    expect(has_runtime_event(
               configured_state.events,
               "runtime.polyglot.dispatch",
               "capability=interop.invoice-v1 status=success reason=polyglot.execution.candidate_success"),
           "successful dispatch should publish invariant result telemetry without payload bytes");

    fs::remove_all(temp_root, ignored);
}

void test_prg_polyglot_dispatch_remains_supervisable_through_spawn() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_polyglot_spawn";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_dispatch.prg";
    write_text(
        main_path,
        "PROCEDURE bridgeworker\n"
        "    RETURN CFPOLYGLOTDISPATCH('interop.spawn-v1', '{\"work\":true}', 23)\n"
        "ENDPROC\n"
        "SPAWN bridgeworker TO nTask\n"
        "cInitial = CFTASKSTATUS(nTask)\n"
        "DO WHILE CFTASKSTATUS(nTask) == 'running'\n"
        "    YIELD\n"
        "ENDDO\n"
        "cResult = CFTASKRESULT(nTask)\n"
        "cStatus = CFJSONGET(cResult, '/status')\n"
        "cPayload = CFJSONGET(cResult, '/payload/worker')\n"
        "AWAIT nTask TO lJoined\n"
        "RETURN\n");

    const std::thread::id caller_thread = std::this_thread::get_id();
    std::atomic<bool> ran_on_worker{false};
    auto options = make_runtime_session_options(main_path, temp_root);
    options.polyglot_dispatch_callback =
        [&](const copperfin::runtime::RuntimePolyglotDispatchRequest& request) {
            ran_on_worker.store(
                std::this_thread::get_id() != caller_thread,
                std::memory_order_relaxed);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            expect(request.capability_id == "interop.spawn-v1" &&
                       request.arguments_json == R"json({"work":true})json" &&
                       request.selection_sample == 23U,
                   "SPAWN should preserve the immutable dispatch request");
            return copperfin::runtime::RuntimePolyglotDispatchResult{
                .status = copperfin::runtime::RuntimePolyglotDispatchStatus::success,
                .error_code = "polyglot.execution.candidate_success",
                .authority = copperfin::runtime::RuntimePolyglotDispatchAuthority::candidate,
                .selection = copperfin::runtime::RuntimePolyglotDispatchSelection::candidate,
                .candidate_invocation_count = 1U,
                .payload_json = R"json({"worker":"complete"})json"};
        };

    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "spawn-supervised PRG polyglot dispatch should complete: " + state.message);
    expect(ran_on_worker.load(std::memory_order_relaxed),
           "SPAWN should keep synchronous dispatch work off the parent PRG task");
    expect(formatted_global(state, "cinitial") == "running",
           "CFTASKSTATUS should observe active dispatch without blocking");
    expect(formatted_global(state, "cstatus") == "success" &&
               formatted_global(state, "cpayload") == "complete",
           "CFTASKRESULT should publish the immutable dispatch evidence document");
    expect(formatted_global(state, "ljoined") == "true",
           "legacy AWAIT should still consume the supervised task");

    const fs::path cancel_path = temp_root / "cancel_dispatch.prg";
    write_text(
        cancel_path,
        "PROCEDURE bridgeworker\n"
        "    RETURN CFPOLYGLOTDISPATCH('interop.cancel-v1', '{}')\n"
        "ENDPROC\n"
        "SPAWN bridgeworker TO nTask\n"
        "SLEEP 250\n"
        "lCancelRequested = CFTASKCANCEL(nTask)\n"
        "DO WHILE CFTASKSTATUS(nTask) == 'running' OR CFTASKSTATUS(nTask) == 'cancel-requested'\n"
        "    YIELD\n"
        "ENDDO\n"
        "cStatus = CFTASKSTATUS(nTask)\n"
        "cResult = CFTASKRESULT(nTask)\n"
        "cDispatchStatus = CFJSONGET(cResult, '/status')\n"
        "AWAIT nTask TO lJoined\n"
        "RETURN\n");

    std::atomic<bool> observed_cancellation{false};
    auto cancel_options = make_runtime_session_options(cancel_path, temp_root);
    cancel_options.polyglot_dispatch_callback =
        [&](const copperfin::runtime::RuntimePolyglotDispatchRequest& request) {
            const auto deadline = std::chrono::steady_clock::now() +
                std::chrono::seconds(2);
            while (std::chrono::steady_clock::now() < deadline &&
                   !request.cancellation_requested()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            observed_cancellation.store(
                request.cancellation_requested(), std::memory_order_relaxed);
            return copperfin::runtime::RuntimePolyglotDispatchResult{
                .status = copperfin::runtime::RuntimePolyglotDispatchStatus::cancelled,
                .error_code = "polyglot.execution.cancelled",
                .authority = copperfin::runtime::RuntimePolyglotDispatchAuthority::candidate,
                .selection = copperfin::runtime::RuntimePolyglotDispatchSelection::candidate,
                .candidate_invocation_count = 1U,
                .payload_json = {}};
        };

    auto cancel_session = copperfin::runtime::PrgRuntimeSession::create(cancel_options);
    const auto cancel_state = cancel_session.run(
        copperfin::runtime::DebugResumeAction::continue_run);
    expect(cancel_state.completed,
           "cancelled spawned dispatch should complete: " + cancel_state.message);
    expect(formatted_global(cancel_state, "lcancelrequested") == "true" &&
               observed_cancellation.load(std::memory_order_relaxed),
           "CFTASKCANCEL should reach the host only through the read-only probe");
    expect(formatted_global(cancel_state, "cstatus") == "completed" &&
               formatted_global(cancel_state, "cdispatchstatus") == "cancelled" &&
               formatted_global(cancel_state, "ljoined") == "true",
           "a cooperative callback should publish cancelled evidence before completing");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_prg_polyglot_dispatch_validates_and_publishes_bounded_evidence();
    test_prg_polyglot_dispatch_remains_supervisable_through_spawn();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}

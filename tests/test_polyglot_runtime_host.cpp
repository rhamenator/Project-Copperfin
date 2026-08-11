// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/polyglot_runtime_host.h"
#include "copperfin/security/sha256.h"
#include "copperfin/platform/executable_path.h"
#include "prg_engine_test_support.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

using namespace copperfin::platform;
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;

constexpr const char* capability_id = "interop.runtime-host-v1";
constexpr const char* protocol_version = "1.0.0";
int failures = 0;

void expect_local(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void configure_binary_standard_streams() {
#if defined(_WIN32)
    (void)::_setmode(::_fileno(stdin), _O_BINARY);
    (void)::_setmode(::_fileno(stdout), _O_BINARY);
    (void)::_setmode(::_fileno(stderr), _O_BINARY);
#endif
}

std::string extract_json_string(
    const std::string& document,
    const std::string& field) {
    const std::string prefix = "\"" + field + "\":\"";
    const std::size_t begin = document.find(prefix);
    if (begin == std::string::npos) {
        return {};
    }
    const std::size_t value_begin = begin + prefix.size();
    const std::size_t end = document.find('"', value_begin);
    return end == std::string::npos
        ? std::string{}
        : document.substr(value_begin, end - value_begin);
}

int run_artifact(const std::string& mode) {
    configure_binary_standard_streams();
    const std::string request{
        std::istreambuf_iterator<char>(std::cin),
        std::istreambuf_iterator<char>()};
    if (mode == "--host-sleep") {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 0;
    }
    const std::string correlation = extract_json_string(
        request, "correlation_id");
    if (extract_json_string(request, "capability_id") != capability_id ||
        extract_json_string(request, "protocol_version") != protocol_version ||
        correlation.empty() ||
        request.find("\"arguments\":{\"value\":7}") == std::string::npos) {
        return 41;
    }
    std::cout
        << "{\"envelope_version\":\"1.0\",\"kind\":\"success\","
        << "\"capability_id\":\"" << capability_id << "\","
        << "\"correlation_id\":\"" << correlation << "\","
        << "\"protocol_version\":\"" << protocol_version << "\","
        << "\"payload\":{\"source\":\"candidate\",\"value\":7}}";
    return 0;
}

unsigned long process_id() {
#if defined(_WIN32)
    return static_cast<unsigned long>(::_getpid());
#else
    return static_cast<unsigned long>(::getpid());
#endif
}

fs::path unique_root() {
    return fs::temp_directory_path() /
        ("copperfin_polyglot_runtime_host_" +
         std::to_string(process_id()) + "_" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
}

std::string utf8_path(const fs::path& path) {
    return copperfin::platform::path_to_utf8_string(path);
}

bool copy_executable(const fs::path& source, const fs::path& destination) {
    std::error_code error;
    fs::copy_file(source, destination, fs::copy_options::overwrite_existing, error);
    if (error) {
        return false;
    }
#if !defined(_WIN32)
    fs::permissions(
        destination,
        fs::perms::owner_read | fs::perms::owner_write | fs::perms::owner_exec,
        fs::perm_options::add,
        error);
#endif
    return !error;
}

PolyglotArtifactAdmissionResult admit(
    const fs::path& artifact,
    const fs::path& root,
    const std::string& admitted_capability = capability_id) {
    const auto digest = copperfin::security::sha256_hex_for_file(
        utf8_path(artifact));
    expect_local(digest.ok, "runtime-host fixture should have a readable digest");
    return admit_polyglot_artifact({
        .capability_id = admitted_capability,
        .process_policy = {
            .executable_name = utf8_path(artifact),
            .allowed_path_roots = {utf8_path(root)},
            .allowed_publishers = {},
            .require_trusted_signature = false},
        .expected_sha256 = digest.hex_digest});
}

PolyglotRuntimeHostConfiguration configuration(
    const fs::path& artifact,
    const fs::path& root,
    const std::string& state,
    const std::string& artifact_mode = "--host-success",
    const std::uint8_t canary_percentage = 0U,
    const PolyglotFallbackPolicy fallback = PolyglotFallbackPolicy::fail_fast) {
    auto routes = load_polyglot_route_registry({
        {capability_id, state, canary_percentage}});
    expect_local(routes.ok(), "runtime-host fixture route should load");
    PolyglotArtifactInvocationRequest candidate;
    candidate.invocation = {
        .capability_id = capability_id,
        .correlation_id = "runtime-host-correlation",
        .protocol_version = protocol_version,
        .arguments_json = {}};
    candidate.policy = {
        .timeout_ms = 3000U,
        .latency_budget_ms = 2500U,
        .cancellation = PolyglotCancellationPolicy::propagate,
        .fallback = fallback,
        .max_attempts = 1U};
    candidate.artifact_arguments = {artifact_mode};
    candidate.working_directory = utf8_path(root);
    candidate.poll_interval_ms = 2U;
    PolyglotRuntimeCapabilityBinding binding{
        .capability_id = capability_id,
        .artifact_admission = admit(artifact, root),
        .candidate_request_template = std::move(candidate),
        .invoke_native = []() {
            return PolyglotNativeInvocationResult{
                true, {}, R"json({"source":"native","value":7})json"};
        },
        .normalize_shadow_parity = [](
            const PolyglotNativeInvocationResult& native,
            const PolyglotArtifactInvocationResult& candidate_result) {
            return PolyglotShadowParityValues{
                {{"$.value", "number", "number",
                  native.success ? "7" : "",
                  candidate_result.ok() ? "7" : ""}},
                {"$.value"},
                {"$.value"}};
        },
        .parity_policy = {}};
    return {std::move(routes.registry), {std::move(binding)}};
}

RuntimePolyglotDispatchResult dispatch_once(
    const std::shared_ptr<const PolyglotRuntimeHost>& host,
    const std::uint8_t sample = 0U,
    std::function<bool()> cancellation = []() { return false; }) {
    return host->dispatch_callback()({
        capability_id,
        R"json({"value":7})json",
        sample,
        std::move(cancellation)});
}

void test_route_states_and_owned_callback_lifetime(
    const fs::path& artifact,
    const fs::path& root) {
    auto off = PolyglotRuntimeHost::create(
        configuration(artifact, root, "off"));
    expect_local(off.ok(), "off runtime host should compose");
    const auto off_result = dispatch_once(off.host);
    expect_local(
        off_result.status == RuntimePolyglotDispatchStatus::success &&
        off_result.selection == RuntimePolyglotDispatchSelection::native &&
        off_result.authority == RuntimePolyglotDispatchAuthority::native &&
        off_result.native_invocation_count == 1U &&
        off_result.candidate_invocation_count == 0U,
        "off should preserve exact native route evidence");

    auto shadow = PolyglotRuntimeHost::create(
        configuration(artifact, root, "shadow"));
    const auto shadow_result = dispatch_once(shadow.host);
    expect_local(
        shadow_result.status == RuntimePolyglotDispatchStatus::success &&
        shadow_result.selection == RuntimePolyglotDispatchSelection::shadow &&
        shadow_result.authority == RuntimePolyglotDispatchAuthority::native &&
        shadow_result.native_invocation_count == 1U &&
        shadow_result.candidate_invocation_count == 1U,
        "shadow should preserve native authority and exact invocation counts");

    auto canary = PolyglotRuntimeHost::create(
        configuration(artifact, root, "canary", "--host-success", 50U));
    expect_local(
        dispatch_once(canary.host, 50U).selection ==
            RuntimePolyglotDispatchSelection::native,
        "canary boundary should remain native");
    const auto canary_candidate = dispatch_once(canary.host, 49U);
    expect_local(
        canary_candidate.selection == RuntimePolyglotDispatchSelection::candidate &&
        canary_candidate.authority == RuntimePolyglotDispatchAuthority::candidate &&
        canary_candidate.payload_json.find("candidate") != std::string::npos,
        "canary-selected candidate should preserve candidate authority and payload");

    for (const char* state : {"on", "retire-legacy"}) {
        auto built = PolyglotRuntimeHost::create(
            configuration(artifact, root, state));
        auto callback = built.host->dispatch_callback();
        built.host.reset();
        const auto result = callback({
            capability_id, R"json({"value":7})json", 0U, []() { return false; }});
        expect_local(
            result.status == RuntimePolyglotDispatchStatus::success &&
            result.selection == RuntimePolyglotDispatchSelection::candidate &&
            result.authority == RuntimePolyglotDispatchAuthority::candidate &&
            result.native_invocation_count == 0U &&
            result.candidate_invocation_count == 1U,
            std::string(state) +
                " callback should own trusted state beyond the host handle lifetime");
    }
}

void test_configuration_fails_before_launch(
    const fs::path& artifact,
    const fs::path& root) {
    auto missing = configuration(artifact, root, "on");
    missing.capabilities.clear();
    expect_local(
        PolyglotRuntimeHost::create(std::move(missing)).error_code ==
            "polyglot.host.capability_binding_required",
        "a route without a capability binding should fail construction");

    auto duplicate = configuration(artifact, root, "on");
    duplicate.capabilities.push_back(duplicate.capabilities.front());
    expect_local(
        PolyglotRuntimeHost::create(std::move(duplicate)).error_code ==
            "polyglot.host.duplicate_capability_binding",
        "duplicate capability bindings should fail construction");

    auto mismatch = configuration(artifact, root, "on");
    mismatch.capabilities.front().candidate_request_template.invocation.capability_id =
        "interop.other-v1";
    expect_local(
        PolyglotRuntimeHost::create(std::move(mismatch)).error_code ==
            "polyglot.host.capability_binding_mismatch",
        "route, admission, and invocation capabilities should match exactly");

    auto invalid_protocol = configuration(artifact, root, "on");
    invalid_protocol.capabilities.front()
        .candidate_request_template.invocation.protocol_version = "1.0";
    expect_local(
        PolyglotRuntimeHost::create(std::move(invalid_protocol)).error_code ==
            "polyglot.host.invalid_candidate_configuration",
        "invalid protocol identity should fail construction");

    auto callback_gap = configuration(artifact, root, "shadow");
    callback_gap.capabilities.front().normalize_shadow_parity = {};
    expect_local(
        PolyglotRuntimeHost::create(std::move(callback_gap)).error_code ==
            "polyglot.host.shadow_normalizer_required",
        "shadow callback gaps should fail construction before execution");

    auto built = PolyglotRuntimeHost::create(
        configuration(artifact, root, "on"));
    const auto unknown = built.host->dispatch_callback()({
        "interop.unknown-v1", "{}", 0U, []() { return false; }});
    expect_local(
        unknown.status == RuntimePolyglotDispatchStatus::invalid_request &&
        unknown.error_code == "polyglot.host.capability_unavailable" &&
        unknown.native_invocation_count == 0U &&
        unknown.candidate_invocation_count == 0U,
        "unknown capabilities should fail before native or candidate launch");
}

void test_cancellation_probe_reaches_bounded_candidate(
    const fs::path& artifact,
    const fs::path& root) {
    auto built = PolyglotRuntimeHost::create(
        configuration(artifact, root, "on", "--host-sleep"));
    std::atomic<std::uint32_t> polls{0U};
    const auto result = dispatch_once(built.host, 0U, [&]() {
        return polls.fetch_add(1U, std::memory_order_relaxed) >= 2U;
    });
    expect_local(
        result.status == RuntimePolyglotDispatchStatus::cancelled &&
        result.selection == RuntimePolyglotDispatchSelection::candidate &&
        result.authority == RuntimePolyglotDispatchAuthority::candidate &&
        result.candidate_invocation_count == 1U && polls.load() >= 3U,
        "the read-only cancellation probe should cooperatively stop one bounded candidate");
}

void test_prg_session_uses_production_host_callback(
    const fs::path& artifact,
    const fs::path& root) {
    const fs::path program = root / "runtime-host.prg";
    write_text(
        program,
        "cDispatch = CFPOLYGLOTDISPATCH('interop.runtime-host-v1', "
        "'{\"value\":7}', 0)\n"
        "cStatus = CFJSONGET(cDispatch, '/status')\n"
        "cAuthority = CFJSONGET(cDispatch, '/authority')\n"
        "cSource = CFJSONGET(cDispatch, '/payload/source')\n"
        "nCalls = CFJSONGET(cDispatch, '/candidate_invocation_count')\n"
        "RETURN\n");
    auto built = PolyglotRuntimeHost::create(
        configuration(artifact, root, "on"));
    auto options = make_runtime_session_options(program, root);
    options.polyglot_dispatch_callback = built.host->dispatch_callback();
    built.host.reset();
    auto session = PrgRuntimeSession::create(options);
    const auto state = session.run(DebugResumeAction::continue_run);
    expect_local(state.completed,
                 "PRG session should complete through the production host callback");
    const auto value = [&](const std::string& name) {
        const auto found = state.globals.find(name);
        return found == state.globals.end()
            ? std::string{}
            : format_value(found->second);
    };
    expect_local(
        value("cstatus") == "success" &&
        value("cauthority") == "candidate" &&
        value("csource") == "candidate" && value("ncalls") == "1",
        "PRG should receive only bounded invariant route evidence and candidate payload");
}

int run_tests(const fs::path& running_executable) {
    const fs::path root = unique_root();
    std::error_code error;
    fs::create_directories(root, error);
    expect_local(!error, "runtime-host fixture root should be created");
    const fs::path artifact = root /
        (std::string("runtime-host-artifact") + running_executable.extension().string());
    expect_local(copy_executable(running_executable, artifact),
                 "runtime-host fixture executable should be copied");
    if (!error && fs::exists(artifact)) {
        test_route_states_and_owned_callback_lifetime(artifact, root);
        test_configuration_fails_before_launch(artifact, root);
        test_cancellation_probe_reaches_bounded_candidate(artifact, root);
        test_prg_session_uses_production_host_callback(artifact, root);
    }
    fs::remove_all(root, error);
    return failures == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 2) {
        return run_artifact(argv[1]);
    }
    if (argc != 1) {
        return 40;
    }
    return run_tests(fs::absolute(argv[0]));
}

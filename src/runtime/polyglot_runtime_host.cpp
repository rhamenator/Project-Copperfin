// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/polyglot_runtime_host.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace copperfin::runtime {

namespace {

using platform::PolyglotRouteExecutionResult;
using platform::PolyglotRouteExecutionStatus;
using platform::PolyglotRouteResultAuthority;
using platform::PolyglotRouteSelection;

bool valid_route_state(const platform::PolyglotRouteState state) noexcept {
    switch (state) {
    case platform::PolyglotRouteState::off:
    case platform::PolyglotRouteState::shadow:
    case platform::PolyglotRouteState::canary:
    case platform::PolyglotRouteState::on:
    case platform::PolyglotRouteState::retire_legacy:
        return true;
    }
    return false;
}

bool valid_registry(const platform::PolyglotRouteRegistry& registry) {
    std::vector<platform::PolyglotRouteConfig> configs;
    configs.reserve(registry.entries.size());
    for (const auto& entry : registry.entries) {
        if (!valid_route_state(entry.state)) {
            return false;
        }
        configs.push_back({
            entry.capability_id,
            platform::polyglot_route_state_name(entry.state),
            entry.canary_percentage});
    }
    return platform::load_polyglot_route_registry(configs).ok();
}

bool route_can_invoke_native(
    const platform::PolyglotRouteEntry& route,
    const platform::PolyglotBridgeInvocationPolicy& policy) noexcept {
    if (route.state == platform::PolyglotRouteState::off ||
        route.state == platform::PolyglotRouteState::shadow ||
        route.state == platform::PolyglotRouteState::canary) {
        return true;
    }
    return route.state == platform::PolyglotRouteState::on &&
        policy.fallback == platform::PolyglotFallbackPolicy::fallback_native;
}

bool valid_supporting_artifact_bindings(
    const PolyglotRuntimeCapabilityBinding& binding) {
    if (binding.supporting_artifact_admissions.size() !=
        binding.supporting_artifact_arguments.size()) {
        return false;
    }
    std::vector<bool> arguments(
        binding.candidate_request_template.artifact_arguments.size(), false);
    std::vector<bool> admissions(
        binding.supporting_artifact_admissions.size(), false);
    for (const auto& mapped : binding.supporting_artifact_arguments) {
        if (mapped.argument_index >= arguments.size() ||
            mapped.admission_index >= admissions.size() ||
            arguments[mapped.argument_index] ||
            admissions[mapped.admission_index]) {
            return false;
        }
        const auto& admitted =
            binding.supporting_artifact_admissions[mapped.admission_index];
        if (!admitted.ok() || admitted.capability_id() != binding.capability_id ||
            binding.candidate_request_template
                    .artifact_arguments[mapped.argument_index] !=
                admitted.resolved_path()) {
            return false;
        }
        arguments[mapped.argument_index] = true;
        admissions[mapped.admission_index] = true;
    }
    return true;
}

RuntimePolyglotDispatchStatus map_status(
    const PolyglotRouteExecutionStatus status) noexcept {
    switch (status) {
    case PolyglotRouteExecutionStatus::success:
        return RuntimePolyglotDispatchStatus::success;
    case PolyglotRouteExecutionStatus::invalid_request:
        return RuntimePolyglotDispatchStatus::invalid_request;
    case PolyglotRouteExecutionStatus::native_failed:
        return RuntimePolyglotDispatchStatus::native_failed;
    case PolyglotRouteExecutionStatus::candidate_failed:
        return RuntimePolyglotDispatchStatus::candidate_failed;
    case PolyglotRouteExecutionStatus::cancelled:
        return RuntimePolyglotDispatchStatus::cancelled;
    case PolyglotRouteExecutionStatus::parity_failed:
        return RuntimePolyglotDispatchStatus::parity_failed;
    }
    return RuntimePolyglotDispatchStatus::invalid_request;
}

RuntimePolyglotDispatchAuthority map_authority(
    const PolyglotRouteResultAuthority authority) noexcept {
    switch (authority) {
    case PolyglotRouteResultAuthority::none:
        return RuntimePolyglotDispatchAuthority::none;
    case PolyglotRouteResultAuthority::native:
        return RuntimePolyglotDispatchAuthority::native;
    case PolyglotRouteResultAuthority::candidate:
        return RuntimePolyglotDispatchAuthority::candidate;
    }
    return RuntimePolyglotDispatchAuthority::none;
}

RuntimePolyglotDispatchSelection map_selection(
    const PolyglotRouteExecutionResult& result) noexcept {
    if (result.status == PolyglotRouteExecutionStatus::invalid_request) {
        return RuntimePolyglotDispatchSelection::none;
    }
    switch (result.route.selection) {
    case PolyglotRouteSelection::native:
        return RuntimePolyglotDispatchSelection::native;
    case PolyglotRouteSelection::shadow:
        return RuntimePolyglotDispatchSelection::shadow;
    case PolyglotRouteSelection::candidate:
        return RuntimePolyglotDispatchSelection::candidate;
    }
    return RuntimePolyglotDispatchSelection::none;
}

RuntimePolyglotDispatchResult invalid_result(std::string error_code) {
    RuntimePolyglotDispatchResult result;
    result.error_code = std::move(error_code);
    return result;
}

RuntimePolyglotDispatchResult map_result(
    const PolyglotRouteExecutionResult& source) {
    RuntimePolyglotDispatchResult result{
        .status = map_status(source.status),
        .error_code = source.error_code,
        .authority = map_authority(source.authority),
        .selection = map_selection(source),
        .native_invocation_count = source.native_invocation_count,
        .candidate_invocation_count = source.candidate_invocation_count,
        .native_fallback_executed = source.native_fallback_executed,
        .payload_json = {}};
    if (source.authority == PolyglotRouteResultAuthority::native &&
        source.native.success) {
        result.payload_json = source.native.payload;
    } else if (source.authority == PolyglotRouteResultAuthority::candidate &&
               source.candidate.response.ok()) {
        result.payload_json = source.candidate.response.envelope.payload_json;
    }
    return result;
}

}  // namespace

struct PolyglotRuntimeHost::State {
    struct Capability {
        explicit Capability(PolyglotRuntimeCapabilityBinding value)
            : binding(std::move(value)) {}

        PolyglotRuntimeCapabilityBinding binding;
        std::uint64_t correlation_sequence = 0U;
        std::mutex invocation_mutex;
    };

    platform::PolyglotRouteRegistry route_registry;
    std::map<std::string, std::shared_ptr<Capability>, std::less<>> capabilities;
};

PolyglotRuntimeHost::PolyglotRuntimeHost(std::shared_ptr<State> state)
    : state_(std::move(state)) {}

PolyglotRuntimeHostBuildResult PolyglotRuntimeHost::create(
    PolyglotRuntimeHostConfiguration configuration) {
    if (!valid_registry(configuration.route_registry)) {
        return {nullptr, "polyglot.host.invalid_route_registry"};
    }

    auto state = std::make_shared<State>();
    state->route_registry = std::move(configuration.route_registry);
    for (auto& binding : configuration.capabilities) {
        const auto route = std::find_if(
            state->route_registry.entries.begin(),
            state->route_registry.entries.end(),
            [&](const platform::PolyglotRouteEntry& entry) {
                return entry.capability_id == binding.capability_id;
            });
        if (route == state->route_registry.entries.end()) {
            return {nullptr, "polyglot.host.capability_route_required"};
        }
        if (!binding.artifact_admission.ok()) {
            return {nullptr, "polyglot.host.artifact_admission_required"};
        }
        if (!valid_supporting_artifact_bindings(binding)) {
            return {nullptr,
                    "polyglot.host.supporting_artifact_binding_required"};
        }
        if (binding.artifact_admission.capability_id() != binding.capability_id ||
            binding.candidate_request_template.invocation.capability_id !=
                binding.capability_id) {
            return {nullptr, "polyglot.host.capability_binding_mismatch"};
        }
        if (binding.candidate_request_template.invocation.correlation_id.empty()) {
            return {nullptr, "polyglot.host.correlation_prefix_required"};
        }
        if (!binding.candidate_request_template.invocation.arguments_json.empty() ||
            binding.candidate_request_template.cancellation_requested) {
            return {nullptr, "polyglot.host.mutable_request_binding_forbidden"};
        }
        auto probe = binding.candidate_request_template;
        probe.invocation.correlation_id += "-1";
        probe.invocation.arguments_json = "{}";
        if (!platform::serialize_polyglot_invocation_request(probe.invocation).ok() ||
            !platform::validate_polyglot_bridge_policy(probe.policy).ok() ||
            probe.policy.max_attempts != 1U) {
            return {nullptr, "polyglot.host.invalid_candidate_configuration"};
        }
        if (route_can_invoke_native(*route, probe.policy) && !binding.invoke_native) {
            return {nullptr, "polyglot.host.native_invoker_required"};
        }
        if (route->state == platform::PolyglotRouteState::shadow &&
            !binding.normalize_shadow_parity) {
            return {nullptr, "polyglot.host.shadow_normalizer_required"};
        }
        const std::string capability_id = binding.capability_id;
        if (!state->capabilities.emplace(
                capability_id,
                std::make_shared<State::Capability>(std::move(binding))).second) {
            return {nullptr, "polyglot.host.duplicate_capability_binding"};
        }
    }
    if (state->capabilities.size() != state->route_registry.entries.size()) {
        return {nullptr, "polyglot.host.capability_binding_required"};
    }

    auto host = std::shared_ptr<PolyglotRuntimeHost>(
        new PolyglotRuntimeHost(std::move(state)));
    return {std::move(host), {}};
}

std::function<RuntimePolyglotDispatchResult(
    const RuntimePolyglotDispatchRequest&)>
PolyglotRuntimeHost::dispatch_callback() const {
    const std::shared_ptr<State> state = state_;
    return [state](const RuntimePolyglotDispatchRequest& request) {
        const auto found = state->capabilities.find(request.capability_id);
        if (found == state->capabilities.end()) {
            return invalid_result("polyglot.host.capability_unavailable");
        }
        const std::shared_ptr<State::Capability>& capability = found->second;
        std::lock_guard<std::mutex> lock(capability->invocation_mutex);
        auto candidate = capability->binding.candidate_request_template;
        if (capability->correlation_sequence ==
            std::numeric_limits<std::uint64_t>::max()) {
            return invalid_result("polyglot.host.correlation_sequence_exhausted");
        }
        ++capability->correlation_sequence;
        candidate.invocation.correlation_id +=
            "-" + std::to_string(capability->correlation_sequence);
        candidate.invocation.arguments_json = request.arguments_json;
        candidate.cancellation_requested = request.cancellation_requested;

        platform::PolyglotRouteExecutionRequest execution;
        execution.registry = &state->route_registry;
        execution.capability_id = request.capability_id;
        execution.selection_sample = request.selection_sample;
        execution.artifact_admission = &capability->binding.artifact_admission;
        execution.supporting_artifact_admissions =
            capability->binding.supporting_artifact_admissions.empty()
            ? nullptr
            : &capability->binding.supporting_artifact_admissions;
        execution.supporting_artifact_arguments =
            capability->binding.supporting_artifact_arguments.empty()
            ? nullptr
            : &capability->binding.supporting_artifact_arguments;
        execution.candidate_request = std::move(candidate);
        execution.invoke_native = capability->binding.invoke_native;
        execution.normalize_shadow_parity =
            capability->binding.normalize_shadow_parity;
        execution.parity_policy = capability->binding.parity_policy;
        return map_result(platform::execute_polyglot_route(execution));
    };
}

}  // namespace copperfin::runtime

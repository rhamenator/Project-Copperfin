// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/platform/polyglot_route_execution.h"
#include "copperfin/runtime/prg_engine.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace copperfin::runtime {

// One explicit trusted host binding. The invocation template owns bridge,
// process, protocol, and bound settings; its capability must match both the
// executable admission, every supporting-file admission, and capability_id.
// Its correlation_id is a non-empty prefix and
// arguments_json/cancellation_requested must be empty because the host binds
// those values from each immutable PRG dispatch request.
struct PolyglotRuntimeCapabilityBinding {
    std::string capability_id;
    platform::PolyglotArtifactAdmissionResult artifact_admission;
    std::vector<platform::PolyglotSupportingArtifactAdmissionResult>
        supporting_artifact_admissions;
    std::vector<platform::PolyglotSupportingArtifactArgumentBinding>
        supporting_artifact_arguments;
    platform::PolyglotArtifactInvocationRequest candidate_request_template;
    platform::PolyglotNativeInvoker invoke_native;
    platform::PolyglotShadowParityNormalizer normalize_shadow_parity;
    platform::PolyglotParityPolicy parity_policy;
};

struct PolyglotRuntimeHostConfiguration {
    platform::PolyglotRouteRegistry route_registry;
    std::vector<PolyglotRuntimeCapabilityBinding> capabilities;
};

class PolyglotRuntimeHost;

struct PolyglotRuntimeHostBuildResult {
    std::shared_ptr<const PolyglotRuntimeHost> host;
    std::string error_code;

    [[nodiscard]] bool ok() const noexcept {
        return host != nullptr && error_code.empty();
    }
};

// Owns the validated route registry and admitted per-capability artifacts for
// the lifetime of every callback copied from it. A callback receives immutable
// PRG request data only and synchronously delegates exactly once to the
// existing route executor.
class PolyglotRuntimeHost {
public:
    [[nodiscard]] static PolyglotRuntimeHostBuildResult create(
        PolyglotRuntimeHostConfiguration configuration);

    [[nodiscard]] std::function<RuntimePolyglotDispatchResult(
        const RuntimePolyglotDispatchRequest&)> dispatch_callback() const;

private:
    struct State;
    explicit PolyglotRuntimeHost(std::shared_ptr<State> state);

    std::shared_ptr<State> state_;
};

}  // namespace copperfin::runtime

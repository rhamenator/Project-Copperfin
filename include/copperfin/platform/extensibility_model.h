// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <string>
#include <vector>

namespace copperfin::localization {
struct LocalizedCatalog;
}

namespace copperfin::platform {

enum class DotNetParityTier {
    exact,
    adapted,
    intentionally_not_supported
};

enum class DotNetInteropDecision {
    allow,
    fallback_native,
    reject
};

struct DotNetParityCapability {
    std::string id;
    std::string title;
    DotNetParityTier tier = DotNetParityTier::adapted;
    std::string rationale;
    std::string verification_reference;
    std::vector<std::string> reason_tags;
};

struct DotNetInteropPolicyRules {
    std::vector<std::string> allowlist;
    std::vector<std::string> denylist;
    std::size_t max_in_process_latency_budget_ms = 25U;
    bool require_policy_audit = true;
    bool allow_reflection_for_untrusted = false;
};

struct DotNetInteropCallRequest {
    std::string capability_id;
    std::size_t estimated_latency_ms = 0U;
    bool requires_reflection = false;
    bool untrusted_input = false;
    bool security_sensitive = false;
};

struct DotNetInteropCallDecision {
    DotNetInteropDecision decision = DotNetInteropDecision::fallback_native;
    std::string execution_path;
    std::string reason;
};

struct LanguageIntegration {
    std::string id;
    std::string title;
    std::string integration_mode;
    std::string trust_boundary;
    std::string output_story;
    bool enabled_by_default = false;
};

struct AiToolingFeature {
    std::string id;
    std::string title;
    std::string description;
    std::string trust_boundary;
    bool enabled_by_default = false;
};

struct DotNetOutputProfile {
    bool available = false;
    bool native_host_executables = false;
    bool managed_wrappers = false;
    bool nuget_sdk = false;
    std::string primary_story;
    DotNetInteropPolicyRules policy{};
    std::vector<DotNetParityCapability> parity_matrix;
};

struct ExtensibilityProfile {
    bool available = false;
    std::vector<LanguageIntegration> languages;
    std::vector<AiToolingFeature> ai_features;
    DotNetOutputProfile dotnet_output{};
    std::vector<std::string> guardrails;
};

[[nodiscard]] ExtensibilityProfile default_extensibility_profile();
[[nodiscard]] ExtensibilityProfile default_extensibility_profile(
    const localization::LocalizedCatalog& catalog);
[[nodiscard]] DotNetInteropCallDecision evaluate_dotnet_interop_call(
    const ExtensibilityProfile& profile,
    const DotNetInteropCallRequest& request);
[[nodiscard]] DotNetInteropCallDecision evaluate_dotnet_interop_call(
    const ExtensibilityProfile& profile,
    const DotNetInteropCallRequest& request,
    const localization::LocalizedCatalog& catalog);

}  // namespace copperfin::platform

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/platform/extensibility_model.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <filesystem>
#include <mutex>

namespace copperfin::platform {

namespace {

bool contains_case_sensitive(const std::vector<std::string>& values, const std::string& candidate) {
    return std::find(values.begin(), values.end(), candidate) != values.end();
}

localization::LocalizedCatalog extensibility_profile_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        localization::LocalizedCatalog catalog;
    };

    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        localization::load_catalogs(
            localization::resolve_catalog_root(),
            localization::default_locale)};
    const std::filesystem::path locale_root = localization::resolve_catalog_root();
    const std::string locale = localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

std::string extensibility_text(
    const localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

}  // namespace

ExtensibilityProfile default_extensibility_profile(const localization::LocalizedCatalog& catalog) {
    ExtensibilityProfile profile;
    profile.available = true;

    profile.languages = {
        {"xbase", extensibility_text(catalog, "Platform.Extensibility.Language.Xbase.Title"), extensibility_text(catalog, "Platform.Extensibility.Language.Xbase.IntegrationMode"), extensibility_text(catalog, "Platform.Extensibility.Language.Xbase.TrustBoundary"), extensibility_text(catalog, "Platform.Extensibility.Language.Xbase.OutputStory"), true},
        {"dotnet", extensibility_text(catalog, "Platform.Extensibility.Language.DotNet.Title"), extensibility_text(catalog, "Platform.Extensibility.Language.DotNet.IntegrationMode"), extensibility_text(catalog, "Platform.Extensibility.Language.DotNet.TrustBoundary"), extensibility_text(catalog, "Platform.Extensibility.Language.DotNet.OutputStory"), true},
        {"c-abi", extensibility_text(catalog, "Platform.Extensibility.Language.CAbi.Title"), extensibility_text(catalog, "Platform.Extensibility.Language.CAbi.IntegrationMode"), extensibility_text(catalog, "Platform.Extensibility.Language.CAbi.TrustBoundary"), extensibility_text(catalog, "Platform.Extensibility.Language.CAbi.OutputStory"), false},
        {"rust", extensibility_text(catalog, "Platform.Extensibility.Language.Rust.Title"), extensibility_text(catalog, "Platform.Extensibility.Language.Rust.IntegrationMode"), extensibility_text(catalog, "Platform.Extensibility.Language.Rust.TrustBoundary"), extensibility_text(catalog, "Platform.Extensibility.Language.Rust.OutputStory"), false},
        {"python", extensibility_text(catalog, "Platform.Extensibility.Language.Python.Title"), extensibility_text(catalog, "Platform.Extensibility.Language.Python.IntegrationMode"), extensibility_text(catalog, "Platform.Extensibility.Language.Python.TrustBoundary"), extensibility_text(catalog, "Platform.Extensibility.Language.Python.OutputStory"), false},
        {"r", extensibility_text(catalog, "Platform.Extensibility.Language.R.Title"), extensibility_text(catalog, "Platform.Extensibility.Language.R.IntegrationMode"), extensibility_text(catalog, "Platform.Extensibility.Language.R.TrustBoundary"), extensibility_text(catalog, "Platform.Extensibility.Language.R.OutputStory"), false}
    };

    profile.ai_features = {
        {"mcp-host", extensibility_text(catalog, "Platform.Extensibility.AiFeature.McpHost.Title"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.McpHost.Description"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.McpHost.TrustBoundary"), false},
        {"ai-assist", extensibility_text(catalog, "Platform.Extensibility.AiFeature.AiAssist.Title"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.AiAssist.Description"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.AiAssist.TrustBoundary"), false},
        {"local-models", extensibility_text(catalog, "Platform.Extensibility.AiFeature.LocalModels.Title"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.LocalModels.Description"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.LocalModels.TrustBoundary"), false},
        {"model-selection", extensibility_text(catalog, "Platform.Extensibility.AiFeature.ModelSelection.Title"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.ModelSelection.Description"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.ModelSelection.TrustBoundary"), false},
        {"ai-debug-assist", extensibility_text(catalog, "Platform.Extensibility.AiFeature.AiDebugAssist.Title"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.AiDebugAssist.Description"), extensibility_text(catalog, "Platform.Extensibility.AiFeature.AiDebugAssist.TrustBoundary"), false}
    };

    profile.dotnet_output.available = true;
    profile.dotnet_output.native_host_executables = true;
    profile.dotnet_output.managed_wrappers = true;
    profile.dotnet_output.nuget_sdk = true;
    profile.dotnet_output.primary_story = extensibility_text(catalog, "Platform.Extensibility.DotNetOutput.PrimaryStory");
    profile.dotnet_output.policy.allowlist = {
        "task-primitives",
        "json-helpers",
        "regex-helpers",
        "safe-http-helpers",
        "crypto-safe-helpers"
    };
    profile.dotnet_output.policy.denylist = {
        "unsafe-reflection-load",
        "insecure-binary-deserialization",
        "legacy-cas-interop"
    };
    profile.dotnet_output.policy.max_in_process_latency_budget_ms = 25U;
    profile.dotnet_output.policy.require_policy_audit = true;
    profile.dotnet_output.policy.allow_reflection_for_untrusted = false;
    profile.dotnet_output.parity_matrix = {
        DotNetParityCapability{
            .id = "task-primitives",
            .title = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.TaskPrimitives.Title"),
            .tier = DotNetParityTier::adapted,
            .rationale = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.TaskPrimitives.Rationale"),
            .verification_reference = "#272",
            .reason_tags = {"ergonomics", "performance"}},
        DotNetParityCapability{
            .id = "json-helpers",
            .title = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.JsonHelpers.Title"),
            .tier = DotNetParityTier::adapted,
            .rationale = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.JsonHelpers.Rationale"),
            .verification_reference = "#280",
            .reason_tags = {"ergonomics"}},
        DotNetParityCapability{
            .id = "unsafe-reflection-load",
            .title = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.UnsafeReflectionLoad.Title"),
            .tier = DotNetParityTier::intentionally_not_supported,
            .rationale = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.UnsafeReflectionLoad.Rationale"),
            .verification_reference = "#279",
            .reason_tags = {"security", "legacy_hazard"}},
        DotNetParityCapability{
            .id = "insecure-binary-deserialization",
            .title = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.InsecureBinaryDeserialization.Title"),
            .tier = DotNetParityTier::intentionally_not_supported,
            .rationale = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.InsecureBinaryDeserialization.Rationale"),
            .verification_reference = "#279",
            .reason_tags = {"security", "legacy_hazard"}},
        DotNetParityCapability{
            .id = "legacy-cas-interop",
            .title = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.LegacyCasInterop.Title"),
            .tier = DotNetParityTier::intentionally_not_supported,
            .rationale = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.LegacyCasInterop.Rationale"),
            .verification_reference = "#275",
            .reason_tags = {"performance", "legacy_hazard"}}
    };

    profile.guardrails = {
        extensibility_text(catalog, "Platform.Extensibility.Guardrail.NativeFirstCore"),
        extensibility_text(catalog, "Platform.Extensibility.Guardrail.DotNetPolicyControlled"),
        extensibility_text(catalog, "Platform.Extensibility.Guardrail.SidecarLanguages"),
        extensibility_text(catalog, "Platform.Extensibility.Guardrail.AiToolingOptIn"),
        extensibility_text(catalog, "Platform.Extensibility.Guardrail.ReleaseOutputDotNetStory")
    };

    return profile;
}

ExtensibilityProfile default_extensibility_profile() {
    return default_extensibility_profile(extensibility_profile_catalog());
}

DotNetInteropCallDecision evaluate_dotnet_interop_call(
    const ExtensibilityProfile& profile,
    const DotNetInteropCallRequest& request) {
    return evaluate_dotnet_interop_call(profile, request, extensibility_profile_catalog());
}

DotNetInteropCallDecision evaluate_dotnet_interop_call(
    const ExtensibilityProfile& profile,
    const DotNetInteropCallRequest& request,
    const localization::LocalizedCatalog& catalog) {
    DotNetInteropCallDecision decision;

    if (!profile.dotnet_output.available) {
        decision.decision = DotNetInteropDecision::fallback_native;
        decision.execution_path = "native";
        decision.reason = extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.DotNetProfileUnavailable");
        return decision;
    }

    if (request.capability_id.empty()) {
        decision.decision = DotNetInteropDecision::reject;
        decision.execution_path = "none";
        decision.reason = extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityIdRequired");
        return decision;
    }

    const DotNetInteropPolicyRules& rules = profile.dotnet_output.policy;
    if (contains_case_sensitive(rules.denylist, request.capability_id)) {
        decision.decision = DotNetInteropDecision::reject;
        decision.execution_path = "none";
        decision.reason = extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityDeniedByPolicy");
        return decision;
    }

    if (!rules.allowlist.empty() && !contains_case_sensitive(rules.allowlist, request.capability_id)) {
        decision.decision = DotNetInteropDecision::fallback_native;
        decision.execution_path = "native";
        decision.reason = extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityNotInAllowlist");
        return decision;
    }

    if (request.security_sensitive && request.untrusted_input && request.requires_reflection && !rules.allow_reflection_for_untrusted) {
        decision.decision = DotNetInteropDecision::reject;
        decision.execution_path = "none";
        decision.reason = extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.ReflectionOnUntrustedInputDenied");
        return decision;
    }

    if (request.estimated_latency_ms > rules.max_in_process_latency_budget_ms) {
        decision.decision = DotNetInteropDecision::fallback_native;
        decision.execution_path = "native";
        decision.reason = extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.EstimatedLatencyExceedsBudget");
        return decision;
    }

    decision.decision = DotNetInteropDecision::allow;
    decision.execution_path = "dotnet";
    decision.reason = rules.require_policy_audit
        ? extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityAllowedWithAuditRequired")
        : extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityAllowedByPolicy");
    return decision;
}

}  // namespace copperfin::platform

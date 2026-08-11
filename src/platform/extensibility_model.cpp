// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/extensibility_model.h"

#include "copperfin/localization/localization.h"
#include "copperfin/platform/json.h"

#include <algorithm>
#include <filesystem>
#include <mutex>
#include <sstream>

namespace copperfin::platform {

namespace {

bool contains_case_sensitive(const std::vector<std::string>& values, const std::string& candidate) {
    return std::find(values.begin(), values.end(), candidate) != values.end();
}

std::string decision_name(DotNetInteropDecision decision) {
    switch (decision) {
        case DotNetInteropDecision::allow:
            return "allow";
        case DotNetInteropDecision::fallback_native:
            return "fallback_native";
        case DotNetInteropDecision::reject:
            return "reject";
    }
    return "reject";
}

std::string audit_json_escape(std::string_view value) {
    const std::string escaped = json_escape_string(value);
    std::string result;
    result.reserve(escaped.size());
    for (const char ch : escaped) {
        if (ch == '|') {
            result += "\\u007c";
        } else {
            result.push_back(ch);
        }
    }
    return result;
}

DotNetInteropCallDecision make_decision(
    const DotNetInteropCallRequest& request,
    DotNetInteropDecision value,
    std::string execution_path,
    std::string reason,
    std::string diagnostic_code,
    bool audit_required) {
    DotNetInteropCallDecision result;
    result.decision = value;
    result.execution_path = std::move(execution_path);
    result.reason = std::move(reason);
    result.diagnostic_code = std::move(diagnostic_code);
    result.audit_event = DotNetInteropAuditEvent{
        .actor_id = request.actor_id,
        .capability_id = request.capability_id,
        .decision = value,
        .outcome = decision_name(value),
        .diagnostic_code = result.diagnostic_code};
    result.audit_commit_required = audit_required;
    return result;
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
        "collection-helpers",
        "crypto-safe-helpers"
    };
    profile.dotnet_output.policy.denylist = {
        "safe-http-helpers",
        "unsafe-reflection-load",
        "insecure-binary-deserialization",
        "legacy-cas-interop"
    };
    profile.dotnet_output.policy.reflection_allowlist = {};
    profile.dotnet_output.policy.assembly_loading_allowlist = {};
    profile.dotnet_output.policy.external_io_allowlist = {};
    profile.dotnet_output.policy.secret_access_allowlist = {};
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
            .id = "regex-helpers",
            .title = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.RegexHelpers.Title"),
            .tier = DotNetParityTier::adapted,
            .rationale = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.RegexHelpers.Rationale"),
            .verification_reference = "#280",
            .reason_tags = {"ergonomics", "security"}},
        DotNetParityCapability{
            .id = "collection-helpers",
            .title = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.CollectionHelpers.Title"),
            .tier = DotNetParityTier::adapted,
            .rationale = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.CollectionHelpers.Rationale"),
            .verification_reference = "#280",
            .reason_tags = {"ergonomics", "compatibility"}},
        DotNetParityCapability{
            .id = "crypto-safe-helpers",
            .title = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.CryptoSafeHelpers.Title"),
            .tier = DotNetParityTier::adapted,
            .rationale = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.CryptoSafeHelpers.Rationale"),
            .verification_reference = "#280",
            .reason_tags = {"ergonomics", "security"}},
        DotNetParityCapability{
            .id = "safe-http-helpers",
            .title = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.SafeHttpHelpers.Title"),
            .tier = DotNetParityTier::intentionally_not_supported,
            .rationale = extensibility_text(catalog, "Platform.Extensibility.DotNetParity.SafeHttpHelpers.Rationale"),
            .verification_reference = "#280",
            .reason_tags = {"security", "external_io"}},
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
    if (!profile.dotnet_output.available) {
        return make_decision(request, DotNetInteropDecision::fallback_native, "native",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.DotNetProfileUnavailable"),
            "dotnet.interop.profile_unavailable", false);
    }

    if (request.capability_id.empty()) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityIdRequired"),
            "dotnet.interop.capability_required", true);
    }

    const DotNetInteropPolicyRules& rules = profile.dotnet_output.policy;
    if (request.actor_id.empty()) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.ActorIdRequired"),
            "dotnet.interop.actor_required", rules.require_policy_audit);
    }
    if (!request.policy_context_verified) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.PolicyContextUnverified"),
            "dotnet.interop.policy_context_unverified", rules.require_policy_audit);
    }
    if (rules.require_policy_audit && !request.audit_sink_available) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.AuditSinkUnavailable"),
            "dotnet.interop.audit_unavailable", true);
    }
    if (!contains_case_sensitive(request.granted_capabilities, request.capability_id)) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityOutsideActorScope"),
            "dotnet.interop.capability_scope_denied", rules.require_policy_audit);
    }
    if (contains_case_sensitive(rules.denylist, request.capability_id)) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityDeniedByPolicy"),
            "dotnet.interop.capability_denied", rules.require_policy_audit);
    }

    if (!rules.allowlist.empty() && !contains_case_sensitive(rules.allowlist, request.capability_id)) {
        return make_decision(request, DotNetInteropDecision::fallback_native, "native",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityNotInAllowlist"),
            "dotnet.interop.capability_not_allowlisted", rules.require_policy_audit);
    }

    if (request.requires_reflection &&
        (!contains_case_sensitive(rules.reflection_allowlist, request.capability_id) ||
         (request.untrusted_input && !rules.allow_reflection_for_untrusted))) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.ReflectionOnUntrustedInputDenied"),
            "dotnet.interop.reflection_denied", rules.require_policy_audit);
    }
    if (request.requires_assembly_loading &&
        !contains_case_sensitive(rules.assembly_loading_allowlist, request.capability_id)) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.AssemblyLoadingDenied"),
            "dotnet.interop.assembly_loading_denied", rules.require_policy_audit);
    }
    if (request.requires_external_io &&
        !contains_case_sensitive(rules.external_io_allowlist, request.capability_id)) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.ExternalIoDenied"),
            "dotnet.interop.external_io_denied", rules.require_policy_audit);
    }
    if (request.requires_secret_access &&
        !contains_case_sensitive(rules.secret_access_allowlist, request.capability_id)) {
        return make_decision(request, DotNetInteropDecision::reject, "none",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.SecretAccessDenied"),
            "dotnet.interop.secret_access_denied", rules.require_policy_audit);
    }

    if (request.estimated_latency_ms > rules.max_in_process_latency_budget_ms) {
        return make_decision(request, DotNetInteropDecision::fallback_native, "native",
            extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.EstimatedLatencyExceedsBudget"),
            "dotnet.interop.latency_budget_exceeded", rules.require_policy_audit);
    }

    return make_decision(request, DotNetInteropDecision::allow,
        rules.require_policy_audit ? "pending_audit" : "dotnet",
        rules.require_policy_audit
            ? extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityAllowedWithAuditRequired")
            : extensibility_text(catalog, "Platform.Extensibility.DotNetInteropDecision.CapabilityAllowedByPolicy"),
        "dotnet.interop.allowed", rules.require_policy_audit);
}

DotNetInteropCallDecision evaluate_and_commit_dotnet_interop_call(
    const ExtensibilityProfile& profile,
    const DotNetInteropCallRequest& request,
    const DotNetInteropAuditSink& sink) {
    return evaluate_and_commit_dotnet_interop_call(
        profile,
        request,
        sink,
        extensibility_profile_catalog());
}

DotNetInteropCallDecision evaluate_and_commit_dotnet_interop_call(
    const ExtensibilityProfile& profile,
    const DotNetInteropCallRequest& request,
    const DotNetInteropAuditSink& sink,
    const localization::LocalizedCatalog& catalog) {
    DotNetInteropCallDecision decision =
        evaluate_dotnet_interop_call(profile, request, catalog);
    if (!decision.audit_commit_required) {
        return decision;
    }

    DotNetInteropAuditCommitResult commit_result;
    if (sink.commit != nullptr) {
        try {
            commit_result = sink.commit(decision.audit_event, sink.context);
        } catch (...) {
            commit_result = {};
        }
    }
    if (!commit_result.ok || commit_result.receipt.empty()) {
        return make_decision(
            request,
            DotNetInteropDecision::reject,
            "none",
            extensibility_text(
                catalog,
                "Platform.Extensibility.DotNetInteropDecision.AuditCommitFailed"),
            "dotnet.interop.audit_commit_failed",
            true);
    }

    decision.audit_committed = true;
    decision.audit_receipt = std::move(commit_result.receipt);
    if (decision.decision == DotNetInteropDecision::allow) {
        decision.execution_path = "dotnet";
    }
    return decision;
}

std::string serialize_dotnet_interop_audit_event(const DotNetInteropAuditEvent& event) {
    std::ostringstream stream;
    stream << "{\"schema_version\":1,\"actor\":\"" << audit_json_escape(event.actor_id)
           << "\",\"capability\":\"" << audit_json_escape(event.capability_id)
           << "\",\"decision\":\"" << audit_json_escape(decision_name(event.decision))
           << "\",\"outcome\":\"" << audit_json_escape(event.outcome)
           << "\",\"diagnostic_code\":\"" << audit_json_escape(event.diagnostic_code) << "\"}";
    return stream.str();
}

}  // namespace copperfin::platform

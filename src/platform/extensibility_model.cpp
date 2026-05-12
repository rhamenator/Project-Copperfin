#include "copperfin/platform/extensibility_model.h"

#include <algorithm>

namespace copperfin::platform {

namespace {

bool contains_case_sensitive(const std::vector<std::string>& values, const std::string& candidate) {
    return std::find(values.begin(), values.end(), candidate) != values.end();
}

}  // namespace

ExtensibilityProfile default_extensibility_profile() {
    ExtensibilityProfile profile;
    profile.available = true;

    profile.languages = {
        {"xbase", "Native Copperfin/xBase Runtime", "native", "trusted core", "Primary runtime and build target for FoxPro-style applications.", true},
        {"dotnet", ".NET Managed Components", "hosted interop", "policy-managed interop", "Managed assemblies can be called from Copperfin and Copperfin logic can be surfaced as .NET-consumable outputs.", true},
        {"c-abi", "Native C ABI Modules", "binary plugin", "signed plugin boundary", "Performance-sensitive extensions can be linked or loaded through stable native interfaces.", false},
        {"rust", "Rust Native Components", "native library", "signed plugin boundary", "Rust is acceptable for safety-sensitive helpers behind stable native interfaces.", false},
        {"python", "Python Sidecar And Analytics Jobs", "out-of-process sidecar", "restricted external process boundary", "Python support is positioned as a sidecar or job facility for data science and automation, not the trusted core.", false},
        {"r", "R Analytics And Statistical Jobs", "out-of-process sidecar", "restricted external process boundary", "R support is positioned as a sidecar or job facility for statistical computing, reporting, and reproducible data-science workflows.", false}
    };

    profile.ai_features = {
        {"mcp-host", "MCP Host Facility", "Expose Copperfin tools through an MCP host so developers can use preferred AI models and assistants.", "policy-managed external tool boundary", false},
        {"ai-assist", "AI-Assisted Developer Workflow", "Optional vibe-coding and code-intelligence helpers for designers, migration, and diagnostics.", "policy-managed external tool boundary", false},
        {"local-models", "Local Or Enterprise AI Backends", "Use local or enterprise-approved models rather than forcing one hosted provider.", "policy-managed external tool boundary", false},
        {"model-selection", "User-Selected AI Models", "Let developers choose the model used for AI debugging and assistance instead of hard-coding one provider or model family.", "policy-managed external tool boundary", false},
        {"ai-debug-assist", "AI Debugging Assistance", "Allow developers to send debugger context, stack traces, and runtime telemetry to an approved assistant workflow for optional debugging help.", "policy-managed external tool boundary", false}
    };

    profile.dotnet_output.available = true;
    profile.dotnet_output.native_host_executables = true;
    profile.dotnet_output.managed_wrappers = true;
    profile.dotnet_output.nuget_sdk = true;
    profile.dotnet_output.primary_story =
        "Copperfin applications should be able to ship as native executables with first-class .NET compatibility, and selected modules should be exposable as managed wrappers or NuGet-consumable SDK outputs.";
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
            .title = "Task/Async primitives",
            .tier = DotNetParityTier::adapted,
            .rationale = "Expose async/await-style behavior through FP/VFP-friendly command and function facades.",
            .verification_reference = "#272",
            .reason_tags = {"ergonomics", "performance"}},
        DotNetParityCapability{
            .id = "json-helpers",
            .title = "JSON projection helpers",
            .tier = DotNetParityTier::adapted,
            .rationale = "Provide high-value JSON conversion features while preserving native null/blank semantics.",
            .verification_reference = "#280",
            .reason_tags = {"ergonomics"}},
        DotNetParityCapability{
            .id = "unsafe-reflection-load",
            .title = "Arbitrary reflection-based assembly loading",
            .tier = DotNetParityTier::intentionally_not_supported,
            .rationale = "Rejected due to trust-boundary and policy-audit risks.",
            .verification_reference = "#279",
            .reason_tags = {"security", "legacy_hazard"}},
        DotNetParityCapability{
            .id = "insecure-binary-deserialization",
            .title = "Legacy insecure binary deserialization flows",
            .tier = DotNetParityTier::intentionally_not_supported,
            .rationale = "Rejected due to known unsafe behavior and exploit history.",
            .verification_reference = "#279",
            .reason_tags = {"security", "legacy_hazard"}},
        DotNetParityCapability{
            .id = "legacy-cas-interop",
            .title = "Code Access Security-era behavior emulation",
            .tier = DotNetParityTier::intentionally_not_supported,
            .rationale = "Rejected because emulating retired CAS behavior adds complexity with little user value.",
            .verification_reference = "#275",
            .reason_tags = {"performance", "legacy_hazard"}}
    };

    profile.guardrails = {
        "The trusted execution core stays native-first and security-first.",
        ".NET interop is first-class, but managed loading is policy-controlled and auditable.",
        "Python and R are supported through sidecars or job runners, not as the product core.",
        "MCP and AI tooling are opt-in developer features with audit, provider policy, and user-selected model controls.",
        "Release outputs must preserve a clear .NET consumption story even when the executable is native."
    };

    return profile;
}

DotNetInteropCallDecision evaluate_dotnet_interop_call(
    const ExtensibilityProfile& profile,
    const DotNetInteropCallRequest& request) {
    DotNetInteropCallDecision decision;

    if (!profile.dotnet_output.available) {
        decision.decision = DotNetInteropDecision::fallback_native;
        decision.execution_path = "native";
        decision.reason = "dotnet output profile unavailable";
        return decision;
    }

    if (request.capability_id.empty()) {
        decision.decision = DotNetInteropDecision::reject;
        decision.execution_path = "none";
        decision.reason = "capability id is required";
        return decision;
    }

    const DotNetInteropPolicyRules& rules = profile.dotnet_output.policy;
    if (contains_case_sensitive(rules.denylist, request.capability_id)) {
        decision.decision = DotNetInteropDecision::reject;
        decision.execution_path = "none";
        decision.reason = "capability denied by policy allowlist/denylist";
        return decision;
    }

    if (!rules.allowlist.empty() && !contains_case_sensitive(rules.allowlist, request.capability_id)) {
        decision.decision = DotNetInteropDecision::fallback_native;
        decision.execution_path = "native";
        decision.reason = "capability not in allowlist";
        return decision;
    }

    if (request.security_sensitive && request.untrusted_input && request.requires_reflection && !rules.allow_reflection_for_untrusted) {
        decision.decision = DotNetInteropDecision::reject;
        decision.execution_path = "none";
        decision.reason = "reflection on untrusted input is blocked";
        return decision;
    }

    if (request.estimated_latency_ms > rules.max_in_process_latency_budget_ms) {
        decision.decision = DotNetInteropDecision::fallback_native;
        decision.execution_path = "native";
        decision.reason = "estimated latency exceeds in-process policy budget";
        return decision;
    }

    decision.decision = DotNetInteropDecision::allow;
    decision.execution_path = "dotnet";
    decision.reason = rules.require_policy_audit
        ? "allowed by policy with audit-required path"
        : "allowed by policy";
    return decision;
}

}  // namespace copperfin::platform

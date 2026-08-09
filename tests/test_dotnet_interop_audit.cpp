// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/extensibility_model.h"
#include "copperfin/security/audit_stream.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

struct FileAuditContext {
    std::string log_path;
    std::string package_root;
    std::size_t calls = 0U;
};

copperfin::platform::DotNetInteropAuditCommitResult commit_to_contained_audit(
    const copperfin::platform::DotNetInteropAuditEvent& event,
    void* opaque_context) {
    auto* context = static_cast<FileAuditContext*>(opaque_context);
    if (context == nullptr) {
        return {};
    }
    ++context->calls;
    const std::string event_name =
        event.decision == copperfin::platform::DotNetInteropDecision::reject
        ? "policy.denied"
        : "interop.dotnet_invoked";
    const auto append = copperfin::security::append_immutable_audit_event_to_contained_file(
        context->log_path,
        context->package_root,
        event_name,
        copperfin::platform::serialize_dotnet_interop_audit_event(event));
    return {
        .ok = append.ok,
        .receipt = append.entry_hash};
}

copperfin::platform::DotNetInteropAuditCommitResult fail_commit(
    const copperfin::platform::DotNetInteropAuditEvent&,
    void*) {
    return {};
}

copperfin::platform::DotNetInteropAuditCommitResult empty_receipt_commit(
    const copperfin::platform::DotNetInteropAuditEvent&,
    void*) {
    return {.ok = true, .receipt = {}};
}

copperfin::platform::DotNetInteropAuditCommitResult throw_commit(
    const copperfin::platform::DotNetInteropAuditEvent&,
    void*) {
    throw std::runtime_error("synthetic audit sink failure");
}

copperfin::platform::DotNetInteropCallRequest authorized_request(
    std::string capability_id) {
    copperfin::platform::DotNetInteropCallRequest request;
    request.capability_id = std::move(capability_id);
    request.actor_id = "prg:main";
    request.granted_capabilities = {request.capability_id};
    request.policy_context_verified = true;
    request.audit_sink_available = true;
    return request;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

void test_dotnet_audit_commit_boundary() {
    namespace fs = std::filesystem;
    const fs::path root =
        fs::temp_directory_path() / "copperfin_dotnet_interop_audit_contract";
    std::error_code error;
    fs::remove_all(root, error);
    error.clear();
    fs::create_directories(root, error);
    expect(!error, "#279: audit test root should be created");
    const fs::path log_path = root / "audit" / "interop.log";

    const auto profile = copperfin::platform::default_extensibility_profile();
    const auto request = authorized_request("task-primitives");
    const auto policy_only =
        copperfin::platform::evaluate_dotnet_interop_call(profile, request);
    expect(policy_only.decision == copperfin::platform::DotNetInteropDecision::allow &&
               policy_only.execution_path == "pending_audit" &&
               policy_only.audit_commit_required &&
               !policy_only.audit_committed,
           "#279: policy evaluation must withhold the executable route until audit commit");

    FileAuditContext context{
        .log_path = log_path.string(),
        .package_root = root.string()};
    const copperfin::platform::DotNetInteropAuditSink file_sink{
        .commit = commit_to_contained_audit,
        .context = &context};
    const auto committed = copperfin::platform::evaluate_and_commit_dotnet_interop_call(
        profile,
        request,
        file_sink);
    expect(committed.decision == copperfin::platform::DotNetInteropDecision::allow &&
               committed.execution_path == "dotnet" &&
               committed.audit_committed &&
               committed.audit_receipt.size() == 64U &&
               context.calls == 1U,
           "#279: durable audit commit should promote an allowed call to the .NET route");
    const auto first_chain = copperfin::security::verify_immutable_audit_chain(log_path.string());
    expect(first_chain.ok && first_chain.entries == 1U,
           "#279: committed .NET allow should create one valid hash-chained audit entry");
    const std::string first_log = read_text(log_path);
    expect(first_log.find("interop.dotnet_invoked") != std::string::npos &&
               first_log.find("\"actor\":\"prg:main\"") != std::string::npos &&
               first_log.find("\"capability\":\"task-primitives\"") != std::string::npos &&
               first_log.find("\"decision\":\"allow\"") != std::string::npos &&
               first_log.find("\"outcome\":\"allow\"") != std::string::npos,
           "#279: durable audit detail should retain actor, capability, decision, and outcome");

    auto delimited_actor_request = request;
    delimited_actor_request.actor_id = "prg:main|batch";
    const auto delimited_actor_commit =
        copperfin::platform::evaluate_and_commit_dotnet_interop_call(
            profile,
            delimited_actor_request,
            file_sink);
    const std::string delimited_actor_log = read_text(log_path);
    const auto delimited_actor_chain =
        copperfin::security::verify_immutable_audit_chain(log_path.string());
    expect(delimited_actor_commit.audit_committed &&
               delimited_actor_chain.ok &&
               delimited_actor_chain.entries == 2U &&
               delimited_actor_log.find("\"actor\":\"prg:main\\u007cbatch\"") !=
                   std::string::npos,
           "#279: a field delimiter in an actor ID must remain lossless JSON in the durable stream");

    for (const auto& sink : {
             copperfin::platform::DotNetInteropAuditSink{},
             copperfin::platform::DotNetInteropAuditSink{.commit = fail_commit},
             copperfin::platform::DotNetInteropAuditSink{.commit = empty_receipt_commit},
             copperfin::platform::DotNetInteropAuditSink{.commit = throw_commit}}) {
        const auto rejected = copperfin::platform::evaluate_and_commit_dotnet_interop_call(
            profile,
            request,
            sink);
        expect(rejected.decision == copperfin::platform::DotNetInteropDecision::reject &&
                   rejected.execution_path == "none" &&
                   rejected.diagnostic_code == "dotnet.interop.audit_commit_failed" &&
                   rejected.reason == "required policy audit event could not be committed" &&
                   !rejected.audit_committed &&
                   rejected.audit_receipt.empty(),
               "#279: absent, failed, empty-receipt, and throwing sinks must fail closed");
    }

    auto denied_request = authorized_request("unsafe-reflection-load");
    denied_request.requires_reflection = true;
    denied_request.untrusted_input = true;
    denied_request.security_sensitive = true;
    const auto committed_denial =
        copperfin::platform::evaluate_and_commit_dotnet_interop_call(
            profile,
            denied_request,
            file_sink);
    expect(committed_denial.decision == copperfin::platform::DotNetInteropDecision::reject &&
               committed_denial.execution_path == "none" &&
               committed_denial.audit_committed &&
               committed_denial.audit_receipt.size() == 64U &&
               context.calls == 3U,
           "#279: policy denials should be durably audited without becoming executable");
    const auto second_chain = copperfin::security::verify_immutable_audit_chain(log_path.string());
    expect(second_chain.ok && second_chain.entries == 3U,
           "#279: allowed and denied decisions should share one valid audit chain");
    expect(read_text(log_path).find("policy.denied") != std::string::npos,
           "#279: denied decisions should use the policy denial event class");

    auto unaudited_profile = profile;
    unaudited_profile.dotnet_output.policy.require_policy_audit = false;
    const auto unaudited = copperfin::platform::evaluate_and_commit_dotnet_interop_call(
        unaudited_profile,
        request,
        {});
    expect(unaudited.decision == copperfin::platform::DotNetInteropDecision::allow &&
               unaudited.execution_path == "dotnet" &&
               !unaudited.audit_commit_required &&
               !unaudited.audit_committed &&
               context.calls == 3U,
           "#279: explicitly unaudited policy should not invoke a sink or require a receipt");

    fs::remove_all(root, error);
}

}  // namespace

int main() {
    test_dotnet_audit_commit_boundary();
    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

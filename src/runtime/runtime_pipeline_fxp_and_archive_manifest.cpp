// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"

#include <locale>

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

namespace {

struct StagedContentFile {
    std::string relative_path;
    std::string absolute_path;
    bool declared_asset = false;
};

std::string staged_content_relative_path(
    const std::filesystem::path& content_root,
    const std::filesystem::path& content_path) {
    return copperfin::platform::path_to_utf8_string(
        content_path.lexically_relative(content_root));
}

std::map<std::string, StagedContentFile> collect_staged_content_files(
    const RuntimePackagePlan& plan,
    std::string* error = nullptr) {
    std::map<std::string, StagedContentFile> files;
    const std::filesystem::path content_root =
        copperfin::platform::path_from_utf8_string(plan.content_root);

    for (const auto& asset : plan.assets) {
        if (!asset.copied || trim_copy(asset.staged_path).empty()) {
            continue;
        }

        files[asset.relative_path] = {
            .relative_path = asset.relative_path,
            .absolute_path = asset.staged_path,
            .declared_asset = true
        };
    }

#if !defined(_WIN32)
    bool fd_handled = false;
    std::vector<std::filesystem::path> fd_relative_files;
    if (!try_collect_fd_backed_regular_files(
            content_root,
            fd_handled,
            fd_relative_files)) {
        if (error != nullptr) {
            *error = runtime_text(
                "Runtime.Package.Error.OpenFileFailed",
                {{"path", copperfin::platform::path_to_utf8_string(content_root)}});
        }
        return files;
    }
    if (fd_handled) {
        for (const auto& relative_path : fd_relative_files) {
            const std::string relative =
                copperfin::platform::path_to_utf8_string(relative_path);
            files.emplace(relative, StagedContentFile{
                .relative_path = relative,
                .absolute_path = copperfin::platform::path_to_utf8_string(
                    content_root / relative_path),
                .declared_asset = false
            });
        }
        return files;
    }
#endif

    std::error_code filesystem_error;
    if (content_root.empty() || !std::filesystem::exists(content_root, filesystem_error)) {
        return files;
    }

    for (std::filesystem::recursive_directory_iterator it(content_root, filesystem_error), end;
         it != end && !filesystem_error;
         it.increment(filesystem_error)) {
        if (filesystem_error || !it->is_regular_file(filesystem_error)) {
            continue;
        }

        const std::string relative_path = staged_content_relative_path(content_root, it->path());
        if (relative_path.empty()) {
            continue;
        }

        auto [found, inserted] = files.emplace(relative_path, StagedContentFile{
            .relative_path = relative_path,
            .absolute_path = copperfin::platform::path_to_utf8_string(it->path()),
            .declared_asset = false
        });
        if (!inserted) {
            found->second.absolute_path = copperfin::platform::path_to_utf8_string(it->path());
        }
    }

    return files;
}

}  // namespace

void append_fxp_statement_lines(
    std::ostringstream& stream,
    const std::string& scope_name,
    const std::vector<Statement>& statements) {
    for (const auto& statement : statements) {
        stream << "statement="
               << quote_manifest_value(scope_name) << "|"
               << statement.location.line << "|"
               << quote_manifest_value(statement.text) << "\n";
    }
}

std::string build_fxp_token_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << "manifest_version=1\n";
    stream << "output_kind=fxp\n";
    stream << "token_contract=logical_statements\n";
    stream << "primary_output=" << quote_manifest_value(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path).filename())) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    for (const auto& asset : plan.assets) {
        if (!should_stage_asset(asset) ||
            lowercase_copy(trim_copy(copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(asset.source_path).extension()))) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        stream << "program=" << quote_manifest_value(asset.relative_path) << "\n";
        append_fxp_statement_lines(stream, "MAIN", program.main.statements);
        for (const auto& routine_entry : program.routines) {
            append_fxp_statement_lines(stream, routine_entry.first, routine_entry.second.statements);
        }
    }
    return stream.str();
}

bool write_fxp_primary_output_contract(
    const RuntimePackagePlan& plan,
    const std::string& token_manifest_text,
    const std::string& output_path,
    std::string& error) {
    std::ostringstream stream;
    stream << "copperfin_fxp_contract_version=1\n";
    stream << "token_contract=copperfin_logical_statement_contract_v1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "token_manifest=" << quote_manifest_value(plan.fxp_token_manifest_path) << "\n";
    stream << token_manifest_text;
    return write_text_file(
        copperfin::platform::path_from_utf8_string(output_path),
        stream.str(),
        error);
}

std::string build_app_archive_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "manifest_version=1\n";
    stream << "output_kind=app\n";
    stream << "archive_contract=staged_content_manifest\n";
    stream << "primary_output=" << quote_manifest_value(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path).filename())) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    for (const auto& asset : plan.assets) {
        stream << "asset="
               << quote_manifest_value(asset.relative_path) << "|"
               << quote_manifest_value(asset.type_title) << "|"
               << (asset.required_for_runtime ? "true" : "false") << "|"
               << (asset.copied ? "true" : "false") << "\n";
    }
    for (const auto& [relative_path, file] : collect_staged_content_files(plan)) {
        stream << "content_file="
               << quote_manifest_value(relative_path) << "|"
               << (file.declared_asset ? "declared_asset" : "companion") << "\n";
    }
    return stream.str();
}

bool write_app_archive_primary_output(
    const RuntimePackagePlan& plan,
    const RuntimePackagePlan& filesystem_plan,
    std::string& error) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << "copperfin_app_archive_version=1\n";
    stream << "archive_contract=copperfin_content_archive_v1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "content_manifest=" << quote_manifest_value(plan.app_archive_manifest_path) << "\n";

    const auto files = collect_staged_content_files(filesystem_plan, &error);
    if (!error.empty()) {
        return false;
    }
    for (const auto& [relative_path, file] : files) {
        error.clear();
        const std::string bytes = read_binary_file(file.absolute_path, error);
        if (!error.empty()) {
            return false;
        }

        const auto digest = security::sha256_hex_for_text(bytes);
        if (!digest.ok) {
            error = digest.error;
            return false;
        }

        stream << "content="
               << quote_manifest_value(relative_path) << "|"
               << quote_manifest_value(file.declared_asset ? "DeclaredAsset" : "Companion") << "|"
               << (file.declared_asset ? "true" : "false") << "|"
               << bytes.size() << "|"
               << quote_manifest_value(digest.hex_digest) << "\n";
        stream << "payload="
               << quote_manifest_value(relative_path) << "|"
               << hex_encode_bytes(bytes) << "\n";
    }

    return write_text_file(
        copperfin::platform::path_from_utf8_string(filesystem_plan.launcher_output_path),
        stream.str(),
        error);
}

void append_library_function_manifest_lines(
    std::ostringstream& stream,
    const RuntimePackagePlan& plan,
    bool include_source_provenance) {
    if (!is_library_output_kind(plan.output_kind)) {
        return;
    }

    const auto parameter_counts = collect_library_export_parameter_counts(plan);
    const auto parameter_names = collect_library_export_parameter_names(plan);
    const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
    const auto routine_kinds = collect_library_export_routine_kinds(plan);
    const auto routine_locations = collect_library_export_routine_locations(plan);
    for (const auto& symbol : plan.exported_symbols) {
        const auto found = parameter_counts.find(symbol);
        const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
        const auto names_found = parameter_names.find(symbol);
        const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
        const auto kind_found = routine_kinds.find(symbol);
        const auto location_found = routine_locations.find(symbol);
        stream << "library_function_kind="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(kind_found == routine_kinds.end() ? std::string("function") : kind_found->second) << "\n";
        if (include_source_provenance) {
            stream << "library_function_source="
                   << quote_manifest_value(symbol) << "|"
                   << (location_found == routine_locations.end()
                           ? std::string{}
                           : build_manifest_source_location(location_found->second))
                   << "\n";
        }
        stream << "library_function_arity="
               << quote_manifest_value(symbol) << "|"
               << parameter_count << "\n";
        stream << "library_function_parameter_declaration="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(
                      declaration_kind_found == parameter_declaration_kinds.end()
                          ? std::string{}
                          : declaration_kind_found->second)
               << "\n";
        stream << "library_function_parameters="
               << quote_manifest_value(symbol) << "|"
               << (names_found == parameter_names.end() ? std::string{} : build_manifest_parameter_names(names_found->second)) << "\n";
        stream << "library_function_call_surface="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(
                      plan.output_kind == BuildOutputKind::fll
                          ? std::string(kFllCallableSignature)
                          : std::string(kVfpLibraryCallableConvention))
               << "|"
               << quote_manifest_value(
                      plan.output_kind == BuildOutputKind::fll
                          ? std::string(kFllDefaultReturnHelper)
                          : (names_found == parameter_names.end()
                                 ? std::string{}
                                 : build_placeholder_int_parameter_list(names_found->second)))
               << "\n";
    }
}

void append_runtime_asset_manifest_lines(std::ostringstream& stream, const RuntimePackagePlan& plan) {
    for (const auto& asset : plan.assets) {
        stream << "asset="
               << asset.record_index << "|"
               << quote_manifest_value(asset.relative_path) << "|"
               << quote_manifest_value(asset.staged_path) << "|"
               << quote_manifest_value(asset.type_title) << "|"
               << (asset.excluded ? "true" : "false") << "|"
               << (asset.exists ? "true" : "false") << "|"
               << quote_manifest_value(asset.sha256) << "|"
               << (asset.copied ? "true" : "false") << "\n";
    }
}

void append_writable_data_manifest_lines(std::ostringstream& stream, const RuntimePackagePlan& plan) {
    for (const auto& asset : plan.assets) {
        if (!asset.package_writable || !asset.copied) {
            continue;
        }
        stream << "data_asset="
               << quote_manifest_value(asset.staged_path) << "|package_writable\n";
    }
    for (const auto& digest : plan.writable_data_payload_digests) {
        stream << "data_payload="
               << quote_manifest_value(digest.path) << "|package_writable|"
               << quote_manifest_value(digest.sha256) << "\n";
    }
}

void append_warning_manifest_lines(std::ostringstream& stream, const RuntimePackagePlan& plan) {
    for (const auto& warning : plan.warnings) {
        stream << "warning=" << quote_manifest_value(warning) << "\n";
    }
}

void append_runtime_feature_flag_manifest_lines(
    std::ostringstream& stream,
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile) {
    append_feature_flag_line(stream, "launcher.dotnet.requested", plan.requested_dotnet_launcher, "rollout");
    append_feature_flag_line(stream, "launcher.dotnet.active", plan.emit_dotnet_launcher, "host_compatibility");
    append_feature_flag_line(stream, "runtime.host.native", is_native_host_output_kind(plan.output_kind), "host_compatibility");
    append_feature_flag_line(stream, "build.output.ast_contract", true, "build_output");
    append_feature_flag_line(stream, "build.output.ir_contract", true, "build_output");
    append_feature_flag_line(stream, "build.output.csharp_transpilation", plan.requested_dotnet_launcher, "build_output");
    append_feature_flag_line(stream, "build.output.library_contract", is_library_output_kind(plan.output_kind), "build_output");
    append_feature_flag_line(stream, "build.output.native_library_wrapper", is_library_output_kind(plan.output_kind), "build_output");
    append_feature_flag_line(stream, "build.output.fll_api_contract", plan.output_kind == BuildOutputKind::fll, "build_output");
    append_feature_flag_line(stream, "build.output.fxp_token_contract", plan.output_kind == BuildOutputKind::fxp, "build_output");
    append_feature_flag_line(stream, "build.output.app_archive_contract", plan.output_kind == BuildOutputKind::app, "build_output");
    append_feature_flag_line(stream, "debug.breakpoints", plan.debug_plan.supports_breakpoints, "debug");
    append_feature_flag_line(stream, "debug.step_debugging", plan.debug_plan.supports_step_debugging, "debug");
    append_feature_flag_line(
        stream,
        "security.native",
        plan.security_enabled && security_profile.available,
        "security");
}

void append_feature_flag_line(
    std::ostringstream& stream,
    std::string_view name,
    bool enabled,
    std::string_view category) {
    stream << "feature_flag="
           << name << "|"
           << (enabled ? "true" : "false") << "|"
           << category << "\n";
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime

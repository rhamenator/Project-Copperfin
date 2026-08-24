// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

#include <locale>

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

std::string build_native_wrapper_cmake_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const std::string output_stem =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.launcher_output_path).stem());
    const std::string output_extension =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.launcher_output_path).extension());
    const std::string output_directory = "..";
    const std::string wrapper_file_name =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.native_wrapper_source_path).filename());
    const std::string module_definition_file_name =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.module_definition_path).filename());

    stream << "cmake_minimum_required(VERSION 3.20)\n";
    stream << "project(" << output_stem << "Wrapper LANGUAGES CXX)\n\n";
    stream << "add_library(" << output_stem << " SHARED " << wrapper_file_name << ")\n";
    stream << "target_compile_features(" << output_stem << " PRIVATE cxx_std_20)\n";
    stream << "set_target_properties(" << output_stem
           << " PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)\n";
    stream << "if(UNIX AND NOT APPLE)\n";
    stream << "  target_link_libraries(" << output_stem << " PRIVATE dl)\n";
    stream << "endif()\n";
    stream << "set_target_properties(" << output_stem
           << " PROPERTIES OUTPUT_NAME \"" << output_stem
           << "\" PREFIX \"\" SUFFIX \"" << output_extension
           << "\" LIBRARY_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/" << output_directory
           << "\" RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/" << output_directory
           << "\" ARCHIVE_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/" << output_directory << "\")\n";
    stream << "foreach(COPPERFIN_CONFIGURATION IN LISTS CMAKE_CONFIGURATION_TYPES)\n";
    stream << "  string(TOUPPER \"${COPPERFIN_CONFIGURATION}\" COPPERFIN_CONFIGURATION_UPPER)\n";
    stream << "  set_target_properties(" << output_stem << " PROPERTIES\n";
    stream << "    \"LIBRARY_OUTPUT_DIRECTORY_${COPPERFIN_CONFIGURATION_UPPER}\" \"${CMAKE_CURRENT_SOURCE_DIR}/"
           << output_directory << "\"\n";
    stream << "    \"RUNTIME_OUTPUT_DIRECTORY_${COPPERFIN_CONFIGURATION_UPPER}\" \"${CMAKE_CURRENT_SOURCE_DIR}/"
           << output_directory << "\"\n";
    stream << "    \"ARCHIVE_OUTPUT_DIRECTORY_${COPPERFIN_CONFIGURATION_UPPER}\" \"${CMAKE_CURRENT_SOURCE_DIR}/"
           << output_directory << "\")\n";
    stream << "endforeach()\n";
    stream << "if(MSVC)\n";
    stream << "  target_link_options(" << output_stem
           << " PRIVATE \"/DEF:${CMAKE_CURRENT_SOURCE_DIR}/../" << module_definition_file_name << "\")\n";
    stream << "endif()\n";
    return stream.str();
}

std::string build_native_wrapper_shell_script_source() {
    std::ostringstream stream;
    stream << "#!/usr/bin/env sh\n";
    stream << "set -eu\n";
    stream << "SCRIPT_DIR=\"$(CDPATH= cd -- \"$(dirname -- \"$0\")\" && pwd)\"\n";
    stream << "cmake -S \"$SCRIPT_DIR\" -B \"$SCRIPT_DIR/build\"\n";
    stream << "cmake --build \"$SCRIPT_DIR/build\"\n";
    return stream.str();
}

std::string build_native_wrapper_powershell_script_source() {
    std::ostringstream stream;
    stream << "$ErrorActionPreference = 'Stop'\n";
    stream << "$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path\n";
    stream << "$buildDir = Join-Path $scriptDir 'build'\n";
    stream << "cmake -S $scriptDir -B $buildDir\n";
    stream << "cmake --build $buildDir\n";
    return stream.str();
}

std::string build_fll_api_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    const auto parameter_counts = collect_library_export_parameter_counts(plan);
    const auto parameter_names = collect_library_export_parameter_names(plan);
    const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
    const auto routine_kinds = collect_library_export_routine_kinds(plan);
    const auto routine_locations = collect_library_export_routine_locations(plan);
    stream << "manifest_version=1\n";
    stream << "manifest_value_encoding=backslash-v1\n";
    stream << "output_kind=fll\n";
    stream << "library_file=" << quote_manifest_value(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path).filename())) << "\n";
    stream << "registration_model=FoxInfo/FoxTable\n";
    stream << "registration_command=SET LIBRARY TO\n";
    stream << "release_command=RELEASE LIBRARY\n";
    stream << "additive_supported=true\n";
    stream << "loader_entrypoint=" << kFllLoaderEntrypoint << "\n";
    stream << "registration_symbol=" << kFllRegistrationSymbol << "\n";
    stream << "callable_signature=" << kFllCallableSignature << "\n";
    stream << "default_return_helper=" << kFllDefaultReturnHelper << "\n";
    for (const auto& symbol : plan.exported_symbols) {
        stream << "function=" << quote_manifest_value(symbol) << "\n";
        const auto kind_found = routine_kinds.find(symbol);
        const auto location_found = routine_locations.find(symbol);
        stream << "function_kind="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(kind_found == routine_kinds.end() ? std::string("function") : kind_found->second) << "\n";
        stream << "function_source="
               << quote_manifest_value(symbol) << "|"
               << (location_found == routine_locations.end()
                       ? std::string{}
                       : build_manifest_source_location(location_found->second))
               << "\n";
        const auto found = parameter_counts.find(symbol);
        const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
        const auto names_found = parameter_names.find(symbol);
        const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
        stream << "function_arity="
               << quote_manifest_value(symbol) << "|"
               << parameter_count << "\n";
        stream << "function_parameter_declaration="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(
                      declaration_kind_found == parameter_declaration_kinds.end()
                          ? std::string{}
                          : declaration_kind_found->second)
               << "\n";
        stream << "function_parameters="
               << quote_manifest_value(symbol) << "|"
               << (names_found == parameter_names.end() ? std::string{} : build_manifest_parameter_names(names_found->second)) << "\n";
        stream << "function_call_surface="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(std::string(kFllCallableSignature)) << "|"
               << quote_manifest_value(std::string(kFllDefaultReturnHelper)) << "\n";
    }
    return stream.str();
}

std::string build_library_api_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    const auto parameter_counts = collect_library_export_parameter_counts(plan);
    const auto parameter_names = collect_library_export_parameter_names(plan);
    const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
    const auto routine_kinds = collect_library_export_routine_kinds(plan);
    const auto routine_locations = collect_library_export_routine_locations(plan);
    stream << "manifest_version=1\n";
    stream << "manifest_value_encoding=backslash-v1\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "library_file=" << quote_manifest_value(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path).filename())) << "\n";
    stream << "callable_convention=" << kVfpLibraryCallableConvention << "\n";
    for (const auto& symbol : plan.exported_symbols) {
        const auto found = parameter_counts.find(symbol);
        const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
        const auto names_found = parameter_names.find(symbol);
        const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
        const auto kind_found = routine_kinds.find(symbol);
        const auto location_found = routine_locations.find(symbol);
        stream << "function=" << quote_manifest_value(symbol) << "\n";
        stream << "function_kind="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(kind_found == routine_kinds.end() ? std::string("function") : kind_found->second) << "\n";
        stream << "function_source="
               << quote_manifest_value(symbol) << "|"
               << (location_found == routine_locations.end()
                       ? std::string{}
                       : build_manifest_source_location(location_found->second))
               << "\n";
        stream << "function_arity="
               << quote_manifest_value(symbol) << "|"
               << parameter_count << "\n";
        stream << "function_parameter_declaration="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(
                      declaration_kind_found == parameter_declaration_kinds.end()
                          ? std::string{}
                          : declaration_kind_found->second)
               << "\n";
        stream << "function_parameters="
               << quote_manifest_value(symbol) << "|"
               << (names_found == parameter_names.end() ? std::string{} : build_manifest_parameter_names(names_found->second)) << "\n";
        stream << "function_call_surface="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(std::string(kVfpLibraryCallableConvention)) << "|"
               << quote_manifest_value(
                      names_found == parameter_names.end()
                          ? std::string{}
                          : build_placeholder_int_parameter_list(names_found->second))
               << "\n";
    }
    return stream.str();
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime

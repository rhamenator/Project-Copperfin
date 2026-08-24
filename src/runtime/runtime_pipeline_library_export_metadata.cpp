// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

#include <locale>

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

std::vector<std::string> collect_library_exported_symbols(const RuntimePackagePlan& plan) {
    std::vector<std::string> exported_symbols;
    std::unordered_set<std::string> seen;

    for (const auto& asset : plan.assets) {
        if (asset.excluded || !asset.exists) {
            continue;
        }

        if (lowercase_copy(copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(asset.source_path).extension())) != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }
            exported_symbols.push_back(export_name);
        }
    }

    return exported_symbols;
}

std::map<std::string, std::size_t> collect_library_export_parameter_counts(const RuntimePackagePlan& plan) {
    std::map<std::string, std::size_t> parameter_counts;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            std::size_t parameter_count = 0U;
            for (const auto& statement : routine.statements) {
                if (statement.kind != StatementKind::parameters_declaration &&
                    statement.kind != StatementKind::lparameters_declaration) {
                    continue;
                }

                parameter_count = statement.names.size();
                break;
            }

            parameter_counts.emplace(export_name, parameter_count);
        }
    }

    return parameter_counts;
}

std::string build_routine_kind_name(const RoutineKind kind) {
    switch (kind) {
        case RoutineKind::procedure:
            return "procedure";
        case RoutineKind::function:
            return "function";
        case RoutineKind::main:
        default:
            return "main";
    }
}

std::map<std::string, std::vector<std::string>> collect_library_export_parameter_names(const RuntimePackagePlan& plan) {
    std::map<std::string, std::vector<std::string>> parameter_names;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            std::vector<std::string> names;
            for (const auto& statement : routine.statements) {
                if (statement.kind != StatementKind::parameters_declaration &&
                    statement.kind != StatementKind::lparameters_declaration) {
                    continue;
                }

                names.reserve(statement.names.size());
                for (std::size_t index = 0; index < statement.names.size(); ++index) {
                    names.push_back(sanitize_cpp_identifier(
                        extract_declared_parameter_name(statement.names[index]),
                        index));
                }
                break;
            }

            parameter_names.emplace(export_name, std::move(names));
        }
    }

    return parameter_names;
}

std::map<std::string, std::string> collect_library_export_parameter_declaration_kinds(const RuntimePackagePlan& plan) {
    std::map<std::string, std::string> declaration_kinds;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            std::string declaration_kind;
            for (const auto& statement : routine.statements) {
                if (statement.kind == StatementKind::parameters_declaration) {
                    declaration_kind = "parameters";
                    break;
                }
                if (statement.kind == StatementKind::lparameters_declaration) {
                    declaration_kind = "lparameters";
                    break;
                }
            }

            declaration_kinds.emplace(export_name, std::move(declaration_kind));
        }
    }

    return declaration_kinds;
}

std::map<std::string, std::string> collect_library_export_routine_kinds(const RuntimePackagePlan& plan) {
    std::map<std::string, std::string> routine_kinds;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            routine_kinds.emplace(export_name, build_routine_kind_name(routine.kind));
        }
    }

    return routine_kinds;
}

std::map<std::string, SourceLocation> collect_library_export_routine_locations(const RuntimePackagePlan& plan) {
    std::map<std::string, SourceLocation> routine_locations;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

        const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            SourceLocation location = routine.declaration_location;
            if (!location.file_path.empty()) {
                location.file_path = copperfin::platform::path_to_utf8_string(
                    normalize_existing_path_spelling(
                        copperfin::platform::path_from_utf8_string(location.file_path)));
            }
            routine_locations.emplace(export_name, std::move(location));
        }
    }

    return routine_locations;
}

std::string build_placeholder_int_parameter_list(const std::vector<std::string>& parameter_names) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < parameter_names.size(); ++index) {
        if (index > 0U) {
            stream << ", ";
        }
        stream << "int " << sanitize_cpp_identifier(parameter_names[index], index);
    }
    return stream.str();
}

std::string build_manifest_parameter_names(const std::vector<std::string>& parameter_names) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < parameter_names.size(); ++index) {
        if (index > 0U) {
            stream << "|";
        }
        stream << quote_manifest_value(parameter_names[index]);
    }
    return stream.str();
}

std::string build_manifest_source_location(const SourceLocation& location) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << quote_manifest_value(location.file_path) << "|" << location.line;
    return stream.str();
}

std::string build_module_definition_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const std::string output_stem =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.launcher_output_path).stem());
    stream << "LIBRARY " << output_stem << "\n";
    stream << "EXPORTS\n";
    for (const auto& symbol : plan.exported_symbols) {
        stream << "    " << symbol << "\n";
    }
    if (plan.output_kind == BuildOutputKind::fll) {
        stream << "    " << kFllLoaderEntrypoint << "\n";
        stream << "    " << kFllRegistrationSymbol << "\n";
    }
    return stream.str();
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime

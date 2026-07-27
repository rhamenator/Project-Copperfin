// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/licensing/license_status.h"
#include "copperfin/licensing/license_status_display.h"
#include "copperfin/localization/localization.h"
#include "copperfin/platform/executable_path.h"
#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/security/process_hardening.h"

#include <filesystem>
#include <iostream>
#include <string>

namespace {

struct CommandLineOptions {
    std::string asset_path;
    std::string locale;
    bool help = false;
    bool valid = true;
    bool license_status = false;
};

CommandLineOptions parse_arguments(int argc, char** argv) {
    CommandLineOptions options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--help" || argument == "-h") {
            options.help = true;
            continue;
        }
        if (argument == "--license-status") {
            options.license_status = true;
            continue;
        }
        if (argument == "--locale") {
            if (index + 1 >= argc) {
                options.valid = false;
                return options;
            }
            options.locale = argv[++index];
            continue;
        }
        if (options.asset_path.empty()) {
            options.asset_path = argument;
        } else {
            options.valid = false;
            return options;
        }
    }
    return options;
}

copperfin::localization::LocalizedCatalog load_localization(
    const std::filesystem::path& executable_path,
    const std::string& explicit_locale) {
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root(executable_path);
    return copperfin::localization::load_catalogs(
        locale_root,
        copperfin::localization::select_locale(explicit_locale));
}

void print_usage(const copperfin::localization::LocalizedCatalog& catalog) {
    std::cout << catalog.translate(
        "Inspect.Usage",
        {
            {"assetPathArgument", "<path-to-vfp-asset>"},
            {"commandName", "copperfin_inspect"},
            {"localeOption", "--locale"},
            {"localeValue", "<locale>"}
        }) << "\n";
}

void print_error_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& error) {
    std::cout << catalog.translate("Inspect.Prefix.Error") << error << "\n";
}

void print_license_status(
    const copperfin::licensing::LicenseStatus& status,
    const copperfin::localization::LocalizedCatalog& catalog) {
    using copperfin::licensing::LicenseState;

    std::cout << "status: ok\n";
    std::cout << "state: " << copperfin::licensing::license_state_name(status.state) << "\n";
    if (status.state == LicenseState::free) {
        return;
    }

    if (!status.license_id.empty()) {
        std::cout << "license_id: " << status.license_id << "\n";
    }
    if (!status.license_type.empty()) {
        std::cout << "license_type: " << status.license_type << "\n";
    }
    if (!status.pricing_model.empty()) {
        std::cout << "pricing_model: " << status.pricing_model << "\n";
    }
    if (!status.licensee_name.empty()) {
        std::cout << "licensee_name: " << status.licensee_name << "\n";
    }
    if (!status.licensee_email.empty()) {
        std::cout << "licensee_email: " << status.licensee_email << "\n";
    }
    if (status.seats > 0) {
        std::cout << "seats: " << status.seats << "\n";
    }
    if (!status.issued_date.empty()) {
        std::cout << "issued_date: " << status.issued_date << "\n";
    }
    if (!status.subscription_expires.empty()) {
        std::cout << "subscription_expires: " << status.subscription_expires << "\n";
    }
    if (status.perpetual_max_major_version > 0) {
        std::cout << "perpetual_max_major_version: " << status.perpetual_max_major_version << "\n";
    }
    if (!status.source_path.empty()) {
        std::cout << "source_path: " << status.source_path << "\n";
    }
    if (!status.diagnostic.empty()) {
        std::cout << "diagnostic: " << copperfin::licensing::localized_license_diagnostic(status, catalog) << "\n";
    }
}

void print_warning_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& warning) {
    std::cerr << catalog.translate("Inspect.Prefix.Warning") << warning << "\n";
}

void print_inspection(
    const copperfin::localization::LocalizedCatalog& catalog,
    const copperfin::vfp::AssetInspectionResult& result) {
    using copperfin::vfp::asset_family_name;
    using copperfin::vfp::index_kind_name;

    std::cout << "path: " << result.path << "\n";
    std::cout << "asset_family: " << asset_family_name(result.family) << "\n";

    if (!result.ok) {
        std::cout << "status: error\n";
        print_error_line(catalog, result.error);
        return;
    }

    std::cout << "status: ok\n";
    if (!result.header_available) {
        std::cout << "header: not_applicable\n";
    } else {
        const auto& header = result.header;
        std::cout << "header.version: 0x" << std::hex << static_cast<unsigned int>(header.version) << std::dec << "\n";
        std::cout << "header.version_description: " << header.version_description() << "\n";
        std::cout << "header.last_update: " << header.last_update_iso8601() << "\n";
        std::cout << "header.record_count: " << header.record_count << "\n";
        std::cout << "header.header_length: " << header.header_length << "\n";
        std::cout << "header.record_length: " << header.record_length << "\n";
        std::cout << "header.table_flags: 0x" << std::hex << static_cast<unsigned int>(header.table_flags) << std::dec << "\n";
        std::cout << "header.code_page_mark: 0x" << std::hex << static_cast<unsigned int>(header.code_page_mark) << std::dec << "\n";
        std::cout << "header.has_memo_file: " << (header.has_memo_file() ? "true" : "false") << "\n";
        std::cout << "header.has_production_index: " << (header.has_production_index() ? "true" : "false") << "\n";
        std::cout << "header.has_structural_cdx: " << (header.has_structural_cdx() ? "true" : "false") << "\n";
        std::cout << "header.has_database_container: " << (header.has_database_container() ? "true" : "false") << "\n";
    }

    if (result.indexes.empty()) {
        std::cout << "indexes: none\n";
        return;
    }

    for (std::size_t index = 0; index < result.indexes.size(); ++index) {
        const auto& item = result.indexes[index];
        const auto& probe = item.probe;

        std::cout << "index[" << index << "].path: " << item.path << "\n";
        std::cout << "index[" << index << "].kind: " << index_kind_name(probe.kind) << "\n";
        std::cout << "index[" << index << "].file_size: " << probe.file_size << "\n";
        std::cout << "index[" << index << "].block_size: " << probe.block_size << "\n";
        std::cout << "index[" << index << "].root_node_offset_hint: " << probe.root_node_offset_hint << "\n";
        std::cout << "index[" << index << "].free_node_offset_hint: " << probe.free_node_offset_hint << "\n";
        std::cout << "index[" << index << "].end_of_file_offset_hint: " << probe.end_of_file_offset_hint << "\n";
        std::cout << "index[" << index << "].key_length_hint: " << probe.key_length_hint << "\n";
        std::cout << "index[" << index << "].max_keys_hint: " << probe.max_keys_hint << "\n";
        std::cout << "index[" << index << "].group_length_hint: " << probe.group_length_hint << "\n";
        std::cout << "index[" << index << "].flags: 0x"
                  << std::hex << static_cast<unsigned int>(probe.flags) << std::dec << "\n";
        std::cout << "index[" << index << "].signature: 0x"
                  << std::hex << static_cast<unsigned int>(probe.signature) << std::dec << "\n";
        std::cout << "index[" << index << "].multi_tag: " << (probe.multi_tag ? "true" : "false") << "\n";
        std::cout << "index[" << index << "].production_candidate: "
                  << (probe.production_candidate ? "true" : "false") << "\n";
        if (!probe.key_expression_hint.empty()) {
            std::cout << "index[" << index << "].key_expression_hint: " << probe.key_expression_hint << "\n";
        }
        if (!probe.for_expression_hint.empty()) {
            std::cout << "index[" << index << "].for_expression_hint: " << probe.for_expression_hint << "\n";
        }
        for (std::size_t tag_index = 0; tag_index < probe.tags.size(); ++tag_index) {
            const auto& tag = probe.tags[tag_index];
            std::cout << "index[" << index << "].tag[" << tag_index << "].name_hint: " << tag.name_hint << "\n";
            std::cout << "index[" << index << "].tag[" << tag_index << "].key_expression_hint: "
                      << tag.key_expression_hint << "\n";
            std::cout << "index[" << index << "].tag[" << tag_index << "].name_offset_hint: "
                      << tag.name_offset_hint << "\n";
            std::cout << "index[" << index << "].tag[" << tag_index << "].key_expression_offset_hint: "
                      << tag.key_expression_offset_hint << "\n";
            std::cout << "index[" << index << "].tag[" << tag_index << "].inferred_name: "
                      << (tag.inferred_name ? "true" : "false") << "\n";
            if (!tag.for_expression_hint.empty()) {
                std::cout << "index[" << index << "].tag[" << tag_index << "].for_expression_hint: "
                          << tag.for_expression_hint << "\n";
            }
            if (tag.for_expression_offset_hint != 0U) {
                std::cout << "index[" << index << "].tag[" << tag_index << "].for_expression_offset_hint: "
                          << tag.for_expression_offset_hint << "\n";
            }
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    const std::filesystem::path invocation_path =
        argc > 0 && argv[0] != nullptr ? std::filesystem::path(argv[0]) : std::filesystem::path();
    const std::filesystem::path running_executable_path =
        copperfin::platform::resolve_running_executable_path(invocation_path);
    const CommandLineOptions options = parse_arguments(argc, argv);
    const copperfin::localization::LocalizedCatalog catalog = load_localization(
        running_executable_path,
        options.locale);
    const auto hardening = copperfin::security::apply_default_process_hardening();
    if (!hardening.applied) {
        print_warning_line(catalog, hardening.message);
    }
    if (options.license_status) {
        print_license_status(copperfin::licensing::load_license_status(running_executable_path), catalog);
        return 0;
    }
    if (!options.valid || options.help || options.asset_path.empty()) {
        print_usage(catalog);
        return options.help && options.valid ? 0 : 1;
    }

    const copperfin::vfp::AssetInspectionResult result = copperfin::vfp::inspect_asset(options.asset_path);
    print_inspection(catalog, result);
    return result.ok ? 0 : 2;
}

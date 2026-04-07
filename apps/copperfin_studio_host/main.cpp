#include "copperfin/studio/document_model.h"
#include "copperfin/studio/product_subsystems.h"
#include "copperfin/studio/vs_launch_contract.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

void print_usage() {
    std::cout << "Usage: copperfin_studio_host --path <asset> [--from-vs] [--read-only] [--json] [--set-property --record <n> --property-name <name> --property-value <value>] [--line <n>] [--column <n>] [--symbol <name>]\n";
    std::cout << "   or: copperfin_studio_host --list-subsystems [--json]\n";
    std::cout << "   or: copperfin_studio_host <asset>\n";
}

std::string json_escape(const std::string& value) {
    std::ostringstream stream;
    for (unsigned char ch : value) {
        switch (ch) {
            case '\"':
                stream << "\\\"";
                break;
            case '\\':
                stream << "\\\\";
                break;
            case '\b':
                stream << "\\b";
                break;
            case '\f':
                stream << "\\f";
                break;
            case '\n':
                stream << "\\n";
                break;
            case '\r':
                stream << "\\r";
                break;
            case '\t':
                stream << "\\t";
                break;
            default:
                if (ch < 0x20U) {
                    stream << "\\u"
                           << std::hex
                           << std::setw(4)
                           << std::setfill('0')
                           << static_cast<unsigned int>(ch)
                           << std::dec
                           << std::setfill(' ');
                } else {
                    stream << static_cast<char>(ch);
                }
                break;
        }
    }
    return stream.str();
}

void print_json_string(const std::string& value) {
    std::cout << "\"" << json_escape(value) << "\"";
}

void print_json_document(const copperfin::studio::StudioDocumentModel& document) {
    const auto objects = copperfin::studio::build_object_snapshot(document);

    std::cout << "{\n";
    std::cout << "  \"status\": \"ok\",\n";
    std::cout << "  \"document\": {\n";
    std::cout << "    \"path\": ";
    print_json_string(document.path);
    std::cout << ",\n";
    std::cout << "    \"displayName\": ";
    print_json_string(document.display_name);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_asset_kind_name(document.kind));
    std::cout << ",\n";
    std::cout << "    \"readOnly\": " << (document.read_only ? "true" : "false") << ",\n";
    std::cout << "    \"launchedFromVisualStudio\": "
              << (document.launched_from_visual_studio ? "true" : "false") << ",\n";
    std::cout << "    \"hasSidecar\": " << (document.has_sidecar ? "true" : "false") << ",\n";
    std::cout << "    \"sidecarPath\": ";
    print_json_string(document.sidecar_path);
    std::cout << ",\n";
    std::cout << "    \"assetFamily\": ";
    print_json_string(copperfin::vfp::asset_family_name(document.inspection.family));
    std::cout << ",\n";
    std::cout << "    \"indexCount\": " << document.inspection.indexes.size() << ",\n";
    std::cout << "    \"headerVersionDescription\": ";
    if (document.inspection.header_available) {
        print_json_string(document.inspection.header.version_description());
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << "    \"fieldCount\": " << document.table_preview.fields.size() << ",\n";
    std::cout << "    \"recordCount\": " << document.table_preview.records.size() << ",\n";
    std::cout << "    \"fields\": [\n";
    for (std::size_t index = 0; index < document.table_preview.fields.size(); ++index) {
        const auto& field = document.table_preview.fields[index];
        std::cout << "      {\"name\": ";
        print_json_string(field.name);
        std::cout << ", \"type\": ";
        print_json_string(std::string(1U, field.type));
        std::cout << ", \"length\": " << static_cast<unsigned int>(field.length);
        std::cout << ", \"decimalCount\": " << static_cast<unsigned int>(field.decimal_count) << "}";
        if ((index + 1U) != document.table_preview.fields.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"objects\": [\n";
    for (std::size_t index = 0; index < objects.size(); ++index) {
        const auto& object = objects[index];
        std::cout << "      {\n";
        std::cout << "        \"recordIndex\": " << object.record_index << ",\n";
        std::cout << "        \"deleted\": " << (object.deleted ? "true" : "false") << ",\n";
        std::cout << "        \"title\": ";
        print_json_string(object.title);
        std::cout << ",\n";
        std::cout << "        \"subtitle\": ";
        print_json_string(object.subtitle);
        std::cout << ",\n";
        std::cout << "        \"properties\": [\n";
        for (std::size_t property_index = 0; property_index < object.properties.size(); ++property_index) {
            const auto& property = object.properties[property_index];
            std::cout << "          {\"name\": ";
            print_json_string(property.name);
            std::cout << ", \"type\": ";
            print_json_string(std::string(1U, property.type));
            std::cout << ", \"isNull\": " << (property.is_null ? "true" : "false") << ", \"value\": ";
            print_json_string(property.value);
            std::cout << "}";
            if ((property_index + 1U) != object.properties.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "        ]\n";
        std::cout << "      }";
        if ((index + 1U) != objects.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  }\n";
    std::cout << "}\n";
}

void print_document(const copperfin::studio::StudioDocumentModel& document) {
    std::cout << "status: ok\n";
    std::cout << "document.path: " << document.path << "\n";
    std::cout << "document.display_name: " << document.display_name << "\n";
    std::cout << "document.kind: " << copperfin::studio::studio_asset_kind_name(document.kind) << "\n";
    std::cout << "document.read_only: " << (document.read_only ? "true" : "false") << "\n";
    std::cout << "document.launched_from_visual_studio: "
              << (document.launched_from_visual_studio ? "true" : "false") << "\n";
    std::cout << "document.has_sidecar: " << (document.has_sidecar ? "true" : "false") << "\n";
    if (!document.sidecar_path.empty()) {
        std::cout << "document.sidecar_path: " << document.sidecar_path << "\n";
    }
    std::cout << "inspection.asset_family: "
              << copperfin::vfp::asset_family_name(document.inspection.family) << "\n";
    std::cout << "inspection.index_count: " << document.inspection.indexes.size() << "\n";
    if (document.inspection.header_available) {
        std::cout << "inspection.header.version_description: "
                  << document.inspection.header.version_description() << "\n";
    }

    if (!document.table_preview_available) {
        return;
    }

    std::cout << "preview.field_count: " << document.table_preview.fields.size() << "\n";
    std::cout << "preview.record_count: " << document.table_preview.records.size() << "\n";

    if (!document.table_preview.fields.empty()) {
        std::cout << "preview.fields:";
        for (const auto& field : document.table_preview.fields) {
            std::cout << " " << field.name << "(" << field.type << "," << static_cast<unsigned int>(field.length) << ")";
        }
        std::cout << "\n";
    }

    for (const auto& record : document.table_preview.records) {
        std::cout << "record[" << record.record_index << "]";
        if (record.deleted) {
            std::cout << " deleted";
        }
        std::cout << "\n";

        for (const auto& value : record.values) {
            if (value.display_value.empty()) {
                continue;
            }
            std::cout << "  " << value.field_name << ": " << value.display_value << "\n";
        }
    }
}

void print_json_subsystems() {
    const auto& subsystems = copperfin::studio::product_subsystems();
    std::cout << "{\n";
    std::cout << "  \"status\": \"ok\",\n";
    std::cout << "  \"subsystems\": [\n";
    for (std::size_t index = 0; index < subsystems.size(); ++index) {
        const auto& subsystem = subsystems[index];
        std::cout << "    {\n";
        std::cout << "      \"id\": ";
        print_json_string(std::string(subsystem.id));
        std::cout << ",\n";
        std::cout << "      \"title\": ";
        print_json_string(std::string(subsystem.title));
        std::cout << ",\n";
        std::cout << "      \"vfp9Equivalent\": ";
        print_json_string(std::string(subsystem.vfp9_equivalent));
        std::cout << ",\n";
        std::cout << "      \"copperfinComponent\": ";
        print_json_string(std::string(subsystem.copperfin_component));
        std::cout << ",\n";
        std::cout << "      \"hostKind\": ";
        print_json_string(copperfin::studio::product_host_kind_name(subsystem.host_kind));
        std::cout << ",\n";
        std::cout << "      \"currentStatus\": ";
        print_json_string(std::string(subsystem.current_status));
        std::cout << ",\n";
        std::cout << "      \"parityScope\": ";
        print_json_string(std::string(subsystem.parity_scope));
        std::cout << ",\n";
        std::cout << "      \"modernEditorDirection\": ";
        print_json_string(std::string(subsystem.modern_editor_direction));
        std::cout << "\n";
        std::cout << "    }";
        if ((index + 1U) != subsystems.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "  ]\n";
    std::cout << "}\n";
}

void print_subsystems() {
    const auto& subsystems = copperfin::studio::product_subsystems();
    std::cout << "status: ok\n";
    std::cout << "subsystem_count: " << subsystems.size() << "\n";
    for (const auto& subsystem : subsystems) {
        std::cout << "subsystem.id: " << subsystem.id << "\n";
        std::cout << "  title: " << subsystem.title << "\n";
        std::cout << "  vfp9_equivalent: " << subsystem.vfp9_equivalent << "\n";
        std::cout << "  copperfin_component: " << subsystem.copperfin_component << "\n";
        std::cout << "  host_kind: " << copperfin::studio::product_host_kind_name(subsystem.host_kind) << "\n";
        std::cout << "  current_status: " << subsystem.current_status << "\n";
        std::cout << "  parity_scope: " << subsystem.parity_scope << "\n";
        std::cout << "  modern_editor_direction: " << subsystem.modern_editor_direction << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args;
    args.reserve(argc > 1 ? static_cast<std::size_t>(argc - 1) : 0U);
    for (int index = 1; index < argc; ++index) {
        args.emplace_back(argv[index]);
    }

    const bool list_subsystems = std::find(args.begin(), args.end(), "--list-subsystems") != args.end();
    if (list_subsystems) {
        const bool output_json = std::find(args.begin(), args.end(), "--json") != args.end();
        if (output_json) {
            print_json_subsystems();
        } else {
            print_subsystems();
        }
        return 0;
    }

    const auto parse_result = copperfin::studio::parse_launch_arguments(args);
    if (!parse_result.ok) {
        std::cout << "status: error\n";
        std::cout << "error: " << parse_result.error << "\n";
        print_usage();
        return 2;
    }

    if (parse_result.show_help) {
        print_usage();
        return 0;
    }

    if (parse_result.request.apply_property_update) {
        const auto update_result = copperfin::vfp::update_visual_object_property({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .property_name = parse_result.request.property_name,
            .property_value = parse_result.request.property_value
        });

        if (!update_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << update_result.error << "\n";
            return 4;
        }
    }

    const auto open_result = copperfin::studio::open_document(parse_result.request);
    if (!open_result.ok) {
        std::cout << "status: error\n";
        std::cout << "error: " << open_result.error << "\n";
        return 3;
    }

    if (parse_result.output_json) {
        print_json_document(open_result.document);
        return 0;
    }

    print_document(open_result.document);
    return 0;
}

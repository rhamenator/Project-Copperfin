#include "copperfin/studio/document_model.h"
#include "copperfin/studio/vs_launch_contract.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

void print_usage() {
    std::cout << "Usage: copperfin_studio_host --path <asset> [--from-vs] [--read-only] [--line <n>] [--column <n>] [--symbol <name>]\n";
    std::cout << "   or: copperfin_studio_host <asset>\n";
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

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args;
    args.reserve(argc > 1 ? static_cast<std::size_t>(argc - 1) : 0U);
    for (int index = 1; index < argc; ++index) {
        args.emplace_back(argv[index]);
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

    const auto open_result = copperfin::studio::open_document(parse_result.request);
    if (!open_result.ok) {
        std::cout << "status: error\n";
        std::cout << "error: " << open_result.error << "\n";
        return 3;
    }

    print_document(open_result.document);
    return 0;
}

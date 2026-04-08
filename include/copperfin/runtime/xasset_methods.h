#pragma once

#include "copperfin/studio/document_model.h"

#include <string>
#include <vector>

namespace copperfin::runtime {

struct XAssetMethod {
    std::size_t record_index = 0;
    std::string object_path;
    std::string method_name;
    std::string routine_name;
    std::string source_text;
};

struct XAssetExecutableModel {
    bool ok = false;
    bool runnable_startup = false;
    std::string asset_path;
    std::string root_object_path;
    std::string error;
    std::vector<XAssetMethod> methods;
    std::vector<std::string> startup_routines;
};

XAssetExecutableModel build_xasset_executable_model(const studio::StudioDocumentModel& document);
std::string build_xasset_bootstrap_source(const XAssetExecutableModel& model, bool include_read_events);

}  // namespace copperfin::runtime

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "copperfin/studio/document_model.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::runtime {

struct XAssetMethod {
    std::size_t record_index = 0;
    std::size_t source_field_index = studio::StudioObjectMissingFieldIndex;
    std::size_t source_line_index = studio::StudioObjectMissingLineIndex;
    std::uint32_t source_memo_block_number = 0;
    std::string object_path;
    std::string method_name;
    std::string routine_name;
    std::string source_text;
};

struct XAssetActionBinding {
    std::size_t record_index = 0;
    std::size_t title_field_index = studio::StudioObjectMissingFieldIndex;
    std::uint32_t title_memo_block_number = 0;
    std::size_t routine_source_field_index = studio::StudioObjectMissingFieldIndex;
    std::size_t routine_source_line_index = studio::StudioObjectMissingLineIndex;
    std::uint32_t routine_source_memo_block_number = 0;
    std::string action_id;
    std::string title;
    std::string kind;
    std::string routine_name;
};

struct XAssetLifecycleStep {
    std::size_t record_index = 0;
    std::size_t source_field_index = studio::StudioObjectMissingFieldIndex;
    std::size_t source_line_index = studio::StudioObjectMissingLineIndex;
    std::uint32_t source_memo_block_number = 0;
    std::string kind;
    std::string command_text;
    std::string routine_name;
};

struct XAssetExecutableModel {
    bool ok = false;
    bool runnable_startup = false;
    bool startup_enters_event_loop = false;
    std::string asset_path;
    std::string root_object_path;
    bool root_is_form = false;
    std::string activation_kind;
    std::string activation_target;
    std::string error;
    std::vector<XAssetMethod> methods;
    std::vector<XAssetActionBinding> actions;
    std::vector<std::string> startup_routines;
    std::vector<std::string> startup_lines;
    std::vector<XAssetLifecycleStep> startup_steps;
    std::vector<std::string> shutdown_routines;
    std::vector<std::string> shutdown_lines;
    std::vector<XAssetLifecycleStep> shutdown_steps;
};

XAssetExecutableModel build_xasset_executable_model(const studio::StudioDocumentModel& document);
std::string build_xasset_bootstrap_source(
    const XAssetExecutableModel& model,
    bool include_read_events,
    const std::string& execution_asset_path = {},
    bool bind_form_lifecycle = false);

}  // namespace copperfin::runtime

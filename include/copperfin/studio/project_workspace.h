// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "copperfin/studio/document_model.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::localization {
struct LocalizedCatalog;
}

namespace copperfin::studio {

inline constexpr std::size_t StudioProjectMissingFieldIndex = static_cast<std::size_t>(-1);

struct StudioProjectEntry {
    std::size_t record_index = 0;
    bool deleted = false;
    std::string name{};
    std::size_t name_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t name_memo_block_number = 0;
    std::string relative_path{};
    std::size_t relative_path_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t relative_path_memo_block_number = 0;
    std::string type_code{};
    std::size_t type_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t type_memo_block_number = 0;
    std::string type_title{};
    std::size_t type_title_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t type_title_memo_block_number = 0;
    std::string group_id{};
    std::size_t group_id_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t group_id_memo_block_number = 0;
    std::string group_title{};
    std::size_t group_title_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t group_title_memo_block_number = 0;
    std::string key{};
    std::size_t key_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t key_memo_block_number = 0;
    std::string comments{};
    std::size_t comments_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t comments_memo_block_number = 0;
    bool excluded = false;
    std::size_t exclude_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t exclude_memo_block_number = 0;
    bool main_program = false;
    std::size_t main_program_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t main_program_memo_block_number = 0;
    bool local = false;
    std::size_t local_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t local_memo_block_number = 0;
};

struct StudioProjectGroup {
    std::string id{};
    std::size_t id_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t id_memo_block_number = 0;
    std::string title{};
    std::size_t title_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t title_memo_block_number = 0;
    std::size_t item_count = 0;
    std::size_t excluded_count = 0;
    std::size_t deleted_count = 0;
    std::vector<std::size_t> record_indexes{};
};

struct StudioProjectBuildPlan {
    bool available = false;
    bool can_build = false;
    std::string project_title{};
    std::size_t project_title_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t project_title_memo_block_number = 0;
    std::string project_key{};
    std::size_t project_key_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t project_key_memo_block_number = 0;
    std::string home_directory{};
    std::size_t home_directory_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t home_directory_memo_block_number = 0;
    std::string output_path{};
    std::size_t output_path_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t output_path_memo_block_number = 0;
    std::string output_kind{};
    std::size_t output_kind_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t output_kind_memo_block_number = 0;
    std::string build_target{};
    std::size_t build_target_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t build_target_memo_block_number = 0;
    std::string startup_item{};
    std::size_t startup_item_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t startup_item_memo_block_number = 0;
    std::size_t startup_record_index = 0;
    std::size_t total_items = 0;
    std::size_t excluded_items = 0;
    std::size_t deleted_items = 0;
    bool debug_enabled = false;
    std::size_t debug_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t debug_memo_block_number = 0;
    bool encrypt_enabled = false;
    std::size_t encrypt_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t encrypt_memo_block_number = 0;
    bool save_code = false;
    std::size_t save_code_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t save_code_memo_block_number = 0;
    bool no_logo = false;
    std::size_t no_logo_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t no_logo_memo_block_number = 0;
};

struct StudioProjectWorkspace {
    bool available = false;
    std::string project_title{};
    std::size_t project_title_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t project_title_memo_block_number = 0;
    std::string project_key{};
    std::size_t project_key_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t project_key_memo_block_number = 0;
    std::string home_directory{};
    std::size_t home_directory_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t home_directory_memo_block_number = 0;
    std::string output_path{};
    std::size_t output_path_field_index = StudioProjectMissingFieldIndex;
    std::uint32_t output_path_memo_block_number = 0;
    std::vector<StudioProjectGroup> groups{};
    std::vector<StudioProjectEntry> entries{};
    StudioProjectBuildPlan build_plan{};
};

[[nodiscard]] StudioProjectWorkspace build_project_workspace(const StudioDocumentModel& document);
[[nodiscard]] StudioProjectWorkspace build_project_workspace(
    const StudioDocumentModel& document,
    const localization::LocalizedCatalog& catalog);

}  // namespace copperfin::studio

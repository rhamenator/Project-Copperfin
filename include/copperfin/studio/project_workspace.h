#pragma once

#include "copperfin/studio/document_model.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

inline constexpr std::size_t StudioProjectMissingFieldIndex = static_cast<std::size_t>(-1);

struct StudioProjectEntry {
    std::size_t record_index = 0;
    std::string name{};
    std::size_t name_field_index = StudioProjectMissingFieldIndex;
    std::string relative_path{};
    std::string type_code{};
    std::size_t type_field_index = StudioProjectMissingFieldIndex;
    std::string type_title{};
    std::string group_id{};
    std::string group_title{};
    std::string key{};
    std::size_t key_field_index = StudioProjectMissingFieldIndex;
    std::string comments{};
    std::size_t comments_field_index = StudioProjectMissingFieldIndex;
    bool excluded = false;
    std::size_t exclude_field_index = StudioProjectMissingFieldIndex;
    bool main_program = false;
    std::size_t main_program_field_index = StudioProjectMissingFieldIndex;
    bool local = false;
    std::size_t local_field_index = StudioProjectMissingFieldIndex;
};

struct StudioProjectGroup {
    std::string id{};
    std::string title{};
    std::size_t item_count = 0;
    std::size_t excluded_count = 0;
    std::vector<std::size_t> record_indexes{};
};

struct StudioProjectBuildPlan {
    bool available = false;
    bool can_build = false;
    std::string project_title{};
    std::string project_key{};
    std::size_t project_key_field_index = StudioProjectMissingFieldIndex;
    std::string home_directory{};
    std::size_t home_directory_field_index = StudioProjectMissingFieldIndex;
    std::string output_path{};
    std::size_t output_path_field_index = StudioProjectMissingFieldIndex;
    std::string output_kind{};
    std::string build_target{};
    std::string startup_item{};
    std::size_t startup_record_index = 0;
    std::size_t total_items = 0;
    std::size_t excluded_items = 0;
    bool debug_enabled = false;
    std::size_t debug_field_index = StudioProjectMissingFieldIndex;
    bool encrypt_enabled = false;
    std::size_t encrypt_field_index = StudioProjectMissingFieldIndex;
    bool save_code = false;
    std::size_t save_code_field_index = StudioProjectMissingFieldIndex;
    bool no_logo = false;
    std::size_t no_logo_field_index = StudioProjectMissingFieldIndex;
};

struct StudioProjectWorkspace {
    bool available = false;
    std::string project_title{};
    std::string project_key{};
    std::string home_directory{};
    std::string output_path{};
    std::vector<StudioProjectGroup> groups{};
    std::vector<StudioProjectEntry> entries{};
    StudioProjectBuildPlan build_plan{};
};

[[nodiscard]] StudioProjectWorkspace build_project_workspace(const StudioDocumentModel& document);

}  // namespace copperfin::studio

// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/studio/project_workspace.h"

#include "copperfin/localization/localization.h"
#include "copperfin/platform/path.h"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string_view>

namespace copperfin::studio {

namespace {

bool is_ascii_space(unsigned char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v';
}

localization::LocalizedCatalog project_workspace_catalog() {
    return localization::load_catalogs(
        localization::resolve_catalog_root(),
        localization::select_locale());
}

std::string project_workspace_text(
    const localization::LocalizedCatalog& catalog,
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {}) {
    return catalog.translate(key, placeholders);
}

const vfp::DbfRecordValue* find_value(const vfp::DbfRecord& record, std::string_view field_name) {
    for (const auto& value : record.values) {
        if (value.field_name == field_name) {
            return &value;
        }
    }
    return nullptr;
}

std::size_t field_index_or_missing(const vfp::DbfRecord& record, std::string_view field_name) {
    for (std::size_t index = 0U; index < record.values.size(); ++index) {
        if (record.values[index].field_name == field_name) {
            return index;
        }
    }
    return StudioProjectMissingFieldIndex;
}

std::uint32_t memo_block_number_or_zero(const vfp::DbfRecord& record, std::string_view field_name) {
    const auto* value = find_value(record, field_name);
    return value == nullptr ? 0U : value->memo_block_number;
}

bool looks_like_unresolved_memo(const std::string& value) {
    return value.rfind("<memo block ", 0) == 0;
}

std::string value_or_empty(const vfp::DbfRecord& record, std::string_view field_name) {
    const auto* value = find_value(record, field_name);
    if (value == nullptr || looks_like_unresolved_memo(value->display_value)) {
        return {};
    }
    return value->display_value;
}

std::string trim_copy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return !is_ascii_space(ch);
    }));
    while (!value.empty() && is_ascii_space(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    return value;
}

char lowercase_ascii_byte(unsigned char ch) {
    return ch >= 'A' && ch <= 'Z'
        ? static_cast<char>(ch - 'A' + 'a')
        : static_cast<char>(ch);
}

char uppercase_ascii_byte(unsigned char ch) {
    return ch >= 'a' && ch <= 'z'
        ? static_cast<char>(ch - 'a' + 'A')
        : static_cast<char>(ch);
}

bool is_ascii_alpha(unsigned char ch) {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return lowercase_ascii_byte(ch);
    });
    return value;
}

bool looks_like_unresolved_output_path(const std::string& value) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    return normalized == "<source>" || looks_like_unresolved_memo(normalized);
}

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return uppercase_ascii_byte(ch);
    });
    return value;
}

bool value_as_bool(const vfp::DbfRecord& record, std::string_view field_name) {
    const std::string value = lowercase_copy(trim_copy(value_or_empty(record, field_name)));
    return value == "true" || value == "t" || value == ".t." || value == "y";
}

std::string type_code_of(const vfp::DbfRecord& record) {
    return uppercase_copy(trim_copy(value_or_empty(record, "TYPE")));
}

std::string extension_of(const std::string& value) {
    return lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(value).extension()));
}

std::string filename_stem_for_vfp_path(const std::string& value) {
    const std::size_t separator = value.find_last_of("/\\");
    const std::string leaf = separator == std::string::npos ? value : value.substr(separator + 1U);
    const std::size_t dot = leaf.find_last_of('.');
    if (dot == std::string::npos || dot == 0U) {
        return leaf;
    }
    return leaf.substr(0U, dot);
}

std::string filename_for_vfp_path(const std::string& value) {
    const std::size_t separator = value.find_last_of("/\\");
    return separator == std::string::npos ? value : value.substr(separator + 1U);
}

std::string parent_for_vfp_path(const std::string& value) {
    const std::size_t separator = value.find_last_of("/\\");
    return separator == std::string::npos ? std::string() : value.substr(0U, separator);
}

std::string slash_normalized_copy(std::string value) {
    std::replace(value.begin(), value.end(), '\\', '/');
    return value;
}

bool is_windows_drive_absolute_path(const std::string& value) {
    return value.size() >= 3U &&
        is_ascii_alpha(static_cast<unsigned char>(value[0])) &&
        value[1] == ':' &&
        (value[2] == '\\' || value[2] == '/');
}

bool is_unc_path(const std::string& value) {
    return value.size() >= 2U &&
        ((value[0] == '\\' && value[1] == '\\') || (value[0] == '/' && value[1] == '/'));
}

bool is_vfp_absolute_path(const std::string& value) {
    return is_windows_drive_absolute_path(value) || is_unc_path(value) ||
        copperfin::platform::path_from_utf8_string(value).is_absolute();
}

std::optional<std::string> vfp_relative_to_document_dir(const StudioDocumentModel& document, const std::string& value) {
    const std::string document_dir = slash_normalized_copy(parent_for_vfp_path(document.path));
    const std::string item_path = slash_normalized_copy(value);
    if (document_dir.empty() || item_path.size() <= document_dir.size()) {
        return std::nullopt;
    }

    const std::string document_dir_lower = lowercase_copy(document_dir);
    const std::string item_path_lower = lowercase_copy(item_path);
    if (item_path_lower.rfind(document_dir_lower, 0U) != 0U) {
        return std::nullopt;
    }

    const char separator = item_path[document_dir.size()];
    if (separator != '/') {
        return std::nullopt;
    }

    return value.substr(document_dir.size() + 1U);
}

std::string fallback_relative_path(const StudioDocumentModel& document, const std::string& value) {
    if (value.empty()) {
        return {};
    }

    if (is_vfp_absolute_path(value)) {
        if (const auto relative = vfp_relative_to_document_dir(document, value)) {
            return *relative;
        }
        return filename_for_vfp_path(value);
    }

    return value;
}

struct ProjectTypeDescriptor {
    std::string type_title;
    std::string group_id;
    std::string group_title;
    std::string_view source_field_name;
};

ProjectTypeDescriptor describe_project_item(
    const std::string& type_code,
    const std::string& extension,
    const std::string& item_name,
    const localization::LocalizedCatalog& catalog) {
    if (type_code == "H") {
        return {
            project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.ProjectHeader"),
            "project",
            project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Project"),
            "TYPE"};
    }

    if (extension == ".scx") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Form"), "forms", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Forms"), "NAME"};
    }
    if (extension == ".vcx") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.ClassLibrary"), "classes", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.ClassLibraries"), "NAME"};
    }
    if (extension == ".frx") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Report"), "reports", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Reports"), "NAME"};
    }
    if (extension == ".lbx") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Label"), "labels", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Labels"), "NAME"};
    }
    if (extension == ".mnx") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Menu"), "menus", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Menus"), "NAME"};
    }
    if (extension == ".prg") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Program"), "programs", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Programs"), "NAME"};
    }
    if (extension == ".dbc") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Database"), "databases", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Databases"), "NAME"};
    }
    if (extension == ".dbf") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Table"), "tables", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Tables"), "NAME"};
    }
    if (extension == ".qpr") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Query"), "queries", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Queries"), "NAME"};
    }
    if (extension == ".h" || extension == ".hpp" || extension == ".ch") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Header"), "code", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Code"), "NAME"};
    }
    if (extension == ".dll" || extension == ".ocx") {
        return {project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.Library"), "libraries", project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.Libraries"), "NAME"};
    }

    if (type_code == "K") {
        return {
            project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.ProjectItem"),
            "project_items",
            project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.ProjectItems"),
            "TYPE"};
    }

    if (!item_name.empty()) {
        return {
            project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.ProjectItem"),
            "other_assets",
            project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.OtherAssets"),
            "NAME"};
    }

    return {
        project_workspace_text(catalog, "Studio.ProjectWorkspace.ItemType.ProjectRecord"),
        "other_records",
        project_workspace_text(catalog, "Studio.ProjectWorkspace.Group.OtherRecords"),
        {}};
}

std::string default_output_path(const StudioDocumentModel& document, const std::string& project_title) {
    const std::string stem = project_title.empty()
        ? filename_stem_for_vfp_path(document.path)
        : project_title;
#if defined(_WIN32)
    const std::string leaf = stem + ".exe";
#else
    const std::string leaf = stem;
#endif
    const std::size_t separator = document.path.find_last_of("/\\");
    if (separator != std::string::npos) {
        return document.path.substr(0U, separator + 1U) + leaf;
    }
    return copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(document.path).parent_path() /
        copperfin::platform::path_from_utf8_string(leaf));
}

std::string infer_output_kind(const std::string& output_path) {
    const std::string extension = extension_of(output_path);
    if (extension == ".dll") {
        return "dll";
    }
    if (extension == ".app") {
        return "app";
    }
    if (extension == ".fll") {
        return "fll";
    }
    if (extension == ".fxp") {
        return "fxp";
    }
    if (extension == ".ocx") {
        return "ocx";
    }
    if (extension == ".exe") {
        return "executable";
    }
    return "unknown";
}

std::string build_target_for_output_kind(
    const std::string& output_kind,
    const std::string& output_path,
    const localization::LocalizedCatalog& catalog) {
    if (output_kind == "dll") {
        return project_workspace_text(catalog, "Studio.ProjectWorkspace.BuildTarget.WindowsDynamicLinkLibrary");
    }
    if (output_kind == "app") {
        return project_workspace_text(catalog, "Studio.ProjectWorkspace.BuildTarget.VisualFoxProApplicationArchive");
    }
    if (output_kind == "fll") {
        return project_workspace_text(catalog, "Studio.ProjectWorkspace.BuildTarget.VisualFoxProLibrary");
    }
    if (output_kind == "fxp") {
        return project_workspace_text(catalog, "Studio.ProjectWorkspace.BuildTarget.VisualFoxProTokenizedProgram");
    }
    if (output_kind == "ocx") {
        return project_workspace_text(catalog, "Studio.ProjectWorkspace.BuildTarget.WindowsActiveXControl");
    }
    if (output_kind == "executable" && extension_of(output_path) != ".exe") {
        return project_workspace_text(catalog, "Studio.ProjectWorkspace.BuildTarget.NativeExecutable");
    }
    return project_workspace_text(catalog, "Studio.ProjectWorkspace.BuildTarget.WindowsExecutable");
}

}  // namespace

StudioProjectWorkspace build_project_workspace(const StudioDocumentModel& document) {
    return build_project_workspace(document, project_workspace_catalog());
}

StudioProjectWorkspace build_project_workspace(
    const StudioDocumentModel& document,
    const localization::LocalizedCatalog& catalog) {
    StudioProjectWorkspace workspace;
    if (document.kind != StudioAssetKind::project || !document.table_preview_available) {
        return workspace;
    }

    workspace.available = true;

    auto header_record = std::find_if(
        document.table_preview.records.begin(),
        document.table_preview.records.end(),
        [](const vfp::DbfRecord& record) {
            return !record.deleted && type_code_of(record) == "H";
        });
    if (header_record == document.table_preview.records.end()) {
        header_record = std::find_if(
            document.table_preview.records.begin(),
            document.table_preview.records.end(),
            [](const vfp::DbfRecord& record) {
                return type_code_of(record) == "H";
            });
    }

    if (header_record != document.table_preview.records.end()) {
        workspace.project_key = trim_copy(value_or_empty(*header_record, "KEY"));
        workspace.project_key_field_index = field_index_or_missing(*header_record, "KEY");
        workspace.project_key_memo_block_number = memo_block_number_or_zero(*header_record, "KEY");
        workspace.project_title = workspace.project_key.empty()
            ? filename_stem_for_vfp_path(document.path)
            : workspace.project_key;
        workspace.project_title_field_index = workspace.project_key.empty()
            ? StudioProjectMissingFieldIndex
            : workspace.project_key_field_index;
        workspace.project_title_memo_block_number = workspace.project_key.empty()
            ? 0U
            : workspace.project_key_memo_block_number;
        workspace.home_directory = trim_copy(value_or_empty(*header_record, "HOMEDIR"));
        workspace.home_directory_field_index = field_index_or_missing(*header_record, "HOMEDIR");
        workspace.home_directory_memo_block_number = memo_block_number_or_zero(*header_record, "HOMEDIR");
        workspace.output_path = trim_copy(value_or_empty(*header_record, "OUTFILE"));
        workspace.output_path_field_index = field_index_or_missing(*header_record, "OUTFILE");
        workspace.output_path_memo_block_number = memo_block_number_or_zero(*header_record, "OUTFILE");
        if (looks_like_unresolved_output_path(workspace.output_path)) {
            workspace.output_path.clear();
            workspace.output_path_field_index = StudioProjectMissingFieldIndex;
            workspace.output_path_memo_block_number = 0U;
        }
    } else {
        workspace.project_title = filename_stem_for_vfp_path(document.path);
    }

    bool generated_default_output_path = false;
    if (workspace.output_path.empty()) {
        workspace.output_path = default_output_path(document, workspace.project_title);
        workspace.output_path_field_index = StudioProjectMissingFieldIndex;
        workspace.output_path_memo_block_number = 0U;
        generated_default_output_path = true;
    }

    std::vector<StudioProjectGroup> groups;

    auto ensure_group = [&](
        const std::string& group_id,
        const std::string& group_title,
        std::size_t group_field_index,
        std::uint32_t group_memo_block_number) -> StudioProjectGroup& {
        const auto existing = std::find_if(groups.begin(), groups.end(), [&](const StudioProjectGroup& group) {
            return group.id == group_id;
        });
        if (existing != groups.end()) {
            return *existing;
        }

        groups.push_back({
            .id = group_id,
            .id_field_index = group_field_index,
            .id_memo_block_number = group_memo_block_number,
            .title = group_title,
            .title_field_index = group_field_index,
            .title_memo_block_number = group_memo_block_number
        });
        return groups.back();
    };

    for (const auto& record : document.table_preview.records) {
        const std::string type_code = type_code_of(record);
        const std::string name = trim_copy(value_or_empty(record, "NAME"));
        const std::string key = trim_copy(value_or_empty(record, "KEY"));
        const std::string comments = trim_copy(value_or_empty(record, "COMMENTS"));
        const std::string relative_path = fallback_relative_path(document, name);
        const ProjectTypeDescriptor descriptor = describe_project_item(type_code, extension_of(name), name, catalog);
        const std::uint32_t name_memo_block_number = memo_block_number_or_zero(record, "NAME");

        StudioProjectEntry entry;
        entry.record_index = record.record_index;
        entry.deleted = record.deleted;
        entry.name = name.empty()
            ? project_workspace_text(
                catalog,
                "Studio.ProjectWorkspace.Fallback.RecordTitle",
                {{"recordIndex", std::to_string(record.record_index)}})
            : name;
        entry.name_field_index = field_index_or_missing(record, "NAME");
        entry.name_memo_block_number = name_memo_block_number;
        entry.relative_path = relative_path;
        entry.relative_path_field_index = relative_path.empty()
            ? StudioProjectMissingFieldIndex
            : entry.name_field_index;
        entry.relative_path_memo_block_number = relative_path.empty()
            ? 0U
            : name_memo_block_number;
        entry.type_code = type_code;
        entry.type_field_index = field_index_or_missing(record, "TYPE");
        entry.type_memo_block_number = memo_block_number_or_zero(record, "TYPE");
        entry.type_title = descriptor.type_title;
        const std::size_t classification_field_index = descriptor.source_field_name.empty()
            ? StudioProjectMissingFieldIndex
            : field_index_or_missing(record, descriptor.source_field_name);
        const std::uint32_t classification_memo_block_number = descriptor.source_field_name.empty()
            ? 0U
            : memo_block_number_or_zero(record, descriptor.source_field_name);
        entry.type_title_field_index = classification_field_index;
        entry.type_title_memo_block_number = classification_memo_block_number;
        entry.group_id = descriptor.group_id;
        entry.group_id_field_index = classification_field_index;
        entry.group_id_memo_block_number = classification_memo_block_number;
        entry.group_title = descriptor.group_title;
        entry.group_title_field_index = classification_field_index;
        entry.group_title_memo_block_number = classification_memo_block_number;
        entry.key = key;
        entry.key_field_index = field_index_or_missing(record, "KEY");
        entry.key_memo_block_number = memo_block_number_or_zero(record, "KEY");
        entry.comments = comments;
        entry.comments_field_index = field_index_or_missing(record, "COMMENTS");
        entry.comments_memo_block_number = memo_block_number_or_zero(record, "COMMENTS");
        entry.excluded = value_as_bool(record, "EXCLUDE");
        entry.exclude_field_index = field_index_or_missing(record, "EXCLUDE");
        entry.exclude_memo_block_number = memo_block_number_or_zero(record, "EXCLUDE");
        entry.main_program = value_as_bool(record, "MAINPROG");
        entry.main_program_field_index = field_index_or_missing(record, "MAINPROG");
        entry.main_program_memo_block_number = memo_block_number_or_zero(record, "MAINPROG");
        entry.local = value_as_bool(record, "LOCAL");
        entry.local_field_index = field_index_or_missing(record, "LOCAL");
        entry.local_memo_block_number = memo_block_number_or_zero(record, "LOCAL");

        auto& group = ensure_group(
            descriptor.group_id,
            descriptor.group_title,
            entry.group_id_field_index,
            entry.group_id_memo_block_number);
        group.record_indexes.push_back(entry.record_index);
        ++group.item_count;
        if (entry.excluded) {
            ++group.excluded_count;
        }
        if (entry.deleted) {
            ++group.deleted_count;
        }

        workspace.entries.push_back(std::move(entry));
    }

    std::sort(groups.begin(), groups.end(), [](const StudioProjectGroup& left, const StudioProjectGroup& right) {
        if (left.id == "project") {
            return true;
        }
        if (right.id == "project") {
            return false;
        }
        return left.id < right.id;
    });
    workspace.groups = std::move(groups);

    workspace.build_plan.available = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.project_title_field_index = workspace.project_title_field_index;
    workspace.build_plan.project_title_memo_block_number = workspace.project_title_memo_block_number;
    workspace.build_plan.project_key = workspace.project_key;
    workspace.build_plan.project_key_memo_block_number = workspace.project_key_memo_block_number;
    workspace.build_plan.home_directory = workspace.home_directory;
    workspace.build_plan.home_directory_memo_block_number = workspace.home_directory_memo_block_number;
    workspace.build_plan.output_path = workspace.output_path;
    workspace.build_plan.output_path_memo_block_number = workspace.output_path_memo_block_number;
    workspace.build_plan.output_kind = infer_output_kind(workspace.output_path);
    if (generated_default_output_path && workspace.build_plan.output_kind == "unknown") {
        workspace.build_plan.output_kind = "executable";
    }
    workspace.build_plan.output_kind_field_index = workspace.output_path_field_index;
    workspace.build_plan.output_kind_memo_block_number = workspace.output_path_memo_block_number;
    workspace.build_plan.build_target = build_target_for_output_kind(
        workspace.build_plan.output_kind,
        workspace.output_path,
        catalog);
    workspace.build_plan.build_target_field_index = workspace.output_path_field_index;
    workspace.build_plan.build_target_memo_block_number = workspace.output_path_memo_block_number;
    workspace.build_plan.total_items = workspace.entries.size();
    workspace.build_plan.excluded_items = static_cast<std::size_t>(std::count_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.excluded;
        }));
    workspace.build_plan.deleted_items = static_cast<std::size_t>(std::count_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.deleted;
        }));

    const auto startup_entry = std::find_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.main_program && !entry.excluded && !entry.deleted;
        });
    const auto program_entry = std::find_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.group_id == "programs" && !entry.excluded && !entry.deleted;
        });
    const auto first_non_header = std::find_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.group_id != "project" && !entry.excluded && !entry.deleted;
        });
    const auto startup_entry_any = std::find_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.main_program && !entry.deleted;
        });
    const auto program_entry_any = std::find_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.group_id == "programs" && !entry.deleted;
        });
    const auto first_non_header_any = std::find_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.group_id != "project" && !entry.deleted;
        });
    const auto deleted_startup_entry_any = std::find_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.main_program;
        });
    const auto deleted_program_entry_any = std::find_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.group_id == "programs";
        });
    const auto deleted_first_non_header_any = std::find_if(
        workspace.entries.begin(),
        workspace.entries.end(),
        [](const StudioProjectEntry& entry) {
            return entry.group_id != "project";
        });

    const StudioProjectEntry* startup = nullptr;
    if (startup_entry != workspace.entries.end()) {
        startup = &(*startup_entry);
    } else if (program_entry != workspace.entries.end()) {
        startup = &(*program_entry);
    } else if (first_non_header != workspace.entries.end()) {
        startup = &(*first_non_header);
    } else if (startup_entry_any != workspace.entries.end()) {
        startup = &(*startup_entry_any);
    } else if (program_entry_any != workspace.entries.end()) {
        startup = &(*program_entry_any);
    } else if (first_non_header_any != workspace.entries.end()) {
        startup = &(*first_non_header_any);
    } else if (deleted_startup_entry_any != workspace.entries.end()) {
        startup = &(*deleted_startup_entry_any);
    } else if (deleted_program_entry_any != workspace.entries.end()) {
        startup = &(*deleted_program_entry_any);
    } else if (deleted_first_non_header_any != workspace.entries.end()) {
        startup = &(*deleted_first_non_header_any);
    }

    if (header_record != document.table_preview.records.end()) {
        workspace.build_plan.project_key_field_index = field_index_or_missing(*header_record, "KEY");
        workspace.build_plan.home_directory_field_index = field_index_or_missing(*header_record, "HOMEDIR");
        workspace.build_plan.output_path_field_index = workspace.output_path_field_index;
        workspace.build_plan.debug_enabled = value_as_bool(*header_record, "DEBUG");
        workspace.build_plan.debug_field_index = field_index_or_missing(*header_record, "DEBUG");
        workspace.build_plan.debug_memo_block_number = memo_block_number_or_zero(*header_record, "DEBUG");
        workspace.build_plan.encrypt_enabled = value_as_bool(*header_record, "ENCRYPT");
        workspace.build_plan.encrypt_field_index = field_index_or_missing(*header_record, "ENCRYPT");
        workspace.build_plan.encrypt_memo_block_number = memo_block_number_or_zero(*header_record, "ENCRYPT");
        workspace.build_plan.save_code = value_as_bool(*header_record, "SAVECODE");
        workspace.build_plan.save_code_field_index = field_index_or_missing(*header_record, "SAVECODE");
        workspace.build_plan.save_code_memo_block_number = memo_block_number_or_zero(*header_record, "SAVECODE");
        workspace.build_plan.no_logo = value_as_bool(*header_record, "NOLOGO");
        workspace.build_plan.no_logo_field_index = field_index_or_missing(*header_record, "NOLOGO");
        workspace.build_plan.no_logo_memo_block_number = memo_block_number_or_zero(*header_record, "NOLOGO");
    }

    if (startup != nullptr) {
        workspace.build_plan.startup_item = startup->name;
        workspace.build_plan.startup_item_field_index = startup->name_field_index;
        workspace.build_plan.startup_item_memo_block_number = startup->name_memo_block_number;
        workspace.build_plan.startup_record_index = startup->record_index;
    }

    workspace.build_plan.can_build =
        !workspace.build_plan.output_path.empty() &&
        !workspace.entries.empty();

    return workspace;
}

}  // namespace copperfin::studio

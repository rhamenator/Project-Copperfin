// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"

#include "copperfin/security/physical_path_containment.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace copperfin::runtime::runtime_pipeline_detail {
namespace {

bool has_windows_drive_designator(const std::string& value) {
    return value.size() >= 2U &&
        std::isalpha(static_cast<unsigned char>(value[0])) != 0 &&
        value[1] == ':';
}

bool is_unc_path(const std::string& value) {
    return value.size() >= 2U &&
        ((value[0] == '\\' && value[1] == '\\') ||
         (value[0] == '/' && value[1] == '/'));
}

bool is_windows_reserved_device_name(std::string component) {
    const std::size_t extension = component.find('.');
    if (extension != std::string::npos) {
        component.erase(extension);
    }
    while (!component.empty() &&
           (component.back() == '.' || component.back() == ' ')) {
        component.pop_back();
    }
    std::transform(
        component.begin(),
        component.end(),
        component.begin(),
        [](const unsigned char ch) {
            return ch >= 'a' && ch <= 'z'
                ? static_cast<char>(ch - 'a' + 'A')
                : static_cast<char>(ch);
        });
    if (component == "CON" || component == "PRN" || component == "AUX" ||
        component == "NUL") {
        return true;
    }
    const bool numbered_device =
        component.rfind("COM", 0U) == 0U || component.rfind("LPT", 0U) == 0U;
    if (!numbered_device) {
        return false;
    }
    if (component.size() == 4U) {
        return component[3] >= '1' && component[3] <= '9';
    }
    if (component.size() != 5U ||
        static_cast<unsigned char>(component[3]) != 0xC2U) {
        return false;
    }
    const unsigned char superscript = static_cast<unsigned char>(component[4]);
    return superscript == 0xB9U || superscript == 0xB2U || superscript == 0xB3U;
}

bool is_portable_package_component(const std::filesystem::path& component) {
    const std::string value = component.generic_string();
    if (value.empty() || value == "." || value == ".." ||
        value.back() == '.' || value.back() == ' ' ||
        is_windows_reserved_device_name(value)) {
        return false;
    }
    return std::none_of(value.begin(), value.end(), [](const unsigned char ch) {
        return ch < 0x20U || ch == '<' || ch == '>' || ch == ':' || ch == '"' ||
            ch == '|' || ch == '?' || ch == '*';
    });
}

std::optional<std::filesystem::path> admitted_relative_path(
    const std::filesystem::path& path) {
    std::string normalized = path.generic_string();
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    const std::filesystem::path relative(normalized);
    if (normalized.empty() || has_windows_drive_designator(normalized) ||
        is_unc_path(normalized) || relative.has_root_path() ||
        relative.is_absolute() || relative.filename().empty()) {
        return std::nullopt;
    }
    for (const auto& component : relative) {
        if (!is_portable_package_component(component)) {
            return std::nullopt;
        }
    }
    return relative.lexically_normal();
}

enum class DirectDirectoryState {
    direct,
    rejected,
    unavailable
};

DirectDirectoryState inspect_direct_directory(const std::filesystem::path& path) {
#if defined(_WIN32)
    const DWORD attributes = ::GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return DirectDirectoryState::unavailable;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0U &&
            (attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0U
        ? DirectDirectoryState::direct
        : DirectDirectoryState::rejected;
#else
    struct stat status{};
    if (::lstat(path.c_str(), &status) != 0) {
        return DirectDirectoryState::unavailable;
    }
    return S_ISDIR(status.st_mode)
        ? DirectDirectoryState::direct
        : DirectDirectoryState::rejected;
#endif
}

bool paths_equal_for_platform(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
#if defined(_WIN32)
    const std::wstring left_value = left.lexically_normal().native();
    const std::wstring right_value = right.lexically_normal().native();
    return ::CompareStringOrdinal(
               left_value.data(),
               static_cast<int>(left_value.size()),
               right_value.data(),
               static_cast<int>(right_value.size()),
               TRUE) == CSTR_EQUAL;
#else
    return left.lexically_normal() == right.lexically_normal();
#endif
}

bool is_containment_policy_rejection(
    const security::PhysicalPathContainmentFailure failure) {
    return failure == security::PhysicalPathContainmentFailure::outside_root ||
        failure == security::PhysicalPathContainmentFailure::indirect_component ||
        failure == security::PhysicalPathContainmentFailure::cross_device_component ||
        failure == security::PhysicalPathContainmentFailure::not_regular_file;
}

std::string rejected_destination(const std::filesystem::path& path) {
    return runtime_text(
        "Runtime.Package.Error.ContentDestinationRejected",
        {{"path", path.string()}});
}

std::string rejected_content_root(const std::filesystem::path& path) {
    return runtime_text(
        "Runtime.Package.Error.ContentRootRejected",
        {{"path", path.string()}});
}

std::string content_root_creation_failed() {
    return runtime_text("Runtime.Package.Error.CreateContentRootFailed");
}

std::string directory_creation_failed(const std::filesystem::path& path) {
    return runtime_text(
        "Runtime.Package.Error.CreateDirectoryFailed",
        {{"path", path.string()}});
}

std::string copy_file_failed(const std::filesystem::path& path) {
    return runtime_text(
        "Runtime.Package.Error.CopyFileFailed",
        {{"path", path.string()}});
}

bool prepare_direct_parent(
    const std::filesystem::path& content_root,
    const std::filesystem::path& relative_parent,
    const std::filesystem::path& reported_destination,
    std::string& error) {
    const DirectDirectoryState root_state = inspect_direct_directory(content_root);
    if (root_state != DirectDirectoryState::direct) {
        error = root_state == DirectDirectoryState::rejected
            ? rejected_destination(reported_destination)
            : directory_creation_failed(content_root);
        return false;
    }
    const auto root_containment =
        security::inspect_physical_path_containment(content_root, content_root);
    if (!root_containment.allowed) {
        error = is_containment_policy_rejection(root_containment.failure)
            ? rejected_destination(reported_destination)
            : directory_creation_failed(content_root);
        return false;
    }

    std::filesystem::path current = content_root;
    for (const auto& component : relative_parent) {
        current /= component;
        std::error_code status_error;
        const auto status = std::filesystem::symlink_status(current, status_error);
        if (status_error && status_error != std::errc::no_such_file_or_directory) {
            error = directory_creation_failed(current);
            return false;
        }
        if (!std::filesystem::exists(status)) {
            std::error_code create_error;
            if (!std::filesystem::create_directory(current, create_error) || create_error) {
                error = directory_creation_failed(current);
                return false;
            }
        }
        const DirectDirectoryState current_state = inspect_direct_directory(current);
        if (current_state != DirectDirectoryState::direct) {
            error = current_state == DirectDirectoryState::rejected
                ? rejected_destination(reported_destination)
                : directory_creation_failed(current);
            return false;
        }
        const auto containment =
            security::inspect_physical_path_containment(current, content_root);
        if (!containment.allowed) {
            error = is_containment_policy_rejection(containment.failure)
                ? rejected_destination(reported_destination)
                : directory_creation_failed(current);
            return false;
        }
    }
    return true;
}

}  // namespace

bool prepare_package_content_root(
    const std::filesystem::path& package_root,
    const std::filesystem::path& content_root,
    std::string& error) {
    std::error_code filesystem_error;
    const std::filesystem::path absolute_package_root =
        std::filesystem::absolute(package_root, filesystem_error).lexically_normal();
    if (filesystem_error) {
        error = content_root_creation_failed();
        return false;
    }
    const std::filesystem::path absolute_content_root =
        std::filesystem::absolute(content_root, filesystem_error).lexically_normal();
    if (filesystem_error) {
        error = content_root_creation_failed();
        return false;
    }
    const DirectDirectoryState package_root_state =
        inspect_direct_directory(absolute_package_root);
    if (!paths_equal_for_platform(
            absolute_content_root,
            absolute_package_root / "content")) {
        error = rejected_content_root(content_root);
        return false;
    }
    if (package_root_state != DirectDirectoryState::direct) {
        error = package_root_state == DirectDirectoryState::rejected
            ? rejected_content_root(content_root)
            : content_root_creation_failed();
        return false;
    }

    const auto package_containment = security::inspect_physical_path_containment(
        absolute_package_root,
        absolute_package_root);
    if (!package_containment.allowed) {
        error = is_containment_policy_rejection(package_containment.failure)
            ? rejected_content_root(content_root)
            : content_root_creation_failed();
        return false;
    }

    const auto status =
        std::filesystem::symlink_status(absolute_content_root, filesystem_error);
    if (filesystem_error == std::errc::no_such_file_or_directory) {
        filesystem_error.clear();
    } else if (filesystem_error) {
        error = content_root_creation_failed();
        return false;
    }
    if (!std::filesystem::exists(status)) {
        if (!std::filesystem::create_directory(
                absolute_content_root,
                filesystem_error) ||
            filesystem_error) {
            error = content_root_creation_failed();
            return false;
        }
    }
    const DirectDirectoryState content_root_state =
        inspect_direct_directory(absolute_content_root);
    if (content_root_state != DirectDirectoryState::direct) {
        error = content_root_state == DirectDirectoryState::rejected
            ? rejected_content_root(content_root)
            : content_root_creation_failed();
        return false;
    }
    const auto content_containment = security::inspect_physical_path_containment(
        absolute_content_root,
        absolute_package_root);
    if (!content_containment.allowed) {
        error = is_containment_policy_rejection(content_containment.failure)
            ? rejected_content_root(content_root)
            : content_root_creation_failed();
        return false;
    }
    return true;
}

bool copy_file_to_package_content(
    const std::filesystem::path& source,
    const std::filesystem::path& package_root,
    const std::filesystem::path& content_root,
    const std::filesystem::path& relative_path,
    std::filesystem::path& destination,
    std::string& error) {
    destination.clear();
    const auto admitted = admitted_relative_path(relative_path);
    if (!admitted.has_value()) {
        error = rejected_destination(relative_path);
        return false;
    }
    if (!prepare_package_content_root(package_root, content_root, error)) {
        return false;
    }

    std::error_code filesystem_error;
    const std::filesystem::path absolute_root =
        std::filesystem::absolute(content_root, filesystem_error).lexically_normal();
    if (filesystem_error) {
        error = directory_creation_failed(content_root);
        return false;
    }
    if (!prepare_direct_parent(
            absolute_root,
            admitted->parent_path(),
            content_root / *admitted,
            error)) {
        return false;
    }
    destination = (content_root / *admitted).lexically_normal();
    const std::filesystem::path write_destination =
        (absolute_root / *admitted).lexically_normal();

    const auto status =
        std::filesystem::symlink_status(write_destination, filesystem_error);
    if (filesystem_error && filesystem_error != std::errc::no_such_file_or_directory) {
        error = copy_file_failed(destination);
        return false;
    }
    if (std::filesystem::exists(status)) {
        const auto containment =
            security::inspect_physical_path_containment(write_destination, absolute_root);
        if (!containment.allowed) {
            error = is_containment_policy_rejection(containment.failure)
                ? rejected_destination(destination)
                : copy_file_failed(destination);
            return false;
        }
        if (containment.identity.link_count != 1U ||
            !std::filesystem::is_regular_file(status)) {
            error = rejected_destination(destination);
            return false;
        }
    }

    if (!copy_file_if_exists(source, write_destination, error)) {
        return false;
    }
    const auto copied =
        security::inspect_physical_path_containment(write_destination, absolute_root);
    if (!copied.allowed) {
        error = is_containment_policy_rejection(copied.failure)
            ? rejected_destination(destination)
            : copy_file_failed(destination);
        return false;
    }
    if (copied.identity.link_count != 1U) {
        error = rejected_destination(destination);
        return false;
    }
    return true;
}

}  // namespace copperfin::runtime::runtime_pipeline_detail

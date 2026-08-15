// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_environment.h"

#include "copperfin/platform/path.h"
#include "copperfin/platform/private_directory.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <limits>
#include <string_view>
#include <system_error>
#include <utility>

namespace copperfin::security {
namespace {

using CapturedDirectory = PhysicalPathContainmentResult;

bool path_has_embedded_nul(const std::filesystem::path& path) {
    const auto& native = path.native();
    return native.find(typename std::filesystem::path::value_type{}) !=
        std::filesystem::path::string_type::npos;
}

bool path_has_dot_component(const std::filesystem::path& path) {
    for (const auto& component : path) {
        if (component == "." || component == "..") {
            return true;
        }
    }
    return false;
}

#if defined(_WIN32)
bool path_has_remote_or_device_root(const std::filesystem::path& path) {
    const auto& native = path.native();
    if (native.rfind(L"\\\\?\\", 0U) == 0U ||
        native.rfind(L"\\\\.\\", 0U) == 0U ||
        native.rfind(L"\\??\\", 0U) == 0U) {
        return true;
    }
    const auto& root = path.root_name().native();
    const auto is_separator = [](wchar_t value) {
        return value == L'\\' || value == L'/';
    };
    return root.size() >= 2U && is_separator(root[0]) && is_separator(root[1]);
}
#else
bool path_has_remote_or_device_root(const std::filesystem::path&) {
    return false;
}
#endif

char path_list_separator() noexcept {
#if defined(_WIN32)
    return ';';
#else
    return ':';
#endif
}

bool valid_utf8(std::string_view value) noexcept {
    std::size_t offset = 0U;
    while (offset < value.size()) {
        const auto lead = static_cast<unsigned char>(value[offset]);
        std::size_t continuation_count = 0U;
        std::uint32_t scalar = 0U;
        if (lead <= 0x7fU) {
            ++offset;
            continue;
        }
        if (lead >= 0xc2U && lead <= 0xdfU) {
            continuation_count = 1U;
            scalar = lead & 0x1fU;
        } else if (lead >= 0xe0U && lead <= 0xefU) {
            continuation_count = 2U;
            scalar = lead & 0x0fU;
        } else if (lead >= 0xf0U && lead <= 0xf4U) {
            continuation_count = 3U;
            scalar = lead & 0x07U;
        } else {
            return false;
        }
        if (continuation_count > value.size() - offset - 1U) {
            return false;
        }
        for (std::size_t index = 1U; index <= continuation_count; ++index) {
            const auto continuation =
                static_cast<unsigned char>(value[offset + index]);
            if ((continuation & 0xc0U) != 0x80U) {
                return false;
            }
            scalar = (scalar << 6U) | (continuation & 0x3fU);
        }
        if ((continuation_count == 1U && scalar < 0x80U) ||
            (continuation_count == 2U && scalar < 0x800U) ||
            (continuation_count == 3U && scalar < 0x10000U) ||
            scalar > 0x10ffffU ||
            (scalar >= 0xd800U && scalar <= 0xdfffU)) {
            return false;
        }
        offset += continuation_count + 1U;
    }
    return true;
}

std::string session_directory_name(std::uint64_t generation) {
    std::array<char, std::numeric_limits<std::uint64_t>::digits10 + 2U> digits{};
    const auto converted = std::to_chars(
        digits.data(), digits.data() + digits.size(), generation);
    if (converted.ec != std::errc{}) {
        return {};
    }
    return "session-" + std::string(digits.data(), converted.ptr);
}

bool strict_absolute_directory_spelling(const std::filesystem::path& path) {
    if (path.empty() || path_has_embedded_nul(path) || !path.is_absolute() ||
        path_has_dot_component(path) || path_has_remote_or_device_root(path)) {
        return false;
    }
    const std::string utf8 = copperfin::platform::path_to_utf8_string(path);
    return !utf8.empty() && valid_utf8(utf8) &&
        utf8.find(path_list_separator()) == std::string::npos;
}

std::optional<CapturedDirectory> capture_directory(
    const std::filesystem::path& path) {
    if (!strict_absolute_directory_spelling(path)) {
        return std::nullopt;
    }
    const auto inspected = inspect_physical_path_containment(path, path);
    if (!inspected.allowed) {
        return std::nullopt;
    }
    std::error_code error;
    if (!std::filesystem::is_directory(inspected.canonical_path, error) || error) {
        return std::nullopt;
    }
    const std::string utf8 =
        copperfin::platform::path_to_utf8_string(inspected.canonical_path);
    if (utf8.empty() || !valid_utf8(utf8) ||
        utf8.find(path_list_separator()) != std::string::npos ||
        utf8.size() > workspace_agent_environment_max_entry_bytes) {
        return std::nullopt;
    }
    return inspected;
}

bool captured_directory_matches(const CapturedDirectory& captured) {
    const auto current = inspect_physical_path_containment(
        captured.canonical_path, captured.canonical_path);
    // Directory size, link count, and modification time are mutable namespace
    // metadata: creating a later session layout legitimately changes them on
    // the storage root. Storage and file identity bind the directory object;
    // the physical inspection and type check continue to reject redirection,
    // replacement, and wrong-kind targets.
    if (!current.allowed ||
        current.identity.storage_id != captured.identity.storage_id ||
        current.identity.file_id != captured.identity.file_id) {
        return false;
    }
    std::error_code error;
    return std::filesystem::is_directory(current.canonical_path, error) && !error;
}

std::optional<CapturedDirectory> capture_contained_directory(
    const std::filesystem::path& path,
    const CapturedDirectory& root) {
    const auto inspected = inspect_physical_path_containment(
        path, root.canonical_path);
    if (!inspected.allowed) {
        return std::nullopt;
    }
    std::error_code error;
    if (!std::filesystem::is_directory(inspected.canonical_path, error) || error ||
        !copperfin::platform::verify_private_directory(
             inspected.canonical_path).ok) {
        return std::nullopt;
    }
    return inspected;
}

bool same_configured_path(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
#if defined(_WIN32)
    return copperfin::platform::path_equal_case_insensitive(left, right);
#else
    return left == right;
#endif
}

std::string path_value(const CapturedDirectory& directory) {
    return copperfin::platform::path_to_utf8_string(directory.canonical_path);
}

std::string joined_path_value(const std::vector<CapturedDirectory>& directories) {
    std::string value;
    for (const auto& directory : directories) {
        const std::string component = path_value(directory);
        if (component.empty()) {
            return {};
        }
        if (!value.empty()) {
            value.push_back(path_list_separator());
        }
        value += component;
    }
    return value;
}

bool entry_less(
    const WorkspaceAgentEnvironmentEntry& left,
    const WorkspaceAgentEnvironmentEntry& right) {
#if defined(_WIN32)
    return std::lexicographical_compare(
        left.name.begin(), left.name.end(), right.name.begin(), right.name.end(),
        [](unsigned char lhs, unsigned char rhs) {
            const auto fold = [](unsigned char value) {
                return value >= 'A' && value <= 'Z'
                    ? static_cast<unsigned char>(value + ('a' - 'A'))
                    : value;
            };
            return fold(lhs) < fold(rhs);
        });
#else
    return left.name < right.name;
#endif
}

bool entries_within_limits(
    const std::vector<WorkspaceAgentEnvironmentEntry>& entries) {
    std::size_t total = 0U;
    for (const auto& entry : entries) {
        if (entry.name.empty() || entry.name.find('=') != std::string::npos ||
            entry.name.find('\0') != std::string::npos ||
            entry.value.find('\0') != std::string::npos ||
            !valid_utf8(entry.value)) {
            return false;
        }
        const std::size_t bytes = entry.name.size() + 1U + entry.value.size();
        if (bytes > workspace_agent_environment_max_entry_bytes ||
            bytes > workspace_agent_environment_max_total_bytes - total) {
            return false;
        }
        total += bytes;
    }
    return true;
}

WorkspaceAgentIsolatedEnvironmentConstruction denied(std::string diagnostic) {
    WorkspaceAgentIsolatedEnvironmentConstruction result;
    result.diagnostic_code = std::move(diagnostic);
    return result;
}

}  // namespace

WorkspaceAgentProcessEnvironmentPlatform
workspace_agent_process_environment_host_platform() noexcept {
#if defined(_WIN32)
    return WorkspaceAgentProcessEnvironmentPlatform::windows_v1;
#else
    return WorkspaceAgentProcessEnvironmentPlatform::posix_v1;
#endif
}

WorkspaceAgentIsolatedEnvironmentBoundary::WorkspaceAgentIsolatedEnvironmentBoundary(
    PhysicalPathContainmentResult session_storage_root,
    std::vector<PhysicalPathContainmentResult> executable_directories,
    std::optional<PhysicalPathContainmentResult> windows_system_root)
    : session_storage_root_(std::move(session_storage_root)),
      executable_directories_(std::move(executable_directories)),
      windows_system_root_(std::move(windows_system_root)) {}

std::optional<WorkspaceAgentIsolatedEnvironmentBoundary>
WorkspaceAgentIsolatedEnvironmentBoundary::create(
    const WorkspaceAgentIsolatedEnvironmentConfiguration& configuration) {
    if (configuration.schema_version != 1U ||
        configuration.trusted_executable_directories.empty() ||
        configuration.trusted_executable_directories.size() >
            workspace_agent_environment_max_path_directories) {
        return std::nullopt;
    }
    auto storage_root = capture_directory(
        configuration.trusted_session_storage_root);
    if (!storage_root.has_value()) {
        return std::nullopt;
    }
    if (!copperfin::platform::verify_private_directory(
             storage_root->canonical_path).ok) {
        return std::nullopt;
    }

    std::vector<PhysicalPathContainmentResult> executable_directories;
    executable_directories.reserve(
        configuration.trusted_executable_directories.size());
    for (const auto& path : configuration.trusted_executable_directories) {
        auto captured = capture_directory(path);
        if (!captured.has_value()) {
            return std::nullopt;
        }
        if (std::any_of(
                executable_directories.begin(), executable_directories.end(),
                [&captured](const PhysicalPathContainmentResult& existing) {
                    return same_configured_path(
                        existing.canonical_path, captured->canonical_path);
                })) {
            return std::nullopt;
        }
        executable_directories.push_back(std::move(*captured));
    }

    std::optional<PhysicalPathContainmentResult> windows_system_root;
#if defined(_WIN32)
    windows_system_root = capture_directory(
        configuration.trusted_windows_system_root);
    if (!windows_system_root.has_value()) {
        return std::nullopt;
    }
#else
    if (!configuration.trusted_windows_system_root.empty()) {
        return std::nullopt;
    }
#endif

    return WorkspaceAgentIsolatedEnvironmentBoundary(
        std::move(*storage_root),
        std::move(executable_directories),
        std::move(windows_system_root));
}

WorkspaceAgentSessionLayoutPreparationResult
WorkspaceAgentIsolatedEnvironmentBoundary::prepare_session_layout(
    const std::uint64_t session_generation) const {
    WorkspaceAgentSessionLayoutPreparationResult result;
    if (session_generation == 0U) {
        result.diagnostic_code =
            "workspace_agent.environment_invalid_session_generation";
        return result;
    }
    if (!captured_directory_matches(session_storage_root_) ||
        !copperfin::platform::verify_private_directory(
             session_storage_root_.canonical_path).ok) {
        result.diagnostic_code =
            "workspace_agent.environment_storage_root_identity_changed";
        return result;
    }

    const std::string session_name = session_directory_name(session_generation);
    if (session_name.empty()) {
        result.diagnostic_code =
            "workspace_agent.environment_invalid_session_generation";
        return result;
    }
    const std::filesystem::path session_root =
        session_storage_root_.canonical_path / session_name;
    const auto session_created =
        copperfin::platform::create_private_directory(session_root);
    if (!session_created.ok) {
        result.diagnostic_code =
            session_created.failure ==
                    copperfin::platform::PrivateDirectoryFailure::already_exists
                ? "workspace_agent.environment_session_layout_exists"
                : "workspace_agent.environment_session_layout_creation_failed";
        return result;
    }

    constexpr std::array<std::string_view, 5U> child_names{
        "home", "temp", "config", "cache", "data"};
    for (const auto child_name : child_names) {
        const auto child_created = copperfin::platform::create_private_directory(
            session_root / child_name);
        if (!child_created.ok) {
            result.diagnostic_code =
                "workspace_agent.environment_session_layout_incomplete";
            return result;
        }
    }

    if (!captured_directory_matches(session_storage_root_) ||
        !copperfin::platform::verify_private_directory(
             session_storage_root_.canonical_path).ok ||
        !capture_contained_directory(
             session_root, session_storage_root_).has_value()) {
        result.diagnostic_code =
            "workspace_agent.environment_session_layout_verification_failed";
        return result;
    }
    for (const auto child_name : child_names) {
        if (!capture_contained_directory(
                 session_root / child_name,
                 session_storage_root_).has_value()) {
            result.diagnostic_code =
                "workspace_agent.environment_session_layout_verification_failed";
            return result;
        }
    }

    result.prepared = true;
    result.session_generation = session_generation;
    result.diagnostic_code =
        "workspace_agent.environment_session_layout_prepared";
    return result;
}

WorkspaceAgentIsolatedEnvironmentConstruction
WorkspaceAgentIsolatedEnvironmentBoundary::construct(
    const std::uint64_t session_generation,
    const WorkspaceAgentProcessEnvironmentPolicy policy) const {
    if (session_generation == 0U) {
        return denied("workspace_agent.environment_invalid_session_generation");
    }
    if (policy != WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1) {
        return denied("workspace_agent.environment_invalid_policy");
    }
    if (!captured_directory_matches(session_storage_root_) ||
        !copperfin::platform::verify_private_directory(
             session_storage_root_.canonical_path).ok) {
        return denied("workspace_agent.environment_storage_root_identity_changed");
    }
    for (const auto& directory : executable_directories_) {
        if (!captured_directory_matches(directory)) {
            return denied("workspace_agent.environment_path_identity_changed");
        }
    }
    if (windows_system_root_.has_value() &&
        !captured_directory_matches(*windows_system_root_)) {
        return denied("workspace_agent.environment_system_root_identity_changed");
    }

    const std::string session_name = session_directory_name(session_generation);
    if (session_name.empty()) {
        return denied("workspace_agent.environment_invalid_session_generation");
    }
    const std::filesystem::path session_root =
        session_storage_root_.canonical_path / session_name;
    const auto session = capture_contained_directory(
        session_root, session_storage_root_);
    if (!session.has_value()) {
        return denied("workspace_agent.environment_session_directory_unavailable");
    }
    const auto home = capture_contained_directory(session_root / "home", *session);
    const auto temporary = capture_contained_directory(session_root / "temp", *session);
    const auto config = capture_contained_directory(session_root / "config", *session);
    const auto cache = capture_contained_directory(session_root / "cache", *session);
    const auto data = capture_contained_directory(session_root / "data", *session);
    if (!home.has_value() || !temporary.has_value() || !config.has_value() ||
        !cache.has_value() || !data.has_value()) {
        return denied("workspace_agent.environment_session_layout_unavailable");
    }

    const std::string home_value = path_value(*home);
    const std::string temporary_value = path_value(*temporary);
    const std::string config_value = path_value(*config);
    const std::string cache_value = path_value(*cache);
    const std::string data_value = path_value(*data);
    const std::string search_path = joined_path_value(executable_directories_);
    if (home_value.empty() || temporary_value.empty() || config_value.empty() ||
        cache_value.empty() || data_value.empty() || search_path.empty()) {
        return denied("workspace_agent.environment_path_encoding_failed");
    }

    std::vector<WorkspaceAgentEnvironmentEntry> entries;
#if defined(_WIN32)
    const std::string system_root = path_value(*windows_system_root_);
    if (system_root.empty()) {
        return denied("workspace_agent.environment_path_encoding_failed");
    }
    entries = {
        {"APPDATA", config_value},
        {"HOME", home_value},
        {"LOCALAPPDATA", data_value},
        {"PATH", search_path},
        {"SystemRoot", system_root},
        {"TEMP", temporary_value},
        {"TMP", temporary_value},
        {"TZ", "UTC"},
        {"USERPROFILE", home_value},
        {"WINDIR", system_root}};
#else
    entries = {
        {"HOME", home_value},
        {"LANG", "C"},
        {"LC_ALL", "C"},
        {"PATH", search_path},
        {"TMPDIR", temporary_value},
        {"TZ", "UTC"},
        {"XDG_CACHE_HOME", cache_value},
        {"XDG_CONFIG_HOME", config_value},
        {"XDG_DATA_HOME", data_value}};
#endif
    std::sort(entries.begin(), entries.end(), entry_less);
    if (!entries_within_limits(entries)) {
        return denied("workspace_agent.environment_size_limit_exceeded");
    }

    if (!copperfin::platform::verify_private_directory(
             session_storage_root_.canonical_path).ok) {
        return denied("workspace_agent.environment_identity_changed");
    }
    const std::array<const PhysicalPathContainmentResult*, 6U> final_directories{
        &session_storage_root_, &*session, &*home, &*temporary, &*config, &*cache};
    for (const auto* directory : final_directories) {
        if (!captured_directory_matches(*directory)) {
            return denied("workspace_agent.environment_identity_changed");
        }
    }
    if (!captured_directory_matches(*data)) {
        return denied("workspace_agent.environment_identity_changed");
    }
    for (const auto& directory : executable_directories_) {
        if (!captured_directory_matches(directory)) {
            return denied("workspace_agent.environment_path_identity_changed");
        }
    }
    if (windows_system_root_.has_value() &&
        !captured_directory_matches(*windows_system_root_)) {
        return denied("workspace_agent.environment_system_root_identity_changed");
    }

    return {
        .allowed = true,
        .session_generation = session_generation,
        .policy = policy,
        .platform = workspace_agent_process_environment_host_platform(),
        .entries = std::move(entries),
        .diagnostic_code = "workspace_agent.environment_constructed"};
}

}  // namespace copperfin::security

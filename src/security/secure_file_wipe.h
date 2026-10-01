// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace copperfin::security {

// Best-effort removal of a file or directory tree that held a copy of table data (a command-undo or
// transaction pre-image): each regular file is overwritten with zeros, flushed, and then the tree is
// deleted. A plain delete only unlinks the name and leaves the bytes on disk.
//
// This is defense in depth, not a guarantee: on copy-on-write or journaling filesystems, SSDs with
// wear levelling, and snapshotted or backed-up volumes the old blocks can survive an overwrite.
// Symbolic links are never followed or overwritten. Never throws.
inline void secure_wipe_file(const std::filesystem::path& file) noexcept {
    try {
        std::error_code error;
        const std::uintmax_t size = std::filesystem::file_size(file, error);
        if (error || size == 0U) {
            return;
        }
        std::ofstream stream(file, std::ios::binary | std::ios::in | std::ios::out);
        if (!stream) {
            return;
        }
        const std::array<char, 65536> zeros{};
        std::uintmax_t remaining = size;
        while (remaining > 0U && stream) {
            const auto chunk = static_cast<std::streamsize>(std::min<std::uintmax_t>(remaining, zeros.size()));
            stream.write(zeros.data(), chunk);
            remaining -= static_cast<std::uintmax_t>(chunk);
        }
        stream.flush();
    } catch (...) {
    }
}

inline void secure_remove_tree(const std::filesystem::path& root) noexcept {
    try {
        std::error_code error;
        const auto status = std::filesystem::symlink_status(root, error);
        if (error || !std::filesystem::exists(status)) {
            return;
        }
        if (std::filesystem::is_regular_file(status)) {
            secure_wipe_file(root);
        } else if (std::filesystem::is_directory(status)) {
            for (std::filesystem::recursive_directory_iterator it(
                     root, std::filesystem::directory_options::skip_permission_denied, error);
                 !error && it != std::filesystem::recursive_directory_iterator();
                 it.increment(error)) {
                std::error_code entry_error;
                if (it->symlink_status(entry_error).type() == std::filesystem::file_type::regular) {
                    secure_wipe_file(it->path());
                }
            }
        }
        error.clear();
        std::filesystem::remove_all(root, error);
    } catch (...) {
    }
}

}  // namespace copperfin::security

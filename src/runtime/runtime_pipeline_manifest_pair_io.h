// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace copperfin::runtime::runtime_pipeline_detail {

enum class ManifestPairEntryKind {
    missing,
    regular,
    directory,
    indirect,
    other,
    unavailable
};

enum class ManifestPairDirectoryAcquireFailure {
    none,
    path_rejected,
    busy
};

class ManifestPairDirectory {
public:
    ManifestPairDirectory() = default;
    ManifestPairDirectory(const ManifestPairDirectory&) = delete;
    ManifestPairDirectory& operator=(const ManifestPairDirectory&) = delete;
    ~ManifestPairDirectory();

    bool acquire(
        const std::filesystem::path& root,
        const std::string& transaction_identity);
    [[nodiscard]] ManifestPairDirectoryAcquireFailure acquire_failure() const;
    [[nodiscard]] std::filesystem::path full_path(
        const std::filesystem::path& leaf) const;
    [[nodiscard]] ManifestPairEntryKind entry_kind(
        const std::filesystem::path& leaf) const;
    bool read_direct_file(
        const std::filesystem::path& leaf,
        std::string& bytes) const;
    bool create_direct_file_and_flush(
        const std::filesystem::path& leaf,
        const std::string& contents) const;
    bool move_direct_file_no_replace(
        const std::filesystem::path& source_leaf,
        const std::filesystem::path& destination_leaf) const;
    bool remove_direct_file(const std::filesystem::path& leaf) const;

private:
    [[nodiscard]] bool valid_leaf(const std::filesystem::path& leaf) const;
    bool synchronize_directory() const;

    std::filesystem::path root_;
    ManifestPairDirectoryAcquireFailure acquire_failure_ =
        ManifestPairDirectoryAcquireFailure::path_rejected;
#if defined(_WIN32)
    void* directory_handle_ = nullptr;
    void* mutex_handle_ = nullptr;
    bool mutex_owned_ = false;
    std::uint64_t volume_id_ = 0U;
#else
    int descriptor_ = -1;
    std::uint64_t storage_id_ = 0U;
#endif
};

}  // namespace copperfin::runtime::runtime_pipeline_detail

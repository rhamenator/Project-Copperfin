// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

#include "runtime_pipeline_manifest_pair_io.h"

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
#include "runtime_pipeline_test_hooks.h"
#endif

#include <array>
#include <atomic>
#include <optional>
#include <unordered_map>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace copperfin::runtime {
namespace {

using runtime_pipeline_detail::ManifestPairDirectory;
using runtime_pipeline_detail::ManifestPairDirectoryAcquireFailure;
using runtime_pipeline_detail::ManifestPairEntryKind;

constexpr std::string_view kManifestPairMarker =
    ".copperfin-manifest-pair.transaction";
constexpr std::string_view kManifestPairNextSuffix =
    ".copperfin-manifest-pair-next";
constexpr std::string_view kManifestPairPreviousSuffix =
    ".copperfin-manifest-pair-previous";

struct ManifestPairJournal {
    std::string identity;
    std::array<std::string, 2U> old_hashes;
    std::array<std::string, 2U> new_hashes;
};

struct DirectFileState {
    ManifestPairEntryKind kind = ManifestPairEntryKind::missing;
    std::string hash;
};

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
std::atomic<test_hooks::ManifestPairPromotionFault> manifest_pair_promotion_fault{
    test_hooks::ManifestPairPromotionFault::none};
#endif

bool paths_equal(const std::filesystem::path& left, const std::filesystem::path& right) {
#if defined(_WIN32)
    const auto left_value = left.lexically_normal().native();
    const auto right_value = right.lexically_normal().native();
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

bool valid_configured_leaf(const std::filesystem::path& leaf) {
    if (leaf.empty() || leaf == "." || leaf == ".." || leaf.has_parent_path()) {
        return false;
    }
#if defined(_WIN32)
    return leaf.native().find(L':') == std::wstring::npos;
#else
    return true;
#endif
}

std::optional<std::string> text_hash(const std::string& value) {
    const auto digest = security::sha256_hex_for_text(value);
    return digest.ok
        ? std::optional<std::string>(digest.hex_digest)
        : std::nullopt;
}

bool is_hex_digest(const std::string& value) {
    return value.size() == 64U &&
        std::all_of(value.begin(), value.end(), [](const unsigned char ch) {
            return std::isxdigit(ch) != 0;
        });
}

std::string build_journal_text(const ManifestPairJournal& journal) {
    std::ostringstream stream;
    stream << "copperfin_manifest_pair_transaction=1\n"
           << "identity=" << journal.identity << "\n"
           << "runtime_old_sha256=" << journal.old_hashes[0] << "\n"
           << "debug_old_sha256=" << journal.old_hashes[1] << "\n"
           << "runtime_new_sha256=" << journal.new_hashes[0] << "\n"
           << "debug_new_sha256=" << journal.new_hashes[1] << "\n";
    return stream.str();
}

std::optional<ManifestPairJournal> parse_journal_text(
    const std::string& text,
    const std::string& expected_identity) {
    std::unordered_map<std::string, std::string> fields;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        const std::size_t separator = line.find('=');
        if (separator == std::string::npos || separator == 0U ||
            !fields.emplace(line.substr(0U, separator), line.substr(separator + 1U)).second) {
            return std::nullopt;
        }
    }

    const auto value = [&](const std::string& key) -> std::optional<std::string> {
        const auto found = fields.find(key);
        return found == fields.end()
            ? std::nullopt
            : std::optional<std::string>(found->second);
    };
    const auto version = value("copperfin_manifest_pair_transaction");
    const auto identity = value("identity");
    const auto runtime_old = value("runtime_old_sha256");
    const auto debug_old = value("debug_old_sha256");
    const auto runtime_new = value("runtime_new_sha256");
    const auto debug_new = value("debug_new_sha256");
    if (fields.size() != 6U || !version.has_value() || *version != "1" ||
        !identity.has_value() || *identity != expected_identity ||
        !runtime_old.has_value() || !debug_old.has_value() ||
        !runtime_new.has_value() || !debug_new.has_value()) {
        return std::nullopt;
    }

    ManifestPairJournal journal{
        .identity = *identity,
        .old_hashes = {*runtime_old, *debug_old},
        .new_hashes = {*runtime_new, *debug_new}
    };
    if (!is_hex_digest(journal.old_hashes[0]) ||
        !is_hex_digest(journal.old_hashes[1]) ||
        !is_hex_digest(journal.new_hashes[0]) ||
        !is_hex_digest(journal.new_hashes[1])) {
        return std::nullopt;
    }
    return journal;
}

class ManifestPairTransaction {
public:
    explicit ManifestPairTransaction(const RuntimePackagePlan& plan)
        : configured_root_(copperfin::platform::path_from_utf8_string(plan.package_root)),
          configured_destinations_{
              copperfin::platform::path_from_utf8_string(plan.manifest_path),
              copperfin::platform::path_from_utf8_string(plan.debug_manifest_path)} {
    }

    bool publish(
        const std::string& runtime_contents,
        const std::string& debug_contents,
        std::string& error) {
        if (!stage(runtime_contents, debug_contents, error)) {
            return false;
        }

        if (!directory_.move_direct_file_no_replace(destination_leaves_[0], backup_leaves_[0])) {
            return fail_and_recover(
                "Runtime.Package.Error.ManifestPairPublishFailed",
                destination_leaves_[0],
                error);
        }
        if (!directory_.move_direct_file_no_replace(destination_leaves_[1], backup_leaves_[1])) {
            return fail_and_recover(
                "Runtime.Package.Error.ManifestPairPublishFailed",
                destination_leaves_[1],
                error);
        }

        if (promotion_fault_triggered(true) ||
            !directory_.move_direct_file_no_replace(stage_leaves_[0], destination_leaves_[0])) {
            return fail_and_recover(
                "Runtime.Package.Error.ManifestPairPublishFailed",
                destination_leaves_[0],
                error);
        }
        if (promotion_fault_triggered(false) ||
            !directory_.move_direct_file_no_replace(stage_leaves_[1], destination_leaves_[1])) {
            return fail_and_recover(
                "Runtime.Package.Error.ManifestPairPublishFailed",
                destination_leaves_[1],
                error);
        }

        DirectFileState runtime_state;
        DirectFileState debug_state;
        if (!read_state(destination_leaves_[0], runtime_state) ||
            !read_state(destination_leaves_[1], debug_state) ||
            runtime_state.hash != journal_->new_hashes[0] ||
            debug_state.hash != journal_->new_hashes[1]) {
            return fail_and_recover(
                "Runtime.Package.Error.ManifestPairPublishFailed",
                marker_leaf_,
                error);
        }

        if (!directory_.remove_direct_file(backup_leaves_[0]) ||
            !directory_.remove_direct_file(backup_leaves_[1]) ||
            !directory_.remove_direct_file(marker_leaf_)) {
            std::string recovery_error;
            if (!recover_existing_transaction(recovery_error)) {
                error = recovery_error;
                return false;
            }
        }
        return true;
    }

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
    bool seed_stale(
        const std::string& runtime_contents,
        const std::string& debug_contents,
        std::string& error) {
        return stage(runtime_contents, debug_contents, error);
    }
#endif

private:
    bool validate_paths(std::string& error) {
        std::error_code filesystem_error;
        root_ = std::filesystem::absolute(configured_root_, filesystem_error).lexically_normal();
        if (filesystem_error) {
            error = path_rejected(configured_root_);
            return false;
        }

        for (std::size_t index = 0U; index < configured_destinations_.size(); ++index) {
            const std::filesystem::path destination = std::filesystem::absolute(
                configured_destinations_[index],
                filesystem_error).lexically_normal();
            if (filesystem_error || !paths_equal(destination.parent_path(), root_) ||
                !valid_configured_leaf(destination.filename())) {
                error = path_rejected(configured_destinations_[index]);
                return false;
            }
            destination_leaves_[index] = destination.filename();
        }
        if (paths_equal(destination_leaves_[0], destination_leaves_[1])) {
            error = path_rejected(destination_leaves_[0]);
            return false;
        }

        marker_leaf_ = kManifestPairMarker;
        for (std::size_t index = 0U; index < destination_leaves_.size(); ++index) {
            stage_leaves_[index] = copperfin::platform::path_from_utf8_string(
                copperfin::platform::path_to_utf8_string(destination_leaves_[index]) +
                std::string(kManifestPairNextSuffix));
            backup_leaves_[index] = copperfin::platform::path_from_utf8_string(
                copperfin::platform::path_to_utf8_string(destination_leaves_[index]) +
                std::string(kManifestPairPreviousSuffix));
        }
        const std::array<std::filesystem::path, 7U> reserved{
            marker_leaf_,
            destination_leaves_[0],
            destination_leaves_[1],
            stage_leaves_[0],
            stage_leaves_[1],
            backup_leaves_[0],
            backup_leaves_[1]};
        for (std::size_t left = 0U; left < reserved.size(); ++left) {
            for (std::size_t right = left + 1U; right < reserved.size(); ++right) {
                if (paths_equal(reserved[left], reserved[right])) {
                    error = path_rejected(reserved[left]);
                    return false;
                }
            }
        }

        std::string identity_input;
#if defined(_WIN32)
        identity_input =
            runtime_pipeline_detail::canonical_casefolded_path_identity(root_) + '\0' +
            runtime_pipeline_detail::canonical_casefolded_path_identity(
                root_ / destination_leaves_[0]) + '\0' +
            runtime_pipeline_detail::canonical_casefolded_path_identity(
                root_ / destination_leaves_[1]);
#else
        identity_input =
            copperfin::platform::path_to_utf8_string(root_) + '\0' +
            copperfin::platform::path_to_utf8_string(destination_leaves_[0]) +
            '\0' + copperfin::platform::path_to_utf8_string(destination_leaves_[1]);
#endif
        const auto identity_hash = text_hash(identity_input);
        if (!identity_hash.has_value()) {
            error = path_rejected(root_);
            return false;
        }
        identity_ = *identity_hash;
        if (!directory_.acquire(root_, identity_)) {
            const std::string_view key =
                directory_.acquire_failure() == ManifestPairDirectoryAcquireFailure::busy
                ? "Runtime.Package.Error.ManifestPairTransactionCollision"
                : "Runtime.Package.Error.ManifestPairPathRejected";
            error = runtime_text(key, {{"path", copperfin::platform::path_to_utf8_string(root_)}});
            return false;
        }
        return true;
    }

    std::string path_rejected(const std::filesystem::path& path) const {
        return runtime_text(
            "Runtime.Package.Error.ManifestPairPathRejected",
            {{"path", copperfin::platform::path_to_utf8_string(path)}});
    }

    std::filesystem::path display_path(const std::filesystem::path& leaf) const {
        return directory_.full_path(leaf);
    }

    bool read_state(const std::filesystem::path& leaf, DirectFileState& state) const {
        state = {.kind = directory_.entry_kind(leaf), .hash = {}};
        if (state.kind == ManifestPairEntryKind::missing) {
            return true;
        }
        if (state.kind != ManifestPairEntryKind::regular) {
            return false;
        }
        std::string bytes;
        if (!directory_.read_direct_file(leaf, bytes)) {
            return false;
        }
        const auto digest = text_hash(bytes);
        if (!digest.has_value()) {
            return false;
        }
        state.hash = *digest;
        return true;
    }

    bool require_current_pair(
        std::array<DirectFileState, 2U>& states,
        std::string& error) const {
        for (std::size_t index = 0U; index < states.size(); ++index) {
            if (!read_state(destination_leaves_[index], states[index]) ||
                states[index].kind != ManifestPairEntryKind::regular) {
                error = path_rejected(display_path(destination_leaves_[index]));
                return false;
            }
        }
        return true;
    }

    bool reject_unowned_artifacts(std::string& error) const {
        for (const auto& leaf : {
                 stage_leaves_[0],
                 stage_leaves_[1],
                 backup_leaves_[0],
                 backup_leaves_[1]}) {
            if (directory_.entry_kind(leaf) != ManifestPairEntryKind::missing) {
                error = runtime_text(
                    "Runtime.Package.Error.ManifestPairTransactionCollision",
                    {{"path", copperfin::platform::path_to_utf8_string(display_path(leaf))}});
                return false;
            }
        }
        return true;
    }

    bool recover_existing_transaction(std::string& error) {
        const ManifestPairEntryKind marker_kind = directory_.entry_kind(marker_leaf_);
        if (marker_kind == ManifestPairEntryKind::missing) {
            return reject_unowned_artifacts(error);
        }
        if (marker_kind != ManifestPairEntryKind::regular) {
            error = transaction_collision(marker_leaf_);
            return false;
        }

        std::string marker_bytes;
        if (!directory_.read_direct_file(marker_leaf_, marker_bytes)) {
            error = transaction_collision(marker_leaf_);
            return false;
        }
        const auto journal = parse_journal_text(marker_bytes, identity_);
        if (!journal.has_value()) {
            error = transaction_collision(marker_leaf_);
            return false;
        }

        std::array<DirectFileState, 2U> destination_states;
        std::array<DirectFileState, 2U> stage_states;
        std::array<DirectFileState, 2U> backup_states;
        for (std::size_t index = 0U; index < destination_leaves_.size(); ++index) {
            if (!read_state(destination_leaves_[index], destination_states[index]) ||
                !read_state(stage_leaves_[index], stage_states[index]) ||
                !read_state(backup_leaves_[index], backup_states[index]) ||
                (stage_states[index].kind == ManifestPairEntryKind::regular &&
                 stage_states[index].hash != journal->new_hashes[index]) ||
                (backup_states[index].kind == ManifestPairEntryKind::regular &&
                 backup_states[index].hash != journal->old_hashes[index])) {
                error = rollback_failed(marker_leaf_);
                return false;
            }
        }

        const bool promotions_complete =
            stage_states[0].kind == ManifestPairEntryKind::missing &&
            stage_states[1].kind == ManifestPairEntryKind::missing &&
            destination_states[0].kind == ManifestPairEntryKind::regular &&
            destination_states[1].kind == ManifestPairEntryKind::regular &&
            destination_states[0].hash == journal->new_hashes[0] &&
            destination_states[1].hash == journal->new_hashes[1];
        if (promotions_complete) {
            if (!directory_.remove_direct_file(backup_leaves_[0]) ||
                !directory_.remove_direct_file(backup_leaves_[1]) ||
                !directory_.remove_direct_file(marker_leaf_)) {
                error = rollback_failed(marker_leaf_);
                return false;
            }
            return true;
        }

        for (std::size_t index = 0U; index < destination_leaves_.size(); ++index) {
            if (backup_states[index].kind == ManifestPairEntryKind::regular) {
                const bool destination_recoverable =
                    destination_states[index].kind == ManifestPairEntryKind::missing ||
                    (destination_states[index].kind == ManifestPairEntryKind::regular &&
                     destination_states[index].hash == journal->new_hashes[index]);
                if (!destination_recoverable) {
                    error = rollback_failed(destination_leaves_[index]);
                    return false;
                }
            } else if (backup_states[index].kind != ManifestPairEntryKind::missing ||
                       destination_states[index].kind != ManifestPairEntryKind::regular ||
                       destination_states[index].hash != journal->old_hashes[index]) {
                error = rollback_failed(destination_leaves_[index]);
                return false;
            }
            if (stage_states[index].kind != ManifestPairEntryKind::missing &&
                stage_states[index].kind != ManifestPairEntryKind::regular) {
                error = rollback_failed(stage_leaves_[index]);
                return false;
            }
        }

        for (std::size_t index = 0U; index < destination_leaves_.size(); ++index) {
            if (backup_states[index].kind == ManifestPairEntryKind::regular) {
                if (destination_states[index].kind == ManifestPairEntryKind::regular &&
                    !directory_.remove_direct_file(destination_leaves_[index])) {
                    error = rollback_failed(destination_leaves_[index]);
                    return false;
                }
                if (!directory_.move_direct_file_no_replace(
                        backup_leaves_[index],
                        destination_leaves_[index])) {
                    error = rollback_failed(destination_leaves_[index]);
                    return false;
                }
            }
            if (!directory_.remove_direct_file(stage_leaves_[index])) {
                error = rollback_failed(stage_leaves_[index]);
                return false;
            }
        }

        std::array<DirectFileState, 2U> restored_states;
        for (std::size_t index = 0U; index < destination_leaves_.size(); ++index) {
            if (!read_state(destination_leaves_[index], restored_states[index]) ||
                restored_states[index].kind != ManifestPairEntryKind::regular ||
                restored_states[index].hash != journal->old_hashes[index]) {
                error = rollback_failed(destination_leaves_[index]);
                return false;
            }
        }
        if (!directory_.remove_direct_file(marker_leaf_)) {
            error = rollback_failed(marker_leaf_);
            return false;
        }
        return true;
    }

    bool stage(
        const std::string& runtime_contents,
        const std::string& debug_contents,
        std::string& error) {
        if (!validate_paths(error) || !recover_existing_transaction(error)) {
            return false;
        }

        std::array<DirectFileState, 2U> old_states;
        if (!require_current_pair(old_states, error)) {
            return false;
        }
        const auto runtime_hash = text_hash(runtime_contents);
        const auto debug_hash = text_hash(debug_contents);
        if (!runtime_hash.has_value() || !debug_hash.has_value()) {
            error = runtime_text(
                "Runtime.Package.Error.ManifestPairStageFailed",
                {{"path", copperfin::platform::path_to_utf8_string(root_)}});
            return false;
        }
        journal_ = ManifestPairJournal{
            .identity = identity_,
            .old_hashes = {old_states[0].hash, old_states[1].hash},
            .new_hashes = {*runtime_hash, *debug_hash}
        };

        if (!directory_.create_direct_file_and_flush(
                marker_leaf_,
                build_journal_text(*journal_))) {
            error = runtime_text(
                "Runtime.Package.Error.ManifestPairStageFailed",
                {{"path", copperfin::platform::path_to_utf8_string(display_path(marker_leaf_))}});
            return false;
        }
        if (!directory_.create_direct_file_and_flush(stage_leaves_[0], runtime_contents)) {
            return fail_and_recover(
                "Runtime.Package.Error.ManifestPairStageFailed",
                stage_leaves_[0],
                error);
        }
        if (!directory_.create_direct_file_and_flush(stage_leaves_[1], debug_contents)) {
            return fail_and_recover(
                "Runtime.Package.Error.ManifestPairStageFailed",
                stage_leaves_[1],
                error);
        }

        for (std::size_t index = 0U; index < destination_leaves_.size(); ++index) {
            DirectFileState current;
            DirectFileState staged;
            if (!read_state(destination_leaves_[index], current) ||
                !read_state(stage_leaves_[index], staged) ||
                current.hash != journal_->old_hashes[index] ||
                staged.hash != journal_->new_hashes[index]) {
                return fail_and_recover(
                    "Runtime.Package.Error.ManifestPairStageFailed",
                    destination_leaves_[index],
                    error);
            }
        }
        return true;
    }

    bool fail_and_recover(
        const std::string_view error_key,
        const std::filesystem::path& leaf,
        std::string& error) {
        const std::string operation_error = runtime_text(
            error_key,
            {{"path", copperfin::platform::path_to_utf8_string(display_path(leaf))}});
        std::string recovery_error;
        if (!recover_existing_transaction(recovery_error)) {
            error = recovery_error + "\n" + operation_error;
            return false;
        }
        error = operation_error;
        return false;
    }

    std::string transaction_collision(const std::filesystem::path& leaf) const {
        return runtime_text(
            "Runtime.Package.Error.ManifestPairTransactionCollision",
            {{"path", copperfin::platform::path_to_utf8_string(display_path(leaf))}});
    }

    std::string rollback_failed(const std::filesystem::path& leaf) const {
        return runtime_text(
            "Runtime.Package.Error.ManifestPairRollbackFailed",
            {{"path", copperfin::platform::path_to_utf8_string(display_path(leaf))}});
    }

    static bool promotion_fault_triggered(const bool first) {
#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
        const auto requested = first
            ? test_hooks::ManifestPairPromotionFault::before_first_promotion
            : test_hooks::ManifestPairPromotionFault::before_second_promotion;
        auto expected = requested;
        return manifest_pair_promotion_fault.compare_exchange_strong(
            expected,
            test_hooks::ManifestPairPromotionFault::none,
            std::memory_order_relaxed);
#else
        (void)first;
        return false;
#endif
    }

    std::filesystem::path configured_root_;
    std::array<std::filesystem::path, 2U> configured_destinations_;
    std::filesystem::path root_;
    std::array<std::filesystem::path, 2U> destination_leaves_;
    std::array<std::filesystem::path, 2U> stage_leaves_;
    std::array<std::filesystem::path, 2U> backup_leaves_;
    std::filesystem::path marker_leaf_;
    std::string identity_;
    std::optional<ManifestPairJournal> journal_;
    ManifestPairDirectory directory_;
};

}  // namespace

namespace runtime_pipeline_detail {

bool write_runtime_manifest_pair_atomically(
    const RuntimePackagePlan& plan,
    const std::string& runtime_contents,
    const std::string& debug_contents,
    std::string& error) {
    ManifestPairTransaction transaction(plan);
    return transaction.publish(runtime_contents, debug_contents, error);
}

}  // namespace runtime_pipeline_detail

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
namespace test_hooks {

void set_manifest_pair_promotion_fault(const ManifestPairPromotionFault fault) {
    manifest_pair_promotion_fault.store(fault, std::memory_order_relaxed);
}

bool seed_stale_manifest_pair_transaction(
    const RuntimePackagePlan& plan,
    const std::string& staged_runtime_manifest,
    const std::string& staged_debug_manifest,
    std::string& error) {
    ManifestPairTransaction transaction(plan);
    return transaction.seed_stale(
        staged_runtime_manifest,
        staged_debug_manifest,
        error);
}

}  // namespace test_hooks
#endif

}  // namespace copperfin::runtime

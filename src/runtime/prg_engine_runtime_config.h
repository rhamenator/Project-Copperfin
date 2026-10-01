// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <filesystem>
#include <optional>
#include <string>

namespace copperfin::runtime {

inline constexpr std::size_t k_default_max_call_depth = 1024U;
inline constexpr std::size_t k_default_max_executed_statements = 500000U;
inline constexpr std::size_t k_default_max_loop_iterations = 200000U;
inline constexpr std::size_t k_default_max_source_bytes = 64U * 1024U * 1024U;
inline constexpr std::size_t k_default_max_aggregate_source_bytes = 256U * 1024U * 1024U;
inline constexpr std::size_t k_default_max_logical_line_bytes = 1024U * 1024U;
inline constexpr std::size_t k_default_max_source_lines = 5000000U;
inline constexpr std::size_t k_default_max_include_files = 4096U;
inline constexpr std::size_t k_default_yield_statement_interval = 4096U;
inline constexpr std::size_t k_default_yield_sleep_ms = 1U;

struct RuntimeConfigFile {
    std::optional<std::size_t> max_call_depth;
    std::optional<std::size_t> max_executed_statements;
    std::optional<std::size_t> max_loop_iterations;
    std::optional<std::size_t> max_source_bytes;
    std::optional<std::size_t> max_aggregate_source_bytes;
    std::optional<std::size_t> max_logical_line_bytes;
    std::optional<std::size_t> max_source_lines;
    std::optional<std::size_t> max_include_files;
    std::optional<std::string> temp_directory;
    std::optional<std::size_t> scheduler_yield_statement_interval;
    std::optional<std::size_t> scheduler_yield_sleep_ms;
    std::optional<int> code_page;
    // True only when CODEPAGE was explicitly present but malformed or unsupported.
    bool invalid_code_page = false;
    std::string source_path;
};

std::optional<RuntimeConfigFile> load_runtime_config_near(
    const std::filesystem::path& startup_path,
    const std::filesystem::path& working_directory);

void apply_runtime_config_defaults(RuntimeSessionOptions& options, const RuntimeConfigFile& config);

std::filesystem::path choose_runtime_temp_directory(const RuntimeSessionOptions& options);

}  // namespace copperfin::runtime

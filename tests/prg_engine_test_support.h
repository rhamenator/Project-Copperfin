#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace copperfin::test_support {

int test_failures();
void expect(bool condition, const std::string& message);

std::string uppercase_ascii(std::string value);
std::string lowercase_copy(std::string value);
void write_text(const std::filesystem::path& path, const std::string& contents);
std::string read_text(const std::filesystem::path& path);
void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value);
void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value);
void write_simple_dbf(const std::filesystem::path& path, const std::vector<std::string>& records);
void write_people_dbf(
    const std::filesystem::path& path,
    const std::vector<std::pair<std::string, int>>& records);
void write_synthetic_cdx(
    const std::filesystem::path& path,
    const std::string& tag_name,
    const std::string& expression);
void write_synthetic_idx(const std::filesystem::path& path, const std::string& expression);
void write_synthetic_idx_with_for(
    const std::filesystem::path& path,
    const std::string& expression,
    const std::string& for_expression);
void mark_simple_dbf_record_deleted(const std::filesystem::path& path, std::size_t recno);
void write_synthetic_ndx(
    const std::filesystem::path& path,
    const std::string& expression,
    bool numeric_or_date_domain);
bool has_runtime_event(
    const std::vector<copperfin::runtime::RuntimeEvent>& events,
    const std::string& category,
    const std::string& detail);

}  // namespace copperfin::test_support

#include "prg_engine_test_support.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

namespace copperfin::test_support {

namespace {

int failures = 0;

}  // namespace

int test_failures() {
    return failures;
}

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::string uppercase_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

void write_text(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream output(path, std::ios::binary);
    output << contents;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void write_simple_dbf(const std::filesystem::path& path, const std::vector<std::string>& records) {
    constexpr std::size_t field_length = 10U;
    constexpr std::size_t header_length = 32U + 32U + 1U;
    constexpr std::size_t record_length = 1U + field_length;
    std::vector<std::uint8_t> bytes(header_length + (records.size() * record_length) + 1U, 0U);
    bytes[0] = 0x30U;
    write_le_u32(bytes, 4U, static_cast<std::uint32_t>(records.size()));
    write_le_u16(bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(bytes, 10U, static_cast<std::uint16_t>(record_length));

    const char name[] = "NAME";
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[32U + index] = static_cast<std::uint8_t>(name[index]);
    }
    bytes[32U + 11U] = static_cast<std::uint8_t>('C');
    write_le_u32(bytes, 32U + 12U, 1U);
    bytes[32U + 16U] = static_cast<std::uint8_t>(field_length);
    bytes[64U] = 0x0DU;

    for (std::size_t record_index = 0; record_index < records.size(); ++record_index) {
        const std::size_t offset = header_length + (record_index * record_length);
        bytes[offset] = 0x20U;
        const std::string value = records[record_index].substr(0U, field_length);
        for (std::size_t index = 0; index < value.size(); ++index) {
            bytes[offset + 1U + index] = static_cast<std::uint8_t>(value[index]);
        }
        for (std::size_t index = value.size(); index < field_length; ++index) {
            bytes[offset + 1U + index] = 0x20U;
        }
    }

    bytes.back() = 0x1AU;
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_people_dbf(
    const std::filesystem::path& path,
    const std::vector<std::pair<std::string, int>>& records) {
    constexpr std::size_t name_length = 10U;
    constexpr std::size_t age_length = 3U;
    constexpr std::size_t header_length = 32U + 32U + 32U + 1U;
    constexpr std::size_t record_length = 1U + name_length + age_length;
    std::vector<std::uint8_t> bytes(header_length + (records.size() * record_length) + 1U, 0U);
    bytes[0] = 0x30U;
    write_le_u32(bytes, 4U, static_cast<std::uint32_t>(records.size()));
    write_le_u16(bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(bytes, 10U, static_cast<std::uint16_t>(record_length));

    const char name_field[] = "NAME";
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[32U + index] = static_cast<std::uint8_t>(name_field[index]);
    }
    bytes[32U + 11U] = static_cast<std::uint8_t>('C');
    write_le_u32(bytes, 32U + 12U, 1U);
    bytes[32U + 16U] = static_cast<std::uint8_t>(name_length);

    const char age_field[] = "AGE";
    for (std::size_t index = 0; index < 3U; ++index) {
        bytes[64U + index] = static_cast<std::uint8_t>(age_field[index]);
    }
    bytes[64U + 11U] = static_cast<std::uint8_t>('N');
    write_le_u32(bytes, 64U + 12U, 1U + static_cast<std::uint32_t>(name_length));
    bytes[64U + 16U] = static_cast<std::uint8_t>(age_length);
    bytes[96U] = 0x0DU;

    for (std::size_t record_index = 0; record_index < records.size(); ++record_index) {
        const std::size_t offset = header_length + (record_index * record_length);
        bytes[offset] = 0x20U;

        const std::string name = records[record_index].first.substr(0U, name_length);
        for (std::size_t index = 0; index < name.size(); ++index) {
            bytes[offset + 1U + index] = static_cast<std::uint8_t>(name[index]);
        }
        for (std::size_t index = name.size(); index < name_length; ++index) {
            bytes[offset + 1U + index] = 0x20U;
        }

        const std::string age = std::to_string(records[record_index].second);
        const std::size_t padding = age.size() >= age_length ? 0U : age_length - age.size();
        for (std::size_t index = 0; index < padding; ++index) {
            bytes[offset + 1U + name_length + index] = 0x20U;
        }
        for (std::size_t index = 0; index < age.size() && index < age_length; ++index) {
            bytes[offset + 1U + name_length + padding + index] = static_cast<std::uint8_t>(age[index]);
        }
    }

    bytes.back() = 0x1AU;
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_synthetic_cdx(const std::filesystem::path& path, const std::string& tag_name, const std::string& expression) {
    std::vector<std::uint8_t> bytes(4096U, 0U);
    write_le_u16(bytes, 0U, 1024U);
    write_le_u16(bytes, 12U, 10U);
    write_le_u16(bytes, 14U, 480U);
    write_le_u16(bytes, 1024U, 0x0003U);
    write_le_u16(bytes, 1026U, 0x0001U);
    write_le_u32(bytes, 1028U, 2048U);

    for (std::size_t index = 0; index < expression.size(); ++index) {
        bytes[2048U + index] = static_cast<std::uint8_t>(expression[index]);
    }

    const std::size_t tail_offset = (3U * 512U) - 10U;
    for (std::size_t index = 0; index < tag_name.size(); ++index) {
        bytes[tail_offset + index] = static_cast<std::uint8_t>(tag_name[index]);
    }

    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_synthetic_idx(const std::filesystem::path& path, const std::string& expression) {
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_le_u32(bytes, 0U, 512U);
    write_le_u32(bytes, 4U, 0U);
    write_le_u32(bytes, 8U, 1024U);
    write_le_u16(bytes, 12U, static_cast<std::uint16_t>(std::min<std::size_t>(expression.size(), 220U)));
    for (std::size_t index = 0; index < expression.size() && index < 220U; ++index) {
        bytes[16U + index] = static_cast<std::uint8_t>(expression[index]);
    }

    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_synthetic_idx_with_for(
    const std::filesystem::path& path,
    const std::string& expression,
    const std::string& for_expression) {
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_le_u32(bytes, 0U, 512U);
    write_le_u32(bytes, 4U, 0U);
    write_le_u32(bytes, 8U, 1024U);
    write_le_u16(bytes, 12U, static_cast<std::uint16_t>(std::min<std::size_t>(expression.size(), 220U)));
    for (std::size_t index = 0; index < expression.size() && index < 220U; ++index) {
        bytes[16U + index] = static_cast<std::uint8_t>(expression[index]);
    }
    for (std::size_t index = 0; index < for_expression.size() && index < 220U; ++index) {
        bytes[236U + index] = static_cast<std::uint8_t>(for_expression[index]);
    }

    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void mark_simple_dbf_record_deleted(const std::filesystem::path& path, std::size_t recno) {
    constexpr std::size_t header_length = 32U + 32U + 1U;
    constexpr std::size_t record_length = 1U + 10U;
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    const std::size_t deletion_offset = header_length + ((recno - 1U) * record_length);
    file.seekp(static_cast<std::streamoff>(deletion_offset), std::ios::beg);
    const char deleted = '*';
    file.write(&deleted, 1);
}

void write_synthetic_ndx(
    const std::filesystem::path& path,
    const std::string& expression,
    bool numeric_or_date_domain) {
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_le_u32(bytes, 0U, 1U);
    write_le_u32(bytes, 4U, 2U);
    write_le_u32(bytes, 8U, 0x00000034U);
    write_le_u16(bytes, 12U, static_cast<std::uint16_t>(std::min<std::size_t>(expression.size(), 100U)));
    write_le_u16(bytes, 14U, 42U);
    write_le_u16(bytes, 16U, numeric_or_date_domain ? 1U : 0U);
    write_le_u16(bytes, 18U, 12U);
    write_le_u16(bytes, 22U, 0U);
    for (std::size_t index = 0; index < expression.size() && index < 100U; ++index) {
        bytes[24U + index] = static_cast<std::uint8_t>(expression[index]);
    }

    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

bool has_runtime_event(
    const std::vector<copperfin::runtime::RuntimeEvent>& events,
    const std::string& category,
    const std::string& detail) {
    return std::any_of(events.begin(), events.end(), [&](const copperfin::runtime::RuntimeEvent& event) {
        return event.category == category && event.detail == detail;
    });
}

}  // namespace copperfin::test_support

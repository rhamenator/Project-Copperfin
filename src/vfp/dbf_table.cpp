#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace copperfin::vfp {

namespace {

std::uint16_t read_le_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1]) << 8U);
}

std::uint32_t read_le_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8U) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24U);
}

std::uint32_t read_be_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 8U) |
           static_cast<std::uint32_t>(bytes[offset + 3]);
}

std::uint16_t read_be_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[offset]) << 8U) |
                                      static_cast<std::uint16_t>(bytes[offset + 1]));
}

std::string trim_right(std::string text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.pop_back();
    }
    return text;
}

std::string trim_both(std::string text) {
    text = trim_right(std::move(text));
    const auto first = std::find_if(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
    text.erase(text.begin(), first);
    return text;
}

std::string read_ascii_name(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::size_t length) {
    std::string value;
    value.reserve(length);
    for (std::size_t index = 0; index < length && (offset + index) < bytes.size(); ++index) {
        const auto raw = bytes[offset + index];
        if (raw == 0U) {
            break;
        }
        value.push_back(static_cast<char>(raw));
    }
    return trim_right(std::move(value));
}

std::string load_file_string(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string infer_memo_sidecar_path(const std::string& path) {
    std::filesystem::path file_path(path);
    const std::string ext = file_path.extension().string();

    if (ext == ".scx") {
        return file_path.replace_extension(".sct").string();
    }
    if (ext == ".vcx") {
        return file_path.replace_extension(".vct").string();
    }
    if (ext == ".frx") {
        return file_path.replace_extension(".frt").string();
    }
    if (ext == ".lbx") {
        return file_path.replace_extension(".lbt").string();
    }
    if (ext == ".mnx") {
        return file_path.replace_extension(".mnt").string();
    }
    if (ext == ".pjx") {
        return file_path.replace_extension(".pjt").string();
    }
    if (ext == ".dbc") {
        return file_path.replace_extension(".dct").string();
    }
    if (ext == ".dbf") {
        return file_path.replace_extension(".fpt").string();
    }
    return {};
}

class MemoReader {
public:
    MemoReader() = default;

    explicit MemoReader(const std::string& path) {
        if (path.empty()) {
            return;
        }

        std::ifstream input(path, std::ios::binary);
        if (!input) {
            return;
        }

        bytes_ = {
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()
        };

        if (bytes_.size() < 512U) {
            bytes_.clear();
            return;
        }

        block_size_ = read_be_u16(bytes_, 6U);
        if (block_size_ == 0U) {
            block_size_ = 1U;
        }
        available_ = true;
    }

    [[nodiscard]] bool available() const {
        return available_;
    }

    [[nodiscard]] std::string read_block(std::uint32_t block_number) const {
        if (!available_ || block_number == 0U) {
            return {};
        }

        const std::uint64_t offset = static_cast<std::uint64_t>(block_number) * block_size_;
        if ((offset + 8U) > bytes_.size()) {
            return {};
        }

        const std::uint32_t length = read_be_u32(bytes_, static_cast<std::size_t>(offset + 4U));
        const std::uint64_t payload_offset = offset + 8U;
        const std::uint64_t payload_end = payload_offset + length;
        if (payload_end > bytes_.size()) {
            return {};
        }

        std::string text;
        text.reserve(length);
        for (std::uint64_t index = payload_offset; index < payload_end; ++index) {
            const auto raw = static_cast<unsigned char>(bytes_[static_cast<std::size_t>(index)]);
            if (raw == 0U) {
                text.push_back(' ');
            } else if (std::isprint(raw) != 0 || std::isspace(raw) != 0) {
                text.push_back(static_cast<char>(raw));
            }
        }

        text = trim_right(std::move(text));
        if (text.size() > 160U) {
            text.resize(160U);
            text += "...";
        }
        return text;
    }

private:
    std::vector<std::uint8_t> bytes_;
    std::uint32_t block_size_ = 0;
    bool available_ = false;
};

std::string format_binary_bytes(const std::vector<std::uint8_t>& bytes) {
    std::ostringstream stream;
    stream << "0x";
    constexpr std::array<char, 16U> hex = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    for (std::uint8_t byte : bytes) {
        stream << hex[(byte >> 4U) & 0x0FU] << hex[byte & 0x0FU];
    }
    return stream.str();
}

std::string decode_value(
    char field_type,
    const std::vector<std::uint8_t>& raw,
    const MemoReader& memo_reader,
    bool& is_null) {
    is_null = false;
    if (raw.empty()) {
        return {};
    }

    switch (field_type) {
        case 'C': {
            return trim_right(std::string(raw.begin(), raw.end()));
        }
        case 'N':
        case 'F': {
            return trim_both(std::string(raw.begin(), raw.end()));
        }
        case 'L': {
            const char value = static_cast<char>(raw[0]);
            if (value == 'Y' || value == 'y' || value == 'T' || value == 't') {
                return "true";
            }
            if (value == 'N' || value == 'n' || value == 'F' || value == 'f') {
                return "false";
            }
            return std::string(1U, value);
        }
        case 'D': {
            const std::string value(raw.begin(), raw.end());
            if (value.size() == 8U) {
                return value.substr(0U, 4U) + "-" + value.substr(4U, 2U) + "-" + value.substr(6U, 2U);
            }
            return trim_both(value);
        }
        case 'I': {
            if (raw.size() < 4U) {
                return {};
            }
            const std::int32_t value = static_cast<std::int32_t>(read_le_u32(raw, 0U));
            return std::to_string(value);
        }
        case 'T': {
            if (raw.size() < 8U) {
                return {};
            }
            const std::uint32_t julian_day = read_le_u32(raw, 0U);
            const std::uint32_t millis = read_le_u32(raw, 4U);
            std::ostringstream stream;
            stream << "julian:" << julian_day << " millis:" << millis;
            return stream.str();
        }
        case 'M':
        case 'G':
        case 'P': {
            if (raw.size() < 4U) {
                return {};
            }
            const std::uint32_t block_number = read_le_u32(raw, 0U);
            const std::string memo_text = memo_reader.read_block(block_number);
            if (!memo_text.empty()) {
                return memo_text;
            }
            std::ostringstream stream;
            stream << "<memo block " << block_number << ">";
            return stream.str();
        }
        case 'Y': {
            if (raw.size() < 8U) {
                return {};
            }
            const std::int64_t low = static_cast<std::uint64_t>(read_le_u32(raw, 0U));
            const std::int64_t high = static_cast<std::uint64_t>(read_le_u32(raw, 4U));
            const std::int64_t scaled = (high << 32U) | low;
            std::ostringstream stream;
            stream << (scaled / 10000) << "." << std::abs(static_cast<int>(scaled % 10000));
            return stream.str();
        }
        case '0': {
            is_null = true;
            return "NULL";
        }
        default:
            return format_binary_bytes(raw);
    }
}

}  // namespace

DbfTableParseResult parse_dbf_table_from_file(const std::string& path, std::size_t max_records) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {.ok = false, .error = "Unable to open table file."};
    }

    std::vector<std::uint8_t> bytes = {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    DbfTable table;
    table.header = header_result.header;

    if (bytes.size() < table.header.header_length) {
        return {.ok = false, .error = "Table file is shorter than its header length."};
    }

    std::size_t field_offset = 32U;
    while ((field_offset + 32U) <= bytes.size() && bytes[field_offset] != 0x0DU) {
        DbfFieldDescriptor field;
        field.name = read_ascii_name(bytes, field_offset, 11U);
        field.type = static_cast<char>(bytes[field_offset + 11U]);
        field.offset = read_le_u32(bytes, field_offset + 12U);
        field.length = bytes[field_offset + 16U];
        field.decimal_count = bytes[field_offset + 17U];
        table.fields.push_back(std::move(field));
        field_offset += 32U;
    }

    const MemoReader memo_reader(infer_memo_sidecar_path(path));
    const std::size_t record_count = std::min<std::size_t>(table.header.record_count, max_records);
    const std::size_t data_offset = table.header.header_length;
    const std::size_t record_length = table.header.record_length;

    for (std::size_t record_index = 0; record_index < record_count; ++record_index) {
        const std::size_t record_offset = data_offset + (record_index * record_length);
        if ((record_offset + record_length) > bytes.size()) {
            break;
        }

        DbfRecord record;
        record.record_index = record_index;
        record.deleted = bytes[record_offset] == 0x2AU;

        for (const auto& field : table.fields) {
            const std::size_t field_start = record_offset + field.offset;
            const std::size_t field_end = field_start + field.length;
            if (field_end > bytes.size()) {
                break;
            }

            std::vector<std::uint8_t> raw(
                bytes.begin() + static_cast<std::ptrdiff_t>(field_start),
                bytes.begin() + static_cast<std::ptrdiff_t>(field_end));

            bool is_null = false;
            const std::string display_value = decode_value(field.type, raw, memo_reader, is_null);
            record.values.push_back({
                .field_name = field.name,
                .field_type = field.type,
                .is_null = is_null,
                .display_value = display_value
            });
        }

        table.records.push_back(std::move(record));
    }

    return {.ok = true, .table = std::move(table)};
}

}  // namespace copperfin::vfp

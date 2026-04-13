#include "copperfin/vfp/index_probe.h"

#include "copperfin/vfp/cdx_header.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

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

std::string read_ascii_hint(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::size_t length) {
    std::string value;
    value.reserve(length);

    for (std::size_t index = 0; index < length && (offset + index) < bytes.size(); ++index) {
        const auto raw = static_cast<unsigned char>(bytes[offset + index]);
        if (raw == 0U) {
            break;
        }
        if (std::isprint(raw) != 0 || std::isspace(raw) != 0) {
            value.push_back(static_cast<char>(raw));
        }
    }

    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }

    return value;
}

std::string trim_copy(std::string value) {
    const auto first = std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
    value.erase(value.begin(), first);

    const auto last = std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
    value.erase(last.base(), value.end());
    return value;
}

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

void append_hint_if_missing(std::vector<std::string>& hints, const std::string& hint) {
    if (hint.empty()) {
        return;
    }

    if (std::find(hints.begin(), hints.end(), hint) == hints.end()) {
        hints.push_back(hint);
    }
}

std::string join_hints(const std::vector<std::string>& hints) {
    std::string joined;
    for (const std::string& hint : hints) {
        if (!joined.empty()) {
            joined += "+";
        }
        joined += hint;
    }
    return joined;
}

std::string format_hex_byte(std::uint8_t value) {
    static const char digits[] = "0123456789ABCDEF";
    std::string text = "0x00";
    text[2] = digits[(value >> 4U) & 0x0FU];
    text[3] = digits[value & 0x0FU];
    return text;
}

std::string format_hex_u16(std::uint16_t value) {
    static const char digits[] = "0123456789ABCDEF";
    std::string text = "0x0000";
    text[2] = digits[(value >> 12U) & 0x0FU];
    text[3] = digits[(value >> 8U) & 0x0FU];
    text[4] = digits[(value >> 4U) & 0x0FU];
    text[5] = digits[value & 0x0FU];
    return text;
}

std::string make_idx_header_sort_marker(std::uint8_t signature, std::uint8_t flags) {
    return "sig:" + format_hex_byte(signature) + ",flags:" + format_hex_byte(flags);
}

std::string make_ndx_header_sort_marker(std::uint8_t signature) {
    return "ver:" + format_hex_byte(signature);
}

std::string make_tag_sort_marker(std::uint16_t flags, std::uint16_t entry_count) {
    return "flags:" + format_hex_u16(flags) + ",entries:" + std::to_string(entry_count);
}

struct PrintableRun {
    std::size_t offset = 0;
    std::string text;
};

bool is_identifier_char(char ch) {
    const auto raw = static_cast<unsigned char>(ch);
    return std::isalnum(raw) != 0 || ch == '_';
}

bool is_expression_char(char ch) {
    const auto raw = static_cast<unsigned char>(ch);
    return std::isalnum(raw) != 0 || ch == '_' || ch == '(' || ch == ')' ||
           ch == '.' || ch == ',' || ch == '\'' || ch == '"' || ch == ' ' ||
           ch == '=' || ch == '<' || ch == '>' || ch == '+' || ch == '-' ||
           ch == '*' || ch == '/' || ch == '!';
}

std::vector<PrintableRun> collect_printable_runs(
    const std::vector<std::uint8_t>& bytes,
    std::size_t start,
    std::size_t end) {
    std::vector<PrintableRun> runs;
    const std::size_t bounded_end = std::min(end, bytes.size());

    for (std::size_t index = std::min(start, bounded_end); index < bounded_end;) {
        const auto raw = static_cast<unsigned char>(bytes[index]);
        if (std::isprint(raw) == 0) {
            ++index;
            continue;
        }

        const std::size_t run_start = index;
        std::string text;
        while (index < bounded_end) {
            const auto current = static_cast<unsigned char>(bytes[index]);
            if (std::isprint(current) == 0) {
                break;
            }
            text.push_back(static_cast<char>(current));
            ++index;
        }

        if (text.size() >= 4U) {
            runs.push_back({.offset = run_start, .text = std::move(text)});
        }
    }

    return runs;
}

bool looks_like_expression_candidate(const std::string& text) {
    if (text.size() < 4U || text.size() > 96U) {
        return false;
    }

    if (!std::all_of(text.begin(), text.end(), [](char ch) { return is_expression_char(ch); })) {
        return false;
    }

    const bool has_identifier = std::any_of(text.begin(), text.end(), [](char ch) {
        return is_identifier_char(ch);
    });
    const bool has_parens = text.find('(') != std::string::npos && text.find(')') != std::string::npos;
    const bool has_operator = text.find('=') != std::string::npos ||
                              text.find('<') != std::string::npos ||
                              text.find('>') != std::string::npos;
    return has_identifier && (has_parens || has_operator || text.find('+') != std::string::npos);
}

bool looks_like_for_expression_candidate(const std::string& text) {
    if (text.empty()) {
        return false;
    }

    const std::string upper = uppercase_copy(text);
    return text.find('=') != std::string::npos ||
           text.find('<') != std::string::npos ||
           text.find('>') != std::string::npos ||
           upper.find(".T.") != std::string::npos ||
           upper.find(".F.") != std::string::npos ||
           upper.starts_with("DELETED(") ||
           upper.find(" AND ") != std::string::npos ||
           upper.find(" OR ") != std::string::npos ||
           upper.starts_with("NOT ");
}

std::vector<PrintableRun> collect_expression_runs(
    const std::vector<std::uint8_t>& bytes,
    std::size_t start,
    std::size_t end) {
    std::vector<PrintableRun> expressions;
    for (const PrintableRun& run : collect_printable_runs(bytes, start, end)) {
        const std::string candidate = trim_copy(run.text);
        if (!looks_like_expression_candidate(candidate)) {
            continue;
        }
        expressions.push_back({.offset = run.offset, .text = candidate});
    }
    return expressions;
}

std::string derive_normalization_hint(const std::string& expression) {
    const std::string upper = uppercase_copy(trim_copy(expression));
    std::vector<std::string> hints;
    if (upper.find("UPPER(") != std::string::npos) {
        append_hint_if_missing(hints, "upper");
    }
    if (upper.find("LOWER(") != std::string::npos) {
        append_hint_if_missing(hints, "lower");
    }
    if (upper.find("ALLTRIM(") != std::string::npos) {
        append_hint_if_missing(hints, "alltrim");
    }
    if (upper.find("LTRIM(") != std::string::npos) {
        append_hint_if_missing(hints, "ltrim");
    }
    if (upper.find("RTRIM(") != std::string::npos) {
        append_hint_if_missing(hints, "rtrim");
    }
    if (upper.find("PADL(") != std::string::npos) {
        append_hint_if_missing(hints, "padl");
    }
    if (upper.find("PADR(") != std::string::npos) {
        append_hint_if_missing(hints, "padr");
    }
    return join_hints(hints);
}

std::string derive_collation_hint(const std::string& expression, const std::string& normalization_hint) {
    const std::string upper = uppercase_copy(trim_copy(expression));
    if (upper.find("UPPER(") != std::string::npos || upper.find("LOWER(") != std::string::npos) {
        return "case-folded";
    }
    if (upper.find("CHRTRAN(") != std::string::npos ||
        upper.find("STRTRAN(") != std::string::npos ||
        !normalization_hint.empty()) {
        return "expression-normalized";
    }
    return {};
}

bool is_valid_optional_offset(std::uint32_t offset, std::uint32_t block_size, std::uint64_t file_size) {
    if (offset == 0U || offset == 0xFFFFFFFFU) {
        return true;
    }
    if (block_size == 0U) {
        return offset < file_size;
    }
    return offset >= block_size && offset < file_size && (offset % block_size) == 0U;
}

IndexParseResult parse_cdx_family_probe(
    const std::vector<std::uint8_t>& bytes,
    std::uint64_t file_size,
    IndexKind kind) {
    const CdxParseResult cdx_result = parse_cdx_header(bytes, file_size);
    if (!cdx_result.ok) {
        return {.ok = false, .error = cdx_result.error};
    }

    IndexProbe probe;
    probe.kind = kind;
    probe.file_size = cdx_result.header.file_size;
    probe.block_size = cdx_result.header.page_size;
    probe.root_node_offset_hint = cdx_result.header.root_node_offset;
    probe.free_node_offset_hint = cdx_result.header.next_free_node_offset;
    probe.key_length_hint = cdx_result.header.key_length_hint;
    probe.group_length_hint = cdx_result.header.key_pool_length_hint;
    probe.tags.reserve(cdx_result.tags.size());
    for (const CdxTagDescriptor& tag : cdx_result.tags) {
        const std::string normalization_hint = derive_normalization_hint(tag.key_expression_hint);
        const std::string collation_hint = derive_collation_hint(tag.key_expression_hint, normalization_hint);
        std::string tag_sort_marker_hint;
        if (tag.tag_page_offset_hint != 0U) {
            const std::size_t tag_page_offset = static_cast<std::size_t>(tag.tag_page_offset_hint);
            if ((tag_page_offset + 4U) <= bytes.size()) {
                const std::uint16_t flags = read_le_u16(bytes, tag_page_offset);
                const std::uint16_t entry_count = read_le_u16(bytes, tag_page_offset + 2U);
                tag_sort_marker_hint = make_tag_sort_marker(flags, entry_count);
            }
        }
        probe.tags.push_back({
            .name_hint = tag.name_hint,
            .key_expression_hint = tag.key_expression_hint,
            .for_expression_hint = tag.for_expression_hint,
            .normalization_hint = normalization_hint,
            .collation_hint = collation_hint,
            .tag_sort_marker_hint = tag_sort_marker_hint,
            .tag_page_offset_hint = tag.tag_page_offset_hint,
            .name_offset_hint = tag.name_offset_hint,
            .key_expression_offset_hint = tag.key_expression_offset_hint,
            .for_expression_offset_hint = tag.for_expression_offset_hint,
            .inferred_name = tag.inferred_name
        });
    }
    if (!probe.tags.empty()) {
        probe.key_expression_hint = probe.tags.front().key_expression_hint;
        probe.normalization_hint = probe.tags.front().normalization_hint;
        probe.collation_hint = probe.tags.front().collation_hint;
        for (const IndexTagProbe& tag : probe.tags) {
            if (!tag.for_expression_hint.empty()) {
                probe.for_expression_hint = tag.for_expression_hint;
                break;
            }
        }
    }
    probe.multi_tag = true;
    probe.production_candidate = (kind == IndexKind::cdx);
    return {.ok = true, .probe = probe};
}

IndexParseResult parse_fox_idx_probe(const std::vector<std::uint8_t>& bytes, std::uint64_t file_size) {
    if (bytes.size() < 512U) {
        return {.ok = false, .error = "File is smaller than the 512-byte Visual FoxPro IDX header size."};
    }

    IndexProbe probe;
    probe.kind = IndexKind::idx;
    probe.file_size = file_size;
    probe.block_size = 512U;
    probe.root_node_offset_hint = read_le_u32(bytes, 0U);
    probe.free_node_offset_hint = read_le_u32(bytes, 4U);
    probe.end_of_file_offset_hint = read_le_u32(bytes, 8U);
    probe.key_length_hint = read_le_u16(bytes, 12U);
    probe.flags = bytes[14];
    probe.signature = bytes[15];
    probe.key_expression_hint = read_ascii_hint(bytes, 16U, 220U);
    probe.for_expression_hint = read_ascii_hint(bytes, 236U, 220U);
    probe.normalization_hint = derive_normalization_hint(probe.key_expression_hint);
    probe.collation_hint = derive_collation_hint(probe.key_expression_hint, probe.normalization_hint);
    probe.header_sort_marker_hint = make_idx_header_sort_marker(probe.signature, probe.flags);

    const bool plausible_size = file_size >= probe.block_size && (file_size % probe.block_size) == 0U;
    const bool plausible_key = probe.key_length_hint > 0U && probe.key_length_hint <= 220U;
    const bool plausible_root = probe.root_node_offset_hint >= probe.block_size &&
                                probe.root_node_offset_hint < file_size &&
                                (probe.root_node_offset_hint % probe.block_size) == 0U;
    const bool plausible_eof = probe.end_of_file_offset_hint >= probe.block_size &&
                               probe.end_of_file_offset_hint <= file_size &&
                               (probe.end_of_file_offset_hint % probe.block_size) == 0U;
    const bool plausible_free = is_valid_optional_offset(
        probe.free_node_offset_hint,
        probe.block_size,
        file_size);

    if (!plausible_size || !plausible_key || !plausible_root || !plausible_eof || !plausible_free) {
        return {
            .ok = false,
            .probe = probe,
            .error = "Header values do not look like a Visual FoxPro IDX file."
        };
    }

    return {.ok = true, .probe = probe};
}

IndexParseResult parse_dbase_ndx_probe(const std::vector<std::uint8_t>& bytes, std::uint64_t file_size) {
    if (bytes.size() < 512U) {
        return {.ok = false, .error = "File is smaller than the 512-byte dBase NDX header size."};
    }

    const std::uint32_t root_block = read_le_u32(bytes, 0U);
    const std::uint32_t eof_block = read_le_u32(bytes, 4U);
    const std::uint32_t version_hint = read_le_u32(bytes, 8U);
    const std::uint16_t key_length = read_le_u16(bytes, 12U);
    const std::uint16_t max_keys = read_le_u16(bytes, 14U);
    const std::uint16_t numeric_or_date_flag = read_le_u16(bytes, 16U);
    const std::uint16_t group_length = read_le_u16(bytes, 18U);
    const std::uint16_t unique_flag = read_le_u16(bytes, 22U);

    IndexProbe probe;
    probe.kind = IndexKind::ndx;
    probe.file_size = file_size;
    probe.block_size = 512U;
    probe.root_node_offset_hint = root_block * probe.block_size;
    probe.end_of_file_offset_hint = eof_block * probe.block_size;
    probe.key_length_hint = key_length;
    probe.max_keys_hint = max_keys;
    probe.group_length_hint = group_length;
    probe.flags = static_cast<std::uint8_t>(unique_flag != 0U ? 0x01U : 0x00U);
    probe.signature = static_cast<std::uint8_t>(version_hint & 0xFFU);
    probe.key_expression_hint = read_ascii_hint(bytes, 24U, 100U);
    probe.normalization_hint = derive_normalization_hint(probe.key_expression_hint);
    probe.collation_hint = derive_collation_hint(probe.key_expression_hint, probe.normalization_hint);
    probe.header_sort_marker_hint = make_ndx_header_sort_marker(probe.signature);
    probe.key_domain_hint = numeric_or_date_flag == 0U ? "character" : "numeric_or_date";

    const bool plausible_size = file_size >= probe.block_size && (file_size % probe.block_size) == 0U;
    const bool plausible_root = root_block > 0U && probe.root_node_offset_hint < file_size;
    const bool plausible_eof = eof_block >= 2U && probe.end_of_file_offset_hint <= file_size;
    const bool plausible_key = key_length > 0U && key_length <= 100U;
    const bool plausible_group = group_length >= static_cast<std::uint16_t>(key_length + 8U);
    const bool plausible_max_keys = max_keys > 0U;
    const bool plausible_type_flag = numeric_or_date_flag <= 1U;

    if (!plausible_size || !plausible_root || !plausible_eof || !plausible_key ||
        !plausible_group || !plausible_max_keys || !plausible_type_flag) {
        return {
            .ok = false,
            .probe = probe,
            .error = "Header values do not look like a dBase NDX file."
        };
    }

    return {.ok = true, .probe = probe};
}

IndexParseResult parse_dbase_mdx_probe(const std::vector<std::uint8_t>& bytes, std::uint64_t file_size) {
    if (bytes.size() < 512U) {
        return {.ok = false, .error = "File is smaller than the minimum MDX probe size (512 bytes)."};
    }

    IndexProbe probe;
    probe.kind = IndexKind::mdx;
    probe.file_size = file_size;
    probe.multi_tag = true;
    probe.production_candidate = true;

    // Read header fields at their documented dBase IV MDX byte offsets.
    const std::uint16_t base_block_size = read_le_u16(bytes, 20U);
    const std::uint16_t block_size_adder = read_le_u16(bytes, 22U);
    const std::uint8_t tag_slots = bytes[25U];
    const std::uint8_t tag_entry_size = bytes[26U];
    const std::uint16_t tags_in_use = read_le_u16(bytes, 28U);
    const std::uint32_t effective_block_size =
        static_cast<std::uint32_t>(base_block_size) + static_cast<std::uint32_t>(block_size_adder);

    const bool plausible_block_size = base_block_size >= 512U &&
                                      base_block_size <= 4096U &&
                                      (base_block_size % 512U) == 0U;
    const bool plausible_adder = block_size_adder <= 512U;
    const bool plausible_effective = effective_block_size >= 512U && effective_block_size <= 4096U;
    const bool plausible_tag_slots = tag_slots > 0U && tag_slots <= 48U;
    const bool plausible_tag_entry = tag_entry_size >= 16U && tag_entry_size <= 64U;
    const bool plausible_tags = tags_in_use <= tag_slots;
    const bool plausible_file = file_size >= static_cast<std::uint64_t>(effective_block_size) * 2U;

    if (!plausible_block_size || !plausible_adder || !plausible_effective ||
        !plausible_tag_slots || !plausible_tag_entry || !plausible_tags || !plausible_file) {
        return {
            .ok = false,
            .probe = probe,
            .error = "Header values do not look like a block-oriented dBase MDX file."
        };
    }

    probe.block_size = effective_block_size;

    probe.header_sort_marker_hint =
        "slots:" + std::to_string(tag_slots) +
        ",entry_size:" + std::to_string(tag_entry_size) +
        ",in_use:" + std::to_string(tags_in_use);

    // Tag table starts at byte offset effective_block_size.
    // Each entry is tag_entry_size bytes: page_num (4), name (11), key_format (1), threads (3), reserved (1), key_type (1), reserved (11).
    const std::size_t tag_table_offset = static_cast<std::size_t>(effective_block_size);
    for (std::uint16_t index = 0U; index < tags_in_use; ++index) {
        const std::size_t entry_offset =
            tag_table_offset + static_cast<std::size_t>(index) * static_cast<std::size_t>(tag_entry_size);
        if (entry_offset + static_cast<std::size_t>(tag_entry_size) > bytes.size()) {
            break;
        }

        const std::string tag_name = read_ascii_hint(bytes, entry_offset + 4U, 11U);
        if (tag_name.empty()) {
            continue;
        }

        IndexTagProbe tag_probe;
        tag_probe.name_hint = tag_name;
        tag_probe.name_offset_hint = static_cast<std::uint32_t>(entry_offset + 4U);
        tag_probe.key_format_marker = bytes[entry_offset + 15U];
        tag_probe.thread_hint = static_cast<std::uint32_t>(bytes[entry_offset + 16U]) |
                    (static_cast<std::uint32_t>(bytes[entry_offset + 17U]) << 8U) |
                    (static_cast<std::uint32_t>(bytes[entry_offset + 18U]) << 16U);
        tag_probe.key_type_marker = bytes[entry_offset + 20U];
        tag_probe.inferred_name = false;

        // Follow the tag header page to read the key expression if the page is in-bounds.
        const std::uint32_t tag_header_page = read_le_u32(bytes, entry_offset);
        if (tag_header_page > 0U) {
            const std::size_t tag_header_offset =
                static_cast<std::size_t>(tag_header_page) * static_cast<std::size_t>(effective_block_size);
            if (tag_header_offset < bytes.size()) {
                tag_probe.tag_page_offset_hint = static_cast<std::uint32_t>(tag_header_offset);
            }
            if (tag_header_offset + 24U < bytes.size()) {
                const std::size_t expr_max = bytes.size() - tag_header_offset - 24U;
                const std::string key_expr =
                    read_ascii_hint(bytes, tag_header_offset + 24U, std::min<std::size_t>(220U, expr_max));
                if (!key_expr.empty()) {
                    tag_probe.key_expression_hint = key_expr;
                    tag_probe.key_expression_offset_hint = static_cast<std::uint32_t>(tag_header_offset + 24U);
                    tag_probe.normalization_hint = derive_normalization_hint(key_expr);
                    tag_probe.collation_hint =
                        derive_collation_hint(key_expr, tag_probe.normalization_hint);
                }
            }

            if (tag_header_offset + 4U <= bytes.size()) {
                const std::uint16_t flags = read_le_u16(bytes, tag_header_offset);
                const std::uint16_t entry_count = read_le_u16(bytes, tag_header_offset + 2U);
                tag_probe.tag_sort_marker_hint = make_tag_sort_marker(flags, entry_count);
            }

            if (tag_probe.key_expression_hint.empty() || tag_probe.for_expression_hint.empty()) {
                const std::size_t page_end = std::min(
                    bytes.size(),
                    tag_header_offset + static_cast<std::size_t>(effective_block_size));
                const std::vector<PrintableRun> expression_runs =
                    collect_expression_runs(bytes, tag_header_offset + 24U, page_end);
                for (const PrintableRun& run : expression_runs) {
                    if (tag_probe.key_expression_hint.empty()) {
                        tag_probe.key_expression_hint = run.text;
                        tag_probe.key_expression_offset_hint = static_cast<std::uint32_t>(run.offset);
                        tag_probe.normalization_hint = derive_normalization_hint(tag_probe.key_expression_hint);
                        tag_probe.collation_hint =
                            derive_collation_hint(tag_probe.key_expression_hint, tag_probe.normalization_hint);
                        continue;
                    }

                    if (tag_probe.for_expression_hint.empty() &&
                        run.offset > tag_probe.key_expression_offset_hint &&
                        looks_like_for_expression_candidate(run.text)) {
                        tag_probe.for_expression_hint = run.text;
                        tag_probe.for_expression_offset_hint = static_cast<std::uint32_t>(run.offset);
                        break;
                    }
                }
            }
        }

        probe.tags.push_back(std::move(tag_probe));
    }

    if (probe.tags.empty()) {
        return {
            .ok = false,
            .probe = probe,
            .error = "No plausible MDX tag metadata was found in the tag table."
        };
    }

    probe.key_expression_hint = probe.tags.front().key_expression_hint;
    probe.normalization_hint = probe.tags.front().normalization_hint;
    probe.collation_hint = probe.tags.front().collation_hint;

    return {.ok = true, .probe = probe};
}

}  // namespace

bool IndexProbe::looks_like_index() const {
    if (kind == IndexKind::unknown || file_size == 0U) {
        return false;
    }
    if (kind == IndexKind::mdx) {
        return file_size >= 512U;
    }
    return block_size != 0U &&
           file_size >= block_size &&
           root_node_offset_hint >= block_size &&
           root_node_offset_hint < file_size;
}

IndexKind index_kind_from_path(const std::string& path) {
    std::string extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    if (extension == ".cdx") {
        return IndexKind::cdx;
    }
    if (extension == ".dcx") {
        return IndexKind::dcx;
    }
    if (extension == ".idx") {
        return IndexKind::idx;
    }
    if (extension == ".ndx") {
        return IndexKind::ndx;
    }
    if (extension == ".mdx") {
        return IndexKind::mdx;
    }
    return IndexKind::unknown;
}

const char* index_kind_name(IndexKind kind) {
    switch (kind) {
        case IndexKind::unknown:
            return "unknown";
        case IndexKind::cdx:
            return "cdx";
        case IndexKind::dcx:
            return "dcx";
        case IndexKind::idx:
            return "idx";
        case IndexKind::ndx:
            return "ndx";
        case IndexKind::mdx:
            return "mdx";
    }
    return "unknown";
}

IndexParseResult parse_index_probe(
    const std::vector<std::uint8_t>& bytes,
    std::uint64_t file_size,
    IndexKind kind) {
    switch (kind) {
        case IndexKind::cdx:
        case IndexKind::dcx:
            return parse_cdx_family_probe(bytes, file_size, kind);
        case IndexKind::idx:
            return parse_fox_idx_probe(bytes, file_size);
        case IndexKind::ndx:
            return parse_dbase_ndx_probe(bytes, file_size);
        case IndexKind::mdx:
            return parse_dbase_mdx_probe(bytes, file_size);
        case IndexKind::unknown:
            return {.ok = false, .error = "Unknown index extension."};
    }
    return {.ok = false, .error = "Unsupported index type."};
}

IndexParseResult parse_index_probe_from_file(const std::string& path) {
    const IndexKind kind = index_kind_from_path(path);
    if (kind == IndexKind::unknown) {
        return {.ok = false, .error = "Path does not use a known xBase index extension."};
    }

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {.ok = false, .error = "Unable to open file."};
    }

    const std::uint64_t file_size = static_cast<std::uint64_t>(std::filesystem::file_size(path));
    std::size_t probe_size = 512U;
    if (kind == IndexKind::cdx || kind == IndexKind::dcx || kind == IndexKind::mdx) {
        probe_size = static_cast<std::size_t>(file_size);
    }
    std::vector<std::uint8_t> bytes(probe_size, 0U);
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

    if (input.gcount() <= 0) {
        return {.ok = false, .error = "Unable to read index header bytes."};
    }

    bytes.resize(static_cast<std::size_t>(input.gcount()));
    return parse_index_probe(bytes, file_size, kind);
}

}  // namespace copperfin::vfp

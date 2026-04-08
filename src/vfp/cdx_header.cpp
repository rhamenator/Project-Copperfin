#include "copperfin/vfp/cdx_header.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <set>

namespace copperfin::vfp {

namespace {

std::uint16_t read_le_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1]) << 8U);
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

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::vector<PrintableRun> collect_printable_runs(const std::vector<std::uint8_t>& bytes) {
    std::vector<PrintableRun> runs;

    for (std::size_t index = 0; index < bytes.size();) {
        const auto raw = static_cast<unsigned char>(bytes[index]);
        if (std::isprint(raw) == 0) {
            ++index;
            continue;
        }

        const std::size_t start = index;
        std::string text;
        while (index < bytes.size()) {
            const auto current = static_cast<unsigned char>(bytes[index]);
            if (std::isprint(current) == 0) {
                break;
            }
            text.push_back(static_cast<char>(current));
            ++index;
        }

        if (text.size() >= 4U) {
            runs.push_back({.offset = start, .text = std::move(text)});
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

    const bool has_lowercase = std::any_of(text.begin(), text.end(), [](unsigned char ch) {
        return std::islower(ch) != 0;
    });
    const bool has_identifier = std::any_of(text.begin(), text.end(), [](char ch) {
        return is_identifier_char(ch);
    });
    const bool has_underscore = text.find('_') != std::string::npos;
    const bool has_parens = text.find('(') != std::string::npos && text.find(')') != std::string::npos;
    const bool has_operator = text.find('=') != std::string::npos ||
                              text.find('<') != std::string::npos ||
                              text.find('>') != std::string::npos;
    const bool has_many_spaces = std::count(text.begin(), text.end(), ' ') > 2;

    if (!has_identifier || has_many_spaces) {
        return false;
    }

    return has_parens || (has_lowercase && (has_underscore || has_operator));
}

std::string derive_name_from_expression(const std::string& expression) {
    std::string candidate = trim_copy(expression);
    const auto open = candidate.find('(');
    const auto close = candidate.rfind(')');
    if (open != std::string::npos && close != std::string::npos && close > open + 1U) {
        const std::string inner = trim_copy(candidate.substr(open + 1U, close - open - 1U));
        if (!inner.empty() &&
            std::all_of(inner.begin(), inner.end(), [](char ch) { return is_identifier_char(ch); })) {
            candidate = inner;
        }
    }

    std::string sanitized;
    sanitized.reserve(candidate.size());

    bool previous_was_separator = false;
    for (char ch : candidate) {
        const auto raw = static_cast<unsigned char>(ch);
        if (std::isalnum(raw) != 0) {
            sanitized.push_back(static_cast<char>(std::toupper(raw)));
            previous_was_separator = false;
        } else if (ch == '_' && !previous_was_separator) {
            sanitized.push_back('_');
            previous_was_separator = true;
        } else if (!previous_was_separator) {
            sanitized.push_back('_');
            previous_was_separator = true;
        }
    }

    while (!sanitized.empty() && sanitized.front() == '_') {
        sanitized.erase(sanitized.begin());
    }
    while (!sanitized.empty() && sanitized.back() == '_') {
        sanitized.pop_back();
    }

    if (sanitized.size() > 10U) {
        sanitized.resize(10U);
    }

    return sanitized;
}

std::vector<PrintableRun> collect_expression_runs(const std::vector<std::uint8_t>& bytes) {
    std::vector<PrintableRun> expressions;
    std::set<std::string> seen_expressions;

    for (const PrintableRun& run : collect_printable_runs(bytes)) {
        const std::string candidate = trim_copy(run.text);
        if (!looks_like_expression_candidate(candidate)) {
            continue;
        }

        const std::string normalized = lowercase_copy(candidate);
        if (!seen_expressions.insert(normalized).second) {
            continue;
        }

        expressions.push_back({.offset = run.offset, .text = candidate});
    }

    return expressions;
}

std::vector<CdxTagDescriptor> extract_tag_descriptors(
    const std::vector<std::uint8_t>& bytes,
    std::size_t /*page_size*/) {
    std::vector<CdxTagDescriptor> tags;
    for (const PrintableRun& expression : collect_expression_runs(bytes)) {
        CdxTagDescriptor tag;
        tag.key_expression_hint = expression.text;
        tag.key_expression_offset_hint = static_cast<std::uint32_t>(expression.offset);
        tag.name_hint = derive_name_from_expression(expression.text);
        tag.inferred_name = !tag.name_hint.empty();
        tags.push_back(std::move(tag));
    }

    return tags;
}

}  // namespace

bool CdxHeader::looks_like_cdx() const {
    if (file_size < page_size || page_size == 0U) {
        return false;
    }
    if ((file_size % page_size) != 0U) {
        return false;
    }
    if (root_node_offset == 0U || (root_node_offset % page_size) != 0U) {
        return false;
    }
    return root_node_offset < file_size;
}

CdxParseResult parse_cdx_header(const std::vector<std::uint8_t>& bytes, std::uint64_t file_size) {
    if (bytes.size() < 16U) {
        return {.ok = false, .error = "File is smaller than the minimum CDX header probe size (16 bytes)."};
    }

    CdxHeader header;
    header.file_size = file_size;

    for (std::size_t index = 0; index < header.raw_words.size(); ++index) {
        header.raw_words[index] = read_le_u16(bytes, index * 2U);
    }

    header.root_node_offset = header.raw_words[0];
    header.next_free_node_offset = header.raw_words[1];
    header.key_length_hint = header.raw_words[6];
    header.key_pool_length_hint = header.raw_words[7];

    if (!header.looks_like_cdx()) {
        return {.ok = false, .header = header, .error = "Header values do not look like a CDX-family index file."};
    }

    CdxParseResult result{.ok = true, .header = header};
    if (bytes.size() > 16U) {
        result.tags = extract_tag_descriptors(bytes, header.page_size);
    }
    return result;
}

CdxParseResult parse_cdx_header_from_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {.ok = false, .error = "Unable to open file."};
    }

    const std::uint64_t file_size = static_cast<std::uint64_t>(std::filesystem::file_size(path));
    if (file_size < 16U) {
        return {.ok = false, .error = "Unable to read a complete 16-byte CDX header probe."};
    }

    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(file_size), 0U);
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    bytes.resize(static_cast<std::size_t>(input.gcount()));
    return parse_cdx_header(bytes, file_size);
}

}  // namespace copperfin::vfp

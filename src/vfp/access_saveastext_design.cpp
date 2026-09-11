// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/access_saveastext_design.h"
#include "copperfin/platform/path.h"

#include "copperfin/localization/localization.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string_view>

namespace copperfin::vfp {

namespace {

localization::LocalizedCatalog access_saveastext_design_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        localization::LocalizedCatalog catalog;
    };

    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        localization::load_catalogs(
            localization::resolve_catalog_root(),
            localization::default_locale)};
    const std::filesystem::path locale_root = localization::resolve_catalog_root();
    const std::string locale = localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

std::string access_saveastext_design_text(std::string_view key) {
    return access_saveastext_design_catalog().translate(key, {});
}

std::string trim_ascii(const std::string& value) {
    const std::size_t first = value.find_first_not_of(" \t");
    if (first == std::string::npos) {
        return {};
    }
    const std::size_t last = value.find_last_not_of(" \t");
    return value.substr(first, last - first + 1);
}

// Splits on '\n' and strips a trailing '\r' from each line (SaveAsText
// output is CRLF-terminated on Windows) -- deliberately preserves each
// line's own original leading/trailing whitespace otherwise, since the
// code-behind section's exact formatting is meaningful VBA source text,
// not just cosmetic indentation the way the structural section's is.
std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> result;
    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t newline = text.find('\n', start);
        std::string line = (newline == std::string::npos) ? text.substr(start) : text.substr(start, newline - start);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        result.push_back(std::move(line));
        if (newline == std::string::npos) {
            break;
        }
        start = newline + 1;
    }
    return result;
}

// Matches a trimmed line's leading "Begin" token, distinguishing a
// block-opening line (`Begin`, or `Begin <TypeName>`) from any other
// content (including a property line whose name happens to start with
// those letters, e.g. a hypothetical "Beginning" -- not observed, but
// guarded against by requiring an exact word match).
bool starts_with_begin_token(const std::string& trimmed, std::string& rest) {
    constexpr std::string_view kBegin = "Begin";
    if (trimmed.size() == kBegin.size() && trimmed == kBegin) {
        rest.clear();
        return true;
    }
    if (trimmed.size() > kBegin.size() && trimmed.compare(0, kBegin.size(), kBegin) == 0 &&
        trimmed[kBegin.size()] == ' ') {
        rest = trim_ascii(trimmed.substr(kBegin.size() + 1U));
        return true;
    }
    return false;
}

// Extracts one quoted chunk's still-escaped inner text from `text`
// (which must start with '"'), scanning for the matching unescaped
// closing quote (an escaped quote `\"` or escaped backslash `\\` does
// not terminate the chunk). Fails if no closing quote is found, or if
// non-whitespace content follows the closing quote on the same line
// (not observed in any real fixture).
bool consume_quoted_chunk(const std::string& text, std::string& out_escaped_inner, std::string& error_key) {
    std::size_t index = 1U;
    while (index < text.size()) {
        if (text[index] == '\\' && index + 1U < text.size()) {
            index += 2U;
            continue;
        }
        if (text[index] == '"') {
            out_escaped_inner = text.substr(1U, index - 1U);
            if (!trim_ascii(text.substr(index + 1U)).empty()) {
                error_key = "Vfp.AccessDesign.Error.MalformedPropertyLine";
                return false;
            }
            return true;
        }
        ++index;
    }
    error_key = "Vfp.AccessDesign.Error.UnterminatedString";
    return false;
}

// Unescapes `\"` -> `"` and `\\` -> `\` -- the only two escape sequences
// confirmed via real SaveAsText output (a combo-box ColumnInfo/BaseInfo
// value with embedded escaped quotes, and a Picture property holding a
// Windows path with escaped backslashes). See this module's own header
// for the fixture evidence.
std::string unescape_string(const std::string& escaped) {
    std::string result;
    result.reserve(escaped.size());
    for (std::size_t index = 0U; index < escaped.size(); ++index) {
        if (escaped[index] == '\\' && index + 1U < escaped.size() &&
            (escaped[index + 1U] == '"' || escaped[index + 1U] == '\\')) {
            result.push_back(escaped[index + 1U]);
            ++index;
        } else {
            result.push_back(escaped[index]);
        }
    }
    return result;
}

struct ParseState {
    const std::vector<std::string>& lines;
    std::size_t index = 0U;
};

bool parse_block_body(ParseState& state, AccessDesignControl& node, std::string& error_key);

// Parses an anonymous `Begin ... End` "children container" block --
// confirmed via real fixtures to hold nothing but a sequence of typed
// child control blocks (e.g. a CommandButton's own child Label, or a
// Section's many sibling controls) -- appending them directly to
// `parent.children` rather than nesting a synthetic wrapper node.
bool parse_children_container(ParseState& state, AccessDesignControl& parent, std::string& error_key) {
    while (true) {
        if (state.index >= state.lines.size()) {
            error_key = "Vfp.AccessDesign.Error.UnbalancedBlock";
            return false;
        }
        const std::string trimmed = trim_ascii(state.lines[state.index]);
        if (trimmed.empty()) {
            ++state.index;
            continue;
        }
        if (trimmed == "End") {
            ++state.index;
            return true;
        }
        std::string rest;
        if (!starts_with_begin_token(trimmed, rest) || rest.empty()) {
            error_key = "Vfp.AccessDesign.Error.MalformedPropertyLine";
            return false;
        }
        ++state.index;
        AccessDesignControl child;
        child.control_type = rest;
        if (!parse_block_body(state, child, error_key)) {
            return false;
        }
        parent.children.push_back(std::move(child));
    }
}

bool parse_blob_lines(ParseState& state, std::vector<std::string>& out_lines, std::string& error_key) {
    while (true) {
        if (state.index >= state.lines.size()) {
            error_key = "Vfp.AccessDesign.Error.UnbalancedBlock";
            return false;
        }
        const std::string trimmed = trim_ascii(state.lines[state.index]);
        ++state.index;
        if (trimmed == "End") {
            return true;
        }
        out_lines.push_back(trimmed);
    }
}

// Parses one block's body (everything between its opening `Begin
// [Type]` line, already consumed by the caller, and its own matching
// `End`), populating `node.properties`/`node.children`.
bool parse_block_body(ParseState& state, AccessDesignControl& node, std::string& error_key) {
    while (true) {
        if (state.index >= state.lines.size()) {
            error_key = "Vfp.AccessDesign.Error.UnbalancedBlock";
            return false;
        }
        const std::string trimmed = trim_ascii(state.lines[state.index]);
        if (trimmed.empty()) {
            ++state.index;
            continue;
        }
        if (trimmed == "End") {
            ++state.index;
            return true;
        }

        std::string rest;
        if (starts_with_begin_token(trimmed, rest)) {
            ++state.index;
            if (rest.empty()) {
                if (!parse_children_container(state, node, error_key)) {
                    return false;
                }
            } else {
                AccessDesignControl child;
                child.control_type = rest;
                if (!parse_block_body(state, child, error_key)) {
                    return false;
                }
                node.children.push_back(std::move(child));
            }
            continue;
        }

        const std::size_t equals_pos = trimmed.find('=');
        if (equals_pos == std::string::npos) {
            error_key = "Vfp.AccessDesign.Error.MalformedPropertyLine";
            return false;
        }
        const std::string name = trim_ascii(trimmed.substr(0U, equals_pos));
        const std::string raw_value = trim_ascii(trimmed.substr(equals_pos + 1U));
        if (name.empty()) {
            error_key = "Vfp.AccessDesign.Error.MalformedPropertyLine";
            return false;
        }
        ++state.index;

        AccessDesignProperty property;
        property.name = name;
        if (raw_value == "Begin") {
            property.is_blob = true;
            if (!parse_blob_lines(state, property.blob_lines, error_key)) {
                return false;
            }
        } else if (raw_value == "NotDefault") {
            property.is_not_default = true;
        } else if (!raw_value.empty() && raw_value.front() == '"') {
            property.is_string = true;
            std::string accumulated_escaped;
            if (!consume_quoted_chunk(raw_value, accumulated_escaped, error_key)) {
                return false;
            }
            while (state.index < state.lines.size()) {
                const std::string next_trimmed = trim_ascii(state.lines[state.index]);
                if (next_trimmed.empty() || next_trimmed.front() != '"') {
                    break;
                }
                ++state.index;
                std::string chunk;
                if (!consume_quoted_chunk(next_trimmed, chunk, error_key)) {
                    return false;
                }
                accumulated_escaped += chunk;
            }
            property.value = unescape_string(accumulated_escaped);
        } else {
            property.value = raw_value;
        }
        node.properties.push_back(std::move(property));
    }
}

}  // namespace

const AccessDesignProperty* AccessDesignControl::find_property(std::string_view name) const {
    for (const AccessDesignProperty& property : properties) {
        if (property.name == name) {
            return &property;
        }
    }
    return nullptr;
}

AccessDesignParseResult parse_access_saveastext_design(const std::string& text) {
    AccessDesignParseResult result;
    const std::vector<std::string> lines = split_lines(text);

    std::size_t index = 0U;
    const auto parse_header_field = [&](std::string_view field_name, std::int64_t& out_value) -> bool {
        if (index >= lines.size()) {
            return false;
        }
        const std::string trimmed = trim_ascii(lines[index]);
        const std::string prefix = std::string(field_name) + " =";
        if (trimmed.compare(0U, prefix.size(), prefix) != 0) {
            return false;
        }
        const std::string number_text = trimmed.substr(prefix.size());
        if (number_text.empty()) {
            return false;
        }
        try {
            std::size_t consumed = 0U;
            out_value = std::stoll(number_text, &consumed);
            if (consumed != number_text.size()) {
                return false;
            }
        } catch (const std::exception&) {
            return false;
        }
        ++index;
        return true;
    };

    if (!parse_header_field("Version", result.version) ||
        !parse_header_field("VersionRequired", result.version_required) ||
        !parse_header_field("Checksum", result.checksum)) {
        result.error = access_saveastext_design_text("Vfp.AccessDesign.Error.MalformedHeader");
        return result;
    }

    if (index >= lines.size()) {
        result.error = access_saveastext_design_text("Vfp.AccessDesign.Error.MissingRootBlock");
        return result;
    }
    std::string root_type;
    if (!starts_with_begin_token(trim_ascii(lines[index]), root_type) || root_type.empty()) {
        result.error = access_saveastext_design_text("Vfp.AccessDesign.Error.MissingRootBlock");
        return result;
    }
    if (root_type != "Form" && root_type != "Report") {
        result.error = access_saveastext_design_text("Vfp.AccessDesign.Error.UnsupportedRootType");
        return result;
    }
    ++index;

    result.root.control_type = root_type;
    ParseState state{lines, index};
    std::string error_key;
    if (!parse_block_body(state, result.root, error_key)) {
        result.error = access_saveastext_design_text(error_key);
        return result;
    }
    index = state.index;

    if (index < lines.size()) {
        const std::string trimmed = trim_ascii(lines[index]);
        if (trimmed == "CodeBehindForm") {
            ++index;
            std::string code_behind;
            for (std::size_t line_index = index; line_index < lines.size(); ++line_index) {
                code_behind += lines[line_index];
                if (line_index + 1U < lines.size()) {
                    code_behind += "\n";
                }
            }
            result.code_behind = std::move(code_behind);
        } else if (!trimmed.empty()) {
            result.error = access_saveastext_design_text("Vfp.AccessDesign.Error.UnexpectedTrailingContent");
            return result;
        }
    }

    result.ok = true;
    return result;
}

AccessDesignParseResult parse_access_saveastext_design_from_file(const std::string& path) {
    std::ifstream input(platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        AccessDesignParseResult result;
        result.error = access_saveastext_design_text("Vfp.AccessDesign.Error.OpenFileFailed");
        return result;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return parse_access_saveastext_design(buffer.str());
}

}  // namespace copperfin::vfp

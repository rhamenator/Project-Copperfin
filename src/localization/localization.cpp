#include "copperfin/localization/localization.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>

namespace copperfin::localization {

namespace {

std::string trim_copy(std::string_view value) {
    std::size_t first = 0U;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])) != 0) {
        ++first;
    }
    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1U])) != 0) {
        --last;
    }
    return std::string(value.substr(first, last - first));
}

std::string lowercase_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

void append_unique(std::vector<std::string>& values, const std::string& value) {
    if (!value.empty() && std::find(values.begin(), values.end(), value) == values.end()) {
        values.push_back(value);
    }
}

std::string environment_value(const char* name) {
#ifdef _WIN32
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, name) != 0 || raw == nullptr) {
        return {};
    }
    std::string value(raw);
    std::free(raw);
    return value;
#else
    if (const char* raw = std::getenv(name); raw != nullptr) {
        return raw;
    }
    return {};
#endif
}

void skip_json_space(std::string_view text, std::size_t& offset) {
    while (offset < text.size() && std::isspace(static_cast<unsigned char>(text[offset])) != 0) {
        ++offset;
    }
}

int hex_value(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return 10 + ch - 'a';
    }
    if (ch >= 'A' && ch <= 'F') {
        return 10 + ch - 'A';
    }
    return -1;
}

void append_utf8(std::string& value, unsigned int codepoint) {
    if (codepoint <= 0x7FU) {
        value.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FFU) {
        value.push_back(static_cast<char>(0xC0U | (codepoint >> 6U)));
        value.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else {
        value.push_back(static_cast<char>(0xE0U | (codepoint >> 12U)));
        value.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
        value.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    }
}

bool parse_json_string(std::string_view text, std::size_t& offset, std::string& value, std::string& error) {
    if (offset >= text.size() || text[offset] != '"') {
        error = "Catalog.Json.ExpectedString";
        return false;
    }
    ++offset;
    value.clear();
    while (offset < text.size()) {
        const char ch = text[offset++];
        if (ch == '"') {
            return true;
        }
        if (ch != '\\') {
            value.push_back(ch);
            continue;
        }
        if (offset >= text.size()) {
            error = "Catalog.Json.UnterminatedEscape";
            return false;
        }
        const char escaped = text[offset++];
        switch (escaped) {
            case '"':
            case '\\':
            case '/':
                value.push_back(escaped);
                break;
            case 'b':
                value.push_back('\b');
                break;
            case 'f':
                value.push_back('\f');
                break;
            case 'n':
                value.push_back('\n');
                break;
            case 'r':
                value.push_back('\r');
                break;
            case 't':
                value.push_back('\t');
                break;
            case 'u': {
                if (offset + 4U > text.size()) {
                    error = "Catalog.Json.IncompleteUnicodeEscape";
                    return false;
                }
                unsigned int codepoint = 0U;
                for (std::size_t index = 0U; index < 4U; ++index) {
                    const int hex = hex_value(text[offset + index]);
                    if (hex < 0) {
                        error = "Catalog.Json.InvalidUnicodeEscape";
                        return false;
                    }
                    codepoint = (codepoint << 4U) | static_cast<unsigned int>(hex);
                }
                offset += 4U;
                append_utf8(value, codepoint);
                break;
            }
            default:
                error = "Catalog.Json.UnsupportedEscape";
                return false;
        }
    }
    error = "Catalog.Json.UnterminatedString";
    return false;
}

std::string pseudo_localize_segment(std::string_view segment) {
    std::string result;
    result.reserve(segment.size() * 2U);
    for (const char ch : segment) {
        switch (ch) {
            case 'A':
                result += "\xC3\x85";
                break;
            case 'a':
                result += "\xC3\xA5";
                break;
            case 'B':
                result += "\xC3\x9F";
                break;
            case 'b':
                result += "\xC3\x9F";
                break;
            case 'E':
                result += "\xC3\x8B";
                break;
            case 'e':
                result += "\xC3\xAB";
                break;
            case 'I':
                result += "\xC3\x8F";
                break;
            case 'i':
                result += "\xC3\xAF";
                break;
            case 'O':
                result += "\xC3\x98";
                break;
            case 'o':
                result += "\xC3\xB8";
                break;
            case 'U':
                result += "\xC3\x9C";
                break;
            case 'u':
                result += "\xC3\xBC";
                break;
            case 'C':
                result += "\xC3\x87";
                break;
            case 'c':
                result += "\xC3\xA7";
                break;
            case 'N':
                result += "\xC3\x91";
                break;
            case 'n':
                result += "\xC3\xB1";
                break;
            case 'T':
                result += "\xC5\xA2";
                break;
            case 't':
                result += "\xC5\xA3";
                break;
            default:
                result.push_back(ch);
                break;
        }
    }
    return result;
}

}  // namespace

std::string normalize_locale(std::string_view locale) {
    std::string value = trim_copy(locale);
    std::replace(value.begin(), value.end(), '_', '-');
    if (value.empty()) {
        return std::string(default_locale);
    }

    std::vector<std::string> parts;
    std::size_t start = 0U;
    while (start <= value.size()) {
        const std::size_t separator = value.find('-', start);
        parts.push_back(value.substr(start, separator == std::string::npos ? std::string::npos : separator - start));
        if (separator == std::string::npos) {
            break;
        }
        start = separator + 1U;
    }

    for (std::size_t index = 0U; index < parts.size(); ++index) {
        if (index == 0U) {
            parts[index] = lowercase_ascii(parts[index]);
        } else if (parts[index].size() == 2U) {
            std::transform(parts[index].begin(), parts[index].end(), parts[index].begin(), [](unsigned char ch) {
                return static_cast<char>(std::toupper(ch));
            });
        } else {
            parts[index] = lowercase_ascii(parts[index]);
        }
    }

    std::ostringstream output;
    for (std::size_t index = 0U; index < parts.size(); ++index) {
        if (index != 0U) {
            output << '-';
        }
        output << parts[index];
    }
    return output.str();
}

std::vector<std::string> locale_fallback_chain(std::string_view locale) {
    const std::string normalized = normalize_locale(locale);
    const std::string lowered = lowercase_ascii(normalized);
    std::vector<std::string> chain;
    append_unique(chain, normalized);

    if (lowered.rfind("es-", 0U) == 0U && lowered != "es-419") {
        append_unique(chain, "es-419");
    }
    const std::size_t separator = normalized.find('-');
    if (separator != std::string::npos) {
        append_unique(chain, normalized.substr(0U, separator));
    }
    append_unique(chain, std::string(default_locale));
    return chain;
}

std::string select_locale(std::string_view explicit_locale) {
    if (!trim_copy(explicit_locale).empty()) {
        return normalize_locale(explicit_locale);
    }
    const std::string configured = environment_value("COPPERFIN_LOCALE");
    if (!trim_copy(configured).empty()) {
        return normalize_locale(configured);
    }
    return std::string(default_locale);
}

CatalogLoadResult parse_catalog_json(std::string_view text) {
    CatalogLoadResult result;
    std::size_t offset = 0U;
    skip_json_space(text, offset);
    if (offset >= text.size() || text[offset] != '{') {
        result.error = "Catalog.Json.ExpectedObject";
        return result;
    }
    ++offset;
    skip_json_space(text, offset);
    if (offset < text.size() && text[offset] == '}') {
        ++offset;
        skip_json_space(text, offset);
        result.ok = offset == text.size();
        if (!result.ok) {
            result.error = "Catalog.Json.TrailingContent";
        }
        return result;
    }

    while (offset < text.size()) {
        std::string key;
        if (!parse_json_string(text, offset, key, result.error)) {
            return result;
        }
        skip_json_space(text, offset);
        if (offset >= text.size() || text[offset] != ':') {
            result.error = "Catalog.Json.ExpectedColon";
            return result;
        }
        ++offset;
        skip_json_space(text, offset);
        std::string value;
        if (!parse_json_string(text, offset, value, result.error)) {
            return result;
        }
        result.entries[key] = value;
        skip_json_space(text, offset);
        if (offset < text.size() && text[offset] == ',') {
            ++offset;
            skip_json_space(text, offset);
            continue;
        }
        if (offset < text.size() && text[offset] == '}') {
            ++offset;
            skip_json_space(text, offset);
            if (offset != text.size()) {
                result.error = "Catalog.Json.TrailingContent";
                return result;
            }
            result.ok = true;
            return result;
        }
        result.error = "Catalog.Json.ExpectedCommaOrObjectEnd";
        return result;
    }

    result.error = "Catalog.Json.UnterminatedObject";
    return result;
}

CatalogLoadResult load_catalog_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        CatalogLoadResult result;
        result.error = "Catalog.FileMissing";
        return result;
    }
    const std::string text{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
    CatalogLoadResult result = parse_catalog_json(text);
    if (!result.ok && result.error.empty()) {
        result.error = "Catalog.ParseFailed";
    }
    return result;
}

LocalizedCatalog load_catalogs(const std::filesystem::path& locale_root, std::string_view requested_locale) {
    LocalizedCatalog catalog;
    catalog.requested_locale = normalize_locale(requested_locale);
    catalog.fallback_chain = locale_fallback_chain(catalog.requested_locale);

    for (const std::string& locale : catalog.fallback_chain) {
        const std::filesystem::path catalog_path = locale_root / locale / "strings.json";
        const CatalogLoadResult loaded = load_catalog_file(catalog_path);
        if (loaded.ok) {
            catalog.catalogs[locale] = loaded.entries;
        } else if (locale == std::string(default_locale)) {
            catalog.load_errors.push_back(loaded.error);
        }
    }
    return catalog;
}

std::filesystem::path resolve_catalog_root(const std::filesystem::path& executable_path) {
    const std::string configured = environment_value("COPPERFIN_LOCALE_DIR");
    if (!trim_copy(configured).empty()) {
        return configured;
    }

    if (!executable_path.empty()) {
        const std::filesystem::path executable_root = std::filesystem::absolute(executable_path).parent_path();
        const std::vector<std::filesystem::path> candidates{
            executable_root / ".." / "share" / "copperfin" / "locales",
            executable_root / "share" / "copperfin" / "locales",
            executable_root / ".." / "resources" / "locales"
        };
        for (const auto& candidate : candidates) {
            if (std::filesystem::exists(candidate)) {
                return candidate;
            }
        }
    }

    const std::filesystem::path developer_tree = std::filesystem::current_path() / "resources" / "locales";
    if (std::filesystem::exists(developer_tree)) {
        return developer_tree;
    }
    return "resources/locales";
}

std::optional<std::string> LocalizedCatalog::find(std::string_view key) const {
    for (const std::string& locale : fallback_chain) {
        const auto catalog = catalogs.find(locale);
        if (catalog == catalogs.end()) {
            continue;
        }
        const auto entry = catalog->second.find(std::string(key));
        if (entry != catalog->second.end()) {
            if (normalize_locale(requested_locale) == std::string(pseudo_locale)) {
                return pseudo_localize(entry->second);
            }
            return entry->second;
        }
    }
    return std::nullopt;
}

std::string LocalizedCatalog::translate(std::string_view key, const PlaceholderMap& placeholders) const {
    const std::optional<std::string> pattern = find(key);
    if (!pattern.has_value()) {
        return std::string(key);
    }
    return format_named_placeholders(*pattern, placeholders);
}

std::string format_named_placeholders(std::string_view pattern, const PlaceholderMap& placeholders) {
    std::string result;
    result.reserve(pattern.size());
    std::size_t offset = 0U;
    while (offset < pattern.size()) {
        if (pattern[offset] != '{') {
            result.push_back(pattern[offset++]);
            continue;
        }
        const std::size_t close = pattern.find('}', offset + 1U);
        if (close == std::string_view::npos) {
            result.append(pattern.substr(offset));
            break;
        }
        const std::string name(pattern.substr(offset + 1U, close - offset - 1U));
        const auto replacement = placeholders.find(name);
        if (replacement == placeholders.end()) {
            result.append(pattern.substr(offset, close - offset + 1U));
        } else {
            result.append(replacement->second);
        }
        offset = close + 1U;
    }
    return result;
}

std::string pseudo_localize(std::string_view pattern) {
    std::string result = "[!! ";
    std::size_t offset = 0U;
    while (offset < pattern.size()) {
        const std::size_t open = pattern.find('{', offset);
        if (open == std::string_view::npos) {
            result += pseudo_localize_segment(pattern.substr(offset));
            break;
        }
        result += pseudo_localize_segment(pattern.substr(offset, open - offset));
        const std::size_t close = pattern.find('}', open + 1U);
        if (close == std::string_view::npos) {
            result += pseudo_localize_segment(pattern.substr(open));
            break;
        }
        result.append(pattern.substr(open, close - open + 1U));
        offset = close + 1U;
    }
    result += " !!]";
    return result;
}

}  // namespace copperfin::localization

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/platform/polyglot_route_registry.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace copperfin::platform {

namespace {

bool is_lowercase_ascii_letter(char value) noexcept {
    return value >= 'a' && value <= 'z';
}

bool is_capability_id(std::string_view value) noexcept {
    if (value.empty() || !is_lowercase_ascii_letter(value.front())) {
        return false;
    }
    return std::all_of(value.begin() + 1, value.end(), [](char character) {
        return is_lowercase_ascii_letter(character) ||
            (character >= '0' && character <= '9') ||
            character == '.' || character == '_' || character == '-';
    });
}

PolyglotRouteRegistryResult invalid_result(
    PolyglotRouteConfigError error,
    const char* error_code) {
    PolyglotRouteRegistryResult result;
    result.error = error;
    result.error_code = error_code;
    return result;
}

class JsonCursor {
public:
    explicit JsonCursor(std::string_view document) : document_(document) {}

    [[nodiscard]] bool consume(char expected) noexcept {
        skip_whitespace();
        if (position_ >= document_.size() || document_[position_] != expected) {
            return false;
        }
        ++position_;
        return true;
    }

    [[nodiscard]] bool next_is(char expected) const noexcept {
        std::size_t position = position_;
        while (position < document_.size()) {
            const char character = document_[position];
            if (character != ' ' && character != '\t' && character != '\r' && character != '\n') {
                return character == expected;
            }
            ++position;
        }
        return false;
    }

    [[nodiscard]] bool parse_string(std::string& value) {
        skip_whitespace();
        if (position_ >= document_.size() || document_[position_] != '"') {
            return false;
        }
        ++position_;
        value.clear();
        while (position_ < document_.size()) {
            const char character = document_[position_++];
            if (character == '"') {
                return true;
            }
            if (static_cast<unsigned char>(character) < 0x20U) {
                return false;
            }
            if (character != '\\') {
                value.push_back(character);
                continue;
            }
            if (position_ >= document_.size()) {
                return false;
            }
            const char escaped = document_[position_++];
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
            default:
                return false;
            }
        }
        return false;
    }

    [[nodiscard]] bool parse_uint8(std::uint8_t& value) noexcept {
        skip_whitespace();
        if (position_ >= document_.size() ||
            document_[position_] < '0' || document_[position_] > '9') {
            return false;
        }
        unsigned int parsed = 0U;
        while (position_ < document_.size() &&
               document_[position_] >= '0' && document_[position_] <= '9') {
            const unsigned int digit = static_cast<unsigned int>(document_[position_] - '0');
            if (parsed > (std::numeric_limits<unsigned int>::max() - digit) / 10U) {
                return false;
            }
            parsed = parsed * 10U + digit;
            ++position_;
        }
        if (parsed > std::numeric_limits<std::uint8_t>::max()) {
            return false;
        }
        value = static_cast<std::uint8_t>(parsed);
        return true;
    }

    [[nodiscard]] bool at_end() const noexcept {
        std::size_t position = position_;
        while (position < document_.size()) {
            const char character = document_[position];
            if (character != ' ' && character != '\t' && character != '\r' && character != '\n') {
                return false;
            }
            ++position;
        }
        return true;
    }

private:
    void skip_whitespace() noexcept {
        while (position_ < document_.size()) {
            const char character = document_[position_];
            if (character != ' ' && character != '\t' && character != '\r' && character != '\n') {
                break;
            }
            ++position_;
        }
    }

    std::string_view document_;
    std::size_t position_ = 0U;
};

bool parse_route_object(JsonCursor& cursor, PolyglotRouteConfig& config) {
    if (!cursor.consume('{')) {
        return false;
    }
    bool has_capability_id = false;
    bool has_state = false;
    bool has_canary_percentage = false;
    while (!cursor.consume('}')) {
        std::string key;
        if (!cursor.parse_string(key) || !cursor.consume(':')) {
            return false;
        }
        if (key == "capability_id") {
            if (has_capability_id || !cursor.parse_string(config.capability_id)) {
                return false;
            }
            has_capability_id = true;
        } else if (key == "state") {
            if (has_state || !cursor.parse_string(config.state)) {
                return false;
            }
            has_state = true;
        } else if (key == "canary_percentage") {
            if (has_canary_percentage || !cursor.parse_uint8(config.canary_percentage)) {
                return false;
            }
            has_canary_percentage = true;
        } else {
            return false;
        }
        if (cursor.consume('}')) {
            break;
        }
        if (!cursor.consume(',')) {
            return false;
        }
        if (cursor.next_is('}')) {
            return false;
        }
    }
    return has_capability_id && has_state;
}

PolyglotRouteRegistryResult parse_route_document(std::string_view document) {
    JsonCursor cursor(document);
    if (!cursor.consume('{')) {
        return invalid_result(
            PolyglotRouteConfigError::invalid_document,
            "polyglot.route.invalid_document");
    }

    std::vector<PolyglotRouteConfig> configs;
    bool has_version = false;
    bool has_routes = false;
    while (!cursor.consume('}')) {
        std::string key;
        if (!cursor.parse_string(key) || !cursor.consume(':')) {
            return invalid_result(
                PolyglotRouteConfigError::invalid_document,
                "polyglot.route.invalid_document");
        }
        if (key == "registry_version") {
            std::string version;
            if (has_version || !cursor.parse_string(version)) {
                return invalid_result(
                    PolyglotRouteConfigError::invalid_document,
                    "polyglot.route.invalid_document");
            }
            has_version = true;
            if (version != "1.0") {
                return invalid_result(
                    PolyglotRouteConfigError::invalid_version,
                    "polyglot.route.invalid_version");
            }
        } else if (key == "routes") {
            if (has_routes || !cursor.consume('[')) {
                return invalid_result(
                    PolyglotRouteConfigError::invalid_document,
                    "polyglot.route.invalid_document");
            }
            has_routes = true;
            if (!cursor.consume(']')) {
                while (true) {
                    PolyglotRouteConfig config;
                    if (!parse_route_object(cursor, config)) {
                        return invalid_result(
                            PolyglotRouteConfigError::invalid_document,
                            "polyglot.route.invalid_document");
                    }
                    configs.push_back(std::move(config));
                    if (cursor.consume(']')) {
                        break;
                    }
                    if (!cursor.consume(',')) {
                        return invalid_result(
                            PolyglotRouteConfigError::invalid_document,
                            "polyglot.route.invalid_document");
                    }
                    if (cursor.next_is(']')) {
                        return invalid_result(
                            PolyglotRouteConfigError::invalid_document,
                            "polyglot.route.invalid_document");
                    }
                }
            }
        } else {
            return invalid_result(
                PolyglotRouteConfigError::invalid_document,
                "polyglot.route.invalid_document");
        }
        if (cursor.consume('}')) {
            break;
        }
        if (!cursor.consume(',')) {
            return invalid_result(
                PolyglotRouteConfigError::invalid_document,
                "polyglot.route.invalid_document");
        }
        if (cursor.next_is('}')) {
            return invalid_result(
                PolyglotRouteConfigError::invalid_document,
                "polyglot.route.invalid_document");
        }
    }
    if (!cursor.at_end()) {
        return invalid_result(
            PolyglotRouteConfigError::invalid_document,
            "polyglot.route.invalid_document");
    }
    if (!has_version) {
        return invalid_result(
            PolyglotRouteConfigError::invalid_version,
            "polyglot.route.invalid_version");
    }
    if (!has_routes) {
        return invalid_result(
            PolyglotRouteConfigError::missing_routes,
            "polyglot.route.missing_routes");
    }
    return load_polyglot_route_registry(configs);
}

}  // namespace

const char* polyglot_route_state_name(PolyglotRouteState state) noexcept {
    switch (state) {
    case PolyglotRouteState::off:
        return "off";
    case PolyglotRouteState::shadow:
        return "shadow";
    case PolyglotRouteState::canary:
        return "canary";
    case PolyglotRouteState::on:
        return "on";
    case PolyglotRouteState::retire_legacy:
        return "retire-legacy";
    }
    return "off";
}

bool parse_polyglot_route_state(
    std::string_view value,
    PolyglotRouteState& state) noexcept {
    if (value == "off") {
        state = PolyglotRouteState::off;
    } else if (value == "shadow") {
        state = PolyglotRouteState::shadow;
    } else if (value == "canary") {
        state = PolyglotRouteState::canary;
    } else if (value == "on") {
        state = PolyglotRouteState::on;
    } else if (value == "retire-legacy") {
        state = PolyglotRouteState::retire_legacy;
    } else {
        state = PolyglotRouteState::off;
        return false;
    }
    return true;
}

PolyglotRouteRegistryResult load_polyglot_route_registry(
    const std::vector<PolyglotRouteConfig>& configs) {
    PolyglotRouteRegistryResult result;
    result.registry.entries.reserve(configs.size());

    for (const PolyglotRouteConfig& config : configs) {
        if (config.capability_id.empty()) {
            return invalid_result(
                PolyglotRouteConfigError::capability_id_required,
                "polyglot.route.capability_id_required");
        }
        if (!is_capability_id(config.capability_id)) {
            return invalid_result(
                PolyglotRouteConfigError::invalid_capability_id,
                "polyglot.route.invalid_capability_id");
        }
        if (std::any_of(
                result.registry.entries.begin(),
                result.registry.entries.end(),
                [&config](const PolyglotRouteEntry& entry) {
                    return entry.capability_id == config.capability_id;
                })) {
            return invalid_result(
                PolyglotRouteConfigError::duplicate_capability_id,
                "polyglot.route.duplicate_capability_id");
        }

        PolyglotRouteState state = PolyglotRouteState::off;
        if (!parse_polyglot_route_state(config.state, state)) {
            return invalid_result(
                PolyglotRouteConfigError::invalid_state,
                "polyglot.route.invalid_state");
        }
        if (config.canary_percentage > 100U ||
            (state != PolyglotRouteState::canary && config.canary_percentage != 0U)) {
            return invalid_result(
                PolyglotRouteConfigError::invalid_canary_percentage,
                config.canary_percentage > 100U
                    ? "polyglot.route.canary_percentage_out_of_range"
                    : "polyglot.route.canary_percentage_not_applicable");
        }

        result.registry.entries.push_back(
            PolyglotRouteEntry{
                config.capability_id,
                state,
                config.canary_percentage});
    }
    return result;
}

PolyglotRouteRegistryResult load_polyglot_route_registry_json(
    std::string_view document) {
    return parse_route_document(document);
}

PolyglotRouteDecision evaluate_polyglot_route(
    const PolyglotRouteRegistry& registry,
    std::string_view capability_id,
    std::uint8_t selection_sample) {
    PolyglotRouteDecision decision;
    const auto entry = std::find_if(
        registry.entries.begin(),
        registry.entries.end(),
        [capability_id](const PolyglotRouteEntry& candidate) {
            return candidate.capability_id == capability_id;
        });
    if (entry == registry.entries.end()) {
        decision.reason_code = "polyglot.route.default_off";
        return decision;
    }

    decision.state = entry->state;
    switch (entry->state) {
    case PolyglotRouteState::off:
        decision.reason_code = "polyglot.route.off";
        break;
    case PolyglotRouteState::shadow:
        decision.selection = PolyglotRouteSelection::shadow;
        decision.invoke_candidate = true;
        decision.reason_code = "polyglot.route.shadow";
        break;
    case PolyglotRouteState::canary:
        if (selection_sample < entry->canary_percentage) {
            decision.selection = PolyglotRouteSelection::candidate;
            decision.invoke_native = false;
            decision.invoke_candidate = true;
            decision.candidate_primary = true;
            decision.reason_code = "polyglot.route.canary_candidate";
        } else {
            decision.reason_code = "polyglot.route.canary_native";
        }
        break;
    case PolyglotRouteState::on:
        decision.selection = PolyglotRouteSelection::candidate;
        decision.invoke_native = false;
        decision.invoke_candidate = true;
        decision.candidate_primary = true;
        decision.reason_code = "polyglot.route.on";
        break;
    case PolyglotRouteState::retire_legacy:
        decision.selection = PolyglotRouteSelection::candidate;
        decision.invoke_native = false;
        decision.invoke_candidate = true;
        decision.candidate_primary = true;
        decision.native_fallback_allowed = false;
        decision.reason_code = "polyglot.route.retire_legacy";
        break;
    }
    return decision;
}

}  // namespace copperfin::platform

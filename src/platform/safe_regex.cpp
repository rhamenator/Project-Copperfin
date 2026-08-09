// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/safe_regex.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace copperfin::platform {

namespace {

constexpr std::size_t hard_max_input_bytes = 64U * 1024U;
constexpr std::size_t hard_max_pattern_bytes = 256U;
constexpr std::size_t hard_max_state_count = 512U;
constexpr std::size_t no_state = std::numeric_limits<std::size_t>::max();

enum class MatcherKind {
    literal,
    any_byte,
    byte_class
};

struct ByteSet final {
    std::array<std::uint64_t, 4U> words{};

    void add(unsigned char value) noexcept {
        words[value / 64U] |= std::uint64_t{1U} << (value % 64U);
    }

    void add_range(unsigned char first, unsigned char last) noexcept {
        for (unsigned int value = first; value <= last; ++value) {
            add(static_cast<unsigned char>(value));
        }
    }

    void merge(const ByteSet& other) noexcept {
        for (std::size_t index = 0U; index < words.size(); ++index) {
            words[index] |= other.words[index];
        }
    }

    [[nodiscard]] bool contains(unsigned char value) const noexcept {
        return (words[value / 64U] & (std::uint64_t{1U} << (value % 64U))) != 0U;
    }
};

struct Matcher final {
    MatcherKind kind = MatcherKind::literal;
    unsigned char literal = 0U;
    ByteSet bytes;
    bool negated = false;
};

enum class Quantifier {
    exactly_one,
    zero_or_one,
    zero_or_more,
    one_or_more
};

struct Atom final {
    Matcher matcher;
    Quantifier quantifier = Quantifier::exactly_one;
};

enum class StateKind {
    match,
    split,
    accept
};

struct State final {
    StateKind kind = StateKind::accept;
    Matcher matcher;
    std::size_t out1 = no_state;
    std::size_t out2 = no_state;
};

struct CompiledPattern final {
    SafeRegexError error = SafeRegexError::none;
    bool anchor_start = false;
    bool anchor_end = false;
    std::size_t start_state = no_state;
    std::size_t accept_state = no_state;
    std::vector<State> states;
};

[[nodiscard]] bool limits_valid(const SafeRegexLimits& limits) noexcept {
    return limits.max_input_bytes > 0U &&
           limits.max_input_bytes <= hard_max_input_bytes &&
           limits.max_pattern_bytes > 0U &&
           limits.max_pattern_bytes <= hard_max_pattern_bytes &&
           limits.max_state_count > 0U &&
           limits.max_state_count <= hard_max_state_count;
}

[[nodiscard]] unsigned char ascii_other_case(unsigned char value) noexcept {
    if (value >= static_cast<unsigned char>('A') && value <= static_cast<unsigned char>('Z')) {
        return static_cast<unsigned char>(value + ('a' - 'A'));
    }
    if (value >= static_cast<unsigned char>('a') && value <= static_cast<unsigned char>('z')) {
        return static_cast<unsigned char>(value - ('a' - 'A'));
    }
    return value;
}

[[nodiscard]] bool matches(
    const Matcher& matcher,
    unsigned char value,
    bool ignore_ascii_case) noexcept {
    if (matcher.kind == MatcherKind::any_byte) {
        return true;
    }
    if (matcher.kind == MatcherKind::literal) {
        return value == matcher.literal ||
               (ignore_ascii_case && ascii_other_case(value) == matcher.literal);
    }
    const bool member = matcher.bytes.contains(value) ||
                        (ignore_ascii_case && matcher.bytes.contains(ascii_other_case(value)));
    return matcher.negated ? !member : member;
}

[[nodiscard]] ByteSet shorthand_bytes(unsigned char code) noexcept {
    ByteSet result;
    switch (code) {
        case static_cast<unsigned char>('d'):
        case static_cast<unsigned char>('D'):
            result.add_range(static_cast<unsigned char>('0'), static_cast<unsigned char>('9'));
            break;
        case static_cast<unsigned char>('w'):
        case static_cast<unsigned char>('W'):
            result.add_range(static_cast<unsigned char>('0'), static_cast<unsigned char>('9'));
            result.add_range(static_cast<unsigned char>('A'), static_cast<unsigned char>('Z'));
            result.add_range(static_cast<unsigned char>('a'), static_cast<unsigned char>('z'));
            result.add(static_cast<unsigned char>('_'));
            break;
        case static_cast<unsigned char>('s'):
        case static_cast<unsigned char>('S'):
            for (const unsigned char value : {' ', '\t', '\n', '\r', '\f', '\v'}) {
                result.add(value);
            }
            break;
        default:
            break;
    }
    return result;
}

struct ClassElement final {
    ByteSet bytes;
    std::optional<unsigned char> single;
    bool negated_shorthand = false;
};

[[nodiscard]] std::optional<ClassElement> parse_class_element(
    std::string_view pattern,
    std::size_t& cursor) {
    if (cursor >= pattern.size() || pattern[cursor] == ']') {
        return std::nullopt;
    }
    unsigned char value = static_cast<unsigned char>(pattern[cursor++]);
    if (value != static_cast<unsigned char>('\\')) {
        ClassElement result;
        result.bytes.add(value);
        result.single = value;
        return result;
    }
    if (cursor >= pattern.size()) {
        return std::nullopt;
    }
    value = static_cast<unsigned char>(pattern[cursor++]);
    if (value == 'd' || value == 'D' || value == 'w' || value == 'W' ||
        value == 's' || value == 'S') {
        ClassElement result;
        result.bytes = shorthand_bytes(value);
        result.negated_shorthand = value == 'D' || value == 'W' || value == 'S';
        return result;
    }
    ClassElement result;
    result.bytes.add(value);
    result.single = value;
    return result;
}

[[nodiscard]] std::optional<Matcher> parse_byte_class(
    std::string_view pattern,
    std::size_t& cursor) {
    Matcher matcher;
    matcher.kind = MatcherKind::byte_class;
    if (cursor < pattern.size() && pattern[cursor] == '^') {
        matcher.negated = true;
        ++cursor;
    }
    bool has_element = false;
    while (cursor < pattern.size() && pattern[cursor] != ']') {
        auto left = parse_class_element(pattern, cursor);
        if (!left.has_value() || left->negated_shorthand) {
            return std::nullopt;
        }
        has_element = true;
        if (cursor < pattern.size() && pattern[cursor] == '-' &&
            cursor + 1U < pattern.size() && pattern[cursor + 1U] != ']') {
            ++cursor;
            auto right = parse_class_element(pattern, cursor);
            if (!right.has_value() || !left->single.has_value() ||
                !right->single.has_value() || right->negated_shorthand ||
                *left->single > *right->single) {
                return std::nullopt;
            }
            matcher.bytes.add_range(*left->single, *right->single);
        } else {
            matcher.bytes.merge(left->bytes);
        }
    }
    if (!has_element || cursor >= pattern.size() || pattern[cursor] != ']') {
        return std::nullopt;
    }
    ++cursor;
    return matcher;
}

[[nodiscard]] std::optional<Matcher> parse_atom(
    std::string_view pattern,
    std::size_t& cursor) {
    if (cursor >= pattern.size()) {
        return std::nullopt;
    }
    const unsigned char value = static_cast<unsigned char>(pattern[cursor++]);
    if (value == static_cast<unsigned char>('.')) {
        Matcher matcher;
        matcher.kind = MatcherKind::any_byte;
        return matcher;
    }
    if (value == static_cast<unsigned char>('[')) {
        return parse_byte_class(pattern, cursor);
    }
    if (value == static_cast<unsigned char>('\\')) {
        if (cursor >= pattern.size()) {
            return std::nullopt;
        }
        const unsigned char escaped = static_cast<unsigned char>(pattern[cursor++]);
        if (escaped == 'd' || escaped == 'D' || escaped == 'w' || escaped == 'W' ||
            escaped == 's' || escaped == 'S') {
            Matcher matcher;
            matcher.kind = MatcherKind::byte_class;
            matcher.bytes = shorthand_bytes(escaped);
            matcher.negated = escaped == 'D' || escaped == 'W' || escaped == 'S';
            return matcher;
        }
        Matcher matcher;
        matcher.literal = escaped;
        return matcher;
    }
    if (value == '^' || value == '$' || value == '*' || value == '+' || value == '?' ||
        value == ']' || value == '(' || value == ')' || value == '{' || value == '}' ||
        value == '|') {
        return std::nullopt;
    }
    Matcher matcher;
    matcher.literal = value;
    return matcher;
}

[[nodiscard]] CompiledPattern compile_pattern(
    std::string_view pattern,
    const SafeRegexLimits& limits) {
    CompiledPattern compiled;
    if (!limits_valid(limits)) {
        compiled.error = SafeRegexError::invalid_limits;
        return compiled;
    }
    if (pattern.size() > limits.max_pattern_bytes) {
        compiled.error = SafeRegexError::pattern_too_large;
        return compiled;
    }

    std::vector<Atom> atoms;
    std::size_t cursor = 0U;
    if (!pattern.empty() && pattern.front() == '^') {
        compiled.anchor_start = true;
        ++cursor;
    }
    while (cursor < pattern.size()) {
        if (pattern[cursor] == '$' && cursor + 1U == pattern.size()) {
            compiled.anchor_end = true;
            ++cursor;
            break;
        }
        auto matcher = parse_atom(pattern, cursor);
        if (!matcher.has_value()) {
            compiled.error = SafeRegexError::invalid_pattern;
            return compiled;
        }
        Atom atom;
        atom.matcher = std::move(*matcher);
        if (cursor < pattern.size()) {
            switch (pattern[cursor]) {
                case '?': atom.quantifier = Quantifier::zero_or_one; ++cursor; break;
                case '*': atom.quantifier = Quantifier::zero_or_more; ++cursor; break;
                case '+': atom.quantifier = Quantifier::one_or_more; ++cursor; break;
                default: break;
            }
        }
        atoms.push_back(std::move(atom));
        if (atoms.size() * 2U + 1U > limits.max_state_count) {
            compiled.error = SafeRegexError::state_limit_exceeded;
            return compiled;
        }
    }

    compiled.states.push_back(State{});
    compiled.accept_state = 0U;
    std::size_t next = compiled.accept_state;
    for (auto iterator = atoms.rbegin(); iterator != atoms.rend(); ++iterator) {
        if (iterator->quantifier == Quantifier::exactly_one) {
            compiled.states.push_back(State{StateKind::match, iterator->matcher, next, no_state});
            next = compiled.states.size() - 1U;
        } else if (iterator->quantifier == Quantifier::zero_or_one) {
            compiled.states.push_back(State{StateKind::match, iterator->matcher, next, no_state});
            const std::size_t match_state = compiled.states.size() - 1U;
            compiled.states.push_back(State{StateKind::split, {}, match_state, next});
            next = compiled.states.size() - 1U;
        } else if (iterator->quantifier == Quantifier::zero_or_more) {
            compiled.states.push_back(State{StateKind::split, {}, no_state, next});
            const std::size_t split_state = compiled.states.size() - 1U;
            compiled.states.push_back(State{StateKind::match, iterator->matcher, split_state, no_state});
            const std::size_t match_state = compiled.states.size() - 1U;
            compiled.states[split_state].out1 = match_state;
            next = split_state;
        } else {
            compiled.states.push_back(State{StateKind::split, {}, no_state, next});
            const std::size_t split_state = compiled.states.size() - 1U;
            compiled.states.push_back(State{StateKind::match, iterator->matcher, split_state, no_state});
            const std::size_t match_state = compiled.states.size() - 1U;
            compiled.states[split_state].out1 = match_state;
            next = match_state;
        }
    }
    compiled.start_state = next;
    return compiled;
}

void add_epsilon_closure(
    const CompiledPattern& compiled,
    std::vector<std::size_t>& active,
    std::size_t state_index,
    std::size_t match_start) {
    std::vector<std::size_t> pending{state_index};
    while (!pending.empty()) {
        const std::size_t index = pending.back();
        pending.pop_back();
        if (active[index] <= match_start) {
            continue;
        }
        active[index] = match_start;
        const State& state = compiled.states[index];
        if (state.kind == StateKind::split) {
            pending.push_back(state.out1);
            pending.push_back(state.out2);
        }
    }
}

}  // namespace

SafeRegexError validate_safe_regex(
    std::string_view pattern,
    const SafeRegexLimits& limits) {
    return compile_pattern(pattern, limits).error;
}

SafeRegexMatch search_safe_regex(
    std::string_view input,
    std::string_view pattern,
    std::size_t start_byte_offset,
    bool ignore_ascii_case,
    const SafeRegexLimits& limits) {
    SafeRegexMatch result;
    if (!limits_valid(limits)) {
        result.error = SafeRegexError::invalid_limits;
        return result;
    }
    if (input.size() > limits.max_input_bytes) {
        result.error = SafeRegexError::input_too_large;
        return result;
    }
    if (start_byte_offset > input.size()) {
        result.error = SafeRegexError::invalid_start;
        return result;
    }
    const CompiledPattern compiled = compile_pattern(pattern, limits);
    if (compiled.error != SafeRegexError::none) {
        result.error = compiled.error;
        return result;
    }

    std::vector<std::size_t> active(compiled.states.size(), no_state);
    std::vector<std::size_t> next(compiled.states.size(), no_state);
    std::optional<std::size_t> best_start;
    std::size_t best_end = 0U;
    for (std::size_t position = start_byte_offset; position <= input.size(); ++position) {
        if (!compiled.anchor_start || (position == 0U && start_byte_offset == 0U)) {
            add_epsilon_closure(compiled, active, compiled.start_state, position);
        }
        const std::size_t accepted_start = active[compiled.accept_state];
        if (accepted_start != no_state && (!compiled.anchor_end || position == input.size())) {
            if (!best_start.has_value() || accepted_start < *best_start ||
                (accepted_start == *best_start && position > best_end)) {
                best_start = accepted_start;
                best_end = position;
            }
        }
        if (position == input.size()) {
            break;
        }
        std::fill(next.begin(), next.end(), no_state);
        const unsigned char value = static_cast<unsigned char>(input[position]);
        for (std::size_t index = 0U; index < compiled.states.size(); ++index) {
            if (active[index] == no_state || compiled.states[index].kind != StateKind::match ||
                !matches(compiled.states[index].matcher, value, ignore_ascii_case)) {
                continue;
            }
            add_epsilon_closure(compiled, next, compiled.states[index].out1, active[index]);
        }
        active.swap(next);
    }
    if (best_start.has_value()) {
        result.matched = true;
        result.byte_offset = *best_start;
        result.byte_length = best_end - *best_start;
    }
    return result;
}

}  // namespace copperfin::platform

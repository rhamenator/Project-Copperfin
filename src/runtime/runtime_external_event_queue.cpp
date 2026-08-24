// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_external_event_queue.h"

#include <cctype>
#include <utility>

namespace copperfin::runtime::detail {

bool ExternalEventTokenQueue::try_push(std::string token) {
    if (!is_valid_token(token)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (tokens_.size() >= kMaximumTokenCount) {
        return false;
    }
    tokens_.push_back(std::move(token));
    return true;
}

std::vector<std::string> ExternalEventTokenQueue::drain() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> drained;
    drained.reserve(tokens_.size());
    while (!tokens_.empty()) {
        drained.push_back(std::move(tokens_.front()));
        tokens_.pop_front();
    }
    return drained;
}

void ExternalEventTokenQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_.clear();
}

std::size_t ExternalEventTokenQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tokens_.size();
}

bool ExternalEventTokenQueue::is_valid_token(const std::string& token) {
    if (token.empty() || token.size() > kMaximumTokenLength) {
        return false;
    }
    for (const unsigned char character : token) {
        if (!(std::isalnum(character) != 0 || character == '_' || character == '.' ||
              character == ':' || character == '-')) {
            return false;
        }
    }
    return true;
}

}  // namespace copperfin::runtime::detail

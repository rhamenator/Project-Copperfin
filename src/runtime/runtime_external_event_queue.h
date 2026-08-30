// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace copperfin::runtime::detail {

// A host callback may place only a normalized event name in this queue.  The
// runtime later consumes the names on its own execution thread; no callback is
// granted access to mutable session state.
class ExternalEventTokenQueue {
public:
    static constexpr std::size_t kMaximumTokenCount = 64U;
    static constexpr std::size_t kMaximumTokenLength = 128U;

    [[nodiscard]] bool try_push(std::string token);
    [[nodiscard]] std::vector<std::string> drain();
    void clear();
    [[nodiscard]] std::size_t size() const;

private:
    static bool is_valid_token(const std::string& token);

    mutable std::mutex mutex_;
    std::deque<std::string> tokens_;
};

}  // namespace copperfin::runtime::detail

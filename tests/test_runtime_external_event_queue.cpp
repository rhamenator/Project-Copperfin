// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "../src/runtime/runtime_external_event_queue.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void expect(const bool condition, const char* const message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    using copperfin::runtime::detail::ExternalEventTokenQueue;

    ExternalEventTokenQueue queue;
    expect(!queue.try_push(""), "empty tokens must fail closed");
    expect(!queue.try_push("event name"), "un-normalized tokens must fail closed");
    expect(!queue.try_push("event\nname"), "control characters must fail closed");
    expect(queue.size() == 0U, "rejected tokens must not mutate the queue");

    expect(queue.try_push("Source.Interface.EventA"), "first normalized token should enqueue");
    expect(queue.try_push("Source.Interface.EventB"), "second normalized token should enqueue");
    const auto drained = queue.drain();
    expect(drained.size() == 2U, "drain should preserve accepted token count");
    expect(drained[0] == "Source.Interface.EventA" &&
               drained[1] == "Source.Interface.EventB",
           "drain must preserve FIFO order");
    expect(queue.size() == 0U, "drain must empty the queue");

    for (std::size_t index = 0U; index < ExternalEventTokenQueue::kMaximumTokenCount; ++index) {
        expect(queue.try_push("Event" + std::to_string(index)), "capacity boundary token should enqueue");
    }
    expect(!queue.try_push("Overflow"), "over-capacity tokens must fail closed");
    expect(queue.size() == ExternalEventTokenQueue::kMaximumTokenCount,
           "over-capacity rejection must not mutate the queue");
    queue.clear();
    expect(queue.size() == 0U, "clear should discard queued tokens");
    return 0;
}

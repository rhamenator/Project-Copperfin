// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "../src/runtime/runtime_external_event_queue.h"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

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

    // The queue is an internal boundary between one runtime consumer and
    // host-owned callback producers.  Keep the test below deliberately below
    // the bounded capacity so every producer result is deterministic while a
    // consumer drains concurrently.
    constexpr std::size_t producer_count = 4U;
    constexpr std::size_t tokens_per_producer = 4U;
    std::atomic<bool> start_producing{false};
    std::atomic<std::size_t> finished_producer_count{0U};
    std::atomic<std::size_t> accepted_token_count{0U};
    std::vector<std::thread> producers;
    producers.reserve(producer_count);
    for (std::size_t producer = 0U; producer < producer_count; ++producer) {
        producers.emplace_back([&queue, &start_producing, &accepted_token_count, &finished_producer_count, producer]() {
            while (!start_producing.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            for (std::size_t token = 0U; token < tokens_per_producer; ++token) {
                if (queue.try_push("Producer" + std::to_string(producer) + "Token" + std::to_string(token))) {
                    accepted_token_count.fetch_add(1U, std::memory_order_relaxed);
                }
            }
            finished_producer_count.fetch_add(1U, std::memory_order_release);
        });
    }

    start_producing.store(true, std::memory_order_release);
    std::vector<std::string> concurrently_drained;
    while (finished_producer_count.load(std::memory_order_acquire) != producer_count || queue.size() != 0U) {
        const auto batch = queue.drain();
        concurrently_drained.insert(concurrently_drained.end(), batch.begin(), batch.end());
        std::this_thread::yield();
    }
    for (std::thread& producer : producers) {
        if (producer.joinable()) {
            producer.join();
        }
    }

    expect(accepted_token_count.load(std::memory_order_relaxed) == producer_count * tokens_per_producer,
           "concurrent producer tokens below capacity must all enqueue");
    expect(concurrently_drained.size() == accepted_token_count.load(std::memory_order_relaxed),
           "concurrent drain must retain every accepted token exactly once");
    std::sort(concurrently_drained.begin(), concurrently_drained.end());
    expect(std::adjacent_find(concurrently_drained.begin(), concurrently_drained.end()) == concurrently_drained.end(),
           "concurrent drain must not duplicate accepted tokens");
    std::vector<std::string> expected_tokens;
    expected_tokens.reserve(producer_count * tokens_per_producer);
    for (std::size_t producer = 0U; producer < producer_count; ++producer) {
        for (std::size_t token = 0U; token < tokens_per_producer; ++token) {
            expected_tokens.push_back(
                "Producer" + std::to_string(producer) + "Token" + std::to_string(token));
        }
    }
    expect(concurrently_drained == expected_tokens,
           "concurrent drain must retain the exact accepted token set");
    expect(queue.size() == 0U, "concurrent consumer must leave the queue empty");
    return 0;
}

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/platform/path.h"
#include "copperfin/security/audit_stream.h"
#include "test_process_capture_support.h"

#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <string_view>

namespace {

constexpr int kWorkerCount = 4;
constexpr int kEventsPerWorker = 8;

int run_worker(const std::filesystem::path& log_path, int worker) {
    for (int event = 0; event < kEventsPerWorker; ++event) {
        const auto result = copperfin::security::append_immutable_audit_event(
            copperfin::platform::path_to_utf8_string(log_path),
            "build.concurrent",
            "worker=" + std::to_string(worker) + ",event=" + std::to_string(event));
        if (!result.ok) {
            std::cerr << result.error << "\n";
            return 1;
        }
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 4 && std::string_view(argv[1]) == "--worker") {
        return run_worker(
            copperfin::platform::path_from_utf8_string(argv[2]),
            std::atoi(argv[3]));
    }

    const std::filesystem::path temp_root = std::filesystem::temp_directory_path() /
        ("copperfin_security_audit_concurrency-" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const std::filesystem::path log_path = temp_root / "security_audit.log";
    std::error_code error;
    std::filesystem::remove_all(temp_root, error);
    std::filesystem::create_directories(temp_root, error);
    if (error) {
        std::cerr << "failed to create audit concurrency fixture: " << error.message() << "\n";
        return 1;
    }

    const std::filesystem::path executable = std::filesystem::absolute(
        copperfin::platform::path_from_utf8_string(argv[0]));
    std::vector<copperfin::test_support::CapturedProcessResult> results(kWorkerCount);
    std::vector<std::thread> workers;
    workers.reserve(kWorkerCount);
    for (int worker = 0; worker < kWorkerCount; ++worker) {
        workers.emplace_back([&, worker]() {
            results[worker] = copperfin::test_support::run_process_capture(
                executable,
                {"--worker", copperfin::platform::path_to_utf8_string(log_path), std::to_string(worker)},
                temp_root);
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }

    bool passed = true;
    for (int worker = 0; worker < kWorkerCount; ++worker) {
        if (!results[worker].started || results[worker].exit_code != 0) {
            std::cerr << "worker " << worker << " failed: " << results[worker].stderr_text << "\n";
            passed = false;
        }
    }
    const auto chain = copperfin::security::verify_immutable_audit_chain(
        copperfin::platform::path_to_utf8_string(log_path));
    passed = passed && chain.ok &&
        chain.entries == static_cast<std::size_t>(kWorkerCount * kEventsPerWorker);
    if (!passed) {
        std::cerr << "concurrent generic audit appends did not preserve one valid chain\n";
    }

    std::filesystem::remove_all(temp_root, error);
    return passed ? 0 : 1;
}

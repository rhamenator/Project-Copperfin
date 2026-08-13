// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/file_stream.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>

namespace {

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

class TemporaryDirectory final {
public:
    TemporaryDirectory() {
        const auto unique_value = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
            ("copperfin-platform-file-stream-" + std::to_string(unique_value));
        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

std::string read_bytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void test_unicode_path_open_and_resize() {
    const TemporaryDirectory temporary;
    const auto path = temporary.path() / std::filesystem::path(u8"résumé-stream.bin");
    std::FILE* stream = copperfin::platform::open_file_stream(path, "wb+");
    expect(stream != nullptr, "#35: platform stream should open a Unicode native path");
    if (stream == nullptr) {
        return;
    }

    constexpr char payload[] = "abcdef";
    expect(std::fwrite(payload, 1U, 6U, stream) == 6U,
           "#35: platform stream should accept binary content");
    expect(std::fflush(stream) == 0,
           "#35: test content should flush before resize");
    expect(copperfin::platform::resize_file_stream(stream, 2U) == 0,
           "#35: platform stream resize should shrink an open file");
    expect(std::fclose(stream) == 0,
           "#35: resized platform stream should close cleanly");
    expect(read_bytes(path) == "ab",
           "#35: shrink should preserve the requested prefix exactly");

    stream = copperfin::platform::open_file_stream(path, "rb+");
    expect(stream != nullptr,
           "#35: platform stream should reopen an existing Unicode path");
    if (stream == nullptr) {
        return;
    }
    expect(copperfin::platform::resize_file_stream(stream, 5U) == 0,
           "#35: platform stream resize should extend an open file");
    expect(std::fclose(stream) == 0,
           "#35: extended platform stream should close cleanly");
    const std::string extended = read_bytes(path);
    expect(extended.size() == 5U && extended.substr(0U, 2U) == "ab" &&
               extended[2] == '\0' && extended[3] == '\0' && extended[4] == '\0',
           "#35: extension should preserve bytes and zero-fill the new range");
}

void test_failures_preserve_stream_contract() {
    const TemporaryDirectory temporary;
    expect(copperfin::platform::open_file_stream(
               temporary.path() / "missing" / "stream.bin", "wb") == nullptr,
           "#35: platform stream open should not create parent directories");

    errno = 0;
    expect(copperfin::platform::resize_file_stream(nullptr, 1U) != 0 && errno == EBADF,
           "#35: null stream resize should fail with a bad-descriptor error");

    const auto path = temporary.path() / "bounded.bin";
    std::FILE* stream = copperfin::platform::open_file_stream(path, "wb+");
    expect(stream != nullptr, "#35: boundary-size test stream should open");
    if (stream == nullptr) {
        return;
    }
    errno = 0;
    expect(copperfin::platform::resize_file_stream(
               stream,
               (std::numeric_limits<std::uint64_t>::max)()) != 0 &&
               errno == EFBIG,
           "#35: unrepresentable stream size should fail before native narrowing");
    expect(std::fclose(stream) == 0,
           "#35: rejected boundary-size stream should remain closeable");
}

}  // namespace

int main() {
    test_unicode_path_open_and_resize();
    test_failures_preserve_stream_contract();
    if (failures == 0) {
        std::cout << "Platform file-stream tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}

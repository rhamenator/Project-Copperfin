// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#ifndef COPPERFIN_TEST_PROCESS_CAPTURE_SUPPORT_H
#define COPPERFIN_TEST_PROCESS_CAPTURE_SUPPORT_H

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::test_support {

struct CapturedProcessResult {
    bool started = false;
    int exit_code = -1;
    int launch_error = 0;
    std::string stdout_text;
    std::string stderr_text;
};

CapturedProcessResult run_process_capture(
    const std::filesystem::path& executable_path,
    const std::vector<std::string>& utf8_arguments,
    const std::filesystem::path& working_directory);

std::string normalize_captured_line_endings(std::string_view text);
CapturedProcessResult normalize_captured_process_line_endings(CapturedProcessResult result);

}  // namespace copperfin::test_support

#endif

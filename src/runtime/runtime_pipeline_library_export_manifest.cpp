// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

#include <locale>

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

std::string build_native_wrapper_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << "// Generated Copperfin native wrapper scaffold\n";
    stream << "// This is an honest bridge scaffold, not a finished FoxPro/VFP-compatible runtime wrapper.\n";
    append_native_wrapper_compilation_preamble(stream);
    append_native_wrapper_host_authentication_source(stream);
    append_native_wrapper_bridge_model_source(stream);
    append_native_wrapper_process_launch_source(stream);
    append_native_wrapper_response_handling_source(stream);
    append_native_wrapper_library_entrypoint_source(stream, plan);
    return stream.str();
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime

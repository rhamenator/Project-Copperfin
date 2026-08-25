// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

void append_native_wrapper_compilation_preamble(std::ostringstream& stream)
{
    stream << "#include <algorithm>\n";
    stream << "#include <array>\n";
    stream << "#include <atomic>\n";
    stream << "#include <cerrno>\n";
    stream << "#include <cstdint>\n";
    stream << "#include <cstring>\n";
    stream << "#include <cwchar>\n";
    stream << "#include <filesystem>\n";
    stream << "#include <fstream>\n";
    stream << "#include <iterator>\n";
    stream << "#include <mutex>\n";
    stream << "#include <sstream>\n";
    stream << "#include <string>\n";
    stream << "#include <utility>\n";
    stream << "#include <vector>\n";
    stream << "#if defined(_WIN32)\n";
    stream << "#include <windows.h>\n";
    stream << "#define COPPERFIN_EXPORT extern \"C\" __declspec(dllexport)\n";
    stream << "#else\n";
    stream << "#include <dlfcn.h>\n";
    stream << "#include <fcntl.h>\n";
    stream << "#include <sys/stat.h>\n";
    stream << "#include <sys/types.h>\n";
    stream << "#include <sys/wait.h>\n";
    stream << "#include <unistd.h>\n";
    stream << "extern char** environ;\n";
    stream << "#define COPPERFIN_EXPORT extern \"C\" __attribute__((visibility(\"default\")))\n";
    stream << "#endif\n\n";
}

} // namespace runtime_pipeline_detail

} // namespace copperfin::runtime

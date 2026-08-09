// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_prg_json_control_plane_facade() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_json_control_plane";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "json_facade.prg";
    write_text(
        main_path,
        "cPayload = '{\"status\":\"ready\\nnow\",\"items\":[10,{\"a/b\":true}],\"exact\":9007199254740993}'\n"
        "lValid = CFJSONVALID(cPayload)\n"
        "cRootType = CFJSONTYPE(cPayload)\n"
        "cStatusType = CFJSONTYPE(cPayload, '/status')\n"
        "cStatus = CFJSONGET(cPayload, '/status')\n"
        "cNested = CFJSONGET(cPayload, '/items/1/a~1b')\n"
        "cExact = CFJSONGET(cPayload, '/exact')\n"
        "cMissingType = CFJSONTYPE(cPayload, '/missing')\n"
        "cMissing = CFJSONGET(cPayload, '/missing', 'fallback')\n"
        "cMalformedType = CFJSONTYPE('[1,]')\n"
        "lMalformed = CFJSONVALID('[1,]')\n"
        "cMalformed = CFJSONGET('[1,]', '', 'invalid-fallback')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("PRG JSON facade should complete: ") + state.message);

    const auto formatted = [&](const std::string& name) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        return found == state.globals.end()
            ? std::string{}
            : copperfin::runtime::format_value(found->second);
    };
    expect(formatted("lvalid") == "true", "CFJSONVALID should accept valid JSON");
    expect(formatted("croottype") == "object", "CFJSONTYPE should classify the root");
    expect(formatted("cstatustype") == "string", "CFJSONTYPE should classify a selected string");
    expect(formatted("cstatus") == "ready\nnow", "CFJSONGET should decode selected strings");
    expect(formatted("cnested") == "true", "CFJSONGET should preserve non-string JSON scalars");
    expect(formatted("cexact") == "9007199254740993",
           "CFJSONGET should preserve exact number spelling");
    expect(formatted("cmissingtype") == "missing",
           "CFJSONTYPE should distinguish a missing value from invalid JSON");
    expect(formatted("cmissing") == "fallback",
           "CFJSONGET should return the caller fallback for a missing value");
    expect(formatted("cmalformedtype") == "invalid",
           "CFJSONTYPE should classify malformed JSON as invalid");
    expect(formatted("lmalformed") == "false",
           "CFJSONVALID should reject malformed JSON");
    expect(formatted("cmalformed") == "invalid-fallback",
           "CFJSONGET should return the caller fallback for malformed JSON");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_prg_json_control_plane_facade();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}

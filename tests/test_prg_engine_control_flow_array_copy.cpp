// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_whole_array_assignment_copies_scoped_storage() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_whole_array_assignment";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "whole_array_assignment.prg";
    write_text(
        main_path,
        "PUBLIC private_copy_one, private_copy_two, private_copy_size\n"
        "DIMENSION aSource[3], aCopy[1]\n"
        "aSource[1] = 'global-one'\n"
        "aSource[2] = 'global-two'\n"
        "aSource[3] = 'global-three'\n"
        "aCopy = aSource\n"
        "aSource[1] = 'changed-after-copy'\n"
        "global_copy_one = aCopy[1]\n"
        "global_copy_three = aCopy[3]\n"
        "global_copy_size = ALEN(aCopy)\n"
        "local_copy_result = copy_local_array()\n"
        "DO copy_private_array\n"
        "RETURN\n"
        "FUNCTION copy_local_array\n"
        "LOCAL ARRAY aLocalSource[2], aLocalCopy[1]\n"
        "aLocalSource[1] = 'local-one'\n"
        "aLocalSource[2] = 'local-two'\n"
        "aLocalCopy = aLocalSource\n"
        "aLocalSource[1] = 'local-changed'\n"
        "RETURN aLocalCopy[1] + '|' + aLocalCopy[2] + '|' + TRANSFORM(ALEN(aLocalCopy))\n"
        "ENDFUNC\n"
        "PROCEDURE copy_private_array\n"
        "PRIVATE ARRAY aPrivateSource[2], aPrivateCopy[1]\n"
        "aPrivateSource[1] = 'private-one'\n"
        "aPrivateSource[2] = 'private-two'\n"
        "aPrivateCopy = aPrivateSource\n"
        "aPrivateSource[1] = 'private-changed'\n"
        "private_copy_one = aPrivateCopy[1]\n"
        "private_copy_two = aPrivateCopy[2]\n"
        "private_copy_size = ALEN(aPrivateCopy)\n"
        "RETURN\n");

    const auto state = copperfin::runtime::PrgRuntimeSession::create(
                           make_runtime_session_options(main_path.string(), temp_root.string(), false))
                           .run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "whole-array assignment script should complete: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), name + " should be captured");
        if (value != state.globals.end())
        {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   name + " should preserve the independent array copy");
        }
    };

    check("global_copy_one", "global-one");
    check("global_copy_three", "global-three");
    check("global_copy_size", "3");
    check("local_copy_result", "local-one|local-two|2");
    check("private_copy_one", "private-one");
    check("private_copy_two", "private-two");
    check("private_copy_size", "2");

    fs::remove_all(temp_root, ignored);
}

} // namespace cf_test_prg_engine_control_flow

// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#define _getpid getpid
#endif
#include <sstream>
#include <system_error>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace {

using namespace copperfin::test_support;

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::set_env_value;

#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
std::filesystem::path declared_dll_fixture_source_path() {
    std::wstring executable_path(32768U, L'\0');
    const DWORD executable_length = GetModuleFileNameW(
        nullptr,
        executable_path.data(),
        static_cast<DWORD>(executable_path.size()));
    if (executable_length == 0U || executable_length >= executable_path.size()) {
        return {};
    }
    executable_path.resize(executable_length);
    return std::filesystem::path(executable_path).parent_path() /
           std::filesystem::path(COPPERFIN_DECLARED_DLL_FIXTURE_NAME);
}
#endif

void test_set_order_and_seek_for_local_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SEEK 'BRAVO'\n"
        "lFound1 = FOUND()\n"
        "nRec1 = RECNO()\n"
        "SEEK 'ZZZZ'\n"
        "lFound2 = FOUND()\n"
        "lEof2 = EOF()\n"
        "nRec2 = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "seek script should complete");

    const auto found1 = state.globals.find("lfound1");
    const auto rec1 = state.globals.find("nrec1");
    const auto found2 = state.globals.find("lfound2");
    const auto eof2 = state.globals.find("leof2");
    const auto rec2 = state.globals.find("nrec2");

    expect(found1 != state.globals.end(), "FOUND() after a successful SEEK should be captured");
    expect(rec1 != state.globals.end(), "RECNO() after a successful SEEK should be captured");
    expect(found2 != state.globals.end(), "FOUND() after a failed SEEK should be captured");
    expect(eof2 != state.globals.end(), "EOF() after a failed SEEK should be captured");
    expect(rec2 != state.globals.end(), "RECNO() after a failed SEEK should be captured");

    if (found1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(found1->second) == "true", "SEEK should set FOUND() when it locates a matching record");
    }
    if (rec1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec1->second) == "2", "SEEK should move the record pointer to the matched row");
    }
    if (found2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(found2->second) == "false", "failed SEEK should clear FOUND()");
    }
    if (eof2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof2->second) == "true", "failed SEEK should move the cursor to EOF");
    }
    if (rec2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec2->second) == "4", "failed SEEK should place RECNO() at record_count + 1");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("NAME [norm=upper, coll=case-folded]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded]: BRAVO -> found") != std::string::npos;
        }),
        "SET ORDER and SEEK should emit runtime.order and runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_seek_search_key_uses_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "NAME");

    const fs::path deep_path = temp_root / "seek_deep.prg";
    write_text(
        deep_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SEEK seek_key(2048)\n"
        "RETURN\n"
        "FUNCTION seek_key\n"
        "LPARAMETERS nDepth\n"
        "IF nDepth <= 0\n"
        "RETURN 'BRAVO'\n"
        "ENDIF\n"
        "RETURN seek_key(nDepth - 1)\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession deep_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(deep_path.string(), temp_root.string()));
    const auto deep_state = deep_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!deep_state.completed, "deep SEEK key recursion should stop at the configured call-depth guard");
    expect(
        deep_state.message.find("maximum call depth") != std::string::npos,
        "deep SEEK key recursion should report Copperfin's call-depth diagnostic instead of overflowing the host stack");

    const fs::path side_effect_path = temp_root / "seek_side_effect.prg";
    write_text(
        side_effect_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "nCalls = 0\n"
        "SEEK seek_key()\n"
        "nFound = FOUND()\n"
        "RETURN\n"
        "FUNCTION seek_key\n"
        "nCalls = nCalls + 1\n"
        "RETURN 'BRAVO'\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession side_effect_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(side_effect_path.string(), temp_root.string()));
    const auto side_effect_state = side_effect_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(side_effect_state.completed, "SEEK key UDF script should complete");

    const auto calls = side_effect_state.globals.find("ncalls");
    const auto found = side_effect_state.globals.find("nfound");
    expect(calls != side_effect_state.globals.end(), "SEEK key UDF call count should be captured");
    expect(found != side_effect_state.globals.end(), "SEEK should still expose FOUND() after a resumed key evaluation");
    if (calls != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(calls->second) == "1", "SEEK key UDF should run exactly once across suspension and resume");
    }
    if (found != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(found->second) == "true", "resumed SEEK should find the UDF-provided key");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_collate_guides_plain_string_seek_comparisons() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_collate";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "NAME");

    const fs::path main_path = temp_root / "seek_collate.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "lMachineMiss = SEEK('bravo')\n"
        "nMachineRec = RECNO()\n"
        "SET COLLATE TO GENERAL\n"
        "GO TOP\n"
        "lGeneralHit = SEEK('bravo')\n"
        "nGeneralRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "collation-guided seek script should complete");

    const auto machine_miss = state.globals.find("lmachinemiss");
    const auto machine_rec = state.globals.find("nmachinerec");
    const auto general_hit = state.globals.find("lgeneralhit");
    const auto general_rec = state.globals.find("ngeneralrec");

    expect(machine_miss != state.globals.end(), "default-collate SEEK result should be captured");
    expect(machine_rec != state.globals.end(), "default-collate SEEK RECNO() should be captured");
    expect(general_hit != state.globals.end(), "SET COLLATE-guided SEEK result should be captured");
    expect(general_rec != state.globals.end(), "SET COLLATE-guided SEEK RECNO() should be captured");

    if (machine_miss != state.globals.end()) {
        expect(copperfin::runtime::format_value(machine_miss->second) == "false",
            "default MACHINE collation should keep plain NAME seeks case-sensitive");
    }
    if (machine_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(machine_rec->second) == "4",
            "default MACHINE collation failed seek should still land at EOF");
    }
    if (general_hit != state.globals.end()) {
        expect(copperfin::runtime::format_value(general_hit->second) == "true",
            "SET COLLATE TO GENERAL should case-fold plain string seeks");
    }
    if (general_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(general_rec->second) == "2",
            "SET COLLATE TO GENERAL should land on the case-folded plain-string match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_uses_grounded_order_normalization_hints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_normalization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_normalization.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "lSeekCmd = SEEK('bravo')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('charlie', 'People', 'NAME')\n"
        "nSeekFnRec = RECNO()\n"
        "SET ORDER TO TAG NAME DESCENDING\n"
        "lSeekDesc = SEEK('alpha')\n"
        "nSeekDescRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "normalization-aware seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");
    const auto seek_desc = state.globals.find("lseekdesc");
    const auto seek_desc_rec = state.globals.find("nseekdescrec");

    expect(seek_cmd != state.globals.end(), "command SEEK result should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() result should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() RECNO() should be captured");
    expect(seek_desc != state.globals.end(), "descending normalized SEEK result should be captured");
    expect(seek_desc_rec != state.globals.end(), "descending normalized SEEK RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should honor grounded upper normalization hints");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "2", "command SEEK should land on the case-folded match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should honor grounded upper normalization hints");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "3", "SEEK() should move to the normalized match");
    }
    if (seek_desc != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_desc->second) == "true", "descending SEEK should also honor grounded normalization hints");
    }
    if (seek_desc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_desc_rec->second) == "1", "descending SEEK should land on the normalized exact match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_composite_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_composite";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "LAST", .type = 'C', .length = 10U},
        {.name = "FIRST", .type = 'C', .length = 10U},
    };
    const std::vector<std::vector<std::string>> records{
        {"DOE", "JOHN"},
        {"SMITH", "JANE"},
        {"TAYLOR", "ALEX"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "composite seek DBF fixture should be created");
    write_synthetic_cdx(cdx_path, "FULLNAME", "UPPER(LAST+FIRST)");

    const fs::path main_path = temp_root / "seek_composite.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG FULLNAME\n"
        "lSeekCmd = SEEK('smithjane')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('doejohn', 'People', 'FULLNAME')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "composite-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a composite tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command composite SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a composite tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() composite RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match concatenated composite-tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "2", "command SEEK should land on the concatenated composite-tag match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match concatenated composite-tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on the requested composite-tag match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_left_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_left";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME3", "UPPER(LEFT(NAME, 3))");

    const fs::path main_path = temp_root / "seek_left.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME3\n"
        "lSeekCmd = SEEK('cha')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('bra', 'People', 'NAME3')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LEFT()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a LEFT() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command LEFT() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a LEFT() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() LEFT() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match LEFT()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on the LEFT()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match LEFT()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested LEFT()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_right_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_right";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAMEEND", "UPPER(RIGHT(NAME, 3))");

    const fs::path main_path = temp_root / "seek_right.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAMEEND\n"
        "lSeekCmd = SEEK('lie')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('avo', 'People', 'NAMEEND')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RIGHT()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a RIGHT() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command RIGHT() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a RIGHT() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() RIGHT() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match RIGHT()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on the RIGHT()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match RIGHT()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested RIGHT()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_substr_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_substr";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAMEMID", "UPPER(SUBSTR(NAME, 2, 3))");

    const fs::path main_path = temp_root / "seek_substr.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAMEMID\n"
        "lSeekCmd = SEEK('har')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('rav', 'People', 'NAMEMID')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SUBSTR()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a SUBSTR() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command SUBSTR() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a SUBSTR() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() SUBSTR() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match SUBSTR()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on the SUBSTR()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match SUBSTR()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested SUBSTR()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_padl_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_padl";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "LPAD", "UPPER(PADL(NAME, 8, '0'))");

    const fs::path main_path = temp_root / "seek_padl.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG LPAD\n"
        "lSeekCmd = SEEK('000ALPHA')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('000BRAVO', 'People', 'LPAD')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PADL()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a PADL() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command PADL() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a PADL() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() PADL() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match PADL()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "1", "command SEEK should land on the PADL()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match PADL()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested PADL()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_padr_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_padr";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "RPAD", "UPPER(PADR(NAME, 8, '0'))");

    const fs::path main_path = temp_root / "seek_padr.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG RPAD\n"
        "lSeekCmd = SEEK('ALPHA000')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('BRAVO000', 'People', 'RPAD')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PADR()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a PADR() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command PADR() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a PADR() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() PADR() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match PADR()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "1", "command SEEK should land on the PADR()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match PADR()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested PADR()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_padl_default_padding_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_padl_default";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "LPADSP", "UPPER(PADL(NAME, 7))");

    const fs::path main_path = temp_root / "seek_padl_default.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG LPADSP\n"
        "lSeekCmd = SEEK('  BRAVO')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('  ALPHA', 'People', 'LPADSP')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PADL() default-padding seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a default PADL() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command default PADL() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a default PADL() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() default PADL() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match default PADL()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "2", "command SEEK should land on the default PADL()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match default PADL()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on the requested default PADL()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_padr_default_padding_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_padr_default";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "RPADSP", "UPPER(PADR(NAME, 7))");

    const fs::path main_path = temp_root / "seek_padr_default.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG RPADSP\n"
        "lSeekCmd = SEEK('ALPHA  ')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('CHARLIE', 'People', 'RPADSP')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PADR() default-padding seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a default PADR() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command default PADR() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a default PADR() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() default PADR() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match default PADR()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "1", "command SEEK should land on the default PADR()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match default PADR()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "3", "SEEK() should land on the requested default PADR()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_str_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_str";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "AGESTR", "UPPER(STR(AGE, 3))");

    const fs::path main_path = temp_root / "seek_str.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG AGESTR\n"
        "lSeekCmd = SEEK(' 30')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK(' 10', 'People', 'AGESTR')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STR()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a STR() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command STR() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a STR() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() STR() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match STR()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on the STR()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match STR()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on the requested STR()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_str_default_width_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_str_default";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "AGESTRD", "UPPER(STR(AGE))");

    const fs::path main_path = temp_root / "seek_str_default.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG AGESTRD\n"
        "lSeekCmd = SEEK('        30')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('        10', 'People', 'AGESTRD')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STR() default-width seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a STR() default-width tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command STR() default-width SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a STR() default-width tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() STR() default-width RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match STR() default-width derived keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on STR() default-width exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match STR() default-width derived keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on requested STR() default-width match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_str_decimal_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_str_decimal";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "AGESTRX", "UPPER(STR(AGE, 5, 1))");

    const fs::path main_path = temp_root / "seek_str_decimal.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG AGESTRX\n"
        "lSeekCmd = SEEK(' 30.0')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK(' 10.0', 'People', 'AGESTRX')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STR() decimal seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a STR() decimal tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command STR() decimal SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a STR() decimal tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() STR() decimal RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match STR() decimal derived keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on STR() decimal exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match STR() decimal derived keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on requested STR() decimal match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_near_changes_seek_failure_position() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_near";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_near.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO'\n"
        "lNearFound = FOUND()\n"
        "lNearEof = EOF()\n"
        "nNearRec = RECNO()\n"
        "SET NEAR OFF\n"
        "SEEK 'BRAVO'\n"
        "lFarFound = FOUND()\n"
        "lFarEof = EOF()\n"
        "nFarRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET NEAR seek script should complete");

    const auto near_found = state.globals.find("lnearfound");
    const auto near_eof = state.globals.find("lneareof");
    const auto near_rec = state.globals.find("nnearrec");
    const auto far_found = state.globals.find("lfarfound");
    const auto far_eof = state.globals.find("lfareof");
    const auto far_rec = state.globals.find("nfarrec");

    expect(near_found != state.globals.end(), "FOUND() after SET NEAR ON should be captured");
    expect(near_eof != state.globals.end(), "EOF() after SET NEAR ON should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after SET NEAR ON should be captured");
    expect(far_found != state.globals.end(), "FOUND() after SET NEAR OFF should be captured");
    expect(far_eof != state.globals.end(), "EOF() after SET NEAR OFF should be captured");
    expect(far_rec != state.globals.end(), "RECNO() after SET NEAR OFF should be captured");

    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "SET NEAR ON should keep FOUND() false when SEEK misses");
    }
    if (near_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_eof->second) == "false", "SET NEAR ON should position to the nearest record instead of EOF");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "2", "SET NEAR ON should position to the next ordered record");
    }
    if (far_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(far_found->second) == "false", "SET NEAR OFF should still report a failed seek");
    }
    if (far_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(far_eof->second) == "true", "SET NEAR OFF should leave the cursor at EOF after a missed seek");
    }
    if (far_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(far_rec->second) == "4", "SET NEAR OFF should place RECNO() at record_count + 1 after a missed seek");
    }

    expect(
        has_runtime_event(state.events, "runtime.set", "NEAR ON") &&
        has_runtime_event(state.events, "runtime.set", "NEAR OFF"),
        "SET NEAR changes should emit runtime.set events");

    fs::remove_all(temp_root, ignored);
}

void test_set_order_descending_changes_seek_ordering() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_order_descending";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_order_descending.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME DESCENDING\n"
        "cOrder = ORDER()\n"
        "SEEK 'CHARLIE'\n"
        "lExactFound = FOUND()\n"
        "nExactRec = RECNO()\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO'\n"
        "lNearFound = FOUND()\n"
        "lNearEof = EOF()\n"
        "nNearRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "descending SET ORDER script should complete");

    const auto order = state.globals.find("corder");
    const auto exact_found = state.globals.find("lexactfound");
    const auto exact_rec = state.globals.find("nexactrec");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_eof = state.globals.find("lneareof");
    const auto near_rec = state.globals.find("nnearrec");

    expect(order != state.globals.end(), "ORDER() after descending SET ORDER should be captured");
    expect(exact_found != state.globals.end(), "FOUND() after exact descending SEEK should be captured");
    expect(exact_rec != state.globals.end(), "RECNO() after exact descending SEEK should be captured");
    expect(near_found != state.globals.end(), "FOUND() after descending SET NEAR seek should be captured");
    expect(near_eof != state.globals.end(), "EOF() after descending SET NEAR seek should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after descending SET NEAR seek should be captured");

    if (order != state.globals.end()) {
        expect(copperfin::runtime::format_value(order->second) == "NAME", "descending SET ORDER should still expose the controlling tag name");
    }
    if (exact_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_found->second) == "true", "descending exact SEEK should still report FOUND()");
    }
    if (exact_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_rec->second) == "2", "descending exact SEEK should still land on the matching row");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "descending SET NEAR SEEK should still report a miss");
    }
    if (near_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_eof->second) == "false", "descending SET NEAR SEEK should stay off EOF when a nearby key exists");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "1", "descending SET NEAR SEEK should move to the next row in descending order");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("NAME [norm=upper, coll=case-folded, dir=descending]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded, dir=descending]: BRAVO -> not found") != std::string::npos;
        }),
        "descending SET ORDER and SEEK should emit order-direction metadata");

    fs::remove_all(temp_root, ignored);
}

void test_seek_command_accepts_tag_override_without_set_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_tag_override";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_tag_override.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SEEK 'BRAVO' TAG NAME\n"
        "lFound = FOUND()\n"
        "nRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK ... TAG script should complete");

    const auto found = state.globals.find("lfound");
    const auto rec = state.globals.find("nrec");
    const auto order_after = state.globals.find("corderafter");

    expect(found != state.globals.end(), "SEEK ... TAG should expose FOUND()");
    expect(rec != state.globals.end(), "SEEK ... TAG should expose RECNO()");
    expect(order_after != state.globals.end(), "SEEK ... TAG should leave ORDER() observable");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "true", "SEEK ... TAG should find matches using the named tag");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "2", "SEEK ... TAG should position the cursor on the matching row");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "SEEK ... TAG should not permanently change the controlling order");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded]: BRAVO -> found") != std::string::npos;
        }),
        "SEEK ... TAG should expose the temporary order metadata in runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_seek_command_accepts_descending_tag_override_without_set_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_descending_tag_override";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_descending_tag_override.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO' TAG NAME DESCENDING\n"
        "lFound = FOUND()\n"
        "lEof = EOF()\n"
        "nRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK ... TAG DESCENDING script should complete");

    const auto found = state.globals.find("lfound");
    const auto eof = state.globals.find("leof");
    const auto rec = state.globals.find("nrec");
    const auto order_after = state.globals.find("corderafter");

    expect(found != state.globals.end(), "SEEK ... TAG DESCENDING should expose FOUND()");
    expect(eof != state.globals.end(), "SEEK ... TAG DESCENDING should expose EOF()");
    expect(rec != state.globals.end(), "SEEK ... TAG DESCENDING should expose RECNO()");
    expect(order_after != state.globals.end(), "SEEK ... TAG DESCENDING should leave ORDER() observable");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "false", "descending tag override should still report a miss for an in-between key");
    }
    if (eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof->second) == "false", "descending tag override should honor SET NEAR and stay off EOF");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "1", "descending tag override should position to the next row in descending order");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "descending tag override should not permanently change the controlling order");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded, dir=descending]: BRAVO -> not found") != std::string::npos;
        }),
        "SEEK ... TAG DESCENDING should expose the temporary descending metadata in runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_seek_related_index_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_functions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_functions.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "cOrder1 = ORDER()\n"
        "cOrder2 = ORDER('People', 1)\n"
        "cTag1 = TAG(1, 'People')\n"
        "lSeekFn = SEEK('BRAVO', 'People', 'NAME')\n"
        "nSeekRec = RECNO()\n"
        "GO TOP\n"
        "lIndexNoMove = INDEXSEEK('CHARLIE', .F., 'People', 'NAME')\n"
        "nAfterNoMove = RECNO()\n"
        "lIndexMove = INDEXSEEK('CHARLIE', .T., 'People', 'NAME')\n"
        "nAfterMove = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "seek/index helper script should complete");

    const auto order1 = state.globals.find("corder1");
    const auto order2 = state.globals.find("corder2");
    const auto tag1 = state.globals.find("ctag1");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_rec = state.globals.find("nseekrec");
    const auto index_no_move = state.globals.find("lindexnomove");
    const auto after_no_move = state.globals.find("nafternomove");
    const auto index_move = state.globals.find("lindexmove");
    const auto after_move = state.globals.find("naftermove");

    expect(order1 != state.globals.end(), "ORDER() should be captured");
    expect(order2 != state.globals.end(), "ORDER(alias, pathFlag) should be captured");
    expect(tag1 != state.globals.end(), "TAG() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() should be captured");
    expect(seek_rec != state.globals.end(), "RECNO() after SEEK() should be captured");
    expect(index_no_move != state.globals.end(), "INDEXSEEK(.F.) should be captured");
    expect(after_no_move != state.globals.end(), "RECNO() after INDEXSEEK(.F.) should be captured");
    expect(index_move != state.globals.end(), "INDEXSEEK(.T.) should be captured");
    expect(after_move != state.globals.end(), "RECNO() after INDEXSEEK(.T.) should be captured");

    if (order1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(order1->second) == "NAME", "ORDER() should expose the controlling tag");
    }
    if (order2 != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(order2->second).find("PEOPLE.CDX") != std::string::npos,
            "ORDER(alias, pathFlag) should expose the controlling index path");
    }
    if (tag1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(tag1->second) == "NAME", "TAG() should expose the first open tag");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should return true for a match");
    }
    if (seek_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_rec->second) == "2", "SEEK() should move the record pointer to the matching row");
    }
    if (index_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_no_move->second) == "true", "INDEXSEEK(.F.) should report matches");
    }
    if (after_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_no_move->second) == "1", "INDEXSEEK(.F.) should not move the record pointer");
    }
    if (index_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_move->second) == "true", "INDEXSEEK(.T.) should report matches");
    }
    if (after_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_move->second) == "3", "INDEXSEEK(.T.) should move the record pointer to the match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_function_accepts_direction_suffix_in_order_designator() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_direction_suffix";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_direction_suffix.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "lAscFound = SEEK('BRAVO', 'People', 'NAME')\n"
        "nAscRec = RECNO()\n"
        "GO TOP\n"
        "lDescFound = SEEK('BRAVO', 'People', 'NAME DESCENDING')\n"
        "nDescRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK() direction-suffix script should complete");

    const auto asc_found = state.globals.find("lascfound");
    const auto asc_rec = state.globals.find("nascrec");
    const auto desc_found = state.globals.find("ldescfound");
    const auto desc_rec = state.globals.find("ndescrec");
    const auto order_after = state.globals.find("corderafter");

    expect(asc_found != state.globals.end(), "ascending SEEK() result should be captured");
    expect(asc_rec != state.globals.end(), "ascending SEEK() RECNO() should be captured");
    expect(desc_found != state.globals.end(), "descending SEEK() result should be captured");
    expect(desc_rec != state.globals.end(), "descending SEEK() RECNO() should be captured");
    expect(order_after != state.globals.end(), "ORDER() after SEEK() probes should be captured");

    if (asc_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(asc_found->second) == "false", "ascending SEEK() should report a miss for an in-between key");
    }
    if (asc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(asc_rec->second) == "2", "ascending SEEK() should move to the next row in ascending order");
    }
    if (desc_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(desc_found->second) == "false", "descending SEEK() should report a miss for an in-between key");
    }
    if (desc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(desc_rec->second) == "1", "descending SEEK() should move to the next row in descending order");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "SEEK() order-designator override should not permanently change ORDER()");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_table_temporary_order_expression_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_local_temp_order_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "NAME");

    const fs::path main_path = temp_root / "local_temp_order_parity.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "lSeekFn = SEEK('bravo', 'People', 'UPPER(NAME)')\n"
        "nSeekFnRec = RECNO('People')\n"
        "GO TOP IN People\n"
        "SET ORDER TO UPPER(NAME)\n"
        "SEEK 'charlie'\n"
        "lFoundCmd = FOUND('People')\n"
        "nSeekCmdRec = RECNO('People')\n"
        "SET NEAR ON\n"
        "GO TOP IN People\n"
        "lSeekNearDesc = SEEK('beta', 'People', 'UPPER(NAME) DESCENDING')\n"
        "nSeekNearDescRec = RECNO('People')\n"
        "cOrderAfter = ORDER('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local temporary-order normalization parity script should complete");

    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");
    const auto found_cmd = state.globals.find("lfoundcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_near_desc = state.globals.find("lseekneardesc");
    const auto seek_near_desc_rec = state.globals.find("nseekneardescrec");
    const auto order_after = state.globals.find("corderafter");

    expect(seek_fn != state.globals.end(), "SEEK() with UPPER(NAME) should be captured for a local table");
    expect(seek_fn_rec != state.globals.end(), "RECNO() after local SEEK() with UPPER(NAME) should be captured");
    expect(found_cmd != state.globals.end(), "FOUND() after command SEEK with local UPPER(NAME) order should be captured");
    expect(seek_cmd_rec != state.globals.end(), "RECNO() after command SEEK with local UPPER(NAME) order should be captured");
    expect(seek_near_desc != state.globals.end(), "descending local SEEK() with UPPER(NAME) should be captured");
    expect(seek_near_desc_rec != state.globals.end(), "RECNO() after descending local SEEK() with UPPER(NAME) should be captured");
    expect(order_after != state.globals.end(), "ORDER() after local temporary-order probes should be captured");

    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should case-fold search keys for local UPPER(NAME) temporary orders");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should position the local cursor on the normalized match");
    }
    if (found_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_cmd->second) == "true", "command SEEK should case-fold search keys for local UPPER(NAME) orders");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should position the local cursor on the normalized match");
    }
    if (seek_near_desc != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_near_desc->second) == "false", "descending local SEEK() should still report a miss for an in-between key");
    }
    if (seek_near_desc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_near_desc_rec->second) == "1", "descending local SEEK() should near-position to the next row in descending order after case-folding");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second) == "UPPER(NAME)", "SET ORDER TO UPPER(NAME) should preserve the local temporary order expression as ORDER()");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("UPPER(NAME) [norm=upper, coll=case-folded]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("UPPER(NAME) [norm=upper, coll=case-folded]: charlie -> found") != std::string::npos;
        }),
        "local temporary-order normalization should emit runtime.order and runtime.seek metadata");

    fs::remove_all(temp_root, ignored);
}

void test_order_and_tag_preserve_index_file_identity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_idx_identity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path idx_path = temp_root / "people.idx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_idx(idx_path, "UPPER(NAME)");

    const fs::path main_path = temp_root / "idx_identity.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "cOrderPath = ORDER('People', 1)\n"
        "cTagFromIdx = TAG('" + idx_path.string() + "', 1, 'People')\n"
        "lSeek = SEEK('CHARLIE', 'People')\n"
        "nSeekRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "idx identity script should complete");

    const auto order_path = state.globals.find("corderpath");
    const auto tag_from_idx = state.globals.find("ctagfromidx");
    const auto seek_value = state.globals.find("lseek");
    const auto seek_rec = state.globals.find("nseekrec");

    expect(order_path != state.globals.end(), "ORDER(alias, pathFlag) should be captured for IDX-backed orders");
    expect(tag_from_idx != state.globals.end(), "TAG(indexFile, tagNumber, alias) should be captured for IDX-backed orders");
    expect(seek_value != state.globals.end(), "SEEK() should be captured for IDX-backed orders");
    expect(seek_rec != state.globals.end(), "RECNO() after IDX-backed SEEK() should be captured");

    if (order_path != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(order_path->second).find("PEOPLE.IDX") != std::string::npos,
            "ORDER(alias, pathFlag) should preserve the actual IDX file identity");
    }
    if (tag_from_idx != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(tag_from_idx->second) == "PEOPLE",
            "TAG(indexFile, tagNumber, alias) should resolve the order from the actual IDX file");
    }
    if (seek_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_value->second) == "true", "SEEK() should work with the loaded IDX order");
    }
    if (seek_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_rec->second) == "3", "SEEK() should move to the matching record when using the IDX order");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_command_seek_in_target_with_temporary_order_expression() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_local_command_seek_in_target_temp_order";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path people_cdx_path = temp_root / "people.cdx";
    const fs::path other_cdx_path = temp_root / "other.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(people_cdx_path, "NAME", "NAME");
    write_synthetic_cdx(other_cdx_path, "NAME", "NAME");

    const fs::path main_path = temp_root / "local_command_seek_in_target_temp_order.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "nOtherRecBefore = RECNO()\n"
        "SET ORDER TO UPPER(NAME) IN People\n"
        "cPeopleOrder = ORDER('People')\n"
        "cOtherOrder = ORDER('Other')\n"
        "SEEK 'CHARLIE' IN People\n"
        "cAliasAfterSeek = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "nPeopleRecAfterSeek = RECNO('People')\n"
        "SELECT People\n"
        "cPeopleNameAfterSeek = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local command-path SEEK IN target temporary-order script should complete");

    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto people_order = state.globals.find("cpeopleorder");
    const auto other_order = state.globals.find("cotherorder");
    const auto alias_after_seek = state.globals.find("caliasafterseek");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto people_rec_after_seek = state.globals.find("npeoplerecafterseek");
    const auto people_name_after_seek = state.globals.find("cpeoplenameafterseek");

    expect(other_rec_before != state.globals.end(), "RECNO() before targeted local SEEK should be captured");
    expect(people_order != state.globals.end(), "ORDER('People') after targeted local SET ORDER should be captured");
    expect(other_order != state.globals.end(), "ORDER('Other') after targeted local SET ORDER should be captured");
    expect(alias_after_seek != state.globals.end(), "ALIAS() after targeted local SEEK should be captured");
    expect(other_rec_after != state.globals.end(), "RECNO() on selected local cursor after targeted SEEK should be captured");
    expect(people_rec_after_seek != state.globals.end(), "RECNO() on targeted local cursor after targeted SEEK should be captured");
    expect(people_name_after_seek != state.globals.end(), "NAME on targeted local cursor after targeted SEEK should be captured");

    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "1", "selected local cursor should begin at first record");
    }
    if (people_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_order->second) == "UPPER(NAME)", "SET ORDER TO UPPER(NAME) IN People should affect the targeted local cursor");
    }
    if (other_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_order->second).empty(), "SET ORDER TO ... IN People should not alter the selected non-target local cursor");
    }
    if (alias_after_seek != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_seek->second)) == "OTHER", "SEEK ... IN People should preserve the current selected alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "1", "SEEK ... IN People should not move the selected non-target local cursor pointer");
    }
    if (people_rec_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_rec_after_seek->second) == "3", "SEEK ... IN People should move the targeted local cursor pointer to the match");
    }
    if (people_name_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_name_after_seek->second) == "CHARLIE", "SEEK ... IN People should expose the targeted local row values");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("UPPER(NAME) [norm=upper, coll=case-folded]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("UPPER(NAME) [norm=upper, coll=case-folded]: CHARLIE -> found") != std::string::npos;
        }),
        "local command-path SET ORDER ... IN and SEEK ... IN should emit runtime.order and runtime.seek events for targeted local cursors");

    fs::remove_all(temp_root, ignored);
}

void test_local_descending_temporary_order_expression_in_target_preserves_selection() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_local_desc_temp_order_in_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path people_cdx_path = temp_root / "people.cdx";
    const fs::path other_cdx_path = temp_root / "other.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(people_cdx_path, "NAME", "NAME");
    write_synthetic_cdx(other_cdx_path, "NAME", "NAME");

    const fs::path main_path = temp_root / "local_desc_temp_order_in_target.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "GO BOTTOM\n"
        "nOtherRecBefore = RECNO()\n"
        "SET ORDER TO UPPER(NAME) IN People DESCENDING\n"
        "SET NEAR ON\n"
        "SEEK 'BETA' IN People\n"
        "cAliasAfterSeek = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "nPeopleRecAfterSeek = RECNO('People')\n"
        "SET NEAR OFF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local descending temporary-order IN-target script should complete");

    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto alias_after_seek = state.globals.find("caliasafterseek");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto people_rec_after_seek = state.globals.find("npeoplerecafterseek");

    expect(other_rec_before != state.globals.end(), "selected local cursor RECNO() before targeted descending seek should be captured");
    expect(alias_after_seek != state.globals.end(), "ALIAS() after targeted descending local seek should be captured");
    expect(other_rec_after != state.globals.end(), "selected local cursor RECNO() after targeted descending seek should be captured");
    expect(people_rec_after_seek != state.globals.end(), "target local cursor RECNO() after targeted descending seek should be captured");

    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected local cursor should start at bottom before targeted descending seek");
    }
    if (alias_after_seek != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_seek->second)) == "OTHER", "SEEK ... IN should preserve the selected local alias with descending order");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "SEEK ... IN should preserve selected local cursor pointer with descending order");
    }
    if (people_rec_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_rec_after_seek->second) == "1", "descending SET ORDER ... IN plus SET NEAR should position targeted local cursor on descending near-match record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_plain_temporary_order_in_target_honors_collate_and_preserves_selection() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_local_plain_temp_order_collate_in_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path people_cdx_path = temp_root / "people.cdx";
    const fs::path other_cdx_path = temp_root / "other.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(people_cdx_path, "NAME", "NAME");
    write_synthetic_cdx(other_cdx_path, "NAME", "NAME");

    const fs::path main_path = temp_root / "local_plain_temp_order_collate_in_target.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "GO BOTTOM\n"
        "nOtherRecBefore = RECNO()\n"
        "SET ORDER TO NAME IN People\n"
        "lMachineMiss = SEEK('bravo', 'People', 'NAME')\n"
        "nPeopleRecAfterMachine = RECNO('People')\n"
        "SET COLLATE TO GENERAL\n"
        "GO TOP IN People\n"
        "lGeneralHit = SEEK('bravo', 'People', 'NAME')\n"
        "SELECT People\n"
        "cPeopleNameAfterGeneral = NAME\n"
        "SELECT Other\n"
        "cAliasAfterSeek = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local plain temporary-order IN-target collate script should complete");

    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto machine_miss = state.globals.find("lmachinemiss");
    const auto people_rec_after_machine = state.globals.find("npeoplerecaftermachine");
    const auto general_hit = state.globals.find("lgeneralhit");
    const auto alias_after_seek = state.globals.find("caliasafterseek");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto people_name_after_general = state.globals.find("cpeoplenameaftergeneral");

    expect(other_rec_before != state.globals.end(), "selected local cursor RECNO() before targeted seek should be captured");
    expect(machine_miss != state.globals.end(), "MACHINE-collate targeted SEEK() miss should be captured");
    expect(people_rec_after_machine != state.globals.end(), "target local cursor RECNO() after MACHINE-collate seek should be captured");
    expect(general_hit != state.globals.end(), "GENERAL-collate targeted SEEK() hit should be captured");
    expect(alias_after_seek != state.globals.end(), "ALIAS() after targeted SEEK() should be captured");
    expect(other_rec_after != state.globals.end(), "selected local cursor RECNO() after targeted SEEK() should be captured");
    expect(people_name_after_general != state.globals.end(), "target local cursor NAME after GENERAL-collate seek should be captured");

    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected non-target local cursor should begin at bottom");
    }
    if (machine_miss != state.globals.end()) {
        expect(copperfin::runtime::format_value(machine_miss->second) == "false", "MACHINE collation should keep plain NAME seek case-sensitive");
    }
    if (people_rec_after_machine != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_rec_after_machine->second) == "4", "MACHINE-collate miss should position targeted cursor at EOF");
    }
    if (general_hit != state.globals.end()) {
        expect(copperfin::runtime::format_value(general_hit->second) == "true", "GENERAL collation should case-fold plain NAME seek in targeted local cursor");
    }
    if (alias_after_seek != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_seek->second)) == "OTHER", "targeted SEEK() should preserve selected local alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "targeted SEEK() should preserve selected non-target local cursor pointer");
    }
    if (people_name_after_general != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_name_after_general->second) == "BRAVO", "GENERAL-collate targeted seek should expose the case-folded match row");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_temporary_order_expression_indexseek_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_local_temp_order_indexseek";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "NAME");

    const fs::path main_path = temp_root / "local_temp_order_indexseek.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO TOP\n"
        "lIndexNoMove = INDEXSEEK('charlie', .F., 'People', 'UPPER(NAME)')\n"
        "nAfterNoMove = RECNO('People')\n"
        "lIndexMove = INDEXSEEK('charlie', .T., 'People', 'UPPER(NAME)')\n"
        "nAfterMove = RECNO('People')\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "lIndexMoveDesc = INDEXSEEK('beta', .T., 'People', 'UPPER(NAME) DESCENDING')\n"
        "nAfterMoveDesc = RECNO('People')\n"
        "cOrderAfter = ORDER('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local temporary-order INDEXSEEK parity script should complete");

    const auto index_no_move = state.globals.find("lindexnomove");
    const auto after_no_move = state.globals.find("nafternomove");
    const auto index_move = state.globals.find("lindexmove");
    const auto after_move = state.globals.find("naftermove");
    const auto index_move_desc = state.globals.find("lindexmovedesc");
    const auto after_move_desc = state.globals.find("naftermovedesc");
    const auto order_after = state.globals.find("corderafter");

    expect(index_no_move != state.globals.end(), "INDEXSEEK(.F.) with UPPER(NAME) should be captured for a local table");
    expect(after_no_move != state.globals.end(), "RECNO() after local INDEXSEEK(.F.) with UPPER(NAME) should be captured");
    expect(index_move != state.globals.end(), "INDEXSEEK(.T.) with UPPER(NAME) should be captured for a local table");
    expect(after_move != state.globals.end(), "RECNO() after local INDEXSEEK(.T.) with UPPER(NAME) should be captured");
    expect(index_move_desc != state.globals.end(), "descending local INDEXSEEK(.T.) with UPPER(NAME) should be captured");
    expect(after_move_desc != state.globals.end(), "RECNO() after descending local INDEXSEEK(.T.) with UPPER(NAME) should be captured");
    expect(order_after != state.globals.end(), "ORDER() after local temporary-order INDEXSEEK probes should be captured");

    if (index_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_no_move->second) == "true", "INDEXSEEK(.F.) should report local UPPER(NAME) matches");
    }
    if (after_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_no_move->second) == "1", "INDEXSEEK(.F.) should not move the local record pointer");
    }
    if (index_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_move->second) == "true", "INDEXSEEK(.T.) should report local UPPER(NAME) matches");
    }
    if (after_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_move->second) == "3", "INDEXSEEK(.T.) should move the local record pointer to the matching row");
    }
    if (index_move_desc != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_move_desc->second) == "false", "descending local INDEXSEEK(.T.) should still report a miss for an in-between key");
    }
    if (after_move_desc != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_move_desc->second) == "1", "descending local INDEXSEEK(.T.) should move to the descending near-match row after case-folding");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "one-off local temporary-order INDEXSEEK probes should not permanently change ORDER()");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_respects_grounded_order_for_expression_hints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_order_for_expression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path idx_path = temp_root / "people.idx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    mark_simple_dbf_record_deleted(table_path, 2U);
    write_synthetic_idx_with_for(idx_path, "UPPER(NAME)", "DELETED() = .F.");

    const fs::path main_path = temp_root / "order_for_expression.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "SEEK 'BRAVO'\n"
        "lDeletedFound = FOUND()\n"
        "lDeletedEof = EOF()\n"
        "nDeletedRec = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "SEEK 'BRAVO'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "SEEK 'CHARLIE'\n"
        "lVisibleFound = FOUND()\n"
        "nVisibleRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR-filtered order SEEK script should complete");

    const auto deleted_found = state.globals.find("ldeletedfound");
    const auto deleted_eof = state.globals.find("ldeletedeof");
    const auto deleted_rec = state.globals.find("ndeletedrec");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_rec = state.globals.find("nnearrec");
    const auto visible_found = state.globals.find("lvisiblefound");
    const auto visible_rec = state.globals.find("nvisiblerec");

    expect(deleted_found != state.globals.end(), "SEEK on a filtered-out key should expose FOUND()");
    expect(deleted_eof != state.globals.end(), "SEEK on a filtered-out key should expose EOF()");
    expect(deleted_rec != state.globals.end(), "SEEK on a filtered-out key should expose RECNO()");
    expect(near_found != state.globals.end(), "SET NEAR SEEK on a filtered-out key should expose FOUND()");
    expect(near_rec != state.globals.end(), "SET NEAR SEEK on a filtered-out key should expose RECNO()");
    expect(visible_found != state.globals.end(), "SEEK on a visible key should expose FOUND()");
    expect(visible_rec != state.globals.end(), "SEEK on a visible key should expose RECNO()");

    if (deleted_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_found->second) == "false", "SEEK should ignore keys filtered out by the grounded order FOR expression");
    }
    if (deleted_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_eof->second) == "true", "SEEK without SET NEAR should move to EOF when only a filtered-out key matches");
    }
    if (deleted_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_rec->second) == "4", "SEEK without SET NEAR should position after the visible rows when the filtered-out key is skipped");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "SET NEAR should still report a miss for a filtered-out key");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "3", "SET NEAR should move to the next visible indexed key after a filtered-out match");
    }
    if (visible_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_found->second) == "true", "SEEK should still find keys allowed by the grounded order FOR expression");
    }
    if (visible_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_rec->second) == "3", "SEEK should position on the visible row that survives the order FOR expression");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_respects_set_deleted_visibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_set_deleted";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "BRAVO", "CHARLIE", "DELTA"});
    mark_simple_dbf_record_deleted(table_path, 2U);
    mark_simple_dbf_record_deleted(table_path, 4U);
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_set_deleted.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SET EXACT OFF\n"
        "SET DELETED OFF\n"
        "SEEK 'CHARLIE'\n"
        "lOffDeletedFound = FOUND()\n"
        "nOffDeletedRec = RECNO()\n"
        "lOffDeletedFlag = DELETED()\n"
        "SET DELETED ON\n"
        "SET NEAR OFF\n"
        "SEEK 'BRAVO'\n"
        "lDuplicateFound = FOUND()\n"
        "nDuplicateRec = RECNO()\n"
        "lDuplicateBof = BOF()\n"
        "lDuplicateEof = EOF()\n"
        "SEEK 'BRA'\n"
        "lPrefixDuplicateFound = FOUND()\n"
        "nPrefixDuplicateRec = RECNO()\n"
        "SEEK 'CHARLIE'\n"
        "lDeletedOnlyFound = FOUND()\n"
        "nDeletedOnlyRec = RECNO()\n"
        "lDeletedOnlyBof = BOF()\n"
        "lDeletedOnlyEof = EOF()\n"
        "lFunctionDeleted = SEEK('CHARLIE')\n"
        "nFunctionDeletedRec = RECNO()\n"
        "lPrefixDeleted = SEEK('CHAR')\n"
        "nPrefixDeletedRec = RECNO()\n"
        "GO TOP\n"
        "nIndexBefore = RECNO()\n"
        "lIndexDeleted = INDEXSEEK('CHARLIE')\n"
        "nIndexAfter = RECNO()\n"
        "lIndexMoveDeleted = INDEXSEEK('CHARLIE', .T.)\n"
        "nIndexMoveAfter = RECNO()\n"
        "SET NEAR ON\n"
        "lIndexMoveNearDeleted = INDEXSEEK('CHARLIE', .T.)\n"
        "nIndexMoveNearAfter = RECNO()\n"
        "SET FILTER TO NAME <> 'DELTA'\n"
        "SEEK 'CHARLIE'\n"
        "lFilteredNearFound = FOUND()\n"
        "nFilteredNearRec = RECNO()\n"
        "lFilteredNearEof = EOF()\n"
        "SET FILTER TO\n"
        "SEEK 'CHARLIE'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "lNearEof = EOF()\n"
        "SEEK 'CHAR'\n"
        "lPrefixNearFound = FOUND()\n"
        "nPrefixNearRec = RECNO()\n"
        "SET ORDER TO TAG NAME DESCENDING\n"
        "SEEK 'CHARLIE'\n"
        "lDescendingNearFound = FOUND()\n"
        "nDescendingNearRec = RECNO()\n"
        "SEEK 'ALPHA'\n"
        "lVisibleFound = FOUND()\n"
        "nVisibleRec = RECNO()\n"
        "SET DELETED OFF\n"
        "SET NEAR OFF\n"
        "SEEK 'CHARLIE'\n"
        "lOffAgainFound = FOUND()\n"
        "nOffAgainRec = RECNO()\n"
        "lOffAgainDeleted = DELETED()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK with SET DELETED visibility should complete");

    const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_value("loffdeletedfound", "true", "SET DELETED OFF should expose a deleted exact-key match");
    expect_value("noffdeletedrec", "4", "SET DELETED OFF should position on the deleted exact-key record");
    expect_value("loffdeletedflag", "true", "SET DELETED OFF should preserve the matched record's deleted flag");
    expect_value("lduplicatefound", "true", "SET DELETED ON should skip an earlier deleted duplicate and find a later live key");
    expect_value("nduplicaterec", "3", "SEEK should position on the first non-deleted duplicate");
    expect_value("lduplicatebof", "false", "a non-deleted duplicate hit should clear BOF()");
    expect_value("lduplicateeof", "false", "a non-deleted duplicate hit should clear EOF()");
    expect_value("lprefixduplicatefound", "true", "prefix SEEK should skip an earlier deleted duplicate and find a later live key");
    expect_value("nprefixduplicaterec", "3", "prefix SEEK should position on the first non-deleted duplicate");
    expect_value("ldeletedonlyfound", "false", "SET DELETED ON should reject a deleted-only exact key");
    expect_value("ndeletedonlyrec", "6", "a deleted-only miss with SET NEAR OFF should move to physical EOF");
    expect_value("ldeletedonlybof", "false", "a deleted-only miss should not set BOF()");
    expect_value("ldeletedonlyeof", "true", "a deleted-only miss with SET NEAR OFF should set EOF()");
    expect_value("lfunctiondeleted", "false", "SEEK() should share SET DELETED visibility behavior");
    expect_value("nfunctiondeletedrec", "6", "SEEK() should retain miss positioning for a deleted-only key");
    expect_value("lprefixdeleted", "false", "prefix SEEK() should reject a deleted-only key under SET DELETED ON");
    expect_value("nprefixdeletedrec", "6", "a deleted-only prefix miss should retain physical EOF positioning");
    expect_value("nindexbefore", "1", "INDEXSEEK() pointer-preservation setup should select the first live row");
    expect_value("lindexdeleted", "false", "INDEXSEEK() should not report a deleted-only key under SET DELETED ON");
    expect_value("nindexafter", "1", "INDEXSEEK(.F.) should preserve the record pointer after a deleted-only miss");
    expect_value("lindexmovedeleted", "false", "INDEXSEEK(.T.) should reject a deleted-only key with SET NEAR OFF");
    expect_value("nindexmoveafter", "1", "INDEXSEEK(.T.) should preserve the pointer when no match exists");
    expect_value("lindexmoveneardeleted", "false", "INDEXSEEK(.T.) should reject a deleted-only key with SET NEAR ON");
    expect_value("nindexmovenearafter", "1", "INDEXSEEK(.T.) should preserve the pointer on a near miss");
    expect_value("lfilterednearfound", "false", "SET FILTER should compose with SET DELETED during near lookup");
    expect_value("nfilterednearrec", "6", "a filter-hidden near candidate should leave the cursor at physical EOF");
    expect_value("lfilteredneareof", "true", "a filter-hidden near candidate should set EOF()");
    expect_value("lnearfound", "false", "SET NEAR should keep FOUND() false after skipping a deleted exact key");
    expect_value("nnearrec", "5", "SET NEAR should position on the next non-deleted indexed row");
    expect_value("lneareof", "false", "a non-deleted near candidate should clear EOF()");
    expect_value("lprefixnearfound", "false", "prefix SET NEAR should keep a deleted-only prefix as a miss");
    expect_value("nprefixnearrec", "5", "prefix SET NEAR should position on the next non-deleted indexed row");
    expect_value("ldescendingnearfound", "false", "descending SET NEAR should keep a deleted exact key as a miss");
    expect_value("ndescendingnearrec", "3", "descending SET NEAR should skip deleted candidates and choose the next live key");
    expect_value("lvisiblefound", "true", "SET DELETED ON should preserve exact hits on live rows");
    expect_value("nvisiblerec", "1", "a live descending exact hit should preserve its physical RECNO()");
    expect_value("loffagainfound", "true", "SET DELETED OFF should restore deleted-record SEEK access");
    expect_value("noffagainrec", "4", "SET DELETED OFF should restore the deleted exact-key position");
    expect_value("loffagaindeleted", "true", "the restored deleted exact hit should expose DELETED() true");

    fs::remove_all(temp_root, ignored);
}

void test_seek_respects_numeric_order_for_expression_hints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_order_for_numeric_expression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path idx_path = temp_root / "people.idx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_idx_with_for(idx_path, "UPPER(NAME)", "AGE >= 20");

    const fs::path main_path = temp_root / "order_for_numeric_expression.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "SEEK 'ALPHA'\n"
        "lFilteredFound = FOUND()\n"
        "lFilteredEof = EOF()\n"
        "nFilteredRec = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "SEEK 'ALPHA'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "SEEK 'CHARLIE'\n"
        "lVisibleFound = FOUND()\n"
        "nVisibleRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "numeric FOR-filtered order SEEK script should complete");

    const auto filtered_found = state.globals.find("lfilteredfound");
    const auto filtered_eof = state.globals.find("lfilteredeof");
    const auto filtered_rec = state.globals.find("nfilteredrec");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_rec = state.globals.find("nnearrec");
    const auto visible_found = state.globals.find("lvisiblefound");
    const auto visible_rec = state.globals.find("nvisiblerec");

    expect(filtered_found != state.globals.end(), "SEEK on a numerically filtered-out key should expose FOUND()");
    expect(filtered_eof != state.globals.end(), "SEEK on a numerically filtered-out key should expose EOF()");
    expect(filtered_rec != state.globals.end(), "SEEK on a numerically filtered-out key should expose RECNO()");
    expect(near_found != state.globals.end(), "SET NEAR SEEK on a numerically filtered-out key should expose FOUND()");
    expect(near_rec != state.globals.end(), "SET NEAR SEEK on a numerically filtered-out key should expose RECNO()");
    expect(visible_found != state.globals.end(), "SEEK on a numerically visible key should expose FOUND()");
    expect(visible_rec != state.globals.end(), "SEEK on a numerically visible key should expose RECNO()");

    if (filtered_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_found->second) == "false", "numeric FOR expressions should filter ALPHA out of the indexed candidate set");
    }
    if (filtered_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_eof->second) == "true", "a numerically filtered-out seek without SET NEAR should still land at EOF");
    }
    if (filtered_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_rec->second) == "4", "a numerically filtered-out seek without SET NEAR should position after the visible rows");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "SET NEAR should still report a miss for a numerically filtered-out key");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "2", "SET NEAR should move to the first row that survives the numeric FOR expression");
    }
    if (visible_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_found->second) == "true", "numeric FOR expressions should still allow visible indexed keys");
    }
    if (visible_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_rec->second) == "3", "numeric FOR expressions should still position on the matching visible row");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_respects_string_order_for_expression_hints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_order_for_string_expression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path idx_path = temp_root / "people.idx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_idx_with_for(idx_path, "UPPER(NAME)", "NAME = 'BRAVO'");

    const fs::path main_path = temp_root / "order_for_string_expression.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "SEEK 'ALPHA'\n"
        "lFilteredFound = FOUND()\n"
        "lFilteredEof = EOF()\n"
        "nFilteredRec = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "SEEK 'ALPHA'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "SEEK 'BRAVO'\n"
        "lVisibleFound = FOUND()\n"
        "nVisibleRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "string FOR-filtered order SEEK script should complete");

    const auto filtered_found = state.globals.find("lfilteredfound");
    const auto filtered_eof = state.globals.find("lfilteredeof");
    const auto filtered_rec = state.globals.find("nfilteredrec");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_rec = state.globals.find("nnearrec");
    const auto visible_found = state.globals.find("lvisiblefound");
    const auto visible_rec = state.globals.find("nvisiblerec");

    expect(filtered_found != state.globals.end(), "SEEK on a string-filtered-out key should expose FOUND()");
    expect(filtered_eof != state.globals.end(), "SEEK on a string-filtered-out key should expose EOF()");
    expect(filtered_rec != state.globals.end(), "SEEK on a string-filtered-out key should expose RECNO()");
    expect(near_found != state.globals.end(), "SET NEAR SEEK on a string-filtered-out key should expose FOUND()");
    expect(near_rec != state.globals.end(), "SET NEAR SEEK on a string-filtered-out key should expose RECNO()");
    expect(visible_found != state.globals.end(), "SEEK on a string-visible key should expose FOUND()");
    expect(visible_rec != state.globals.end(), "SEEK on a string-visible key should expose RECNO()");

    if (filtered_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_found->second) == "false", "string FOR expressions should filter ALPHA out of the indexed candidate set");
    }
    if (filtered_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_eof->second) == "true", "a string-filtered seek without SET NEAR should still land at EOF");
    }
    if (filtered_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_rec->second) == "4", "a string-filtered seek without SET NEAR should position after the visible rows");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "SET NEAR should still report a miss for a string-filtered-out key");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "2", "SET NEAR should move to the surviving string-filtered row");
    }
    if (visible_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_found->second) == "true", "string FOR expressions should still allow visible indexed keys");
    }
    if (visible_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_rec->second) == "2", "string FOR expressions should still position on the matching visible row");
    }

    fs::remove_all(temp_root, ignored);
}


void test_ndx_numeric_domain_guides_seek_near_ordering() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_ndx_numeric_domain";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path ndx_path = temp_root / "people.ndx";
    write_people_dbf(table_path, {{"ALPHA", 2}, {"BRAVO", 10}, {"CHARLIE", 20}});
    write_synthetic_ndx(ndx_path, "AGE", true);

    const fs::path main_path = temp_root / "ndx_numeric_domain.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "SET NEAR ON\n"
        "SEEK '9'\n"
        "lFound = FOUND()\n"
        "lEof = EOF()\n"
        "nRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "NDX numeric-domain seek script should complete");

    const auto found = state.globals.find("lfound");
    const auto eof = state.globals.find("leof");
    const auto rec = state.globals.find("nrec");

    expect(found != state.globals.end(), "NDX numeric-domain seek should expose FOUND()");
    expect(eof != state.globals.end(), "NDX numeric-domain seek should expose EOF()");
    expect(rec != state.globals.end(), "NDX numeric-domain seek should expose RECNO()");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "false", "NDX numeric-domain seek should still report a miss for a non-existent key");
    }
    if (eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof->second) == "false", "NDX numeric-domain SET NEAR should position to the next numeric key instead of EOF");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "2", "NDX numeric-domain seek should treat AGE keys numerically when choosing the nearest record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_numeric_temporary_order_domain_guides_seek_near_ordering() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_local_numeric_temporary_order_domain";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 2}, {"BRAVO", 10}, {"CHARLIE", 20}});

    const fs::path main_path = temp_root / "local_numeric_temporary_order_domain.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO AGE\n"
        "SEEK '1'\n"
        "lPrefixFound = FOUND()\n"
        "lPrefixEof = EOF()\n"
        "nPrefixRec = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "SEEK '9'\n"
        "lFound = FOUND()\n"
        "nRec = RECNO()\n"
        "nAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local numeric temporary-order key-domain seek script should complete");

    const auto prefix_found = state.globals.find("lprefixfound");
    const auto prefix_eof = state.globals.find("lprefixeof");
    const auto prefix_rec = state.globals.find("nprefixrec");
    const auto found = state.globals.find("lfound");
    const auto rec = state.globals.find("nrec");
    const auto age = state.globals.find("nage");

    expect(prefix_found != state.globals.end(), "local numeric temporary-order exact-seek miss should expose FOUND()");
    expect(prefix_eof != state.globals.end(), "local numeric temporary-order exact-seek miss should expose EOF()");
    expect(prefix_rec != state.globals.end(), "local numeric temporary-order exact-seek miss should expose RECNO()");
    expect(found != state.globals.end(), "local numeric temporary-order seek should expose FOUND()");
    expect(rec != state.globals.end(), "local numeric temporary-order seek should expose RECNO()");
    expect(age != state.globals.end(), "local numeric temporary-order seek should expose AGE");

    if (prefix_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(prefix_found->second) == "false", "local numeric temporary-order SEEK() should not treat a numeric prefix as an exact hit when SET EXACT is OFF");
    }
    if (prefix_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(prefix_eof->second) == "true", "local numeric temporary-order exact miss without SET NEAR should still land at EOF");
    }
    if (prefix_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(prefix_rec->second) == "4", "local numeric temporary-order exact miss without SET NEAR should place RECNO() at record_count + 1");
    }
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "false", "local numeric temporary-order SEEK() should still report a miss for a non-existent key");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "2", "local numeric temporary-order SET NEAR should move to the next numeric row instead of lexicographic EOF");
    }
    if (age != state.globals.end()) {
        expect(copperfin::runtime::format_value(age->second) == "10", "local numeric temporary-order key-domain hints should treat AGE numerically");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_near_is_scoped_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_near_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_near_datasession.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO'\n"
        "lNear1Found = FOUND()\n"
        "lNear1Eof = EOF()\n"
        "nNear1Rec = RECNO()\n"
        "SET DATASESSION TO 2\n"
        "USE '" + table_path.string() + "' ALIAS PeopleTwo IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SEEK 'BRAVO'\n"
        "lNear2Found = FOUND()\n"
        "lNear2Eof = EOF()\n"
        "nNear2Rec = RECNO()\n"
        "SET DATASESSION TO 1\n"
        "GO TOP\n"
        "SEEK 'BRAVO'\n"
        "lNear1BackFound = FOUND()\n"
        "lNear1BackEof = EOF()\n"
        "nNear1BackRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET NEAR data-session script should complete");

    const auto near1_found = state.globals.find("lnear1found");
    const auto near1_eof = state.globals.find("lnear1eof");
    const auto near1_rec = state.globals.find("nnear1rec");
    const auto near2_found = state.globals.find("lnear2found");
    const auto near2_eof = state.globals.find("lnear2eof");
    const auto near2_rec = state.globals.find("nnear2rec");
    const auto near1_back_found = state.globals.find("lnear1backfound");
    const auto near1_back_eof = state.globals.find("lnear1backeof");
    const auto near1_back_rec = state.globals.find("nnear1backrec");

    expect(near1_found != state.globals.end(), "session-1 SET NEAR FOUND() should be captured");
    expect(near1_eof != state.globals.end(), "session-1 SET NEAR EOF() should be captured");
    expect(near1_rec != state.globals.end(), "session-1 SET NEAR RECNO() should be captured");
    expect(near2_found != state.globals.end(), "session-2 SEEK FOUND() should be captured");
    expect(near2_eof != state.globals.end(), "session-2 SEEK EOF() should be captured");
    expect(near2_rec != state.globals.end(), "session-2 SEEK RECNO() should be captured");
    expect(near1_back_found != state.globals.end(), "restored session-1 SEEK FOUND() should be captured");
    expect(near1_back_eof != state.globals.end(), "restored session-1 SEEK EOF() should be captured");
    expect(near1_back_rec != state.globals.end(), "restored session-1 SEEK RECNO() should be captured");

    if (near1_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_found->second) == "false", "SET NEAR ON should still leave FOUND() false on a missed seek");
    }
    if (near1_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_eof->second) == "false", "SET NEAR ON in session 1 should keep the cursor off EOF");
    }
    if (near1_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_rec->second) == "2", "SET NEAR ON in session 1 should move to the nearest ordered row");
    }
    if (near2_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_found->second) == "false", "a fresh second data session should still report a missed seek");
    }
    if (near2_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_eof->second) == "true", "SET NEAR should not bleed into a fresh second data session");
    }
    if (near2_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_rec->second) == "4", "a fresh second data session should keep the default SET NEAR OFF seek position");
    }
    if (near1_back_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_found->second) == "false", "restoring session 1 should preserve missed-seek FOUND() behavior");
    }
    if (near1_back_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_eof->second) == "false", "restoring session 1 should restore its SET NEAR ON behavior");
    }
    if (near1_back_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_rec->second) == "2", "restoring session 1 should restore its nearest-record seek position");
    }

    fs::remove_all(temp_root, ignored);
}

void test_foxtools_registration_and_call_bridge() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_foxtools";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "foxtools.prg";
    write_text(
        main_path,
        "nLibraryPathCalls = 0\n"
        "SET LIBRARY TO library_name('Foxtools')\n"
        "cFoxTools = FoxToolVer()\n"
        "nMain = MainHwnd()\n"
        "hPid = RegFn32('GetCurrentProcessId', '', 'I', 'kernel32.dll')\n"
        "nPid = CallFn(hPid)\n"
        "hLen = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "nLen = CallFn(hLen, 'Copperfin')\n"
        "RETURN\n"
        "FUNCTION library_name\n"
        "LPARAMETERS value\n"
        "nLibraryPathCalls = nLibraryPathCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Foxtools bridge script should complete");

    const auto foxtools = state.globals.find("cfoxtools");
    const auto main = state.globals.find("nmain");
    const auto hpid = state.globals.find("hpid");
    const auto pid = state.globals.find("npid");
    const auto hlen = state.globals.find("hlen");
    const auto length = state.globals.find("nlen");
    const auto library_path_calls = state.globals.find("nlibrarypathcalls");

    expect(foxtools != state.globals.end(), "FoxToolVer() should be captured");
    expect(main != state.globals.end(), "MainHwnd() should be captured");
    expect(hpid != state.globals.end(), "RegFn32 handle should be captured");
    expect(pid != state.globals.end(), "CallFn(handle) should be captured");
    expect(hlen != state.globals.end(), "second RegFn32 handle should be captured");
    expect(length != state.globals.end(), "CallFn(string) should be captured");
    expect(library_path_calls != state.globals.end(), "SET LIBRARY should preserve the designator resolver call counter");
    if (library_path_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(library_path_calls->second) == "1",
               "SET LIBRARY should evaluate the designator UDF exactly once");
    }

    if (foxtools != state.globals.end()) {
        expect(!copperfin::runtime::format_value(foxtools->second).empty(), "FoxToolVer() should return a non-empty version string");
    }
    if (main != state.globals.end()) {
        expect(copperfin::runtime::format_value(main->second) == "1001", "MainHwnd() should expose the placeholder host window handle");
    }
    if (hpid != state.globals.end()) {
        expect(copperfin::runtime::format_value(hpid->second) == "1", "first RegFn32 call should allocate handle 1");
    }
    if (pid != state.globals.end()) {
        expect(copperfin::runtime::format_value(pid->second) != "0", "CallFn(GetCurrentProcessId) should return a non-zero process id");
    }
    if (hlen != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen->second) == "2", "second RegFn32 call should allocate handle 2");
    }
    if (length != state.globals.end()) {
        expect(copperfin::runtime::format_value(length->second) == "9", "CallFn(lstrlenA, 'Copperfin') should return the string length");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.library"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "interop.regfn"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "interop.callfn"; }),
        "Foxtools bridge should emit library, registration, and call events");
    const auto regfn_getpid_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "interop.regfn" && event.detail.find("GetCurrentProcessId") != std::string::npos &&
               event.detail.find("returns I") != std::string::npos && event.detail.find("args=void") != std::string::npos;
    });
    const auto regfn_strlen_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "interop.regfn" && event.detail.find("lstrlenA") != std::string::npos &&
               event.detail.find("returns I") != std::string::npos && event.detail.find("args=C") != std::string::npos;
    });
    const auto callpid_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "interop.callfn" && event.detail.find("GetCurrentProcessId#1") != std::string::npos &&
               event.detail.find("expects") != std::string::npos;
    });
    const auto callstr_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "interop.callfn" && event.detail.find("lstrlenA#2") != std::string::npos &&
               event.detail.find("expects") != std::string::npos;
    });
    expect(regfn_getpid_event != state.events.end(), "interop.regfn should include GetCurrentProcessId contract detail");
    expect(regfn_strlen_event != state.events.end(), "interop.regfn should include lstrlenA contract detail");
    expect(callpid_event != state.events.end(), "interop.callfn should include handle-scoped function contract detail for GetCurrentProcessId");
    expect(callstr_event != state.events.end(), "interop.callfn should include handle-scoped function contract detail for lstrlenA");

    fs::remove_all(temp_root, ignored);
}

void test_foxtools_registration_is_scoped_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_foxtools_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "foxtools_datasession.prg";
    write_text(
        main_path,
        "SET LIBRARY TO 'Foxtools'\n"
        "hPid1 = RegFn32('GetCurrentProcessId', '', 'I', 'kernel32.dll')\n"
        "SET DATASESSION TO 2\n"
        "SET LIBRARY TO 'Foxtools'\n"
        "nCrossCall = CallFn(hPid1)\n"
        "hLen2 = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "nLen2 = CallFn(hLen2, 'AB')\n"
        "SET DATASESSION TO 1\n"
        "nPid1Back = CallFn(hPid1)\n"
        "hLen1Back = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Foxtools data-session script should complete");

    const auto hpid1 = state.globals.find("hpid1");
    const auto cross_call = state.globals.find("ncrosscall");
    const auto hlen2 = state.globals.find("hlen2");
    const auto len2 = state.globals.find("nlen2");
    const auto pid1_back = state.globals.find("npid1back");
    const auto hlen1_back = state.globals.find("hlen1back");

    expect(hpid1 != state.globals.end(), "session-1 RegFn32 handle should be captured");
    expect(cross_call != state.globals.end(), "cross-session CallFn result should be captured");
    expect(hlen2 != state.globals.end(), "session-2 RegFn32 handle should be captured");
    expect(len2 != state.globals.end(), "session-2 CallFn result should be captured");
    expect(pid1_back != state.globals.end(), "restored session-1 CallFn result should be captured");
    expect(hlen1_back != state.globals.end(), "restored session-1 RegFn32 handle should be captured");

    if (hpid1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(hpid1->second) == "1", "the first RegFn32 handle in session 1 should be 1");
    }
    if (cross_call != state.globals.end()) {
        expect(copperfin::runtime::format_value(cross_call->second) == "-1", "CallFn should reject a RegFn32 handle from another data session");
    }
    if (hlen2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen2->second) == "1", "the first RegFn32 handle in a fresh second data session should restart at 1");
    }
    if (len2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(len2->second) == "2", "session-2 CallFn should use its own registered handle");
    }
    if (pid1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(pid1_back->second) != "0", "restoring session 1 should restore its RegFn32 handle lookup");
    }
    if (hlen1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen1_back->second) == "2", "restoring session 1 should restore its next RegFn32 handle allocation");
    }

    fs::remove_all(temp_root, ignored);
}

void test_declared_dll_string_byref_argument_writeback() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "declared_dll_byref.prg";
    write_text(
        main_path,
        "DECLARE INTEGER lstrcpyA(STRING @, STRING) IN 'kernel32.dll'\n"
        "cBuffer = SPACE(32)\n"
        "nResult = lstrcpyA(@cBuffer, 'Copperfin')\n"
        "cCopied = cBuffer\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "declared DLL by-ref script should complete: " + state.message);

    const auto result = state.globals.find("nresult");
    const auto buffer = state.globals.find("cbuffer");
    const auto copied = state.globals.find("ccopied");

    expect(result != state.globals.end(), "declared DLL call result should be captured");
    expect(buffer != state.globals.end(), "declared DLL by-ref target should be captured");
    expect(copied != state.globals.end(), "copied by-ref value should be captured");

    if (result != state.globals.end()) {
        expect(copperfin::runtime::format_value(result->second) != "0", "lstrcpyA should return a non-null destination pointer");
    }
    if (buffer != state.globals.end()) {
        expect(copperfin::runtime::format_value(buffer->second) == "Copperfin", "STRING @ arguments should write back the mutated buffer");
    }
    if (copied != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied->second) == "Copperfin", "subsequent reads should observe the by-ref writeback");
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("lstrcpyA") != std::string::npos;
    });
    expect(declare_event != state.events.end(), "declared DLL by-ref script should emit the DECLARE event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_explicit_relative_child_path() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_relative";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    expect(!fixture_source.empty(), "#3921: test executable path should be available");
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3921: explicit-relative DECLARE fixture should copy beside the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_relative.prg";
    write_text(
        main_path,
        "DECLARE INTEGER CopperfinDeclaredDllFixtureValue() IN 'native/" +
            fixture_name.string() + "'\n"
        "nValue = CopperfinDeclaredDllFixtureValue()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3921: explicit-relative DECLARE script should complete: " + state.message);

    const auto value = state.globals.find("nvalue");
    expect(value != state.globals.end(), "#3921: explicit-relative DECLARE result should be captured");
    if (value != state.globals.end()) {
        expect(copperfin::runtime::format_value(value->second) == "3921",
               "#3921: explicit-relative DECLARE should invoke the child DLL");
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllFixtureValue") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3921: explicit-relative DECLARE should emit the stable machine event");

    const std::string missing_library_name = "copperfin_missing_declared_dll_3921.dll";
    const fs::path missing_path = temp_root / "declared_dll_missing.prg";
    write_text(
        missing_path,
        "DECLARE INTEGER MissingExport() IN '" + missing_library_name + "'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession missing_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(missing_path.string(), temp_root.string()));
    const auto missing_state = missing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!missing_state.completed, "#3921: a missing parentless DLL should retain the load failure");
    expect(missing_state.message.find(missing_library_name) != std::string::npos,
           "#3921: a missing parentless DLL diagnostic should retain its invariant designator");
    expect(missing_state.message.find(temp_root.string()) == std::string::npos,
           "#3921: a missing parentless DLL should not be rewritten under the PRG working directory");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_double_arguments_follow_x64_abi() {
#if defined(_WIN64)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_double_abi";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::create_directories(fixture_copy.parent_path());
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3895: typed x64 DECLARE fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_double_abi.prg";
    write_text(
        main_path,
        "DECLARE DOUBLE CopperfinDeclaredDllFraction IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE STRING CopperfinDeclaredDllText IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllInt64 IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllInt64BeyondDouble IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllInt64Echo IN 'native/" + fixture_name.string() + "' INTEGER64 value\n"
        "DECLARE LONG CopperfinDeclaredDllInt64ByRef IN 'native/" + fixture_name.string() + "' INTEGER64 @ value\n"
        "DECLARE DOUBLE CopperfinDeclaredDllOneSlot IN 'native/" + fixture_name.string() + "' DOUBLE first\n"
        "DECLARE DOUBLE CopperfinDeclaredDllMultiply IN 'native/" + fixture_name.string() + "' DOUBLE left, DOUBLE right\n"
        "DECLARE DOUBLE CopperfinDeclaredDllAffine IN 'native/" + fixture_name.string() + "' DOUBLE left, DOUBLE right\n"
        "DECLARE DOUBLE CopperfinDeclaredDllScale IN 'native/" + fixture_name.string() + "' DOUBLE value, INTEGER factor\n"
        "DECLARE DOUBLE CopperfinDeclaredDllThreeSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third\n"
        "DECLARE DOUBLE CopperfinDeclaredDllFourSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllFiveSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth, DOUBLE fifth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllSixSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth, DOUBLE fifth, INTEGER sixth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllSevenSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth, DOUBLE fifth, INTEGER sixth, DOUBLE seventh\n"
        "DECLARE DOUBLE CopperfinDeclaredDllEightSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth, DOUBLE fifth, INTEGER sixth, DOUBLE seventh, INTEGER eighth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllSplit IN 'native/" + fixture_name.string() + "' DOUBLE value, DOUBLE @ whole\n"
        "DECLARE INTEGER CopperfinDeclaredDllDecrement IN 'native/" + fixture_name.string() + "' INTEGER @ value\n"
        "nWhole = 0\n"
        "nCounter = 0\n"
        "nConstant = CopperfinDeclaredDllFraction()\n"
        "cText = CopperfinDeclaredDllText()\n"
        "nInt64 = CopperfinDeclaredDllInt64()\n"
        "nExactInt64 = CopperfinDeclaredDllInt64BeyondDouble()\n"
        "nEchoInt64 = CopperfinDeclaredDllInt64Echo(nExactInt64)\n"
        "nByRefInt64 = 0\n"
        "nByRefResult = CopperfinDeclaredDllInt64ByRef(@nByRefInt64)\n"
        "nOneSlot = CopperfinDeclaredDllOneSlot(1.0)\n"
        "nProduct = CopperfinDeclaredDllMultiply(2.5, 4.0)\n"
        "nAffine = CopperfinDeclaredDllAffine(2.5, 4.0)\n"
        "nScaled = CopperfinDeclaredDllScale(1.5, 8)\n"
        "nThreeSlots = CopperfinDeclaredDllThreeSlots(1.0, 2, 3.0)\n"
        "nFourSlots = CopperfinDeclaredDllFourSlots(1.0, 2, 3.0, 4)\n"
        "nFiveSlots = CopperfinDeclaredDllFiveSlots(1.0, 2, 3.0, 4, 5.0)\n"
        "nSixSlots = CopperfinDeclaredDllSixSlots(1.0, 2, 3.0, 4, 5.0, 6)\n"
        "nSevenSlots = CopperfinDeclaredDllSevenSlots(1.0, 2, 3.0, 4, 5.0, 6, 7.0)\n"
        "nEightSlots = CopperfinDeclaredDllEightSlots(1.0, 2, 3.0, 4, 5.0, 6, 7.0, 8)\n"
        "nFraction = CopperfinDeclaredDllSplit(3.75, @nWhole)\n"
        "nDecremented = CopperfinDeclaredDllDecrement(@nCounter)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "declared DLL double-argument script should complete: " + state.message);

    const auto constant = state.globals.find("nconstant");
    const auto text = state.globals.find("ctext");
    const auto int64_value = state.globals.find("nint64");
    const auto exact_int64 = state.globals.find("nexactint64");
    const auto echo_int64 = state.globals.find("nechoint64");
    const auto byref_int64 = state.globals.find("nbyrefint64");
    const auto byref_result = state.globals.find("nbyrefresult");
    const auto one_slot = state.globals.find("noneslot");
    const auto product = state.globals.find("nproduct");
    const auto affine = state.globals.find("naffine");
    const auto scaled = state.globals.find("nscaled");
    const auto three_slots = state.globals.find("nthreeslots");
    const auto four_slots = state.globals.find("nfourslots");
    const auto five_slots = state.globals.find("nfiveslots");
    const auto six_slots = state.globals.find("nsixslots");
    const auto seven_slots = state.globals.find("nsevenslots");
    const auto eight_slots = state.globals.find("neightslots");
    const auto fraction = state.globals.find("nfraction");
    const auto whole = state.globals.find("nwhole");
    const auto decremented = state.globals.find("ndecremented");
    const auto counter = state.globals.find("ncounter");
    expect(constant != state.globals.end(), "zero-argument DOUBLE fixture result should be captured");
    expect(text != state.globals.end(), "pointer-shaped STRING fixture result should be captured");
    expect(int64_value != state.globals.end(), "signed 64-bit fixture result should be captured");
    expect(exact_int64 != state.globals.end(), "64-bit result beyond binary64 precision should be captured");
    expect(echo_int64 != state.globals.end(), "64-bit argument beyond binary64 precision should be captured");
    expect(byref_int64 != state.globals.end(), "64-bit by-reference output should be captured");
    expect(byref_result != state.globals.end(), "64-bit by-reference return should be captured");
    expect(one_slot != state.globals.end(), "one-slot fixture result should be captured");
    expect(product != state.globals.end(), "two-DOUBLE fixture result should be captured");
    expect(affine != state.globals.end(), "ordered DOUBLE fixture result should be captured");
    expect(scaled != state.globals.end(), "mixed DOUBLE/INTEGER fixture result should be captured");
    expect(three_slots != state.globals.end(), "three-slot fixture result should be captured");
    expect(four_slots != state.globals.end(), "four-slot fixture result should be captured");
    expect(five_slots != state.globals.end(), "five-slot fixture result should be captured");
    expect(six_slots != state.globals.end(), "six-slot fixture result should be captured");
    expect(seven_slots != state.globals.end(), "seven-slot fixture result should be captured");
    expect(eight_slots != state.globals.end(), "eight-slot fixture result should be captured");
    expect(fraction != state.globals.end(), "DOUBLE @ fixture fractional result should be captured");
    expect(whole != state.globals.end(), "DOUBLE @ fixture output should be captured");
    expect(decremented != state.globals.end(), "signed 32-bit return should be captured");
    expect(counter != state.globals.end(), "INTEGER @ output should be captured");
    if (constant != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(constant->second);
        expect(actual == "0.625", "zero-argument DOUBLE return should preserve fractional precision; actual=" + actual);
    }
    if (text != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(text->second);
        expect(actual == "copperfin", "pointer-shaped STRING returns should preserve the native address; actual=" + actual);
    }
    if (int64_value != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(int64_value->second);
        expect(actual == "-4294967297", "signed 64-bit native returns should retain their sign and width; actual=" + actual);
    }
    if (exact_int64 != state.globals.end()) {
        expect(exact_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
                   copperfin::runtime::format_value(exact_int64->second) == "9007199254740993",
               "64-bit native returns should remain exact beyond binary64 precision");
    }
    if (echo_int64 != state.globals.end()) {
        expect(echo_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
                   copperfin::runtime::format_value(echo_int64->second) == "9007199254740993",
               "64-bit native arguments should remain exact beyond binary64 precision");
    }
    if (byref_int64 != state.globals.end()) {
        expect(byref_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
                   copperfin::runtime::format_value(byref_int64->second) == "9007199254740993",
               "64-bit native by-reference writeback should remain exact beyond binary64 precision");
    }
    if (byref_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(byref_result->second) == "1",
               "64-bit native by-reference fixture should return its success sentinel");
    }
    if (one_slot != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(one_slot->second);
        expect(actual == "1", "one-argument typed dispatch should preserve its DOUBLE slot; actual=" + actual);
    }
    if (product != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(product->second);
        expect(actual == "10", "two DOUBLE arguments should reach XMM0 and XMM1; actual=" + actual);
    }
    if (affine != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(affine->second);
        expect(actual == "29", "DOUBLE arguments should retain their declared positions; actual=" + actual);
    }
    if (scaled != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(scaled->second);
        expect(actual == "12", "mixed DOUBLE/INTEGER arguments should preserve x64 register classes; actual=" + actual);
    }
    const auto expect_slot_sum = [&](const auto iterator, const std::string &expected, const std::string &label) {
        if (iterator == state.globals.end()) {
            return;
        }
        const std::string actual = copperfin::runtime::format_value(iterator->second);
        expect(actual == expected, label + "; actual=" + actual);
    };
    expect_slot_sum(three_slots, "6", "three-argument typed dispatch should preserve all slots");
    expect_slot_sum(four_slots, "10", "four-argument typed dispatch should preserve all register slots");
    expect_slot_sum(five_slots, "15", "five-argument typed dispatch should cross into stack slots");
    expect_slot_sum(six_slots, "21", "six-argument typed dispatch should preserve register and stack slots");
    expect_slot_sum(seven_slots, "28", "seven-argument typed dispatch should preserve register and stack slots");
    if (eight_slots != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(eight_slots->second);
        expect(actual == "36", "all eight x64 argument slots should preserve declared register and stack classes; actual=" + actual);
    }
    if (fraction != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(fraction->second);
        expect(actual == "0.75", "DOUBLE return values should preserve fractional precision; actual=" + actual);
    }
    if (whole != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(whole->second);
        expect(actual == "3", "DOUBLE @ arguments should write native changes back to the caller; actual=" + actual);
    }
    if (decremented != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(decremented->second);
        expect(actual == "-1", "signed 32-bit native returns should remain negative; actual=" + actual);
    }
    if (counter != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(counter->second);
        expect(actual == "-1", "INTEGER @ arguments should use signed 32-bit backing storage; actual=" + actual);
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllEightSlots") != std::string::npos;
    });
    expect(declare_event != state.events.end(), "typed native declarations should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_long_uses_vfp_32_bit_width() {
#if defined(_WIN64)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_long_width";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3932: controlled LONG-width fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_long_width.prg";
    write_text(
        main_path,
        "DECLARE LONG CopperfinDeclaredDllLongWidth IN 'native/" + fixture_name.string() +
            "' DOUBLE multiplier, LONG input, LONG @ output\n"
        "nOutput = 42\n"
        "nReturn = CopperfinDeclaredDllLongWidth(1.5, 42, @nOutput)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3932: LONG-width fixture should complete: " + state.message);

    const auto returned = state.globals.find("nreturn");
    const auto output = state.globals.find("noutput");
    expect(returned != state.globals.end(), "#3932: signed LONG return should be captured");
    expect(output != state.globals.end(), "#3932: LONG by-reference output should be captured");
    if (returned != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(returned->second);
        expect(actual == "-2147483000", "#3932: LONG returns should narrow as signed 32-bit values; actual=" + actual);
    }
    if (output != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(output->second);
        expect(actual == "-123456789", "#3932: LONG @ storage should write back as signed 32-bit; actual=" + actual);
    }

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_single_uses_vfp_32_bit_float_width() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_single_width";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3933: controlled SINGLE fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_single_width.prg";
    write_text(
        main_path,
        "DECLARE SINGLE CopperfinDeclaredDllSingleConstant IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE SINGLE CopperfinDeclaredDllSingleMixed IN 'native/" + fixture_name.string() +
            "' INTEGER first, SINGLE second, DOUBLE third, SINGLE fourth, SINGLE fifth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllSingleToDouble IN 'native/" + fixture_name.string() +
            "' SINGLE value, DOUBLE multiplier\n"
        "DECLARE SINGLE CopperfinDeclaredDllSingleSlots IN 'native/" + fixture_name.string() +
            "' SINGLE first, SINGLE second, SINGLE third, SINGLE fourth, SINGLE fifth, SINGLE sixth, SINGLE seventh, SINGLE eighth\n"
        "DECLARE SINGLE CopperfinDeclaredDllSingleSplit IN 'native/" + fixture_name.string() +
            "' SINGLE value, SINGLE @ whole\n"
        "nWhole = 0\n"
        "nConstant = CopperfinDeclaredDllSingleConstant()\n"
        "nMixed = CopperfinDeclaredDllSingleMixed(1, 2.25, 3.5, 4.25, 5.0)\n"
        "nDouble = CopperfinDeclaredDllSingleToDouble(2.25, 1.5)\n"
        "nSlots = CopperfinDeclaredDllSingleSlots(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)\n"
        "nFraction = CopperfinDeclaredDllSingleSplit(3.75, @nWhole)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3933: SINGLE fixture should complete: " + state.message);

    const auto expect_numeric = [&](const std::string &name,
                                    const std::string &expected,
                                    const std::string &label)
    {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), label + " should be captured");
        if (value != state.globals.end())
        {
            const std::string actual = copperfin::runtime::format_value(value->second);
            expect(actual == expected, label + "; actual=" + actual);
        }
    };
    expect_numeric("nconstant", "0.625", "#3933: zero-argument SINGLE return");
    expect_numeric("nmixed", "16", "#3933: mixed INTEGER/SINGLE/DOUBLE register and stack call");
    expect_numeric("ndouble", "3.375", "#3933: DOUBLE return through a SINGLE signature");
    expect_numeric("nslots", "36", "#3933: eight-position SINGLE call");
    expect_numeric("nfraction", "0.75", "#3933: SINGLE return with by-reference input");
    expect_numeric("nwhole", "3", "#3933: SINGLE @ writeback");

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllSingleSlots") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3933: SINGLE declarations should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_win32_uses_typed_stdcall_slots() {
#if defined(_WIN32) && !defined(_WIN64)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_win32_typed";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3940: controlled Win32 typed fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_win32_typed.prg";
    write_text(
        main_path,
        "DECLARE DOUBLE CopperfinDeclaredDllX86Mixed IN 'native/" + fixture_name.string() +
            "' AS X86MixedAlias LONG first, DOUBLE second, INTEGER64 third, INTEGER fourth\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllX86Int64 IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllX86Int64BeyondDouble IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllX86Int64Echo IN 'native/" + fixture_name.string() + "' INTEGER64 value\n"
        "DECLARE LONG CopperfinDeclaredDllX86Int64ByRef IN 'native/" + fixture_name.string() + "' INTEGER64 @ value\n"
        "DECLARE DOUBLE CopperfinDeclaredDllX86Split IN 'native/" + fixture_name.string() +
            "' DOUBLE value, DOUBLE @ whole\n"
        "DECLARE STRING CopperfinDeclaredDllX86Text IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE LONG CopperfinDeclaredDllX86Eight IN 'native/" + fixture_name.string() +
            "' INTEGER first, INTEGER second, INTEGER third, INTEGER fourth, INTEGER fifth, INTEGER sixth, INTEGER seventh, INTEGER eighth\n"
        "DECLARE LONG CopperfinDeclaredDllX86NumericByRef IN 'native/" + fixture_name.string() +
            "' LONG @ longValue, INTEGER64 @ integer64Value\n"
        "nWhole = 0\n"
        "nLongOut = 0\n"
        "nInteger64Out = 0\n"
        "nMixed = X86MixedAlias(1, 2.5, 4294967297, 4)\n"
        "nInt64 = CopperfinDeclaredDllX86Int64()\n"
        "nExactInt64 = CopperfinDeclaredDllX86Int64BeyondDouble()\n"
        "nEchoInt64 = CopperfinDeclaredDllX86Int64Echo(nExactInt64)\n"
        "nByRefInt64 = 0\n"
        "nByRefResult = CopperfinDeclaredDllX86Int64ByRef(@nByRefInt64)\n"
        "nFraction = CopperfinDeclaredDllX86Split(3.75, @nWhole)\n"
        "cText = CopperfinDeclaredDllX86Text()\n"
        "nEight = CopperfinDeclaredDllX86Eight(1, 2, 3, 4, 5, 6, 7, 8)\n"
        "nByRefResult = CopperfinDeclaredDllX86NumericByRef(@nLongOut, @nInteger64Out)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3940: Win32 typed fixture should complete: " + state.message);

    const auto expect_value = [&](const std::string &name,
                                  const std::string &expected,
                                  const std::string &label)
    {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), label + " should be captured");
        if (value != state.globals.end())
        {
            const std::string actual = copperfin::runtime::format_value(value->second);
            expect(actual == expected, label + "; actual=" + actual);
        }
    };
    const auto mixed = state.globals.find("nmixed");
    expect(mixed != state.globals.end(),
           "#3940: mixed LONG/DOUBLE/INTEGER64/INTEGER call should be captured");
    if (mixed != state.globals.end())
    {
        expect(mixed->second.kind == copperfin::runtime::PrgValueKind::number &&
                   mixed->second.number_value == 4294967304.5,
               "#3940: mixed LONG/DOUBLE/INTEGER64/INTEGER call should preserve its exact numeric payload; actual=" +
                   std::to_string(mixed->second.number_value));
    }
    expect_value("nint64", "-4294967297", "#3940: signed 64-bit return");
    const auto exact_int64 = state.globals.find("nexactint64");
    expect(exact_int64 != state.globals.end() &&
               exact_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
               copperfin::runtime::format_value(exact_int64->second) == "9007199254740993",
           "#3934: Win32 signed 64-bit return should remain exact beyond binary64 precision");
    const auto echo_int64 = state.globals.find("nechoint64");
    expect(echo_int64 != state.globals.end() &&
               echo_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
               copperfin::runtime::format_value(echo_int64->second) == "9007199254740993",
           "#3934: Win32 signed 64-bit argument should remain exact beyond binary64 precision");
    const auto byref_int64 = state.globals.find("nbyrefint64");
    expect(byref_int64 != state.globals.end() &&
               byref_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
               copperfin::runtime::format_value(byref_int64->second) == "9007199254740993",
           "#3934: Win32 signed 64-bit by-reference writeback should remain exact beyond binary64 precision");
    expect_value("nbyrefresult", "1", "#3934: Win32 signed 64-bit by-reference return");
    expect_value("nfraction", "0.75", "#3940: DOUBLE return with by-reference input");
    expect_value("nwhole", "3", "#3940: DOUBLE @ writeback");
    expect_value("ctext", "copperfin-x86", "#3940: pointer-shaped STRING return");
    expect_value("neight", "36", "#3940: signed LONG return across eight stdcall stack positions");
    expect_value("nbyrefresult", "-7", "#3940: signed LONG return from numeric by-reference call");
    expect_value("nlongout", "-123456789", "#3940: signed LONG @ writeback");
    expect_value("ninteger64out", "-4294967297", "#3940: signed INTEGER64 @ writeback");

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("X86MixedAlias") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3940: Win32 typed declarations should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_win32_resolves_no_underscore_stdcall_export() {
#if defined(_MSC_VER) && defined(_M_IX86)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_win32_name_at_n";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3941: no-underscore stdcall fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_win32_name_at_n.prg";
    write_text(
        main_path,
        "DECLARE LONG CopperfinDeclaredDllNoUnderscore IN 'native/" + fixture_name.string() + "' LONG value\n"
        "nResult = CopperfinDeclaredDllNoUnderscore(41)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3941: Name@N-only stdcall fixture should complete: " + state.message);

    const auto result = state.globals.find("nresult");
    expect(result != state.globals.end(), "#3941: Name@N-only result should be captured");
    if (result != state.globals.end())
    {
        expect(copperfin::runtime::format_value(result->second) == "42",
               "#3941: Name@N-only stdcall export should resolve through the final decorated probe");
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllNoUnderscore") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3941: Name@N-only declaration should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_short_return_uses_signed_16_bit_width() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_short_return";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3938: controlled SHORT-return fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_short_return.prg";
    write_text(
        main_path,
        "DECLARE SHORT CopperfinDeclaredDllShortNegative IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE SHORT CopperfinDeclaredDllShortInternetShape IN 'native/" + fixture_name.string() +
            "' INTEGER @ lpdwFlags, INTEGER dwReserved\n"
        "DECLARE SHORT InternetGetConnectedState IN 'wininet.dll' INTEGER @ lpdwFlags, INTEGER dwReserved\n"
        "nFlags = 0\n"
        "nRealFlags = 0\n"
        "nNegative = CopperfinDeclaredDllShortNegative()\n"
        "nConnected = CopperfinDeclaredDllShortInternetShape(@nFlags, 0)\n"
        "nRealConnected = InternetGetConnectedState(@nRealFlags, 0)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3938: SHORT-return fixture should complete: " + state.message);

    const auto negative = state.globals.find("nnegative");
    const auto connected = state.globals.find("nconnected");
    const auto flags = state.globals.find("nflags");
    const auto real_connected = state.globals.find("nrealconnected");
    const auto real_flags = state.globals.find("nrealflags");
    expect(negative != state.globals.end(), "#3938: negative SHORT return should be captured");
    expect(connected != state.globals.end(), "#3938: WinInet-shaped SHORT return should be captured");
    expect(flags != state.globals.end(), "#3938: WinInet-shaped INTEGER @ output should be captured");
    expect(real_connected != state.globals.end(), "#3938: shipped WinInet SHORT return should be captured");
    expect(real_flags != state.globals.end(), "#3938: shipped WinInet INTEGER @ output should be captured");
    if (negative != state.globals.end())
    {
        expect(copperfin::runtime::format_value(negative->second) == "-12345",
               "#3938: SHORT returns should preserve signed 16-bit values");
    }
    if (connected != state.globals.end())
    {
        expect(copperfin::runtime::format_value(connected->second) == "-1",
               "#3938: Microsoft-shipped WinInet declaration shape should preserve a negative SHORT return");
    }
    if (flags != state.globals.end())
    {
        expect(copperfin::runtime::format_value(flags->second) == "305419896",
               "#3938: SHORT return selection should preserve adjacent INTEGER @ writeback");
    }
    if (real_connected != state.globals.end())
    {
        const std::string actual = copperfin::runtime::format_value(real_connected->second);
        expect(actual == "0" || actual == "1",
               "#3938: shipped InternetGetConnectedState SHORT declaration should return a logical status; actual=" + actual);
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllShortInternetShape") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3938: SHORT declarations should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_short_parameter_is_rejected_and_localized() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_short_parameter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "declared_dll_short_parameter.prg";
    write_text(
        main_path,
        "DECLARE SHORT InvalidShortParameter IN 'kernel32.dll' SHORT invalid\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#3938: help-invalid SHORT parameters should reject the declaration");
    expect(state.message.find("[!! ") == 0U &&
               state.message.find("SHORT") != std::string::npos &&
               state.message.find("parameter type") == std::string::npos,
           "#3938: SHORT parameter rejection should pseudo-localize prose and preserve the invariant type token");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_win32api_search_and_ansi_fallback() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_win32api";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "declared_dll_win32api.prg";
    write_text(
        main_path,
        "DECLARE INTEGER GetCurrentProcessId IN WIN32API\n"
        "DECLARE INTEGER GetSystemMetrics IN WIN32API INTEGER index\n"
        "DECLARE INTEGER GetUserName IN WIN32API STRING @ buffer, INTEGER @ size\n"
        "cUserBuffer = SPACE(256)\n"
        "nUserSize = 256\n"
        "nProcessId = GetCurrentProcessId()\n"
        "nScreenWidth = GetSystemMetrics(0)\n"
        "nUserResult = GetUserName(@cUserBuffer, @nUserSize)\n"
        "cUserName = cUserBuffer\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3939: bare WIN32API declarations should complete: " + state.message);

    const auto process_id = state.globals.find("nprocessid");
    const auto screen_width = state.globals.find("nscreenwidth");
    const auto user_result = state.globals.find("nuserresult");
    const auto user_size = state.globals.find("nusersize");
    const auto user_name = state.globals.find("cusername");
    expect(process_id != state.globals.end(), "#3939: Kernel32 result should be captured");
    expect(screen_width != state.globals.end(), "#3939: User32 result should be captured");
    expect(user_result != state.globals.end(), "#3939: Advapi32 ANSI-fallback result should be captured");
    expect(user_size != state.globals.end(), "#3939: GetUserName size writeback should be captured");
    expect(user_name != state.globals.end(), "#3939: GetUserName buffer writeback should be captured");
    if (process_id != state.globals.end())
    {
        expect(copperfin::runtime::format_value(process_id->second) != "0",
               "#3939: GetCurrentProcessId should return a nonzero process id");
    }
    if (user_result != state.globals.end())
    {
        expect(copperfin::runtime::format_value(user_result->second) == "1",
               "#3939: GetUserName should resolve through GetUserNameA");
    }
    if (user_size != state.globals.end())
    {
        expect(copperfin::runtime::format_value(user_size->second) != "256",
               "#3939: GetUserNameA should update its size argument");
    }
    if (user_name != state.globals.end())
    {
        expect(!copperfin::runtime::format_value(user_name->second).empty(),
               "#3939: GetUserNameA should write back a nonempty user name");
    }

    const auto has_declare_event = [&](const std::string &function_name)
    {
        return std::any_of(state.events.begin(), state.events.end(), [&](const auto &event)
        {
            return event.category == "runtime.declare_dll" &&
                   event.detail == function_name + " IN WIN32API";
        });
    };
    expect(has_declare_event("GetCurrentProcessId"),
           "#3939: Kernel32 declaration event should preserve the WIN32API designator");
    expect(has_declare_event("GetSystemMetrics"),
           "#3939: User32 declaration event should preserve the WIN32API designator");
    expect(has_declare_event("GetUserName"),
           "#3939: ANSI-fallback declaration event should preserve the source export name");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_win32api_missing_symbol_localizes() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_win32api_missing";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "declared_dll_win32api_missing.prg";
    write_text(
        main_path,
        "DECLARE INTEGER CopperfinMissingWin32ApiSymbol IN WIN32API\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#3939: a missing WIN32API symbol should fail the declaration");
    expect(state.message.find("[!! ") == 0U &&
               state.message.find("CopperfinMissingWin32ApiSymbol") != std::string::npos &&
               state.message.find("WIN32API") != std::string::npos &&
               state.message.find("function 'CopperfinMissingWin32ApiSymbol' not found") == std::string::npos,
           "#3939: WIN32API failure should pseudo-localize prose and preserve machine identifiers");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_module_ownership_and_failed_redeclare_rollback() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_ownership";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(declared_dll_fixture_source_path(), fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3939: module-ownership fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_ownership.prg";
    write_text(
        main_path,
        "PUBLIC nDeclareErrors\n"
        "nDeclareErrors = 0\n"
        "DECLARE INTEGER CopperfinDeclaredDllFixtureValue IN 'native/" + fixture_name.string() +
            "' AS FirstAlias\n"
        "DECLARE INTEGER CopperfinDeclaredDllFixtureValue IN 'native/" + fixture_name.string() +
            "' AS SecondAlias\n"
        "ON ERROR DO HandleDeclareError\n"
        "DECLARE INTEGER CopperfinMissingDeclaredDllExport IN 'native/" + fixture_name.string() +
            "' AS FirstAlias\n"
        "ON ERROR\n"
        "nFirstValue = FirstAlias()\n"
        "nSecondValue = SecondAlias()\n"
        "RETURN\n"
        "PROCEDURE HandleDeclareError\n"
        "nDeclareErrors = nDeclareErrors + 1\n"
        "RETURN\n"
        "ENDPROC\n");

    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3939: failed redeclaration should roll back to the old binding: " + state.message);

        const auto first_value = state.globals.find("nfirstvalue");
        const auto second_value = state.globals.find("nsecondvalue");
        const auto error_count = state.globals.find("ndeclareerrors");
        expect(first_value != state.globals.end() &&
                   copperfin::runtime::format_value(first_value->second) == "3921",
               "#3939: failed redeclaration should retain the first alias binding");
        expect(second_value != state.globals.end() &&
                   copperfin::runtime::format_value(second_value->second) == "3921",
               "#3939: a second alias should retain its independently owned module reference");
        expect(error_count != state.globals.end() &&
                   copperfin::runtime::format_value(error_count->second) == "1",
               "#3939: failed redeclaration should dispatch exactly one recoverable error");
        expect(GetModuleHandleW(fixture_copy.wstring().c_str()) != nullptr,
               "#3939: the owned fixture module should remain loaded while aliases are callable");
    }

    expect(GetModuleHandleW(fixture_copy.wstring().c_str()) == nullptr,
           "#3939: session cleanup should release every owned module reference");
    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_explicit_ansi_fallback_and_exact_precedence() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_ansi_fallback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3942: explicit ANSI-fallback fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_ansi_fallback.prg";
    write_text(
        main_path,
        "DECLARE LONG CopperfinDeclaredDllAnsiOnly IN '" + fixture_name.string() +
            "' AS ParentlessAnsiAlias LONG value\n"
        "DECLARE LONG CopperfinDeclaredDllModulePath IN '" + fixture_name.string() +
            "' AS ParentlessModulePathAlias STRING @ buffer, LONG capacity\n"
        "DECLARE LONG CopperfinDeclaredDllAnsiOnly IN 'native/" + fixture_name.string() +
            "' AS RelativeAnsiAlias LONG value\n"
        "DECLARE LONG CopperfinDeclaredDllAnsiCdeclOnly IN 'native/" + fixture_name.string() +
            "' AS RelativeCdeclAnsiAlias\n"
        "DECLARE LONG CopperfinDeclaredDllModulePath IN 'native/" + fixture_name.string() +
            "' AS RelativeModulePathAlias STRING @ buffer, LONG capacity\n"
        "DECLARE LONG CopperfinDeclaredDllExactPrecedence IN 'native/" + fixture_name.string() +
            "' AS ExactPrecedenceAlias LONG value\n"
        "DECLARE INTEGER GetSystemDirectory IN 'kernel32.dll' STRING @ buffer, INTEGER size\n"
        "cParentlessModulePath = SPACE(32768)\n"
        "cRelativeModulePath = SPACE(32768)\n"
        "cSystemDirectory = SPACE(32768)\n"
        "nParentlessAnsi = ParentlessAnsiAlias(3)\n"
        "nParentlessModulePathLength = ParentlessModulePathAlias(@cParentlessModulePath, 32768)\n"
        "nRelativeAnsi = RelativeAnsiAlias(2)\n"
        "nRelativeCdeclAnsi = RelativeCdeclAnsiAlias()\n"
        "nRelativeModulePathLength = RelativeModulePathAlias(@cRelativeModulePath, 32768)\n"
        "nExact = ExactPrecedenceAlias(41)\n"
        "nSystemDirectoryLength = GetSystemDirectory(@cSystemDirectory, 32768)\n"
        "RETURN\n");

    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3942: explicit ANSI-fallback declarations should complete: " + state.message);

        const auto relative_ansi = state.globals.find("nrelativeansi");
        const auto relative_cdecl_ansi = state.globals.find("nrelativecdeclansi");
        const auto parentless_ansi = state.globals.find("nparentlessansi");
        const auto parentless_module_path = state.globals.find("cparentlessmodulepath");
        const auto relative_module_path = state.globals.find("crelativemodulepath");
        const auto parentless_module_path_length = state.globals.find("nparentlessmodulepathlength");
        const auto relative_module_path_length = state.globals.find("nrelativemodulepathlength");
        const auto exact = state.globals.find("nexact");
        const auto system_directory = state.globals.find("csystemdirectory");
        const auto system_directory_length = state.globals.find("nsystemdirectorylength");
        expect(relative_ansi != state.globals.end() &&
                   copperfin::runtime::format_value(relative_ansi->second) == "3944",
               "#3942: quoted relative lookup should bind the A-suffixed export");
        expect(relative_cdecl_ansi != state.globals.end() &&
                   copperfin::runtime::format_value(relative_cdecl_ansi->second) == "4002",
               "#3942: quoted relative lookup should bind the cdecl A-suffixed export");
        expect(parentless_ansi != state.globals.end() &&
                   copperfin::runtime::format_value(parentless_ansi->second) == "3945",
               "#3942: parentless loader lookup should bind the A-suffixed export");
        expect(parentless_module_path != state.globals.end() &&
                   fs::equivalent(
                       fs::path(copperfin::runtime::format_value(parentless_module_path->second)),
                       fixture_source,
                       ignored) &&
                   !ignored,
               "#3942: parentless lookup should bind the fixture beside the test executable");
        ignored.clear();
        expect(relative_module_path != state.globals.end() &&
                   fs::equivalent(
                       fs::path(copperfin::runtime::format_value(relative_module_path->second)),
                       fixture_copy,
                       ignored) &&
                   !ignored,
               "#3942: explicit relative lookup should bind the copied fixture path");
        ignored.clear();
        expect(parentless_module_path_length != state.globals.end() &&
                   parentless_module_path_length->second.kind == copperfin::runtime::PrgValueKind::number &&
                   parentless_module_path_length->second.number_value > 0.0,
               "#3942: parentless module-path fallback should return a positive path length");
        expect(relative_module_path_length != state.globals.end() &&
                   relative_module_path_length->second.kind == copperfin::runtime::PrgValueKind::number &&
                   relative_module_path_length->second.number_value > 0.0,
               "#3942: relative module-path fallback should return a positive path length");
        expect(exact != state.globals.end() &&
                   copperfin::runtime::format_value(exact->second) == "42",
               "#3942: exact export should take precedence over its A-suffixed sibling");
        expect(system_directory != state.globals.end() &&
                   !copperfin::runtime::format_value(system_directory->second).empty(),
               "#3942: shipped GetSystemDirectory declaration should write back a path");
        expect(system_directory_length != state.globals.end() &&
                   system_directory_length->second.kind == copperfin::runtime::PrgValueKind::number &&
                   system_directory_length->second.number_value > 0.0,
               "#3942: shipped GetSystemDirectory declaration should return a positive path length");

        const auto has_declare_event = [&](const std::string &detail)
        {
            return std::any_of(state.events.begin(), state.events.end(), [&](const auto &event)
            {
                return event.category == "runtime.declare_dll" && event.detail == detail;
            });
        };
        expect(has_declare_event("RelativeAnsiAlias IN native/" + fixture_name.string()),
               "#3942: relative fallback event should preserve source alias and library designator");
        expect(has_declare_event("ParentlessAnsiAlias IN " + fixture_name.string()),
               "#3942: parentless fallback event should preserve source alias and library designator");
        expect(has_declare_event("ExactPrecedenceAlias IN native/" + fixture_name.string()),
               "#3942: exact-precedence event should preserve source export identity");
        expect(has_declare_event("GetSystemDirectory IN kernel32.dll"),
               "#3942: shipped explicit-DLL event should preserve its unsuffixed source function name");
        expect(GetModuleHandleW(fixture_copy.wstring().c_str()) != nullptr,
               "#3942: relative fixture module should remain loaded during the session");
        expect(GetModuleHandleW(fixture_source.wstring().c_str()) != nullptr,
               "#3942: parentless fixture module should remain loaded during the session");
    }

    expect(GetModuleHandleW(fixture_copy.wstring().c_str()) == nullptr,
           "#3942: explicit fallback module references should unload with the session");
    expect(GetModuleHandleW(fixture_source.wstring().c_str()) == nullptr,
           "#3942: parentless fallback module references should unload with the session");
    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_argument_count_is_validated_before_native_entry() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_arity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(declared_dll_fixture_source_path(), fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3946: controlled arity fixture should copy under the PRG working directory");

    const std::string module = "native/" + fixture_name.string();
    const std::string counter_declarations =
        "DECLARE LONG CopperfinDeclaredDllArityReset IN '" + module + "' AS ArityReset\n"
        "DECLARE LONG CopperfinDeclaredDllArityCount IN '" + module + "' AS ArityCount\n";
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);

    const fs::path baseline_path = temp_root / "arity_baseline.prg";
    write_text(
        baseline_path,
        counter_declarations +
        "DECLARE LONG CopperfinDeclaredDllArityOne IN '" + module + "' AS ArityTarget LONG value\n"
        "nReset = ArityReset()\n"
        "nValidResult = ArityTarget(7)\n"
        "nNativeEntries = ArityCount()\n"
        "RETURN\n");
    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(baseline_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3946: arity fixture baseline should complete: " + state.message);
        const auto valid_result = state.globals.find("nvalidresult");
        const auto native_entries = state.globals.find("nnativeentries");
        expect(valid_result != state.globals.end() &&
                   copperfin::runtime::format_value(valid_result->second) == "7",
               "#3946: valid one-argument stdcall should enter the controlled fixture");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "1",
               "#3946: controlled fixture counter should observe the valid native entry");
    }

    const fs::path handler_argument_path = temp_root / "arity_handler_argument.prg";
    write_text(
        handler_argument_path,
        counter_declarations +
        "DECLARE LONG CopperfinDeclaredDllArityOne IN '" + module + "' AS ArityTarget LONG value\n"
        "nReset = ArityReset()\n"
        "ON ERROR DO HandleOuterError WITH ArityTarget()\n"
        "DO CopperfinMissingArityHandlerTarget\n"
        "ON ERROR\n"
        "nOuterCode = ERROR()\n"
        "nNativeEntries = ArityCount()\n"
        "RETURN\n"
        "PROCEDURE HandleOuterError\n"
        "RETURN\n"
        "ENDPROC\n");
    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(handler_argument_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(!state.completed && state.reason == copperfin::runtime::DebugPauseReason::error,
               "#3946: an arity fault in ON ERROR arguments should pause safely");
        expect(state.message == "Too few arguments.",
               "#3946: the outer run boundary should not wrap an arity compatibility fault");

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "#3946: the runtime should remain resumable after an outer-boundary arity fault: " + state.message);
        const auto outer_code = state.globals.find("noutercode");
        const auto native_entries = state.globals.find("nnativeentries");
        expect(outer_code != state.globals.end() &&
                   copperfin::runtime::format_value(outer_code->second) == "1229",
               "#3946: the outer run boundary should preserve VFP error 1229");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "0",
               "#3946: handler-argument arity faults must not enter the native target");
    }

    struct ArityCase {
        std::string label;
        std::string export_name;
        std::string declaration_suffix;
        std::string invocation;
        int expected_error;
        std::string expected_message;
        bool pseudo_locale = false;
    };

    const std::vector<ArityCase> cases{
        {"zero_too_many", "CopperfinDeclaredDllArityZero", "", "ArityTarget(1)", 1230, "Too many arguments."},
        {"stdcall_one_too_few", "CopperfinDeclaredDllArityOne", " LONG value", "ArityTarget()", 1229, "Too few arguments."},
        {"stdcall_one_too_many", "CopperfinDeclaredDllArityOne", " LONG value", "ArityTarget(1, 2)", 1230, "Too many arguments."},
        {"cdecl_one_too_few", "CopperfinDeclaredDllArityCdeclOne", " LONG value", "ArityTarget()", 1229, "Too few arguments.", true},
        {"mixed_byref_too_few", "CopperfinDeclaredDllArityMixed",
         " LONG first, DOUBLE second, INTEGER64 @ third, SINGLE fourth",
         "ArityTarget(1, 2.0, @nWide)", 1229, "Too few arguments."},
        {"eight_too_few", "CopperfinDeclaredDllArityEight",
         " LONG first, LONG second, LONG third, LONG fourth, LONG fifth, LONG sixth, LONG seventh, LONG eighth",
         "ArityTarget(1, 2, 3, 4, 5, 6, 7)", 1229, "Too few arguments."},
        {"eight_too_many", "CopperfinDeclaredDllArityEight",
         " LONG first, LONG second, LONG third, LONG fourth, LONG fifth, LONG sixth, LONG seventh, LONG eighth",
         "ArityTarget(1, 2, 3, 4, 5, 6, 7, 8, 9)", 1230, "Too many arguments."},
        {"native_limit", "CopperfinDeclaredDllArityCdeclOne",
         " LONG first, LONG second, LONG third, LONG fourth, LONG fifth, LONG sixth, LONG seventh, LONG eighth, LONG ninth",
         "ArityTarget(1, 2, 3, 4, 5, 6, 7, 8, 9)", 1230,
         "Native DLL call has 9 arguments; the maximum is 8", true}
    };

    for (const ArityCase &arity_case : cases) {
        set_env_value("COPPERFIN_LOCALE", arity_case.pseudo_locale ? "qps-ploc" : "en-US", true);
        const fs::path script_path = temp_root / (arity_case.label + ".prg");
        write_text(
            script_path,
            "PUBLIC nCapturedCode\n"
            "PUBLIC cCapturedMessage\n"
            "nCapturedCode = 0\n"
            "cCapturedMessage = ''\n" +
            counter_declarations +
            "DECLARE LONG " + arity_case.export_name + " IN '" + module +
                "' AS ArityTarget" + arity_case.declaration_suffix + "\n"
            "nWide = 3\n"
            "nReset = ArityReset()\n"
            "ON ERROR DO HandleArityError\n"
            "nUnexpected = " + arity_case.invocation + "\n"
            "ON ERROR\n"
            "nNativeEntries = ArityCount()\n"
            "RETURN\n"
            "PROCEDURE HandleArityError\n"
            "nCapturedCode = ERROR()\n"
            "cCapturedMessage = MESSAGE()\n"
            "RETURN\n"
            "ENDPROC\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3946: recoverable " + arity_case.label + " mismatch should complete: " + state.message);

        const auto captured_code = state.globals.find("ncapturedcode");
        const auto captured_message = state.globals.find("ccapturedmessage");
        const auto native_entries = state.globals.find("nnativeentries");
        const std::string expected_message = arity_case.pseudo_locale
            ? copperfin::localization::pseudo_localize(arity_case.expected_message)
            : arity_case.expected_message;
        expect(captured_code != state.globals.end() &&
                   copperfin::runtime::format_value(captured_code->second) == std::to_string(arity_case.expected_error),
               "#3946: " + arity_case.label + " should preserve the grounded VFP error identity");
        expect(captured_message != state.globals.end() &&
                   copperfin::runtime::format_value(captured_message->second) == expected_message,
               "#3946: " + arity_case.label + " should preserve the localized arity diagnostic");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "0",
               "#3946: " + arity_case.label + " must not enter the native target");
        expect(std::any_of(state.events.begin(), state.events.end(), [&](const auto &event) {
                   return event.category == "runtime.declare_dll" &&
                          event.detail == "ArityTarget IN " + module;
               }),
               "#3946: " + arity_case.label + " should retain the invariant source alias/path DECLARE event");
        expect(std::any_of(state.events.begin(), state.events.end(), [&](const auto &event) {
                   return event.category == "runtime.error" && event.detail == expected_message;
               }),
               "#3946: " + arity_case.label + " should emit the stable runtime.error category");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
                   return event.category == "runtime.error_handler";
               }),
               "#3946: " + arity_case.label + " should emit the stable runtime.error_handler category");
    }

    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    expect(GetModuleHandleW(fixture_copy.wstring().c_str()) == nullptr,
           "#3946: arity validation sessions should release every controlled module reference");
    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_numeric_byref_requires_callsite_reference() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_numeric_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(declared_dll_fixture_source_path(), fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3944: controlled numeric by-reference fixture should copy under the PRG working directory");

    const std::string module = "native/" + fixture_name.string();
    const std::string declarations =
        "DECLARE LONG CopperfinDeclaredDllNumericByRefReset IN '" + module + "' AS NumericByRefReset\n"
        "DECLARE LONG CopperfinDeclaredDllNumericByRefCount IN '" + module + "' AS NumericByRefCount\n"
        "DECLARE LONG CopperfinDeclaredDllNumericByRefProbe IN '" + module +
            "' AS NumericByRef INTEGER @ integerValue, LONG @ longValue, INTEGER64 @ integer64Value, SINGLE @ singleValue, DOUBLE @ doubleValue\n";
    const std::string initialize_values =
        "nInteger = 1\n"
        "nLong = 2\n"
        "nInteger64 = 3\n"
        "nSingle = 4\n"
        "nDouble = 5\n";

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);

    const fs::path valid_path = temp_root / "numeric_byref_valid.prg";
    write_text(
        valid_path,
        declarations + initialize_values +
        "nReset = NumericByRefReset()\n"
        "nResult = NumericByRef(@nInteger, @nLong, @nInteger64, @nSingle, @nDouble)\n"
        "nNativeEntries = NumericByRefCount()\n"
        "RETURN\n");
    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(valid_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3944: valid numeric by-reference call should complete: " + state.message);
        const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &label) {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end() &&
                       copperfin::runtime::format_value(found->second) == expected,
                   label);
        };
        expect_value("nresult", "3944", "#3944: valid numeric by-reference fixture should return its sentinel");
        expect_value("ninteger", "-11", "#3944: INTEGER @ should retain valid writeback");
        expect_value("nlong", "-22", "#3944: LONG @ should retain valid writeback");
        expect_value("ninteger64", "-4294967297", "#3944: INTEGER64 @ should retain valid writeback");
        expect_value("nsingle", "3.5", "#3944: SINGLE @ should retain valid writeback");
        expect_value("ndouble", "4.25", "#3944: DOUBLE @ should retain valid writeback");
        expect_value("nnativeentries", "1", "#3944: valid numeric by-reference call should enter native code once");

        const HMODULE loaded_module = GetModuleHandleW(fixture_copy.wstring().c_str());
        expect(loaded_module != nullptr, "#3944: controlled numeric by-reference module should be loaded");
        if (loaded_module != nullptr) {
#if defined(_WIN64)
            expect(GetProcAddress(loaded_module, "CopperfinDeclaredDllNumericByRefProbe") != nullptr,
                   "#3944: x64 should expose the unified exact numeric by-reference export");
#else
            expect(GetProcAddress(loaded_module, "CopperfinDeclaredDllNumericByRefProbe") == nullptr,
                   "#3944: Win32 fixture should not hide stdcall decoration behind an exact alias");
            expect(GetProcAddress(loaded_module, "_CopperfinDeclaredDllNumericByRefProbe@20") != nullptr ||
                       GetProcAddress(loaded_module, "CopperfinDeclaredDllNumericByRefProbe@20") != nullptr,
                   "#3944: Win32 should resolve the five pointer slots through a stdcall @20 export");
#endif
        }
    }

    const fs::path stack_path = temp_root / "numeric_byref_stack_frugal.prg";
    write_text(
        stack_path,
        "PUBLIC nHandled\n"
        "nHandled = 0\n" +
        declarations + initialize_values +
        "nReset = NumericByRefReset()\n"
        "nAttempts = 0\n"
        "ON ERROR DO HandleRepeatedNumericByRefError\n"
        "DO WHILE nAttempts < 512\n"
        "nAttempts = nAttempts + 1\n"
        "nUnexpected = NumericByRef(1, @nLong, @nInteger64, @nSingle, @nDouble)\n"
        "ENDDO\n"
        "ON ERROR\n"
        "nNativeEntries = NumericByRefCount()\n"
        "RETURN\n"
        "PROCEDURE HandleRepeatedNumericByRefError\n"
        "nHandled = nHandled + 1\n"
        "RETURN\n"
        "ENDPROC\n");
    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(stack_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "#3944: repeated numeric by-reference rejections should remain stack-frugal: " + state.message);
        const auto attempts = state.globals.find("nattempts");
        const auto handled = state.globals.find("nhandled");
        const auto native_entries = state.globals.find("nnativeentries");
        expect(attempts != state.globals.end() &&
                   copperfin::runtime::format_value(attempts->second) == "512",
               "#3944: repeated rejection loop should complete all iterations");
        expect(handled != state.globals.end() &&
                   copperfin::runtime::format_value(handled->second) == "512",
               "#3944: repeated rejection loop should unwind every recoverable handler frame");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "0",
               "#3944: repeated rejection loop must never enter native code");
    }

    struct MissingReferenceCase {
        std::string label;
        std::string invocation_arguments;
        std::size_t position;
        bool pseudo_locale = false;
    };
    const std::vector<MissingReferenceCase> cases{
        {"integer_literal", "1, @nLong, @nInteger64, @nSingle, @nDouble", 1U},
        {"long_variable", "@nInteger, nLong, @nInteger64, @nSingle, @nDouble", 2U},
        {"integer64_variable", "@nInteger, @nLong, nInteger64, @nSingle, @nDouble", 3U},
        {"single_variable", "@nInteger, @nLong, @nInteger64, nSingle, @nDouble", 4U},
        {"double_variable", "@nInteger, @nLong, @nInteger64, @nSingle, nDouble", 5U, true}
    };

    for (const MissingReferenceCase &missing_case : cases) {
        set_env_value("COPPERFIN_LOCALE", missing_case.pseudo_locale ? "qps-ploc" : "en-US", true);
        const fs::path script_path = temp_root / (missing_case.label + ".prg");
        write_text(
            script_path,
            "PUBLIC nCapturedCode\n"
            "PUBLIC cCapturedMessage\n"
            "nCapturedCode = 0\n"
            "cCapturedMessage = ''\n" +
            declarations + initialize_values +
            "nReset = NumericByRefReset()\n"
            "ON ERROR DO HandleNumericByRefError\n"
            "nUnexpected = NumericByRef(" + missing_case.invocation_arguments + ")\n"
            "ON ERROR\n"
            "nNativeEntries = NumericByRefCount()\n"
            "RETURN\n"
            "PROCEDURE HandleNumericByRefError\n"
            "nCapturedCode = ERROR()\n"
            "cCapturedMessage = MESSAGE()\n"
            "RETURN\n"
            "ENDPROC\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "#3944: recoverable " + missing_case.label + " rejection should complete: " + state.message);

        const auto captured_code = state.globals.find("ncapturedcode");
        const auto captured_message = state.globals.find("ccapturedmessage");
        const auto native_entries = state.globals.find("nnativeentries");
        expect(captured_code != state.globals.end() &&
                   copperfin::runtime::format_value(captured_code->second) == "11",
               "#3944: " + missing_case.label + " should report VFP-compatible Error 11");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "0",
               "#3944: " + missing_case.label + " must not enter native code");

        const std::string message = captured_message == state.globals.end()
            ? std::string{}
            : copperfin::runtime::format_value(captured_message->second);
        const std::string position = std::to_string(missing_case.position);
        if (missing_case.pseudo_locale) {
            expect(message.find("[!! ") == 0U &&
                       message.find("NumericByRef") != std::string::npos &&
                       message.find(position) != std::string::npos &&
                       message.find("is declared numeric by reference") == std::string::npos,
                   "#3944: qps-ploc should pseudo-localize the rejection while preserving alias and position");
        } else {
            expect(message ==
                       "Native DLL function NumericByRef argument " + position +
                           " is declared numeric by reference and requires a call-site @ variable.",
                   "#3944: " + missing_case.label + " should report the actionable localized rejection");
        }
        expect(state.globals.find("nunexpected") == state.globals.end(),
               "#3944: " + missing_case.label + " should reject before assigning a native result");
        expect(std::any_of(state.events.begin(), state.events.end(), [&](const auto &event) {
                   return event.category == "runtime.declare_dll" &&
                          event.detail == "NumericByRef IN " + module;
               }),
               "#3944: " + missing_case.label + " should preserve the invariant source DECLARE event");
        expect(std::any_of(state.events.begin(), state.events.end(), [&](const auto &event) {
                   return event.category == "runtime.error" && event.detail == message;
               }),
               "#3944: " + missing_case.label + " should preserve the invariant runtime.error category");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
                   return event.category == "runtime.error_handler";
               }),
               "#3944: " + missing_case.label + " should dispatch the recoverable error handler");
    }

    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    expect(GetModuleHandleW(fixture_copy.wstring().c_str()) == nullptr,
           "#3944: numeric by-reference sessions should release every controlled module reference");
    fs::remove_all(temp_root, ignored);
#endif
}

void test_declare_dll_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declare_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "declare_localization.prg";
#if defined(_WIN32)
    write_text(
        main_path,
        "DECLARE INTEGER MissingSymbol() IN 'kernel32.dll'\n"
        "RETURN\n");
#else
    write_text(
        main_path,
        "DECLARE INTEGER lstrcpyA(STRING @, STRING) IN 'kernel32.dll'\n"
        "RETURN\n");
#endif

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#2715: qps-ploc DECLARE error script should fail");
#if defined(_WIN32)
    expect(
        state.message.find("[!! ") == 0U &&
            state.message.find("MissingSymbol") != std::string::npos &&
            state.message.find("kernel32.dll") != std::string::npos &&
            state.message.find("function 'MissingSymbol' not found") == std::string::npos,
        "#2715: qps-ploc DECLARE function-not-found error should pseudo-localize prose while preserving function and path");
#else
    expect(
        state.message == copperfin::localization::pseudo_localize("DECLARE DLL is only supported on Windows."),
        "#2715: qps-ploc DECLARE Windows-only guard should route through the pseudo-localization transform");
#endif

    fs::remove_all(temp_root, ignored);
}

void test_set_exact_affects_comparisons_and_seek() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_exact";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_exact.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "lEqOff = 'CHARLIE' = 'CHAR'\n"
        "lEqOffReverse = 'CHAR' = 'CHARLIE'\n"
        "lSeekOff = SEEK('BR')\n"
        "nRecOff = RECNO()\n"
        "SET EXACT ON\n"
        "lEqOn = 'CHARLIE' = 'CHAR'\n"
        "lExactLeadingDiff = ' CHARLIE' = 'CHARLIE'\n"
        "lExactLeadingSame = ' CHARLIE' = ' CHARLIE'\n"
        "lExactTrailingSpaces = 'CHARLIE  ' = 'CHARLIE'\n"
        "lExactAllSpaces = '   ' = ''\n"
        "lExactDifferentLength = 'CHARLIE' = 'CHAR'\n"
        "lExactTrailingTab = ('CHARLIE' + CHR(9)) = 'CHARLIE'\n"
        "lExactTrailingNul = ('CHARLIE' + CHR(0)) = 'CHARLIE'\n"
        "lSeekOn = SEEK('BR')\n"
        "lEofOn = EOF()\n"
        "SET DATASESSION TO 2\n"
        "lEqSession2 = 'CHARLIE' = 'CHAR'\n"
        "SET DATASESSION TO 1\n"
        "lEqBack = 'CHARLIE' = 'CHAR'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET EXACT script should complete");

    const auto eq_off = state.globals.find("leqoff");
    const auto eq_off_reverse = state.globals.find("leqoffreverse");
    const auto seek_off = state.globals.find("lseekoff");
    const auto rec_off = state.globals.find("nrecoff");
    const auto eq_on = state.globals.find("leqon");
    const auto seek_on = state.globals.find("lseekon");
    const auto eof_on = state.globals.find("leofon");
    const auto eq_session2 = state.globals.find("leqsession2");
    const auto eq_back = state.globals.find("leqback");

    expect(eq_off != state.globals.end(), "SET EXACT OFF comparison result should be captured");
    expect(eq_off_reverse != state.globals.end(), "SET EXACT OFF reverse comparison result should be captured");
    expect(seek_off != state.globals.end(), "SET EXACT OFF seek result should be captured");
    expect(rec_off != state.globals.end(), "SET EXACT OFF RECNO() should be captured");
    expect(eq_on != state.globals.end(), "SET EXACT ON comparison result should be captured");
    expect(seek_on != state.globals.end(), "SET EXACT ON seek result should be captured");
    expect(eof_on != state.globals.end(), "SET EXACT ON EOF() should be captured");
    expect(eq_session2 != state.globals.end(), "session 2 comparison result should be captured");
    expect(eq_back != state.globals.end(), "session 1 restored comparison result should be captured");

    if (eq_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_off->second) == "true", "SET EXACT OFF should allow right-side prefix string comparison");
    }
    if (eq_off_reverse != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_off_reverse->second) == "false",
               "SET EXACT OFF should preserve documented right-operand-length comparison");
    }
    if (seek_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_off->second) == "true", "SET EXACT OFF should allow prefix seeks");
    }
    if (rec_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_off->second) == "2", "SET EXACT OFF seek should land on the matching prefix row");
    }
    if (eq_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_on->second) == "false", "SET EXACT ON should require full string equality");
    }
    if (seek_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_on->second) == "false", "SET EXACT ON should reject prefix seeks");
    }
    if (eof_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_on->second) == "true", "SET EXACT ON failed seek should leave the cursor at EOF");
    }
    if (eq_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_session2->second) == "true", "SET EXACT should be scoped to the current data session");
    }
    if (eq_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_back->second) == "false", "restoring the original data session should restore its SET EXACT state");
    }

    const auto check_exact = [&](const std::string& name, const std::string& expected, const std::string& message) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), name + " SET EXACT ON result should be captured");
        if (value != state.globals.end()) {
            expect(copperfin::runtime::format_value(value->second) == expected, message);
        }
    };
    check_exact("lexactleadingdiff", "false", "#3963: SET EXACT ON must preserve significant leading spaces");
    check_exact("lexactleadingsame", "true", "#3963: equal leading spaces should still compare equal");
    check_exact("lexacttrailingspaces", "true", "#3963: SET EXACT ON should disregard trailing spaces");
    check_exact("lexactallspaces", "true", "#3963: all-space and empty strings should compare equal after right padding");
    check_exact("lexactdifferentlength", "false", "#3963: non-space length differences should remain significant");
    check_exact("lexacttrailingtab", "false", "#3963: SET EXACT ON should not discard trailing tabs");
    check_exact("lexacttrailingnul", "false", "#3963: SET EXACT ON should not discard trailing NUL bytes");

    fs::remove_all(temp_root, ignored);
}

void test_use_again_and_alias_collision_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_again";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_again.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS PeopleAgain AGAIN IN 0\n"
        "nAreaAgain = SELECT()\n"
        "cAliasAgain = ALIAS()\n"
        "USE '" + table_path.string() + "' ALIAS PeopleThird IN 0\n"
        "xAfterError = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "duplicate table opens without AGAIN should pause with an error");
    expect(state.location.line == 5U, "duplicate table opens without AGAIN should highlight the failing USE statement");
    expect(
        state.message.find("USE AGAIN is required") != std::string::npos,
        "duplicate table opens without AGAIN should report a USE AGAIN message");

    const auto area_again = state.globals.find("nareaagain");
    const auto alias_again = state.globals.find("caliasagain");
    expect(area_again != state.globals.end(), "USE AGAIN should let execution reach the second-area checks");
    expect(alias_again != state.globals.end(), "USE AGAIN should let execution expose the second alias");
    if (area_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_again->second) == "2", "USE AGAIN IN 0 should allocate a second work area");
    }
    if (alias_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_again->second) == "PeopleAgain", "USE AGAIN should keep the requested second alias");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a USE AGAIN error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after continuing");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "7", "post-error statements should still update globals");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_again_without_in_allocates_new_area_and_preserves_alias_selection() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_again_without_in";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_again_without_in.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS P1 IN 0\n"
        "nAreaP1 = SELECT()\n"
        "USE '" + table_path.string() + "' AGAIN ALIAS P2\n"
        "nAreaP2 = SELECT()\n"
        "nP1Area = SELECT('P1')\n"
        "nP2Area = SELECT('P2')\n"
        "SELECT P1\n"
        "nSelectedAfterSelectP1 = SELECT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "USE AGAIN without IN should complete and preserve both aliases");

    const auto area_p1 = state.globals.find("nareap1");
    const auto area_p2 = state.globals.find("nareap2");
    const auto p1_area = state.globals.find("np1area");
    const auto p2_area = state.globals.find("np2area");
    const auto selected_after_select_p1 = state.globals.find("nselectedafterselectp1");

    expect(area_p1 != state.globals.end(), "initial work area should be captured");
    expect(area_p2 != state.globals.end(), "work area after USE AGAIN should be captured");
    expect(p1_area != state.globals.end(), "SELECT('P1') lookup should be captured");
    expect(p2_area != state.globals.end(), "SELECT('P2') lookup should be captured");
    expect(selected_after_select_p1 != state.globals.end(), "SELECT P1 result should be captured");

    if (area_p1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_p1->second) == "1", "initial USE IN 0 should allocate area 1");
    }
    if (area_p2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_p2->second) == "2", "USE AGAIN without IN should allocate a new work area");
    }
    if (p1_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(p1_area->second) == "1", "P1 alias should remain bound to area 1");
    }
    if (p2_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(p2_area->second) == "2", "P2 alias should bind to area 2");
    }
    if (selected_after_select_p1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_select_p1->second) == "1", "SELECT P1 should successfully target the original alias");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_selected_alias_replacement_clears_old_alias_and_order_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_selected_alias_replacement";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path people_idx_path = temp_root / "people.idx";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "ROME"});
    write_synthetic_idx(people_idx_path, "UPPER(NAME)");

    const fs::path main_path = temp_root / "selected_alias_replacement.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "GO BOTTOM\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN People\n"
        "nOldAliasArea = SELECT('People')\n"
        "nNewAliasArea = SELECT('Cities')\n"
        "cAliasAfter = ALIAS()\n"
        "cOrderAfter = ORDER()\n"
        "nRecAfter = RECNO()\n"
        "cTopAfter = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "selected alias replacement script should complete");

    const auto old_alias_area = state.globals.find("noldaliasarea");
    const auto new_alias_area = state.globals.find("nnewaliasarea");
    const auto alias_after = state.globals.find("caliasafter");
    const auto order_after = state.globals.find("corderafter");
    const auto rec_after = state.globals.find("nrecafter");
    const auto top_after = state.globals.find("ctopafter");

    expect(old_alias_area != state.globals.end(), "SELECT('People') after selected alias replacement should be captured");
    expect(new_alias_area != state.globals.end(), "SELECT('Cities') after selected alias replacement should be captured");
    expect(alias_after != state.globals.end(), "ALIAS() after selected alias replacement should be captured");
    expect(order_after != state.globals.end(), "ORDER() after selected alias replacement should be captured");
    expect(rec_after != state.globals.end(), "RECNO() after selected alias replacement should be captured");
    expect(top_after != state.globals.end(), "field access after selected alias replacement should be captured");

    if (old_alias_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(old_alias_area->second) == "0", "selected alias replacement should clear the old alias lookup");
    }
    if (new_alias_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(new_alias_area->second) == "1", "selected alias replacement should reuse the selected work area in place");
    }
    if (alias_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after->second) == "Cities", "selected alias replacement should expose the new alias immediately");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "selected alias replacement should clear the old active order state when the replacement has no orders");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "1", "selected alias replacement should reset the cursor position for the new table");
    }
    if (top_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_after->second) == "OSLO", "selected alias replacement should expose the new table's first record");
    }

    fs::remove_all(temp_root, ignored);
}


void test_select_missing_alias_is_an_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_select_missing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "select_missing.prg";
    write_text(
        main_path,
        "SELECT MissingAlias\n"
        "xAfterError = 9\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "SELECT MissingAlias should pause with an error");
    expect(state.location.line == 1U, "SELECT MissingAlias should highlight the failing line");
    expect(
        state.message.find("SELECT target work area not found") != std::string::npos,
        "SELECT MissingAlias should report a missing-target message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a SELECT error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after SELECT errors");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "9", "post-error statements should still update globals after SELECT errors");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_missing_alias_is_an_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_in_missing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_in_missing_alias.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN MissingAlias\n"
        "xAfterError = 11\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "USE ... IN MissingAlias should pause with an error");
    expect(state.location.line == 1U, "USE ... IN MissingAlias should highlight the failing line");
    expect(
        state.message.find("USE target work area not found") != std::string::npos,
        "USE ... IN MissingAlias should report a missing-target message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a USE ... IN alias error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after USE ... IN alias errors");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "11", "post-error statements should still update globals after USE ... IN alias errors");
    }
    expect(state.work_area.aliases.empty(), "USE ... IN MissingAlias should not open a fallback work area");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_fault_containment() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "faults.prg";
    write_text(
        main_path,
        "x = 'abc' - 1\n"
        "y = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "mixed-type code should pause with an error instead of killing the host");
    expect(state.location.line == 1U, "runtime faults should highlight the faulting line");
    expect(state.message.find("Operator/operand type mismatch.") != std::string::npos,
        "mixed string-minus operands should report VFP Error 107 prose");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.error"; }),
        "runtime faults should emit a runtime.error event");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a trapped runtime error should keep the session alive");
    const auto y = state.globals.find("y");
    expect(y != state.globals.end(), "post-fault statements should still be able to run");
    if (y != state.globals.end()) {
        expect(copperfin::runtime::format_value(y->second) == "7", "post-fault statements should update globals");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_filter_scopes_local_cursor_visibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "SET FILTER TO AGE >= 30\n"
        "GO TOP\n"
        "cTopFiltered = NAME\n"
        "SKIP\n"
        "cNextFiltered = NAME\n"
        "SKIP\n"
        "lFilteredEof = EOF()\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "lFilteredFound = FOUND()\n"
        "lFilteredLocateEof = EOF()\n"
        "SELECT Other\n"
        "GO TOP\n"
        "cOtherTop = NAME\n"
        "SELECT People\n"
        "SET FILTER TO\n"
        "GO TOP\n"
        "cTopUnfiltered = NAME\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cLocateUnfiltered = NAME\n"
        "SET FILTER TO AGE >= 30\n"
        "SET FILTER OFF\n"
        "GO TOP\n"
        "cTopAfterOff = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FILTER script should complete");

    const auto top_filtered = state.globals.find("ctopfiltered");
    const auto next_filtered = state.globals.find("cnextfiltered");
    const auto filtered_eof = state.globals.find("lfilteredeof");
    const auto filtered_found = state.globals.find("lfilteredfound");
    const auto filtered_locate_eof = state.globals.find("lfilteredlocateeof");
    const auto other_top = state.globals.find("cothertop");
    const auto top_unfiltered = state.globals.find("ctopunfiltered");
    const auto locate_unfiltered = state.globals.find("clocateunfiltered");
    const auto top_after_off = state.globals.find("ctopafteroff");

    expect(top_filtered != state.globals.end(), "filtered GO TOP should expose the first visible record");
    expect(next_filtered != state.globals.end(), "filtered SKIP should expose the next visible record");
    expect(filtered_eof != state.globals.end(), "filtered SKIP past the last visible row should update EOF()");
    expect(filtered_found != state.globals.end(), "filtered LOCATE should expose FOUND()");
    expect(filtered_locate_eof != state.globals.end(), "filtered LOCATE miss should expose EOF()");
    expect(other_top != state.globals.end(), "filters should not bleed into a second alias/work area");
    expect(top_unfiltered != state.globals.end(), "bare SET FILTER TO should restore unfiltered navigation");
    expect(locate_unfiltered != state.globals.end(), "bare SET FILTER TO should restore unfiltered LOCATE behavior");
    expect(top_after_off != state.globals.end(), "SET FILTER OFF should remain a supported clear-filter form");

    if (top_filtered != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_filtered->second) == "CHARLIE", "GO TOP should land on the first filtered-visible row");
    }
    if (next_filtered != state.globals.end()) {
        expect(copperfin::runtime::format_value(next_filtered->second) == "DELTA", "SKIP should move among filtered-visible rows");
    }
    if (filtered_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_eof->second) == "true", "SKIP past the filtered-visible tail should reach EOF");
    }
    if (filtered_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_found->second) == "false", "LOCATE should not find rows excluded by the active filter");
    }
    if (filtered_locate_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_locate_eof->second) == "true", "LOCATE misses within a filtered set should leave the cursor at EOF");
    }
    if (other_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top->second) == "ALPHA", "SET FILTER should remain scoped to the targeted cursor/work area");
    }
    if (top_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_unfiltered->second) == "ALPHA", "bare SET FILTER TO should restore full-table GO TOP semantics");
    }
    if (locate_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_unfiltered->second) == "BRAVO", "bare SET FILTER TO should restore full-table LOCATE behavior");
    }
    if (top_after_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_after_off->second) == "ALPHA", "SET FILTER OFF should continue restoring full-table GO TOP semantics");
    }

    expect(
        std::count_if(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail.find("AGE >= 30") != std::string::npos; }) >= 2 &&
        std::count_if(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail == "OFF"; }) >= 2,
        "SET FILTER expressions and both clear forms should emit invariant runtime.filter events");

    fs::remove_all(temp_root, ignored);
}

void test_set_filter_defers_local_cursor_evaluation_until_navigation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter_deferred";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter_deferred.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "LOCATE FOR NAME = 'DELTA'\n"
        "SET FILTER TO AGE <= 20\n"
        "nBeforeOnlyRec = RECNO()\n"
        "cBeforeOnlyName = NAME\n"
        "lBeforeOnlyFound = FOUND()\n"
        "lBeforeOnlyBof = BOF()\n"
        "lBeforeOnlyEof = EOF()\n"
        "GO TOP\n"
        "nBeforeOnlyTop = RECNO()\n"
        "SET FILTER TO\n"
        "LOCATE FOR NAME = 'MISSING'\n"
        "GO 1\n"
        "SET FILTER TO AGE >= 30\n"
        "nAfterOnlyRec = RECNO()\n"
        "cAfterOnlyName = NAME\n"
        "lAfterOnlyFound = FOUND()\n"
        "lAfterOnlyBof = BOF()\n"
        "lAfterOnlyEof = EOF()\n"
        "SKIP\n"
        "nAfterOnlySkip = RECNO()\n"
        "cAfterOnlySkipName = NAME\n"
        "SET FILTER TO\n"
        "GO 2\n"
        "SET FILTER TO .F.\n"
        "nEmptyRec = RECNO()\n"
        "cEmptyName = NAME\n"
        "lEmptyBof = BOF()\n"
        "lEmptyEof = EOF()\n"
        "lEmptyFound = FOUND()\n"
        "GO TOP\n"
        "nEmptyTopRec = RECNO()\n"
        "lEmptyTopBof = BOF()\n"
        "lEmptyTopEof = EOF()\n"
        "lEmptyTopFound = FOUND()\n"
        "cEmptyTopName = NAME\n"
        "nEmptyTopAge = AGE\n"
        "GO BOTTOM\n"
        "nEmptyBottomRec = RECNO()\n"
        "lEmptyBottomBof = BOF()\n"
        "lEmptyBottomEof = EOF()\n"
        "lEmptyBottomFound = FOUND()\n"
        "cEmptyBottomName = NAME\n"
        "nEmptyBottomAge = AGE\n"
        "SET FILTER TO\n"
        "GO 3\n"
        "SET FILTER TO AGE >= 30\n"
        "nVisibleRec = RECNO()\n"
        "cVisibleName = NAME\n"
        "lVisibleBof = BOF()\n"
        "lVisibleEof = EOF()\n"
        "SET FILTER TO\n"
        "GO 4 IN People\n"
        "SELECT Other\n"
        "GO 2\n"
        "SET FILTER TO AGE <= 20 IN People\n"
        "cSelectedAfterTarget = ALIAS()\n"
        "nSelectedRecAfterTarget = RECNO()\n"
        "nTargetRecAfterFilter = RECNO('People')\n"
        "cTargetNameAfterFilter = People.NAME\n"
        "GO TOP IN People\n"
        "nTargetTopAfterMove = RECNO('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "deferred local SET FILTER evaluation script should complete");

    const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_value("nbeforeonlyrec", "4", "SET FILTER with visible rows only before the pointer should preserve RECNO()");
    expect_value("cbeforeonlyname", "DELTA", "field access before navigation should retain the excluded physical row");
    expect_value("lbeforeonlyfound", "true", "SET FILTER installation should preserve FOUND()");
    expect_value("lbeforeonlybof", "false", "SET FILTER installation should preserve BOF()");
    expect_value("lbeforeonlyeof", "false", "SET FILTER installation should preserve EOF()");
    expect_value("nbeforeonlytop", "1", "GO TOP should evaluate the filter and find an earlier visible row");
    expect_value("nafteronlyrec", "1", "SET FILTER with visible rows only after the pointer should preserve RECNO()");
    expect_value("cafteronlyname", "ALPHA", "deferred filtering should preserve immediate field access before forward navigation");
    expect_value("lafteronlyfound", "false", "deferred SET FILTER installation should preserve a false FOUND() state");
    expect_value("lafteronlybof", "false", "deferred SET FILTER installation should not synthesize BOF()");
    expect_value("lafteronlyeof", "false", "deferred SET FILTER installation should not synthesize EOF()");
    expect_value("nafteronlyskip", "3", "SKIP should evaluate the installed filter from the preserved physical row");
    expect_value("cafteronlyskipname", "CHARLIE", "SKIP should expose the next filter-visible row");
    expect_value("nemptyrec", "2", "an empty filter result should not move the pointer until navigation");
    expect_value("cemptyname", "BRAVO", "an empty filter result should not hide the current field before navigation");
    expect_value("lemptybof", "false", "an empty filter result should preserve BOF() before navigation");
    expect_value("lemptyeof", "false", "an empty filter result should preserve EOF() before navigation");
    expect_value("lemptyfound", "false", "an empty filter result should preserve FOUND() before navigation");
    expect_value("nemptytoprec", "5", "GO TOP with no filter-visible rows should move past the physical record set");
    expect_value("lemptytopbof", "false", "GO TOP with no filter-visible rows in a nonempty cursor should leave BOF() false");
    expect_value("lemptytopeof", "true", "GO TOP should report EOF() when no records satisfy the filter");
    expect_value("lemptytopfound", "false", "GO TOP should preserve FOUND() when no records satisfy the filter");
    expect_value("cemptytopname", "", "field access at filtered EOF should return the field's typed blank value");
    expect_value("nemptytopage", "0", "numeric field access at filtered EOF should return zero");
    expect_value("nemptybottomrec", "5", "GO BOTTOM with no filter-visible rows should retain the physical EOF record number");
    expect_value("lemptybottombof", "true", "GO BOTTOM with no filter-visible rows should set BOF()");
    expect_value("lemptybottomeof", "true", "GO BOTTOM with no filter-visible rows should retain EOF()");
    expect_value("lemptybottomfound", "false", "GO BOTTOM should preserve FOUND() when no records satisfy the filter");
    expect_value("cemptybottomname", "", "character field access after empty-result GO BOTTOM should stay blank");
    expect_value("nemptybottomage", "0", "numeric field access after empty-result GO BOTTOM should stay zero");
    expect_value("nvisiblerec", "3", "a current filter-visible row should remain selected");
    expect_value("cvisiblename", "CHARLIE", "a current filter-visible row should retain field access");
    expect_value("lvisiblebof", "false", "a current filter-visible row should retain BOF()");
    expect_value("lvisibleeof", "false", "a current filter-visible row should retain EOF()");
    expect_value("cselectedaftertarget", "Other", "targeted SET FILTER should preserve the selected work area");
    expect_value("nselectedrecaftertarget", "2", "targeted SET FILTER should preserve the selected cursor pointer");
    expect_value("ntargetrecafterfilter", "4", "targeted SET FILTER should preserve the non-selected target pointer");
    expect_value("ctargetnameafterfilter", "DELTA", "targeted SET FILTER should preserve immediate target field access");
    expect_value("ntargettopaftermove", "1", "targeted navigation should later evaluate the installed filter");

    std::vector<std::string> filter_event_details;
    for (const auto &event : state.events) {
        if (event.category == "runtime.filter") {
            filter_event_details.push_back(event.detail);
        }
    }
    expect(
        filter_event_details == std::vector<std::string>{
            "AGE <= 20", "OFF", "AGE >= 30", "OFF", ".F.", "OFF", "AGE >= 30", "OFF", "AGE <= 20"},
        "deferred and targeted SET FILTER changes should emit one invariant event with exact detail per statement");

    fs::remove_all(temp_root, ignored);
}

void test_seek_respects_active_filter_visibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path other_table_path = temp_root / "other.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"BRAVO", 30}, {"CHARLIE", 40}});
    write_people_dbf(other_table_path, {{"OTHER", 99}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_filter.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "nMinimumAge = 30\n"
        "SET FILTER TO AGE >= nMinimumAge\n"
        "SEEK 'BRAVO'\n"
        "lDuplicateFound = FOUND()\n"
        "nDuplicateRec = RECNO()\n"
        "lDuplicateBof = BOF()\n"
        "lDuplicateEof = EOF()\n"
        "SET NEAR OFF\n"
        "SEEK 'ALPHA'\n"
        "lHiddenFound = FOUND()\n"
        "nHiddenRec = RECNO()\n"
        "lHiddenEof = EOF()\n"
        "SET NEAR ON\n"
        "SEEK 'ALPHA'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "lNearEof = EOF()\n"
        "SEEK 'CHARLIE'\n"
        "lVisibleFound = FOUND()\n"
        "nVisibleRec = RECNO()\n"
        "SET NEAR OFF\n"
        "lFunctionHidden = SEEK('ALPHA')\n"
        "nFunctionRec = RECNO()\n"
        "GO 4\n"
        "nIndexBefore = RECNO()\n"
        "lIndexHidden = INDEXSEEK('ALPHA')\n"
        "nIndexAfter = RECNO()\n"
        "SET FILTER TO SEEK(NAME)\n"
        "SEEK 'BRAVO'\n"
        "lReentrantFound = FOUND()\n"
        "nReentrantRec = RECNO()\n"
        "SET FILTER TO SEEK('MISSING') OR AGE >= nMinimumAge\n"
        "SEEK 'BRAVO'\n"
        "lNestedMissFound = FOUND()\n"
        "nNestedMissRec = RECNO()\n"
        "SET FILTER TO IIF(NAME = 'ALPHA', 1 / 0 = 0, AGE >= nMinimumAge)\n"
        "SEEK 'CHARLIE'\n"
        "lLazyFilterFound = FOUND()\n"
        "nLazyFilterRec = RECNO()\n"
        "SET FILTER TO\n"
        "USE '" + other_table_path.string() + "' ALIAS Other IN 0\n"
        "SELECT People\n"
        "SET ORDER TO 0\n"
        "SET FILTER TO EMPTY(ORDER()) AND RECNO() = 3 AND AGE >= nMinimumAge IN People\n"
        "SELECT Other\n"
        "lTargetContextFound = SEEK('BRAVO', 'People', 'NAME')\n"
        "nTargetContextRec = RECNO('People')\n"
        "cSelectedAfterTarget = ALIAS()\n"
        "nSelectedRecAfterTarget = RECNO()\n"
        "SELECT People\n"
        "SET FILTER TO EMPTY(ORDER()) AND AGE >= nMinimumAge\n"
        "SEEK 'BRAVO'\n"
        "lImplicitOrderFound = FOUND()\n"
        "nImplicitOrderRec = RECNO()\n"
        "cImplicitOrderAfter = ORDER()\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "lRushmoreFilterFound = FOUND()\n"
        "nRushmoreFilterRec = RECNO()\n"
        "cRushmoreFilterOrder = ORDER()\n"
        "SET FILTER TO\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "GO 5\n"
        "SET FIELDS TO NAME\n"
        "cFaultFieldsBefore = SET('FIELDS')\n"
        "SET FILTER TO 1 / 0 = 0\n"
        "SET ORDER TO 0\n"
        "GO 2\n"
        "nFaultRecBefore = RECNO()\n"
        "lFaultFoundBefore = FOUND()\n"
        "lSeekFaultCaught = .F.\n"
        "TRY\n"
        "    SEEK 'BRAVO' TAG NAME\n"
        "CATCH\n"
        "    lSeekFaultCaught = .T.\n"
        "ENDTRY\n"
        "nFaultRecAfter = RECNO()\n"
        "lFaultFoundAfter = FOUND()\n"
        "lFaultBofAfter = BOF()\n"
        "lFaultEofAfter = EOF()\n"
        "cFaultOrderAfter = ORDER()\n"
        "cFaultFieldsAfter = SET('FIELDS')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK with an active filter should complete");

    const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_value("lduplicatefound", "true", "SEEK should find a later visible duplicate after skipping the hidden first duplicate");
    expect_value("nduplicaterec", "3", "SEEK should land on the first filter-visible duplicate");
    expect_value("lduplicatebof", "false", "a filter-visible duplicate hit should clear BOF()");
    expect_value("lduplicateeof", "false", "a filter-visible duplicate hit should clear EOF()");
    expect_value("lhiddenfound", "false", "SEEK should not find a key available only on a filtered-out record");
    expect_value("nhiddenrec", "5", "a filtered-out exact key with SET NEAR OFF should move to physical EOF");
    expect_value("lhiddeneof", "true", "a filtered-out exact key with SET NEAR OFF should set EOF()");
    expect_value("lnearfound", "false", "SET NEAR should not turn a filtered-out exact key into a hit");
    expect_value("nnearrec", "3", "SET NEAR should land on the next filter-visible indexed record");
    expect_value("lneareof", "false", "a filter-visible near record should clear EOF()");
    expect_value("lvisiblefound", "true", "SEEK should continue finding filter-visible exact keys");
    expect_value("nvisiblerec", "4", "SEEK should position on the filter-visible exact key");
    expect_value("lfunctionhidden", "false", "SEEK() should share command visibility behavior");
    expect_value("nfunctionrec", "5", "SEEK() with SET NEAR OFF should move to physical EOF on a filtered key");
    expect_value("nindexbefore", "4", "INDEXSEEK() pointer-preservation setup should select a visible record");
    expect_value("lindexhidden", "false", "INDEXSEEK() should not report a filtered-out key");
    expect_value("nindexafter", "4", "INDEXSEEK() without pointer movement should preserve RECNO()");
    expect_value("lreentrantfound", "true", "SEEK inside the active filter expression should not recursively overflow the runtime");
    expect_value("nreentrantrec", "2", "a re-entrant SEEK filter should preserve ordinary duplicate-key order");
    expect_value("lnestedmissfound", "true", "a nested same-cursor SEEK miss should not hide the candidate record from the rest of the filter");
    expect_value("nnestedmissrec", "3", "a nested SEEK miss should still allow the first filter-visible duplicate");
    expect_value("llazyfilterfound", "true", "SEEK should not evaluate filters for unrelated keys before the lower bound");
    expect_value("nlazyfilterrec", "4", "lazy filter evaluation should preserve the visible exact-key position");
    expect_value("ltargetcontextfound", "true", "SEEK should evaluate an active filter in its non-selected target work area");
    expect_value("ntargetcontextrec", "3", "a targeted SEEK should use the target cursor's RECNO() and logical order in its filter");
    expect_value("cselectedaftertarget", "Other", "a targeted SEEK should preserve the selected work area");
    expect_value("nselectedrecaftertarget", "1", "a targeted SEEK should preserve the selected cursor position");
    expect_value("limplicitorderfound", "true", "bare SEEK should evaluate filters against the logical order before implicit index selection");
    expect_value("nimplicitorderrec", "3", "bare SEEK should find the first visible duplicate through an implicitly selected index");
    expect_value("cimplicitorderafter", "", "bare SEEK should restore the logical controlling order after implicit index selection");
    expect_value("lrushmorefilterfound", "true", "Rushmore LOCATE should evaluate the active filter against the logical controlling order");
    expect_value("nrushmorefilterrec", "3", "Rushmore LOCATE should land on the first filter-visible matching record");
    expect_value("crushmorefilterorder", "", "Rushmore LOCATE should not expose its temporary search order");
    expect_value("nfaultrecbefore", "2", "filter-fault restoration should start from the selected physical record");
    expect_value("lfaultfoundbefore", "true", "filter-fault restoration should start with the prior FOUND() state");
    expect_value("cfaultfieldsbefore", "NAME", "filter-fault restoration should start with the configured SET FIELDS list");
    expect_value("lseekfaultcaught", "true", "a filter evaluation fault during SEEK should remain catchable");
    expect_value("nfaultrecafter", "2", "a filter evaluation fault should restore RECNO()");
    expect_value("lfaultfoundafter", "true", "a filter evaluation fault should restore FOUND()");
    expect_value("lfaultbofafter", "false", "a filter evaluation fault should restore BOF()");
    expect_value("lfaulteofafter", "false", "a filter evaluation fault should restore EOF()");
    expect_value("cfaultorderafter", "", "a filter evaluation fault should restore the prior controlling order");
    expect_value("cfaultfieldsafter", "NAME", "a caught filter evaluation fault should restore SET FIELDS state");

    fs::remove_all(temp_root, ignored);
}

void test_set_filter_in_targets_nonselected_alias() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter_in";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter_in.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "cTargetAlias = 'Other'\n"
        "SET FILTER TO AGE >= 30 IN cTargetAlias\n"
        "GO TOP\n"
        "cPeopleTop = NAME\n"
        "SELECT Other\n"
        "GO TOP\n"
        "cOtherTop = NAME\n"
        "SET FILTER OFF IN cTargetAlias\n"
        "GO TOP\n"
        "cOtherTopUnfiltered = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FILTER ... IN script should complete");

    const auto people_top = state.globals.find("cpeopletop");
    const auto other_top = state.globals.find("cothertop");
    const auto other_top_unfiltered = state.globals.find("cothertopunfiltered");

    expect(people_top != state.globals.end(), "selected alias top row should be captured");
    expect(other_top != state.globals.end(), "targeted alias filtered row should be captured");
    expect(other_top_unfiltered != state.globals.end(), "targeted alias unfiltered row should be captured");

    if (people_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_top->second) == "ALPHA", "SET FILTER ... IN cTargetAlias should not affect People");
    }
    if (other_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top->second) == "CHARLIE", "SET FILTER ... IN cTargetAlias should affect the targeted alias");
    }
    if (other_top_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top_unfiltered->second) == "ALPHA", "SET FILTER OFF IN cTargetAlias should restore unfiltered navigation for the targeted alias");
    }

    expect(
        std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.filter";
        }) >= 2,
        "SET FILTER ... IN and SET FILTER OFF IN should emit runtime.filter events");

    fs::remove_all(temp_root, ignored);
}


}  // namespace

int main() {
    test_set_order_and_seek_for_local_tables();
    test_seek_search_key_uses_heap_backed_frame_continuations();
    test_set_collate_guides_plain_string_seek_comparisons();
    test_seek_uses_grounded_order_normalization_hints();
    test_seek_supports_composite_tag_expressions();
    test_seek_supports_left_function_tag_expressions();
    test_seek_supports_right_function_tag_expressions();
    test_seek_supports_substr_function_tag_expressions();
    test_seek_supports_padl_function_tag_expressions();
    test_seek_supports_padr_function_tag_expressions();
    test_seek_supports_padl_default_padding_tag_expressions();
    test_seek_supports_padr_default_padding_tag_expressions();
    test_seek_supports_str_function_tag_expressions();
    test_seek_supports_str_default_width_tag_expressions();
    test_seek_supports_str_decimal_tag_expressions();
    test_set_near_changes_seek_failure_position();
    test_set_order_descending_changes_seek_ordering();
    test_seek_command_accepts_tag_override_without_set_order();
    test_seek_command_accepts_descending_tag_override_without_set_order();
    test_seek_related_index_functions();
    test_seek_function_accepts_direction_suffix_in_order_designator();
    test_local_table_temporary_order_expression_parity();
    test_order_and_tag_preserve_index_file_identity();
    test_local_command_seek_in_target_with_temporary_order_expression();
    test_local_descending_temporary_order_expression_in_target_preserves_selection();
    test_local_plain_temporary_order_in_target_honors_collate_and_preserves_selection();
    test_local_temporary_order_expression_indexseek_parity();
    test_seek_respects_grounded_order_for_expression_hints();
    test_seek_respects_set_deleted_visibility();
    test_seek_respects_numeric_order_for_expression_hints();
    test_seek_respects_string_order_for_expression_hints();
    test_ndx_numeric_domain_guides_seek_near_ordering();
    test_local_numeric_temporary_order_domain_guides_seek_near_ordering();
    test_set_near_is_scoped_by_data_session();
    test_foxtools_registration_and_call_bridge();
    test_foxtools_registration_is_scoped_by_data_session();
    test_declared_dll_string_byref_argument_writeback();
    test_declared_dll_explicit_relative_child_path();
    test_declared_dll_double_arguments_follow_x64_abi();
    test_declared_dll_long_uses_vfp_32_bit_width();
    test_declared_dll_single_uses_vfp_32_bit_float_width();
    test_declared_dll_win32_uses_typed_stdcall_slots();
    test_declared_dll_win32_resolves_no_underscore_stdcall_export();
    test_declared_dll_short_return_uses_signed_16_bit_width();
    test_declared_dll_short_parameter_is_rejected_and_localized();
    test_declared_dll_win32api_search_and_ansi_fallback();
    test_declared_dll_win32api_missing_symbol_localizes();
    test_declared_dll_module_ownership_and_failed_redeclare_rollback();
    test_declared_dll_explicit_ansi_fallback_and_exact_precedence();
    test_declared_dll_argument_count_is_validated_before_native_entry();
    test_declared_dll_numeric_byref_requires_callsite_reference();
    test_declare_dll_runtime_errors_localize();
    test_set_exact_affects_comparisons_and_seek();
    test_use_again_and_alias_collision_semantics();
    test_use_again_without_in_allocates_new_area_and_preserves_alias_selection();
    test_use_in_selected_alias_replacement_clears_old_alias_and_order_state();
    test_select_missing_alias_is_an_error();
    test_use_in_missing_alias_is_an_error();
    test_runtime_fault_containment();
    test_set_filter_scopes_local_cursor_visibility();
    test_set_filter_defers_local_cursor_evaluation_until_navigation();
    test_seek_respects_active_filter_visibility();
    test_set_filter_in_targets_nonselected_alias();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

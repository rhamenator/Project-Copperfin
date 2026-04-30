#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
        "SET LIBRARY TO 'Foxtools'\n"
        "cFoxTools = FoxToolVer()\n"
        "nMain = MainHwnd()\n"
        "hPid = RegFn32('GetCurrentProcessId', '', 'I', 'kernel32.dll')\n"
        "nPid = CallFn(hPid)\n"
        "hLen = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "nLen = CallFn(hLen, 'Copperfin')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Foxtools bridge script should complete");

    const auto foxtools = state.globals.find("cfoxtools");
    const auto main = state.globals.find("nmain");
    const auto hpid = state.globals.find("hpid");
    const auto pid = state.globals.find("npid");
    const auto hlen = state.globals.find("hlen");
    const auto length = state.globals.find("nlen");

    expect(foxtools != state.globals.end(), "FoxToolVer() should be captured");
    expect(main != state.globals.end(), "MainHwnd() should be captured");
    expect(hpid != state.globals.end(), "RegFn32 handle should be captured");
    expect(pid != state.globals.end(), "CallFn(handle) should be captured");
    expect(hlen != state.globals.end(), "second RegFn32 handle should be captured");
    expect(length != state.globals.end(), "CallFn(string) should be captured");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
        "lSeekOff = SEEK('BR')\n"
        "nRecOff = RECNO()\n"
        "SET EXACT ON\n"
        "lEqOn = 'CHARLIE' = 'CHAR'\n"
        "lSeekOn = SEEK('BR')\n"
        "lEofOn = EOF()\n"
        "SET DATASESSION TO 2\n"
        "lEqSession2 = 'CHARLIE' = 'CHAR'\n"
        "SET DATASESSION TO 1\n"
        "lEqBack = 'CHARLIE' = 'CHAR'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET EXACT script should complete");

    const auto eq_off = state.globals.find("leqoff");
    const auto seek_off = state.globals.find("lseekoff");
    const auto rec_off = state.globals.find("nrecoff");
    const auto eq_on = state.globals.find("leqon");
    const auto seek_on = state.globals.find("lseekon");
    const auto eof_on = state.globals.find("leofon");
    const auto eq_session2 = state.globals.find("leqsession2");
    const auto eq_back = state.globals.find("leqback");

    expect(eq_off != state.globals.end(), "SET EXACT OFF comparison result should be captured");
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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "broken code should pause with an error instead of killing the host");
    expect(state.location.line == 1U, "runtime faults should highlight the faulting line");
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
        "SET FILTER OFF\n"
        "GO TOP\n"
        "cTopUnfiltered = NAME\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cLocateUnfiltered = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    expect(top_filtered != state.globals.end(), "filtered GO TOP should expose the first visible record");
    expect(next_filtered != state.globals.end(), "filtered SKIP should expose the next visible record");
    expect(filtered_eof != state.globals.end(), "filtered SKIP past the last visible row should update EOF()");
    expect(filtered_found != state.globals.end(), "filtered LOCATE should expose FOUND()");
    expect(filtered_locate_eof != state.globals.end(), "filtered LOCATE miss should expose EOF()");
    expect(other_top != state.globals.end(), "filters should not bleed into a second alias/work area");
    expect(top_unfiltered != state.globals.end(), "SET FILTER OFF should restore unfiltered navigation");
    expect(locate_unfiltered != state.globals.end(), "SET FILTER OFF should restore unfiltered LOCATE behavior");

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
        expect(copperfin::runtime::format_value(top_unfiltered->second) == "ALPHA", "SET FILTER OFF should restore full-table GO TOP semantics");
    }
    if (locate_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_unfiltered->second) == "BRAVO", "SET FILTER OFF should restore full-table LOCATE behavior");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail.find("AGE >= 30") != std::string::npos; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail == "OFF"; }),
        "SET FILTER changes should emit runtime.filter events");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
    test_local_temporary_order_expression_indexseek_parity();
    test_seek_respects_grounded_order_for_expression_hints();
    test_seek_respects_numeric_order_for_expression_hints();
    test_seek_respects_string_order_for_expression_hints();
    test_ndx_numeric_domain_guides_seek_near_ordering();
    test_set_near_is_scoped_by_data_session();
    test_foxtools_registration_and_call_bridge();
    test_foxtools_registration_is_scoped_by_data_session();
    test_set_exact_affects_comparisons_and_seek();
    test_use_again_and_alias_collision_semantics();
    test_use_again_without_in_allocates_new_area_and_preserves_alias_selection();
    test_use_in_selected_alias_replacement_clears_old_alias_and_order_state();
    test_select_missing_alias_is_an_error();
    test_use_in_missing_alias_is_an_error();
    test_runtime_fault_containment();
    test_set_filter_scopes_local_cursor_visibility();
    test_set_filter_in_targets_nonselected_alias();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

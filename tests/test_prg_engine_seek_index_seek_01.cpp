#include "test_prg_engine_seek_index_support.h"

namespace copperfin::seek_index_tests
{
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


}

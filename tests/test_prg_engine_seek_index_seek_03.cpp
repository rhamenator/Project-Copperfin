#include "test_prg_engine_seek_index_support.h"

namespace copperfin::seek_index_tests
{
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


}

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <system_error>
#include <vector>

namespace copperfin::table_mutation_tests
{

using namespace copperfin::test_support;

void test_lock_functions_and_unlock_command_track_session_locks() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_lock_functions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path other_path = temp_root / "other.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    write_people_dbf(other_path, {{"OTHER", 1}});

    const fs::path main_path = temp_root / "locks.prg";
    write_text(
        main_path,
        "cDefaultReprocess = SET('REPROCESS')\n"
        "cDefaultMultilocks = SET('MULTILOCKS')\n"
        "SET REPROCESS TO 3\n"
        "SET MULTILOCKS ON\n"
        "cReprocess = SET('REPROCESS')\n"
        "cMultilocks = SET('MULTILOCKS')\n"
        "SET DATASESSION TO 2\n"
        "cReprocessSession2 = SET('REPROCESS')\n"
        "cMultilocksSession2 = SET('MULTILOCKS')\n"
        "SET DATASESSION TO 1\n"
        "cReprocessRestored = SET('REPROCESS')\n"
        "cMultilocksRestored = SET('MULTILOCKS')\n"
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "lRecordLock = RLOCK()\n"
        "lRecordLocked = ISRLOCKED()\n"
        "lFileLock = FLOCK()\n"
        "lFileLocked = ISFLOCKED()\n"
        "UNLOCK\n"
        "lRecordAfterUnlock = ISRLOCKED()\n"
        "lFileAfterUnlock = ISFLOCKED()\n"
        "lNamedRecordLock = RLOCK('People')\n"
        "lFileRelock = FLOCK()\n"
        "GO 2\n"
        "lSecondRecordLock = RLOCK()\n"
        "UNLOCK RECORD 1 IN People\n"
        "GO 1\n"
        "lRecordOneAfterSpecificUnlock = ISRLOCKED()\n"
        "lFileAfterSpecificUnlock = ISFLOCKED()\n"
        "GO 2\n"
        "lRecordTwoAfterSpecificUnlock = ISRLOCKED()\n"
        "USE '" + other_path.string() + "' ALIAS Other IN 0\n"
        "lOtherFileLock = FLOCK()\n"
        "SELECT People\n"
        "lPeopleRecordLocked = ISRLOCKED()\n"
        "UNLOCK ALL\n"
        "lPeopleAfterUnlockAll = ISRLOCKED()\n"
        "SELECT Other\n"
        "lOtherAfterUnlockAll = ISFLOCKED()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "lock function script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
            name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("cdefaultreprocess", "AUTOMATIC");
    check("cdefaultmultilocks", "OFF");
    check("creprocess", "3");
    check("cmultilocks", "ON");
    check("creprocesssession2", "AUTOMATIC");
    check("cmultilockssession2", "OFF");
    check("creprocessrestored", "3");
    check("cmultilocksrestored", "ON");
    check("lrecordlock", "true");
    check("lrecordlocked", "true");
    check("lfilelock", "true");
    check("lfilelocked", "true");
    check("lrecordafterunlock", "false");
    check("lfileafterunlock", "false");
    check("lnamedrecordlock", "true");
    check("lfilerelock", "true");
    check("lsecondrecordlock", "true");
    check("lrecordoneafterspecificunlock", "false");
    check("lfileafterspecificunlock", "true");
    check("lrecordtwoafterspecificunlock", "true");
    check("lotherfilelock", "true");
    check("lpeoplerecordlocked", "true");
    check("lpeopleafterunlockall", "false");
    check("lotherafterunlockall", "false");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock";
    }), "lock functions should emit runtime.lock events");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.unlock" && event.detail == "ALL";
    }), "UNLOCK ALL should emit a runtime.unlock event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.unlock" && event.detail == "People RECORD 1";
    }), "UNLOCK RECORD should emit a record-specific runtime.unlock event");

    fs::remove_all(temp_root, ignored);
}

void test_record_lock_argument_conversion_is_bounded() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_lock_argument_bounds";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "lock_arguments.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "lNegative = RLOCK(-1)\n"
        "lSmallFraction = RLOCK(0.4)\n"
        "lRoundedDown = RLOCK(1.49)\n"
        "UNLOCK ALL\n"
        "lRoundedUp = RLOCK(1.5)\n"
        "lRoundedUpLocked = ISRLOCKED(1.5)\n"
        "UNLOCK ALL\n"
        "lOversized = RLOCK(VAL('1e20'))\n"
        "lNonFinite = RLOCK(EXP(1000))\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "bounded lock argument script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
            name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("lnegative", "false");
    check("lsmallfraction", "false");
    check("lroundeddown", "true");
    check("lroundedup", "true");
    check("lroundeduplocked", "true");
    check("loversized", "false");
    check("lnonfinite", "false");

    fs::remove_all(temp_root, ignored);
}

void test_replacing_a_used_work_area_releases_prior_table_locks() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_use_unlock";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path held_path = temp_root / "held.dbf";
    const fs::path replacement_path = temp_root / "replacement.dbf";
    write_people_dbf(held_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    write_people_dbf(replacement_path, {{"OTHER", 1}});

    const fs::path writer_path = temp_root / "writer.prg";
    write_text(
        writer_path,
        "SET MULTILOCKS ON\n"
        "USE '" + held_path.string() + "' ALIAS Held IN 0\n"
        "lHeldLock = FLOCK()\n"
        "USE '" + replacement_path.string() + "' ALIAS Replacement IN Held\n"
        "UNLOCK\n"
        "RETURN\n");

    {
        copperfin::runtime::PrgRuntimeSession writer =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(writer_path.string(), temp_root.string()));
        const auto writer_state = writer.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(writer_state.completed, "#3673: writer script should complete after replacing a locked work area");

        const auto held_lock = writer_state.globals.find("lheldlock");
        expect(held_lock != writer_state.globals.end(), "#3673: writer script should capture the initial table lock result");
        if (held_lock != writer_state.globals.end()) {
            expect(copperfin::runtime::format_value(held_lock->second) == "true",
                   "#3673: writer should successfully hold the initial table lock before replacement");
        }
    }

    const fs::path reader_path = temp_root / "reader.prg";
    write_text(
        reader_path,
        "SET MULTILOCKS ON\n"
        "USE '" + held_path.string() + "' ALIAS HeldAgain SHARED IN 0\n"
        "lRelock = FLOCK()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession reader =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(reader_path.string(), temp_root.string()));
    const auto reader_state = reader.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(reader_state.completed, "#3673: reader script should complete after the writer session exits");

    const auto relock = reader_state.globals.find("lrelock");
    expect(relock != reader_state.globals.end(), "#3673: reader script should capture the relock attempt");
    if (relock != reader_state.globals.end()) {
        expect(copperfin::runtime::format_value(relock->second) == "true",
               "#3673: replacing a used work area should release the old table lock for a later runtime session");
    }

    fs::remove_all(temp_root, ignored);
}

void test_reprocess_contention_retries_and_mutation_lock_timeouts() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_reprocess_contention";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "reprocess_contention.prg";
    write_text(
        main_path,
        "cDefaultReprocess = SET('REPROCESS')\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleOne SHARED IN 0\n"
        "GO 1\n"
        "lHeldLock = RLOCK()\n"
        "SET DATASESSION TO 2\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleTwo SHARED IN 0\n"
        "GO 1\n"
        "cDefaultReprocessSession2 = SET('REPROCESS')\n"
        "lDefaultConflict = RLOCK()\n"
        "SET REPROCESS TO 2\n"
        "lRetryConflict = RLOCK()\n"
        "TRY\n"
        "    REPLACE NAME WITH 'BLOCKED'\n"
        "    lReplaceBlocked = .F.\n"
        "CATCH TO err_text\n"
        "    lReplaceBlocked = .T.\n"
        "    cReplaceError = err_text.Message\n"
        "ENDTRY\n"
        "SET REPROCESS TO 1.234\n"
        "cGroupedReprocess = SET('REPROCESS')\n"
        "lGroupedConflict = RLOCK()\n"
        "SET REPROCESS TO '2abc'\n"
        "cTrailingReprocess = SET('REPROCESS')\n"
        "lTrailingConflict = RLOCK()\n"
        "SET REPROCESS TO -1\n"
        "cNegativeReprocess = SET('REPROCESS')\n"
        "lNegativeConflict = RLOCK()\n"
        "SET REPROCESS TO '999999999999999999999999999999'\n"
        "cOverflowReprocess = SET('REPROCESS')\n"
        "lOverflowConflict = RLOCK()\n"
        "SET REPROCESS TO 0\n"
        "lZeroConflict = RLOCK()\n"
        "SET DATASESSION TO 1\n"
        "UNLOCK ALL\n"
        "SET DATASESSION TO 2\n"
        "SET REPROCESS TO 2\n"
        "lAfterRelease = RLOCK()\n"
        "lAfterReleaseState = ISRLOCKED()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPROCESS contention script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("cdefaultreprocess", "AUTOMATIC");
    check("cdefaultreprocesssession2", "AUTOMATIC");
    check("lheldlock", "true");
    check("ldefaultconflict", "false");
    check("lretryconflict", "false");
    check("lreplaceblocked", "true");
    check("cgroupedreprocess", "1.234");
    check("lgroupedconflict", "false");
    check("ctrailingreprocess", "2ABC");
    check("ltrailingconflict", "false");
    check("cnegativereprocess", "-1");
    check("lnegativeconflict", "false");
    check("coverflowreprocess", "999999999999999999999999999999");
    check("loverflowconflict", "false");
    check("lzeroconflict", "false");
    check("lafterrelease", "true");
    check("lafterreleasestate", "true");

    const auto replace_error = state.globals.find("creplaceerror");
    expect(replace_error != state.globals.end(), "REPLACE contention script should capture the caught error text");
    if (replace_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(replace_error->second).find("timed out waiting for record lock") != std::string::npos,
               "REPLACE contention error should report the record-lock timeout");
    }

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    for (const auto& [locale, expected_text] : std::vector<std::pair<std::string, std::string>>{
             {"en-US", "REPLACE timed out waiting for record lock (2)"},
             {"es-419", "REPLACE agotó el tiempo de espera mientras esperaba el bloqueo del registro (2)"},
             {"pt-BR", "REPLACE atingiu o tempo limite aguardando o bloqueio do registro (2)"}}) {
        set_env_value("COPPERFIN_LOCALE", locale, true);
        auto localized_session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto localized_state = localized_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(localized_state.completed, locale + " REPROCESS contention script should complete");
        const auto localized_error = localized_state.globals.find("creplaceerror");
        expect(localized_error != localized_state.globals.end(), locale + " should capture the localized timeout error");
        if (localized_error != localized_state.globals.end()) {
            expect(copperfin::runtime::format_value(localized_error->second) == expected_text,
                   locale + " should localize the record-lock timeout while preserving placeholders");
        }
        expect(std::any_of(localized_state.events.begin(), localized_state.events.end(), [](const auto& event) {
            return event.category == "runtime.lock_timeout" &&
                   event.detail.find("REPLACE timeout recno=1 reprocess=2") != std::string::npos;
        }), locale + " should preserve the invariant record-lock timeout telemetry");
    }

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    auto pseudo_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto pseudo_state = pseudo_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(pseudo_state.completed, "qps-ploc REPROCESS contention script should complete");
    const auto pseudo_error = pseudo_state.globals.find("creplaceerror");
    expect(pseudo_error != pseudo_state.globals.end(), "qps-ploc should capture the pseudo-localized timeout error");
    if (pseudo_error != pseudo_state.globals.end()) {
        const std::string message = copperfin::runtime::format_value(pseudo_error->second);
        expect(message.starts_with("[!! ") && message.find("REPLACE") != std::string::npos &&
                   message.find("(2)") != std::string::npos &&
                   message != "REPLACE timed out waiting for record lock (2)",
               "qps-ploc should decorate timeout prose while preserving placeholders");
    }
    expect(std::any_of(pseudo_state.events.begin(), pseudo_state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_timeout" &&
               event.detail.find("REPLACE timeout recno=1 reprocess=2") != std::string::npos;
    }), "qps-ploc should preserve invariant record-lock timeout telemetry");

    const auto count_retry_events = [&](const std::string& detail_fragment) {
        return static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [&](const auto& event) {
            return event.category == "runtime.lock_retry" &&
                   event.detail.find(detail_fragment) != std::string::npos;
        }));
    };

    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=AUTOMATIC") == 8,
           "default REPROCESS should perform eight retry/yield attempts before RLOCK() fails");
    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=2") == 2,
           "SET REPROCESS TO 2 should perform two retry attempts before RLOCK() fails");
    expect(count_retry_events("REPLACE recno=1 reprocess=2") == 2,
           "REPLACE under lock contention should honor the per-session REPROCESS retry budget");
    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=1.234") == 0,
           "grouped/decimal REPROCESS text should fail closed without prefix-derived retries");
    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=2ABC") == 0,
           "trailing REPROCESS text should fail closed without prefix-derived retries");
    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=-1") == 0,
           "negative REPROCESS text should fail closed without inventing a retry budget");
    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=999999999999999999999999999999") == 0,
           "overflowing REPROCESS text should fail closed without inventing a retry budget");
    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=0") == 0,
           "SET REPROCESS TO 0 should not busy-spin before RLOCK() fails");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_timeout" &&
               event.detail.find("PeopleTwo RLOCK timeout recno=1 reprocess=AUTOMATIC") != std::string::npos;
    }), "default RLOCK contention should emit a deterministic runtime.lock_timeout event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_timeout" &&
               event.detail.find("REPLACE timeout recno=1 reprocess=2") != std::string::npos;
    }), "REPLACE contention should emit a deterministic runtime.lock_timeout event");

    fs::remove_all(temp_root, ignored);
}

void test_reprocess_table_lock_timeouts_are_localized() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_lock_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    const fs::path main_path = temp_root / "table_lock_localization.prg";
    write_text(
        main_path,
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleOne SHARED IN 0\n"
        "lHeldLock = FLOCK()\n"
        "SET DATASESSION TO 2\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleTwo SHARED IN 0\n"
        "SET REPROCESS TO 2\n"
        "TRY\n"
        "    APPEND BLANK\n"
        "    lAppendBlocked = .F.\n"
        "CATCH TO err_text\n"
        "    lAppendBlocked = .T.\n"
        "    cAppendError = err_text.Message\n"
        "ENDTRY\n"
        "RETURN\n");

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    for (const auto& [locale, expected_text] : std::vector<std::pair<std::string, std::string>>{
             {"en-US", "APPEND BLANK timed out waiting for table lock (2)"},
             {"es-419", "APPEND BLANK agotó el tiempo de espera mientras esperaba el bloqueo de la tabla (2)"},
             {"pt-BR", "APPEND BLANK atingiu o tempo limite aguardando o bloqueio da tabela (2)"}}) {
        set_env_value("COPPERFIN_LOCALE", locale, true);
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, locale + " table-lock contention script should complete");
        const auto blocked = state.globals.find("lappendblocked");
        const auto error = state.globals.find("cappenderror");
        expect(blocked != state.globals.end(), locale + " should capture APPEND BLANK lock failure");
        expect(error != state.globals.end(), locale + " should capture the table-lock timeout error");
        if (blocked != state.globals.end()) {
            expect(copperfin::runtime::format_value(blocked->second) == "true",
                   locale + " APPEND BLANK should remain blocked by the held table lock");
        }
        if (error != state.globals.end()) {
            expect(copperfin::runtime::format_value(error->second) == expected_text,
                   locale + " should localize the table-lock timeout while preserving placeholders");
        }
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.lock_timeout" &&
                   event.detail.find("APPEND BLANK timeout reprocess=2") != std::string::npos;
        }), locale + " should preserve invariant table-lock timeout telemetry");
    }

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    auto pseudo_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto pseudo_state = pseudo_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(pseudo_state.completed, "qps-ploc table-lock contention script should complete");
    const auto pseudo_error = pseudo_state.globals.find("cappenderror");
    expect(pseudo_error != pseudo_state.globals.end(), "qps-ploc should capture the pseudo-localized table-lock error");
    if (pseudo_error != pseudo_state.globals.end()) {
        const std::string message = copperfin::runtime::format_value(pseudo_error->second);
        expect(message.starts_with("[!! ") && message.find("APPEND BLANK") != std::string::npos &&
                   message.find("(2)") != std::string::npos &&
                   message != "APPEND BLANK timed out waiting for table lock (2)",
               "qps-ploc should decorate table-lock timeout prose while preserving placeholders");
    }
    expect(std::any_of(pseudo_state.events.begin(), pseudo_state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_timeout" &&
               event.detail.find("APPEND BLANK timeout reprocess=2") != std::string::npos;
    }), "qps-ploc should preserve invariant table-lock timeout telemetry");

    fs::remove_all(temp_root, ignored);
}

void test_lock_retry_blocking_is_rejected_inside_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_lock_retry_critical_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "lock_retry_critical_policy.prg";
    write_text(
        main_path,
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleOne SHARED IN 0\n"
        "GO 1\n"
        "lHeldLock = RLOCK()\n"
        "SET DATASESSION TO 2\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleTwo SHARED IN 0\n"
        "GO 1\n"
        "TRY\n"
        "    ENTER CRITICAL shared\n"
        "    REPLACE NAME WITH 'BLOCKED'\n"
        "    lPolicyBlocked = .F.\n"
        "CATCH TO err_text\n"
        "    lPolicyBlocked = .T.\n"
        "    cPolicyError = err_text.Message\n"
        "ENDTRY\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section lock-retry policy script should complete: " + state.message);

    const auto held_lock = state.globals.find("lheldlock");
    const auto policy_blocked = state.globals.find("lpolicyblocked");
    const auto policy_error = state.globals.find("cpolicyerror");
    expect(held_lock != state.globals.end(), "lock-retry policy script should capture the first-session held lock");
    expect(policy_blocked != state.globals.end(), "lock-retry policy script should capture the policy-block result");
    expect(policy_error != state.globals.end(), "lock-retry policy script should capture the policy-block message");
    if (held_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(held_lock->second) == "true",
               "lock-retry policy script should hold the first-session record lock");
    }
    if (policy_blocked != state.globals.end()) {
        expect(copperfin::runtime::format_value(policy_blocked->second) == "true",
               "REPLACE under contention inside a critical section should be rejected");
    }
    if (policy_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(policy_error->second) ==
                   "Blocking operation LOCK RETRY is not allowed while holding CRITICAL section shared",
               "lock-retry policy error should route through the default locale catalog");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=LOCK RETRY") != std::string::npos;
    }), "lock-retry contention inside a critical section should emit a runtime.critical.blocking_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_rlock_retry_blocking_is_rejected_inside_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_rlock_critical_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "rlock_critical_policy.prg";
    write_text(
        main_path,
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleOne SHARED IN 0\n"
        "GO 1\n"
        "lHeldLock = RLOCK()\n"
        "SET DATASESSION TO 2\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleTwo SHARED IN 0\n"
        "GO 1\n"
        "SET REPROCESS TO 2\n"
        "ENTER CRITICAL shared\n"
        "lSecondLock = RLOCK()\n"
        "IF lSecondLock\n"
        "    lPolicyBlocked = .F.\n"
        "ELSE\n"
        "    lPolicyBlocked = .T.\n"
        "ENDIF\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section RLOCK policy script should complete: " + state.message);

    const auto held_lock = state.globals.find("lheldlock");
    const auto second_lock = state.globals.find("lsecondlock");
    const auto policy_blocked = state.globals.find("lpolicyblocked");
    expect(held_lock != state.globals.end(), "RLOCK policy script should capture the first-session held lock");
    expect(second_lock != state.globals.end(), "RLOCK policy script should capture the contested second lock result");
    expect(policy_blocked != state.globals.end(), "RLOCK policy script should capture the policy-block result");
    if (held_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(held_lock->second) == "true",
               "RLOCK policy script should hold the first-session record lock");
    }
    if (second_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_lock->second) == "false",
               "RLOCK under contention inside a critical section should return false");
    }
    if (policy_blocked != state.globals.end()) {
        expect(copperfin::runtime::format_value(policy_blocked->second) == "true",
               "RLOCK under contention inside a critical section should be rejected");
    }

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_retry" &&
               event.detail.find("PeopleTwo RLOCK") != std::string::npos;
    }), "RLOCK inside a critical section should fail before emitting retry backoff events");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=LOCK RETRY") != std::string::npos &&
               event.detail.find("PeopleTwo RLOCK") != std::string::npos;
    }), "RLOCK contention inside a critical section should emit a runtime.critical.blocking_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_flock_retry_blocking_is_rejected_inside_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_flock_critical_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "flock_critical_policy.prg";
    write_text(
        main_path,
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleOne SHARED IN 0\n"
        "lHeldLock = FLOCK()\n"
        "SET DATASESSION TO 2\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleTwo SHARED IN 0\n"
        "ENTER CRITICAL shared\n"
        "lSecondLock = FLOCK()\n"
        "lPolicyRejected = (lSecondLock = .F.)\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section FLOCK policy script should complete: " + state.message);

    const auto held_lock = state.globals.find("lheldlock");
    const auto second_lock = state.globals.find("lsecondlock");
    const auto policy_rejected = state.globals.find("lpolicyrejected");
    expect(held_lock != state.globals.end(), "FLOCK policy script should capture the first-session file lock");
    expect(second_lock != state.globals.end(), "FLOCK policy script should capture the second-session contested lock attempt");
    expect(policy_rejected != state.globals.end(), "FLOCK policy script should capture rejection state");
    if (held_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(held_lock->second) == "true",
               "first-session FLOCK should acquire and hold the file lock");
    }
    if (second_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_lock->second) == "false",
               "FLOCK under contention inside a critical section should return false");
    }
    if (policy_rejected != state.globals.end()) {
        expect(copperfin::runtime::format_value(policy_rejected->second) == "true",
               "FLOCK contention inside a critical section should be rejected");
    }

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_retry" &&
               event.detail.find("PeopleTwo FLOCK") != std::string::npos;
    }), "FLOCK inside a critical section should fail before emitting retry backoff events");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=LOCK RETRY") != std::string::npos &&
               event.detail.find("PeopleTwo FLOCK") != std::string::npos;
    }), "FLOCK contention inside a critical section should emit a runtime.critical.blocking_violation event");

    fs::remove_all(temp_root, ignored);
}

} // namespace copperfin::table_mutation_tests

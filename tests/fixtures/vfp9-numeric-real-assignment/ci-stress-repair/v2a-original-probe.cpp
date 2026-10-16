// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Original V2a function extracted unchanged except controlled delayed-start injection.
#include "test_prg_engine_control_flow_support.h"
namespace cf_test_prg_engine_control_flow {
void test_state_sequence_retry_after_caught_rollback_succeeds_cleanly() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_state_seq_retry_after_rollback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "race.dbf";
    const fs::path main_path = temp_root / "state_seq_retry_after_rollback.prg";
    write_text(
        main_path,
        "PUBLIC error_seen, nTxnLevelAtCatch, nTxnLevelAfterRollback, lHolderDone, "
        "nTxnLevelAfterRetryCommit, cRecord1Final, cRecord2Final\n"
        "error_seen = .F.\n"
        "ON ERROR DO caught\n"
        "CREATE TABLE race (name C(12))\n"
        "SELECT race\n"
        "APPEND BLANK\n"
        "REPLACE name WITH 'ORIGINAL1'\n"
        "APPEND BLANK\n"
        "REPLACE name WITH 'ORIGINAL2'\n"
        "SPAWN holder TO nHolder\n"
        "SLEEP 50\n"
        "BEGIN TRANSACTION\n"
        "GO 2\n"
        "REPLACE name WITH 'FIRSTATTEMPT'\n"
        "RETURN\n"
        "PROCEDURE caught\n"
        "error_seen = .T.\n"
        "nTxnLevelAtCatch = TXNLEVEL()\n"
        "ROLLBACK\n"
        "nTxnLevelAfterRollback = TXNLEVEL()\n"
        "AWAIT nHolder TO lHolderDone\n"
        "BEGIN TRANSACTION\n"
        "GO 2\n"
        "REPLACE name WITH 'RETRIED'\n"
        "END TRANSACTION\n"
        "nTxnLevelAfterRetryCommit = TXNLEVEL()\n"
        "GO 1\n"
        "cRecord1Final = ALLTRIM(name)\n"
        "GO 2\n"
        "cRecord2Final = ALLTRIM(name)\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE holder\n"
#if defined(V2A_DELAYED_HOLDER)
        "SLEEP 150\n"
#endif
        "SELECT race\n"
        "GO 2\n"
        "lHolderLocked = RLOCK()\n"
        "SLEEP 300\n"
        "RETURN lHolderLocked\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6495 V2a: state-sequence test should complete: " + state.message);

    const auto error_seen_it = state.globals.find("error_seen");
    expect(error_seen_it != state.globals.end() && error_seen_it->second.boolean_value,
           "#6495 V2a: the first transaction's REPLACE should contend with the sibling holder's RLOCK() and "
           "raise a catchable error, not silently succeed against a record the holder has claimed");

    const auto timeout_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.lock_timeout"; });
    expect(timeout_event != state.events.end(),
           "#6495 V2a: the first attempt's REPLACE should genuinely contend with the holder's RLOCK() and time "
           "out via the reprocess retry budget, proving real record-level contention");

    const auto txn_at_catch_it = state.globals.find("ntxnlevelatcatch");
    expect(txn_at_catch_it != state.globals.end() && txn_at_catch_it->second.number_value == 1.0,
           "#6495 V2a: the ON ERROR handler should observe the still-open transaction level before ROLLBACK");

    const auto txn_after_rollback_it = state.globals.find("ntxnlevelafterrollback");
    expect(txn_after_rollback_it != state.globals.end() && txn_after_rollback_it->second.number_value == 0.0,
           "#6495 V2a: ROLLBACK should reset TXNLEVEL() to 0 after the caught error");

    const auto txn_after_retry_it = state.globals.find("ntxnlevelafterretrycommit");
    expect(txn_after_retry_it != state.globals.end() && txn_after_retry_it->second.number_value == 0.0,
           "#6495 V2a: the retried transaction's END TRANSACTION should commit and reset TXNLEVEL() to 0, with "
           "no residue leaked from the failed first attempt");

    const auto record1_it = state.globals.find("crecord1final");
    expect(record1_it != state.globals.end() && record1_it->second.string_value == "ORIGINAL1",
           "#6495 V2a: record 1 should remain untouched by either attempt");

    const auto record2_it = state.globals.find("crecord2final");
    expect(record2_it != state.globals.end() && record2_it->second.string_value == "RETRIED",
           "#6495 V2a: record 2 should reflect only the retried transaction's committed write, not the failed "
           "first attempt's, got '" +
               (record2_it != state.globals.end() ? record2_it->second.string_value : "<missing>") + "'");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#6495 V2a: race.dbf should remain readable after rollback and retry-commit");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[1].values[0].display_value == "RETRIED",
               "#6495 V2a: on-disk record 2 should reflect the retried commit, got '" +
                   parse_result.table.records[1].values[0].display_value + "'");
    }

    const fs::path transaction_root = temp_root / "runtime-temp" / "transactions";
    if (std::filesystem::exists(transaction_root, ignored)) {
        bool found_pending_journal = false;
        for (const auto &entry : std::filesystem::directory_iterator(transaction_root, ignored)) {
            if (!entry.is_directory(ignored)) {
                continue;
            }
            if (entry.path().filename().string().rfind("txn_", 0U) == 0U) {
                found_pending_journal = true;
                break;
            }
        }
        expect(!found_pending_journal,
               "#6495 V2a: no pending transaction journal should remain after the failed attempt's ROLLBACK "
               "and the retried attempt's successful END TRANSACTION");
    }

    fs::remove_all(temp_root, ignored);
}

}
int main() {
 cf_test_prg_engine_control_flow::test_state_sequence_retry_after_caught_rollback_succeeds_cleanly();
 return copperfin::test_support::test_failures() ? 1 : 0;
}

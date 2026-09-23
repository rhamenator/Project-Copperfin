// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

// #6495 state-sequence hunt: deterministic, seed-replayable short sequences
// crossing SPAWN/AWAIT/cancellation/teardown and cursor-lock/transaction
// state. The existing `stress` Cloud Defect Hunt lane only repeats a fixed
// set of whole tests (docs/cloud-validation.md); it does not construct new
// interleavings. Each sequence below is deterministic without relying on raw
// OS thread-scheduling luck: real record/table-lock contention (which only
// proceeds once a genuine conflict is confirmed under
// `concurrency_state->mutex`) and `SET REPROCESS TO n` (which widens the
// existing linear per-attempt backoff, `prg_engine_records.inl`'s
// `pause_for_lock_retry`) give a wide, real synchronization window instead
// of a guessed one. Each function documents its invariant and expected
// result before the script that exercises it, per #6495's acceptance
// criteria. These tests are compiled into `test_prg_engine_control_flow`, so
// they are automatically included in that binary's existing `stress` lane
// repetitions (`scripts/run-cloud-validation.py`) with no workflow change.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

// Crossing #1 (SPAWN/AWAIT x cancellation/assignment-failure x teardown),
// sequence V1a.
//
// Invariant: a task that ends via CANCELLATION, not just one that ends via
// natural completion (already covered by #6456's regression), must remain
// registered and independently AWAITable after a failed output-assignment
// attempt; a later valid-target AWAIT must still consume it exactly once.
// Expected result: the first AWAIT with a bad target raises a catchable
// error and leaves the task's status different from the post-erase sentinel
// ('unknown'); the retry AWAIT then succeeds and finally erases it.
void test_state_sequence_await_retry_after_cancellation_reuses_still_registered_task() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_state_seq_await_retry_after_cancel";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "state_seq_await_retry_after_cancel.prg";
    write_text(
        main_path,
        "PUBLIC error_seen\n"
        "error_seen = .F.\n"
        "ON ERROR DO caught\n"
        "SPAWN worker TO nWorker\n"
        "SPAWN canceler TO nCancel\n"
        "AWAIT nCancel TO lCancelDone\n"
        "DO WHILE CFTASKSTATUS(nWorker) == 'running'\n"
        "ENDDO\n"
        "AWAIT nWorker TO missing.property\n"
        "lStillRegisteredAfterCancelFailure = (CFTASKSTATUS(nWorker) != 'unknown')\n"
        "AWAIT nWorker TO lWorkerRetryDone\n"
        "cStatusAfterRetry = CFTASKSTATUS(nWorker)\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "SLEEP 50\n"
        "RETURN .T.\n"
        "ENDPROC\n"
        "PROCEDURE canceler\n"
        "SLEEP 1\n"
        "CANCEL\n"
        "ENDPROC\n"
        "PROCEDURE caught\n"
        "error_seen = .T.\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6495 V1a: state-sequence test should complete: " + state.message);

    const auto cancelled_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.task.cancelled"; });
    expect(cancelled_event != state.events.end(),
           "#6495 V1a: the worker must genuinely end via cancellation, not natural completion, for this "
           "sequence to exercise the intended crossing");

    const auto error_seen_it = state.globals.find("error_seen");
    expect(error_seen_it != state.globals.end() && error_seen_it->second.boolean_value,
           "#6495 V1a: AWAIT nWorker TO missing.property should raise a catchable error the ON ERROR handler "
           "observes, exactly as it does for a naturally-completed task in #6456");

    const auto still_registered_it = state.globals.find("lstillregisteredaftercancelfailure");
    expect(still_registered_it != state.globals.end() && still_registered_it->second.boolean_value,
           "#6495 V1a: a cancelled task must remain registered (not erased) after a failed output assignment, "
           "generalizing #6456's erase-only-on-success ordering to the cancelled terminal state, not just the "
           "completed one");

    const auto retry_it = state.globals.find("lworkerretrydone");
    expect(retry_it != state.globals.end() && !retry_it->second.boolean_value,
           "#6495 V1a: a retry AWAIT with a valid target should successfully consume the still-registered "
           "cancelled task and report its non-completed (cancelled) result, not silently fabricate success");

    const auto status_after_retry_it = state.globals.find("cstatusafterretry");
    expect(status_after_retry_it != state.globals.end() &&
               status_after_retry_it->second.string_value == "unknown",
           "#6495 V1a: the successful retry AWAIT should finally erase the cancelled task, exactly once, got '" +
               (status_after_retry_it != state.globals.end() ? status_after_retry_it->second.string_value
                                                               : "<missing>") +
               "'");

    fs::remove_all(temp_root, ignored);
}

// Crossing #1, sequence V1b.
//
// Invariant: cancellation observed while genuinely blocked mid explicit
// FLOCK() retry must never leave a residual entry in the shared lock-owner
// map -- extending #6453's release-scoping invariant (already proven for
// QUIT and for natural completion) to the cancelled-while-waiting path.
// `SET REPROCESS TO 40` widens the real linear backoff
// (`pause_for_lock_retry`) to a ~820ms window so a `SLEEP 300` cancellation
// trigger lands comfortably inside genuine contention instead of racing a
// ~36ms default window.
// Expected (CURRENT, documented) result: this sequence surfaced a real,
// newly filed defect (#6499): cancellation observed inside an *explicit*
// FLOCK()/RLOCK() retry loop is silently swallowed -- `pause_for_lock_retry`
// runs the cancellation bookkeeping (a `runtime.task.cancelled` event still
// fires) but returns `false` exactly like an ordinary retry-budget timeout,
// so the calling script's `RETURN FLOCK()` completes *normally* with `.F.`
// instead of halting with an error pause. This test therefore asserts
// today's actual behavior (the worker's task reports AWAIT-completed, not
// cancelled) rather than the not-yet-fixed halt semantics, per #6495's
// direction to file focused defects rather than hide or paper over them.
// The retry/no-timeout/no-residual-ownership assertions below remain
// meaningful and correct regardless of #6499: they prove the worker
// genuinely contended, that cancellation still preempted the C++-level
// retry loop before natural exhaustion, and that -- because the swallowed
// cancellation means the worker never actually acquired the lock -- no
// stale ownership is left behind for a later intruder to trip over.
void test_state_sequence_cancellation_during_widened_lock_retry_leaves_no_residual_lock_ownership() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_state_seq_cancel_during_lock_retry";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "state_seq_cancel_during_lock_retry.prg";
    write_text(
        main_path,
        "CREATE TABLE race (name C(12))\n"
        "SELECT race\n"
        "APPEND BLANK\n"
        "REPLACE name WITH 'ORIGINAL1'\n"
        "SET REPROCESS TO 40\n"
        "lParentLock = FLOCK()\n"
        "SPAWN worker TO nWorker\n"
        "SPAWN canceler TO nCancel\n"
        "AWAIT nCancel TO lCancelDone\n"
        "DO WHILE CFTASKSTATUS(nWorker) == 'running'\n"
        "ENDDO\n"
        "AWAIT nWorker TO lWorkerDone\n"
        "UNLOCK\n"
        "SPAWN intruder TO nIntruder\n"
        "DO WHILE CFTASKSTATUS(nIntruder) == 'running'\n"
        "ENDDO\n"
        "lIntruderAcquiredLock = CFTASKRESULT(nIntruder)\n"
        "AWAIT nIntruder TO lIntruderDone\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "SELECT race\n"
        "RETURN FLOCK()\n"
        "ENDPROC\n"
        "PROCEDURE canceler\n"
        "SLEEP 300\n"
        "CANCEL\n"
        "ENDPROC\n"
        "PROCEDURE intruder\n"
        "SELECT race\n"
        "RETURN FLOCK()\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6495 V1b: state-sequence test should complete: " + state.message);

    const auto parent_lock_it = state.globals.find("lparentlock");
    expect(parent_lock_it != state.globals.end() && parent_lock_it->second.boolean_value,
           "#6495 V1b: the parent should successfully acquire FLOCK() before spawning");

    const auto retry_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.lock_retry"; });
    expect(retry_event != state.events.end(),
           "#6495 V1b: the worker's FLOCK() must genuinely contend and begin retrying against the parent's "
           "still-held lock -- otherwise this sequence never reaches the intended mid-wait cancellation");

    const auto timeout_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.lock_timeout"; });
    expect(timeout_event == state.events.end(),
           "#6495 V1b: cancellation should preempt the worker's widened retry budget before it naturally "
           "exhausts -- a timeout event here means the cancellation failed to interrupt genuine contention");

    const auto cancelled_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.task.cancelled"; });
    expect(cancelled_event != state.events.end(),
           "#6495 V1b: the worker's cancellation bookkeeping should still run (see #6499) even though it does "
           "not halt the calling script for an explicit FLOCK() wait");

    const auto worker_done_it = state.globals.find("lworkerdone");
    expect(worker_done_it != state.globals.end() && worker_done_it->second.boolean_value,
           "#6495 V1b: documents #6499's current behavior -- a cancellation observed inside FLOCK()'s explicit "
           "retry loop is swallowed, so the worker's task reports AWAIT-completed (not cancelled/error) with "
           "FLOCK() simply evaluating to .F.; if this ever becomes false, #6499 has likely been fixed and this "
           "assertion (and its comment) should be updated to require the halted/cancelled outcome instead");

    const auto intruder_it = state.globals.find("lintruderacquiredlock");
    expect(intruder_it != state.globals.end() && intruder_it->second.boolean_value,
           "#6495 V1b: once the parent explicitly UNLOCKs, a fresh intruder must be able to acquire the table "
           "immediately -- a residual lock-owner entry left behind by the worker (which never actually acquired "
           "the lock, per #6499) would make this fail or contend and time out");

    const auto intruder_timeout_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.lock_timeout" &&
                                        event.detail.find("intruder") != std::string::npos; });
    expect(intruder_timeout_event == state.events.end(),
           "#6495 V1b: the intruder's post-release FLOCK() should succeed immediately with no residual "
           "contention from the cancelled worker");

    fs::remove_all(temp_root, ignored);
}

// Crossing #2 (cursor/record lock or transaction state x caught
// error/QUIT/shutdown and retry), sequence V2a.
//
// Invariant: after a transaction fails because its implicit record lock
// genuinely contends with a sibling task's held RLOCK(), a caught error plus
// explicit ROLLBACK must leave no journal/transaction-level residue that
// would corrupt or block an immediately following retry of the same logical
// operation.
// Timing: the parent's `SLEEP 10` after SPAWNing the holder is a fixed,
// generous window for the holder's own background thread to start and
// acquire its RLOCK() before the parent's REPLACE ever attempts the same
// record (PRG scripts cannot observe a sibling's variables to synchronize
// more precisely -- SPAWN deep-copies an independent Impl). The holder's own
// `SLEEP 150` then keeps it held well past the parent's default `SET
// REPROCESS` budget (8 attempts, ~36ms cumulative backoff), guaranteeing the
// parent's REPLACE genuinely exhausts its retry budget while the holder
// still owns the record, rather than racing a hold that might release first.
// Expected result: the first transaction attempt fails with a catchable
// error and TXNLEVEL() returns to 0 after ROLLBACK; the retried transaction,
// attempted only after the sibling releases the record, then completes and
// commits cleanly, and no pending transaction journal directory remains.
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
        "SLEEP 10\n"
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
        "SELECT race\n"
        "GO 2\n"
        "lHolderLocked = RLOCK()\n"
        "SLEEP 150\n"
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

// Crossing #2, sequence V2b.
//
// Invariant: an explicit record lock released after a caught error and
// ROLLBACK must be immediately acquirable by a sibling task that was
// genuinely blocked waiting for it; that sibling's own subsequent
// transaction-and-QUIT must in turn release its own newly-acquired record
// lock, extending #6453's QUIT-releases-only-its-own-locks invariant from
// table-level FLOCK() to record-level RLOCK() specifically.
// Note: ROLLBACK's own documented contract governs transaction/journal
// state, not necessarily an explicitly-held RLOCK() (VFP9 historically
// treats explicit record/table locks as independent of transaction
// boundaries). Rather than assume undocumented parity, the parent releases
// its RLOCK() with an explicit UNLOCK right after ROLLBACK, so this sequence
// exercises the caught-error/ROLLBACK/lock-handoff/QUIT crossing without
// depending on an unverified ROLLBACK-releases-explicit-locks assumption.
// `SET REPROCESS TO 40` (inherited by the child at SPAWN time) widens its
// retry budget well past the real time the parent's divide-by-zero fault,
// error-handler dispatch, and actual transaction-journal rollback I/O take,
// so the child reliably outlasts that handling instead of racing a bare
// ~36ms default budget against unpredictable rollback I/O latency.
// Expected result: the child's RLOCK() fails while the parent still holds
// it, succeeds once the parent's UNLOCK releases it, and the parent can
// RLOCK() the same record again immediately after the child's QUIT with no
// timeout.
void test_state_sequence_record_lock_handoff_across_rollback_and_child_quit_leaves_no_residue() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_state_seq_record_lock_handoff";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "state_seq_record_lock_handoff.prg";
    write_text(
        main_path,
        "PUBLIC error_seen, lChildDone, lParentReacquiredAfterChildQuit, cRecordAfterChildQuit\n"
        "error_seen = .F.\n"
        "ON ERROR DO caught\n"
        "CREATE TABLE race (name C(12))\n"
        "SELECT race\n"
        "APPEND BLANK\n"
        "REPLACE name WITH 'ORIGINAL1'\n"
        "SET REPROCESS TO 40\n"
        "BEGIN TRANSACTION\n"
        "lParentLocked = RLOCK()\n"
        "SPAWN child TO nChild\n"
        "SLEEP 10\n"
        "? 1 / 0\n"
        "RETURN\n"
        "PROCEDURE caught\n"
        "error_seen = .T.\n"
        "ROLLBACK\n"
        "UNLOCK\n"
        "AWAIT nChild TO lChildDone\n"
        "lParentReacquiredAfterChildQuit = RLOCK()\n"
        "GO 1\n"
        "cRecordAfterChildQuit = ALLTRIM(name)\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE child\n"
        "SELECT race\n"
        "lChildLocked = RLOCK()\n"
        "IF lChildLocked\n"
        "    BEGIN TRANSACTION\n"
        "    REPLACE name WITH 'CHILD'\n"
        "    END TRANSACTION\n"
        "    QUIT\n"
        "ENDIF\n"
        "RETURN lChildLocked\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6495 V2b: state-sequence test should complete: " + state.message);

    const auto parent_locked_it = state.globals.find("lparentlocked");
    expect(parent_locked_it != state.globals.end() && parent_locked_it->second.boolean_value,
           "#6495 V2b: the parent should successfully RLOCK() record 1 before spawning the contending child");

    const auto error_seen_it = state.globals.find("error_seen");
    expect(error_seen_it != state.globals.end() && error_seen_it->second.boolean_value,
           "#6495 V2b: the deliberate divide-by-zero should raise a catchable error the ON ERROR handler "
           "observes, independent of the lock-handoff mechanics under test");

    const auto child_done_it = state.globals.find("lchilddone");
    expect(child_done_it != state.globals.end() && child_done_it->second.boolean_value,
           "#6495 V2b: the spawned QUIT task should be observed as completed, matching #6453's own QUIT-path "
           "precedent");

    const auto record_it = state.globals.find("crecordafterchildquit");
    expect(record_it != state.globals.end() && record_it->second.string_value == "CHILD",
           "#6495 V2b: the child must have genuinely acquired the record lock once the parent's ROLLBACK+UNLOCK "
           "released it, entered its own transaction, and committed 'CHILD' -- proving the handoff worked, not "
           "just that the child eventually stopped running, got '" +
               (record_it != state.globals.end() ? record_it->second.string_value : "<missing>") + "'");

    const auto parent_reacquired_it = state.globals.find("lparentreacquiredafterchildquit");
    expect(parent_reacquired_it != state.globals.end() && parent_reacquired_it->second.boolean_value,
           "#6495 V2b: after the child QUITs, the parent must be able to RLOCK() the same record again "
           "immediately -- extending #6453's QUIT-releases-only-its-own-locks invariant from table-level "
           "FLOCK() to record-level RLOCK()");

    const auto final_timeout_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.lock_timeout"; });
    expect(final_timeout_event == state.events.end(),
           "#6495 V2b: neither the child's post-rollback RLOCK() nor the parent's post-QUIT RLOCK() should "
           "contend and time out -- any timeout here means a lock was not released when it should have been");

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow

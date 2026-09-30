// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "../src/security/secure_clear.h"
#include "../src/security/secure_file_wipe.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-DBF-NULL-HARDENING-001.
//
// Assigning .NULL. to a nullable field sets its _NullFlags bit (the flag stays authoritative) and
// must not leave the previous value behind: the record slot is blanked, a memo field's pointer is
// cleared and its superseded FPT block is zero-filled (unless another pointer still references the
// block), and the whole-table / memo working buffers that held the old value are wiped.

std::string slurp(const fs::path &path) {
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

bool contains(const std::string &haystack, const std::string &needle) {
    return haystack.find(needle) != std::string::npos;
}

std::size_t occurrences(const std::string &haystack, const std::string &needle) {
    std::size_t count = 0U;
    for (std::size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + 1U)) {
        ++count;
    }
    return count;
}

fs::path fresh_dir(const std::string &name) {
    std::error_code ignored;
    const fs::path dir = fs::temp_directory_path() / ("copperfin_dbf_null_hardening_" + name);
    fs::remove_all(dir, ignored);
    fs::create_directories(dir / "script");
    return dir;
}

bool run_script(const fs::path &dir, const std::string &body, std::string &message) {
    write_text(dir / "script" / "run.prg", body + "RETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "run.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    message = state.message;
    return state.completed;
}

std::uint16_t le_u16(const std::string &bytes, std::size_t at) {
    return static_cast<std::uint16_t>(static_cast<unsigned char>(bytes[at]) |
                                      (static_cast<unsigned char>(bytes[at + 1U]) << 8U));
}

void test_null_blanks_the_dbf_slot_for_every_field_type() {
    const fs::path dir = fresh_dir("slots");
    std::string message;
    const bool ok = run_script(dir,
        "CREATE TABLE secrets (NAME C(12) NULL, AMT N(8,2) NULL, DT D NULL, FLAG L NULL, QTY I NULL)\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'topsecret', AMT WITH 1234.56, DT WITH DATE(2026,1,2), FLAG WITH .T., QTY WITH 987654\n"
        "USE\nUSE secrets\n"
        "REPLACE NAME WITH .NULL., AMT WITH .NULL., DT WITH .NULL., FLAG WITH .NULL., QTY WITH .NULL.\n"
        "STRTOFILE(IIF(ISNULL(NAME),'1','0')+IIF(ISNULL(AMT),'1','0')+IIF(ISNULL(DT),'1','0')+"
        "IIF(ISNULL(FLAG),'1','0')+IIF(ISNULL(QTY),'1','0'), 'flags.txt')\n"
        "USE\n",
        message);
    expect(ok, "null slots: script should complete: " + message);
    expect(read_text(dir / "flags.txt") == "11111", "null slots: every field must still read as .NULL. (the flag is authoritative)");
    const std::string dbf = slurp(dir / "secrets.dbf");
    expect(!contains(dbf, "topsecret"), "null slots: the character value must not remain in the record");
    expect(!contains(dbf, "1234.56"), "null slots: the numeric value must not remain in the record");
    expect(!contains(dbf, "20260102"), "null slots: the date value must not remain in the record");
    const std::size_t header_length = le_u16(dbf, 8U);
    const std::size_t record_start = header_length + 1U;   // past the deletion flag
    const std::string name_slot = dbf.substr(record_start, 12U);
    expect(name_slot == std::string(12U, ' '), "null slots: the C(12) slot should be blank");
    const std::string amt_slot = dbf.substr(record_start + 12U, 8U);
    expect(amt_slot == std::string(8U, ' '), "null slots: the N(8,2) slot should be blank");
    const std::string qty_bytes = std::string("\x06\x12\x0F\x00", 4U);   // 987654 little-endian
    expect(!contains(dbf.substr(record_start), qty_bytes), "null slots: the integer value must not remain in the record");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_assigning_a_value_after_null_clears_the_flag() {
    const fs::path dir = fresh_dir("reassign");
    std::string message;
    const bool ok = run_script(dir,
        "CREATE TABLE t (NAME C(12) NULL)\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH .NULL.\n"
        "USE\nUSE t\n"
        "REPLACE NAME WITH 'back'\n"
        "STRTOFILE(IIF(ISNULL(NAME),'1','0')+ALLTRIM(NAME), 'out.txt')\n"
        "USE\n",
        message);
    expect(ok, "reassign: script should complete: " + message);
    expect(read_text(dir / "out.txt") == "0back", "reassign: a value assigned after .NULL. must read back and clear the flag");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// Two memo rows; nulling the first must leave the second untouched.
const char *kMemoSetup =
    "CREATE TABLE notes (ID N(3), NOTE M NULL)\n"
    "APPEND BLANK\nREPLACE ID WITH 1, NOTE WITH 'memo-secret-text-one'\n"
    "APPEND BLANK\nREPLACE ID WITH 2, NOTE WITH 'keep-me-please'\n"
    "USE\nUSE notes\n";

void test_null_memo_scrubs_the_superseded_block() {
    const fs::path dir = fresh_dir("memo_null");
    std::string message;
    const bool ok = run_script(dir,
        std::string(kMemoSetup) +
        "GO 1\nREPLACE NOTE WITH .NULL.\n"
        "STRTOFILE(IIF(ISNULL(NOTE),'1','0'), 'first.txt')\n"
        "GO 2\nSTRTOFILE(ALLTRIM(NOTE), 'second.txt')\n"
        "USE\n",
        message);
    expect(ok, "memo null: script should complete: " + message);
    expect(read_text(dir / "first.txt") == "1", "memo null: the nulled memo field must read as .NULL. (its flag is now set)");
    expect(read_text(dir / "second.txt") == "keep-me-please", "memo null: the other row's memo must be untouched");
    const std::string fpt = slurp(dir / "notes.fpt");
    expect(!contains(fpt, "memo-secret-text-one"), "memo null: the superseded block must be scrubbed from the FPT");
    expect(contains(fpt, "keep-me-please"), "memo null: the other row's block must remain in the FPT");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_overwriting_a_memo_scrubs_the_old_block() {
    const fs::path dir = fresh_dir("memo_overwrite");
    std::string message;
    const bool ok = run_script(dir,
        "CREATE TABLE notes (NOTE M NULL)\n"
        "APPEND BLANK\nREPLACE NOTE WITH 'old-secret-A'\n"
        "USE\nUSE notes\n"
        "REPLACE NOTE WITH 'new-B'\n"
        "STRTOFILE(ALLTRIM(NOTE), 'out.txt')\n"
        "USE\n",
        message);
    expect(ok, "memo overwrite: script should complete: " + message);
    expect(read_text(dir / "out.txt") == "new-B", "memo overwrite: the new text must read back");
    const std::string fpt = slurp(dir / "notes.fpt");
    expect(!contains(fpt, "old-secret-A"), "memo overwrite: the replaced text must be scrubbed from the FPT");
    expect(contains(fpt, "new-B"), "memo overwrite: the new text must be in the FPT");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_additive_memo_append_scrubs_the_superseded_block() {
    const fs::path dir = fresh_dir("memo_additive");
    std::string message;
    const bool ok = run_script(dir,
        "CREATE TABLE notes (NOTE M NULL)\n"
        "APPEND BLANK\nREPLACE NOTE WITH 'AAAA-secret'\n"
        "USE\nUSE notes\n"
        "REPLACE NOTE WITH ' BBBB' ADDITIVE\n"
        "STRTOFILE(ALLTRIM(NOTE), 'out.txt')\n"
        "USE\n",
        message);
    expect(ok, "memo additive: script should complete: " + message);
    expect(read_text(dir / "out.txt") == "AAAA-secret BBBB", "memo additive: the combined text must read back");
    const std::string fpt = slurp(dir / "notes.fpt");
    expect(occurrences(fpt, "AAAA-secret") == 1U,
        "memo additive: only the new combined block may contain the old text, got " + std::to_string(occurrences(fpt, "AAAA-secret")));
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// A block still referenced by another record must never be scrubbed.
void test_a_shared_memo_block_is_not_scrubbed() {
    const fs::path dir = fresh_dir("memo_shared");
    std::string message;
    bool ok = run_script(dir,
        "CREATE TABLE notes (ID N(3), NOTE M NULL)\n"
        "APPEND BLANK\nREPLACE ID WITH 1, NOTE WITH 'shared-block-text'\n"
        "APPEND BLANK\nREPLACE ID WITH 2\n"
        "USE\n",
        message);
    expect(ok, "memo shared: setup should complete: " + message);
    std::string dbf = slurp(dir / "notes.dbf");
    const std::size_t header_length = le_u16(dbf, 8U);
    const std::size_t record_length = le_u16(dbf, 10U);
    const std::size_t pointer_offset = 1U + 3U;   // deletion flag + ID N(3)
    const std::size_t first = header_length + pointer_offset;
    const std::size_t second = header_length + record_length + pointer_offset;
    for (std::size_t index = 0U; index < 4U; ++index) {
        dbf[second + index] = dbf[first + index];   // record 2 now points at record 1's block
    }
    {
        std::ofstream file(dir / "notes.dbf", std::ios::binary | std::ios::trunc);
        file.write(dbf.data(), static_cast<std::streamsize>(dbf.size()));
    }
    ok = run_script(dir,
        "USE notes\nGO 1\nREPLACE NOTE WITH .NULL.\n"
        "GO 2\nSTRTOFILE(ALLTRIM(NOTE), 'second.txt')\n"
        "USE\n",
        message);
    expect(ok, "memo shared: script should complete: " + message);
    expect(read_text(dir / "second.txt") == "shared-block-text", "memo shared: the other record must still read the shared block");
    expect(contains(slurp(dir / "notes.fpt"), "shared-block-text"), "memo shared: a block another record references must not be scrubbed");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_memo_scrub_can_be_disabled_by_environment() {
    const fs::path dir = fresh_dir("memo_env");
    copperfin::test_support::ScopedEnvironmentValue disable_scrub("COPPERFIN_DBF_MEMO_SCRUB", "off");
    std::string message;
    const bool ok = run_script(dir,
        "CREATE TABLE notes (NOTE M NULL)\n"
        "APPEND BLANK\nREPLACE NOTE WITH 'kept-when-disabled'\n"
        "USE\nUSE notes\nREPLACE NOTE WITH .NULL.\n"
        "STRTOFILE(IIF(ISNULL(NOTE),'1','0'), 'out.txt')\nUSE\n",
        message);
    expect(ok, "memo env: script should complete: " + message);
    expect(read_text(dir / "out.txt") == "1", "memo env: the null flag is set whether or not the scrub runs");
    expect(contains(slurp(dir / "notes.fpt"), "kept-when-disabled"),
        "memo env: COPPERFIN_DBF_MEMO_SCRUB=off must leave the superseded block as VFP9 does");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// Every field type, through the runtime: the field reads as .NULL., the old value is not in the
// record, and (where the type stores inline) the slot holds a canonical blank/zero.
void test_null_blanks_inline_storage_for_every_inline_type() {
    struct Spec { const char *name; const char *decl; const char *value; const char *needle; };
    const Spec specs[] = {
        {"C", "C(12)", "'SECRETVALUE'", "SECRETVALUE"},
        {"N", "N(10,2)", "123456.78", "123456.78"},
        {"F", "F(10,2)", "123456.78", "123456.78"},
        {"D", "D", "DATE(2026,1,2)", "20260102"},
        {"T", "T", "DATETIME(2026,1,2,3,4,5)", ""},
        {"L", "L", ".T.", ""},
        {"I", "I", "987654", ""},
        {"B", "B(4)", "3.14159", ""},
        {"Y", "Y", "1234.5678", ""},
        {"V", "V(20)", "'SECRETVALUE'", "SECRETVALUE"},
        {"Q", "Q(20)", "'SECRETVALUE'", "SECRETVALUE"},
    };
    for (const Spec &spec : specs) {
        const fs::path dir = fresh_dir(std::string("inline_") + spec.name);
        std::string message;
        const bool ok = run_script(dir,
            std::string("CREATE TABLE tt (ID N(3), F ") + spec.decl + " NULL)\n"
            "APPEND BLANK\nREPLACE ID WITH 1, F WITH " + spec.value + "\n"
            "USE\nUSE tt\n"
            "REPLACE F WITH .NULL.\n"
            "STRTOFILE(IIF(ISNULL(F),'NULL','NOTNULL'), 'isnull.txt')\n"
            "USE\n",
            message);
        const std::string label = std::string("inline type ") + spec.name + ": ";
        expect(ok, label + "script should complete: " + message);
        expect(read_text(dir / "isnull.txt") == "NULL", label + "the field must read as .NULL.");
        const std::string dbf = slurp(dir / "tt.dbf");
        if (spec.needle[0] != '\0') {
            expect(!contains(dbf, spec.needle), label + "the old value must not remain in the record");
        }
        // The record is: deletion flag, ID N(3), the field, then the _NullFlags byte. The field's
        // slot must be entirely blank (spaces), zero, or the logical-null '?' -- never the old bytes.
        const std::size_t header_length = le_u16(dbf, 8U);
        const std::size_t record_length = le_u16(dbf, 10U);
        const std::string record = dbf.substr(header_length, record_length);
        const std::string slot = record.substr(4U, record_length - 4U - 1U);
        bool canonical = true;
        for (const char ch : slot) {
            canonical = canonical && (ch == ' ' || ch == '\0' || ch == '?');
        }
        expect(canonical, label + "the slot must be a canonical blank/zero after NULL");
        std::error_code ignored;
        fs::remove_all(dir, ignored);
    }
}

// General ('G'), Picture ('P') and Blob ('W') fields hold a block pointer into the FPT. A Blob is
// created as a Memo and retyped, because the DBF layer has no Blob payload writer of its own.
void test_null_reclaims_the_fpt_block_of_general_picture_and_blob_fields() {
    for (const char type : {'G', 'P', 'W'}) {
        const fs::path dir = fresh_dir(std::string("pointer_") + type);
        const std::string path = (dir / "obj.dbf").string();
        const char create_as = type == 'W' ? 'M' : type;
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
            {.name = "KEEP", .type = 'M', .length = 4U},
            {.name = "OBJ", .type = create_as, .length = 4U, .nullable = true},
        };
        const auto created = copperfin::vfp::create_dbf_table_file(
            path, fields, {{"keep-me-please", std::string("SECRET-") + type + "-PAYLOAD"}});
        const std::string label = std::string("pointer type ") + type + ": ";
        expect(created.ok, label + "fixture should be created: " + created.error);
        if (type == 'W') {
            // Retype the second descriptor M -> W (the type byte is at offset 11 of each descriptor).
            std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
            file.seekp(32 + 32 + 11);
            file.put('W');
        }
        const std::string payload = std::string("SECRET-") + type + "-PAYLOAD";
        expect(contains(slurp(dir / "obj.fpt"), payload), label + "fixture: the payload should be in the FPT");

        const auto nulled = copperfin::vfp::replace_record_field_value(path, 0U, "OBJ", "", false, true);
        expect(nulled.ok, label + "REPLACE ... WITH .NULL. should succeed: " + nulled.error);
        const std::string fpt = slurp(dir / "obj.fpt");
        expect(!contains(fpt, payload), label + "the superseded FPT block must be scrubbed");
        expect(contains(fpt, "keep-me-please"), label + "the other field's block must be untouched");

        const std::string dbf = slurp(path);
        const std::size_t header_length = le_u16(dbf, 8U);
        const std::size_t record_length = le_u16(dbf, 10U);
        const std::size_t obj_offset = header_length + 1U + 4U;   // deletion flag + KEEP pointer
        expect(dbf.substr(obj_offset, 4U) == std::string(4U, '\0'), label + "the pointer must be cleared");
        // The hidden _NullFlags byte is the last byte of the record; OBJ is the only nullable field,
        // so its bit is bit 0.
        const unsigned char flags = static_cast<unsigned char>(dbf[header_length + record_length - 1U]);
        expect((flags & 0x01U) != 0U, label + "the field's _NullFlags bit must be set");
        std::error_code ignored;
        fs::remove_all(dir, ignored);
    }
}

// Assigning a non-null value to a Blob repoints or rewrites its pointer: the superseded block goes.
void test_replacing_a_blob_pointer_reclaims_the_old_block() {
    const fs::path dir = fresh_dir("blob_replace");
    const std::string path = (dir / "blob.dbf").string();
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
        {.name = "OBJ", .type = 'M', .length = 4U, .nullable = true},
    };
    const auto created = copperfin::vfp::create_dbf_table_file(path, fields, {{"BLOB-ORIGINAL-PAYLOAD"}});
    expect(created.ok, "blob replace: fixture should be created: " + created.error);
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(32 + 11);
        file.put('W');
    }
    const auto replaced = copperfin::vfp::replace_record_field_value(path, 0U, "OBJ", "0x00000000", false, false);
    expect(replaced.ok, "blob replace: the opaque assignment should succeed: " + replaced.error);
    expect(!contains(slurp(dir / "blob.fpt"), "BLOB-ORIGINAL-PAYLOAD"),
        "blob replace: the block the old pointer referenced must be scrubbed");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// Table buffering keeps the pre-edit and pending copies of a row; wiping them on TABLEUPDATE/TABLEREVERT
// must not change what those commands do.
void test_buffered_null_keeps_commit_and_revert_semantics() {
    const fs::path dir = fresh_dir("buffered");
    std::string message;
    const bool ok = run_script(dir,
        "CREATE TABLE t (NAME C(12) NULL)\n"
        "APPEND BLANK\nREPLACE NAME WITH 'topsecret'\n"
        "USE\nUSE t\n"
        "CURSORSETPROP('Buffering', 5)\n"
        "REPLACE NAME WITH .NULL.\n"
        "STRTOFILE(IIF(ISNULL(NAME),'1','0'), 'pending.txt')\n"
        "TABLEREVERT(.T.)\n"
        "STRTOFILE(IIF(ISNULL(NAME),'1','0')+ALLTRIM(NAME), 'reverted.txt')\n"
        "REPLACE NAME WITH .NULL.\n"
        "TABLEUPDATE(.T.)\n"
        "STRTOFILE(IIF(ISNULL(NAME),'1','0'), 'committed.txt')\n"
        "USE\n",
        message);
    expect(ok, "buffered null: script should complete: " + message);
    expect(read_text(dir / "pending.txt") == "1", "buffered null: the pending value reads as .NULL.");
    expect(read_text(dir / "reverted.txt") == "0topsecret", "buffered null: TABLEREVERT must restore the original value");
    expect(read_text(dir / "committed.txt") == "1", "buffered null: TABLEUPDATE must commit the .NULL.");
    expect(!contains(slurp(dir / "t.dbf"), "topsecret"), "buffered null: the committed record must not hold the old value");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// Command UNDO keeps a full pre-image of the table (DBF and FPT) so a REPLACE can be undone. That
// pre-image holds exactly the value a NULL assignment supersedes, so it must not outlive the point at
// which the command can still be undone: it is wiped when the session ends, and when the journal entry
// is consumed or dropped.
std::string scan_tree_for(const fs::path &root, const std::string &needle) {
    std::error_code ignored;
    if (!fs::exists(root, ignored)) {
        return {};
    }
    for (const auto &entry : fs::recursive_directory_iterator(root, ignored)) {
        if (entry.is_regular_file(ignored) && contains(slurp(entry.path()), needle)) {
            return entry.path().string();
        }
    }
    return {};
}

void test_command_undo_backups_do_not_keep_the_superseded_value() {
    const fs::path dir = fresh_dir("undo_backup");
    const fs::path runtime_temp = dir / "runtime_temp";
    fs::create_directories(runtime_temp);
    const std::string body =
        "CREATE TABLE notes (NAME C(12) NULL, NOTE M NULL)\n"
        "APPEND BLANK\nREPLACE NAME WITH 'topsecret', NOTE WITH 'memo-secret-text'\n"
        "USE\nUSE notes\n"
        "REPLACE NAME WITH .NULL., NOTE WITH .NULL.\n"
        "USE\n";
    write_text(dir / "script" / "run.prg", body + "RETURN\n");
    std::string alive_hit_name;
    std::string alive_hit_memo;
    {
        auto options = make_runtime_session_options((dir / "script" / "run.prg").string(), dir.string(), false);
        options.temp_directory = runtime_temp.string();
        auto session = copperfin::runtime::PrgRuntimeSession::create(options);
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "undo backup: script should complete: " + state.message);
        // While the session is alive the pre-image is still needed for UNDO and is allowed to exist.
        alive_hit_name = scan_tree_for(runtime_temp, "topsecret");
        alive_hit_memo = scan_tree_for(runtime_temp, "memo-secret-text");
    }
    expect(scan_tree_for(runtime_temp, "topsecret").empty(),
        "undo backup: no runtime-temp file may keep the old character value after the session ends, found " +
            scan_tree_for(runtime_temp, "topsecret") + " (alive: " + alive_hit_name + ")");
    expect(scan_tree_for(runtime_temp, "memo-secret-text").empty(),
        "undo backup: no runtime-temp file may keep the old memo text after the session ends, found " +
            scan_tree_for(runtime_temp, "memo-secret-text") + " (alive: " + alive_hit_memo + ")");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// BEGIN TRANSACTION backs the table up the same way; commit and rollback must not leave that backup
// (which holds the pre-transaction value) behind in the temp directory.
void test_transaction_backups_do_not_keep_the_superseded_value() {
    for (const char *ending : {"END TRANSACTION", "ROLLBACK"}) {
        const std::string mode = std::string(ending) == "ROLLBACK" ? "rollback" : "commit";
        const fs::path dir = fresh_dir("txn_" + mode);
        const fs::path runtime_temp = dir / "runtime_temp";
        fs::create_directories(runtime_temp);
        write_text(dir / "script" / "run.prg",
            "CREATE TABLE notes (NAME C(12) NULL, NOTE M NULL)\n"
            "APPEND BLANK\nREPLACE NAME WITH 'txnsecret', NOTE WITH 'txn-memo-secret'\n"
            "USE\nUSE notes\n"
            "BEGIN TRANSACTION\n"
            "REPLACE NAME WITH .NULL., NOTE WITH .NULL.\n" + std::string(ending) + "\n"
            "USE\nRETURN\n");
        {
            auto options = make_runtime_session_options((dir / "script" / "run.prg").string(), dir.string(), false);
            options.temp_directory = runtime_temp.string();
            auto session = copperfin::runtime::PrgRuntimeSession::create(options);
            const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.completed, "transaction " + mode + ": script should complete: " + state.message);
        }
        expect(scan_tree_for(runtime_temp, "txnsecret").empty(),
            "transaction " + mode + ": no runtime-temp file may keep the old value, found " + scan_tree_for(runtime_temp, "txnsecret"));
        expect(scan_tree_for(runtime_temp, "txn-memo-secret").empty(),
            "transaction " + mode + ": no runtime-temp file may keep the old memo text, found " + scan_tree_for(runtime_temp, "txn-memo-secret"));
        std::error_code ignored;
        fs::remove_all(dir, ignored);
    }
}

void test_secure_remove_tree_deletes_files_and_never_follows_symlinks() {
    const fs::path dir = fresh_dir("remove_tree");
    const fs::path tree = dir / "tree";
    fs::create_directories(tree / "nested");
    write_text(tree / "a.dbf", std::string(200000U, 'S'));   // larger than one wipe chunk
    write_text(tree / "nested" / "b.fpt", "secret");
    write_text(dir / "outside.txt", "must-survive");
    std::error_code link_error;
#if !defined(_WIN32)
    fs::create_symlink(dir / "outside.txt", tree / "link.txt", link_error);
#else
    link_error = std::make_error_code(std::errc::operation_not_supported);
#endif
    copperfin::security::secure_remove_tree(tree);
    expect(!fs::exists(tree), "secure_remove_tree must delete the whole tree");
    expect(read_text(dir / "outside.txt") == "must-survive",
        "secure_remove_tree must never follow a symlink out of the tree" + std::string(link_error ? " (no symlinks on this host)" : ""));
    copperfin::security::secure_remove_tree(dir / "does-not-exist");   // must not throw
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// A memo pointer may be wider than four bytes (M(10) fixtures exist); the shared-block check must
// see it, or replacing another field could scrub a block the wider field still references.
void test_a_block_referenced_by_a_wide_pointer_is_not_scrubbed() {
    const fs::path dir = fresh_dir("wide_pointer");
    const std::string path = (dir / "wide.dbf").string();
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
        {.name = "WIDE", .type = 'M', .length = 10U},
        {.name = "NARROW", .type = 'M', .length = 4U, .nullable = true},
    };
    const auto created = copperfin::vfp::create_dbf_table_file(path, fields, {{"wide-payload-kept", "narrow-payload-gone"}});
    expect(created.ok, "wide pointer: fixture should be created: " + created.error);
    std::string dbf = slurp(dir / "wide.dbf");
    const std::size_t header_length = le_u16(dbf, 8U);
    const std::size_t wide_offset = header_length + 1U;
    const std::size_t narrow_offset = header_length + 1U + 10U;
    for (std::size_t index = 0U; index < 4U; ++index) {
        dbf[narrow_offset + index] = dbf[wide_offset + index];   // NARROW now shares WIDE's block
    }
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file.write(dbf.data(), static_cast<std::streamsize>(dbf.size()));
    }
    const auto nulled = copperfin::vfp::replace_record_field_value(path, 0U, "NARROW", "", false, true);
    expect(nulled.ok, "wide pointer: nulling NARROW should succeed: " + nulled.error);
    expect(contains(slurp(dir / "wide.fpt"), "wide-payload-kept"),
        "wide pointer: a block still referenced by an M(10) field must not be scrubbed");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// The scrub trusts nothing in the FPT block header: an inflated length must not let it zero the
// neighbouring blocks, including the one just written for the replacement.
void test_an_inflated_fpt_block_length_does_not_reach_other_blocks() {
    for (const bool past_end : {false, true}) {
        const fs::path dir = fresh_dir(past_end ? "inflated_past_end" : "inflated_in_file");
        const std::string path = (dir / "two.dbf").string();
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
            {.name = "NOTE", .type = 'M', .length = 4U, .nullable = true},
        };
        const auto created = copperfin::vfp::create_dbf_table_file(path, fields, {{"first-payload-text"}, {"second-payload-kept"}});
        expect(created.ok, "inflated length: fixture should be created: " + created.error);
        const std::string dbf = slurp(dir / "two.dbf");
        const std::size_t header_length = le_u16(dbf, 8U);
        const std::size_t record_length = le_u16(dbf, 10U);
        const auto block_of = [&](std::size_t record) {
            const std::size_t at = header_length + record * record_length + 1U;
            return static_cast<std::size_t>(static_cast<unsigned char>(dbf[at])) |
                   (static_cast<std::size_t>(static_cast<unsigned char>(dbf[at + 1U])) << 8U);
        };
        std::string fpt = slurp(dir / "two.fpt");
        const std::size_t block_size = (static_cast<unsigned char>(fpt[6]) << 8U) | static_cast<unsigned char>(fpt[7]);
        const std::size_t first_offset = block_of(0U) * block_size;
        // Claim a payload far longer than the real one: either running past the end of the file, or
        // just long enough to cover the next block.
        const std::uint32_t claimed = past_end ? 0x00FFFFFFU : static_cast<std::uint32_t>(block_size + 100U);
        fpt[first_offset + 4U] = static_cast<char>((claimed >> 24U) & 0xFFU);
        fpt[first_offset + 5U] = static_cast<char>((claimed >> 16U) & 0xFFU);
        fpt[first_offset + 6U] = static_cast<char>((claimed >> 8U) & 0xFFU);
        fpt[first_offset + 7U] = static_cast<char>(claimed & 0xFFU);
        {
            std::ofstream file(dir / "two.fpt", std::ios::binary | std::ios::trunc);
            file.write(fpt.data(), static_cast<std::streamsize>(fpt.size()));
        }
        const auto nulled = copperfin::vfp::replace_record_field_value(path, 0U, "NOTE", "", false, true);
        expect(nulled.ok, "inflated length: nulling should succeed: " + nulled.error);
        const std::string after = slurp(dir / "two.fpt");
        const std::string label = std::string("inflated length (") + (past_end ? "past the end" : "into the next block") + "): ";
        expect(contains(after, "second-payload-kept"), label + "the neighbouring block must survive an untrusted length");
        std::error_code ignored;
        fs::remove_all(dir, ignored);
    }
}

void test_secure_clear_zero_fills_buffers() {
    std::vector<std::uint8_t> bytes(64U, 0xAAU);
    copperfin::security::secure_clear(bytes);
    bool all_zero = true;
    for (const volatile std::uint8_t &byte : bytes) {
        all_zero = all_zero && byte == 0U;
    }
    expect(all_zero, "secure_clear(vector) must zero every byte");
    expect(bytes.size() == 64U, "secure_clear must not change the buffer size");

    std::string text(32U, 'x');
    copperfin::security::secure_clear(text);
    expect(text == std::string(32U, '\0'), "secure_clear(string) must zero every character");

    std::vector<std::uint8_t> guarded(16U, 0x5AU);
    std::vector<std::uint8_t> inactive(16U, 0x5AU);
    {
        copperfin::security::SecureClearGuard active_guard;
        active_guard.add(guarded);
        copperfin::security::SecureClearGuard inactive_guard;
        inactive_guard.set_active(false);
        inactive_guard.add(inactive);
        expect(guarded[0] == 0x5AU, "a guard must not clear before it leaves scope");
    }
    expect(guarded == std::vector<std::uint8_t>(16U, 0U), "a guard must wipe its buffers when it leaves scope");
    expect(inactive == std::vector<std::uint8_t>(16U, 0x5AU), "an inactive guard must leave its buffers alone");

    copperfin::security::secure_clear(nullptr, 8U);   // must not crash
    copperfin::security::secure_clear(bytes.data(), 0U);
}

}  // namespace

int main() {
    // The scrub switch is read from the environment; a value inherited from the caller (for example
    // COPPERFIN_DBF_MEMO_SCRUB=off) must not change the default-behavior cases below. The variable is
    // cleared for the whole run and the caller's value is restored on exit.
    copperfin::test_support::ScopedEnvironmentValue isolated_scrub_setting("COPPERFIN_DBF_MEMO_SCRUB");
    test_null_blanks_the_dbf_slot_for_every_field_type();
    test_assigning_a_value_after_null_clears_the_flag();
    test_null_memo_scrubs_the_superseded_block();
    test_overwriting_a_memo_scrubs_the_old_block();
    test_additive_memo_append_scrubs_the_superseded_block();
    test_a_shared_memo_block_is_not_scrubbed();
    test_memo_scrub_can_be_disabled_by_environment();
    test_null_blanks_inline_storage_for_every_inline_type();
    test_null_reclaims_the_fpt_block_of_general_picture_and_blob_fields();
    test_replacing_a_blob_pointer_reclaims_the_old_block();
    test_buffered_null_keeps_commit_and_revert_semantics();
    test_command_undo_backups_do_not_keep_the_superseded_value();
    test_transaction_backups_do_not_keep_the_superseded_value();
    test_secure_remove_tree_deletes_files_and_never_follows_symlinks();
    test_a_block_referenced_by_a_wide_pointer_is_not_scrubbed();
    test_an_inflated_fpt_block_length_does_not_reach_other_blocks();
    test_secure_clear_zero_fills_buffers();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_cursor_use_and_seek_errors_use_default_locale_messages() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_cursor_seek_errors";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const auto run_error_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script);
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    const auto duplicate_alias = run_error_script(
        "duplicate_alias",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS People AGAIN IN 0\n");
    expect(duplicate_alias.reason == copperfin::runtime::DebugPauseReason::error,
        "duplicate alias USE should pause with an error");
    expect(duplicate_alias.message == "Alias already open in this data session: People",
        "duplicate alias USE error should interpolate alias through the default locale catalog");

    const auto duplicate_table = run_error_script(
        "duplicate_table",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other IN 0\n");
    expect(duplicate_table.reason == copperfin::runtime::DebugPauseReason::error,
        "duplicate table USE without AGAIN should pause with an error");
    expect(
        duplicate_table.message == "Table already open in this data session; USE AGAIN is required: " + table_path.string(),
        "duplicate table USE error should interpolate path through the default locale catalog");

    const std::string overflowing_area = "999999999999999999999";
    const auto overflowing_select = run_error_script(
        "overflowing_select",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT '" + overflowing_area + "'\n");
    expect(overflowing_select.reason == copperfin::runtime::DebugPauseReason::error,
        "SELECT with an overflowing numeric work-area designator should pause with an error");
    expect(overflowing_select.message == "SELECT target work area not found: " + overflowing_area,
        "SELECT overflow should use the established target-work-area diagnostic");

    const auto overflowing_use = run_error_script(
        "overflowing_use",
        "USE '" + table_path.string() + "' ALIAS People IN '" + overflowing_area + "'\n");
    expect(overflowing_use.reason == copperfin::runtime::DebugPauseReason::error,
        "USE with an overflowing numeric work-area designator should pause with an error");
    expect(overflowing_use.message == "USE target work area not found: " + overflowing_area,
        "USE overflow should use the established target-work-area diagnostic");

    const auto overflowing_target_functions = run_error_script(
        "overflowing_target_functions",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "nCount = RECCOUNT('" + overflowing_area + "')\n"
        "nRecno = RECNO('" + overflowing_area + "')\n"
        "RETURN\n");
    expect(overflowing_target_functions.completed,
        "targeted cursor functions should fail closed for overflowing work-area designators");
    const auto count = overflowing_target_functions.globals.find("ncount");
    const auto recno = overflowing_target_functions.globals.find("nrecno");
    expect(count != overflowing_target_functions.globals.end() &&
               copperfin::runtime::format_value(count->second) == "0",
        "RECCOUNT should return zero for an overflowing work-area designator");
    expect(recno != overflowing_target_functions.globals.end() &&
               copperfin::runtime::format_value(recno->second) == "0",
        "RECNO should return zero for an overflowing work-area designator");

    const auto missing_order = run_error_script(
        "missing_order",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 2\n");
    expect(missing_order.reason == copperfin::runtime::DebugPauseReason::error,
        "SET ORDER to a missing numeric order should pause with an error");
    expect(missing_order.message == "Requested order does not exist",
        "missing order error should route through the default locale catalog");

    const auto overflowing_order = run_error_script(
        "overflowing_order",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 999999999999999999999\n");
    expect(overflowing_order.reason == copperfin::runtime::DebugPauseReason::error,
        "SET ORDER with an overflowing numeric selector should pause with an error");
    expect(overflowing_order.message == "Requested order does not exist",
        "SET ORDER overflow should use the existing missing-order diagnostic");

    const auto missing_tag = run_error_script(
        "missing_tag",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SET ORDER TO TAG doesnotexist\n");
    expect(missing_tag.reason == copperfin::runtime::DebugPauseReason::error,
        "SET ORDER TO TAG with an unknown tag should pause with an error");
    expect(missing_tag.message == "Requested order does not exist",
        "missing TAG order error should route through the default locale catalog");
    expect(
        std::count_if(missing_tag.events.begin(), missing_tag.events.end(), [](const auto& event) {
            return event.category == "runtime.order";
        }) == 1,
        "unknown TAG requests should not emit a bogus fallback runtime.order event");

    const auto missing_locate = run_error_script(
        "missing_locate",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "CONTINUE\n");
    expect(missing_locate.reason == copperfin::runtime::DebugPauseReason::error,
        "CONTINUE without an active LOCATE should pause with an error");
    expect(missing_locate.message == "CONTINUE requires an active LOCATE command",
        "CONTINUE without an active LOCATE should route through the default locale catalog");

    fs::remove_all(temp_root, ignored);
}

void test_sql_runtime_errors_localize_without_changing_runtime_behavior() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_sql_error_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script);
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    const auto spanish_missing_command = run_script(
        "sql_missing_command_es",
        "DIMENSION aSqlErr[1]\n"
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn)\n"
        "nRows = AERROR(aSqlErr)\n"
        "cSqlMessage = aSqlErr[1,2]\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");
    expect(spanish_missing_command.completed,
           "#2596: es-419 SQL missing-command script should complete");
    const auto spanish_sql_message = spanish_missing_command.globals.find("csqlmessage");
    expect(spanish_sql_message != spanish_missing_command.globals.end() &&
               copperfin::runtime::format_value(spanish_sql_message->second) ==
                   "SQLEXEC requiere un comando o una instruccion SQL preparada",
           "#2596: es-419 SQLEXEC missing-command error should localize the prose (got '" +
               (spanish_sql_message != spanish_missing_command.globals.end()
                    ? copperfin::runtime::format_value(spanish_sql_message->second)
                    : std::string("<missing>")) +
               "')");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    const auto portuguese_missing_handle = run_script(
        "sql_missing_handle_pt",
        "DIMENSION aSqlErr[1]\n"
        "nExec = SQLEXEC(7, 'select * from customers')\n"
        "nRows = AERROR(aSqlErr)\n"
        "cSqlMessage = aSqlErr[1,2]\n"
        "RETURN\n");
    expect(portuguese_missing_handle.completed,
           "#2596: pt-BR SQL missing-handle script should complete");
    const auto portuguese_sql_message = portuguese_missing_handle.globals.find("csqlmessage");
    expect(portuguese_sql_message != portuguese_missing_handle.globals.end() &&
               copperfin::runtime::format_value(portuguese_sql_message->second) == "Handle SQL nao encontrado: 7",
           "#2596: pt-BR SQL missing-handle error should localize the prose while preserving the handle value (got '" +
               (portuguese_sql_message != portuguese_missing_handle.globals.end()
                    ? copperfin::runtime::format_value(portuguese_sql_message->second)
                    : std::string("<missing>")) +
               "')");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto pseudo_missing_command = run_script(
        "sql_missing_command_qps",
        "DIMENSION aSqlErr[1]\n"
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn)\n"
        "nRows = AERROR(aSqlErr)\n"
        "cSqlMessage = aSqlErr[1,2]\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");
    expect(pseudo_missing_command.completed,
           "#2596: qps-ploc SQL missing-command script should complete");
    const auto pseudo_sql_message = pseudo_missing_command.globals.find("csqlmessage");
    expect(pseudo_sql_message != pseudo_missing_command.globals.end() &&
               copperfin::runtime::format_value(pseudo_sql_message->second) ==
                   copperfin::localization::pseudo_localize("SQLEXEC requires a command or a prepared SQL statement"),
           "#2596: qps-ploc SQLEXEC missing-command error should resolve through the pseudo-localization transform (got '" +
               (pseudo_sql_message != pseudo_missing_command.globals.end()
                    ? copperfin::runtime::format_value(pseudo_sql_message->second)
                    : std::string("<missing>")) +
               "')");

    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow

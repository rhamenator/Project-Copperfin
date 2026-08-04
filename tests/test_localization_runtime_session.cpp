// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_localization_support.h"

void test_runtime_session_diagnostics_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.CommandUndo.Error.NoCommand") == "No command to UNDO",
        "#2607: command UNDO empty-stack error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.CriticalSection.Error.UnknownSection", {{"section", "orders"}}) ==
            "Unknown critical section: orders",
        "#2607: critical-section unknown-name error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.JournalInitializeFailed") ==
            "Unable to initialize transaction journal",
        "#2607: transaction journal initialize error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.JournalStatePersistFailed") ==
            "Unable to persist transaction journal state",
        "#2607: transaction journal state persist error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.BackupCreateFailed", {{"path", "fixtures/people.dbf"}}) ==
            "Unable to create transaction backup for: fixtures/people.dbf",
        "#2607: transaction backup error should preserve the named path placeholder in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.BackupJournalPersistFailed") ==
            "Unable to persist transaction backup journal",
        "#2607: transaction backup journal persist error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.JournalReplayFailed") ==
            "Failed to replay transaction journal",
        "#2607: transaction replay error should remain catalog-backed in en-US");

    const std::string spanish_command_undo =
        spanish.translate("Runtime.Prg.CommandUndo.Error.BackupCreateFailed", {{"path", "fixtures/people.dbf"}});
    expect(
        spanish_command_undo ==
            "No se pudo crear el respaldo de undo del comando para: fixtures/people.dbf",
        "#2607: es-419 command UNDO backup error should localize the prose while preserving the path");
    expect(
        spanish_command_undo.find("fixtures/people.dbf") != std::string::npos &&
            spanish_command_undo.find("Unable to create command undo backup") == std::string::npos,
        "#2607: es-419 command UNDO backup error should preserve the path without falling back to English prose");

    const std::string spanish_critical_section = spanish.translate(
        "Runtime.Prg.CriticalSection.Error.BlockingOperation",
        {{"operation", "WAIT"}, {"section", "orders"}});
    expect(
        spanish_critical_section ==
            "La operacion bloqueante WAIT no esta permitida mientras se mantiene la seccion CRITICAL orders",
        "#2607: es-419 critical-section blocking-operation error should localize the prose while preserving invariant tokens");
    expect(
        spanish_critical_section.find("WAIT") != std::string::npos &&
            spanish_critical_section.find("CRITICAL") != std::string::npos &&
            spanish_critical_section.find("Blocking operation") == std::string::npos,
        "#2607: es-419 critical-section blocking-operation error should preserve invariant tokens without falling back to English prose");

    const std::string portuguese_command_undo =
        portuguese.translate("Runtime.Prg.CommandUndo.Error.NoCommand");
    expect(
        portuguese_command_undo == "Nao ha comando para UNDO",
        "#2607: pt-BR command UNDO empty-stack error should localize the prose");

    const std::string portuguese_transaction =
        portuguese.translate("Runtime.Prg.Transaction.Error.JournalReplayFailed");
    expect(
        portuguese_transaction == "Falha ao reproduzir o diario da transacao",
        "#2607: pt-BR transaction replay error should localize the prose");

    const std::string pseudo_critical_section = pseudo.translate(
        "Runtime.Prg.CriticalSection.Error.MutexNotFound",
        {{"section", "orders"}});
    expect(
        pseudo_critical_section.find("[!! ") == 0U &&
            pseudo_critical_section.find("orders") != std::string::npos &&
            pseudo_critical_section.find("Critical section mutex not found: orders") == std::string::npos,
        "#2607: qps-ploc critical-section mutex error should pseudo-localize prose while preserving the section name");

    const std::string pseudo_transaction =
        pseudo.translate("Runtime.Prg.Transaction.Error.BackupCreateFailed", {{"path", "fixtures/people.dbf"}});
    expect(
        pseudo_transaction.find("[!! ") == 0U &&
            pseudo_transaction.find("fixtures/people.dbf") != std::string::npos &&
        pseudo_transaction.find("Unable to create transaction backup for: fixtures/people.dbf") == std::string::npos,
        "#2607: qps-ploc transaction backup error should pseudo-localize prose while preserving the path");
}

void test_runtime_cursor_diagnostics_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Cursor.Error.AliasAlreadyOpen", {{"alias", "People"}}) ==
            "Alias already open in this data session: People",
        "#2608: cursor duplicate-alias error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Cursor.Error.TableAlreadyOpenUseAgainRequired", {{"path", "fixtures/people.dbf"}}) ==
            "Table already open in this data session; USE AGAIN is required: fixtures/people.dbf",
        "#2608: cursor duplicate-table error should preserve the named path placeholder in en-US");
    expect(
        english.translate("Runtime.Prg.Cursor.Error.UseTargetWorkAreaNotFound", {{"target", "42"}}) ==
            "USE target work area not found: 42",
        "#2608: cursor missing-work-area error should remain catalog-backed in en-US");

    const std::string spanish_alias =
        spanish.translate("Runtime.Prg.Cursor.Error.AliasAlreadyOpen", {{"alias", "People"}});
    expect(
        spanish_alias == "El alias ya esta abierto en esta sesion de datos: People",
        "#2608: es-419 cursor duplicate-alias error should localize the prose while preserving the alias");
    expect(
        spanish_alias.find("People") != std::string::npos &&
            spanish_alias.find("Alias already open in this data session") == std::string::npos,
        "#2608: es-419 cursor duplicate-alias error should preserve the alias without falling back to English prose");

    const std::string spanish_seek =
        spanish.translate("Runtime.Prg.Cursor.Error.SeekRequiresActiveOrder");
    expect(
        spanish_seek == "SEEK requiere un orden activo",
        "#2608: es-419 cursor active-order error should localize the prose");
    expect(
        spanish_seek.find("SEEK") != std::string::npos &&
            spanish_seek.find("requires an active order") == std::string::npos,
        "#2608: es-419 cursor active-order error should preserve SEEK without falling back to English prose");

    const std::string portuguese_use_target =
        portuguese.translate("Runtime.Prg.Cursor.Error.UseTargetResolveFailed", {{"path", "fixtures/missing.dbf"}});
    expect(
        portuguese_use_target == "Nao foi possivel resolver o destino de USE: fixtures/missing.dbf",
        "#2608: pt-BR cursor USE-target resolve error should localize the prose while preserving the path");

    const std::string portuguese_local_seek =
        portuguese.translate("Runtime.Prg.Cursor.Error.SeekRequiresLocalTableBackedCursor");
    expect(
        portuguese_local_seek == "SEEK exige um cursor local com suporte de tabela",
        "#2608: pt-BR cursor local-table requirement should localize the prose");

    const std::string pseudo_use_again = pseudo.translate(
        "Runtime.Prg.Cursor.Error.TableAlreadyOpenUseAgainRequired",
        {{"path", "fixtures/people.dbf"}});
    expect(
        pseudo_use_again.find("[!! ") == 0U &&
            pseudo_use_again.find("fixtures/people.dbf") != std::string::npos &&
            pseudo_use_again.find("Table already open in this data session; USE AGAIN is required: fixtures/people.dbf") == std::string::npos,
        "#2608: qps-ploc cursor duplicate-table error should pseudo-localize prose while preserving the path");

    const std::string pseudo_work_area =
        pseudo.translate("Runtime.Prg.Cursor.Error.UseTargetWorkAreaNotFound", {{"target", "42"}});
    expect(
        pseudo_work_area.find("[!! ") == 0U &&
            pseudo_work_area.find("42") != std::string::npos &&
        pseudo_work_area.find("USE target work area not found: 42") == std::string::npos,
        "#2608: qps-ploc cursor missing-work-area error should pseudo-localize prose while preserving the target");
}

void test_runtime_total_diagnostics_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Total.Error.RequiresToTarget") == "TOTAL requires a TO target",
        "#2609: TOTAL missing-target error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Total.Error.RequiresOnField") == "TOTAL requires an ON field",
        "#2609: TOTAL missing-ON-field error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Total.Error.FieldNotFound", {{"fieldName", "MISSING"}}) ==
            "TOTAL field was not found: MISSING",
        "#2609: TOTAL missing-field error should preserve the named field placeholder in en-US");

    const std::string spanish_target =
        spanish.translate("Runtime.Prg.Total.Error.RequiresSelectedWorkArea");
    expect(
        spanish_target == "TOTAL requiere un area de trabajo seleccionada",
        "#2609: es-419 TOTAL missing-work-area error should localize the prose");
    expect(
        spanish_target.find("TOTAL") != std::string::npos &&
            spanish_target.find("requires a selected work area") == std::string::npos,
        "#2609: es-419 TOTAL missing-work-area error should preserve TOTAL without falling back to English prose");

    const std::string spanish_field =
        spanish.translate("Runtime.Prg.Total.Error.FieldNotFound", {{"fieldName", "MISSING"}});
    expect(
        spanish_field == "No se encontro el campo de TOTAL: MISSING",
        "#2609: es-419 TOTAL missing-field error should localize the prose while preserving the field name");
    expect(
        spanish_field.find("MISSING") != std::string::npos &&
            spanish_field.find("TOTAL field was not found") == std::string::npos,
        "#2609: es-419 TOTAL missing-field error should preserve the field name without falling back to English prose");

    const std::string portuguese_numeric =
        portuguese.translate("Runtime.Prg.Total.Error.RequiresNumericField");
    expect(
        portuguese_numeric == "TOTAL exige pelo menos um campo numerico para totalizar",
        "#2609: pt-BR TOTAL numeric-field requirement should localize the prose");

    const std::string portuguese_local_cursor =
        portuguese.translate("Runtime.Prg.Total.Error.RequiresLocalTableBackedCursor");
    expect(
        portuguese_local_cursor == "TOTAL exige um cursor local com suporte de tabela",
        "#2609: pt-BR TOTAL local-cursor requirement should localize the prose");

    const std::string pseudo_only_numeric =
        pseudo.translate("Runtime.Prg.Total.Error.OnlyNumericFields");
    expect(
        pseudo_only_numeric.find("[!! ") == 0U &&
            pseudo_only_numeric.find("TOTAL only supports numeric FIELDS in the first pass") == std::string::npos,
        "#2609: qps-ploc TOTAL numeric-fields error should pseudo-localize prose");

    const std::string pseudo_missing_field =
        pseudo.translate("Runtime.Prg.Total.Error.FieldNotFound", {{"fieldName", "MISSING"}});
    expect(
        pseudo_missing_field.find("[!! ") == 0U &&
            pseudo_missing_field.find("MISSING") != std::string::npos &&
            pseudo_missing_field.find("TOTAL field was not found: MISSING") == std::string::npos,
        "#2609: qps-ploc TOTAL missing-field error should pseudo-localize prose while preserving the field name");
}

void test_runtime_report_output_messages_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.ReportOutput.Error.PathRequired") ==
            "REPORT/LABEL TO clause requires a writable output path",
        "#2536: report output path-required error should be catalog-backed");
    expect(
        english.translate("Runtime.Prg.ReportOutput.Error.OpenFailed", {{"path", "renders/invoice.txt"}}) ==
            "Unable to open report output path: renders/invoice.txt",
        "#2536: report output open error should preserve the named path placeholder");
    expect(
        english.translate("Runtime.Prg.ReportOutput.Error.WriteFailed", {{"path", "renders/invoice.txt"}}) ==
            "Unable to write report output path: renders/invoice.txt",
        "#2536: report output write error should preserve the named path placeholder");
    expect(
        english.translate("Runtime.Prg.ReportAsset.Error.ResolveFailed", {{"path", "reports/missing_invoice.frx"}}) ==
            "Unable to resolve report asset: reports/missing_invoice.frx",
        "#2597: report asset resolve error should be catalog-backed");

    const std::string spanish_path_required =
        spanish.translate("Runtime.Prg.ReportOutput.Error.PathRequired");
    expect(
        spanish_path_required == "La clausula TO de REPORT/LABEL requiere una ruta de salida escribible",
        "#2597: es-419 report output path-required error should localize the prose");
    expect(
        spanish_path_required.find("REPORT/LABEL") != std::string::npos &&
            spanish_path_required.find("TO") != std::string::npos &&
            spanish_path_required.find("requires a writable output path") == std::string::npos,
        "#2597: es-419 report output path-required error should preserve command tokens without falling back to English prose");

    const std::string portuguese_resolve =
        portuguese.translate("Runtime.Prg.ReportAsset.Error.ResolveFailed", {{"path", "reports/missing_invoice.frx"}});
    expect(
        portuguese_resolve == "Nao foi possivel resolver o asset do relatorio: reports/missing_invoice.frx",
        "#2597: pt-BR report asset resolve error should localize the prose while preserving the path");

    const std::string pseudo_open =
        pseudo.translate("Runtime.Prg.ReportOutput.Error.OpenFailed", {{"path", "renders/invoice.txt"}});
    expect(
        pseudo_open.find("[!! ") == 0U &&
            pseudo_open.find("renders/invoice.txt") != std::string::npos &&
            pseudo_open.find("Unable to open report output path") == std::string::npos,
        "#2597: qps-ploc report output open error should pseudo-localize prose while preserving the path");
}

std::vector<std::uint8_t> make_vfp_header() {
    std::vector<std::uint8_t> bytes(32U, 0U);
    bytes[0] = 0x30U;
    bytes[1] = 126U;
    bytes[2] = 4U;
    bytes[3] = 7U;
    bytes[8] = 0xA1U;
    bytes[9] = 0x00U;
    bytes[10] = 0x40U;
    bytes[11] = 0x00U;
    bytes[28] = 0x01U;
    bytes[29] = 0x03U;
    return bytes;
}

void write_synthetic_vfp_asset(const std::filesystem::path& path) {
    const auto bytes = make_vfp_header();
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void test_runtime_report_output_errors_localize_without_changing_runtime_behavior() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_report_output_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_vfp_asset(report_path);

    auto run_script = [&](const std::string& stem, const std::string& script) {
        const fs::path main_path = temp_root / (stem + ".prg");
        write_text(main_path, script);
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "es-419", true);
        const auto state = run_script(
            "report_missing_path_es",
            "REPORT FORM '" + report_path.string() + "' TO FILE\n");
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "#2597: es-419 report output missing-path script should pause with an error");
        expect(state.message == "La clausula TO de REPORT/LABEL requiere una ruta de salida escribible",
               "#2597: es-419 report output missing-path error should localize the prose (got '" + state.message + "')");
    }

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
        const fs::path blocking_parent = temp_root / "not_a_directory";
        write_text(blocking_parent, "block");
        const auto state = run_script(
            "report_open_failed_pt",
            "REPORT FORM '" + report_path.string() + "' TO FILE '" + (blocking_parent / "invoice.txt").string() + "'\n");
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "#2597: pt-BR report output open-failed script should pause with an error");
        expect(state.message == "Nao foi possivel abrir o caminho de saida do relatorio: " + (blocking_parent / "invoice.txt").string(),
               "#2597: pt-BR report output open-failed error should localize the prose while preserving the path");
    }

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
        const auto state = run_script(
            "report_missing_asset_qps",
            "REPORT FORM '" + (temp_root / "missing_invoice.frx").string() + "' PREVIEW\n");
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "#2597: qps-ploc missing report asset script should pause with an error");
        expect(state.message.find("[!! ") == 0U &&
                   state.message.find((temp_root / "missing_invoice.frx").string()) != std::string::npos &&
                   state.message.find("Unable to resolve report asset") == std::string::npos,
               "#2597: qps-ploc missing report asset error should pseudo-localize prose while preserving the path");
    }

#if !defined(_WIN32)
    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "en-US", true);
        const fs::path loop_root = temp_root / "report_status_loop";
        std::error_code loop_error;
        fs::create_symlink(loop_root, loop_root, loop_error);
        if (!loop_error) {
            const auto state = run_script(
                "report_status_error",
                "REPORT FORM '" + (loop_root / "invoice.frx").string() + "' PREVIEW\n");
            expect(state.reason == copperfin::runtime::DebugPauseReason::error,
                   "#4398: report asset filesystem status errors should become runtime faults");
            expect(state.message == "Unable to resolve report asset: " +
                       (loop_root / "invoice.frx").string(),
                   "#4398: report asset filesystem status errors should use the localized resolve diagnostic");
        }
    }
#endif

#if defined(__linux__)
    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "es-419", true);
        const auto state = run_script(
            "report_write_failed_es",
            "REPORT FORM '" + report_path.string() + "' TO FILE '/dev/full'\n");
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "#2597: es-419 report output write-failed script should pause with an error on /dev/full");
        expect(state.message == "No se pudo escribir la ruta de salida del reporte: /dev/full",
               "#2597: es-419 report output write-failed error should localize the prose while preserving the path");
    }
#endif

    fs::remove_all(temp_root, ignored);
}

void test_runtime_aggregate_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap function_placeholders{{"function", "SUM"}};
    const copperfin::localization::PlaceholderMap to_placeholders{
        {"function", "SUM"},
        {"toKeyword", "TO"},
    };
    const copperfin::localization::PlaceholderMap to_array_placeholders{
        {"function", "SUM"},
        {"toKeyword", "TO"},
        {"arrayKeyword", "ARRAY"},
    };

    expect(
        english.translate("Runtime.Prg.Aggregate.Error.CalculateRequiresAssignments") ==
            "CALCULATE requires one or more aggregate TO/INTO assignments",
        "#2595: aggregate missing-assignment error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.CalculateRequiresAggregateExpression") ==
            "CALCULATE requires aggregate expressions like COUNT() or SUM(field)",
        "#2595: aggregate malformed-expression error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.CountToSingleTarget") ==
            "COUNT TO only accepts a single variable target",
        "#2595: count multi-target error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.RequiresSelectedWorkArea", function_placeholders) ==
            "SUM requires a selected work area",
        "#2721: aggregate no-work-area error should preserve the invariant command token");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.TargetWorkAreaNotFound", function_placeholders) ==
            "SUM target work area not found",
        "#2721: aggregate missing-target-work-area error should preserve the invariant command token");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.ToArrayRequiresTargetArrayName", to_array_placeholders) ==
            "SUM TO ARRAY requires a target array name",
        "#2721: aggregate TO ARRAY missing-target error should preserve invariant command tokens");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.ToArraySingleTargetOnly", to_array_placeholders) ==
            "SUM TO ARRAY accepts exactly one array target",
        "#2721: aggregate TO ARRAY multi-target error should preserve invariant command tokens");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.RequiresExpressions", function_placeholders) ==
            "SUM requires one or more expressions",
        "#2721: aggregate missing-expressions error should preserve the invariant command token");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.ToRequiresVariablePerAggregateExpression", to_placeholders) ==
            "SUM TO requires one variable per aggregate expression",
        "#2721: aggregate TO target-count mismatch error should preserve invariant command tokens");

    const std::string spanish_assignments =
        spanish.translate("Runtime.Prg.Aggregate.Error.CalculateRequiresAssignments");
    expect(
        spanish_assignments == "CALCULATE requiere una o mas asignaciones agregadas TO/INTO",
        "#2595: es-419 aggregate missing-assignment error should localize the prose");
    expect(
        spanish_assignments.find("CALCULATE") != std::string::npos &&
            spanish_assignments.find("TO/INTO") != std::string::npos &&
            spanish_assignments.find("requires one or more") == std::string::npos,
        "#2595: es-419 aggregate missing-assignment error should preserve invariant command tokens without falling back to English prose");

    const std::string portuguese_expression =
        portuguese.translate("Runtime.Prg.Aggregate.Error.CalculateRequiresAggregateExpression");
    expect(
        portuguese_expression == "CALCULATE exige expressoes agregadas como COUNT() ou SUM(field)",
        "#2595: pt-BR aggregate malformed-expression error should localize the prose");
    expect(
        spanish.translate("Runtime.Prg.Aggregate.Error.RequiresSelectedWorkArea", function_placeholders)
                .find("requires a selected work area") == std::string::npos,
        "#2721: es-419 aggregate no-work-area error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Aggregate.Error.ToRequiresVariablePerAggregateExpression", to_placeholders)
                .find("one variable per aggregate expression") == std::string::npos,
        "#2721: pt-BR aggregate TO target-count mismatch error should not fall back to raw English");

    const std::string pseudo_count =
        pseudo.translate("Runtime.Prg.Aggregate.Error.CountToSingleTarget");
    expect(
        pseudo_count ==
            copperfin::localization::pseudo_localize("COUNT TO only accepts a single variable target"),
        "#2595: qps-ploc aggregate count-target error should resolve through the pseudo-localization transform");
    const std::string pseudo_to_array =
        pseudo.translate("Runtime.Prg.Aggregate.Error.ToArrayRequiresTargetArrayName", to_array_placeholders);
    expect(
        pseudo_to_array.find("[!! ") == 0U &&
            pseudo_to_array.find("SUM") != std::string::npos &&
            pseudo_to_array.find("TO ARRAY") != std::string::npos &&
            pseudo_to_array.find("requires a target array name") == std::string::npos,
        "#2721: qps-ploc aggregate TO ARRAY error should pseudo-localize prose while preserving invariant command tokens");
}

void test_runtime_sql_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Sql.Error.HandleNotFound", {{"handle", "7"}}) ==
            "SQL handle not found: 7",
        "#2596: SQL missing-handle error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Sql.Error.SqlExecRequiresCommandOrPreparedStatement") ==
            "SQLEXEC requires a command or a prepared SQL statement",
        "#2596: SQLEXEC missing-command error should remain catalog-backed in en-US");

    const std::string spanish_handle =
        spanish.translate("Runtime.Prg.Sql.Error.HandleNotFound", {{"handle", "7"}});
    expect(
        spanish_handle == "No se encontro el handle SQL: 7",
        "#2596: es-419 SQL missing-handle error should localize the prose");
    expect(
        spanish_handle.find("7") != std::string::npos &&
            spanish_handle.find("SQL") != std::string::npos &&
            spanish_handle.find("SQL handle not found") == std::string::npos,
        "#2596: es-419 SQL missing-handle error should preserve invariant handle and SQL token values without falling back to English prose");

    const std::string portuguese_sqlexec =
        portuguese.translate("Runtime.Prg.Sql.Error.SqlExecRequiresCommandOrPreparedStatement");
    expect(
        portuguese_sqlexec == "SQLEXEC exige um comando ou uma instrucao SQL preparada",
        "#2596: pt-BR SQLEXEC missing-command error should localize the prose");

    const std::string pseudo_handle =
        pseudo.translate("Runtime.Prg.Sql.Error.HandleNotFound", {{"handle", "7"}});
    expect(
        pseudo_handle.find("[!! ") == 0U &&
            pseudo_handle.find("7") != std::string::npos &&
            pseudo_handle.find("SQL handle not found: 7") == std::string::npos,
        "#2596: qps-ploc SQL missing-handle error should pseudo-localize prose while preserving the handle value");
}

void test_build_host_catalog_entries_cover_placeholder_locales() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap usage_placeholders{
        {"buildCommand", "build"},
        {"commandName", "copperfin_build_host"},
        {"configurationOption", "--configuration"},
        {"configurationValue", "debug|release"},
        {"emitDotnetLauncherOption", "--emit-dotnet-launcher"},
        {"enableSecurityOption", "--enable-security"},
        {"outputDirOption", "--output-dir"},
        {"outputDirValue", "<directory>"},
        {"projectOption", "--project"},
        {"projectValue", "<path-to-pjx>"},
        {"runtimeHostOption", "--runtime-host"},
        {"runtimeHostValue", "<path>"}
    };

    const std::string english_usage = english.translate("BuildHost.Usage", usage_placeholders);
    const std::string spanish_usage = spanish.translate("BuildHost.Usage", usage_placeholders);
    const std::string portuguese_usage = portuguese.translate("BuildHost.Usage", usage_placeholders);
    const std::string pseudo_usage = pseudo.translate("BuildHost.Usage", usage_placeholders);

    expect(
        english.translate("BuildHost.Warning.ProcessHardening", {{"message", "harden"}}) == "warning: harden",
        "#2539: build-host process-hardening warning label should be catalog-backed");
    expect(
        spanish_usage.find("Uso: copperfin_build_host build") != std::string::npos &&
            spanish_usage.find("--project") != std::string::npos &&
            spanish_usage.find("debug|release") != std::string::npos,
        "#2539: es-419 build-host usage should preserve CLI invariants while routing prose through the catalog");
    expect(
        spanish_usage != english_usage && spanish_usage.find("Usage: copperfin_build_host") == std::string::npos,
        "#2539: es-419 build-host usage should not fall back to raw English prose");
    expect(
        portuguese.translate("BuildHost.Error.UnknownOrIncompleteArgument", {{"argument", "--project"}}) ==
            "Argumento desconhecido ou incompleto: --project",
        "#2539: pt-BR build-host parse diagnostics should preserve CLI placeholders");
    expect(
        portuguese_usage != english_usage && portuguese_usage.find("Usage: copperfin_build_host") == std::string::npos,
        "#2539: pt-BR build-host usage should not fall back to raw English prose");
    expect(
        pseudo_usage.find("[!! ") == 0U &&
            pseudo_usage.find("copperfin_build_host") != std::string::npos &&
            pseudo_usage.find("--output-dir") != std::string::npos,
        "#2539: qps-ploc build-host usage should pseudo-localize prose while preserving placeholder values");
}

void test_inspect_catalog_entries_cover_placeholder_locales() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap usage_placeholders{
        {"assetPathArgument", "<path-to-vfp-asset>"},
        {"commandName", "copperfin_inspect"},
        {"localeOption", "--locale"},
        {"localeValue", "<locale>"}
    };

    const std::string english_usage = english.translate("Inspect.Usage", usage_placeholders);
    const std::string spanish_usage = spanish.translate("Inspect.Usage", usage_placeholders);
    const std::string portuguese_usage = portuguese.translate("Inspect.Usage", usage_placeholders);
    const std::string pseudo_usage = pseudo.translate("Inspect.Usage", usage_placeholders);

    expect(
        spanish_usage.find("Uso: copperfin_inspect") != std::string::npos &&
            spanish_usage.find("--locale") != std::string::npos &&
            spanish_usage.find("<path-to-vfp-asset>") != std::string::npos,
        "#2579: es-419 inspect usage should preserve CLI invariants while routing prose through the catalog");
    expect(
        spanish_usage != english_usage && spanish_usage.find("Usage: copperfin_inspect") == std::string::npos,
        "#2579: es-419 inspect usage should not fall back to raw English prose");
    expect(
        portuguese_usage.find("Uso: copperfin_inspect") != std::string::npos &&
            portuguese_usage.find("--locale") != std::string::npos &&
            portuguese_usage.find("<path-to-vfp-asset>") != std::string::npos,
        "#2579: pt-BR inspect usage should preserve CLI invariants while routing prose through the catalog");
    expect(
        portuguese_usage != english_usage && portuguese_usage.find("Usage: copperfin_inspect") == std::string::npos,
        "#2579: pt-BR inspect usage should not fall back to raw English prose");
    expect(
        pseudo_usage.find("[!! ") == 0U &&
            pseudo_usage.find("copperfin_inspect") != std::string::npos &&
            pseudo_usage.find("--locale") != std::string::npos,
        "#2579: qps-ploc inspect usage should pseudo-localize prose while preserving placeholder values");
}

void test_shared_core_catalog_entries_cover_placeholder_locales() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        spanish.translate("Command.Build") == "Compilar",
        "#2584: es-419 shared command labels should localize build labels");
    expect(
        portuguese.translate("Command.Inspect") == "Inspecionar",
        "#2584: pt-BR shared command labels should localize inspect labels");
    expect(
        spanish.translate("Help.LocaleOption") != english.translate("Help.LocaleOption") &&
            spanish.translate("Help.LocaleOption").find("Select the user-interface locale.") == std::string::npos,
        "#2584: es-419 shared help prose should not fall back to raw English");

    const std::string spanish_diagnostic = spanish.translate(
        "Diagnostic.ExpectedTokenBeforeToken",
        {{"expectedToken", "ENDSCAN"}, {"actualToken", "ENDIF"}});
    expect(
        spanish_diagnostic == "Se esperaba ENDSCAN antes de ENDIF.",
        "#2584: es-419 shared diagnostics should preserve parser tokens while localizing prose");

    const std::string portuguese_error =
        portuguese.translate("Error.UnknownLocale", {{"locale", "zz-ZZ"}});
    expect(
        portuguese_error == "Localidade desconhecida: zz-ZZ",
        "#2584: pt-BR shared error prose should preserve locale placeholders");

    const std::string pseudo_diagnostic = pseudo.translate(
        "Diagnostic.ExpectedTokenBeforeToken",
        {{"expectedToken", "ENDSCAN"}, {"actualToken", "ENDIF"}});
    expect(
        pseudo_diagnostic.find("[!! ") == 0U &&
            pseudo_diagnostic.find("ENDSCAN") != std::string::npos &&
            pseudo_diagnostic.find("ENDIF") != std::string::npos &&
            pseudo_diagnostic.find("{expectedToken}") == std::string::npos,
        "#2584: qps-ploc shared diagnostics should pseudo-localize prose while preserving placeholders");
}

void test_runtime_host_manifest_verification_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap file_name{{"fileName", "helper.dll"}};

    expect(
        english.translate("RuntimeHost.Error.ManifestMissingRuntimeHostSha256") ==
            "Security-enabled manifest is missing runtime_host_sha256.",
        "#2588: runtime-host manifest verification should preserve the en-US missing runtime_host_sha256 output");
    expect(
        english.translate("RuntimeHost.Error.ExtensionPayloadMissingFromPackage", file_name) ==
            "Extension payload is missing from the package: helper.dll",
        "#2588: runtime-host manifest verification should preserve the en-US payload-path placeholder output");

    const std::string spanish_missing_sha256 =
        spanish.translate("RuntimeHost.Error.ManifestMissingRuntimeHostSha256");
    expect(
        spanish_missing_sha256 ==
            "Al manifiesto con seguridad habilitada le falta runtime_host_sha256.",
        "#2588: es-419 runtime-host manifest verification should localize missing runtime_host_sha256 prose");
    expect(
        spanish_missing_sha256.find("Security-enabled manifest is missing") == std::string::npos,
        "#2588: es-419 runtime-host manifest verification should not fall back to raw English prose");

    const std::string portuguese_missing_payload =
        portuguese.translate("RuntimeHost.Error.ExtensionPayloadMissingFromPackage", file_name);
    expect(
        portuguese_missing_payload ==
            "O payload de extensao esta ausente do pacote: helper.dll",
        "#2588: pt-BR runtime-host manifest verification should localize payload-path prose while preserving the file name");
    expect(
        portuguese_missing_payload.find("helper.dll") != std::string::npos &&
            portuguese_missing_payload.find("Extension payload is missing") == std::string::npos,
        "#2588: pt-BR runtime-host manifest verification should preserve payload placeholders without falling back to English");

    const std::string pseudo_missing_payload =
        pseudo.translate("RuntimeHost.Error.ExtensionPayloadMissingFromPackage", file_name);
    expect(
        pseudo_missing_payload.find("[!! ") == 0U &&
            pseudo_missing_payload.find("helper.dll") != std::string::npos &&
            pseudo_missing_payload.find("{fileName}") == std::string::npos,
        "#2588: qps-ploc runtime-host manifest verification should pseudo-localize prose while preserving payload placeholders");
}

void test_runtime_host_quit_prompt_routes_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap confirmation_tokens{
        {"yesToken", "y"},
        {"defaultNoToken", "N"}
    };

    expect(
        english.translate("RuntimeHost.Prompt.QuitConfirm", confirmation_tokens) ==
            "Do you want to quit this application? [y/N]: ",
        "#2591: runtime-host quit prompt should preserve the en-US confirmation prompt");

    const std::string spanish_prompt = spanish.translate("RuntimeHost.Prompt.QuitConfirm", confirmation_tokens);
    expect(
        spanish_prompt == "Desea salir de esta aplicacion? [y/N]: ",
        "#2591: es-419 runtime-host quit prompt should localize the prose");
    expect(
        spanish_prompt.find("Do you want to quit this application?") == std::string::npos &&
            spanish_prompt.find("[y/N]: ") != std::string::npos,
        "#2591: es-419 runtime-host quit prompt should preserve confirmation tokens without falling back to English");

    const std::string portuguese_prompt =
        portuguese.translate("RuntimeHost.Prompt.QuitConfirm", confirmation_tokens);
    expect(
        portuguese_prompt == "Deseja sair deste aplicativo? [y/N]: ",
        "#2591: pt-BR runtime-host quit prompt should localize the prose");

    const std::string pseudo_prompt = pseudo.translate("RuntimeHost.Prompt.QuitConfirm", confirmation_tokens);
    expect(
        pseudo_prompt.find("[!! ") == 0U &&
            pseudo_prompt.find("[y/N]: ") != std::string::npos &&
            pseudo_prompt.find("Do you want to quit this application?") == std::string::npos,
        "#2591: qps-ploc runtime-host quit prompt should pseudo-localize prose while preserving confirmation tokens");
}

void test_runtime_host_security_policy_denial_routes_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap denial_placeholders{
        {"permission", "runtime.admin"},
        {"role", "developer"}
    };

    expect(
        english.translate("RuntimeHost.Error.SecurityPolicyDenied", denial_placeholders) ==
            "Security policy denied runtime.admin for role 'developer'.",
        "#2592: runtime-host security denial should preserve the en-US default output");

    const std::string spanish_denial =
        spanish.translate("RuntimeHost.Error.SecurityPolicyDenied", denial_placeholders);
    expect(
        spanish_denial == "La politica de seguridad denego runtime.admin para el rol 'developer'.",
        "#2592: es-419 runtime-host security denial should localize the prose");
    expect(
        spanish_denial.find("Security policy denied") == std::string::npos &&
            spanish_denial.find("runtime.admin") != std::string::npos &&
            spanish_denial.find("developer") != std::string::npos,
        "#2592: es-419 runtime-host security denial should preserve invariant permission and role ids without falling back to English");

    const std::string portuguese_denial =
        portuguese.translate("RuntimeHost.Error.SecurityPolicyDenied", denial_placeholders);
    expect(
        portuguese_denial == "A politica de seguranca negou runtime.admin para a funcao 'developer'.",
        "#2592: pt-BR runtime-host security denial should localize the prose");

    const std::string pseudo_denial =
        pseudo.translate("RuntimeHost.Error.SecurityPolicyDenied", denial_placeholders);
    expect(
        pseudo_denial.find("[!! ") == 0U &&
            pseudo_denial.find("runtime.admin") != std::string::npos &&
            pseudo_denial.find("developer") != std::string::npos &&
            pseudo_denial.find("Security policy denied") == std::string::npos,
        "#2592: qps-ploc runtime-host security denial should pseudo-localize prose while preserving invariant ids");
}

void test_platform_federation_ai_planner_fallback_routes_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::string english_translation_error =
        english.translate("Platform.QueryTranslator.Error.SelectFromOnly");
    const std::string spanish_translation_error =
        spanish.translate("Platform.QueryTranslator.Error.SelectFromOnly");
    const std::string portuguese_translation_error =
        portuguese.translate("Platform.QueryTranslator.Error.SelectFromOnly");
    const std::string pseudo_translation_error =
        pseudo.translate("Platform.QueryTranslator.Error.SelectFromOnly");

    const copperfin::localization::PlaceholderMap english_placeholders{
        {"planMode", "optional"},
        {"translationError", english_translation_error}
    };
    const copperfin::localization::PlaceholderMap spanish_placeholders{
        {"planMode", "optional"},
        {"translationError", spanish_translation_error}
    };
    const copperfin::localization::PlaceholderMap portuguese_placeholders{
        {"planMode", "optional"},
        {"translationError", portuguese_translation_error}
    };
    const copperfin::localization::PlaceholderMap pseudo_placeholders{
        {"planMode", "optional"},
        {"translationError", pseudo_translation_error}
    };

    expect(
        english.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", english_placeholders) ==
            "Planner is not yet implemented for optional AI policy. Deterministic translation failed: "
            "Only first-pass SELECT...FROM SQL translation is supported.",
        "#2593: federation AI planner fallback should preserve the en-US default output");

    const std::string spanish_message =
        spanish.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", spanish_placeholders);
    expect(
        spanish_message ==
            "El planner aun no esta implementado para la politica de IA optional. La traduccion deterministica fallo: "
            "Solo se admite la traduccion SQL deterministica de primera pasada de SELECT...FROM.",
        "#2594: es-419 federation AI planner fallback should localize both the wrapper and translator payload");
    expect(
        spanish_message.find("Planner is not yet implemented") == std::string::npos &&
            spanish_message.find("optional") != std::string::npos &&
            spanish_message.find("Only first-pass SELECT...FROM SQL translation is supported.") == std::string::npos &&
            spanish_message.find("SELECT...FROM") != std::string::npos,
        "#2594: es-419 federation AI planner fallback should preserve invariant SQL tokens without falling back to English payload prose");

    const std::string portuguese_message =
        portuguese.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", portuguese_placeholders);
    expect(
        portuguese_message ==
            "O planner ainda nao esta implementado para a politica de IA optional. A traducao deterministica falhou: "
            "Somente a traducao SQL deterministica de primeira passagem de SELECT...FROM e suportada.",
        "#2594: pt-BR federation AI planner fallback should localize both the wrapper and translator payload");

    const std::string pseudo_message =
        pseudo.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", pseudo_placeholders);
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message.find(pseudo_translation_error) != std::string::npos &&
            pseudo_message.find("optional") != std::string::npos &&
            pseudo_message.find("Platform.QueryTranslator.Error.SelectFromOnly") == std::string::npos &&
            pseudo_message.find("Only first-pass SELECT...FROM SQL translation is supported.") == std::string::npos &&
            pseudo_message.find("Planner is not yet implemented") == std::string::npos,
        "#2594: qps-ploc federation AI planner fallback should pseudo-localize both the wrapper and translator payload without leaking raw English or unresolved keys");
}

void test_runtime_numeric_domain_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap log_placeholders{
        {"function", "LOG()"},
        {"value", "0.000000"}
    };
    const copperfin::localization::PlaceholderMap asin_placeholders{
        {"function", "ASIN()"},
        {"value", "2.000000"}
    };

    expect(
        english.translate("Runtime.Prg.Numeric.Error.PositiveArgumentRequired", log_placeholders) ==
            "LOG() requires a positive argument (got 0.000000)",
        "#2540: positive-argument numeric error should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Numeric.Error.UnitRangeArgumentRequired", asin_placeholders) ==
            "ASIN() requires an argument between -1 and 1 (got 2.000000)",
        "#2540: unit-range numeric error should preserve en-US default output");

    const std::string spanish_log =
        spanish.translate("Runtime.Prg.Numeric.Error.PositiveArgumentRequired", log_placeholders);
    expect(
        spanish_log.find("LOG()") != std::string::npos &&
            spanish_log.find("0.000000") != std::string::npos &&
            spanish_log.find("requires a positive argument") == std::string::npos,
        "#2540: es-419 numeric error should preserve placeholders without falling back to English prose");

    const std::string portuguese_asin =
        portuguese.translate("Runtime.Prg.Numeric.Error.UnitRangeArgumentRequired", asin_placeholders);
    expect(
        portuguese_asin.find("ASIN()") != std::string::npos &&
            portuguese_asin.find("2.000000") != std::string::npos &&
            portuguese_asin.find("requires an argument between") == std::string::npos,
        "#2540: pt-BR numeric error should preserve placeholders without falling back to English prose");

    const std::string pseudo_message =
        pseudo.translate("Runtime.Prg.Numeric.Error.PositiveArgumentRequired", log_placeholders);
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message.find("LOG()") != std::string::npos &&
            pseudo_message.find("0.000000") != std::string::npos &&
            pseudo_message.find("{function}") == std::string::npos,
        "#2540: qps-ploc numeric error should pseudo-localize prose while preserving placeholders");
}

void test_runtime_expression_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Expression.Error.IntegerDivisionByZero") ==
            "Division by zero in integer expression",
        "#2541: integer division-by-zero error should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Expression.Error.DivisionByZero") == "Division by zero",
        "#2541: division-by-zero error should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Expression.Error.ExpectedFunctionArgument") == "Expected function argument",
        "#2541: expected-function-argument error should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Expression.Error.OperatorOperandTypeMismatch") ==
            "Operator/operand type mismatch.",
        "#3875: operator/operand mismatch should preserve VFP Error 107 en-US prose");
    expect(
        spanish.translate("Runtime.Prg.Expression.Error.DivisionByZero") !=
            english.translate("Runtime.Prg.Expression.Error.DivisionByZero"),
        "#2541: es-419 expression division error should not fall back to raw English prose");
    expect(
        portuguese.translate("Runtime.Prg.Expression.Error.ExpectedFunctionArgument") !=
            english.translate("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"),
        "#2541: pt-BR expected-argument error should not fall back to raw English prose");

    const std::string pseudo_message = pseudo.translate("Runtime.Prg.Expression.Error.IntegerDivisionByZero");
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message != english.translate("Runtime.Prg.Expression.Error.IntegerDivisionByZero"),
        "#2541: qps-ploc expression error should pseudo-localize prose");
    expect(
        pseudo.translate("Runtime.Prg.Expression.Error.OperatorOperandTypeMismatch").find("[!! ") == 0U,
        "#3875: qps-ploc operator/operand mismatch should pseudo-localize prose");
}

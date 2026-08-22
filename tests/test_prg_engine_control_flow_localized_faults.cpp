// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_file_operation_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_file_ops_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path nonempty_dir = temp_root / "busy";
    fs::create_directories(nonempty_dir);
    write_text(nonempty_dir / "child.txt", "payload");

    const fs::path erase_path = temp_root / "erase_error.prg";
    write_text(
        erase_path,
        "ERASE 'busy'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession erase_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(erase_path.string(), temp_root.string(), false));
    const auto erase_state = erase_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!erase_state.completed, "#2706: qps-ploc ERASE non-empty-directory script should fail");
    expect(
        erase_state.message.find("[!! ") == 0U &&
            erase_state.message.find("ERASE failed:") == std::string::npos &&
            erase_state.message.find("busy") != std::string::npos,
        "#2706: qps-ploc ERASE runtime error should localize the prose while preserving the path");

    const fs::path copy_path = temp_root / "copy_error.prg";
    write_text(
        copy_path,
        "COPY FILE 'missing.txt' TO 'copied.txt'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession copy_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(copy_path.string(), temp_root.string(), false));
    const auto copy_state = copy_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!copy_state.completed, "#2706: qps-ploc COPY FILE missing-source script should fail");
    expect(
        copy_state.message.find("[!! ") == 0U &&
            copy_state.message.find("COPY FILE failed:") == std::string::npos,
        "#2706: qps-ploc COPY FILE runtime error should localize the prose while preserving the OS error text");

    const fs::path rename_path = temp_root / "rename_error.prg";
    write_text(
        rename_path,
        "RENAME 'missing.txt' TO 'renamed.txt'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession rename_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(rename_path.string(), temp_root.string(), false));
    const auto rename_state = rename_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!rename_state.completed, "#2706: qps-ploc RENAME missing-source script should fail");
    expect(
        rename_state.message.find("[!! ") == 0U &&
            rename_state.message.find("RENAME failed:") == std::string::npos,
        "#2706: qps-ploc RENAME runtime error should localize the prose while preserving the OS error text");

    write_text(temp_root / "source.txt", "source-content");
    write_text(temp_root / "existing.txt", "existing-content");
    const fs::path rename_existing_path = temp_root / "rename_existing_error.prg";
    write_text(
        rename_existing_path,
        "RENAME 'source.txt' TO 'existing.txt'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession rename_existing_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(rename_existing_path.string(), temp_root.string(), false));
    const auto rename_existing_state =
        rename_existing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!rename_existing_state.completed,
           "#3703: qps-ploc RENAME existing-destination script should fail");
    expect(
        rename_existing_state.message.find("[!! ") == 0U &&
            rename_existing_state.message.find("destination already exists") == std::string::npos &&
            rename_existing_state.message.find("existing.txt") != std::string::npos,
        "#3703: qps-ploc existing-destination RENAME error should localize prose while preserving the target path");
    expect(read_text(temp_root / "existing.txt") == "existing-content",
           "#3703: localized existing-destination RENAME failures should preserve the destination contents");

    fs::remove_all(temp_root, ignored);
}

void test_residual_dispatch_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_residual_dispatch_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path text_path = temp_root / "text_missing_target.prg";
    write_text(
        text_path,
        "TEXT\n"
        "Hello\n"
        "ENDTEXT\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession text_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(text_path.string(), temp_root.string(), false));
    const auto text_state = text_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!text_state.completed, "#2717: qps-ploc TEXT without TO target should fail");
    expect(
        text_state.message ==
            copperfin::localization::pseudo_localize("TEXT requires TO <variable> in the current runtime slice"),
        "#2717: qps-ploc TEXT missing-target error should route through the pseudo-localization transform");

    const fs::path try_path = temp_root / "try_missing_endtry.prg";
    write_text(
        try_path,
        "TRY\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession try_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(try_path.string(), temp_root.string(), false));
    const auto try_state = try_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!try_state.completed, "#2717: qps-ploc TRY without ENDTRY should fail");
    expect(
        try_state.message ==
            copperfin::localization::pseudo_localize("TRY block is missing ENDTRY"),
        "#2717: qps-ploc TRY missing-ENDTRY error should route through the pseudo-localization transform");

    const fs::path replace_path = temp_root / "replace_missing_assignments.prg";
    write_text(
        replace_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE FOR .T.\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession replace_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(replace_path.string(), temp_root.string(), false));
    const auto replace_state = replace_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!replace_state.completed, "#2717: qps-ploc REPLACE without assignments should fail");
    expect(
        replace_state.message ==
            copperfin::localization::pseudo_localize("REPLACE requires at least one FIELD WITH expression assignment"),
        "#2717: qps-ploc REPLACE assignment error should route through the pseudo-localization transform");

    const fs::path update_path = temp_root / "update_missing_assignments.prg";
    write_text(
        update_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "UPDATE People\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession update_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(update_path.string(), temp_root.string(), false));
    const auto update_state = update_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!update_state.completed, "#2717: qps-ploc UPDATE without SET assignments should fail");
    expect(
        update_state.message ==
            copperfin::localization::pseudo_localize("UPDATE requires SET field = expression assignments"),
        "#2717: qps-ploc UPDATE assignment error should route through the pseudo-localization transform");

    const fs::path insert_path = temp_root / "insert_missing_values.prg";
    write_text(
        insert_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "INSERT INTO People\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession insert_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(insert_path.string(), temp_root.string(), false));
    const auto insert_state = insert_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!insert_state.completed, "#2717: qps-ploc INSERT INTO without VALUES should fail");
    expect(
        insert_state.message ==
            copperfin::localization::pseudo_localize("INSERT INTO requires a VALUES clause"),
        "#2717: qps-ploc INSERT INTO VALUES-clause error should route through the pseudo-localization transform");

    const fs::path unlock_path = temp_root / "unlock_missing_record.prg";
    write_text(
        unlock_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "UNLOCK RECORD 99 IN People\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession unlock_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(unlock_path.string(), temp_root.string(), false));
    const auto unlock_state = unlock_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!unlock_state.completed, "#2717: qps-ploc UNLOCK RECORD with a missing record should fail");
    expect(
        unlock_state.message ==
            copperfin::localization::pseudo_localize("UNLOCK RECORD target record not found"),
        "#2717: qps-ploc UNLOCK RECORD target-record error should route through the pseudo-localization transform");

    const fs::path sleep_cancel_path = temp_root / "sleep_cancelled_task.prg";
    write_text(
        sleep_cancel_path,
        "PROCEDURE worker\n"
        "    SLEEP 50\n"
        "    RETURN\n"
        "ENDPROC\n"
        "PROCEDURE canceler\n"
        "    SLEEP 1\n"
        "    CANCEL\n"
        "ENDPROC\n"
        "SPAWN worker TO nWorker\n"
        "SPAWN canceler TO nCancel\n"
        "AWAIT nCancel TO lCancelDone\n"
        "AWAIT nWorker TO lWorkerDone\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession sleep_cancel_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(sleep_cancel_path.string(), temp_root.string(), false));
    const auto sleep_cancel_state = sleep_cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(sleep_cancel_state.completed, "#2717: qps-ploc spawned-task cancellation script should complete");
    const auto sleep_cancel_event = std::find_if(
        sleep_cancel_state.events.begin(),
        sleep_cancel_state.events.end(),
        [](const auto& event) {
            return event.category == "runtime.task.await" &&
                   event.detail.find("state=error") != std::string::npos &&
                   event.detail.find(copperfin::localization::pseudo_localize("SLEEP cancelled.")) != std::string::npos;
        });
    expect(sleep_cancel_event != sleep_cancel_state.events.end(),
           "#2717: qps-ploc spawned-task cancellation should report an errored AWAIT event");
    if (sleep_cancel_event != sleep_cancel_state.events.end()) {
        expect(
            sleep_cancel_event->detail.find(copperfin::localization::pseudo_localize("SLEEP cancelled.")) != std::string::npos,
            "#2717: qps-ploc spawned-task cancellation should preserve the localized SLEEP cancellation text");
    }

    fs::remove_all(temp_root, ignored);
}

void test_dispatch_array_and_object_target_runtime_errors_use_default_locale_messages() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_dispatch_helper_defaults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    const auto run_error_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script + "RETURN\n");
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    const auto copy_missing_array = run_error_script("copy_missing_array", "COPY TO ARRAY FIELDS NAME\n");
    expect(!copy_missing_array.completed, "#2722: COPY TO ARRAY without a target array should fail");
    expect(
        copy_missing_array.message == "COPY TO ARRAY: array name required",
        "#2722: COPY TO ARRAY should keep the default-locale array-name-required helper text");

    const auto copy_invalid_array = run_error_script(
        "copy_invalid_array",
        "COPY TO ARRAY 'bad name'\n");
    expect(!copy_invalid_array.completed, "#2722: COPY TO ARRAY with an invalid array target should fail");
    expect(
        copy_invalid_array.message == "COPY TO ARRAY: invalid array name",
        "#2722: COPY TO ARRAY should keep the default-locale invalid-array-name helper text");

    const auto scatter_invalid_object = run_error_script(
        "scatter_invalid_object",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SCATTER NAME 'bad path'\n");
    expect(!scatter_invalid_object.completed, "#2722: SCATTER NAME with an invalid object target should fail");
    expect(
        scatter_invalid_object.message == "SCATTER NAME: invalid object target",
        "#2722: SCATTER NAME should keep the default-locale invalid-object-target helper text");

    fs::remove_all(temp_root, ignored);
}

void test_dispatch_array_and_object_target_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_dispatch_helper_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");

    const auto run_error_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script + "RETURN\n");
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    const auto copy_missing_array = run_error_script("copy_missing_array_es", "COPY TO ARRAY FIELDS NAME\n");
    expect(!copy_missing_array.completed, "#2722: es-419 COPY TO ARRAY without a target array should fail");
    expect(
        copy_missing_array.message == "COPY TO ARRAY: se requiere un nombre de arreglo",
        "#2722: es-419 COPY TO ARRAY helper error should localize the array-name-required text");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    const auto scatter_invalid_object = run_error_script(
        "scatter_invalid_object_pt",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SCATTER NAME 'bad path'\n");
    expect(!scatter_invalid_object.completed, "#2722: pt-BR SCATTER NAME with an invalid object target should fail");
    expect(
        scatter_invalid_object.message == "SCATTER NAME: destino de objeto invalido",
        "#2722: pt-BR SCATTER NAME helper error should localize the invalid-object-target text");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto scatter_invalid_object_qps = run_error_script(
        "scatter_invalid_object_qps",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SCATTER NAME 'bad path'\n");
    expect(!scatter_invalid_object_qps.completed, "#2722: qps-ploc SCATTER NAME with an invalid object target should fail");
    expect(
        scatter_invalid_object_qps.message.find("[!! ") == 0U &&
            scatter_invalid_object_qps.message.find("SCATTER NAME") != std::string::npos &&
            scatter_invalid_object_qps.message.find("invalid object target") == std::string::npos,
        "#2722: qps-ploc SCATTER NAME helper error should pseudo-localize prose while preserving the command token");

    fs::remove_all(temp_root, ignored);
}

void test_ole_property_assignment_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_ole_assignment_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "ole_assignment_localization.prg";
    write_text(
        main_path,
        "ON ERROR DO oleerr\n"
        "missingOle.SomeProperty = 42\n"
        "RETURN\n"
        "PROCEDURE oleerr\n"
        "PUBLIC nOleRows, cOleMessage, cOleDetail\n"
        "nOleRows = AERROR(aOleErr)\n"
        "cOleMessage = aOleErr[1,2]\n"
        "cOleDetail = aOleErr[1,3]\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#2718: qps-ploc OLE property-assignment handler script should complete");

    const auto rows = state.globals.find("nolerows");
    const auto message = state.globals.find("colemessage");
    const auto detail = state.globals.find("coledetail");
    expect(rows != state.globals.end(), "#2718: qps-ploc OLE AERROR should return a row count");
    expect(message != state.globals.end(), "#2718: qps-ploc OLE AERROR should populate the localized message");
    expect(detail != state.globals.end(), "#2718: qps-ploc OLE AERROR should populate the failing member path");

    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "1",
               "#2718: qps-ploc OLE AERROR should expose one row");
    }
    if (message != state.globals.end()) {
        const std::string localized_message = copperfin::runtime::format_value(message->second);
        expect(
            localized_message.find("[!! ") == 0U &&
                localized_message.find("missingOle.SomeProperty") != std::string::npos &&
                localized_message.find("OLE object not found for property assignment") == std::string::npos,
            "#2718: qps-ploc OLE property-assignment message should pseudo-localize prose while preserving the member path");
    }
    if (detail != state.globals.end()) {
        expect(copperfin::runtime::format_value(detail->second).find("missingOle.SomeProperty") != std::string::npos,
               "#2718: qps-ploc OLE AERROR detail should preserve the failing member path");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ole_invocation_and_property_read_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_ole_read_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "ole_read_localization.prg";
    write_text(
        main_path,
        "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
        "oDict.Add('Alpha', 41)\n"
        "TRY\n"
        "  missingOle.NoSuchMethod()\n"
        "CATCH TO oMissingMethod\n"
        "  cMissingMethod = oMissingMethod.Message\n"
        "ENDTRY\n"
        "TRY\n"
        "  xMissingObjectProperty = missingOle.SomeProperty\n"
        "CATCH TO oMissingProperty\n"
        "  cMissingProperty = oMissingProperty.Message\n"
        "ENDTRY\n"
        "TRY\n"
        "  oDict.NoSuchMethod(7)\n"
        "CATCH TO oMissingMemberMethod\n"
        "  cMissingMemberMethod = oMissingMemberMethod.Message\n"
        "ENDTRY\n"
        "TRY\n"
        "  xMissingMemberProperty = oDict.NoSuchProperty\n"
        "CATCH TO oMissingMemberProperty\n"
        "  cMissingMemberProperty = oMissingMemberProperty.Message\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#2719: qps-ploc OLE invocation/property-read localization script should complete");

    const auto missing_method = state.globals.find("cmissingmethod");
    const auto missing_property = state.globals.find("cmissingproperty");
    const auto missing_member_method = state.globals.find("cmissingmembermethod");
    const auto missing_member_property = state.globals.find("cmissingmemberproperty");

    expect(missing_method != state.globals.end(), "#2719: qps-ploc missing OLE method invocation should populate CATCH text");
    expect(missing_property != state.globals.end(), "#2719: qps-ploc missing OLE property read should populate CATCH text");
    expect(missing_member_method != state.globals.end(), "#2719: qps-ploc missing OLE member method should populate CATCH text");
    expect(missing_member_property != state.globals.end(), "#2719: qps-ploc missing OLE member property should populate CATCH text");

    if (missing_method != state.globals.end()) {
        std::string localized_message = copperfin::runtime::format_value(missing_method->second);
        std::string folded_message = localized_message;
        std::transform(folded_message.begin(), folded_message.end(), folded_message.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(
            localized_message.find("[!! ") != std::string::npos &&
                folded_message.find("missingole.nosuchmethod") != std::string::npos &&
                folded_message.find("ole object not found for method invocation") == std::string::npos,
            "#2719: qps-ploc missing OLE method invocation should pseudo-localize prose while preserving the target identifier");
    }
    if (missing_property != state.globals.end()) {
        std::string localized_message = copperfin::runtime::format_value(missing_property->second);
        std::string folded_message = localized_message;
        std::transform(folded_message.begin(), folded_message.end(), folded_message.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(
            localized_message.find("[!! ") != std::string::npos &&
                folded_message.find("missingole.someproperty") != std::string::npos &&
                folded_message.find("ole object not found for property read") == std::string::npos,
            "#2719: qps-ploc missing OLE property read should pseudo-localize prose while preserving the property path");
    }
    if (missing_member_method != state.globals.end()) {
        std::string localized_message = copperfin::runtime::format_value(missing_member_method->second);
        std::string folded_message = localized_message;
        std::transform(folded_message.begin(), folded_message.end(), folded_message.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(
            localized_message.find("[!! ") != std::string::npos &&
                folded_message.find("scripting.dictionary.nosuchmethod") != std::string::npos &&
                folded_message.find("ole member not found for method invocation") == std::string::npos,
            "#2719: qps-ploc missing OLE member method should pseudo-localize prose while preserving the member identifier");
    }
    if (missing_member_property != state.globals.end()) {
        std::string localized_message = copperfin::runtime::format_value(missing_member_property->second);
        std::string folded_message = localized_message;
        std::transform(folded_message.begin(), folded_message.end(), folded_message.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(
            localized_message.find("[!! ") != std::string::npos &&
                folded_message.find("scripting.dictionary.nosuchproperty") != std::string::npos &&
                folded_message.find("ole member not found for property read") == std::string::npos,
            "#2719: qps-ploc missing OLE member property should pseudo-localize prose while preserving the member identifier");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow


// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"
#include "test_locale_catalog_environment_support.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>
#include <thread>

namespace {

using namespace copperfin::test_support;

void write_synthetic_vcx_class_library(
    const std::filesystem::path& table_path,
    const std::string& base_class = "Custom",
    const std::string& answer_method =
        "FUNCTION Answer\r\n"
        "RETURN THIS.nSeed + 1\r\n"
        "ENDFUNC\r\n") {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 32U},
        {.name = "CLASS", .type = 'C', .length = 32U},
        {.name = "BASECLASS", .type = 'C', .length = 32U},
        {.name = "METHODS", .type = 'M', .length = 4U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "MyWidget",
            "",
            "MyWidget",
            base_class,
            "PROCEDURE Init\r\n"
            "LPARAMETERS seed\r\n"
            "THIS.nSeed = seed\r\n"
            "ENDPROC\r\n"
            + answer_method,
            "Caption = 'vcx-default'\r\n"
        }
    };
    const auto result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        records);
    expect(result.ok, "synthetic VCX class library should be created");
}

void test_set_procedure_classes_follow_vfp_activation_precedence() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_set_procedure_native_classes";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path first_path = temp_root / "first.prg";
    const fs::path second_path = temp_root / "second.prg";
    const fs::path explicit_path = temp_root / "explicit.prg";
    const fs::path main_path = temp_root / "main.prg";

    write_text(
        first_path,
        "DEFINE CLASS ProcedureWorker AS Custom\n"
        "    nSeed = 0\n"
        "    PROCEDURE Init\n"
        "        LPARAMETERS seed\n"
        "        THIS.nSeed = seed\n"
        "    ENDPROC\n"
        "    FUNCTION Answer\n"
        "        RETURN THIS.nSeed + 2\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS SharedWorker AS Custom\n"
        "    FUNCTION Origin\n"
        "        RETURN 'first'\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS CurrentWins AS Custom\n"
        "    FUNCTION Origin\n"
        "        RETURN 'procedure'\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS ProcedureBase AS Custom\n"
        "    inheritedValue = 'base'\n"
        "    cBacking = 'base-access'\n"
        "    FUNCTION BaseAnswer\n"
        "        RETURN 9\n"
        "    ENDFUNC\n"
        "    FUNCTION Caption_Access\n"
        "        RETURN THIS.cBacking\n"
        "    ENDFUNC\n"
        "    PROCEDURE Caption_Assign\n"
        "        LPARAMETERS value\n"
        "        THIS.cBacking = value + '-assigned'\n"
        "    ENDPROC\n"
        "    FUNCTION Describe\n"
        "        RETURN 'base'\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS Custom AS Custom\n"
        "    FUNCTION Shadow\n"
        "        RETURN 'procedure-custom'\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n");
    write_text(
        second_path,
        "DEFINE CLASS SharedWorker AS Custom\n"
        "    FUNCTION Origin\n"
        "        RETURN 'second'\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n");
    write_text(
        explicit_path,
        "DEFINE CLASS ExplicitOnly AS Custom\n"
        "ENDDEFINE\n");
    write_text(
        main_path,
        "SET PROCEDURE TO first\n"
        "SET PROCEDURE TO second ADDITIVE\n"
        "oLoaded = CREATEOBJECT('ProcedureWorker', 40)\n"
        "nLoadedAnswer = oLoaded.Answer()\n"
        "oBareNew = NEWOBJECT('ProcedureWorker', 5)\n"
        "nBareNewAnswer = oBareNew.Answer()\n"
        "oDuplicate = CREATEOBJECT('SharedWorker')\n"
        "cDuplicateOrigin = oDuplicate.Origin()\n"
        "oCurrent = CREATEOBJECT('CurrentWins')\n"
        "cCurrentOrigin = oCurrent.Origin()\n"
        "oIntrinsic = CREATEOBJECT('IntrinsicDerived')\n"
        "cIntrinsicShadow = oIntrinsic.Shadow()\n"
        "oDerived = CREATEOBJECT('LocalDerived')\n"
        "nBaseAnswer = oDerived.BaseAnswer()\n"
        "cInheritedValue = oDerived.inheritedValue\n"
        "cInitialCaption = oDerived.Caption\n"
        "cInitialDescription = oDerived.Describe()\n"
        "SET PROCEDURE TO second\n"
        "nBaseAfterReplace = oDerived.BaseAnswer()\n"
        "oReplacement = CREATEOBJECT('SharedWorker')\n"
        "cReplacementOrigin = oReplacement.Origin()\n"
        "SET PROCEDURE TO first\n"
        "oExplicit = NEWOBJECT('ProcedureWorker', 'explicit.prg')\n"
        "cExplicitFallback = oExplicit.Answer()\n"
        "SET PROCEDURE TO\n"
        "nBaseAfterClear = oDerived.BaseAnswer()\n"
        "cCaptionAfterClear = oDerived.Caption\n"
        "cDescriptionAfterClear = oDerived.Describe()\n"
        "oDerived.Caption = 'changed'\n"
        "cCaptionAfterAssign = oDerived.Caption\n"
        "oDerived.WriteMethod('BaseAnswer', 'RETURN 11')\n"
        "nBaseAfterWrite = oDerived.BaseAnswer()\n"
        "oCleared = CREATEOBJECT('ProcedureWorker')\n"
        "cClearedFallback = oCleared.Answer()\n"
        "RETURN\n"
        "DEFINE CLASS LocalDerived AS ProcedureBase\n"
        "    FUNCTION Describe\n"
        "        RETURN 'derived-' + DODEFAULT()\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS IntrinsicDerived AS Custom\n"
        "ENDDEFINE\n"
        "DEFINE CLASS CurrentWins AS Custom\n"
        "    FUNCTION Origin\n"
        "        RETURN 'current'\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("SET PROCEDURE native-class script should complete: ") +
               state.message + " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        if (found == state.globals.end()) {
            expect(false, name + " variable not found");
            return;
        }
        const std::string actual = copperfin::runtime::format_value(found->second);
        expect(actual == expected,
               name + " expected '" + expected + "' got '" + actual + "'");
    };

    check("nloadedanswer", "42");
    check("nbarenewanswer", "7");
    check("cduplicateorigin", "first");
    check("ccurrentorigin", "current");
    check("cintrinsicshadow", "ole:IntrinsicDerived.shadow");
    check("nbaseanswer", "9");
    check("cinheritedvalue", "base");
    check("cinitialcaption", "base-access");
    check("cinitialdescription", "derived-base");
    check("nbaseafterreplace", "9");
    check("creplacementorigin", "second");
    check("cexplicitfallback", "ole:ProcedureWorker.answer");
    check("nbaseafterclear", "9");
    check("ccaptionafterclear", "base-access");
    check("cdescriptionafterclear", "derived-base");
    check("ccaptionafterassign", "changed-assigned");
    check("nbaseafterwrite", "11");
    check("cclearedfallback", "ole:ProcedureWorker.answer");

    const auto procedure_worker = std::find_if(
        state.ole_objects.begin(),
        state.ole_objects.end(),
        [&](const copperfin::runtime::RuntimeOleObjectState& object) {
            return object.prog_id == "ProcedureWorker" && object.source == first_path.string();
        });
    expect(procedure_worker != state.ole_objects.end(),
           "CREATEOBJECT should retain the loaded procedure PRG as native class provenance");

    const auto local_derived = std::find_if(
        state.ole_objects.begin(),
        state.ole_objects.end(),
        [&](const copperfin::runtime::RuntimeOleObjectState& object) {
            return object.prog_id == "LocalDerived" && object.source == main_path.string();
        });
    expect(local_derived != state.ole_objects.end(),
           "a caller-local class should remain owned by the caller PRG");
    if (local_derived != state.ole_objects.end()) {
        expect(std::find(local_derived->class_hierarchy.begin(),
                         local_derived->class_hierarchy.end(),
                         "PROCEDUREBASE") != local_derived->class_hierarchy.end(),
               "caller-local class lineage should include its loaded-procedure base class");
    }

    fs::remove_all(temp_root, ignored);
}

void test_newobject_local_vcx_materializes_native_class() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_local_vcx_activation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path class_library_path = temp_root / "myclasslib.vcx";
    write_synthetic_vcx_class_library(class_library_path);
    const fs::path main_path = temp_root / "newobject_local_vcx_activation.prg";
    write_text(
        main_path,
        "oWidget = NEWOBJECT('MyWidget', 'myclasslib.vcx', 40)\n"
        "nAnswer = oWidget.Answer()\n"
        "cCaption = oWidget.Caption\n"
        "cClass = oWidget.Class\n"
        "cBaseClass = oWidget.BaseClass\n"
        "cClassLibrary = oWidget.ClassLibrary\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("local VCX NEWOBJECT should activate a native class: ") + state.message);
    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };
    check("nanswer", "41");
    check("ccaption", "vcx-default");
    check("cclass", "MyWidget");
    check("cbaseclass", "Custom");
    check("cclasslibrary", class_library_path.string());
    expect(state.ole_objects.size() == 1U,
           "local VCX NEWOBJECT should create one native runtime object");
    expect(std::none_of(
               state.events.begin(),
               state.events.end(),
               [](const auto& event) { return event.category == "ole.newobject"; }),
           "local VCX NEWOBJECT should not emit an ole.newobject event");
    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const auto& event) { return event.category == "prg.object.newobject"; }),
           "local VCX NEWOBJECT should emit the native object activation event");

    fs::remove_all(temp_root, ignored);
}

void test_newobject_local_vcx_generated_source_consumes_companion_header() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_local_vcx_companion_header";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path class_library_path = temp_root / "headerclass.vcx";
    write_synthetic_vcx_class_library(
        class_library_path,
        "Custom",
        "FUNCTION Answer\r\n"
        "RETURN INCLUDED_VALUE\r\n"
        "ENDFUNC\r\n");
    write_text(temp_root / "headerclass.h", "#DEFINE INCLUDED_VALUE 42\n");
    const fs::path main_path = temp_root / "newobject_local_vcx_companion_header.prg";
    write_text(
        main_path,
        "oWidget = NEWOBJECT('MyWidget', 'headerclass.vcx', 40)\n"
        "nAnswer = oWidget.Answer()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("VCX generated source should consume its companion header: ") + state.message);
    const auto answer = state.globals.find("nanswer");
    expect(answer != state.globals.end(),
           "VCX companion-header test should expose the generated method result");
    if (answer != state.globals.end()) {
        expect(copperfin::runtime::format_value(answer->second) == "42",
               "VCX generated methods should see macros from the companion header");
    }

    fs::remove_all(temp_root, ignored);
}

void test_newobject_local_vcx_uses_verified_snapshot() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_local_vcx_verified";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path class_library_path = temp_root / "myclasslib.vcx";
    write_synthetic_vcx_class_library(class_library_path);
    fs::path memo_path = class_library_path;
    memo_path.replace_extension(".vct");
    const std::string verified_library_bytes = read_text(class_library_path);
    const std::string verified_memo_bytes = read_text(memo_path);
    const fs::path main_path = temp_root / "newobject_local_vcx_verified.prg";
    write_text(
        main_path,
        "oWidget = NEWOBJECT('MyWidget', 'myclasslib.vcx', 40)\n"
        "nAnswer = oWidget.Answer()\n"
        "cCaption = oWidget.Caption\n"
        "RETURN\n");

    auto options = make_runtime_session_options(main_path, temp_root);
    options.verified_file_byte_overrides.emplace(class_library_path.string(), verified_library_bytes);
    options.verified_file_byte_overrides.emplace(memo_path.string(), verified_memo_bytes);
    options.require_verified_file_byte_overrides = true;

    write_text(class_library_path, "tampered VCX bytes");
    fs::remove(memo_path, ignored);

    std::atomic<bool> writer_ready{false};
    std::atomic<bool> stop_writer{false};
    std::thread physical_swap_writer([&]()
    {
        write_text(class_library_path, "concurrently replaced VCX bytes");
        write_text(memo_path, "concurrently replaced VCT bytes");
        writer_ready.store(true);
        while (!stop_writer.load())
        {
            write_text(class_library_path, "concurrently replaced VCX bytes");
            write_text(memo_path, "concurrently replaced VCT bytes");
            std::this_thread::yield();
        }
    });
    while (!writer_ready.load())
    {
        std::this_thread::yield();
    }

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    stop_writer.store(true);
    physical_swap_writer.join();
    expect(state.completed,
           std::string("strict local VCX NEWOBJECT should use admitted bytes: ") + state.message);
    const auto answer = state.globals.find("nanswer");
    expect(answer != state.globals.end() && copperfin::runtime::format_value(answer->second) == "41",
           "strict local VCX NEWOBJECT should preserve the admitted Init and method source");
    const auto caption = state.globals.find("ccaption");
    expect(caption != state.globals.end() && copperfin::runtime::format_value(caption->second) == "vcx-default",
           "strict local VCX NEWOBJECT should preserve admitted memo-backed properties");

    fs::remove_all(temp_root, ignored);
}

void test_newobject_local_vcx_requires_verified_snapshot() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_local_vcx_unverified";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path class_library_path = temp_root / "myclasslib.vcx";
    write_synthetic_vcx_class_library(class_library_path);
    const fs::path main_path = temp_root / "newobject_local_vcx_unverified.prg";
    write_text(
        main_path,
        "oWidget = NEWOBJECT('MyWidget', 'myclasslib.vcx')\n"
        "RETURN\n");

    auto options = make_runtime_session_options(main_path, temp_root);
    options.require_verified_file_byte_overrides = true;
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed,
           "strict local VCX NEWOBJECT should fail without admitted bytes");
    expect(state.message.find("Verified package bytes are unavailable") != std::string::npos,
           "strict local VCX NEWOBJECT should report the localized verified-byte diagnostic");
    expect(state.ole_objects.empty(),
           "unverified local VCX NEWOBJECT should not register a runtime object");

    fs::remove_all(temp_root, ignored);
}

void test_newobject_external_prg_uses_admitted_source_when_path_is_absent() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_external_prg_verified";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path external_path = temp_root / "external_worker.prg";
    const fs::path main_path = temp_root / "newobject_external_prg_verified.prg";
    write_text(
        main_path,
        "oWorker = NEWOBJECT('ExternalWorker', 'external_worker.prg', 41)\n"
        "nAnswer = oWorker.Answer()\n"
        "RETURN\n");
    const std::string admitted_source =
        "DEFINE CLASS ExternalWorker AS Custom\n"
        "    PROCEDURE Init\n"
        "        LPARAMETERS seed\n"
        "        THIS.nSeed = seed\n"
        "    ENDPROC\n"
        "    FUNCTION Answer\n"
        "        RETURN THIS.nSeed + 1\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n";

    auto options = make_runtime_session_options(main_path, temp_root);
    options.startup_source_text = read_text(main_path);
    options.source_text_overrides.emplace(external_path.string(), admitted_source);
    options.require_source_text_overrides = true;
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("strict external PRG NEWOBJECT should use admitted source bytes: ") + state.message);
    const auto answer = state.globals.find("nanswer");
    expect(answer != state.globals.end() && copperfin::runtime::format_value(answer->second) == "42",
           "strict external PRG NEWOBJECT should execute the admitted class method");
    expect(state.ole_objects.size() == 1U,
           "strict external PRG NEWOBJECT should register one native object");
    if (state.ole_objects.size() == 1U)
    {
        expect(state.ole_objects.front().source == external_path.string(),
               "strict external PRG NEWOBJECT should retain the logical source path");
    }

    auto missing_options = make_runtime_session_options(main_path, temp_root);
    missing_options.startup_source_text = read_text(main_path);
    missing_options.require_source_text_overrides = true;
    copperfin::runtime::PrgRuntimeSession missing_session =
        copperfin::runtime::PrgRuntimeSession::create(missing_options);
    const auto missing_state = missing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!missing_state.completed,
           "strict external PRG NEWOBJECT should fail without admitted source bytes");
    expect(missing_state.message.find("Verified package source is unavailable") != std::string::npos,
           "strict external PRG NEWOBJECT should preserve the verified-source diagnostic");
    expect(missing_state.ole_objects.empty(),
           "unverified external PRG NEWOBJECT should not register a runtime object");

    fs::remove_all(temp_root, ignored);
}

void test_newobject_local_vcx_rejects_missing_class() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_local_vcx_missing_class";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path class_library_path = temp_root / "myclasslib.vcx";
    write_synthetic_vcx_class_library(class_library_path);
    const fs::path main_path = temp_root / "newobject_local_vcx_missing_class.prg";
    write_text(
        main_path,
        "oWidget = NEWOBJECT('MissingWidget', 'myclasslib.vcx')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed,
           "missing local VCX class should fail closed");
    expect(state.message.find("Class MissingWidget was not found in visual class library") !=
               std::string::npos,
           "missing local VCX class should report the localized class-not-found diagnostic");
    expect(state.ole_objects.empty(),
           "missing local VCX class should not register a runtime object");
    expect(std::none_of(
               state.events.begin(),
               state.events.end(),
               [](const auto& event) { return event.category == "ole.newobject"; }),
           "missing local VCX class should not emit an ole.newobject event");

    fs::remove_all(temp_root, ignored);
}

void test_newobject_local_vcx_rejects_missing_library() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_local_vcx_missing_library";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "newobject_local_vcx_missing_library.prg";
    write_text(
        main_path,
        "oWidget = NEWOBJECT('MyWidget', 'missing.vcx')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed,
           "missing local VCX library should fail closed");
    expect(state.message.find("Visual class library could not be opened for NEWOBJECT: missing.vcx") !=
               std::string::npos,
           "missing local VCX library should report the localized open-failure diagnostic");
    expect(state.ole_objects.empty(),
           "missing local VCX library should not register a runtime object");
    expect(std::none_of(
               state.events.begin(),
               state.events.end(),
               [](const auto& event) { return event.category == "ole.newobject"; }),
           "missing local VCX library should not emit an ole.newobject event");

    fs::remove_all(temp_root, ignored);
}

void test_newobject_local_vcx_fallback_details_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_local_vcx_fallback_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment catalog_environment;
    copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE");

    const auto run = [&](const fs::path& main_path, const fs::path& working_directory,
                         std::optional<fs::path> temp_directory = std::nullopt) {
        auto options = make_runtime_session_options(main_path, working_directory);
        if (temp_directory.has_value()) {
            options.temp_directory = temp_directory->string();
        }
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(options);
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    fs::create_directories(temp_root / "not_a_file.vcx");
    const auto missing_library_path = temp_root / "missing_library.prg";
    write_text(missing_library_path, "oWidget = NEWOBJECT('MyWidget', 'not_a_file.vcx')\nRETURN\n");
    const auto missing_library_state = run(missing_library_path, temp_root);
    expect(missing_library_state.message.find("archivo no encontrado") != std::string::npos,
           "#4424: Spanish VCX missing-file fallback should be localized");

    const auto invalid_base_library_path = temp_root / "invalid_base.vcx";
    write_synthetic_vcx_class_library(invalid_base_library_path, "Bad Base");
    const auto invalid_base_path = temp_root / "invalid_base.prg";
    write_text(invalid_base_path, "oWidget = NEWOBJECT('MyWidget', 'invalid_base.vcx')\nRETURN\n");
    const auto invalid_base_state = run(invalid_base_path, temp_root);
    expect(invalid_base_state.message.find("la BASECLASS raíz no es un identificador válido") != std::string::npos,
           "#4424: Spanish invalid-root-BASECLASS fallback should be localized");

    const auto generated_source_library_path = temp_root / "generated_source.vcx";
    write_synthetic_vcx_class_library(generated_source_library_path);
    const auto blocked_temp_path = temp_root / "blocked-runtime-temp";
    write_text(blocked_temp_path, "not a directory");
    const auto generated_source_path = temp_root / "generated_source.prg";
    write_text(generated_source_path, "oWidget = NEWOBJECT('MyWidget', 'generated_source.vcx')\nRETURN\n");
    const auto generated_source_state = run(generated_source_path, temp_root, blocked_temp_path);
    expect(generated_source_state.message.find("no se pudo escribir el origen de clase generado") != std::string::npos,
           "#4424: Spanish generated-source write fallback should be localized");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto pseudo_missing_library_path = temp_root / "pseudo_missing_library.prg";
    write_text(pseudo_missing_library_path, "oWidget = NEWOBJECT('MyWidget', 'missing.vcx')\nRETURN\n");
    const auto pseudo_missing_library_state = run(pseudo_missing_library_path, temp_root);
    expect(pseudo_missing_library_state.message.find("[!! ") == 0U &&
               pseudo_missing_library_state.message.find("file not found") == std::string::npos,
           "#4424: qps-ploc VCX missing-file fallback should expose pseudo-localization");

    fs::remove_all(temp_root, ignored);
}

void test_release_object_alias_waits_for_last_variable_reference() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_release_object_alias_reference";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "release_object_alias_reference.prg";
    write_text(
        main_path,
        "nDestroyCount = 0\n"
        "oFirst = CREATEOBJECT('AliasWorker')\n"
        "oSecond = oFirst\n"
        "RELEASE oFirst\n"
        "nDestroyAfterFirstRelease = nDestroyCount\n"
        "RELEASE oSecond\n"
        "nDestroyAfterSecondRelease = nDestroyCount\n"
        "RETURN\n"
        "DEFINE CLASS AliasWorker AS Custom\n"
        "    PROCEDURE Destroy\n"
        "        nDestroyCount = nDestroyCount + 1\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("RELEASE object alias script should complete: ") +
               state.message + " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        if (found == state.globals.end()) {
            expect(false, name + " variable not found");
            return;
        }
        const std::string actual = copperfin::runtime::format_value(found->second);
        expect(actual == expected,
               name + " expected '" + expected + "' got '" + actual + "'");
    };

    check("ndestroyafterfirstrelease", "0");
    check("ndestroyaftersecondrelease", "1");
    expect(state.ole_objects.empty(),
           "RELEASE of the last tracked object variable should remove the native object");

    fs::remove_all(temp_root, ignored);
}

void test_scope_exit_releases_unreferenced_objects_and_preserves_returns() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_scope_exit_object_lifetime";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "scope_exit_object_lifetime.prg";
    write_text(
        main_path,
        "nDestroyCount = 0\n"
        "DO MakeLocal\n"
        "nDestroyAfterLocal = nDestroyCount\n"
        "oGlobal = CREATEOBJECT('ScopeWorker')\n"
        "DO MakeAlias\n"
        "nDestroyAfterAliasScope = nDestroyCount\n"
        "RELEASE oGlobal\n"
        "nDestroyAfterGlobalRelease = nDestroyCount\n"
        "DO MakeArray\n"
        "nDestroyAfterArrayScope = nDestroyCount\n"
        "oReturned = MakeReturned()\n"
        "nDestroyAfterReturn = nDestroyCount\n"
        "RELEASE oReturned\n"
        "nDestroyAfterReturnedRelease = nDestroyCount\n"
        "RETURN\n"
        "PROCEDURE MakeLocal\n"
        "    LOCAL oLocal\n"
        "    oLocal = CREATEOBJECT('ScopeWorker')\n"
        "ENDPROC\n"
        "PROCEDURE MakeAlias\n"
        "    LOCAL oAlias\n"
        "    oAlias = oGlobal\n"
        "ENDPROC\n"
        "PROCEDURE MakeArray\n"
        "    LOCAL aObjects\n"
        "    DIMENSION aObjects[2]\n"
        "    aObjects[1] = CREATEOBJECT('ScopeWorker')\n"
        "    aObjects[2] = CREATEOBJECT('ScopeWorker')\n"
        "ENDPROC\n"
        "FUNCTION MakeReturned\n"
        "    LOCAL oLocal\n"
        "    oLocal = CREATEOBJECT('ScopeWorker')\n"
        "    RETURN oLocal\n"
        "ENDFUNC\n"
        "DEFINE CLASS ScopeWorker AS Custom\n"
        "    PROCEDURE Destroy\n"
        "        nDestroyCount = nDestroyCount + 1\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("scope-exit object lifetime script should complete: ") + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };
    check("ndestroyafterlocal", "1");
    check("ndestroyafteraliasscope", "1");
    check("ndestroyafterglobalrelease", "2");
    check("ndestroyafterarrayscope", "4");
    check("ndestroyafterreturn", "4");
    check("ndestroyafterreturnedrelease", "5");
    expect(state.ole_objects.empty(),
           "scope-exit lifetime test should release every unreferenced native object");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_set_procedure_classes_follow_vfp_activation_precedence();
    test_newobject_local_vcx_materializes_native_class();
    test_newobject_local_vcx_generated_source_consumes_companion_header();
    test_newobject_local_vcx_uses_verified_snapshot();
    test_newobject_local_vcx_requires_verified_snapshot();
    test_newobject_external_prg_uses_admitted_source_when_path_is_absent();
    test_newobject_local_vcx_rejects_missing_class();
    test_newobject_local_vcx_rejects_missing_library();
    test_newobject_local_vcx_fallback_details_localize();
    test_release_object_alias_waits_for_last_variable_reference();
    test_scope_exit_releases_unreferenced_objects_and_preserves_returns();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

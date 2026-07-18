// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

namespace {

using namespace copperfin::test_support;

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

void test_newobject_local_vcx_fails_closed_without_fabricated_ole_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_local_vcx_rejection";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "newobject_local_vcx_rejection.prg";
    write_text(
        main_path,
        "oWidget = NEWOBJECT('MyWidget', 'myclasslib.vcx')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed,
           "local VCX NEWOBJECT should fail instead of returning a fabricated object handle");
    expect(state.message.find("Visual class library support is not implemented for NEWOBJECT: myclasslib.vcx") !=
               std::string::npos,
           "local VCX NEWOBJECT should report the catalog-routed unsupported-library diagnostic");
    expect(state.ole_objects.empty(),
           "local VCX NEWOBJECT should not register an OLE callback object");
    expect(std::none_of(
               state.events.begin(),
               state.events.end(),
               [](const auto& event) { return event.category == "ole.newobject"; }),
           "local VCX NEWOBJECT should not emit an ole.newobject event");

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
    test_newobject_local_vcx_fails_closed_without_fabricated_ole_state();
    test_release_object_alias_waits_for_last_variable_reference();
    test_scope_exit_releases_unreferenced_objects_and_preserves_returns();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

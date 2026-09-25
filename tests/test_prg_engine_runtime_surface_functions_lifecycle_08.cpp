#include "test_prg_engine_runtime_surface_functions_support.h"
#include "copperfin/localization/localization.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_removeobject_destroys_subtree_and_invalidates_references()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_removeobject_destroy_lifecycle";
        const fs::path program_path = temp_root / "removeobject_destroy_lifecycle.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        write_text(
            program_path,
            "PUBLIC gnChildDestroyed, gnGrandDestroyed, gcDestroyOrder, gnReentrantChildDestroyed, gnReentrantOwnerDestroyed, glReentrantThisSurvived, gcCommandArgType, gcSiblingReleaseOrder, glAddDuringDestroyResult, goFinishedSibling, gnFinishedSiblingDestroyCount, gnSelfRemovalDelegateCount\n"
            "gnChildDestroyed = 0\n"
            "gnGrandDestroyed = 0\n"
            "gcDestroyOrder = ''\n"
            "gnReentrantChildDestroyed = 0\n"
            "gnReentrantOwnerDestroyed = 0\n"
            "glReentrantThisSurvived = .F.\n"
            "gcCommandArgType = ''\n"
            "gcSiblingReleaseOrder = ''\n"
            "glAddDuringDestroyResult = .T.\n"
            "gnFinishedSiblingDestroyCount = 0\n"
            "gnSelfRemovalDelegateCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.child\n"
            "oGrand = oChild.grand\n"
            "cSameHandleLookalike = 'object:DemoChild#2'\n"
            "nHandleLikeLiteralLength = LEN('object:Foo#123')\n"
            "DIMENSION aHeld[1]\n"
            "aHeld[1] = oChild\n"
            "oHeld = CREATEOBJECT('Collection')\n"
            "oHeld.Add(oChild, 'child')\n"
            "oHeld.Add('object:DemoChild#2', 'lookalike')\n"
            "cLiveHandleLiteralAfter = CaptureArgText('object:DemoChild#2', oForm.RemoveObject('child'))\n"
            "lRemoved = !PEMSTATUS(oForm, 'child', 1)\n"
            "nChildDestroyedAfter = gnChildDestroyed\n"
            "nGrandDestroyedAfter = gnGrandDestroyed\n"
            "cDestroyOrderAfter = gcDestroyOrder\n"
            "lAliasStillObject = VARTYPE(oChild) == 'O'\n"
            "lGrandAliasStillObject = VARTYPE(oGrand) == 'O'\n"
            "lArrayStillObject = VARTYPE(aHeld[1]) == 'O'\n"
            "lCollectionStillObject = VARTYPE(oHeld.Item('child')) == 'O'\n"
            "cCollectionLookalikeAfter = oHeld.Item('lookalike')\n"
            "lOwnerStillHasChild = PEMSTATUS(oForm, 'child', 1)\n"
            "lSiblingSurvives = VARTYPE(oForm.sibling) == 'O'\n"
            "cSameHandleLookalikeAfter = cSameHandleLookalike\n"
            "TRY\n"
            "  =oForm.RemoveObject('child')\n"
            "CATCH TO oMissing\n"
            "  nMissingError = oMissing.ErrorNo\n"
            "  cMissingMessage = oMissing.Message\n"
            "ENDTRY\n"
            "TRY\n"
            "  =oForm.RemoveObject('')\n"
            "CATCH TO oEmpty\n"
            "  nEmptyError = oEmpty.ErrorNo\n"
            "ENDTRY\n"
            "TRY\n"
            "  =oForm.RemoveObject('objects')\n"
            "CATCH TO oHidden\n"
            "  nHiddenError = oHidden.ErrorNo\n"
            "ENDTRY\n"
            "lSiblingAfterFailures = VARTYPE(oForm.sibling) == 'O'\n"
            "oReentrant = CREATEOBJECT('ReentrantForm')\n"
            "oReentrantChild = oReentrant.child\n"
            "lReentrantRemoved = oReentrant.RemoveObject('child')\n"
            "lReentrantOwnerStillObject = VARTYPE(oReentrant) == 'O'\n"
            "lReentrantChildStillObject = VARTYPE(oReentrantChild) == 'O'\n"
            "nReentrantChildDestroyed = gnReentrantChildDestroyed\n"
            "nReentrantOwnerDestroyed = gnReentrantOwnerDestroyed\n"
            "lReentrantThisSurvived = glReentrantThisSurvived\n"
            "oDirectForm = CREATEOBJECT('DemoForm')\n"
            "oDirectChild = oDirectForm.child\n"
            "cDirectArgType = CaptureArgType(oDirectChild, oDirectForm.RemoveObject('child'))\n"
            "oResumeForm = CREATEOBJECT('DemoForm')\n"
            "oResumeChild = oResumeForm.child\n"
            "cResumeArgType = CaptureArgType(oResumeChild, RemoveChild(oResumeForm))\n"
            "oCommandForm = CREATEOBJECT('DemoForm')\n"
            "oCommandChild = oCommandForm.child\n"
            "DO CaptureCommandArgs WITH oCommandChild, RemoveChild(oCommandForm)\n"
            "cCommandArgType = gcCommandArgType\n"
            "oProtected = CREATEOBJECT('ProtectedForm')\n"
            "TRY\n"
            "  =oProtected.RemoveObject('child')\n"
            "CATCH TO oProtectedError\n"
            "  nProtectedError = oProtectedError.ErrorNo\n"
            "ENDTRY\n"
            "lProtectedChildSurvives = PEMSTATUS(oProtected, 'child', 1)\n"
            "oBranchForm = CREATEOBJECT('BranchForm')\n"
            "lBranchRemoved = oBranchForm.RemoveObject('branch')\n"
            "cSiblingReleaseOrder = gcSiblingReleaseOrder\n"
            "lAddDuringDestroyResult = glAddDuringDestroyResult\n"
            "oFinishedForm = CREATEOBJECT('FinishedSiblingForm')\n"
            "goFinishedSibling = oFinishedForm.branch.a\n"
            "lFinishedBranchRemoved = oFinishedForm.RemoveObject('branch')\n"
            "nFinishedSiblingDestroyCount = gnFinishedSiblingDestroyCount\n"
            "oSelfForm = CREATEOBJECT('SelfRemovingForm')\n"
            "oSelfChild = oSelfForm.child\n"
            "oSelfSink = CREATEOBJECT('SelfRemovalSink')\n"
            "lSelfBound = BINDEVENT(oSelfChild, 'Ping', oSelfSink, 'OnPing', 1)\n"
            "=oSelfChild.Ping()\n"
            "lSelfChildStillObject = VARTYPE(oSelfChild) == 'O'\n"
            "nSelfRemovalDelegateCount = gnSelfRemovalDelegateCount\n"
            "RETURN\n"
            "FUNCTION CaptureArgType(toValue, tlRemoved)\n"
            "  RETURN VARTYPE(toValue)\n"
            "ENDFUNC\n"
            "FUNCTION CaptureArgText(tcValue, tlRemoved)\n"
            "  RETURN tcValue\n"
            "ENDFUNC\n"
            "FUNCTION RemoveChild(toForm)\n"
            "  RETURN toForm.RemoveObject('child')\n"
            "ENDFUNC\n"
            "PROCEDURE CaptureCommandArgs(toValue, tlRemoved)\n"
            "  gcCommandArgType = VARTYPE(toValue)\n"
            "ENDPROC\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "  PROCEDURE Init\n"
            "    THIS.AddObject('child', 'DemoChild')\n"
            "    THIS.AddObject('sibling', 'CommandButton')\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ReentrantForm AS Form\n"
            "  ADD OBJECT child AS ReentrantChild\n"
            "  PROCEDURE Destroy\n"
            "    gnReentrantOwnerDestroyed = gnReentrantOwnerDestroyed + 1\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ReentrantChild AS CommandButton\n"
            "  PROCEDURE Destroy\n"
            "    gnReentrantChildDestroyed = gnReentrantChildDestroyed + 1\n"
            "    THIS.Parent.Release()\n"
            "    glReentrantThisSurvived = VARTYPE(THIS) == 'O'\n"
            "    THIS.Release()\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ProtectedForm AS Form\n"
            "  PROTECTED child\n"
            "  ADD OBJECT child AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BranchForm AS Form\n"
            "  ADD OBJECT branch AS BranchContainer\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BranchContainer AS Container\n"
            "  ADD OBJECT a AS ReleaseSiblingA\n"
            "  ADD OBJECT b AS ReleaseSiblingB\n"
            "  PROCEDURE Destroy\n"
            "    glAddDuringDestroyResult = THIS.AddObject('late', 'CommandButton')\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ReleaseSiblingA AS CommandButton\n"
            "  PROCEDURE Destroy\n"
            "    gcSiblingReleaseOrder = gcSiblingReleaseOrder + 'A1'\n"
            "    THIS.Parent.b.Release()\n"
            "    gcSiblingReleaseOrder = gcSiblingReleaseOrder + 'A2'\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ReleaseSiblingB AS CommandButton\n"
            "  PROCEDURE Destroy\n"
            "    gcSiblingReleaseOrder = gcSiblingReleaseOrder + 'B'\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS FinishedSiblingForm AS Form\n"
            "  ADD OBJECT branch AS FinishedSiblingContainer\n"
            "ENDDEFINE\n"
            "DEFINE CLASS FinishedSiblingContainer AS Container\n"
            "  ADD OBJECT a AS FinishedSiblingA\n"
            "  ADD OBJECT b AS FinishedSiblingB\n"
            "ENDDEFINE\n"
            "DEFINE CLASS FinishedSiblingA AS CommandButton\n"
            "  PROCEDURE Destroy\n"
            "    gnFinishedSiblingDestroyCount = gnFinishedSiblingDestroyCount + 1\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS FinishedSiblingB AS CommandButton\n"
            "  PROCEDURE Destroy\n"
            "    goFinishedSibling.Release()\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SelfRemovingForm AS Form\n"
            "  ADD OBJECT child AS SelfRemovingChild\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SelfRemovingChild AS CommandButton\n"
            "  PROCEDURE Ping\n"
            "    THIS.Parent.RemoveObject('child')\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SelfRemovalSink AS Custom\n"
            "  PROCEDURE OnPing\n"
            "    gnSelfRemovalDelegateCount = gnSelfRemovalDelegateCount + 1\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoChild AS Container\n"
            "  ADD OBJECT grand AS DemoGrand\n"
            "  PROCEDURE Destroy\n"
            "    gnChildDestroyed = gnChildDestroyed + 1\n"
            "    gcDestroyOrder = gcDestroyOrder + 'C'\n"
            "  ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoGrand AS CommandButton\n"
            "  PROCEDURE Destroy\n"
            "    gnGrandDestroyed = gnGrandDestroyed + 1\n"
            "    gcDestroyOrder = gcDestroyOrder + 'G'\n"
            "  ENDPROC\n"
            "ENDDEFINE\n");

        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.localization_catalog = std::make_shared<const copperfin::localization::LocalizedCatalog>(
            copperfin::localization::load_catalogs(
                copperfin::localization::resolve_catalog_root(), "en-US"));
        const auto state = copperfin::runtime::PrgRuntimeSession::create(std::move(options))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "RQ-CF-PRG-036/#6288: REMOVEOBJECT lifecycle script should complete: " + state.message);

        const auto expect_global = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), "RQ-CF-PRG-036/#6288: " + name + " should be captured");
            if (found != state.globals.end())
            {
                const std::string actual = copperfin::runtime::format_value(found->second);
                expect(actual == expected,
                       "RQ-CF-PRG-036/#6288: " + name + " expected " + expected + ", got " + actual);
            }
        };

        expect_global("lremoved", "true");
        expect_global("nchilddestroyedafter", "1");
        expect_global("ngranddestroyedafter", "1");
        expect_global("cdestroyorderafter", "GC");
        expect_global("laliasstillobject", "false");
        expect_global("lgrandaliasstillobject", "false");
        expect_global("larraystillobject", "false");
        expect_global("lcollectionstillobject", "false");
        expect_global("ccollectionlookalikeafter", "object:DemoChild#2");
        expect_global("lownerstillhaschild", "false");
        expect_global("lsiblingsurvives", "true");
        expect_global("csamehandlelookalikeafter", "object:DemoChild#2");
        expect_global("clivehandleliteralafter", "object:DemoChild#2");
        expect_global("nhandlelikeliterallength", "14");
        expect_global("nmissingerror", "1925");
        expect_global("cmissingmessage", "Unknown member child.");
        expect_global("nemptyerror", "1925");
        expect_global("nhiddenerror", "1925");
        expect_global("lsiblingafterfailures", "true");
        expect_global("lreentrantremoved", "true");
        expect_global("lreentrantownerstillobject", "false");
        expect_global("lreentrantchildstillobject", "false");
        expect_global("nreentrantchilddestroyed", "1");
        expect_global("nreentrantownerdestroyed", "1");
        expect_global("lreentrantthissurvived", "true");
        expect_global("cdirectargtype", "U");
        expect_global("cresumeargtype", "U");
        expect_global("ccommandargtype", "U");
        expect_global("nprotectederror", "1925");
        expect_global("lprotectedchildsurvives", "true");
        expect_global("lbranchremoved", "true");
        expect_global("csiblingreleaseorder", "A1BA2");
        expect_global("laddduringdestroyresult", "false");
        expect_global("lfinishedbranchremoved", "true");
        expect_global("nfinishedsiblingdestroycount", "1");
        expect_global("lselfbound", "1");
        expect_global("lselfchildstillobject", "false");
        expect_global("nselfremovaldelegatecount", "1");

        const auto object_is_retired = [&](const std::string &prog_id)
        {
            return std::none_of(
                state.ole_objects.begin(),
                state.ole_objects.end(),
                [&](const auto &object) { return object.prog_id == prog_id; });
        };
        expect(object_is_retired("DemoChild"),
               "RQ-CF-PRG-036/#6288: removed child should leave no runtime object snapshot");
        expect(object_is_retired("DemoGrand"),
               "RQ-CF-PRG-036/#6288: removed descendant should leave no runtime object snapshot");

        const auto event_count = [&](const std::string &category, const std::string &detail)
        {
            return std::count_if(
                state.events.begin(),
                state.events.end(),
                [&](const auto &event)
                {
                    return event.category == category && event.detail == detail;
                });
        };
        expect(event_count("prg.object.removeobject", "DemoForm.child") == 4U,
               "RQ-CF-PRG-036/#6288: each successful removal should emit one event");
        expect(event_count("prg.object.destroy", "DemoGrand.Destroy") == 4U,
               "RQ-CF-PRG-036/#6288: each descendant Destroy should run exactly once");
        expect(event_count("prg.object.destroy", "DemoChild.Destroy") == 4U,
               "RQ-CF-PRG-036/#6288: each child Destroy should run exactly once");
        expect(event_count("prg.object.destroy", "ReentrantChild.Destroy") == 1U,
               "RQ-CF-PRG-036/#6288: reentrant child Destroy should run exactly once");
        expect(event_count("prg.object.destroy", "ReentrantForm.Destroy") == 1U,
               "RQ-CF-PRG-036/#6288: reentrant owner release should run Destroy exactly once");

        fs::remove_all(temp_root, ignored);
    }
}

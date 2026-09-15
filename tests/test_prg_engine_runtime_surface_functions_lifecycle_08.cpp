#include "test_prg_engine_runtime_surface_functions_support.h"

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
            "PUBLIC gnChildDestroyed, gnGrandDestroyed, gcDestroyOrder, gnReentrantChildDestroyed, gnReentrantOwnerDestroyed\n"
            "gnChildDestroyed = 0\n"
            "gnGrandDestroyed = 0\n"
            "gcDestroyOrder = ''\n"
            "gnReentrantChildDestroyed = 0\n"
            "gnReentrantOwnerDestroyed = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.child\n"
            "oGrand = oChild.grand\n"
            "DIMENSION aHeld[1]\n"
            "aHeld[1] = oChild\n"
            "oHeld = CREATEOBJECT('Collection')\n"
            "oHeld.Add(oChild, 'child')\n"
            "lRemoved = oForm.RemoveObject('child')\n"
            "nChildDestroyedAfter = gnChildDestroyed\n"
            "nGrandDestroyedAfter = gnGrandDestroyed\n"
            "cDestroyOrderAfter = gcDestroyOrder\n"
            "lAliasStillObject = VARTYPE(oChild) == 'O'\n"
            "lGrandAliasStillObject = VARTYPE(oGrand) == 'O'\n"
            "lArrayStillObject = VARTYPE(aHeld[1]) == 'O'\n"
            "lCollectionStillObject = VARTYPE(oHeld.Item('child')) == 'O'\n"
            "lOwnerStillHasChild = PEMSTATUS(oForm, 'child', 1)\n"
            "lSiblingSurvives = VARTYPE(oForm.sibling) == 'O'\n"
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
            "RETURN\n"
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

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(program_path.string(), temp_root.string()))
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
        expect_global("lownerstillhaschild", "false");
        expect_global("lsiblingsurvives", "true");
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
        expect(event_count("prg.object.removeobject", "DemoForm.child") == 1U,
               "RQ-CF-PRG-036/#6288: successful removal should emit one event");
        expect(event_count("prg.object.destroy", "DemoGrand.Destroy") == 1U,
               "RQ-CF-PRG-036/#6288: descendant Destroy should run exactly once");
        expect(event_count("prg.object.destroy", "DemoChild.Destroy") == 1U,
               "RQ-CF-PRG-036/#6288: child Destroy should run exactly once");
        expect(event_count("prg.object.destroy", "ReentrantChild.Destroy") == 1U,
               "RQ-CF-PRG-036/#6288: reentrant child Destroy should run exactly once");
        expect(event_count("prg.object.destroy", "ReentrantForm.Destroy") == 1U,
               "RQ-CF-PRG-036/#6288: reentrant owner release should run Destroy exactly once");

        fs::remove_all(temp_root, ignored);
    }
}

#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_picture_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_picture";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_picture.prg";
        write_text(
            main_path,
            "oImage = CREATEOBJECT('Image')\n"
            "oButton = CREATEOBJECT('CommandButton')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "cImageDefault = oImage.Picture\n"
            "cButtonDefault = GETPEM(oButton, 'Picture')\n"
            "lImageHas = PEMSTATUS(oImage, 'Picture', 1)\n"
            "lButtonHas = PEMSTATUS(oButton, 'Picture', 1)\n"
            "lLabelHas = PEMSTATUS(oLabel, 'Picture', 1)\n"
            "oImage.Picture = 'images\\logo.bmp'\n"
            "cDirect = oImage.Picture\n"
            "lSetPem = SETPEM(oButton, 'Picture', 'buttons\\run.bmp')\n"
            "cAfterSetPem = GETPEM(oButton, 'Picture')\n"
            "lPutPem = PUTPEM(oImage, 'Picture', 'images\\save.bmp')\n"
            "cAfterPutPem = oImage.Picture\n"
            "lAddProperty = ADDPROPERTY(oImage, 'Picture', 'other.bmp')\n"
            "lRemoveProperty = REMOVEPROPERTY(oImage, 'Picture')\n"
            "nMembers = AMEMBERS(aMembers, oImage, 1)\n"
            "lMembersHas = .F.\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'PICTURE'\n"
            "        lMembersHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedImage')\n"
            "cDerived = oDerived.Picture\n"
            "oDerived.Picture = 'derived.bmp'\n"
            "cDerivedAfter = GETPEM(oDerived, 'Picture')\n"
            "RETURN\n"
            "DEFINE CLASS DerivedImage AS Image\n"
            "    Picture = 'class-image.bmp'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Picture script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected) {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end()) {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("cimagedefault", "");
        check("cbuttondefault", "");
        check("limagehas", "true");
        check("lbuttonhas", "true");
        check("llabelhas", "false");
        check("cdirect", "images\\logo.bmp");
        check("lsetpem", "true");
        check("caftersetpem", "buttons\\run.bmp");
        check("lputpem", "true");
        check("cafterputpem", "images\\save.bmp");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmembershas", "true");
        check("cderived", "class-image.bmp");
        check("cderivedafter", "derived.bmp");

        fs::remove_all(temp_root, ignored);
    }
}

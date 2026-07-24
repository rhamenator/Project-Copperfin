#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_fontshadow_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fontshadow";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fontshadow.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'FontShadow', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'FontShadow', 5)\n"
            "lDefault = oButton.FontShadow\n"
            "oButton.FontShadow = .T.\n"
            "lDirect = oButton.FontShadow\n"
            "lSetPem = SETPEM(oButton, 'FontShadow', 0)\n"
            "lSetPemValue = GETPEM(oButton, 'FontShadow')\n"
            "lPutPem = PUTPEM(oButton, 'FontShadow', 'true')\n"
            "lPutPemValue = GETPEM(oButton, 'FontShadow')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'FontShadow', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'FontShadow')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FONTSHADOW'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFontShadow')\n"
            "lDerived = oDerived.FontShadow\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontShadow AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.FontShadow = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FontShadow script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("ldefault", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("lsetpemvalue", "false");
        check("lputpem", "true");
        check("lputpemvalue", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "true");
        expect(state.ole_objects.size() == 2U,
               "native FontShadow coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_fontoutline_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fontoutline";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fontoutline.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'FontOutline', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'FontOutline', 5)\n"
            "lDefault = oButton.FontOutline\n"
            "oButton.FontOutline = .T.\n"
            "lDirect = oButton.FontOutline\n"
            "lSetPem = SETPEM(oButton, 'FontOutline', 0)\n"
            "lSetPemValue = GETPEM(oButton, 'FontOutline')\n"
            "lPutPem = PUTPEM(oButton, 'FontOutline', 'true')\n"
            "lPutPemValue = GETPEM(oButton, 'FontOutline')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'FontOutline', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'FontOutline')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FONTOUTLINE'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFontOutline')\n"
            "lDerived = oDerived.FontOutline\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontOutline AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.FontOutline = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FontOutline script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("ldefault", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("lsetpemvalue", "false");
        check("lputpem", "true");
        check("lputpemvalue", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "true");
        expect(state.ole_objects.size() == 2U,
               "native FontOutline coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_fontstrikethru_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fontstrikethru";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fontstrikethru.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'FontStrikethru', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'FontStrikethru', 5)\n"
            "lDefault = oButton.FontStrikethru\n"
            "oButton.FontStrikethru = .T.\n"
            "lDirect = oButton.FontStrikethru\n"
            "lSetPem = SETPEM(oButton, 'FontStrikethru', 0)\n"
            "lSetPemValue = GETPEM(oButton, 'FontStrikethru')\n"
            "lPutPem = PUTPEM(oButton, 'FontStrikethru', 'true')\n"
            "lPutPemValue = GETPEM(oButton, 'FontStrikethru')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'FontStrikethru', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'FontStrikethru')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FONTSTRIKETHRU'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFontStrikethru')\n"
            "lDerived = oDerived.FontStrikethru\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontStrikethru AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.FontStrikethru = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FontStrikethru script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("ldefault", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("lsetpemvalue", "false");
        check("lputpem", "true");
        check("lputpemvalue", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "true");
        expect(state.ole_objects.size() == 2U,
               "native FontStrikethru coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_fontunderline_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fontunderline";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fontunderline.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'FontUnderline', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'FontUnderline', 5)\n"
            "lDefault = oButton.FontUnderline\n"
            "oButton.FontUnderline = .T.\n"
            "lDirect = oButton.FontUnderline\n"
            "lSetPem = SETPEM(oButton, 'FontUnderline', 0)\n"
            "lSetPemValue = GETPEM(oButton, 'FontUnderline')\n"
            "lPutPem = PUTPEM(oButton, 'FontUnderline', 'true')\n"
            "lPutPemValue = GETPEM(oButton, 'FontUnderline')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'FontUnderline', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'FontUnderline')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FONTUNDERLINE'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFontUnderline')\n"
            "lDerived = oDerived.FontUnderline\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontUnderline AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.FontUnderline = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FontUnderline script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("ldefault", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("lsetpemvalue", "false");
        check("lputpem", "true");
        check("lputpemvalue", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "true");
        expect(state.ole_objects.size() == 2U,
               "native FontUnderline coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_fontitalic_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fontitalic";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fontitalic.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'FontItalic', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'FontItalic', 5)\n"
            "lDefault = oButton.FontItalic\n"
            "oButton.FontItalic = .T.\n"
            "lDirect = oButton.FontItalic\n"
            "lSetPem = SETPEM(oButton, 'FontItalic', 0)\n"
            "lSetPemValue = GETPEM(oButton, 'FontItalic')\n"
            "lPutPem = PUTPEM(oButton, 'FontItalic', 'true')\n"
            "lPutPemValue = GETPEM(oButton, 'FontItalic')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'FontItalic', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'FontItalic')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FONTITALIC'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFontItalic')\n"
            "lDerived = oDerived.FontItalic\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontItalic AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.FontItalic = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FontItalic script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("ldefault", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("lsetpemvalue", "false");
        check("lputpem", "true");
        check("lputpemvalue", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "true");
        expect(state.ole_objects.size() == 2U,
               "native FontItalic coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_fontbold_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fontbold";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fontbold.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'FontBold', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'FontBold', 5)\n"
            "lDefault = oButton.FontBold\n"
            "oButton.FontBold = .T.\n"
            "lDirect = oButton.FontBold\n"
            "lSetPem = SETPEM(oButton, 'FontBold', 0)\n"
            "lSetPemValue = GETPEM(oButton, 'FontBold')\n"
            "lPutPem = PUTPEM(oButton, 'FontBold', 'true')\n"
            "lPutPemValue = GETPEM(oButton, 'FontBold')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'FontBold', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'FontBold')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FONTBOLD'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFontBold')\n"
            "lDerived = oDerived.FontBold\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontBold AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.FontBold = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FontBold script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("ldefault", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("lsetpemvalue", "false");
        check("lputpem", "true");
        check("lputpemvalue", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "true");
        expect(state.ole_objects.size() == 2U,
               "native FontBold coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_fontsize_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fontsize";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fontsize.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'FontSize', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'FontSize', 5)\n"
            "nDefault = oButton.FontSize\n"
            "oButton.FontSize = 12.5\n"
            "nDirect = oButton.FontSize\n"
            "lSetPem = SETPEM(oButton, 'FontSize', 9.75)\n"
            "nSetPem = GETPEM(oButton, 'FontSize')\n"
            "lPutPem = PUTPEM(oButton, 'FontSize', -1)\n"
            "nPutPem = GETPEM(oButton, 'FontSize')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'FontSize', 3)\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'FontSize')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FONTSIZE'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFontSize')\n"
            "nDerived = oDerived.FontSize\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontSize AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.FontSize = 8.25\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FontSize script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("ndefault", "10");
        check("ndirect", "12.5");
        check("lsetpem", "true");
        check("nsetpem", "9.75");
        check("lputpem", "true");
        check("nputpem", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("nderived", "8.25");
        expect(state.ole_objects.size() == 2U,
               "native FontSize coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_fontname_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fontname";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fontname.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'FontName', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'FontName', 5)\n"
            "cDefault = oButton.FontName\n"
            "oButton.FontName = 'Tahoma'\n"
            "cDirect = oButton.FontName\n"
            "lSetPem = SETPEM(oButton, 'FontName', 'Verdana')\n"
            "cSetPem = GETPEM(oButton, 'FontName')\n"
            "lPutPem = PUTPEM(oButton, 'FontName', 'Consolas')\n"
            "cPutPem = GETPEM(oButton, 'FontName')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'FontName', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'FontName')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FONTNAME'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFontName')\n"
            "cDerived = oDerived.FontName\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontName AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.FontName = 'Segoe UI'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FontName script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("cdefault", "Arial");
        check("cdirect", "Tahoma");
        check("lsetpem", "true");
        check("csetpem", "Verdana");
        check("lputpem", "true");
        check("cputpem", "Consolas");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("cderived", "Segoe UI");
        expect(state.ole_objects.size() == 2U,
               "native FontName coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_dynamicfontname_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_dynamicfontname";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_dynamicfontname.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('Column')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'DynamicFontName', 1)\n"
            "lControlHas = PEMSTATUS(oControl, 'DynamicFontName', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'DynamicFontName', 5)\n"
            "cDefault = oButton.DynamicFontName\n"
            "oButton.DynamicFontName = 'IIF(EMPTY(Value), 999999, 111111)'\n"
            "cDirect = oButton.DynamicFontName\n"
            "lSetPem = SETPEM(oButton, 'DynamicFontName', 'UPPER(Value)')\n"
            "cSetPem = GETPEM(oButton, 'DynamicFontName')\n"
            "lPutPem = PUTPEM(oButton, 'DynamicFontName', 'TRANSFORM(Value)')\n"
            "cPutPem = GETPEM(oButton, 'DynamicFontName')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'DynamicFontName', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'DynamicFontName')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DYNAMICFONTNAME'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDynamicFontName')\n"
            "cDerived = oDerived.DynamicFontName\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDynamicFontName AS Column\n"
            "    PROCEDURE Init\n"
            "        THIS.DynamicFontName = 'IIF(Value > 0, 1, 0)'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DynamicFontName script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lcontrolhas", "false");
        check("lreadonly", "false");
        check("cdefault", "");
        check("cdirect", "IIF(EMPTY(Value), 999999, 111111)");
        check("lsetpem", "true");
        check("csetpem", "UPPER(Value)");
        check("lputpem", "true");
        check("cputpem", "TRANSFORM(Value)");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("cderived", "IIF(Value > 0, 1, 0)");
        expect(state.ole_objects.size() == 3U,
               "native DynamicFontName coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_dynamicfontsize_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_dynamicfontsize";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_dynamicfontsize.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('Column')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'DynamicFontSize', 1)\n"
            "lControlHas = PEMSTATUS(oControl, 'DynamicFontSize', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'DynamicFontSize', 5)\n"
            "cDefault = oButton.DynamicFontSize\n"
            "oButton.DynamicFontSize = 'IIF(EMPTY(Value), 10, 12.5)'\n"
            "cDirect = oButton.DynamicFontSize\n"
            "lSetPem = SETPEM(oButton, 'DynamicFontSize', 'MAX(Value, 8)')\n"
            "cSetPem = GETPEM(oButton, 'DynamicFontSize')\n"
            "lPutPem = PUTPEM(oButton, 'DynamicFontSize', 'TRANSFORM(Value)')\n"
            "cPutPem = GETPEM(oButton, 'DynamicFontSize')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'DynamicFontSize', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'DynamicFontSize')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DYNAMICFONTSIZE'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDynamicFontSize')\n"
            "cDerived = oDerived.DynamicFontSize\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDynamicFontSize AS Column\n"
            "    PROCEDURE Init\n"
            "        THIS.DynamicFontSize = 'IIF(Value > 0, 9.5, 11)'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DynamicFontSize script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lcontrolhas", "false");
        check("lreadonly", "false");
        check("cdefault", "");
        check("cdirect", "IIF(EMPTY(Value), 10, 12.5)");
        check("lsetpem", "true");
        check("csetpem", "MAX(Value, 8)");
        check("lputpem", "true");
        check("cputpem", "TRANSFORM(Value)");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("cderived", "IIF(Value > 0, 9.5, 11)");
        expect(state.ole_objects.size() == 3U,
               "native DynamicFontSize coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_dynamicfontshadow_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_dynamicfontshadow";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_dynamicfontshadow.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('Column')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'DynamicFontShadow', 1)\n"
            "lControlHas = PEMSTATUS(oControl, 'DynamicFontShadow', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'DynamicFontShadow', 5)\n"
            "cDefault = oButton.DynamicFontShadow\n"
            "oButton.DynamicFontShadow = 'IIF(EMPTY(Value), .F., .T.)'\n"
            "cDirect = oButton.DynamicFontShadow\n"
            "lSetPem = SETPEM(oButton, 'DynamicFontShadow', 'Value > 0')\n"
            "cSetPem = GETPEM(oButton, 'DynamicFontShadow')\n"
            "lPutPem = PUTPEM(oButton, 'DynamicFontShadow', 'TRANSFORM(Value)')\n"
            "cPutPem = GETPEM(oButton, 'DynamicFontShadow')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'DynamicFontShadow', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'DynamicFontShadow')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DYNAMICFONTSHADOW'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDynamicFontShadow')\n"
            "cDerived = oDerived.DynamicFontShadow\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDynamicFontShadow AS Column\n"
            "    PROCEDURE Init\n"
            "        THIS.DynamicFontShadow = 'IIF(Value > 0, .T., .F.)'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DynamicFontShadow script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lcontrolhas", "false");
        check("lreadonly", "false");
        check("cdefault", "");
        check("cdirect", "IIF(EMPTY(Value), .F., .T.)");
        check("lsetpem", "true");
        check("csetpem", "Value > 0");
        check("lputpem", "true");
        check("cputpem", "TRANSFORM(Value)");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("cderived", "IIF(Value > 0, .T., .F.)");
        expect(state.ole_objects.size() == 3U,
               "native DynamicFontShadow coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_dynamicfontoutline_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_dynamicfontoutline";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_dynamicfontoutline.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('Column')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'DynamicFontOutline', 1)\n"
            "lControlHas = PEMSTATUS(oControl, 'DynamicFontOutline', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'DynamicFontOutline', 5)\n"
            "cDefault = oButton.DynamicFontOutline\n"
            "oButton.DynamicFontOutline = 'IIF(EMPTY(Value), .F., .T.)'\n"
            "cDirect = oButton.DynamicFontOutline\n"
            "lSetPem = SETPEM(oButton, 'DynamicFontOutline', 'Value > 0')\n"
            "cSetPem = GETPEM(oButton, 'DynamicFontOutline')\n"
            "lPutPem = PUTPEM(oButton, 'DynamicFontOutline', 'TRANSFORM(Value)')\n"
            "cPutPem = GETPEM(oButton, 'DynamicFontOutline')\n"
            "lAddProperty = ADDPROPERTY(oButton, 'DynamicFontOutline', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'DynamicFontOutline')\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DYNAMICFONTOUTLINE'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDynamicFontOutline')\n"
            "cDerived = oDerived.DynamicFontOutline\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDynamicFontOutline AS Column\n"
            "    PROCEDURE Init\n"
            "        THIS.DynamicFontOutline = 'IIF(Value > 0, .T., .F.)'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DynamicFontOutline script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lcontrolhas", "false");
        check("lreadonly", "false");
        check("cdefault", "");
        check("cdirect", "IIF(EMPTY(Value), .F., .T.)");
        check("lsetpem", "true");
        check("csetpem", "Value > 0");
        check("lputpem", "true");
        check("cputpem", "TRANSFORM(Value)");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("cderived", "IIF(Value > 0, .T., .F.)");
        expect(state.ole_objects.size() == 3U,
               "native DynamicFontOutline coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_mousepointer_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_mousepointer";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_mousepointer.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'MousePointer', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'MousePointer', 5)\n"
            "nDefault = oButton.MousePointer\n"
            "oButton.MousePointer = 2\n"
            "nDirect = GETPEM(oButton, 'MousePointer')\n"
            "lSetPem = SETPEM(oButton, 'MousePointer', 4)\n"
            "nSetPem = GETPEM(oButton, 'MousePointer')\n"
            "lPutPem = PUTPEM(oButton, 'MousePointer', 6)\n"
            "nPutPem = GETPEM(oButton, 'MousePointer')\n"
            "oButton.MousePointer = -1\n"
            "nNormalized = oButton.MousePointer\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'MOUSEPOINTER'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedMousePointer')\n"
            "nDerived = oDerived.MousePointer\n"
            "RETURN\n"
            "DEFINE CLASS DerivedMousePointer AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.MousePointer = 7\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native MousePointer script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("ndefault", "0");
        check("ndirect", "2");
        check("lsetpem", "true");
        check("nsetpem", "4");
        check("lputpem", "true");
        check("nputpem", "6");
        check("nnormalized", "0");
        check("lprophas", "true");
        check("nderived", "7");
        expect(state.ole_objects.size() == 2U,
               "native MousePointer coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_caption_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_caption";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_caption.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('CaptionForm')\n"
            "oPage = CREATEOBJECT('Page')\n"
            "oPageFrame = CREATEOBJECT('PageFrame')\n"
            "oGroup = CREATEOBJECT('CommandGroup')\n"
            "oOptionGroup = CREATEOBJECT('OptionGroup')\n"
            "lFormHas = PEMSTATUS(oForm, 'Caption', 1)\n"
            "lTextHas = PEMSTATUS(oForm.txtValue, 'Caption', 1)\n"
            "lPageHas = PEMSTATUS(oPage, 'Caption', 1)\n"
            "lPageFrameHas = PEMSTATUS(oPageFrame, 'Caption', 1)\n"
            "lGroupHas = PEMSTATUS(oGroup, 'Caption', 1)\n"
            "lOptionGroupHas = PEMSTATUS(oOptionGroup, 'Caption', 1)\n"
            "cBefore = oForm.Caption\n"
            "xBefore = GETPEM(oForm, 'Caption')\n"
            "oForm.Caption = 'Main form'\n"
            "cAfterDirect = oForm.Caption\n"
            "lSetPem = SETPEM(oForm, 'Caption', 'Updated form')\n"
            "cAfterSetPem = GETPEM(oForm, 'Caption')\n"
            "lAddProperty = ADDPROPERTY(oForm, 'Caption', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oForm, 'Caption')\n"
            "nMembers = AMEMBERS(aMembers, oForm, 1)\n"
            "lMember = .F.\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'CAPTION'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "cChildBefore = oForm.cmdSave.Caption\n"
            "cChildRead = oForm.cmdSave.ReadCaption()\n"
            "oForm.cmdSave.SetCaption()\n"
            "cChildAfter = oForm.cmdSave.Caption\n"
            "cDerived = CREATEOBJECT('DerivedCaptionButton').Caption\n"
            "RETURN\n"
            "DEFINE CLASS CaptionProbe AS CommandButton\n"
            "    FUNCTION ReadCaption\n"
            "        RETURN THISFORM.cmdSave.Caption\n"
            "    ENDFUNC\n"
            "    PROCEDURE SetCaption\n"
            "        THISFORM.cmdSave.Caption = 'child updated'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CaptionForm AS Form\n"
            "    Caption = 'Initial form'\n"
            "    ADD OBJECT txtValue AS TextBox\n"
            "    ADD OBJECT cmdSave AS CaptionProbe WITH Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedCaptionButton AS CommandButton\n"
            "    Caption = 'Derived'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Caption script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        check("lformhas", "true");
        check("ltexthas", "false");
        check("lpagehas", "true");
        check("lpageframehas", "true");
        check("lgrouphas", "true");
        check("loptiongrouphas", "true");
        check("cbefore", "Initial form");
        check("xbefore", "Initial form");
        check("cafterdirect", "Main form");
        check("lsetpem", "true");
        check("caftersetpem", "Updated form");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cchildbefore", "Save");
        check("cchildread", "Save");
        check("cchildafter", "child updated");
        check("cderived", "Derived");

        fs::remove_all(temp_root, ignored);
    }
}

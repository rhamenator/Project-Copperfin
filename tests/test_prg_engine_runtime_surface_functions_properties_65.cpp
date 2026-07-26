#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_relation_onetomany_defaults_are_readonly_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_relation_onetomany";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_relation_onetomany.prg";
        write_text(
            main_path,
            "oRelation = CREATEOBJECT('EmptyOneToManyRelation')\n"
            "lHas = PEMSTATUS(oRelation, 'OneToMany', 1)\n"
            "lReadOnly = PEMSTATUS(oRelation, 'OneToMany', 5)\n"
            "lDefault = !oRelation.OneToMany\n"
            "lGetPemDefault = !GETPEM(oRelation, 'OneToMany')\n"
            "oRelation.OneToMany = .T.\n"
            "lAfterDirect = !oRelation.OneToMany\n"
            "lSetPem = SETPEM(oRelation, 'OneToMany', .T.)\n"
            "lAfterSetPem = !GETPEM(oRelation, 'OneToMany')\n"
            "lPutPem = PUTPEM(oRelation, 'OneToMany', .T.)\n"
            "lAfterPutPem = !GETPEM(oRelation, 'OneToMany')\n"
            "lAdd = ADDPROPERTY(oRelation, 'OneToMany', .T.)\n"
            "lRemove = REMOVEPROPERTY(oRelation, 'OneToMany')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oRelation, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'ONETOMANY'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedOneToManyRelation')\n"
            "lDerived = oDerived.OneToMany\n"
            "RETURN\n"
            "DEFINE CLASS EmptyOneToManyRelation AS Relation\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedOneToManyRelation AS Relation\n"
            "    OneToMany = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Relation OneToMany script should complete: ") + state.message +
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
        check("lreadonly", "true");
        check("ldefault", "true");
        check("lgetpemdefault", "true");
        check("lafterdirect", "true");
        check("lsetpem", "false");
        check("laftersetpem", "true");
        check("lputpem", "false");
        check("lafterputpem", "true");
        check("ladd", "false");
        check("lremove", "false");
        check("lmember", "true");
        check("lderived", "true");

        fs::remove_all(temp_root, ignored);
    }
}

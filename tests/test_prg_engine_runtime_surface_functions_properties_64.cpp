#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_relation_relationalexpr_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_relationalexpr";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_relationalexpr.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oRelation = CREATEOBJECT('EmptyRelationalRelation')\n"
            "lGridHas = PEMSTATUS(oGrid, 'RelationalExpr', 1)\n"
            "lRelationHas = PEMSTATUS(oRelation, 'RelationalExpr', 1)\n"
            "cGridDefault = oGrid.RelationalExpr\n"
            "cRelationDefault = oRelation.RelationalExpr\n"
            "oGrid.RelationalExpr = 'customer_id'\n"
            "cGridDirect = oGrid.RelationalExpr\n"
            "lGridSetPem = SETPEM(oGrid, 'RelationalExpr', 'customer_id + 1')\n"
            "cGridAfterSetPem = GETPEM(oGrid, 'RelationalExpr')\n"
            "lRelationPutPem = PUTPEM(oRelation, 'RelationalExpr', 'customer_id')\n"
            "cRelationAfterPutPem = GETPEM(oRelation, 'RelationalExpr')\n"
            "lGridAdd = ADDPROPERTY(oGrid, 'RelationalExpr', 'shadow')\n"
            "lRelationRemove = REMOVEPROPERTY(oRelation, 'RelationalExpr')\n"
            "lGridMember = .F.\n"
            "lRelationMember = .F.\n"
            "nGridMembers = AMEMBERS(aGridMembers, oGrid, 1)\n"
            "nRelationMembers = AMEMBERS(aRelationMembers, oRelation, 1)\n"
            "FOR i = 1 TO nGridMembers\n"
            "    IF UPPER(aGridMembers[i]) == 'RELATIONALEXPR'\n"
            "        lGridMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "FOR i = 1 TO nRelationMembers\n"
            "    IF UPPER(aRelationMembers[i]) == 'RELATIONALEXPR'\n"
            "        lRelationMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerivedGrid = CREATEOBJECT('DerivedRelationalGrid')\n"
            "oDerivedRelation = CREATEOBJECT('DerivedRelationalRelation')\n"
            "cDerivedGrid = oDerivedGrid.RelationalExpr\n"
            "cDerivedRelation = oDerivedRelation.RelationalExpr\n"
            "RETURN\n"
            "DEFINE CLASS DerivedRelationalGrid AS Grid\n"
            "    RelationalExpr = 'grid_expr'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedRelationalRelation AS Relation\n"
            "    RelationalExpr = 'relation_expr'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS EmptyRelationalRelation AS Relation\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid/Relation RelationalExpr script should complete: ") + state.message +
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

        check("lgridhas", "true");
        check("lrelationhas", "true");
        check("cgriddefault", "");
        check("crelationdefault", "");
        check("cgriddirect", "customer_id");
        check("lgridsetpem", "true");
        check("cgridaftersetpem", "customer_id + 1");
        check("lrelationputpem", "true");
        check("crelationafterputpem", "customer_id");
        check("lgridadd", "false");
        check("lrelationremove", "false");
        check("lgridmember", "true");
        check("lrelationmember", "true");
        check("cderivedgrid", "grid_expr");
        check("cderivedrelation", "relation_expr");

        fs::remove_all(temp_root, ignored);
    }
}

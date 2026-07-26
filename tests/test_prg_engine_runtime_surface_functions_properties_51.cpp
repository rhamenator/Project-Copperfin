#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_column_tag_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_column_tag";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_column_tag.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lHeaderHas = PEMSTATUS(oHeader, 'Tag', 1)\n"
            "lColumnHas = PEMSTATUS(oColumn, 'Tag', 1)\n"
            "cHeaderDefault = oHeader.Tag\n"
            "cColumnDefault = oColumn.Tag\n"
            "oHeader.Tag = 'header-direct'\n"
            "cHeaderDirect = oHeader.Tag\n"
            "lColumnSetPem = SETPEM(oColumn, 'Tag', 'column-set')\n"
            "cColumnSetPem = GETPEM(oColumn, 'Tag')\n"
            "lHeaderPutPem = PUTPEM(oHeader, 'Tag', 'header-put')\n"
            "cHeaderPutPem = GETPEM(oHeader, 'Tag')\n"
            "lHeaderAdd = ADDPROPERTY(oHeader, 'Tag', 'shadow')\n"
            "lHeaderRemove = REMOVEPROPERTY(oHeader, 'Tag')\n"
            "lColumnAdd = ADDPROPERTY(oColumn, 'Tag', 'shadow')\n"
            "lColumnRemove = REMOVEPROPERTY(oColumn, 'Tag')\n"
            "lHeaderMember = .F.\n"
            "nHeaderMembers = AMEMBERS(aHeaderMembers, oHeader, 1)\n"
            "FOR i = 1 TO nHeaderMembers\n"
            "    IF UPPER(aHeaderMembers[i]) == 'TAG'\n"
            "        lHeaderMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Header.Tag = 'grid-header'\n"
            "cGridHeaderTag = oGrid.Columns(1).Header.Tag\n"
            "oDerivedHeader = CREATEOBJECT('DerivedTagHeader')\n"
            "oDerivedColumn = CREATEOBJECT('DerivedTagColumn')\n"
            "cDerivedHeaderTag = oDerivedHeader.Tag\n"
            "cDerivedColumnTag = oDerivedColumn.Tag\n"
            "RETURN\n"
            "DEFINE CLASS DerivedTagHeader AS Header\n"
            "    Tag = 'derived-header'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedTagColumn AS Column\n"
            "    Tag = 'derived-column'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header/Column Tag script should complete: ") + state.message +
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

        check("lheaderhas", "true");
        check("lcolumnhas", "true");
        check("cheaderdefault", "");
        check("ccolumndefault", "");
        check("cheaderdirect", "header-direct");
        check("lcolumnsetpem", "true");
        check("ccolumnsetpem", "column-set");
        check("lheaderputpem", "true");
        check("cheaderputpem", "header-put");
        check("lheaderadd", "false");
        check("lheaderremove", "false");
        check("lcolumnadd", "false");
        check("lcolumnremove", "false");
        check("lheadermember", "true");
        check("cgridheadertag", "grid-header");
        check("cderivedheadertag", "derived-header");
        check("cderivedcolumntag", "derived-column");

        fs::remove_all(temp_root, ignored);
    }
}

#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_cursor_xml_numeric_metadata_fails_closed()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root =
            fs::temp_directory_path() / "copperfin_prg_engine_cursor_xml_numeric_metadata";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path existing_path = temp_root / "existing.dbf";
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "ID", .type = 'N', .length = 4U},
            {.name = "NAME", .type = 'C', .length = 20U}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            existing_path.string(),
            fields,
            {{"1", "ALPHA"}, {"2", "BETA"}});
        expect(create_result.ok, "XMLTOCURSOR numeric-metadata fixture should be created");
        const std::string original_bytes = read_text(existing_path);

        const auto cursor_xml = [](const std::string &numeric_attributes)
        {
            return std::string("<CopperfinCursor alias=\"Source\"><Fields>") +
                   "<Field name=\"ID\" type=\"N\" " + numeric_attributes + " />" +
                   "<Field name=\"NAME\" type=\"C\" width=\"20\" decimals=\"0\" />" +
                   "</Fields><Rows><Row><Col>3</Col><Col>GAMMA</Col></Row>" +
                   "</Rows></CopperfinCursor>";
        };
        const std::array<std::pair<std::string, std::string>, 7U> malformed_cases{{
            {"partial_width.xml", "width=\"4junk\" decimals=\"0\""},
            {"grouped_width.xml", "width=\"2.0\" decimals=\"0\""},
            {"negative_width.xml", "width=\"-4\" decimals=\"0\""},
            {"missing_width.xml", "decimals=\"0\""},
            {"overflow_width.xml", "width=\"999999999999999999999999999999999\" decimals=\"0\""},
            {"partial_decimals.xml", "width=\"4\" decimals=\"0junk\""},
            {"missing_decimals.xml", "width=\"4\""}
        }};
        for (const auto &[file_name, numeric_attributes] : malformed_cases)
        {
            write_text(temp_root / file_name, cursor_xml(numeric_attributes));
        }

        const fs::path main_path = temp_root / "cursor_xml_numeric_metadata.prg";
        write_text(
            main_path,
            "USE 'existing.dbf' ALIAS ExistingXml\n"
            "GO 2\n"
            "nPartialExisting = XMLTOCURSOR('partial_width.xml', 'ExistingXml')\n"
            "nGroupedNew = XMLTOCURSOR('grouped_width.xml', 'GroupedDest')\n"
            "nNegativeExisting = XMLTOCURSOR('negative_width.xml', 'ExistingXml')\n"
            "nMissingWidthNew = XMLTOCURSOR('missing_width.xml', 'MissingWidthDest')\n"
            "nOverflowExisting = XMLTOCURSOR('overflow_width.xml', 'ExistingXml')\n"
            "nPartialDecimalsNew = XMLTOCURSOR('partial_decimals.xml', 'PartialDecimalsDest')\n"
            "nMissingDecimalsNew = XMLTOCURSOR('missing_decimals.xml', 'MissingDecimalsDest')\n"
            "lGroupedUsed = USED('GroupedDest')\n"
            "lMissingWidthUsed = USED('MissingWidthDest')\n"
            "lPartialDecimalsUsed = USED('PartialDecimalsDest')\n"
            "lMissingDecimalsUsed = USED('MissingDecimalsDest')\n"
            "cAliasAfterFailures = ALIAS()\n"
            "nRecnoAfterFailures = RECNO()\n"
            "nExistingCount = RECCOUNT()\n"
            "nExistingId = ID\n"
            "cExistingName = NAME\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("XMLTOCURSOR malformed numeric metadata should fail safely: ") +
                   state.message + " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            if (found == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        };

        check("npartialexisting", "0");
        check("ngroupednew", "0");
        check("nnegativeexisting", "0");
        check("nmissingwidthnew", "0");
        check("noverflowexisting", "0");
        check("npartialdecimalsnew", "0");
        check("nmissingdecimalsnew", "0");
        check("lgroupedused", "false");
        check("lmissingwidthused", "false");
        check("lpartialdecimalsused", "false");
        check("lmissingdecimalsused", "false");
        check("caliasafterfailures", "ExistingXml");
        check("nrecnoafterfailures", "2");
        check("nexistingcount", "2");
        check("nexistingid", "2");
        check("cexistingname", "BETA");

        expect(read_text(existing_path) == original_bytes,
               "rejected XMLTOCURSOR numeric metadata should preserve destination bytes");
        expect(std::count_if(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "runtime.warning";
        }) >= malformed_cases.size(),
               "each malformed XMLTOCURSOR numeric field should emit a warning event");
        expect(std::none_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "runtime.xmltocursor";
        }),
               "rejected XMLTOCURSOR numeric metadata should not emit success events");

        fs::remove_all(temp_root, ignored);
    }

    void test_cursor_xml_cardinality_mismatch_preserves_destinations()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cursor_xml_cardinality";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path existing_path = temp_root / "existing.dbf";
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "ID", .type = 'N', .length = 4U},
            {.name = "NAME", .type = 'C', .length = 20U}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            existing_path.string(),
            fields,
            {{"1", "ALPHA"}, {"2", "BETA"}});
        expect(create_result.ok, "XMLTOCURSOR cardinality fixture should be created");
        const std::string original_bytes = read_text(existing_path);

        const std::string xml_prefix =
            "<CopperfinCursor alias=\"Source\"><Fields>"
            "<Field name=\"ID\" type=\"N\" width=\"4\" decimals=\"0\" />"
            "<Field name=\"NAME\" type=\"C\" width=\"20\" decimals=\"0\" />"
            "</Fields><Rows><Row>";
        const std::string xml_suffix = "</Row></Rows></CopperfinCursor>";
        const std::string extra_column_xml =
            xml_prefix + "<Col>3</Col><Col>GAMMA</Col><Col>EXTRA</Col>" + xml_suffix;
        const std::string missing_column_xml = xml_prefix + "<Col>3</Col>" + xml_suffix;
        write_text(temp_root / "extra_columns.xml", extra_column_xml);
        write_text(temp_root / "missing_columns.xml", missing_column_xml);

        const fs::path main_path = temp_root / "cursor_xml_cardinality.prg";
        write_text(
            main_path,
            "USE 'existing.dbf' ALIAS ExistingXml\n"
            "GO 2\n"
            "cInlineExtra = '" + extra_column_xml + "'\n"
            "cInlineMissing = '" + missing_column_xml + "'\n"
            "nInlineExtraExisting = XMLTOCURSOR(cInlineExtra, 'ExistingXml')\n"
            "nInlineMissingNew = XMLTOCURSOR(cInlineMissing, 'InlineMissingDest')\n"
            "nFileExtraNew = XMLTOCURSOR('extra_columns.xml', 'FileExtraDest')\n"
            "nFileMissingExisting = XMLTOCURSOR('missing_columns.xml', 'ExistingXml')\n"
            "lInlineMissingUsed = USED('InlineMissingDest')\n"
            "lFileExtraUsed = USED('FileExtraDest')\n"
            "lExistingUsed = USED('ExistingXml')\n"
            "cAliasAfterFailures = ALIAS()\n"
            "nRecnoAfterFailures = RECNO()\n"
            "SELECT ExistingXml\n"
            "nExistingCount = RECCOUNT()\n"
            "nExistingRecno = RECNO()\n"
            "nExistingId = ID\n"
            "cExistingName = NAME\n"
            "cExistingAlias = ALIAS()\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("XMLTOCURSOR cardinality script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto found = state.globals.find(name);
            if (found == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        };

        check("ninlineextraexisting", "0");
        check("ninlinemissingnew", "0");
        check("nfileextranew", "0");
        check("nfilemissingexisting", "0");
        check("linlinemissingused", "false");
        check("lfileextraused", "false");
        check("lexistingused", "true");
        check("caliasafterfailures", "ExistingXml");
        check("nrecnoafterfailures", "2");
        check("nexistingcount", "2");
        check("nexistingrecno", "2");
        check("nexistingid", "2");
        check("cexistingname", "BETA");
        check("cexistingalias", "ExistingXml");

        expect(read_text(existing_path) == original_bytes,
               "rejected XMLTOCURSOR cardinality mismatches should preserve destination bytes");
        expect(std::count_if(state.events.begin(), state.events.end(), [](const auto& event)
        {
            return event.category == "runtime.warning";
        }) >= 4,
               "each rejected XMLTOCURSOR cardinality mismatch should emit a warning event");
        expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event)
        {
            return event.category == "runtime.xmltocursor";
        }),
               "rejected XMLTOCURSOR cardinality mismatches should not emit success events");

        fs::remove_all(temp_root, ignored);
    }

    void test_newobject_getpem_setpem_compobj_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_getpem_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "newobject_getpem_setpem.prg";
        write_text(
            main_path,
            "oa = NEWOBJECT('Scripting.Dictionary')\n"
            "ob = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lsamesame = COMPOBJ(oa, oa)\n"
            "ldiffab = COMPOBJ(oa, ob)\n"
            "lnullleft = COMPOBJ(.NULL., oa)\n"
            "lnullright = COMPOBJ(oa, .NULL.)\n"
            "lbothnull = COMPOBJ(.NULL., .NULL.)\n"
            "ngetprop = GETPEM(oa, 'comparemode')\n"
            "lgetmethod = GETPEM(oa, 'add')\n"
            "xgetmissing = GETPEM(oa, 'nosuchprop')\n"
            "lsetprop = SETPEM(oa, 'comparemode', 1)\n"
            "ngetpropafterset = GETPEM(oa, 'comparemode')\n"
            "lputprop = PUTPEM(oa, 'comparemode', 2)\n"
            "ngetpropafterput = GETPEM(oa, 'comparemode')\n"
            "lsetreadonly = SETPEM(oa, 'count', 99)\n"
            "lsetunknown = SETPEM(oa, 'nosuchprop', 42)\n"
            "lsetmethod = SETPEM(oa, 'add', 'MyAddProc')\n"
            "cgetmethodafterset = GETPEM(oa, 'add')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("NEWOBJECT/GETPEM/SETPEM script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected)
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

        // NEWOBJECT creates valid object refs
        expect(state.globals.count("oa") && state.globals.at("oa").kind == copperfin::runtime::PrgValueKind::string,
               "NEWOBJECT('Scripting.Dictionary') should return a string object ref");
        expect(state.globals.count("ob") && state.globals.at("ob").kind == copperfin::runtime::PrgValueKind::string,
               "NEWOBJECT('Scripting.Dictionary', 'vbscript.dll') should return a string object ref");

        // NEWOBJECT should have emitted ole.newobject events
        const bool has_newobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto& ev)
        {
            return ev.category == "ole.newobject";
        });
        expect(has_newobject_event, "NEWOBJECT() should emit ole.newobject event");

        // COMPOBJ
        check("lsamesame", "true");
        check("ldiffab", "false");
        check("lnullleft", "false");
        check("lnullright", "false");
        check("lbothnull", "false");

        // GETPEM — property, method presence, missing member
        check("ngetprop", "0");        // comparemode default = 0
        check("lgetmethod", "true");   // 'add' is a method → .T.
        // missing returns empty (format_value of empty is "")
        {
            const auto it = state.globals.find("xgetmissing");
            expect(it != state.globals.end() && it->second.kind == copperfin::runtime::PrgValueKind::empty,
                   "GETPEM unknown member should return empty (.NULL.)");
        }

        // SETPEM
        check("lsetprop", "true");           // setting comparemode succeeds
        check("ngetpropafterset", "1");      // comparemode now 1
        check("lputprop", "true");           // documented PUTPEM spelling succeeds
        check("ngetpropafterput", "2");      // comparemode now 2
        check("lsetreadonly", "false");      // count is read-only → .F.
        check("lsetunknown", "false");       // unknown property → .F.
        check("lsetmethod", "true");         // setting method ref succeeds
        check("cgetmethodafterset", "MyAddProc");  // method ref retrievable as property

        fs::remove_all(temp_root, ignored);
    }

}

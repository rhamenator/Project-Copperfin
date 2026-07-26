#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_reportlistener_strict_configuration_uses_admitted_bytes()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
            "copperfin_reportlistener_verified_config";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path config_path = temp_root / "_ReportOutputConfig.dbf";
        const auto config_create = copperfin::vfp::create_dbf_table_file(
            config_path.string(),
            {
                {.name = "OBJTYPE", .type = 'I', .length = 4U},
                {.name = "OBJCODE", .type = 'I', .length = 4U},
                {.name = "OBJNAME", .type = 'V', .length = 60U},
                {.name = "OBJVALUE", .type = 'V', .length = 60U},
                {.name = "OBJINFO", .type = 'M', .length = 4U}
            },
            {});
        expect(config_create.ok, "strict ReportListener configuration fixture should be writable");

        const auto read_binary = [](const fs::path &path) {
            std::ifstream input(path, std::ios::binary);
            return std::string(
                std::istreambuf_iterator<char>(input),
                std::istreambuf_iterator<char>());
        };
        const std::string config_bytes = read_binary(config_path);
        const fs::path memo_path = config_path.parent_path() /
            (config_path.stem().string() + ".fpt");
        const std::string memo_bytes = read_binary(memo_path);
        expect(!config_bytes.empty(), "strict ReportListener fixture bytes should be readable");
        expect(!memo_bytes.empty(), "strict ReportListener memo sidecar bytes should be readable");

        const fs::path main_path = temp_root / "reportlistener_verified_config.prg";
        write_text(
            main_path,
            "oListener = CREATEOBJECT('ReportListenerShim')\n"
            "cConfig = oListener.GetConfigTable()\n"
            "lValid = oListener.VerifyConfigTable('OutputConfig')\n"
            "lHadError = oListener.HadError\n"
            "RETURN\n"
            "DEFINE CLASS ReportListenerShim AS ReportListener\n"
            "ENDDEFINE\n");

        const auto run_strict = [&](bool admit_config) {
            copperfin::runtime::RuntimeSessionOptions options =
                make_runtime_session_options(main_path, temp_root);
            options.require_verified_file_byte_overrides = true;
            if (admit_config)
            {
                const fs::path admitted_config = temp_root / "_REPORTOUTPUTCONFIG.DBF";
                const fs::path admitted_memo = temp_root / "_REPORTOUTPUTCONFIG.FPT";
                options.verified_file_byte_overrides.emplace(
                    admitted_config.string(),
                    config_bytes);
                options.verified_file_byte_overrides.emplace(
                    admitted_memo.string(),
                    memo_bytes);
            }
            return copperfin::runtime::PrgRuntimeSession::create(std::move(options)).run(
                copperfin::runtime::DebugResumeAction::continue_run);
        };

        write_text(config_path, "tampered physical configuration table");
        const auto admitted_tampered_state = run_strict(true);
        expect(admitted_tampered_state.completed,
               "strict admitted ReportListener configuration should complete despite physical tampering");
        const auto admitted_tampered_valid = admitted_tampered_state.globals.find("lvalid");
        const auto admitted_tampered_error = admitted_tampered_state.globals.find("lhaderror");
        expect(admitted_tampered_valid != admitted_tampered_state.globals.end() &&
                   copperfin::runtime::format_value(admitted_tampered_valid->second) == "true",
               "strict admitted ReportListener configuration should validate admitted DBF bytes");
        expect(admitted_tampered_error != admitted_tampered_state.globals.end() &&
                   copperfin::runtime::format_value(admitted_tampered_error->second) == "false",
               "strict admitted ReportListener configuration should clear HadError");

        fs::remove(config_path, ignored);
        fs::remove(memo_path, ignored);
        const auto admitted_absent_state = run_strict(true);
        expect(admitted_absent_state.completed,
               "strict admitted ReportListener configuration should complete without physical files");
        const auto admitted_absent_valid = admitted_absent_state.globals.find("lvalid");
        expect(admitted_absent_valid != admitted_absent_state.globals.end() &&
                   copperfin::runtime::format_value(admitted_absent_valid->second) == "true",
               "strict admitted ReportListener configuration should resolve absent physical DBF bytes");

        const auto unadmitted_create = copperfin::vfp::create_dbf_table_file(
            config_path.string(),
            {
                {.name = "OBJTYPE", .type = 'I', .length = 4U},
                {.name = "OBJCODE", .type = 'I', .length = 4U},
                {.name = "OBJNAME", .type = 'V', .length = 60U},
                {.name = "OBJVALUE", .type = 'V', .length = 60U},
                {.name = "OBJINFO", .type = 'M', .length = 4U}
            },
            {});
        expect(unadmitted_create.ok, "unadmitted physical configuration fixture should be writable");
        const auto unadmitted_state = run_strict(false);
        expect(unadmitted_state.completed,
               "strict unadmitted ReportListener configuration should complete without consulting disk");
        const auto unadmitted_config = unadmitted_state.globals.find("cconfig");
        const auto unadmitted_valid = unadmitted_state.globals.find("lvalid");
        expect(unadmitted_config != unadmitted_state.globals.end() &&
                   copperfin::runtime::format_value(unadmitted_config->second).empty(),
               "strict GetConfigTable should reject an unadmitted physical table");
        expect(unadmitted_valid != unadmitted_state.globals.end() &&
                   copperfin::runtime::format_value(unadmitted_valid->second) == "false",
               "strict VerifyConfigTable should reject an unadmitted physical table");

        fs::remove_all(temp_root, ignored);
    }
}

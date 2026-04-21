#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include "copperfin/vfp/dbf_table.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace
{

    using namespace copperfin::test_support;

    void test_type_and_null_expression_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_type_null";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "type_null.prg";
        write_text(
            main_path,
            "x = 5\n"
            "t_num = VARTYPE(x)\n"
            "t_str = VARTYPE('hello')\n"
            "t_bool = VARTYPE(.T.)\n"
            "em = EMPTY('')\n"
            "em2 = EMPTY(0)\n"
            "not_em = EMPTY('hi')\n"
            "nvl_result = NVL('', 'fallback')\n"
            "nvl_ok = NVL('value', 'fallback')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({.startup_path = main_path.string(),
                                                                                                       .working_directory = temp_root.string(),
                                                                                                       .stop_on_entry = false});

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "type/null function script should complete");

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

        check("t_num", "N");
        check("t_str", "C");
        check("t_bool", "L");
        check("em", "true");
        check("em2", "true");
        check("not_em", "false");
        check("nvl_result", "");
        check("nvl_ok", "value");

        fs::remove_all(temp_root, ignored);
    }

    void test_count_to_array_expression()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_count_to_array";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / "people.dbf";
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "NAME", .type = 'C', .length = 10U},
            {.name = "AGE", .type = 'N', .length = 3U},
        };
        const std::vector<std::vector<std::string>> records{
            {"Alice", "42"},
            {"Bob", "35"},
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
        expect(create_result.ok, "COUNT TO ARRAY test DBF fixture should be created");

        const fs::path main_path = temp_root / "count_to_array.prg";
        write_text(
            main_path,
            "USE '" + table_path.string() + "'\n"
            "COUNT TO ARRAY aCount\n"
            "nArrayLen = ALEN(aCount)\n"
            "nCount = aCount[1]\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "COUNT TO ARRAY script should complete");

        const auto array_len = state.globals.find("narraylen");
        const auto count = state.globals.find("ncount");

        expect(array_len != state.globals.end(), "ALEN(aCount) should expose array element count");
        expect(count != state.globals.end(), "aCount[1] should expose the count result");

        if (array_len != state.globals.end()) {
            expect(copperfin::runtime::format_value(array_len->second) == "1", "COUNT TO ARRAY should create a one-element array");
        }
        if (count != state.globals.end()) {
            expect(copperfin::runtime::format_value(count->second) == "2", "COUNT TO ARRAY should store the record count");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_sum_and_average_to_array_expression()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sum_avg_to_array";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / "people.dbf";
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "NAME", .type = 'C', .length = 10U},
            {.name = "AGE", .type = 'N', .length = 5U},
        };
        const std::vector<std::vector<std::string>> records{
            {"Alice", "42"},
            {"Bob", "35"},
            {"Cara", "50"},
            {"Dan", "20"},
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
        expect(create_result.ok, "SUM/AVERAGE TO ARRAY test DBF fixture should be created");

        const fs::path main_path = temp_root / "sum_average_to_array.prg";
        write_text(
            main_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "SUM AGE TO ARRAY aSum\n"
            "AVERAGE AGE TO ARRAY aAvg\n"
            "nSumLen = ALEN(aSum)\n"
            "nAvgLen = ALEN(aAvg)\n"
            "nSum = aSum[1]\n"
            "nAvg = aAvg[1]\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "SUM/AVERAGE TO ARRAY script should complete");

        const auto sum_len = state.globals.find("nsumlen");
        const auto avg_len = state.globals.find("navglen");
        const auto sum_value = state.globals.find("nsum");
        const auto avg_value = state.globals.find("navg");

        expect(sum_len != state.globals.end(), "ALEN(aSum) should expose array element count");
        expect(avg_len != state.globals.end(), "ALEN(aAvg) should expose array element count");
        expect(sum_value != state.globals.end(), "aSum[1] should expose the sum result");
        expect(avg_value != state.globals.end(), "aAvg[1] should expose the average result");

        if (sum_len != state.globals.end())
        {
            expect(copperfin::runtime::format_value(sum_len->second) == "1", "SUM TO ARRAY should create a one-element array");
        }
        if (avg_len != state.globals.end())
        {
            expect(copperfin::runtime::format_value(avg_len->second) == "1", "AVERAGE TO ARRAY should create a one-element array");
        }
        if (sum_value != state.globals.end())
        {
            expect(copperfin::runtime::format_value(sum_value->second) == "147", "SUM TO ARRAY should store the aggregate result");
        }
        if (avg_value != state.globals.end())
        {
            expect(copperfin::runtime::format_value(avg_value->second) == "36.75", "AVERAGE TO ARRAY should store the aggregate result");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_aggregate_to_array_scope_and_in_targeting()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aggregate_to_array_scope";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / "people.dbf";
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "NAME", .type = 'C', .length = 10U},
            {.name = "AGE", .type = 'N', .length = 5U},
        };
        const std::vector<std::vector<std::string>> records{
            {"Alice", "42"},
            {"Bob", "35"},
            {"Cara", "50"},
            {"Dan", "20"},
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
        expect(create_result.ok, "aggregate TO ARRAY scope fixture should be created");

        const fs::path main_path = temp_root / "aggregate_to_array_scope.prg";
        write_text(
            main_path,
            "USE '" + table_path.string() + "' ALIAS p1\n"
            "COUNT TO ARRAY aCountAll\n"
            "COUNT FOR AGE >= 40 TO ARRAY aCountFor\n"
            "GO 2\n"
            "COUNT REST TO ARRAY aCountRest\n"
            "COUNT NEXT 2 TO ARRAY aCountNext\n"
            "COUNT RECORD 3 TO ARRAY aCountRecord\n"
            "SUM AGE FOR AGE >= 40 TO ARRAY aSumFor\n"
            "AVERAGE AGE FOR AGE >= 40 TO ARRAY aAvgFor\n"
            "USE '" + table_path.string() + "' AGAIN ALIAS p2\n"
            "SUM AGE TO ARRAY aSumIn IN p2\n"
            "COUNT TO ARRAY aCountIn IN p2\n"
            "nCountAll = aCountAll[1]\n"
            "nCountFor = aCountFor[1]\n"
            "nCountRest = aCountRest[1]\n"
            "nCountNext = aCountNext[1]\n"
            "nCountRecord = aCountRecord[1]\n"
            "nSumFor = aSumFor[1]\n"
            "nAvgFor = aAvgFor[1]\n"
            "nSumIn = aSumIn[1]\n"
            "nCountIn = aCountIn[1]\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "aggregate TO ARRAY scope/IN script should complete: " + state.message);

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

        check("ncountall", "4");
        check("ncountfor", "2");
        check("ncountrest", "3");
        check("ncountnext", "2");
        check("ncountrecord", "1");
        check("nsumfor", "92");
        check("navgfor", "46");
        check("nsumin", "147");
        check("ncountin", "4");

        fs::remove_all(temp_root, ignored);
    }

    void test_aggregate_to_array_diagnostics()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aggregate_to_array_diagnostics";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / "people.dbf";
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "NAME", .type = 'C', .length = 10U},
            {.name = "AGE", .type = 'N', .length = 3U},
        };
        const std::vector<std::vector<std::string>> records{{"Alice", "42"}};
        const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
        expect(create_result.ok, "aggregate TO ARRAY diagnostic fixture should be created");

        const fs::path missing_target_path = temp_root / "aggregate_to_array_missing_target.prg";
        write_text(
            missing_target_path,
            "USE '" + table_path.string() + "'\n"
            "SUM AGE TO ARRAY\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession missing_target_session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = missing_target_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });
        const auto missing_target_state = missing_target_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(!missing_target_state.completed, "SUM TO ARRAY without a target should fail");
        expect(lowercase_copy(missing_target_state.message).find("to array requires a target array name") != std::string::npos,
               "SUM TO ARRAY missing-target error should explain the required array target");

        const fs::path multiple_targets_path = temp_root / "aggregate_to_array_multiple_targets.prg";
        write_text(
            multiple_targets_path,
            "USE '" + table_path.string() + "'\n"
            "AVERAGE AGE TO ARRAY aOne, aTwo\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession multiple_targets_session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = multiple_targets_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });
        const auto multiple_targets_state = multiple_targets_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(!multiple_targets_state.completed, "AVERAGE TO ARRAY with multiple targets should fail");
        expect(lowercase_copy(multiple_targets_state.message).find("to array accepts exactly one array target") != std::string::npos,
               "AVERAGE TO ARRAY multi-target error should explain that only one array is allowed");

        fs::remove_all(temp_root, ignored);
    }

    void test_expression_runtime_surface_extensions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_runtime_surface_extensions";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_surface_extensions.prg";
        write_text(
            main_path,
            "x = 5\n"
            "DIMENSION aValues[2]\n"
            "aValues[1] = 'A'\n"
            "aValues[2] = 'B'\n"
            "nEval = EVALUATE('x + 7')\n"
            "cTransformDefault = TRANSFORM(x)\n"
            "cTransformPicture = TRANSFORM(3.14159, '999.00')\n"
            "cTransformUpper = TRANSFORM('legacy', '@!')\n"
            "cTypeArray = TYPE('aValues')\n"
            "cTypeUnknown = TYPE('notDefinedAnywhere')\n"
            "SET PATH TO '/tmp/copperfin'\n"
            "cPathValue = SET('PATH')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "runtime surface extension script should complete");

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

        check("neval", "12");
        check("ctransformdefault", "5");
        check("ctransformpicture", "3.14");
        check("ctransformupper", "LEGACY");
        check("ctypearray", "A");
        check("ctypeunknown", "U");
        check("cpathvalue", "TO '/tmp/copperfin'");

        fs::remove_all(temp_root, ignored);
    }

} // namespace

int main()
{
    test_type_and_null_expression_functions();
    test_count_to_array_expression();
    test_sum_and_average_to_array_expression();
    test_aggregate_to_array_scope_and_in_targeting();
    test_aggregate_to_array_diagnostics();
    test_expression_runtime_surface_extensions();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

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

    void test_date_time_expression_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_date_time";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "date_time.prg";
        write_text(
            main_path,
            "d = CTOD('04/18/2026')\n"
            "date_ctor = DATE(2026, 4, 18)\n"
            "date_ctor_invalid = DATE(2026, 2, 31)\n"
            "datetime_ctor = DATETIME(2026, 4, 18, 13, 45, 56)\n"
            "datetime_ctor_partial = DATETIME(2026, 4, 18)\n"
            "datetime_ctor_invalid = DATETIME(2026, 4, 18, 24, 0, 0)\n"
            "dow_default = DOW(d)\n"
            "dow_monday = DOW(d, 2)\n"
            "day_name = CDOW(d)\n"
            "month_name = CMONTH(d)\n"
            "next_month = GOMONTH(CTOD('01/31/2026'), 1)\n"
            "prev_month = GOMONTH(CTOD('03/31/2026'), -1)\n"
            "mdy_value = MDY(4, 18, 2026)\n"
            "dtot_value = DTOT(CTOD('04/18/2026'))\n"
            "ttod_value = TTOD('04/18/2026 13:45:56')\n"
            "ctot_value = CTOT('04/18/2026 13:45:56')\n"
            "hour_value = HOUR('04/18/2026 13:45:56')\n"
            "minute_value = MINUTE('04/18/2026 13:45:56')\n"
            "sec_value = SEC('04/18/2026 13:45:56')\n"
            "dtos_value = DTOS('04/18/2026')\n"
            "stod_value = STOD('20260418')\n"
            "stod_invalid = STOD('2026-04-18')\n"
            "ctod_value = CTOD('04/18/2026')\n"
            "dtoc_value = DTOC('20260418')\n"
            "dtoc_compact_flag = DTOC('04/18/2026', 1)\n"
            "ttoc_value = TTOC('04/18/2026 13:45:56')\n"
            "ttoc_compact_flag = TTOC('04/18/2026 13:45:56', 1)\n"
            "ttoc_date_compact_flag = TTOC('04/18/2026', 1)\n"
            "ttos_value = TTOS('04/18/2026 13:45:56')\n"
            "ttos_date = TTOS('04/18/2026')\n"
            "year_value = YEAR('04/18/2026')\n"
            "month_value = MONTH('04/18/2026')\n"
            "day_value = DAY('04/18/2026')\n"
            "year_compact = YEAR('20260418')\n"
            "month_compact = MONTH('20260418')\n"
            "day_compact = DAY('20260418')\n"
            "quarter_value = QUARTER('04/18/2026')\n"
            "quarter_compact = QUARTER('20261231')\n"
            "week_value = WEEK('04/18/2026')\n"
            "week_monday = WEEK('01/05/2026', 2)\n"
            "week_mode1_jan1 = WEEK('01/01/2026', 1, 1)\n"
            "week_mode2_jan1 = WEEK('01/01/2026', 1, 2)\n"
            "week_mode2_jan3 = WEEK('01/03/2026', 1, 2)\n"
            "week_mode2_jan4 = WEEK('01/04/2026', 1, 2)\n"
            "week_mode3_jan1 = WEEK('01/01/2026', 1, 3)\n"
            "week_mode3_jan4 = WEEK('01/04/2026', 1, 3)\n"
            "week_mode3_dec_rollover = WEEK('12/29/2024', 1, 3)\n"
            "eomonth_value = EOMONTH('04/18/2026')\n"
            "eomonth_plus = EOMONTH('01/10/2026', 1)\n"
            "eomonth_minus = EOMONTH('03/10/2026', -1)\n"
            "dtos_compact = DTOS('20260418')\n"
            "ctod_invalid = CTOD('not-a-date')\n"
            "ctod_trailing_invalid = CTOD('04/18/2026junk')\n"
            "ctod_compact_trailing_invalid = CTOD('20260418junk')\n"
            "ttoc_date = TTOC('20260418')\n"
            "ttoj_value = TTOJ('04/18/2026')\n"
            "ttoj_datetime = TTOJ('04/18/2026 13:45:56')\n"
            "ttoj_invalid = TTOJ('not-a-date')\n"
            "jtot_value = JTOT(ttoj_value)\n"
            "jtot_invalid = JTOT(0)\n"
            "dtoj_value = DTOJ('04/18/2026')\n"
            "dtoj_invalid = DTOJ('not-a-date')\n"
            "dtoj_trailing_invalid = DTOJ('04/18/2026junk')\n"
            "jtod_value = JTOD(dtoj_value)\n"
            "jtod_invalid = JTOD(0)\n"
            "dmy_value = DMY(18, 4, 2026)\n"
            "dmy_invalid = DMY(31, 2, 2026)\n"
            "isleap_2024 = ISLEAPYEAR(2024)\n"
            "isleap_2026 = ISLEAPYEAR(2026)\n"
            "seconds_now = SECONDS()\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({.startup_path = main_path.string(),
                                                                                                       .working_directory = temp_root.string(),
                                                                                                       .stop_on_entry = false});

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "date/time function script should complete");

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

        check("date_ctor", "04/18/2026");
        check("date_ctor_invalid", "");
        check("datetime_ctor", "04/18/2026 13:45:56");
        check("datetime_ctor_partial", "04/18/2026 00:00:00");
        check("datetime_ctor_invalid", "");
        check("dow_default", "7");
        check("dow_monday", "6");
        check("day_name", "Saturday");
        check("month_name", "April");
        check("next_month", "02/28/2026");
        check("prev_month", "02/28/2026");
        check("mdy_value", "04/18/2026");
        check("dtot_value", "04/18/2026 00:00:00");
        check("ttod_value", "04/18/2026");
        check("ctot_value", "04/18/2026 13:45:56");
        check("hour_value", "13");
        check("minute_value", "45");
        check("sec_value", "56");
        check("dtos_value", "20260418");
        check("stod_value", "04/18/2026");
        check("stod_invalid", "");
        check("ctod_value", "04/18/2026");
        check("dtoc_value", "04/18/2026");
        check("dtoc_compact_flag", "20260418");
        check("ttoc_value", "04/18/2026 13:45:56");
        check("ttoc_compact_flag", "20260418134556");
        check("ttoc_date_compact_flag", "20260418000000");
        check("ttos_value", "20260418134556");
        check("ttos_date", "20260418000000");
        check("year_value", "2026");
        check("month_value", "4");
        check("day_value", "18");
        check("year_compact", "2026");
        check("month_compact", "4");
        check("day_compact", "18");
        check("quarter_value", "2");
        check("quarter_compact", "4");
        check("week_value", "16");
        check("week_monday", "2");
        check("week_mode1_jan1", "1");
        check("week_mode2_jan1", "52");
        check("week_mode2_jan3", "52");
        check("week_mode2_jan4", "1");
        check("week_mode3_jan1", "53");
        check("week_mode3_jan4", "1");
        check("week_mode3_dec_rollover", "1");
        check("eomonth_value", "04/30/2026");
        check("eomonth_plus", "02/28/2026");
        check("eomonth_minus", "02/28/2026");
        check("dtos_compact", "20260418");
        check("ctod_invalid", "");
        check("ctod_trailing_invalid", "");
        check("ctod_compact_trailing_invalid", "");
        check("ttoc_date", "04/18/2026 00:00:00");
        check("ttoj_value", "2460447");
        check("ttoj_datetime", "2460447");
        check("ttoj_invalid", "0");
        check("jtot_value", "04/18/2026");
        check("jtot_invalid", "");
        check("dtoj_value", "2460447");
        check("dtoj_invalid", "0");
        check("dtoj_trailing_invalid", "0");
        check("jtod_value", "04/18/2026");
        check("jtod_invalid", "");
        check("dmy_value", "04/18/2026");
        check("dmy_invalid", "");
        check("isleap_2024", "true");
        check("isleap_2026", "false");

        const auto seconds_it = state.globals.find("seconds_now");
        if (seconds_it == state.globals.end())
        {
            expect(false, "seconds_now variable not found");
        }
        else
        {
            double seconds = -1.0;
            try
            {
                seconds = std::stod(copperfin::runtime::format_value(seconds_it->second));
            }
            catch (...)
            {
                seconds = -1.0;
            }
            expect(seconds >= 0.0 && seconds <= 86399.0,
                   "SECONDS() should return a second-of-day value between 0 and 86399");
        }

        fs::remove_all(temp_root, ignored);
    }


} // namespace

int main()
{
    test_date_time_expression_functions();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

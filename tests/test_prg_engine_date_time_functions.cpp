// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
            "date_plus_days = DATE(2024, 2, 28) + 1\n"
            "days_plus_date = 1 + DATE(2024, 2, 28)\n"
            "date_plus_fractional_days = DATE(2024, 3, 2) + (-1.5)\n"
            "date_minus_days = DATE(2024, 3, 1) - 1\n"
            "date_day_difference = DATE(2024, 3, 2) - DATE(2024, 2, 28)\n"
            "date_arithmetic_type = VARTYPE(DATE(2024, 2, 28) + 1)\n"
            "datetime_plus_seconds = DATETIME(2024, 2, 28, 23, 59, 30) + 90\n"
            "seconds_plus_datetime = 90 + DATETIME(2024, 2, 28, 23, 59, 30)\n"
            "datetime_minus_seconds = DATETIME(2024, 3, 1, 0, 0, 30) - 90\n"
            "datetime_second_difference = DATETIME(2024, 3, 1, 0, 0, 30) - DATETIME(2024, 2, 29, 23, 59, 0)\n"
            "datetime_arithmetic_type = VARTYPE(DATETIME(2024, 2, 28, 23, 59, 30) + 90)\n"
            "date_default = SET('DATE')\n"
            "century_default = SET('CENTURY')\n"
            "epoch_default = SET('EPOCH')\n"
            "mark_default = SET('MARK')\n"
            "hours_default = SET('HOURS')\n"
            "seconds_set_default = SET('SECONDS')\n"
            "fdow_default = SET('FDOW')\n"
            "fweek_default = SET('FWEEK')\n"
            "dow_default = DOW(d)\n"
            "dow_monday = DOW(d, 2)\n"
            "day_name = CDOW(d)\n"
            "month_name = CMONTH(d)\n"
            "next_month = GOMONTH(CTOD('01/31/2026'), 1)\n"
            "prev_month = GOMONTH(CTOD('03/31/2026'), -1)\n"
            "mdy_value = MDY(4, 18, 2026)\n"
            "dtot_value = DTOT(CTOD('04/18/2026'))\n"
            "ttod_value = TTOD('04/18/2026 13:45:56')\n"
            "ttod_compact_datetime = TTOD('20260418134556')\n"
            "ctot_value = CTOT('04/18/2026 13:45:56')\n"
            "ctot_compact_datetime = CTOT('20260418134556')\n"
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
            "week_fdow_before = WEEK('01/04/2026')\n"
            "SET FDOW TO 2\n"
            "fdow_after = SET('FDOW')\n"
            "week_fdow_after = WEEK('01/04/2026')\n"
            "week_fdow_explicit = WEEK('01/04/2026', 1)\n"
            "SET FWEEK TO 2\n"
            "fweek_after = SET('FWEEK')\n"
            "week_fweek_after = WEEK('01/01/2026')\n"
            "week_fweek_explicit = WEEK('01/01/2026', 1, 1)\n"
            "SET FDOW TO 1\n"
            "SET FWEEK TO 3\n"
            "fweek_after_three = SET('FWEEK')\n"
            "week_fweek_three = WEEK('01/01/2026')\n"
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
            "SET DATE TO DMY\n"
            "date_set_dmy = SET('DATE')\n"
            "ctod_dmy = CTOD('18/04/2026')\n"
            "date_arithmetic_dmy = CTOD('18/04/2026') + 14\n"
            "dtoc_dmy = DTOC('18/04/2026')\n"
            "ttoc_dmy = TTOC('18/04/2026 13:45:56')\n"
            "year_dmy = YEAR(ctod_dmy)\n"
            "month_dmy = MONTH(ctod_dmy)\n"
            "day_dmy = DAY(ctod_dmy)\n"
            "quarter_dmy = QUARTER(ctod_dmy)\n"
            "gomonth_dmy = GOMONTH(ctod_dmy, 1)\n"
            "eomonth_dmy = EOMONTH(ctod_dmy)\n"
            "dtos_dmy = DTOS(ctod_dmy)\n"
            "dtoj_dmy = DTOJ(ctod_dmy)\n"
            "ctot_dmy = CTOT('18/04/2026 13:45:56')\n"
            "hour_dmy = HOUR(ctot_dmy)\n"
            "minute_dmy = MINUTE(ctot_dmy)\n"
            "sec_dmy = SEC(ctot_dmy)\n"
            "ttoj_dmy = TTOJ(ctot_dmy)\n"
            "ctod_dmy_ambiguous = DTOC(CTOD('01/02/2024'), 1)\n"
            "dtoc_dmy_ambiguous = DTOC('01/02/2024', 1)\n"
            "ttoc_dmy_ambiguous = TTOC('01/02/2024 13:45:56', 1)\n"
            "ttos_dmy_ambiguous = TTOS('01/02/2024 13:45:56')\n"
            "ctot_dmy_ambiguous = TTOC(CTOT('01/02/2024 13:45:56'), 1)\n"
            "dtot_dmy_ambiguous = TTOC(DTOT('01/02/2024'), 1)\n"
            "ttod_dmy_ambiguous = DTOC(TTOD('01/02/2024 13:45:56'), 1)\n"
            "ctod_epoch_default_49 = DTOC(CTOD('01/02/49'), 1)\n"
            "ctod_epoch_default_50 = DTOC(CTOD('01/02/50'), 1)\n"
            "SET EPOCH TO 1975\n"
            "epoch_1975 = SET('EPOCH')\n"
            "ctod_epoch_1975_74 = DTOC(CTOD('01/02/74'), 1)\n"
            "ctod_epoch_1975_75 = DTOC(CTOD('01/02/75'), 1)\n"
            "ctot_epoch_1975_74 = TTOC(CTOT('01/02/74 13:45:56'), 1)\n"
            "dtoc_epoch_1975_74 = DTOC('01/02/74', 1)\n"
            "ttoc_epoch_1975_74 = TTOC('01/02/74 13:45:56', 1)\n"
            "ttos_epoch_1975_74 = TTOS('01/02/74 13:45:56')\n"
            "dtot_epoch_1975_74 = TTOC(DTOT('01/02/74'), 1)\n"
            "ttod_epoch_1975_74 = DTOC(TTOD('01/02/74 13:45:56'), 1)\n"
            "SET CENTURY OFF\n"
            "century_off = SET('CENTURY')\n"
            "dtoc_century_off = DTOC('18/04/2026')\n"
            "SET DATE TO YMD\n"
            "date_set_ymd = SET('DATE')\n"
            "dtoc_ymd_century_off = DTOC('2026/04/18')\n"
            "ctot_ymd_century_off = CTOT('2026/04/18 13:45:56')\n"
            "SET CENTURY ON\n"
            "century_on = SET('CENTURY')\n"
            "dtoc_ymd_century_on = DTOC('2026/04/18')\n"
            "SET MARK TO '-'\n"
            "mark_hyphen = SET('MARK')\n"
            "dtoc_mark_hyphen = DTOC('2026-04-18')\n"
            "ctot_mark_hyphen = CTOT('2026-04-18 13:45:56')\n"
            "SET DATE TO DMY\n"
            "SET MARK TO '.'\n"
            "mark_dot = SET('MARK')\n"
            "dtoc_mark_dot_dmy = DTOC('18.04.2026')\n"
            "ttoc_mark_dot_dmy = TTOC('18.04.2026 13:45:56')\n"
            "ctod_mark_dot_dmy = CTOD('18.04.2026')\n"
            "year_mark_dot_dmy = YEAR(ctod_mark_dot_dmy)\n"
            "gomonth_mark_dot_dmy = GOMONTH(ctod_mark_dot_dmy, 1)\n"
            "SET DATE TO MDY\n"
            "SET MARK TO '/'\n"
            "SET HOURS TO 12\n"
            "hours_12 = SET('HOURS')\n"
            "ttoc_hours_12 = TTOC('04/18/2026 13:45:56')\n"
            "datetime_arithmetic_hours_12 = DATETIME(2026, 4, 18, 13, 45, 56) + 60\n"
            "SET SECONDS OFF\n"
            "seconds_off = SET('SECONDS')\n"
            "ttoc_hours_12_seconds_off = TTOC('04/18/2026 13:45:56')\n"
            "SET HOURS TO 24\n"
            "hours_24 = SET('HOURS')\n"
            "ttoc_hours_24_seconds_off = TTOC('04/18/2026 13:45:56')\n"
            "SET SECONDS ON\n"
            "seconds_on = SET('SECONDS')\n"
            "ttoc_hours_24_seconds_on = TTOC('04/18/2026 13:45:56')\n"
            "SET DATASESSION TO 2\n"
            "date_session2 = SET('DATE')\n"
            "century_session2 = SET('CENTURY')\n"
            "epoch_session2 = SET('EPOCH')\n"
            "mark_session2 = SET('MARK')\n"
            "hours_session2 = SET('HOURS')\n"
            "seconds_session2 = SET('SECONDS')\n"
            "fdow_session2 = SET('FDOW')\n"
            "fweek_session2 = SET('FWEEK')\n"
            "SET DATASESSION TO 1\n"
            "date_restored = SET('DATE')\n"
            "century_restored = SET('CENTURY')\n"
            "epoch_restored = SET('EPOCH')\n"
            "mark_restored = SET('MARK')\n"
            "hours_restored = SET('HOURS')\n"
            "seconds_restored = SET('SECONDS')\n"
            "fdow_restored = SET('FDOW')\n"
            "fweek_restored = SET('FWEEK')\n"
            "seconds_now = SECONDS()\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

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
        check("date_plus_days", "02/29/2024");
        check("days_plus_date", "02/29/2024");
        check("date_plus_fractional_days", "02/29/2024");
        check("date_minus_days", "02/29/2024");
        check("date_day_difference", "3");
        check("date_arithmetic_type", "D");
        check("datetime_plus_seconds", "02/29/2024 00:01:00");
        check("seconds_plus_datetime", "02/29/2024 00:01:00");
        check("datetime_minus_seconds", "02/29/2024 23:59:00");
        check("datetime_second_difference", "90");
        check("datetime_arithmetic_type", "T");
        check("date_default", "MDY");
        check("century_default", "ON");
        check("epoch_default", "1950");
        check("mark_default", "/");
        check("hours_default", "24");
        check("seconds_set_default", "ON");
        check("fdow_default", "1");
        check("fweek_default", "1");
        check("dow_default", "7");
        check("dow_monday", "6");
        check("day_name", "Saturday");
        check("month_name", "April");
        check("next_month", "02/28/2026");
        check("prev_month", "02/28/2026");
        check("mdy_value", "04/18/2026");
        check("dtot_value", "04/18/2026 00:00:00");
        check("ttod_value", "04/18/2026");
        check("ttod_compact_datetime", "04/18/2026");
        check("ctot_value", "04/18/2026 13:45:56");
        check("ctot_compact_datetime", "04/18/2026 13:45:56");
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
        check("week_fdow_before", "2");
        check("fdow_after", "2");
        check("week_fdow_after", "1");
        check("week_fdow_explicit", "2");
        check("fweek_after", "2");
        check("week_fweek_after", "52");
        check("week_fweek_explicit", "1");
        check("fweek_after_three", "3");
        check("week_fweek_three", "53");
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
        check("date_set_dmy", "DMY");
        check("ctod_dmy", "18/04/2026");
        check("date_arithmetic_dmy", "02/05/2026");
        check("dtoc_dmy", "18/04/2026");
        check("ttoc_dmy", "18/04/2026 13:45:56");
        check("year_dmy", "2026");
        check("month_dmy", "4");
        check("day_dmy", "18");
        check("quarter_dmy", "2");
        check("gomonth_dmy", "18/05/2026");
        check("eomonth_dmy", "30/04/2026");
        check("dtos_dmy", "20260418");
        check("dtoj_dmy", "2460447");
        check("hour_dmy", "13");
        check("minute_dmy", "45");
        check("sec_dmy", "56");
        check("ttoj_dmy", "2460447");
        check("ctod_dmy_ambiguous", "20240201");
        check("dtoc_dmy_ambiguous", "20240201");
        check("ttoc_dmy_ambiguous", "20240201134556");
        check("ttos_dmy_ambiguous", "20240201134556");
        check("ctot_dmy_ambiguous", "20240201134556");
        check("dtot_dmy_ambiguous", "20240201000000");
        check("ttod_dmy_ambiguous", "20240201");
        check("ctod_epoch_default_49", "20490201");
        check("ctod_epoch_default_50", "19500201");
        check("epoch_1975", "1975");
        check("ctod_epoch_1975_74", "20740201");
        check("ctod_epoch_1975_75", "19750201");
        check("ctot_epoch_1975_74", "20740201134556");
        check("dtoc_epoch_1975_74", "20740201");
        check("ttoc_epoch_1975_74", "20740201134556");
        check("ttos_epoch_1975_74", "20740201134556");
        check("dtot_epoch_1975_74", "20740201000000");
        check("ttod_epoch_1975_74", "20740201");
        check("century_off", "OFF");
        check("dtoc_century_off", "18/04/26");
        check("date_set_ymd", "YMD");
        check("dtoc_ymd_century_off", "26/04/18");
        check("ctot_ymd_century_off", "26/04/18 13:45:56");
        check("century_on", "ON");
        check("dtoc_ymd_century_on", "2026/04/18");
        check("mark_hyphen", "-");
        check("dtoc_mark_hyphen", "2026-04-18");
        check("ctot_mark_hyphen", "2026-04-18 13:45:56");
        check("mark_dot", ".");
        check("dtoc_mark_dot_dmy", "18.04.2026");
        check("ttoc_mark_dot_dmy", "18.04.2026 13:45:56");
        check("year_mark_dot_dmy", "2026");
        check("gomonth_mark_dot_dmy", "18.05.2026");
        check("hours_12", "12");
        check("ttoc_hours_12", "04/18/2026 01:45:56 PM");
        check("datetime_arithmetic_hours_12", "04/18/2026 01:46:56 PM");
        check("seconds_off", "OFF");
        check("ttoc_hours_12_seconds_off", "04/18/2026 01:45 PM");
        check("hours_24", "24");
        check("ttoc_hours_24_seconds_off", "04/18/2026 13:45");
        check("seconds_on", "ON");
        check("ttoc_hours_24_seconds_on", "04/18/2026 13:45:56");
        check("date_session2", "MDY");
        check("century_session2", "ON");
        check("epoch_session2", "1950");
        check("mark_session2", "/");
        check("hours_session2", "24");
        check("seconds_session2", "ON");
        check("fdow_session2", "1");
        check("fweek_session2", "1");
        check("date_restored", "MDY");
        check("century_restored", "ON");
        check("epoch_restored", "1975");
        check("mark_restored", "/");
        check("hours_restored", "24");
        check("seconds_restored", "ON");
        check("fdow_restored", "1");
        check("fweek_restored", "3");

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

    void test_date_time_arithmetic_rejects_unsupported_operands_without_ending_the_session()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_date_time_arithmetic_fault";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "date_time_arithmetic_fault.prg";
        write_text(
            main_path,
            "invalid_sum = DATE(2024, 2, 28) + DATE(2024, 2, 29)\n"
            "after_fault = 1\n"
            "RETURN\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "date-plus-date should pause with a runtime operand error");
        expect(state.location.line == 1U,
               "date-plus-date should identify the exact faulting statement");
        expect(state.message.find("Operator/operand type mismatch.") != std::string::npos,
               "unsupported date arithmetic should retain localized VFP Error 107 prose");

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "continuing after an unsupported date arithmetic fault should keep the session alive");
        expect(state.globals.contains("after_fault"),
               "statements after a trapped date arithmetic fault should execute");

        fs::remove_all(temp_root, ignored);
    }


} // namespace

int main()
{
    test_date_time_expression_functions();
    test_date_time_arithmetic_rejects_unsupported_operands_without_ending_the_session();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

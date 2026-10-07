// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_test_support.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "test_environment_support.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <limits>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace {
using namespace copperfin::test_support;

// RQ-CF-PRG-SET-CENTURY-WINDOW-001 / NUMERIC-001 / QUERY-001 (#3698).
// Requirements come from retained installed-VFP9 observations and the owner's
// EPOCH retention/failure-atomic policy, not the old implementation.
void run_case(const std::string& label, const std::string& source,
              const std::map<std::string, std::string>& expected,
              int century_events = -1) {
    const auto root = std::filesystem::temp_directory_path() / "copperfin-century-rollover-3698";
    std::filesystem::create_directories(root);
    const auto path = root / "case.prg";
    write_text(path, source);
    auto session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(path, root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, label + " should complete: " + state.message);
    for (const auto& [name, value] : expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), label + ": missing " + name);
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == value,
                   label + ": " + name + " expected " + value + " got " + copperfin::runtime::format_value(found->second));
        }
    }
    if (century_events >= 0) {
        int actual = 0;
        for (const auto& event : state.events) {
            if (event.category == "runtime.set" && event.detail.starts_with("CENTURY")) ++actual;
        }
        expect(actual == century_events, label + ": rejected CENTURY must not emit success");
    }
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
}

void test_commands_and_atomicity() {
    struct Row { const char* command; int error; int epoch; int vfp_error; int vfp_epoch; };
    const Row rows[] = {
        {"SET CENTURY OFF",0,1975,0,1975}, {"SET CENTURY ON",0,1975,0,1975},
        {"SET CENTURY TO 19 ROLLOVER 50",0,1950,0,1950},
        {"SET CENTURY TO 20 ROLLOVER 25",0,2025,0,2025},
        {"SET CENTURY TO 20",0,2075,0,2075},
        {"SET CENTURY TO (18 + 1) ROLLOVER (70 + 5)",0,1975,0,1975},
        {"SET CENTURY TO 1 ROLLOVER 0",0,100,0,100},
        {"SET CENTURY TO 99 ROLLOVER 99",0,9999,0,9999},
        {"SET CENTURY TO 19.9 ROLLOVER 74.9",0,1974,0,1974},
        {"SET CENTURY TO $19.9000 ROLLOVER $74.9000",0,1974,0,1974},
        {"SET CENTURY TO 0",11,1975,11,1975}, {"SET CENTURY TO 100",11,1975,11,1975},
        {"SET CENTURY TO -1",11,1975,11,1975}, {"SET CENTURY TO 0.9",11,1975,11,1975},
        {"SET CENTURY TO 1E300",11,1975,11,1975},
        {"SET CENTURY TO (1E300 * 1E300)",11,1975,11,1975},
        {"SET CENTURY TO 20 ROLLOVER -1",11,1975,11,1975},
        {"SET CENTURY TO 20 ROLLOVER 100",11,1975,11,1975},
        {"SET CENTURY TO 20 ROLLOVER 1E300",11,1975,0,2000},
        {"SET CENTURY TO 19 ROLLOVER (1E300 * 1E300)",11,1975,0,1900},
        {"SET CENTURY TO 19 ROLLOVER 4294967321",11,1975,0,1925},
        {"SET CENTURY TO 19 ROLLOVER -4294967271",11,1975,0,1925},
        {"SET CENTURY TO 4294967315 ROLLOVER 25",11,1975,0,1925},
        {"SET CENTURY TO '19' ROLLOVER 50",9,1975,9,1975},
        {"SET CENTURY TO .T. ROLLOVER 50",9,1975,9,1975},
        {"SET CENTURY TO .NULL. ROLLOVER 50",9,1975,9,1975},
        {"SET CENTURY TO 20 ROLLOVER '50'",9,1975,9,1975},
        {"SET CENTURY TO 20 ROLLOVER .T.",9,1975,9,1975},
        {"SET CENTURY TO 20 ROLLOVER .NULL.",9,1975,9,1975},
        {"SET CENTURY TO 19 ROLLOVER",67,1975,67,1975},
        {"SET CENTURY TO ROLLOVER 50",12,1975,12,1975},
        {"SET CENTURY",10,1975,10,1975}, {"SET CENTURY BAD",10,1975,10,1975},
        // Derived exact lexical safety: never reclassify range failure as Character.
        {"SET CENTURY TO 1E999",11,1975,11,1975},
    };
    for (const auto mode : {"COPPERFIN", "VFP9"}) {
        for (const auto& row : rows) {
            const bool vfp = std::string(mode) == "VFP9";
            const int error = vfp ? row.vfp_error : row.error;
            const int epoch = vfp ? row.vfp_epoch : row.epoch;
            const std::string command = row.command;
            const std::string display = error == 0 && command == "SET CENTURY ON" ? "ON" : "OFF";
            run_case(std::string(mode)+" "+command,
                "SET DATE TO MDY\nSET NUMERICBEHAVIOR TO "+std::string(mode)+
                "\nSET EPOCH TO 1975\nSET CENTURY OFF\nnError=0\nTRY\n"+command+
                "\nCATCH TO oError\nnError=oError.ErrorNo\nENDTRY\n"
                "cEpoch=SET('EPOCH')\nnCentury=SET('CENTURY',1)\nnRollover=SET('CENTURY',2)\n"
                "cDisplay=SET('CENTURY')\ncType=VARTYPE(nCentury)+VARTYPE(nRollover)\n",
                {{"nerror",std::to_string(error)}, {"cepoch",std::to_string(epoch)},
                 {"ncentury",std::to_string(epoch/100)}, {"nrollover",std::to_string(epoch%100)},
                 {"cdisplay",display}, {"ctype","NN"}}, error == 0 ? 2 : 1);
        }
    }
}

void test_queries() {
    struct Row { const char* expression; const char* copperfin; const char* vfp; };
    const Row rows[] = {
        {"SET('CENTURY')","C:OFF","C:OFF"},
        {"SET('CENTURY',1)","N:20","N:20"}, {"SET('CENTURY',2)","N:25","N:25"},
        {"SET('CENTURY',1.9)","N:20","N:20"}, {"SET('CENTURY',2.9)","N:25","N:25"},
        {"SET('CENTURY',0.9)","ERR:11","ERR:11"}, {"SET('CENTURY',4)","ERR:11","ERR:11"},
        {"SET('CENTURY',-1)","ERR:11","ERR:11"},
        {"SET('CENTURY',4294967297)","ERR:11","N:20"},
        {"SET('CENTURY',-4294967295)","ERR:11","N:20"},
        {"SET('CENTURY',1E300)","ERR:11","ERR:11"},
        {"SET('CENTURY',(1E300 * 1E300))","ERR:11","ERR:11"},
        {"SET('CENTURY',$1.9000)","N:20","N:20"},
        {"SET('CENTURY',$2.9000)","N:25","N:25"},
        {"SET('CENTURY','1')","ERR:11","ERR:11"}, {"SET('CENTURY','2')","ERR:11","ERR:11"},
        {"SET('CENTURY','3')","ERR:11","ERR:11"}, {"SET('CENTURY','')","ERR:11","ERR:11"},
        {"SET('CENTURY','arbitrary')","ERR:11","ERR:11"},
        {"SET('CENTURY',.T.)","ERR:11","ERR:11"}, {"SET('CENTURY',.F.)","ERR:11","ERR:11"},
        {"SET('CENTURY',.NULL.)","ERR:11","ERR:11"},
        {"SET('CENTURY',{},1)","ERR:1230","ERR:1230"},
        {"SET('CENTURY',1,2)","ERR:1230","ERR:1230"},
    };
    for (const auto mode : {"COPPERFIN", "VFP9"}) {
        for (const auto& row : rows) {
            const std::string expression = row.expression;
            run_case(std::string(mode)+" "+expression,
                "SET NUMERICBEHAVIOR TO "+std::string(mode)+"\nSET CENTURY OFF\n"
                "SET CENTURY TO 20 ROLLOVER 25\nTRY\ncResult=VARTYPE("+expression+")+':'+TRANSFORM("+expression+")\n"
                "CATCH TO oError\ncResult='ERR:'+ALLTRIM(STR(oError.ErrorNo,4,0))\nENDTRY\n",
                {{"cresult",std::string(mode)=="VFP9" ? row.vfp : row.copperfin}});
        }
    }
}

void test_direct_boundaries() {
    using namespace copperfin::runtime;
    // Derived exact extended-integer/NaN and clock-boundary policy under
    // #3698/#6776 and HZ-runtime-crash-01, not native datatype observations.
    struct Row { PrgValue value; std::array<int,3> copperfin; std::array<int,3> vfp; };
    const std::array<int,3> rejected{-1,-1,-1};
    PrgValue typed_null = make_number_value(19);
    typed_null.is_null = true;
    const Row rows[] = {
        {make_number_value(19.9),{19,19,-1},{19,19,-1}},
        {make_number_value(0.9),{-1,0,-1},{-1,0,-1}},
        {make_number_value(-0.9),{-1,0,-1},{-1,0,-1}},
        {make_number_value(2.9),{2,2,2},{2,2,2}},
        {make_number_value(99.9),{99,99,-1},{99,99,-1}},
        {make_number_value(100),rejected,rejected},
        {make_number_value(std::nextafter(1.0,0.0)),{-1,0,-1},{-1,0,-1}},
        {make_number_value(std::nextafter(100.0,0.0)),{99,99,-1},{99,99,-1}},
        {make_number_value(1E300),rejected,{-1,0,-1}},
        {make_number_value(-1E300),rejected,{-1,0,-1}},
        {make_number_value(std::numeric_limits<double>::infinity()),rejected,{-1,0,-1}},
        {make_number_value(-std::numeric_limits<double>::infinity()),rejected,{-1,0,-1}},
        {make_number_value(std::numeric_limits<double>::quiet_NaN()),rejected,{-1,0,-1}},
        {make_number_value(4294967315.0),rejected,{19,19,-1}},
        {make_number_value(-4294967277.0),rejected,{19,19,-1}},
        {make_int64_value(9007199254740993LL),rejected,{1,1,1}},
        {make_uint64_value(9223372036854775811ULL),rejected,{3,3,3}},
        {make_int64_value(std::numeric_limits<std::int64_t>::min()),rejected,{-1,0,-1}},
        {make_int64_value(std::numeric_limits<std::int64_t>::max()),rejected,rejected},
        {make_uint64_value(std::numeric_limits<std::uint64_t>::max()),rejected,rejected},
        {make_currency_value(199999),{19,19,-1},{19,19,-1}},
        {make_currency_value(-9000),{-1,0,-1},{-1,0,-1}},
        {make_currency_value(42949673219999LL),rejected,{25,25,-1}},
        {make_string_value("19"),rejected,rejected},
        {make_boolean_value(true),rejected,rejected},
        {typed_null,rejected,rejected},
    };
    for (const auto mode : {NumericBehavior::copperfin,NumericBehavior::vfp9}) {
        for (const auto& row : rows) for (int domain=0;domain<3;++domain) {
            const auto converted = checked_set_century_argument(row.value,mode,domain==1 ? 0 : 1,domain==2 ? 3 : 99);
            const int expected = (mode==NumericBehavior::vfp9 ? row.vfp : row.copperfin)[domain];
            expect(converted.value_or(-1)==expected,"direct CENTURY domain "+std::to_string(domain));
        }
    }
    for (const auto& [year,epoch] : std::map<int,int>{{1,1},{1998,1948},{2026,1976},{2049,1999},{2050,2000},{9999,9949},
                                                   {std::numeric_limits<int>::min(),1},{std::numeric_limits<int>::max(),9999}}) {
        expect(default_set_century_epoch_for_year(year)==epoch,"derived clock boundary "+std::to_string(year));
    }
}

void test_localized_errors() {
    const char* messages[] = {
        "SET CENTURY TO requires a converted century from 1 through 99 and a rollover from 0 through 99.",
        "SET CENTURY TO requiere un siglo convertido de 1 a 99 y un punto de cambio de 0 a 99.",
        "SET CENTURY TO requer um século convertido de 1 a 99 e um ponto de mudança de 0 a 99.",
        "[!! SET CENTURY TO řëqüïřëš å çøñṽëřţëð çëñţüřý ƒřøm 1 ţhřøüĝh 99 åñð å řøľľøṽëř ƒřøm 0 ţhřøüĝh 99. !!]"
    };
    int index=0;
    for (const auto locale : {"en-US","es-419","pt-BR","qps-ploc"}) {
        ScopedEnvironmentValue locale_guard("COPPERFIN_LOCALE",locale);
        run_case(std::string("localized ")+locale,
            "SET CENTURY OFF\nSET CENTURY TO 19 ROLLOVER 75\nTRY\nSET CENTURY TO 20 ROLLOVER 100\n"
            "CATCH TO oError\nnError=oError.ErrorNo\ncMessage=oError.Message\nENDTRY\n"
            "cEpoch=SET('EPOCH')\ncDisplay=SET('CENTURY')\n",
            {{"nerror","11"},{"cmessage",messages[index++]},{"cepoch","1975"},{"cdisplay","OFF"}},2);
    }
}

void test_window_consumers_and_interoperability() {
    struct Window { int century; int rollover; std::array<int,8> years; };
    const Window windows[] = {
        {19,50,{2000,2024,2025,2049,1950,1974,1975,1999}},
        {19,75,{2000,2024,2025,2049,2050,2074,1975,1999}},
        {20,25,{2100,2124,2025,2049,2050,2074,2075,2099}}
    };
    const std::array<int,8> short_years{0,24,25,49,50,74,75,99};
    for (const auto mode : {"COPPERFIN","VFP9"}) for (const auto display : {"OFF","ON"}) {
        for (const auto& window : windows) for (std::size_t i=0;i<short_years.size();++i) {
            const auto yy = (short_years[i]<10 ? "0" : "")+std::to_string(short_years[i]);
            const auto date = std::to_string(window.years[i])+"0102";
            const auto formatted = std::string("01/02/")+(std::string(display)=="OFF" ? yy : std::to_string(window.years[i]));
            run_case("CENTURY consumers "+date+" "+display+" "+mode,
                "SET NUMERICBEHAVIOR TO "+std::string(mode)+"\nSET DATE TO MDY\nSET CENTURY "+display+
                "\nSET CENTURY TO "+std::to_string(window.century)+" ROLLOVER "+std::to_string(window.rollover)+
                "\nd=CTOD('01/02/"+yy+"')\nt=CTOT('01/02/"+yy+" 13:45:56')\n"
                "cDate=DTOC(d,1)\ncDisplay=DTOC(d)\ncTime=TTOC(t,1)\ncDateFromTime=DTOC(TTOD(t),1)\n"
                "cMidnight=TTOC(DTOT(d),1)\ncTtos=TTOS(t)\ncFull=DTOC(CTOD('01/02/1949'),1)\n",
                {{"cdate",date},{"cdisplay",formatted},{"ctime",date+"134556"},
                 {"cdatefromtime",date},{"cmidnight",date+"000000"},{"cttos",date+"134556"},{"cfull","19490102"}});
        }
        run_case(std::string("interface/reset/session ")+mode+display,
            "SET NUMERICBEHAVIOR TO "+std::string(mode)+"\nSET CENTURY "+display+"\n"
            "nDefault=YEAR(DATE())-50\nlInitial=VAL(SET('EPOCH'))=nDefault\n"
            "SET EPOCH TO 2025\nlFromEpoch=SET('CENTURY',1)=20 AND SET('CENTURY',2)=25\n"
            "SET CENTURY TO 19 ROLLOVER 75\nlFromCentury=SET('EPOCH')='1975'\n"
            "SET EPOCH TO 1\nlLowEpoch=SET('CENTURY',1)=0 AND SET('CENTURY',2)=1\n"
            "SET CENTURY TO\nlReset=VAL(SET('EPOCH'))=nDefault\ncDisplay=SET('CENTURY')\n"
            "SET EPOCH TO\nlEpochReset=SET('EPOCH')='1950'\nSET EPOCH TO 2025\n"
            "SET DATASESSION TO 2\nlFresh=VAL(SET('EPOCH'))=nDefault\ncFreshDisplay=SET('CENTURY')\n"
            "SET CENTURY TO 18 ROLLOVER 25\nSET DATASESSION TO 1\nlRestored=SET('EPOCH')='2025'\n"
            "SET DATASESSION TO 2\nlSecond=SET('EPOCH')='1825'\n",
            {{"linitial","true"},{"lfromepoch","true"},{"lfromcentury","true"},
             {"llowepoch","true"},{"lreset","true"},{"cdisplay",display},
             {"lepochreset","true"},{"lfresh","true"},{"cfreshdisplay","OFF"},
             {"lrestored","true"},{"lsecond","true"}});
    }
}

void test_regional_calendar_query() {
    int maximum = -1;
#if defined(_WIN32)
    DWORD calendar = 0, year = 0;
    if (GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_ICALENDARTYPE|LOCALE_RETURN_NUMBER,
                      reinterpret_cast<LPWSTR>(&calendar), sizeof(calendar)/sizeof(wchar_t)) &&
        GetCalendarInfoW(LOCALE_USER_DEFAULT, calendar, CAL_ITWODIGITYEARMAX|CAL_RETURN_NUMBER,
                         nullptr,0,&year)) maximum=static_cast<int>(year);
#endif
    for (const auto selector : {"3","3.9"}) run_case(std::string("calendar ")+selector,
        "SET CENTURY TO 19 ROLLOVER 75\nnBefore=SET('CENTURY',"+std::string(selector)+")\n"
        "SET EPOCH TO 2025\nnAfter=SET('CENTURY',"+selector+")\ncType=VARTYPE(nBefore)\n",
        {{"nbefore",std::to_string(maximum)},{"nafter",std::to_string(maximum)},{"ctype","N"}});
}
} // namespace

int main() {
    test_direct_boundaries();
    test_commands_and_atomicity();
    test_queries();
    test_window_consumers_and_interoperability();
    test_regional_calendar_query();
    test_localized_errors();
    if (test_failures()) { std::cerr << test_failures() << " test(s) failed.\n"; return EXIT_FAILURE; }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

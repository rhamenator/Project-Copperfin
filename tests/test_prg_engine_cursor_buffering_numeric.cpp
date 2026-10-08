// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <cmath>
#include <filesystem>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-CURSORSETPROP-BUFFERING-NUMERIC-001: independent constants
// from native Numeric observations or derived exact-extension admission.
struct Case { const char* expression; double value; std::optional<int> copperfin, vfp9; };
const double infinity=std::numeric_limits<double>::infinity();
const std::vector<Case> cases{
        {"0", 0, std::nullopt, std::nullopt},
        {"1", 1, 1, 1},
        {"2", 2, 2, 2},
        {"3", 3, 3, 3},
        {"4", 4, 4, 4},
        {"5", 5, 5, 5},
        {"6", 6, std::nullopt, std::nullopt},
        {"-1", -1, std::nullopt, std::nullopt},
        {"0.49", 0.49, std::nullopt, std::nullopt},
        {"0.5", 0.5, std::nullopt, std::nullopt},
        {"0.9", 0.9, std::nullopt, std::nullopt},
        {"1.49", 1.49, 1, 1},
        {"1.5", 1.5, 1, 1},
        {"1.9", 1.9, 1, 1},
        {"2.5", 2.5, 2, 2},
        {"3.5", 3.5, 3, 3},
        {"4.5", 4.5, 4, 4},
        {"4.9", 4.9, 4, 4},
        {"5.9", 5.9, 5, 5},
        {"-0.5", -0.5, std::nullopt, std::nullopt},
        {"-1.5", -1.5, std::nullopt, std::nullopt},
        {"2147483647", 2147483647, std::nullopt, std::nullopt},
        {"2147483648", 2147483648, std::nullopt, std::nullopt},
        {"-2147483648", -2147483648, std::nullopt, std::nullopt},
        {"4294967295", 4294967295, std::nullopt, std::nullopt},
        {"4294967296", 4294967296, std::nullopt, std::nullopt},
        {"4294967297", 4294967297, std::nullopt, 1},
        {"4294967298", 4294967298, std::nullopt, 2},
        {"4294967299", 4294967299, std::nullopt, 3},
        {"4294967300", 4294967300, std::nullopt, 4},
        {"4294967301", 4294967301, std::nullopt, 5},
        {"-4294967295", -4294967295, std::nullopt, 1},
        {"9007199254740992", 9007199254740992, std::nullopt, std::nullopt},
        {"1E20", 1e20, std::nullopt, std::nullopt},
        {"-1E20", -1e20, std::nullopt, std::nullopt},
        {"1E300", 1e300, std::nullopt, std::nullopt},
        {"-1E300", -1e300, std::nullopt, std::nullopt},
        {"1E300*1E300", infinity, std::nullopt, std::nullopt},
        {"-1E300*1E300", -infinity, std::nullopt, std::nullopt},
};
void direct_boundaries() {
    struct Extra { PrgValue value; std::optional<int> copperfin, vfp9; };
    const std::vector<Extra> extra{
        {make_number_value(std::numeric_limits<double>::quiet_NaN()),std::nullopt,std::nullopt},
        {make_number_value(std::nextafter(1.0,0.0)),std::nullopt,std::nullopt},
        {make_number_value(std::nextafter(6.0,0.0)),5,5},
        {make_number_value(std::nextafter(6.0,infinity)),std::nullopt,std::nullopt},
        {make_number_value(std::nextafter(9223372036854775808.0,0.0)),std::nullopt,std::nullopt},
        {make_number_value(9223372036854775808.0),std::nullopt,std::nullopt},
        {make_number_value(-9223372036854775808.0),std::nullopt,std::nullopt},
        {make_int64_value(5),5,5},
        {make_int64_value(INT64_MAX),std::nullopt,std::nullopt},
        {make_int64_value(INT64_MIN),std::nullopt,std::nullopt},
        {make_int64_value(INT64_MIN+1),std::nullopt,1},
        {make_int64_value(INT64_MIN+5),std::nullopt,5},
        {make_int64_value(INT64_C(9007199254740993)),std::nullopt,1},
        {make_uint64_value(UINT64_C(9007199254740993)),std::nullopt,1},
        {make_uint64_value(UINT64_MAX),std::nullopt,std::nullopt},
        {make_uint64_value(UINT64_C(9223372036854775813)),std::nullopt,5},
        {make_uint64_value(1),1,1},
        {make_null_value(),std::nullopt,std::nullopt},
        {make_boolean_value(true),1,1},
        {make_string_value("3"),3,3},
        {make_currency_value(35000),4,4},
        {make_currency_value(INT64_MAX),std::nullopt,std::nullopt},
        {make_currency_value(INT64_MIN),std::nullopt,std::nullopt},
    };
    for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
        for(const auto& row:cases) {
            expect(checked_cursor_buffering_mode_argument(make_number_value(row.value),mode)==
                (mode==NumericBehavior::copperfin?row.copperfin:row.vfp9),
                std::string("BUFFERING direct ")+row.expression);
        }
        for(std::size_t i=0;i<extra.size();++i) {
            expect(checked_cursor_buffering_mode_argument(extra[i].value,mode)==
                (mode==NumericBehavior::copperfin?extra[i].copperfin:extra[i].vfp9),
                "BUFFERING exact/control "+std::to_string(i));
        }
    }
}
void script_cases() {
    for(const bool legacy:{false,true}) for(const bool explicit_alias:{false,true}) {
        const std::string mode=legacy?"VFP9":"COPPERFIN";
        const auto dir=fs::temp_directory_path()/("copperfin_cursorbuffer_5611_"+mode+(explicit_alias?"_alias":"_implicit"));
        fs::create_directories(dir);
        const auto path=dir/"buffering.prg";
        std::string source="SET NUMERICBEHAVIOR TO "+mode+"\nSET MULTILOCKS ON\ncOut=''\n";
        std::vector<std::string> expected;
        const auto add=[&](const std::string& expression,const std::optional<int> accepted,
            const bool numeric,const std::string& operand) {
            source+="CREATE CURSOR modeguard (label C(8))\nINSERT INTO modeguard VALUES ('guard')\nGO TOP\n"+
                std::string("=CURSORSETPROP('BUFFERING',3,'modeguard')\nx=-99\ncStatus='OK'\ncResult='unassigned'\ncMessage=''\nTRY\n")+
                "x=CURSORSETPROP('BUFFERING',"+expression+(explicit_alias?",'modeguard'":"")+")\n"+
                "cResult=IIF(x=.T.,'T','F')\nCATCH TO oError\ncStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))\ncMessage=oError.Message\nENDTRY\n"+
                "cOut=cOut+cStatus+'|'+cResult+'|'+cMessage+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','modeguard'))+'|'+"+
                "TRANSFORM(RECNO('modeguard'))+':'+TRANSFORM(RECCOUNT('modeguard'))+':'+ALLTRIM(modeguard.label)+'|'+"+
                "SET('NUMERICBEHAVIOR')+'|'+SET('DATASESSION')+CHR(10)\nUSE IN modeguard\n";
            const std::string head=accepted?"OK|T||":numeric?
                "ERR1469|unassigned|Unsupported cursor buffering mode: "+operand+"|":"OK|F||";
            expected.push_back(head+std::to_string(accepted.value_or(3))+"|1:1:guard|"+mode+"|1");
        };
        for(const auto& row:cases) add(row.expression,legacy?row.vfp9:row.copperfin,true,format_round_trip_decimal(row.value));
        // Preserve existing other-type results; native type parity is separate.
        add(".NULL.",std::nullopt,false,""); add(".T.",1,false,"");
        add("'3'",3,false,""); add("$3.5",4,false,"");
        source+="SET MULTILOCKS OFF\nSET NUMERICBEHAVIOR TO COPPERFIN\ncCleanup=IIF(USED('modeguard'),'open','closed')+'|'+SET('MULTILOCKS')+'|'+SET('NUMERICBEHAVIOR')\nRETURN\n";
        write_text(path,source);
        auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
        const auto state=session.run(DebugResumeAction::continue_run);
        expect(state.completed,"BUFFERING guarded script completes "+mode+": "+state.message);
        const auto output=state.globals.find("cout");
        const std::string actual=output==state.globals.end()?"<missing>":format_value(output->second);
        std::istringstream stream(actual);
        std::string line;
        for(std::size_t i=0;i<expected.size();++i) {
            std::getline(stream,line);
            expect(line==expected[i],"BUFFERING row "+mode+(explicit_alias?" alias ":" implicit ")+
                std::to_string(i)+" expected "+expected[i]+" got "+line);
        }
        expect(!std::getline(stream,line),"BUFFERING no extra output");
        const auto cleanup=state.globals.find("ccleanup");
        expect(cleanup!=state.globals.end()&&format_value(cleanup->second)=="closed|OFF|COPPERFIN","BUFFERING cleanup/reset");
        std::error_code ignored;
        fs::remove_all(dir,ignored);
    }
}
}
int main() {
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE","en-US",true);
    direct_boundaries();
    script_cases();
    return test_failures()==0?0:1;
}

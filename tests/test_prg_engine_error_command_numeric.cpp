// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-ERROR-COMMAND-NUMERIC-001: constants independent of the
// helper; native-supported catalog entries versus derived conversion domain
// are distinguished in the fixture. #7079's unsupported catalog is unchanged.
struct Case { const char* expression; double value; std::optional<int> copperfin, vfp9; };
const double infinity = std::numeric_limits<double>::infinity();
const std::vector<Case> cases{
    {"-1",-1,std::nullopt,std::nullopt}, {"0",0,0,0},
    {"0.49",0.49,0,0}, {"0.5",0.5,0,0}, {"0.9",0.9,0,0},
    {"1",1,1,1}, {"1.49",1.49,1,1}, {"1.5",1.5,1,1}, {"1.9",1.9,1,1},
    {"11",11,11,11}, {"11.5",11.5,11,11}, {"12",12,12,12},
    {"99",99,99,99}, {"1098",1098,1098,1098}, {"2000",2000,2000,2000},
    {"2001",2001,2001,2001}, {"9999",9999,9999,9999}, {"65535",65535,65535,65535},
    {"2147483647",2147483647,INT32_MAX,INT32_MAX},
    {"2147483648",2147483648,std::nullopt,std::nullopt},
    {"-2147483648",-2147483648,std::nullopt,std::nullopt},
    {"4294967295",4294967295,std::nullopt,std::nullopt},
    {"4294967296",4294967296,std::nullopt,std::nullopt},
    {"4294967297",4294967297,std::nullopt,std::nullopt},
    {"-4294967295",-4294967295,std::nullopt,1},
    {"9007199254740992",9007199254740992,std::nullopt,std::nullopt},
    {"1E20",1e20,std::nullopt,std::nullopt}, {"-1E20",-1e20,std::nullopt,0},
    {"1E300",1e300,std::nullopt,std::nullopt}, {"-1E300",-1e300,std::nullopt,0},
    {"1E300*1E300",infinity,std::nullopt,std::nullopt},
    {"-1E300*1E300",-infinity,std::nullopt,0},
};
void direct_boundaries() {
    struct Extra { PrgValue value; std::optional<int> copperfin, vfp9; };
    const std::vector<Extra> extra{
        {make_number_value(std::numeric_limits<double>::quiet_NaN()),std::nullopt,std::nullopt},
        {make_number_value(-0.0),0,0}, {make_number_value(-0.5),0,0},
        {make_number_value(std::nextafter(1.0,0.0)),0,0},
        {make_number_value(std::nextafter(2147483648.0,0.0)),INT32_MAX,INT32_MAX},
        {make_number_value(std::nextafter(2147483648.0,infinity)),std::nullopt,std::nullopt},
        {make_number_value(std::nextafter(9223372036854775808.0,0.0)),std::nullopt,std::nullopt},
        {make_number_value(9223372036854775808.0),std::nullopt,std::nullopt},
        {make_number_value(-9223372036854775808.0),std::nullopt,0},
        {make_int64_value(12),12,12}, {make_int64_value(INT32_MAX),INT32_MAX,INT32_MAX},
        {make_int64_value(INT64_MAX),std::nullopt,std::nullopt},
        {make_int64_value(INT64_MIN),std::nullopt,0},
        {make_int64_value(INT64_MIN+1),std::nullopt,1},
        {make_int64_value(INT64_C(9007199254740993)),std::nullopt,std::nullopt},
        {make_uint64_value(UINT64_C(9007199254740993)),std::nullopt,std::nullopt},
        {make_uint64_value(UINT64_MAX),std::nullopt,std::nullopt},
        {make_uint64_value(12),12,12}, {make_uint64_value(INT32_MAX),INT32_MAX,INT32_MAX},
        {make_currency_value(115000),12,12}, {make_currency_value(5000),1,1},
        {make_currency_value(-5000),std::nullopt,std::nullopt},
        {make_currency_value(INT64_MAX),std::nullopt,std::nullopt},
        {make_currency_value(INT64_MIN),std::nullopt,std::nullopt},
        {make_null_value(),std::nullopt,std::nullopt},
        {make_boolean_value(true),std::nullopt,std::nullopt},
        {make_string_value("12"),std::nullopt,std::nullopt},
    };
    for (const auto mode : {NumericBehavior::copperfin,NumericBehavior::vfp9}) {
        for (const auto& row : cases) {
            expect(checked_error_number_argument(make_number_value(row.value),mode)==
                (mode==NumericBehavior::copperfin?row.copperfin:row.vfp9),
                std::string("ERROR direct ")+row.expression);
        }
        for (std::size_t i=0;i<extra.size();++i) {
            expect(checked_error_number_argument(extra[i].value,mode)==
                (mode==NumericBehavior::copperfin?extra[i].copperfin:extra[i].vfp9),
                "ERROR exact/control "+std::to_string(i));
        }
    }
}
void script_cases() {
    for (const bool legacy : {false,true}) for (const bool parameter : {false,true}) {
        const std::string mode = legacy?"VFP9":"COPPERFIN";
        const auto dir = fs::temp_directory_path()/("copperfin_errornumber_5611_"+mode+(parameter?"_parameter":"_bare"));
        fs::create_directories(dir);
        const auto path = dir/"errornumber.prg";
        std::string source = "SET NUMERICBEHAVIOR TO "+mode+"\nCREATE CURSOR errorguard (label C(8))\nINSERT INTO errorguard VALUES ('guard')\nPUBLIC nCalls\ncOut=''\n";
        std::vector<std::string> expected;
        const auto add = [&](const std::string& expression,const std::optional<int> converted,
                             const std::string& operand,const bool character=false) {
            const int code = character?1098:converted&&*converted>0?*converted:1941;
            const int calls = parameter&&converted?1:0;
            const std::string message = character?"custom":code==1941?
                "Invalid ERROR number: "+operand:parameter?
                "Error "+std::to_string(code)+": 'guard'.":"Error "+std::to_string(code)+".";
            // Seed old metadata, then check each rejected/accepted request replaces
            // it, preserves cursor/session/mode, and cannot fall through.
            source += "TRY\nERROR 12, 'prior'\nCATCH\nENDTRY\nnCalls=0\nlAfter=.F.\ncStatus='none'\ncMessage=''\nnError=-99\nnAError=-99\nlMetadata=.F.\nTRY\nERROR "+expression+
                (parameter&&!character?",ParameterText()":"")+"\nlAfter=.T.\nCATCH TO oError\ncStatus=ALLTRIM(STR(oError.ErrorNo,20,0))\ncMessage=oError.Message\nnError=ERROR()\n=AERROR(aErr)\nnAError=aErr[1,1]\nlMetadata=aErr[1,2]==oError.Message\nENDTRY\n"+
                "cOut=cOut+cStatus+'|'+IIF(lAfter,'T','F')+'|'+TRANSFORM(nCalls)+'|'+ALLTRIM(STR(nError,20,0))+'|'+ALLTRIM(STR(nAError,20,0))+'|'+IIF(lMetadata,'T','F')+'|'+cMessage+'|'+"+
                "TRANSFORM(RECNO('errorguard'))+':'+TRANSFORM(RECCOUNT('errorguard'))+':'+ALLTRIM(errorguard.label)+'|'+SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+CHR(10)\n";
            expected.push_back(std::to_string(code)+"|F|"+std::to_string(calls)+"|"+std::to_string(code)+"|"+std::to_string(code)+"|T|"+message+"|1:1:guard|1|"+mode);
        };
        for (const auto& row : cases) add(row.expression,legacy?row.vfp9:row.copperfin,format_round_trip_decimal(row.value));
        add("$11.5",12,""); add("$0.5",1,"");
        add("$922337203685477.5807",std::nullopt,"922337203685477.5807");
        add("-$11.5",std::nullopt,"-11.5000"); add("'custom'",std::nullopt,"",true);
        source += "USE IN errorguard\nSET NUMERICBEHAVIOR TO COPPERFIN\ncCleanup=IIF(USED('errorguard'),'open','closed')+'|'+SET('NUMERICBEHAVIOR')\nRETURN\nFUNCTION ParameterText\nnCalls=nCalls+1\nRETURN 'guard'\nENDFUNC\n";
        write_text(path,source);
        auto session = PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
        const auto state = session.run(DebugResumeAction::continue_run);
        expect(state.completed,"ERROR guarded script completes "+mode+": "+state.message);
        const auto output = state.globals.find("cout");
        const std::string actual = output==state.globals.end()?"<missing>":format_value(output->second);
        std::istringstream stream(actual);
        std::string line;
        for (std::size_t i=0;i<expected.size();++i) {
            std::getline(stream,line);
            expect(line.ends_with("|1:1:guard|1|"+mode),"ERROR state suffix "+std::to_string(i));
            if (i==12 || i==32 || i==33) {
                std::cout << "CONTROL " << mode << (parameter?" parameter ":" bare ")
                          << i << ' ' << line << '\n';
            }
            expect(line==expected[i],"ERROR row "+mode+(parameter?" parameter ":" bare ")+
                std::to_string(i)+" expected "+expected[i]+" got "+line);
        }
        expect(!std::getline(stream,line),"ERROR no extra output");
        const auto cleanup = state.globals.find("ccleanup");
        expect(cleanup!=state.globals.end()&&format_value(cleanup->second)=="closed|COPPERFIN","ERROR cleanup/reset");
        std::error_code ignored;
        fs::remove_all(dir,ignored);
    }
}
void localized_rejections() {
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    for (const std::string locale : {"en-US","es-419","pt-BR","qps-ploc"}) {
        set_env_value("COPPERFIN_LOCALE",locale.c_str(),true);
        for (const std::string mode : {"COPPERFIN","VFP9"}) {
            const auto dir = fs::temp_directory_path()/("copperfin_error_locale_5611_"+locale+mode);
            fs::create_directories(dir);
            const auto path = dir/"localized.prg";
            write_text(path,"SET NUMERICBEHAVIOR TO "+mode+
                "\nnCode=-99\ncMessage='missing'\nTRY\nERROR -1\nCATCH TO oError\nnCode=oError.ErrorNo\ncMessage=oError.Message\nENDTRY\nRETURN\n");
            auto session = PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
            const auto state = session.run(DebugResumeAction::continue_run);
            expect(state.completed,"ERROR localized script "+locale+": "+state.message);
            const auto code = state.globals.find("ncode");
            expect(code!=state.globals.end()&&format_value(code->second)=="1941","ERROR localized code invariant");
            const auto message = state.globals.find("cmessage");
            const std::string text = message==state.globals.end()?"<missing>":format_value(message->second);
            const std::string expected = locale=="en-US"?"Invalid ERROR number: -1":
                locale=="es-419"?"Número de ERROR no válido: -1":"Número de ERROR inválido: -1";
            expect(locale=="qps-ploc"?text.starts_with("[!! ")&&text.find("-1")!=std::string::npos:
                text==expected,"ERROR localized operand/message "+locale+": "+text);
            std::error_code ignored;
            fs::remove_all(dir,ignored);
        }
    }
}
}
int main() {
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE","en-US",true);
    direct_boundaries();
    script_cases();
    localized_rejections();
    return test_failures()==0?0:1;
}

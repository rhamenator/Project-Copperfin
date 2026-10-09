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
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-BITCLEAR-NUMERIC-001: frozen independent installed literal results.
// Value conversion is recovered from BOTH clear-bit0 and clear-bit1 observations.
struct Row { const char* text; double raw; std::optional<std::int64_t> normal, legacy; };
const std::vector<Row> values{
 {"0",0,0,0},
 {"1",1,1,1},
 {"-1",-1,-1,-1},
 {"0.49",0.49,0,0},
 {"0.5",0.5,0,0},
 {"0.9",0.9,0,0},
 {"1.49",1.49,1,1},
 {"1.5",1.5,1,1},
 {"1.9",1.9,1,1},
 {"-0.49",-0.49,0,0},
 {"-0.5",-0.5,0,0},
 {"-0.9",-0.9,0,0},
 {"-1.49",-1.49,-1,-1},
 {"-1.5",-1.5,-1,-1},
 {"-1.9",-1.9,-1,-1},
 {"2.5",2.5,2,2},
 {"-2.5",-2.5,-2,-2},
 {"2147483647",2147483647,2147483647,2147483647},
 {"2147483647.9",2147483647.9,2147483647,2147483647},
 {"2147483648",2147483648,{},-2147483648},
 {"-2147483648",-2147483648,-2147483648,-2147483648},
 {"-2147483648.9",-2147483648.9,-2147483648,-2147483648},
 {"-2147483649",-2147483649,{},2147483647},
 {"4294967295",4294967295,{},-1},
 {"4294967296",4294967296,{},0},
 {"4294967297",4294967297,{},1},
 {"-4294967295",-4294967295,{},1},
 {"-4294967296",-4294967296,{},0},
 {"-4294967297",-4294967297,{},-1},
 {"9007199254740992",9007199254740992,{},0},
 {"1E20",1E20,{},0},
 {"-1E20",-1E20,{},0},
 {"1E300",1E300,{},0},
 {"-1E300",-1E300,{},0},
 {"EXP(1000)",std::numeric_limits<double>::infinity(),{},0},
 {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{},0},
};
const std::vector<Row> positions{
 {"-1.9",-1.9,{},{}},
 {"-1",-1,{},{}},
 {"-0.9",-0.9,0,0},
 {"-0.5",-0.5,0,0},
 {"0",0,0,0},
 {"0.49",0.49,0,0},
 {"0.5",0.5,0,0},
 {"0.9",0.9,0,0},
 {"1.49",1.49,1,1},
 {"1.5",1.5,1,1},
 {"1.9",1.9,1,1},
 {"2.5",2.5,2,2},
 {"30.9",30.9,30,30},
 {"31",31,31,31},
 {"31.9",31.9,31,31},
 {"32",32,{},{}},
 {"32.1",32.1,{},{}},
 {"2147483647",2147483647,{},{}},
 {"2147483648",2147483648,{},{}},
 {"-2147483648",-2147483648,{},{}},
 {"-2147483649",-2147483649,{},{}},
 {"4294967295",4294967295,{},{}},
 {"4294967296",4294967296,{},0},
 {"4294967297",4294967297,{},1},
 {"4294967327",4294967327,{},31},
 {"-4294967295",-4294967295,{},1},
 {"-4294967296",-4294967296,{},0},
 {"-4294967297",-4294967297,{},{}},
 {"-4294967265",-4294967265,{},31},
 {"9007199254740992",9007199254740992,{},0},
 {"1E20",1E20,{},0},
 {"-1E20",-1E20,{},0},
 {"1E300",1E300,{},0},
 {"-1E300",-1E300,{},0},
 {"EXP(1000)",std::numeric_limits<double>::infinity(),{},0},
 {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{},0},
};
struct PublicRow { const char* args; std::optional<std::int64_t> normal, legacy; };
const std::vector<PublicRow> public_rows{
 {"0,0",0,0},
 {"0,1",0,0},
 {"1,0",0,0},
 {"1,1",1,1},
 {"-1,0",-2,-2},
 {"-1,1",-3,-3},
 {"0.49,0",0,0},
 {"0.49,1",0,0},
 {"0.5,0",0,0},
 {"0.5,1",0,0},
 {"0.9,0",0,0},
 {"0.9,1",0,0},
 {"1.49,0",0,0},
 {"1.49,1",1,1},
 {"1.5,0",0,0},
 {"1.5,1",1,1},
 {"1.9,0",0,0},
 {"1.9,1",1,1},
 {"-0.49,0",0,0},
 {"-0.49,1",0,0},
 {"-0.5,0",0,0},
 {"-0.5,1",0,0},
 {"-0.9,0",0,0},
 {"-0.9,1",0,0},
 {"-1.49,0",-2,-2},
 {"-1.49,1",-3,-3},
 {"-1.5,0",-2,-2},
 {"-1.5,1",-3,-3},
 {"-1.9,0",-2,-2},
 {"-1.9,1",-3,-3},
 {"2.5,0",2,2},
 {"2.5,1",0,0},
 {"-2.5,0",-2,-2},
 {"-2.5,1",-4,-4},
 {"2147483647,0",2147483646,2147483646},
 {"2147483647,1",2147483645,2147483645},
 {"2147483647.9,0",2147483646,2147483646},
 {"2147483647.9,1",2147483645,2147483645},
 {"2147483648,0",{},-2147483648},
 {"2147483648,1",{},-2147483648},
 {"-2147483648,0",-2147483648,-2147483648},
 {"-2147483648,1",-2147483648,-2147483648},
 {"-2147483648.9,0",-2147483648,-2147483648},
 {"-2147483648.9,1",-2147483648,-2147483648},
 {"-2147483649,0",{},2147483646},
 {"-2147483649,1",{},2147483645},
 {"4294967295,0",{},-2},
 {"4294967295,1",{},-3},
 {"4294967296,0",{},0},
 {"4294967296,1",{},0},
 {"4294967297,0",{},0},
 {"4294967297,1",{},1},
 {"-4294967295,0",{},0},
 {"-4294967295,1",{},1},
 {"-4294967296,0",{},0},
 {"-4294967296,1",{},0},
 {"-4294967297,0",{},-2},
 {"-4294967297,1",{},-3},
 {"9007199254740992,0",{},0},
 {"9007199254740992,1",{},0},
 {"1E20,0",{},0},
 {"1E20,1",{},0},
 {"-1E20,0",{},0},
 {"-1E20,1",{},0},
 {"1E300,0",{},0},
 {"1E300,1",{},0},
 {"-1E300,0",{},0},
 {"-1E300,1",{},0},
 {"EXP(1000),0",{},0},
 {"EXP(1000),1",{},0},
 {"-EXP(1000),0",{},0},
 {"-EXP(1000),1",{},0},
 {"0,-1.9",{},{}},
 {"-1,-1.9",{},{}},
 {"0,-1",{},{}},
 {"-1,-1",{},{}},
 {"0,-0.9",0,0},
 {"-1,-0.9",-2,-2},
 {"0,-0.5",0,0},
 {"-1,-0.5",-2,-2},
 {"0,0",0,0},
 {"-1,0",-2,-2},
 {"0,0.49",0,0},
 {"-1,0.49",-2,-2},
 {"0,0.5",0,0},
 {"-1,0.5",-2,-2},
 {"0,0.9",0,0},
 {"-1,0.9",-2,-2},
 {"0,1.49",0,0},
 {"-1,1.49",-3,-3},
 {"0,1.5",0,0},
 {"-1,1.5",-3,-3},
 {"0,1.9",0,0},
 {"-1,1.9",-3,-3},
 {"0,2.5",0,0},
 {"-1,2.5",-5,-5},
 {"0,30.9",0,0},
 {"-1,30.9",-1073741825,-1073741825},
 {"0,31",0,0},
 {"-1,31",2147483647,2147483647},
 {"0,31.9",0,0},
 {"-1,31.9",2147483647,2147483647},
 {"0,32",{},{}},
 {"-1,32",{},{}},
 {"0,32.1",{},{}},
 {"-1,32.1",{},{}},
 {"0,2147483647",{},{}},
 {"-1,2147483647",{},{}},
 {"0,2147483648",{},{}},
 {"-1,2147483648",{},{}},
 {"0,-2147483648",{},{}},
 {"-1,-2147483648",{},{}},
 {"0,-2147483649",{},{}},
 {"-1,-2147483649",{},{}},
 {"0,4294967295",{},{}},
 {"-1,4294967295",{},{}},
 {"0,4294967296",{},0},
 {"-1,4294967296",{},-2},
 {"0,4294967297",{},0},
 {"-1,4294967297",{},-3},
 {"0,4294967327",{},0},
 {"-1,4294967327",{},2147483647},
 {"0,-4294967295",{},0},
 {"-1,-4294967295",{},-3},
 {"0,-4294967296",{},0},
 {"-1,-4294967296",{},-2},
 {"0,-4294967297",{},{}},
 {"-1,-4294967297",{},{}},
 {"0,-4294967265",{},0},
 {"-1,-4294967265",{},2147483647},
 {"0,9007199254740992",{},0},
 {"-1,9007199254740992",{},-2},
 {"0,1E20",{},0},
 {"-1,1E20",{},-2},
 {"0,-1E20",{},0},
 {"-1,-1E20",{},-2},
 {"0,1E300",{},0},
 {"-1,1E300",{},-2},
 {"0,-1E300",{},0},
 {"-1,-1E300",{},-2},
 {"0,EXP(1000)",{},0},
 {"-1,EXP(1000)",{},-2},
 {"0,-EXP(1000)",{},0},
 {"-1,-EXP(1000)",{},-2},
 {"-1,0",-2,-2},
 {"-1,1",-3,-3},
 {"-1,2",-5,-5},
 {"-1,3",-9,-9},
 {"-1,4",-17,-17},
 {"-1,5",-33,-33},
 {"-1,6",-65,-65},
 {"-1,7",-129,-129},
 {"-1,8",-257,-257},
 {"-1,9",-513,-513},
 {"-1,10",-1025,-1025},
 {"-1,11",-2049,-2049},
 {"-1,12",-4097,-4097},
 {"-1,13",-8193,-8193},
 {"-1,14",-16385,-16385},
 {"-1,15",-32769,-32769},
 {"-1,16",-65537,-65537},
 {"-1,17",-131073,-131073},
 {"-1,18",-262145,-262145},
 {"-1,19",-524289,-524289},
 {"-1,20",-1048577,-1048577},
 {"-1,21",-2097153,-2097153},
 {"-1,22",-4194305,-4194305},
 {"-1,23",-8388609,-8388609},
 {"-1,24",-16777217,-16777217},
 {"-1,25",-33554433,-33554433},
 {"-1,26",-67108865,-67108865},
 {"-1,27",-134217729,-134217729},
 {"-1,28",-268435457,-268435457},
 {"-1,29",-536870913,-536870913},
 {"-1,30",-1073741825,-1073741825},
 {"-1,31",2147483647,2147483647},
 {"5,1",5,5},
 {"7.9,1.9",5,5},
 {"-2.9,0.9",-2,-2},
 {"0,31.9",0,0},
 {"0,32",{},{}},
 {"0,1E300",{},0},
 {"-1,32",{},{}},
 {"7,1E20",{},6},
 {"4294967297,4294967297",{},1},
 {"EXP(1000),EXP(1000)",{},0},
};
int direct_count=0, public_count=0;
void direct_cases() {
 const auto check=[](const PrgValue& v,NumericBehavior mode,std::optional<std::int64_t> result,bool position,const char* tag) {
  ++direct_count;
  const auto operand=position?checked_bitclear_position_argument(v,mode):checked_bitclear_value_argument(v,mode);
  const bool match=result?operand&&static_cast<std::int64_t>(*operand)==*result:!operand;
  expect(match,std::string("BITCLEAR direct ")+(position?"position ":"value ")+tag);
 };
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  const bool legacy=mode==NumericBehavior::vfp9;
  for(const auto& row:values)check(make_number_value(row.raw),mode,legacy?row.legacy:row.normal,false,row.text);
  for(const auto& row:positions)check(make_number_value(row.raw),mode,legacy?row.legacy:row.normal,true,row.text);
  for(const bool position:{false,true}) {
   check(make_number_value(std::numeric_limits<double>::quiet_NaN()),mode,{},position,"NaN");
   check(make_int64_value(4294967297LL),mode,legacy?std::optional<std::int64_t>(1):std::nullopt,position,"signed alias");
   check(make_uint64_value(4294967297ULL),mode,legacy?std::optional<std::int64_t>(1):std::nullopt,position,"unsigned alias");
   check(make_int64_value(9007199254740993LL),mode,legacy?std::optional<std::int64_t>(1):std::nullopt,position,"exact above2^53");
   check(make_int64_value(INT64_MIN),mode,legacy?std::optional<std::int64_t>(0):std::nullopt,position,"signed min64");
   check(make_uint64_value(UINT64_MAX),mode,legacy&&!position?std::optional<std::int64_t>(-1):std::nullopt,position,"unsigned max64");
   check(make_int64_value(INT64_MAX),mode,legacy&&!position?std::optional<std::int64_t>(-1):std::nullopt,position,"signed max64");
   // Safe preservation controls only; not native type/Currency requirements.
   check(make_boolean_value(true),mode,1,position,"Logical");
   check(make_string_value("1.9"),mode,2,position,"Character");
   check(make_currency_value(15000),mode,2,position,"Currency");
   check(make_empty_value(),mode,0,position,"empty");
  }
  check(make_number_value(std::nextafter(2147483648.,0.)),mode,2147483647,false,"below2^31");
  check(make_number_value(std::nextafter(-2147483649.,0.)),mode,-2147483648LL,false,"above-minus-limit");
  check(make_number_value(std::nextafter(32.,0.)),mode,31,true,"below32");
  check(make_number_value(std::nextafter(-1.,0.)),mode,0,true,"above-minus1");
  check(make_int64_value(INT32_MIN),mode,-2147483648LL,false,"exact min32");
  check(make_int64_value(INT32_MAX),mode,2147483647,false,"exact max32");
  check(make_int64_value(31),mode,31,true,"exact31");
  check(make_uint64_value(32),mode,{},true,"exact invalid32");
  check(make_string_value("1E300"),mode,{},true,"unsafe other-position containment");
 }
}
std::string value(const RuntimePauseState& state,const char* name) {
 const auto found=state.globals.find(name);
 return found==state.globals.end()?"<missing>":format_value(found->second);
}
void public_case(const fs::path& path,const std::string& mode,int selected,const std::string& args,std::optional<std::int64_t> expected) {
 ++public_count;
 const auto dir=path.parent_path();
 const auto tag=mode+" session"+std::to_string(selected)+" "+args;
 const auto other=mode=="VFP9"?"COPPERFIN":"VFP9";
 write_text(path,(selected==2?std::string("SET NUMERICBEHAVIOR TO ")+other+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC nResult,nCode,lAfter,cBefore,cAfter,cReset,nGuard\nnResult=12345\nnCode=0\nlAfter=.F.\n"
  "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "cBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\nnResult=BITCLEAR("+args+")\nCATCH TO problem\nnCode=problem.ErrorNo\nENDTRY\nlAfter=.T.\nnGuard=BITCLEAR(5,1)\n"
  "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
  "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"BITCLEAR completion "+tag+": "+state.message);
 expect(value(state,"ncode")== (expected?"0":"11"),"BITCLEAR selected error "+tag+" got "+value(state,"ncode"));
 expect(value(state,"nresult")== (expected?std::to_string(*expected):"12345"),"BITCLEAR selected result "+tag+" got "+value(state,"nresult"));
 if(expected)expect(state.globals.at("nresult").kind==PrgValueKind::int64,"BITCLEAR result kind "+tag);
 expect(value(state,"lafter")=="true"&&value(state,"nguard")=="5","BITCLEAR continuation/control "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(value(state,"cbefore")==context&&value(state,"cafter")==context,"BITCLEAR mode/session/cursor guard "+tag);
 expect(value(state,"creset")=="1|COPPERFIN|","BITCLEAR cleanup/reset "+tag);
}
void public_cases() {
 const auto dir=fs::temp_directory_path()/"copperfin_bitclear_numeric_5611";
 fs::create_directories(dir);const auto path=dir/"query.prg";
 for(const std::string mode:{"COPPERFIN","VFP9"})for(const int selected:{1,2}) {
  const bool legacy=mode=="VFP9";
  for(const auto& row:public_rows)public_case(path,mode,selected,row.args,legacy?row.legacy:row.normal);
  for(const std::string text:{"(CAST(9007199254740992 AS INT64)+CAST(1 AS INT64))","CAST(4294967297 AS UINT64)"}) {
   public_case(path,mode,selected,text+",1",legacy?std::optional<std::int64_t>(1):std::nullopt);
   public_case(path,mode,selected,"0,"+text,legacy?std::optional<std::int64_t>(0):std::nullopt);
   public_case(path,mode,selected,"-1,"+text,legacy?std::optional<std::int64_t>(-3):std::nullopt);
  }
  for(const auto& row:std::vector<std::pair<std::string,std::int64_t>>{{".T.",1},{"'1.9'",2},{"$1.5",2}}) {
   public_case(path,mode,selected,row.first+",0",row.second==1?0:2);
   public_case(path,mode,selected,"-1,"+row.first,row.second==1?-3:-5);
  }
  // Guard invalid late positions even when the value is already zero.
  public_case(path,mode,selected,"0,CAST(32 AS UINT64)",{});
  public_case(path,mode,selected,"CAST(-2147483648 AS INT64),CAST(31 AS INT64)",0);
 }
 std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_cases();public_cases();
 std::cout<<"BITCLEAR frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

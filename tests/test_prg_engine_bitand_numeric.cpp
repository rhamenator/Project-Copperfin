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
// RQ-CF-PRG-BITAND-NUMERIC-001: independent literal installed results.
// AND with -1 exposes conversion; expected values are not helper-derived.
struct Row { const char* text; double raw; std::optional<std::int64_t> normal; std::int64_t legacy; };
const std::vector<Row> rows{
 {"0",0,0,0},{"1",1,1,1},{"-1",-1,-1,-1},
 {"0.49",.49,0,0},{"0.5",.5,0,0},{"0.9",.9,0,0},
 {"1.49",1.49,1,1},{"1.5",1.5,1,1},{"1.9",1.9,1,1},
 {"-0.49",-.49,0,0},{"-0.5",-.5,0,0},{"-0.9",-.9,0,0},
 {"-1.49",-1.49,-1,-1},{"-1.5",-1.5,-1,-1},{"-1.9",-1.9,-1,-1},
 {"2.5",2.5,2,2},{"-2.5",-2.5,-2,-2},
 {"2147483647",2147483647.,2147483647,2147483647},
 {"2147483647.9",2147483647.9,2147483647,2147483647},
 {"2147483648",2147483648.,{},-2147483648LL},
 {"-2147483648",-2147483648.,-2147483648LL,-2147483648LL},
 {"-2147483648.9",-2147483648.9,-2147483648LL,-2147483648LL},
 {"-2147483649",-2147483649.,{},2147483647},
 {"4294967295",4294967295.,{},-1},{"4294967296",4294967296.,{},0},
 {"4294967297",4294967297.,{},1},{"-4294967295",-4294967295.,{},1},
 {"-4294967296",-4294967296.,{},0},{"-4294967297",-4294967297.,{},-1},
 {"9007199254740992",9007199254740992.,{},0},
 {"1E20",1e20,{},0},{"-1E20",-1e20,{},0},
 {"1E300",1e300,{},0},{"-1E300",-1e300,{},0},
 {"EXP(1000)",std::numeric_limits<double>::infinity(),{},0},
 {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{},0}
};
int direct_count=0, public_count=0;
void direct_cases() {
 const auto check=[](const PrgValue& value,NumericBehavior mode,std::optional<std::int64_t> result,const char* tag) {
  ++direct_count;
  const auto operand=checked_bitand_argument(value,mode);
  const bool match=result ? operand&&static_cast<std::int64_t>(*operand)==*result : !operand;
  expect(match,std::string("BITAND direct ")+tag);
 };
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  for(const auto& row:rows)check(make_number_value(row.raw),mode,mode==NumericBehavior::vfp9?std::optional<std::int64_t>(row.legacy):row.normal,row.text);
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),mode,{},"NaN containment");
  check(make_number_value(std::nextafter(2147483648.,0.)),mode,2147483647,"below2^31");
  check(make_number_value(std::nextafter(-2147483649.,0.)),mode,-2147483648LL,"above-minus-limit");
  check(make_number_value(std::nextafter(-2147483649.,-INFINITY)),mode,mode==NumericBehavior::vfp9?std::optional<std::int64_t>(2147483647):std::nullopt,"below-minus-limit");
  const bool legacy=mode==NumericBehavior::vfp9;
  check(make_int64_value(INT32_MAX),mode,2147483647,"exact signed max32");
  check(make_int64_value(INT32_MIN),mode,-2147483648LL,"exact signed min32");
  check(make_int64_value(4294967297LL),mode,legacy?std::optional<std::int64_t>(1):std::nullopt,"exact signed alias");
  check(make_uint64_value(4294967297ULL),mode,legacy?std::optional<std::int64_t>(1):std::nullopt,"exact unsigned alias");
  check(make_int64_value(9007199254740993LL),mode,legacy?std::optional<std::int64_t>(1):std::nullopt,"exact above2^53");
  check(make_uint64_value(UINT64_MAX),mode,legacy?std::optional<std::int64_t>(-1):std::nullopt,"exact unsigned max");
  check(make_int64_value(INT64_MIN),mode,legacy?std::optional<std::int64_t>(0):std::nullopt,"exact signed min");
  check(make_int64_value(INT64_MAX),mode,legacy?std::optional<std::int64_t>(-1):std::nullopt,"exact signed max");
  // Preservation controls, not native type/Currency parity requirements.
  check(make_boolean_value(true),mode,1,"preserved Logical");
  check(make_string_value("1.9"),mode,2,"preserved Character");
  check(make_currency_value(15000),mode,2,"preserved Currency");
  check(make_empty_value(),mode,0,"preserved empty");
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
  "TRY\nnResult=BITAND("+args+")\nCATCH TO problem\nnCode=problem.ErrorNo\nENDTRY\nlAfter=.T.\nnGuard=BITAND(3,6)\n"
  "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
  "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"BITAND completion "+tag+": "+state.message);
 expect(value(state,"ncode")== (expected?"0":"11"),"BITAND selected error "+tag+" got "+value(state,"ncode"));
 expect(value(state,"nresult")== (expected?std::to_string(*expected):"12345"),"BITAND selected result "+tag+" got "+value(state,"nresult"));
 expect(value(state,"lafter")=="true"&&value(state,"nguard")=="2","BITAND continuation/control "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(value(state,"cbefore")==context&&value(state,"cafter")==context,"BITAND mode/session/cursor guard "+tag);
 expect(value(state,"creset")=="1|COPPERFIN|","BITAND cleanup/reset "+tag);
}
void public_cases() {
 const auto dir=fs::temp_directory_path()/"copperfin_bitand_numeric_5611";
 fs::create_directories(dir);const auto path=dir/"query.prg";
 for(const std::string mode:{"COPPERFIN","VFP9"})for(const int selected:{1,2}) {
  const bool legacy=mode=="VFP9";
  for(const auto& row:rows) {
   const auto expected=legacy?std::optional<std::int64_t>(row.legacy):row.normal;
   public_case(path,mode,selected,std::string(row.text)+",-1",expected);
   public_case(path,mode,selected,std::string("-1,")+row.text,expected);
  }
  for(int slot=0;slot<26;++slot) {
   std::string args;
   for(int index=0;index<26;++index)args+=(index==0?"":",")+std::string(index==slot?"1.9":"-1");
   public_case(path,mode,selected,args,1);
  }
  public_case(path,mode,selected,"3,6",2);
  public_case(path,mode,selected,"7.9,3.9,1.9",1);
  public_case(path,mode,selected,"-2.9,-3.9",-4);
  public_case(path,mode,selected,"-2147483648,-1",-2147483648LL);
  for(const std::string args:{"0,1E300","-1,0,1E300","1E300,0"})
   public_case(path,mode,selected,args,legacy?std::optional<std::int64_t>(0):std::nullopt);
  public_case(path,mode,selected,"-1,4294967297,3",legacy?std::optional<std::int64_t>(1):std::nullopt);
  // Parent-derived all-operands validation even after intermediate result0.
  for(const std::string args:{"0,EXP(1000)","-1,0,EXP(1000)"})
   public_case(path,mode,selected,args,legacy?std::optional<std::int64_t>(0):std::nullopt);
  for(const std::string text:{"(CAST(9007199254740992 AS INT64)+CAST(1 AS INT64))","CAST(4294967297 AS UINT64)"}) {
   public_case(path,mode,selected,text+",-1",legacy?std::optional<std::int64_t>(1):std::nullopt);
   public_case(path,mode,selected,"-1,"+text,legacy?std::optional<std::int64_t>(1):std::nullopt);
  }
  for(const auto& row:std::vector<std::pair<std::string,std::int64_t>>{{".T.",1},{"'1.9'",2},{"$1.5",2}}) {
   public_case(path,mode,selected,row.first+",-1",row.second);
   public_case(path,mode,selected,"-1,"+row.first,row.second);
  }
 }
 std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_cases();public_cases();
 std::cout<<"BITAND frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

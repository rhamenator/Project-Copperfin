// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-GETBAR-NUMERIC-001: independent native identities plus parent
// safety/first-pass zero fallback, not correlated setter conversion.
struct Row { const char* text; double raw; std::optional<int> ordinary, legacy; };
const std::vector<Row> rows{
 {"-3",-3,{},{}},{"-2.9",-2.9,{},{}},{"-2",-2,{},{}},
 {"-1",-1,{},{}},{"-0.9",-0.9,{},{}},{"0",0,{},{}},
 {"0.5",0.5,{},{}},{"0.9",0.9,{},{}},{"1",1,1,1},
 {"1.4",1.4,1,1},{"1.5",1.5,1,1},{"1.9",1.9,1,1},
 {"2",2,2,2},{"2.9",2.9,2,2},
 {"3",3,3,3},{"3.9",3.9,3,3},{"4",4,4,4},{"4.9",4.9,4,4},{"5.9",5.9,5,5},
 {"2147483647",2147483647.,INT32_MAX,INT32_MAX},
 {"2147483647.9",2147483647.9,INT32_MAX,INT32_MAX},
 {"2147483648",2147483648.,{},{}},{"2147483649",2147483649.,{},{}},
 {"4294967293",4294967293.,{},{}},{"4294967294",4294967294.,{},{}},
 {"4294967295",4294967295.,{},{}},{"4294967296",4294967296.,{},{}},
 {"4294967297",4294967297.,{},1},
 {"4294967298",4294967298.,{},2},{"4294967299",4294967299.,{},3},
 {"-4294967295",-4294967295.,{},1},{"-4294967294",-4294967294.,{},2},
 {"-4294967293",-4294967293.,{},3},
 {"-4294967296",-4294967296.,{},{}},
 {"1E20",1e20,{},{}},{"1E300",1e300,{},{}},{"-1E300",-1e300,{},{}}
};
std::string value(const RuntimePauseState& state,const char* name) {
 const auto found=state.globals.find(name);
 return found==state.globals.end()?"<missing>":format_value(found->second);
}
void direct_boundaries() {
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  for(const auto& row:rows) {
   const auto expected=mode==NumericBehavior::vfp9?row.legacy:row.ordinary;
   expect(checked_getbar_position_argument(make_number_value(row.raw),mode)==expected,"GETBAR direct "+std::string(row.text));
  }
  for(const auto raw:{std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
   expect(!checked_getbar_position_argument(make_number_value(raw),mode),"GETBAR nonfinite both modes");
  expect(!checked_getbar_position_argument(make_number_value(std::nextafter(1.,0.)),mode),"GETBAR below1");
  expect(checked_getbar_position_argument(make_number_value(std::nextafter(2147483648.,0.)),mode)==INT32_MAX,"GETBAR below2^31");
  expect(!checked_getbar_position_argument(make_number_value(std::nextafter(4294967296.,0.)),mode),"GETBAR below2^32 negative alias");
  for(const auto raw:{std::int64_t(1),std::int64_t(INT32_MAX),std::int64_t(4294967297LL),std::int64_t(9007199254740993LL)}) {
   const auto expected=raw<=INT32_MAX?std::optional<std::int64_t>(raw):
     (mode==NumericBehavior::vfp9?std::optional<std::int64_t>(1):std::nullopt);
   expect(checked_getbar_position_argument(make_int64_value(raw),mode)==expected,"GETBAR exact signed precision");
   expect(checked_getbar_position_argument(make_uint64_value(static_cast<std::uint64_t>(raw)),mode)==expected,"GETBAR exact unsigned precision");
  }
  expect(!checked_getbar_position_argument(make_int64_value(std::numeric_limits<std::int64_t>::min()),mode),"GETBAR signed minimum");
  expect(!checked_getbar_position_argument(make_uint64_value(std::numeric_limits<std::uint64_t>::max()),mode),"GETBAR unsigned maximum");
  expect(checked_getbar_position_argument(make_string_value("1.9"),mode)==2,"GETBAR preserved Character half-away");
  expect(checked_getbar_position_argument(make_boolean_value(true),mode)==1,"GETBAR preserved Logical true");
  expect(!checked_getbar_position_argument(make_string_value("1E300"),mode),"GETBAR checked oversized Character");
 }
}
std::string setup(int seed,bool disabled) {
 std::string text="PUBLIC nChoice,nCode,lAfter,nQuery,nCount,cPrompts,cState,cSkip,cMark,nCleanup,cReset\n"
 "nChoice=0\nnCode=0\nlAfter=.F.\nnQuery=0\nnCleanup=-1\ncReset=''\n"
 "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
 "DEFINE POPUP parent RELATIVE\nDEFINE BAR 4 OF parent PROMPT 'apply'\n"
 "DEFINE BAR 13 OF parent PROMPT 'first'\nDEFINE BAR 47 OF parent PROMPT 'second'\n"
 "DEFINE BAR 100 OF parent PROMPT 'finish'\nDEFINE BAR 2147483647 OF parent PROMPT 'cap'\n"
 "ON SELECTION BAR 4 OF parent DO Apply\nON SELECTION BAR 100 OF parent DO Finish\n"
 "ON SELECTION BAR 13 OF parent nChoice=11\nON SELECTION BAR 47 OF parent nChoice=22\n"
 "ON SELECTION BAR 2147483647 OF parent nChoice=33\nSET MARK OF BAR 47 OF parent TO .T.\n";
 for(const int key:{13,47,INT32_MAX})
  text+="SET SKIP OF BAR "+std::to_string(key)+" OF parent "+((key==seed?disabled:!disabled)?".T.":".F.")+"\n";
 return text;
}
std::string callbacks(const std::string& number) {
 return "RETURN\nPROCEDURE Apply\nTRY\nnQuery=GETBAR('parent',"+number+")\nlAfter=.T.\n"
 "CATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\n"
 "nCount=CNTBAR('parent')\ncPrompts=PRMBAR('parent',13)+'|'+PRMBAR('parent',47)+'|'+PRMBAR('parent',2147483647)\n"
 "cSkip=IIF(SKPBAR('parent',13),'.T.','.F.')+'|'+IIF(SKPBAR('parent',47),'.T.','.F.')+'|'+IIF(SKPBAR('parent',2147483647),'.T.','.F.')\n"
 "cMark=IIF(MRKBAR('parent',13),'.T.','.F.')+'|'+IIF(MRKBAR('parent',47),'.T.','.F.')+'|'+IIF(MRKBAR('parent',2147483647),'.T.','.F.')\n"
 "cState=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\nRETURN\nENDPROC\n"
 "PROCEDURE Finish\nDEACTIVATE POPUP parent\nRELEASE POPUP parent\nnCleanup=CNTBAR('parent')\n"
 "USE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
 "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\nENDPROC\n";
}
void run_guard(const std::string& mode,const Row& row,int seed,bool disabled,int selected_session) {
 const auto id=mode=="VFP9"?row.legacy:row.ordinary;
 const std::vector<int> positions{4,13,47,100,INT32_MAX};
 const std::string expected=!id || *id>5?"0":std::to_string(positions[static_cast<std::size_t>(*id-1)]);
 const auto dir=fs::temp_directory_path()/"copperfin_getbar_numeric_5611";
 fs::create_directories(dir);const auto path=dir/"query.prg";
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto prefix=selected_session==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"";
 write_text(path,prefix+"SET NUMERICBEHAVIOR TO "+mode+"\n"+setup(seed,disabled)+"ACTIVATE POPUP parent\n"+callbacks(row.text));
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 auto state=session.run(DebugResumeAction::continue_run);
 const auto tag=mode+" "+row.text+" seed"+std::to_string(seed)+" session"+std::to_string(selected_session)+(disabled?" DISABLED":" ENABLED");
 expect(state.waiting_for_events,"GETBAR guarded wait "+tag+": "+state.message);
 const auto events=[](const RuntimePauseState& s){return std::count_if(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.set_skip";});};
 const auto before=events(state);expect(before==3,"GETBAR independent seed events "+tag);
 const auto marks=[](const RuntimePauseState& s){return std::count_if(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.set_mark";});};
 const auto beforeMarks=marks(state);expect(beforeMarks==1,"GETBAR independent mark seed event "+tag);
 expect(session.dispatch_popup_bar_selection("parent",4),"GETBAR apply callback "+tag);
 state=session.run(DebugResumeAction::continue_run);
 expect(state.waiting_for_events&&value(state,"ncode")=="0"&&value(state,"lafter")=="true","GETBAR preserved zero-fallback continuation "+tag);
 expect(value(state,"nquery")==expected,"GETBAR query identity "+tag+" got "+value(state,"nquery"));
 expect(events(state)==before,"GETBAR no skip mutation/event "+tag);
 expect(marks(state)==beforeMarks,"GETBAR no mark mutation/event "+tag);
 expect(value(state,"ncount")=="5"&&value(state,"cprompts")=="first|second|cap","GETBAR prompts/count guard "+tag);
 expect(value(state,"cstate")==std::to_string(selected_session)+"|"+mode+"|GUARD|1|guard","GETBAR session/cursor guard "+tag);
 expect(value(state,"cmark")==".F.|.T.|.F.","GETBAR independent mark guard "+tag);
 std::string flags;
 for(const auto& [key,marker]:std::vector<std::pair<int,int>>{{13,11},{47,22},{INT32_MAX,33}}) {
  if(!flags.empty())flags+="|";
  flags+=(key==seed?disabled:!disabled)?".T.":".F.";
  for(int repeat=0;repeat<2;++repeat) {
   const bool dispatched=session.dispatch_popup_bar_selection("parent",key);
   expect(dispatched==!(key==seed?disabled:!disabled),"GETBAR preserved host selection "+tag);
   if(dispatched) {state=session.run(DebugResumeAction::continue_run);
    expect(state.waiting_for_events&&value(state,"nchoice")==std::to_string(marker),"GETBAR preserved callback identity "+tag);}
  }
 }
 expect(value(state,"cskip")==flags,"GETBAR known-key skip-state guard "+tag);
 expect(session.dispatch_popup_bar_selection("parent",100),"GETBAR cleanup callback "+tag);
 state=session.run(DebugResumeAction::continue_run);
 expect(value(state,"ncleanup")=="0"&&value(state,"creset")=="1|COPPERFIN|","GETBAR explicit cleanup/reset "+tag);
 // #4630 remains separate: reap only the test-owned callback-return wait.
 session.request_cancel();(void)session.run(DebugResumeAction::continue_run);
 std::error_code ec;fs::remove_all(dir,ec);
}
// Shipped GETBAR positions are not bar identifiers. Native singleton/sparse
// controls independently establish identities without callback/activation state.
void inactive_layouts() {
 const auto dir=fs::temp_directory_path()/"copperfin_getbar_inactive_5611";
 fs::create_directories(dir);const auto path=dir/"inactive.prg";
 for(const std::string mode:{"COPPERFIN","VFP9"})for(const bool sparse:{false,true})for(const auto& row:rows) {
  const auto id=mode=="VFP9"?row.legacy:row.ordinary;
  const std::vector<int> positions=sparse?std::vector<int>{13,47,INT32_MAX}:std::vector<int>{INT32_MAX};
  const auto expected=!id||static_cast<std::size_t>(*id)>positions.size()?"0":
    std::to_string(positions[static_cast<std::size_t>(*id-1)]);
  write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC nResult,nMissing,nCount,nFirst,nLast,nAfter,cMode\n"
   "DEFINE POPUP inactive RELATIVE\n"+
   (sparse?"DEFINE BAR 13 OF inactive PROMPT 'first'\nDEFINE BAR 47 OF inactive PROMPT 'second'\n":"")+
   "DEFINE BAR 2147483647 OF inactive PROMPT 'cap'\n"
   "nResult=GETBAR('INACTIVE',"+row.text+")\nnMissing=GETBAR('absent',"+row.text+")\n"
   "nCount=CNTBAR('inactive')\nnFirst=GETBAR('inactive',1)\nnLast=GETBAR('inactive',CNTBAR('inactive'))\n"
   "cMode=SET('NUMERICBEHAVIOR')\nRELEASE POPUP inactive\nnAfter=CNTBAR('inactive')\nRETURN\n");
  auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
  const auto state=session.run(DebugResumeAction::continue_run);
  const auto tag=mode+" "+row.text+(sparse?" sparse":" singleton");
  expect(state.completed,"GETBAR inactive completion "+tag+": "+state.message);
  expect(value(state,"nresult")==expected,"GETBAR inactive identity "+tag);
  expect(value(state,"nmissing")=="0"&&value(state,"ncount")==(sparse?"3":"1"),"GETBAR absent/count guard "+tag);
  expect(value(state,"nfirst")==(sparse?"13":"2147483647")&&value(state,"nlast")=="2147483647","GETBAR independent endpoints "+tag);
  expect(value(state,"cmode")==mode&&value(state,"nafter")=="0","GETBAR inactive mode/cleanup "+tag);
 }
 std::error_code ec;fs::remove_all(dir,ec);
}
void guarded_cases() {
 for(const std::string mode:{"COPPERFIN","VFP9"})for(bool disabled:{false,true})for(const int seed:{13,47,INT32_MAX}) {
  for(const auto& row:rows)run_guard(mode,row,seed,disabled,1);
  for(const auto& row:rows)if(std::string(row.text)=="4294967297"||std::string(row.text)=="-4294967295")
   run_guard(mode,row,seed,disabled,2);
 }
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_boundaries();guarded_cases();inactive_layouts();return test_failures()==0?0:1;
}

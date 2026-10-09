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
// RQ-CF-PRG-PRMBAR-NUMERIC-001: independent native identities plus parent
// safety/first-pass empty fallback, not correlated setter conversion.
struct Row { const char* text; double raw; std::optional<int> ordinary, legacy; };
const std::vector<Row> rows{
 {"-3",-3,{},{}},{"-2.9",-2.9,{},{}},{"-2",-2,{},{}},
 {"-1",-1,{},{}},{"-0.9",-0.9,{},{}},{"0",0,{},{}},
 {"0.5",0.5,{},{}},{"0.9",0.9,{},{}},{"1",1,1,1},
 {"1.4",1.4,1,1},{"1.5",1.5,1,1},{"1.9",1.9,1,1},
 {"2",2,2,2},{"2.9",2.9,2,2},
 {"2147483647",2147483647.,INT32_MAX,INT32_MAX},
 {"2147483647.9",2147483647.9,INT32_MAX,INT32_MAX},
 {"2147483648",2147483648.,{},{}},{"2147483649",2147483649.,{},{}},
 {"4294967293",4294967293.,{},{}},{"4294967294",4294967294.,{},{}},
 {"4294967295",4294967295.,{},{}},{"4294967296",4294967296.,{},{}},
 {"4294967297",4294967297.,{},1},
 {"-4294967295",-4294967295.,{},1},{"-4294967294",-4294967294.,{},2},
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
   expect(checked_prmbar_number_argument(make_number_value(row.raw),mode)==expected,"PRMBAR direct "+std::string(row.text));
  }
  for(const auto raw:{std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
   expect(!checked_prmbar_number_argument(make_number_value(raw),mode),"PRMBAR nonfinite both modes");
  expect(!checked_prmbar_number_argument(make_number_value(std::nextafter(1.,0.)),mode),"PRMBAR below1");
  expect(checked_prmbar_number_argument(make_number_value(std::nextafter(2147483648.,0.)),mode)==INT32_MAX,"PRMBAR below2^31");
  expect(!checked_prmbar_number_argument(make_number_value(std::nextafter(4294967296.,0.)),mode),"PRMBAR below2^32 negative alias");
  for(const auto raw:{std::int64_t(1),std::int64_t(INT32_MAX),std::int64_t(4294967297LL),std::int64_t(9007199254740993LL)}) {
   const auto expected=raw<=INT32_MAX?std::optional<std::int64_t>(raw):
     (mode==NumericBehavior::vfp9?std::optional<std::int64_t>(1):std::nullopt);
   expect(checked_prmbar_number_argument(make_int64_value(raw),mode)==expected,"PRMBAR exact signed precision");
   expect(checked_prmbar_number_argument(make_uint64_value(static_cast<std::uint64_t>(raw)),mode)==expected,"PRMBAR exact unsigned precision");
  }
  expect(!checked_prmbar_number_argument(make_int64_value(std::numeric_limits<std::int64_t>::min()),mode),"PRMBAR signed minimum");
  expect(!checked_prmbar_number_argument(make_uint64_value(std::numeric_limits<std::uint64_t>::max()),mode),"PRMBAR unsigned maximum");
  expect(checked_prmbar_number_argument(make_string_value("1.9"),mode)==2,"PRMBAR preserved Character half-away");
  expect(checked_prmbar_number_argument(make_boolean_value(true),mode)==1,"PRMBAR preserved Logical true");
  expect(!checked_prmbar_number_argument(make_string_value("1E300"),mode),"PRMBAR checked oversized Character");
 }
}
std::string setup(int seed,bool disabled) {
 std::string text="PUBLIC nChoice,nCode,lAfter,cQuery,nCount,cPrompts,cState,cSkip,cMark,nCleanup,cReset\n"
 "nChoice=0\nnCode=0\nlAfter=.F.\ncQuery=''\nnCleanup=-1\ncReset=''\n"
 "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
 "DEFINE POPUP parent RELATIVE\nDEFINE BAR 1 OF parent PROMPT 'one'\n"
 "DEFINE BAR 2 OF parent PROMPT 'two'\nDEFINE BAR 2147483647 OF parent PROMPT 'cap'\n"
 "DEFINE BAR 4 OF parent PROMPT 'apply'\nDEFINE BAR 100 OF parent PROMPT 'finish'\n"
 "ON SELECTION BAR 4 OF parent DO Apply\nON SELECTION BAR 100 OF parent DO Finish\n"
 "ON SELECTION BAR 1 OF parent nChoice=11\nON SELECTION BAR 2 OF parent nChoice=22\n"
 "ON SELECTION BAR 2147483647 OF parent nChoice=33\nSET MARK OF BAR 2 OF parent TO .T.\n";
 for(const int key:{1,2,INT32_MAX})
  text+="SET SKIP OF BAR "+std::to_string(key)+" OF parent "+((key==seed?disabled:!disabled)?".T.":".F.")+"\n";
 return text;
}
std::string callbacks(const std::string& number) {
 return "RETURN\nPROCEDURE Apply\nTRY\ncQuery=PRMBAR('parent',"+number+")\nlAfter=.T.\n"
 "CATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\n"
 "nCount=CNTBAR('parent')\ncPrompts=PRMBAR('parent',1)+'|'+PRMBAR('parent',2)+'|'+PRMBAR('parent',2147483647)\n"
 "cSkip=IIF(SKPBAR('parent',1),'.T.','.F.')+'|'+IIF(SKPBAR('parent',2),'.T.','.F.')+'|'+IIF(SKPBAR('parent',2147483647),'.T.','.F.')\n"
 "cMark=IIF(MRKBAR('parent',1),'.T.','.F.')+'|'+IIF(MRKBAR('parent',2),'.T.','.F.')+'|'+IIF(MRKBAR('parent',2147483647),'.T.','.F.')\n"
 "cState=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\nRETURN\nENDPROC\n"
 "PROCEDURE Finish\nDEACTIVATE POPUP parent\nRELEASE POPUP parent\nnCleanup=CNTBAR('parent')\n"
 "USE IN guard\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
 "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\nENDPROC\n";
}
void run_guard(const std::string& mode,const Row& row,int seed,bool disabled,int selected_session) {
 const auto id=mode=="VFP9"?row.legacy:row.ordinary;
 const std::string expected=!id?"":(*id==1?"one":(*id==2?"two":(*id==INT32_MAX?"cap":"")));
 const auto dir=fs::temp_directory_path()/"copperfin_prmbar_numeric_5611";
 fs::create_directories(dir);const auto path=dir/"query.prg";
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto prefix=selected_session==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"";
 write_text(path,prefix+"SET NUMERICBEHAVIOR TO "+mode+"\n"+setup(seed,disabled)+"ACTIVATE POPUP parent\n"+callbacks(row.text));
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
 auto state=session.run(DebugResumeAction::continue_run);
 const auto tag=mode+" "+row.text+" seed"+std::to_string(seed)+" session"+std::to_string(selected_session)+(disabled?" DISABLED":" ENABLED");
 expect(state.waiting_for_events,"PRMBAR guarded wait "+tag+": "+state.message);
 const auto events=[](const RuntimePauseState& s){return std::count_if(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.set_skip";});};
 const auto before=events(state);expect(before==3,"PRMBAR independent seed events "+tag);
 const auto marks=[](const RuntimePauseState& s){return std::count_if(s.events.begin(),s.events.end(),[](const auto& e){return e.category=="runtime.set_mark";});};
 const auto beforeMarks=marks(state);expect(beforeMarks==1,"PRMBAR independent mark seed event "+tag);
 expect(session.dispatch_popup_bar_selection("parent",4),"PRMBAR apply callback "+tag);
 state=session.run(DebugResumeAction::continue_run);
 expect(state.waiting_for_events&&value(state,"ncode")=="0"&&value(state,"lafter")=="true","PRMBAR preserved empty-fallback continuation "+tag);
 expect(value(state,"cquery")==expected,"PRMBAR query identity "+tag+" got "+value(state,"cquery"));
 expect(events(state)==before,"PRMBAR no skip mutation/event "+tag);
 expect(marks(state)==beforeMarks,"PRMBAR no mark mutation/event "+tag);
 expect(value(state,"ncount")=="5"&&value(state,"cprompts")=="one|two|cap","PRMBAR prompts/count guard "+tag);
 expect(value(state,"cstate")==std::to_string(selected_session)+"|"+mode+"|GUARD|1|guard","PRMBAR session/cursor guard "+tag);
 expect(value(state,"cmark")==".F.|.T.|.F.","PRMBAR independent mark guard "+tag);
 std::string flags;
 for(const auto& [key,marker]:std::vector<std::pair<int,int>>{{1,11},{2,22},{INT32_MAX,33}}) {
  if(!flags.empty())flags+="|";
  flags+=(key==seed?disabled:!disabled)?".T.":".F.";
  for(int repeat=0;repeat<2;++repeat) {
   const bool dispatched=session.dispatch_popup_bar_selection("parent",key);
   expect(dispatched==!(key==seed?disabled:!disabled),"PRMBAR preserved host selection "+tag);
   if(dispatched) {state=session.run(DebugResumeAction::continue_run);
    expect(state.waiting_for_events&&value(state,"nchoice")==std::to_string(marker),"PRMBAR preserved callback identity "+tag);}
  }
 }
 expect(value(state,"cskip")==flags,"PRMBAR known-key skip-state guard "+tag);
 expect(session.dispatch_popup_bar_selection("parent",100),"PRMBAR cleanup callback "+tag);
 state=session.run(DebugResumeAction::continue_run);
 expect(value(state,"ncleanup")=="0"&&value(state,"creset")=="1|COPPERFIN|","PRMBAR explicit cleanup/reset "+tag);
 // #4630 remains separate: reap only the test-owned callback-return wait.
 session.request_cancel();(void)session.run(DebugResumeAction::continue_run);
 std::error_code ec;fs::remove_all(dir,ec);
}
// Shipped PRMBAR help governs prefix normalization; query an inactive popup
// without state mutation. Numeric conversion must not change this path.
void normalization_cases() {
 const auto dir=fs::temp_directory_path()/"copperfin_prmbar_normalization_5611";
 fs::create_directories(dir);const auto path=dir/"normalization.prg";
 const std::vector<std::pair<std::string,std::string>> prompts{
  {"plain","plain"},{"\\<access","access"},{"\\disabled","disabled"},
  {"\\-",""},{"\\-separator",""},{"a\\<inside","a\\<inside"}};
 for(const std::string mode:{"COPPERFIN","VFP9"})for(const auto& [prompt,expected]:prompts) {
  write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC cResult,cMissing,cCount,cMode\n"
   "DEFINE POPUP inactive RELATIVE\nDEFINE BAR 2 OF inactive PROMPT '"+prompt+"'\n"
   "cResult=PRMBAR('INACTIVE',2.9)\ncMissing=PRMBAR('absent',2.9)\n"
   "cCount=TRANSFORM(CNTBAR('inactive'))\ncMode=SET('NUMERICBEHAVIOR')\nRELEASE POPUP inactive\nRETURN\n");
  auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
  const auto state=session.run(DebugResumeAction::continue_run);
  const auto tag=mode+" "+prompt;
  expect(state.completed,"PRMBAR inactive normalization completion "+tag+": "+state.message);
  expect(value(state,"cresult")==expected,"PRMBAR preserved prefix normalization "+tag);
  expect(value(state,"cmissing").empty()&&value(state,"ccount")=="1","PRMBAR absent/count guard "+tag);
  expect(value(state,"cmode")==mode,"PRMBAR normalization mode guard "+tag);
 }
 std::error_code ec;fs::remove_all(dir,ec);
}
void guarded_cases() {
 for(const std::string mode:{"COPPERFIN","VFP9"})for(bool disabled:{false,true})for(const int seed:{1,2,INT32_MAX}) {
  for(const auto& row:rows)run_guard(mode,row,seed,disabled,1);
  for(const auto& row:{rows[22],rows[23]})run_guard(mode,row,seed,disabled,2);
 }
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_boundaries();guarded_cases();normalization_cases();return test_failures()==0?0:1;
}

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
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-COLLECTION-SELECTOR-NUMERIC-001: independent installed native
// alpha/beta/gamma identities, not correlated expected values from the helper.
struct Row { const char* text; double raw; std::optional<int> normal, legacy; };
const std::vector<Row> rows{
 {"0",0,{},{}},{"0.5",0.5,{},{}},{"0.9",0.9,{},{}},
 {"1",1,1,1},{"1.4",1.4,1,1},{"1.5",1.5,1,1},{"1.9",1.9,1,1},
 {"2",2,2,2},{"2.5",2.5,2,2},{"2.9",2.9,2,2},
 {"3",3,3,3},{"3.5",3.5,3,3},{"3.9",3.9,3,3},{"4",4,4,4},
 {"-0.5",-0.5,{},{}},{"-0.9",-0.9,{},{}},{"-1",-1,{},{}},
 {"-1.5",-1.5,{},{}},{"-2",-2,{},{}},
 {"2147483647",2147483647.,INT32_MAX,INT32_MAX},
 {"2147483648",2147483648.,{},{}},
 {"4294967293",4294967293.,{},{}},{"4294967294",4294967294.,{},{}},
 {"4294967295",4294967295.,{},{}},{"4294967296",4294967296.,{},{}},
 {"4294967297",4294967297.,{},1},{"4294967298",4294967298.,{},2},
 {"4294967299",4294967299.,{},3},
 {"-4294967295",-4294967295.,{},1},{"-4294967294",-4294967294.,{},2},
 {"-4294967293",-4294967293.,{},3},{"-4294967296",-4294967296.,{},{}},
 {"1E20",1e20,{},{}},{"1E300",1e300,{},{}},{"-1E300",-1e300,{},{}}
};
int direct_count=0, public_count=0, readonly_count=0;
void direct_boundaries() {
 const auto check=[](const PrgValue& operand,NumericBehavior mode,std::optional<std::int64_t> expected,const char* tag) {
  ++direct_count;
  expect(checked_collection_selector_argument(operand,mode)==expected,std::string("Collection direct ")+tag);
 };
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  for(const auto& row:rows)check(make_number_value(row.raw),mode,mode==NumericBehavior::vfp9?row.legacy:row.normal,row.text);
  for(const auto raw:{std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
   check(make_number_value(raw),mode,{},"nonfinite");
  check(make_number_value(std::nextafter(1.,0.)),mode,{},"below1");
  check(make_number_value(std::nextafter(2147483648.,0.)),mode,INT32_MAX,"below2^31");
  check(make_number_value(std::nextafter(4294967296.,0.)),mode,{},"below2^32");
  for(const auto raw:{std::int64_t(1),std::int64_t(INT32_MAX),std::int64_t(4294967297LL),std::int64_t(9007199254740993LL)}) {
   const auto expected=raw<=INT32_MAX?std::optional<std::int64_t>(raw):
    (mode==NumericBehavior::vfp9?std::optional<std::int64_t>(1):std::nullopt);
   check(make_int64_value(raw),mode,expected,"exact signed");
   check(make_uint64_value(static_cast<std::uint64_t>(raw)),mode,expected,"exact unsigned");
  }
  check(make_int64_value(-4294967295LL),mode,mode==NumericBehavior::vfp9?std::optional<std::int64_t>(1):std::nullopt,"exact negative alias");
  check(make_int64_value(std::numeric_limits<std::int64_t>::min()),mode,{},"signed minimum");
  check(make_uint64_value(std::numeric_limits<std::uint64_t>::max()),mode,{},"unsigned maximum");
  check(make_currency_value(15000),mode,2,"preserved Currency half-away");
  check(make_currency_value(5000),mode,1,"preserved Currency half-away zero");
  check(make_currency_value(-15000),mode,{},"preserved Currency negative");
  check(make_boolean_value(true),mode,{},"Logical remains key path");
  check(make_string_value("1.9"),mode,{},"Character remains key path");
  check(make_empty_value(),mode,{},"empty remains key path");
 }
}
std::string value(const RuntimePauseState& state,const char* name) {
 const auto it=state.globals.find(name);
 return it==state.globals.end()?"<missing>":format_value(it->second);
}
void identity(const RuntimePauseState& state,const char* name,const std::optional<int>& position,const std::string& tag) {
 const auto it=state.globals.find(name);
 const std::vector<std::string> names{"alpha","beta","gamma"};
 const bool present=position&&*position<=3;
 expect(it!=state.globals.end(),"Collection result exists "+tag+" "+name);
 if(it==state.globals.end())return;
 expect(present ? it->second.kind==PrgValueKind::string&&format_value(it->second)==names[static_cast<std::size_t>(*position-1)] :
  it->second.kind==PrgValueKind::empty,"Collection selected identity "+tag+" "+name+" got "+format_value(it->second));
}
void public_cases() {
 const auto dir=fs::temp_directory_path()/"copperfin_collection_selector_numeric_5611";
 fs::create_directories(dir);const auto path=dir/"query.prg";
 for(const std::string mode:{"COPPERFIN","VFP9"})for(const int selected:{1,2})for(const auto& row:rows) {
  ++public_count;
  const auto position=mode=="VFP9"?row.legacy:row.normal;
  const bool removed=position&&*position<=3;
  const std::string tag=mode+" session"+std::to_string(selected)+" "+row.text;
  const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
  write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"")+
   "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC vExplicit,vBare,vBracket,vMember,vMemberBracket,vRemoved,nCountBefore,nCountAfter,cBefore,cAfter,cKeysBefore,vFirst,vSecond,vThird,cMode,cReset,nCode,lAfter\n"
   "nCode=0\nlAfter=.F.\nCREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
   "oHost=CREATEOBJECT('HostBox')\noItems=oHost.oItems\n"
   "oItems.Add('alpha','first')\noItems.Add('beta','second')\noItems.Add('gamma','third')\n"
   "nCountBefore=oItems.Count\ncBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
   "cKeysBefore=oItems.Item('first')+'|'+oItems.Item('second')+'|'+oItems.Item('third')\nTRY\n"
   "vExplicit=oItems.Item("+row.text+")\nvBare=oItems("+row.text+")\nvBracket=oItems["+row.text+"]\n"
   "vMember=oHost.oItems("+row.text+")\nvMemberBracket=oHost.oItems["+row.text+"]\n"
   "vRemoved=oItems.Remove("+row.text+")\nlAfter=.T.\nCATCH TO oError\nnCode=oError.ErrorNo\nENDTRY\n"
   "nCountAfter=oItems.Count\ncAfter=''\nFOR EACH cItem IN oItems\ncAfter=cAfter+'['+cItem+']'\nENDFOR\n"
   "vFirst=oItems.Item('first')\nvSecond=oItems.Item('second')\nvThird=oItems.Item('third')\n"
   "cMode=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
   "USE IN guard\nRELEASE oItems,oHost\nSET NUMERICBEHAVIOR TO COPPERFIN\nSET DATASESSION TO 1\nSET NUMERICBEHAVIOR TO COPPERFIN\ncReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n"
   "DEFINE CLASS WorkerCollection AS Collection\nENDDEFINE\nDEFINE CLASS HostBox AS Custom\noItems=.NULL.\n"
   "PROCEDURE Init\nTHIS.oItems=CREATEOBJECT('WorkerCollection')\nRETURN\nENDPROC\nENDDEFINE\n");
  auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
  const auto state=session.run(DebugResumeAction::continue_run);
  expect(state.completed,"Collection completion "+tag+": "+state.message);
  expect(value(state,"ncode")=="0"&&value(state,"lafter")=="true","Collection preserved soft-fallback continuation "+tag);
  for(const char* name:{"vexplicit","vbare","vbracket","vmember","vmemberbracket"})identity(state,name,position,tag);
  expect(value(state,"ncountbefore")=="3"&&value(state,"ckeysbefore")=="alpha|beta|gamma","Collection independent setup/key guard "+tag);
  expect(value(state,"vremoved")== (removed?"true":"false"),"Collection selected removal result "+tag);
  expect(value(state,"ncountafter")==(removed?"2":"3"),"Collection selected removal count "+tag);
  std::string survivors;
  const std::vector<std::string> labels{"alpha","beta","gamma"};
  const std::vector<const char*> keys{"vfirst","vsecond","vthird"};
  for(int i=1;i<=3;++i) {
   const bool gone=removed&&*position==i;
   if(!gone)survivors+="["+labels[static_cast<std::size_t>(i-1)]+"]";
   identity(state,keys[static_cast<std::size_t>(i-1)],gone?std::nullopt:std::optional<int>(i),tag+" key-survivor");
  }
  expect(value(state,"cafter")==survivors,"Collection selected removal survivor order "+tag);
  const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
  expect(value(state,"cbefore")==context&&value(state,"cmode")==context,"Collection selected-session/cursor guard "+tag);
  expect(value(state,"creset")=="1|COPPERFIN|","Collection cleanup/reset "+tag);
 }
 std::error_code ignored;fs::remove_all(dir,ignored);
}
void hidden_collection_cases() {
 const auto dir=fs::temp_directory_path()/"copperfin_collection_readonly_numeric_5611";
 fs::create_directories(dir);const auto path=dir/"hidden.prg";
 for(const std::string mode:{"COPPERFIN","VFP9"})for(const auto& row:rows) {
  const auto position=mode=="VFP9"?row.legacy:row.normal;
  if(!position||*position>3)continue; // Invalid member chains are #7104, not this slice.
  ++readonly_count;
  const auto tag=mode+" "+row.text;
  const std::vector<std::string> labels{"alpha","beta","gamma"};
  write_text(path,"SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC cSelected,cBracket,lRemoved,nBefore,nAfter,cKeys\n"
   "oPanel=CREATEOBJECT('Panel')\noHidden=oPanel.Controls\nnBefore=oHidden.Count\n"
   "cSelected=oPanel.Controls("+row.text+").Caption\ncBracket=oPanel.Controls["+row.text+"].Caption\n"
   "lRemoved=oHidden.Remove("+row.text+")\nnAfter=oHidden.Count\n"
   "cKeys=oPanel.Controls(1).Caption+'|'+oPanel.Controls(2).Caption+'|'+oPanel.Controls(3).Caption\n"
   "RELEASE oHidden,oPanel\nSET NUMERICBEHAVIOR TO COPPERFIN\nRETURN\n"
   "DEFINE CLASS FirstButton AS CommandButton\nCaption='alpha'\nENDDEFINE\n"
   "DEFINE CLASS SecondButton AS CommandButton\nCaption='beta'\nENDDEFINE\n"
   "DEFINE CLASS ThirdButton AS CommandButton\nCaption='gamma'\nENDDEFINE\n"
   "DEFINE CLASS Panel AS Container\nADD OBJECT first AS FirstButton\nADD OBJECT second AS SecondButton\nADD OBJECT third AS ThirdButton\nENDDEFINE\n");
  auto session=PrgRuntimeSession::create(make_runtime_session_options(path,dir,false));
  const auto state=session.run(DebugResumeAction::continue_run);
  expect(state.completed,"Collection hidden completion "+tag+": "+state.message);
  const auto expected=labels[static_cast<std::size_t>(*position-1)];
  expect(value(state,"cselected")==expected&&value(state,"cbracket")==expected,"Collection hidden selected identity "+tag);
  expect(value(state,"lremoved")=="false"&&value(state,"nbefore")=="3"&&value(state,"nafter")=="3"&&value(state,"ckeys")=="alpha|beta|gamma","Collection hidden read-only guard "+tag);
 }
 std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 direct_boundaries();public_cases();hidden_collection_cases();
 std::cout<<"Collection frozen cases: direct="<<direct_count<<" public="<<public_count<<" hidden="<<readonly_count<<'\n';
 return test_failures()==0?0:1;
}

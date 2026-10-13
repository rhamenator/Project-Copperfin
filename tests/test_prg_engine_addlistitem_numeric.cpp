// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <system_error>
#include <vector>
namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs=std::filesystem;
using ID=std::optional<std::int32_t>;
// RQ-CF-PRG-ADDLISTITEM-ID-NUMERIC-001 / VR-5611-ADDLISTITEM-ID-NUMERIC-001/002.
// Independent AddListItem native/boundary source literals, NOT sibling/helper output.
// Default/exact/NaN/binary boundaries derive from the named numeric parent.
// Existing-ID native caption replacement #7173 and successful shared state/return excluded.
struct NumberCase {const char* source;double value;ID modern,legacy;};
const std::vector<NumberCase> numbers{
 {"0",0,{},{}},{"-0.9",-.9,{},{}},{"0.9",.9,{},{}},
 {"1",1,1,1},{"1.4",1.4,1,1},{"1.5",1.5,1,1},{"1.9",1.9,1,1},
 {"2",2,2,2},{"2.4",2.4,2,2},{"2.5",2.5,2,2},{"2.9",2.9,2,2},
 {"3",3,3,3},{"3.9",3.9,3,3},{"4",4,4,4},{"4.9",4.9,4,4},
 {"11",11,11,11},{"11.9",11.9,11,11},{"22.9",22.9,22,22},{"33.9",33.9,33,33},
 {"34",34,34,34},{"-1",-1,{},{}},{"-1.9",-1.9,{},{}},
 {"32766",32766,32766,32766},{"32766.9",32766.9,32766,32766},
 {"32767",32767,32767,32767},{"32767.1",32767.1,32767,32767},
 {"32767.9",32767.9,32767,32767},{"32768",32768,{},{}},
 {"32768.9",32768.9,{},{}},{"32769",32769,{},{}},
 {"65535",65535,{},{}},{"65537",65537,{},{}},
 {"2147483647",2147483647,{},{}},{"2147483648",2147483648,{},{}},
 {"4294967295",4294967295,{},{}},{"4294967296",4294967296,{},{}},
 {"4294967297",4294967297,{},1},{"4294967298.9",4294967298.9,{},2},
 {"4294967299",4294967299,{},3},{"4294967307.9",4294967307.9,{},11},
 {"-4294967295.9",-4294967295.9,{},1},{"-4294967294.5",-4294967294.5,{},2},
 {"-4294967293",-4294967293,{},3},{"-4294967285.5",-4294967285.5,{},11},
 {"4295000063.9",4295000063.9,{},32767},{"-4294934529.9",-4294934529.9,{},32767},
 {"4295000064",4295000064,{},{}},{"-4294934528",-4294934528,{},{}},
 {"9007199254740991",9007199254740991,{},{}},
 {"-9007199254740991",-9007199254740991,{},1},
 {"9007199254740992",9007199254740992,{},{}},
 {"1E300",1E300,{},{}},{"-1E300",-1E300,{},{}},
 {"1E-300",1E-300,{},{}},{"-1E-300",-1E-300,{},{}},
 {"1-2^(-52)",1.-std::ldexp(1.,-52),{},{}},
 {"1-2^(-53)",1.-std::ldexp(1.,-53),{},{}},
 {"32767.999",32767.999,32767,32767}
};
struct ExactCase {const char* source;ID modern,legacy;};
const std::vector<ExactCase> exacts{
 {"CAST(0 AS INT64)",{},{}},{"CAST(1 AS INT64)",1,1},
 {"CAST(4 AS INT64)",4,4},{"CAST(11 AS INT64)",11,11},
 {"CAST(32767 AS INT64)",32767,32767},{"CAST(32768 AS INT64)",{},{}},
 {"CAST(4294967307 AS INT64)",{},11},{"CAST(-4294967285 AS INT64)",{},11},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(34 AS INT64))",{},34},
 {"CAST(0 AS UINT64)",{},{}},{"CAST(11 AS UINT64)",11,11},
 {"CAST(4295000063 AS UINT64)",{},32767},
 {"(CAST(9223372036854775808 AS UINT64)+CAST(34 AS UINT64))",{},34},
 {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{},{}}
};
int direct_count=0,public_count=0;
void direct_cases(){
 expect(numbers.size()==58&&exacts.size()==14,"AddListItem frozen table counts");
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}){
  const auto check=[&](const PrgValue& value,ID modern,ID legacy,const char* label){
   ++direct_count;
   expect(checked_addlistitem_id_argument(value,mode)==(mode==NumericBehavior::vfp9?legacy:modern),
    std::string("AddListItem direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+label);
  };
  for(const auto& r:numbers)check(make_number_value(r.value),r.modern,r.legacy,r.source);
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},{},"derived NaN");
  check(make_number_value(INFINITY),{},{},"positive infinity");
  check(make_number_value(-INFINITY),{},{},"negative infinity");
  check(make_number_value(std::nextafter(1.,0.)),{},{},"binary belowone");
  check(make_number_value(std::nextafter(1.,2.)),1,1,"binary aboveone");
  check(make_number_value(std::nextafter(2.,0.)),1,1,"binary belowtwo");
  check(make_number_value(std::nextafter(32768.,0.)),32767,32767,"upper inner");
  check(make_number_value(std::nextafter(32768.,INFINITY)),{},{},"upper outer");
  check(make_number_value(std::numeric_limits<double>::denorm_min()),{},{},"positive subnormal");
  check(make_number_value(-std::numeric_limits<double>::denorm_min()),{},{},"negative subnormal");
  struct S{std::int64_t value;ID modern,legacy;};
  for(const auto& r:std::vector<S>{
   {0,{},{}},{1,1,1},{4,4,4},{11,11,11},{34,34,34},{32767,32767,32767},
   {32768,{},{}},{4294967307LL,{},11},{-4294967285LL,{},11},
   {INT64_MIN,{},{}},{INT64_MIN+1,{},1},{INT64_MIN+34,{},34},
   {INT64_MIN+32767,{},32767},{INT64_MIN+32768,{},{}},{INT64_MAX,{},{}}})
   check(make_int64_value(r.value),r.modern,r.legacy,"derived exact signed64");
  struct U{std::uint64_t value;ID modern,legacy;};
  for(const auto& r:std::vector<U>{
   {0,{},{}},{1,1,1},{4,4,4},{34,34,34},{32767,32767,32767},{32768,{},{}},
   {4294967307ULL,{},11},{9223372036854775842ULL,{},34},
   {UINT64_MAX,{},{}},{UINT64_MAX-4294967261ULL,{},34},
   {UINT64_MAX-4294934528ULL,{},32767}})
   check(make_uint64_value(r.value),r.modern,r.legacy,"derived exact unsigned64");
  auto typed_null=make_number_value(1.9);typed_null.is_null=true;
  check(typed_null,{},{},"typed Numeric NULL");
  check(make_null_value(),{},{},"NULL");
  check(make_string_value("1"),{},{},"Character");
  check(make_boolean_value(true),{},{},"Logical");
  check(make_currency_value(19000),{},{},"Currency");
 }
}
std::string get(const RuntimePauseState& state,const char* key){
 const auto it=state.globals.find(key);
 return it==state.globals.end()?"<missing>":format_value(it->second);
}
void invoke_case(const fs::path& path,const std::string& mode,int selected,
 const std::string& cls,int shape,const std::string& source,ID id,
 bool multi=false,const std::string& config={},const std::string& message={}){
 ++public_count;
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const int before=shape==0?0:shape==3?1:3;
 const std::vector<int> old_ids=shape==0?std::vector<int>{}:shape==3?std::vector<int>{32767}:
  shape==1?std::vector<int>{1,2,3}:std::vector<int>{11,22,33};
 bool duplicate=false;for(int old:old_ids)if(id&&old==*id)duplicate=true;
 const bool append=id.has_value()&&!duplicate;
 const std::string tag=mode+" session"+std::to_string(selected)+" "+cls+" shape"+
  std::to_string(shape)+" "+config+" "+source;
 std::string setup;
 for(std::size_t i=0;i<old_ids.size();++i){
  const auto row=std::to_string(i+1);
  const std::string caption=i==0?"first":i==1?"second":"third";
  setup+="o.AddListItem('"+caption+"',"+std::to_string(old_ids[i])+")\n";
  setup+="o.List("+row+",2)='cell"+row+"'\no.ItemData("+row+")="+std::to_string((i+1)*101)+"\n";
 }
 if(before)setup+="o.ListIndex="+std::string(before==1?"1":"2")+"\n";
 if(cls=="ListBox"&&before)setup+="o.TopIndex="+std::string(before==1?"1":"2")+"\n";
 if(multi)setup+="o.MultiSelect=.T.\no.Selected(1)=.T.\no.Selected(2)=.T.\n";
 const std::string snapshot="TRANSFORM(o.ListCount)+'|'+TRANSFORM(o.ListIndex)+'|'+TRANSFORM(o.ListItemID)+'|'+TRANSFORM(o.Value)+'|'+TRANSFORM(o.DisplayValue)+'|'+TRANSFORM(o.NewIndex)+'|'+TRANSFORM(o.NewItemID)"+
  (cls=="ListBox"?std::string("+'|'+TRANSFORM(o.TopIndex)+'|'+TRANSFORM(o.TopItemID)"):"");
 write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC o,nCode,cMessage,lReturned,lAfter,cBefore,cAfter,cContextBefore,cContextAfter,cOther,cReset,nBefore,nGuard\n"
  "nCode=0\ncMessage=''\nlReturned=.F.\nlAfter=.F.\nCREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "o=CREATEOBJECT('"+cls+"')\no.ColumnCount=2\n"+config+setup+
  "nBefore=o.ListCount\ncBefore="+snapshot+"\n"
  "cContextBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\no.AddListItem('candidate',"+source+")\nlReturned=.T.\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\nlAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\ncAfter="+snapshot+"\n"
  "cContextAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\ncReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"AddListItem completion "+tag+": "+state.message);
 expect(get(state,"nbefore")==std::to_string(before),"AddListItem populated precondition "+tag);
 expect(get(state,"ncode")==std::string(id?"0":"11"),"AddListItem admission error "+tag+" got "+get(state,"ncode"));
 expect(get(state,"lreturned")==std::string(id?"true":"false"),"AddListItem invocation continuation "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","AddListItem catchability "+tag);
 if(!message.empty())expect(get(state,"cmessage")==message,"AddListItem localized11 "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"ccontextbefore")==context&&get(state,"ccontextafter")==context,"AddListItem selected-session/workarea "+tag);
 expect(get(state,"cother")==(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|","AddListItem session isolation/reset "+tag);
 if(!id)expect(get(state,"cbefore")==get(state,"cafter"),"AddListItem complete rejection property state "+tag);
 bool seen=false;
 for(const auto& object:state.ole_objects)if(copperfin::runtime::lowercase_copy(object.base_class_name)==copperfin::runtime::lowercase_copy(cls)){
  seen=true;
  const auto count=static_cast<std::size_t>(before+(append?1:0));
  expect(object.list_rows.size()==count&&object.collection_items.size()==count&&object.list_item_data.size()==count,
   "AddListItem coherent materialized count "+tag);
  std::vector<std::string> keys;for(int old:old_ids)keys.push_back(std::to_string(old));
  if(append)keys.push_back(std::to_string(*id));
  expect(object.collection_item_keys==keys,"AddListItem independently expected stable IDs "+tag);
  if(object.list_rows.size()>=old_ids.size()&&object.list_item_data.size()>=old_ids.size()){
   for(std::size_t i=0;i<old_ids.size();++i){
    const auto caption=i==0?"first":i==1?"second":"third";
    expect(object.list_rows[i].size()>=2&&format_value(object.list_rows[i][0])==caption&&
     format_value(object.list_rows[i][1])=="cell"+std::to_string(i+1),"AddListItem retained two-column cells "+tag);
    expect(format_value(object.list_item_data[i])==std::to_string((i+1)*101),"AddListItem retained payload "+tag);
   }
  }
  if(append&&object.list_rows.size()==count){
   expect(!object.list_rows.back().empty()&&format_value(object.list_rows.back()[0])=="candidate",
    "AddListItem new row caption "+tag);
  }
  if(!id){
   const auto bits=shape==0?std::vector<bool>{}:shape==3?std::vector<bool>{true}:
    multi?std::vector<bool>{true,true,false}:std::vector<bool>{false,true,false};
   expect(object.list_selected==bits,"AddListItem rejected selection retained "+tag);
  }
  // Duplicate no-op is deliberate current-branch preservation, NOT native parity (#7173).
  // Do not assert successful focus/text/top/return (#7163), lifecycle or other methods.
 }
 expect(seen,"AddListItem captured native control "+tag);
 for(const auto& event:state.events)expect(event.category!="ole.invoke","AddListItem no external invocation "+tag);
}
void public_cases(const std::string& mode,int selected){
 const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
 const auto dir=fs::temp_directory_path()/("copperfin_addlistitem_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
 fs::create_directories(dir);const auto path=dir/"query.prg";const bool legacy=mode=="VFP9";
 for(const std::string cls:{"ListBox","ComboBox"})for(int shape=0;shape<4;++shape){
  for(const auto& r:numbers)invoke_case(path,mode,selected,cls,shape,r.source,legacy?r.legacy:r.modern);
  for(const auto& r:exacts)invoke_case(path,mode,selected,cls,shape,r.source,legacy?r.legacy:r.modern);
  invoke_case(path,mode,selected,cls,shape,"(EXP(1000)-EXP(1000))",{});
 }
 for(const auto& r:numbers)invoke_case(path,mode,selected,"ListBox",2,r.source,legacy?r.legacy:r.modern,true);
 // Native configure-before-populate and named-parent exact typed-zero extension.
 const std::vector<NumberCase> guards{
  {"1.9",1.9,1,1},{"2.5",2.5,2,2},{"11.9",11.9,11,11},
  {"0",0,{},{}},{"32768",32768,{},{}},{"4294967307.9",4294967307.9,{},11}};
 for(const std::string cls:{"ListBox","ComboBox"})
 for(const std::string rst:{"","o.RowSourceType=0\n","o.RowSourceType=CAST(0 AS INT64)\n","o.RowSourceType=CAST(0 AS UINT64)\n"})
 for(const std::string binding:{"o.ControlSource=''\n","o.ControlSource='   '\n","o.ControlSource=CHR(9)+'  '\n"})
 for(const auto& r:guards)invoke_case(path,mode,selected,cls,2,r.source,legacy?r.legacy:r.modern,false,rst+binding);
 for(const auto& locale:std::vector<std::vector<std::string>>{
  {"en-US","Function argument value, type, or count is invalid."},
  {"es-419","El valor, tipo o numero de un argumento de funcion no es valido."},
  {"pt-BR","O valor, tipo ou numero de um argumento de funcao e invalido."},
  {"qps-ploc","[!! Füñçţïøñ årĝümëñţ vålüë, ţÿþë, ør çøüñţ ïš ïñvålïð. !!]"}}){
  set_env_value("COPPERFIN_LOCALE",locale[0],true);
  for(const std::string cls:{"ListBox","ComboBox"}){
   invoke_case(path,mode,selected,cls,2,"0",{},false,{},locale[1]);
   invoke_case(path,mode,selected,cls,2,"32768",{},false,{},locale[1]);
   invoke_case(path,mode,selected,cls,2,"4",4);
  }
 }
 set_env_value("COPPERFIN_LOCALE","en-US",true);
 std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv){
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 if(argc==1){direct_cases();expect(direct_count==198&&public_count==0,"AddListItem complete direct count");}
 else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
  (std::string(argv[2])=="1"||std::string(argv[2])=="2")){
  public_cases(argv[1],std::string(argv[2])=="1"?1:2);
  expect(direct_count==0&&public_count==810,"AddListItem complete public count");
 }else{std::cerr<<"Expected no args or COPPERFIN|VFP9 1|2\n";return 2;}
 std::cout<<"AddListItem frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

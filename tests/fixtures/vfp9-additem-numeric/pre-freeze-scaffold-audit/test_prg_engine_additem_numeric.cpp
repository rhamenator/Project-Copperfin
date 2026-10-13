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
using ID=std::optional<std::size_t>;
// RQ-CF-PRG-ADDITEM-POSITION-NUMERIC-001 / VR-5611-ADDITEM-POSITION-NUMERIC-001/002.
// Independent AddItem native/boundary/count-alias input and observation literals.
// A missing position means rejection at sampled counts 0,1,3,10,33 ONLY:
// NOT a global capacity limit or AddListItem's stable-ID domain.
// Exact64/NaN/host safeguards derive from RQ-CF-PRG-NUMERIC-BEHAVIOR-001.
// Exclude native decimal-nearone parser boundary and generated-ID gap #7176.
// Successful shared state/return and fallback behavior are not parity assertions.
struct NumberCase {const char* source;double value;ID modern,legacy;};
const std::vector<NumberCase> numbers{
 {"0",0,{},{}},
 {"-0.9",-0.9,{},{}},
 {"0.9",0.9,{},{}},
 {"1",1,1,1},
 {"1.4",1.4,1,1},
 {"1.5",1.5,1,1},
 {"1.9",1.9,1,1},
 {"2",2,2,2},
 {"2.4",2.4,2,2},
 {"2.5",2.5,2,2},
 {"2.9",2.9,2,2},
 {"3",3,3,3},
 {"3.9",3.9,3,3},
 {"4",4,4,4},
 {"4.9",4.9,4,4},
 {"11",11,11,11},
 {"11.9",11.9,11,11},
 {"22.9",22.9,22,22},
 {"33.9",33.9,33,33},
 {"34",34,34,34},
 {"-1",-1,{},{}},
 {"-1.9",-1.9,{},{}},
 {"32766",32766,{},{}},
 {"32766.9",32766.9,{},{}},
 {"32767",32767,{},{}},
 {"32767.1",32767.1,{},{}},
 {"32767.9",32767.9,{},{}},
 {"32768",32768,{},{}},
 {"32768.9",32768.9,{},{}},
 {"32769",32769,{},{}},
 {"65535",65535,{},{}},
 {"65537",65537,{},{}},
 {"2147483647",2147483647,{},{}},
 {"2147483648",2147483648,{},{}},
 {"4294967295",4294967295,{},{}},
 {"4294967296",4294967296,{},{}},
 {"4294967297",4294967297,{},1},
 {"4294967298.9",4294967298.9,{},2},
 {"4294967299",4294967299,{},3},
 {"4294967307.9",4294967307.9,{},11},
 {"-4294967295.9",-4294967295.9,{},1},
 {"-4294967294.5",-4294967294.5,{},2},
 {"-4294967293",-4294967293,{},3},
 {"-4294967285.5",-4294967285.5,{},11},
 {"4295000063.9",4295000063.9,{},{}},
 {"-4294934529.9",-4294934529.9,{},{}},
 {"4295000064",4295000064,{},{}},
 {"-4294934528",-4294934528,{},{}},
 {"9007199254740991",9007199254740991,{},{}},
 {"-9007199254740991",-9007199254740991,{},1},
 {"9007199254740992",9007199254740992,{},{}},
 {"1E300",1E300,{},{}},
 {"-1E300",-1E300,{},{}},
 {"1E-300",1E-300,{},{}},
 {"-1E-300",-1E-300,{},{}},
 {"4294967330.9",4294967330.9,{},34},
 {"-4294967262.9",-4294967262.9,{},34},
 {"1-2^(-52)",1.-std::ldexp(1.,-52),{},{}},
 {"1-2^(-53)",1.-std::ldexp(1.,-53),{},{}},
 {"1+2^(-52)",1.+std::ldexp(1.,-52),1,1},
 {"2-2^(-52)",2.-std::ldexp(1.,-52),1,1},
 {"2.0000000000000004",2.0000000000000004,2,2},
 {"3.9999999999999996",3.9999999999999996,3,3},
 {"4-2^(-51)",4.-std::ldexp(1.,-51),3,3},
 {"4.999999999999999",4.999999999999999,4,4},
 {"5-2^(-50)",5.-std::ldexp(1.,-50),4,4},
 {"5",5,5,5},
 {"5.000000000000001",5.000000000000001,5,5},
 {"4294967297.9",4294967297.9,{},1},
 {"4294967299.9",4294967299.9,{},3},
 {"4294967300.9",4294967300.9,{},4},
 {"4294967301",4294967301,{},5},
 {"-4294967294.9",-4294967294.9,{},2},
 {"-4294967293.9",-4294967293.9,{},3},
 {"-4294967292.9",-4294967292.9,{},4},
 {"-4294967291.9",-4294967291.9,{},5},
 {"8589934593",8589934593,{},1},
 {"-8589934591.9",-8589934591.9,{},1},
 {"1E20",1E20,{},{}},
 {"-1E20",-1E20,{},{}},
 {"(1E300*1E300)",INFINITY,{},{}},
 {"(-1E300*1E300)",-INFINITY,{},{}}
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
ID admitted(ID position,std::size_t count){
 return position&&*position<=count+1?position:ID{};
}
int direct_count=0,public_count=0;
void direct_cases(){
 expect(numbers.size()==82&&exacts.size()==14,"AddItem frozen table counts");
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}){
  for(const std::size_t count:{0U,1U,3U,10U,33U}){
  const auto check=[&](const PrgValue& value,ID modern,ID legacy,const char* label){
   ++direct_count;
   expect(checked_additem_position_argument(value,mode,count)==admitted(mode==NumericBehavior::vfp9?legacy:modern,count),
    std::string("AddItem direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+"count"+std::to_string(count)+" "+label);
  };
  for(const auto& r:numbers)check(make_number_value(r.value),r.modern,r.legacy,r.source);
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},{},"derived NaN");
  check(make_number_value(INFINITY),{},{},"positive infinity");
  check(make_number_value(-INFINITY),{},{},"negative infinity");
  check(make_number_value(std::nextafter(1.,0.)),{},{},"binary belowone");
  check(make_number_value(std::nextafter(1.,2.)),1,1,"binary aboveone");
  check(make_number_value(std::nextafter(2.,0.)),1,1,"binary belowtwo");
  check(make_number_value(std::nextafter(4.,0.)),3,3,"binary belowfour");
  check(make_number_value(std::nextafter(5.,0.)),4,4,"binary belowfive");
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
  const auto max=std::numeric_limits<std::size_t>::max();
  const auto host_check=[&](const PrgValue& value,std::size_t count,ID wanted,const char* label){
   ++direct_count;expect(checked_additem_position_argument(value,mode,count)==wanted,label);
  };
  host_check(make_number_value(1.),max,{},"AddItem count+1 overflow containment");
  host_check(make_uint64_value(max),max-1,mode==NumericBehavior::copperfin?ID{max}:ID{},
   "AddItem exact host unsigned upper without double conversion");
  const double exclusive=std::ldexp(1.,std::numeric_limits<std::size_t>::digits);
  host_check(make_number_value(exclusive),max-1,{},"AddItem exclusive host limit rejected before cast");
  const double inner=std::nextafter(exclusive,0.);
  host_check(make_number_value(inner),max-1,
   mode==NumericBehavior::copperfin?ID{static_cast<std::size_t>(inner)}:ID{},
   "AddItem inner representable host Numeric boundary");
  const std::uint64_t exact_inner=9007199254740993ULL;
  host_check(make_uint64_value(exact_inner),max-1,
   mode==NumericBehavior::vfp9?ID{1}:
    exact_inner<=max?ID{static_cast<std::size_t>(exact_inner)}:ID{},
   "AddItem unsigned above 2^53 exactness or defined low32");
  host_check(make_int64_value(INT64_MAX),max-1,
   mode==NumericBehavior::copperfin&&static_cast<std::uint64_t>(INT64_MAX)<=max?
    ID{static_cast<std::size_t>(INT64_MAX)}:ID{},
   "AddItem exact signed host boundary without double conversion");
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
 const int before=shape==0?0:shape==3?1:shape==4?10:shape==5?33:3;
 id=admitted(id,static_cast<std::size_t>(before));
 std::vector<int> old_ids;
 for(int row=1;row<=before;++row)old_ids.push_back(shape==3?32767:shape>=4?row*3:shape==1?row:row*11);
 const bool append=id.has_value();
 const std::string tag=mode+" session"+std::to_string(selected)+" "+cls+" shape"+
  std::to_string(shape)+" "+config+" "+source;
 std::string setup;
 for(std::size_t i=0;i<old_ids.size();++i){
  const auto row=std::to_string(i+1);
  const std::string caption=i==0?"first":i==1?"second":i==2?"third":"seed"+row;
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
 expect(state.completed,"AddItem completion "+tag+": "+state.message);
 expect(get(state,"nbefore")==std::to_string(before),"AddItem populated precondition "+tag);
 expect(get(state,"ncode")==std::string(id?"0":"11"),"AddItem admission error "+tag+" got "+get(state,"ncode"));
 expect(get(state,"lreturned")==std::string(id?"true":"false"),"AddItem invocation continuation "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","AddItem catchability "+tag);
 if(!message.empty())expect(get(state,"cmessage")==message,"AddItem localized11 "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"ccontextbefore")==context&&get(state,"ccontextafter")==context,"AddItem selected-session/workarea "+tag);
 expect(get(state,"cother")==(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|","AddItem session isolation/reset "+tag);
 if(!id)expect(get(state,"cbefore")==get(state,"cafter"),"AddItem complete rejection property state "+tag);
 bool seen=false;
 for(const auto& object:state.ole_objects)if(copperfin::runtime::lowercase_copy(object.base_class_name)==copperfin::runtime::lowercase_copy(cls)){
  seen=true;
  const auto count=static_cast<std::size_t>(before+(append?1:0));
  expect(object.list_rows.size()==count&&object.collection_items.size()==count&&object.list_item_data.size()==count,
   "AddItem coherent materialized count "+tag);
  expect(object.collection_item_keys.size()==count,"AddItem coherent stable-ID storage "+tag);
  const auto slot=id?*id-1:count;
  if(object.list_rows.size()==count&&object.list_item_data.size()==count&&object.collection_item_keys.size()==count){
   for(std::size_t i=0;i<old_ids.size();++i){
    const auto at=i+(id&&i>=slot?1U:0U);
    const auto row=std::to_string(i+1);
    const auto caption=i==0?std::string("first"):i==1?std::string("second"):i==2?std::string("third"):"seed"+row;
    expect(object.collection_item_keys[at]==std::to_string(old_ids[i]),"AddItem retained prior stable ID "+tag);
    expect(object.list_rows[at].size()>=2&&format_value(object.list_rows[at][0])==caption&&
     format_value(object.list_rows[at][1])=="cell"+row,"AddItem retained two-column cells "+tag);
    expect(format_value(object.list_item_data[at])==std::to_string((i+1)*101),"AddItem retained payload "+tag);
   }
   if(id)expect(!object.list_rows[slot].empty()&&format_value(object.list_rows[slot][0])=="candidate",
    "AddItem independently expected insertion position "+tag);
  }
  if(!id){
   std::vector<bool> bits(static_cast<std::size_t>(before),false);
   if(before)bits[before==1?0:1]=true;
   if(multi&&before)bits[0]=true;
   expect(object.list_selected==bits,"AddItem rejected selection retained "+tag);
  }
  // Do NOT assert new generated ID against native (#7176) or successful
  // focus/text/top/return, selection shifts, sorting, lifecycle or other methods.
 }
 expect(seen,"AddItem captured native control "+tag);
 for(const auto& event:state.events)expect(event.category!="ole.invoke","AddItem no external invocation "+tag);
}
void public_cases(const std::string& mode,int selected){
 const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
 const auto dir=fs::temp_directory_path()/("copperfin_additem_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
 fs::create_directories(dir);const auto path=dir/"query.prg";const bool legacy=mode=="VFP9";
 for(const std::string cls:{"ListBox","ComboBox"})for(int shape=0;shape<4;++shape){
  for(const auto& r:numbers)invoke_case(path,mode,selected,cls,shape,r.source,legacy?r.legacy:r.modern);
  for(const auto& r:exacts)invoke_case(path,mode,selected,cls,shape,r.source,legacy?r.legacy:r.modern);
  invoke_case(path,mode,selected,cls,shape,"(EXP(1000)-EXP(1000))",{});
 }
 for(const auto& r:numbers)invoke_case(path,mode,selected,"ListBox",2,r.source,legacy?r.legacy:r.modern,true);
 // Count10/33 independently observed position11/34 and BOTH-sign aliases.
 const std::vector<NumberCase> count_guards{
  {"0",0,{},{}},{"1.9",1.9,1,1},{"4.9",4.9,4,4},{"11.9",11.9,11,11},
  {"33.9",33.9,33,33},{"34",34,34,34},
  {"4294967307.9",4294967307.9,{},11},{"-4294967285.5",-4294967285.5,{},11},
  {"4294967330.9",4294967330.9,{},34},{"-4294967262.9",-4294967262.9,{},34},
  {"1E300",1E300,{},{}},{"(1E300*1E300)",INFINITY,{},{}}};
 for(const std::string cls:{"ListBox","ComboBox"})for(int shape:{4,5}){
  for(const auto& r:count_guards)invoke_case(path,mode,selected,cls,shape,r.source,legacy?r.legacy:r.modern);
  for(const auto& r:std::vector<ExactCase>{
   {"CAST(11 AS INT64)",11,11},{"CAST(4294967307 AS INT64)",{},11},
   {"(CAST(-9223372036854775808 AS INT64)+CAST(34 AS INT64))",{},34},
   {"(CAST(9223372036854775808 AS UINT64)+CAST(34 AS UINT64))",{},34}})
   invoke_case(path,mode,selected,cls,shape,r.source,legacy?r.legacy:r.modern);
 }
 // Configure BEFORE population. Exact typed-zero is named-parent derived.
 const std::vector<NumberCase> guards{
  {"1.9",1.9,1,1},{"2.5",2.5,2,2},{"4.9",4.9,4,4},
  {"0",0,{},{}},{"5",5,5,5},{"4294967300.9",4294967300.9,{},4}};
 for(const std::string cls:{"ListBox","ComboBox"})
 for(const std::string rst:{"","o.RowSourceType=0\n","o.RowSourceType=CAST(0 AS INT64)\n","o.RowSourceType=CAST(0 AS UINT64)\n"})
 for(const std::string binding:{"o.ControlSource=''\n","o.ControlSource='   '\n","o.ControlSource=CHR(9)+'  '\n"})
 for(const auto& r:guards)invoke_case(path,mode,selected,cls,2,r.source,legacy?r.legacy:r.modern,false,rst+binding);
 // Sorted admission only: unchanged rejection or position1/candidate-first;
 // do not assert native successful sort/shared-state behavior.
 for(const std::string cls:{"ListBox","ComboBox"})
 for(const auto& r:std::vector<NumberCase>{
  {"1.9",1.9,1,1},{"0",0,{},{}},{"5",5,5,5},
  {"4294967297.9",4294967297.9,{},1},{"-4294967295.9",-4294967295.9,{},1},
  {"1E300",1E300,{},{}}})
  invoke_case(path,mode,selected,cls,2,r.source,legacy?r.legacy:r.modern,false,"o.Sorted=.T.\no.RowSourceType=0\n");
 for(const auto& locale:std::vector<std::vector<std::string>>{
  {"en-US","Function argument value, type, or count is invalid."},
  {"es-419","El valor, tipo o numero de un argumento de funcion no es valido."},
  {"pt-BR","O valor, tipo ou numero de um argumento de funcao e invalido."},
  {"qps-ploc","[!! Füñçţïøñ årĝümëñţ vålüë, ţÿþë, ør çøüñţ ïš ïñvålïð. !!]"}}){
  set_env_value("COPPERFIN_LOCALE",locale[0],true);
  for(const std::string cls:{"ListBox","ComboBox"}){
   invoke_case(path,mode,selected,cls,2,"0",{},false,{},locale[1]);
   invoke_case(path,mode,selected,cls,2,"5",{},false,{},locale[1]);
   invoke_case(path,mode,selected,cls,2,"4",4);
  }
 }
 set_env_value("COPPERFIN_LOCALE","en-US",true);
 std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv){
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 if(argc==1){direct_cases();expect(direct_count==1242&&public_count==0,"AddItem complete direct count");}
 else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
  (std::string(argv[2])=="1"||std::string(argv[2])=="2")){
  public_cases(argv[1],std::string(argv[2])=="1"?1:2);
  expect(direct_count==0&&public_count==1102,"AddItem complete public count");
 }else{std::cerr<<"Expected no args or COPPERFIN|VFP9 1|2\n";return 2;}
 std::cout<<"AddItem frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

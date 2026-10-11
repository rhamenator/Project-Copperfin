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
namespace fs = std::filesystem;
using O = std::optional<std::int32_t>;
// RQ-CF-PRG-LISTINDEX-NUMERIC-001 / VR-5611-LISTINDEX-NUMERIC-001/002.
// Independently enumerated projection/final-row literals from the installed
// VFP9/help map and named parent arithmetic, never current helper output.
// Existing test support is scaffolding only, not a semantic oracle.
struct Row {
 const char* expression; double raw; O modern,legacy;
 int modern_list,modern_combo,legacy_list,legacy_combo;
};
const std::vector<Row> rows{
 {"0",0,0,0,0,0,0,0},
 {"-0",-0.,0,0,0,0,0,0},
 {"0.49",0.49,0,0,0,0,0,0},
 {"0.5",0.5,0,0,0,0,0,0},
 {"0.9",0.9,0,0,0,0,0,0},
 {"1",1,1,1,1,1,1,1},
 {"1.49",1.49,1,1,1,1,1,1},
 {"1.5",1.5,1,1,1,1,1,1},
 {"1.9",1.9,1,1,1,1,1,1},
 {"2",2,2,2,2,2,2,2},
 {"2.49",2.49,2,2,2,2,2,2},
 {"2.5",2.5,2,2,2,2,2,2},
 {"2.9",2.9,2,2,2,2,2,2},
 {"3",3,3,3,3,3,3,3},
 {"3.49",3.49,3,3,3,3,3,3},
 {"3.5",3.5,3,3,3,3,3,3},
 {"3.9",3.9,3,3,3,3,3,3},
 {"4",4,4,4,0,4,0,4},
 {"10.9",10.9,10,10,0,4,0,4},
 {"11",11,11,11,0,4,0,4},
 {"11.49",11.49,11,11,0,4,0,4},
 {"11.5",11.5,11,11,0,4,0,4},
 {"11.9",11.9,11,11,0,4,0,4},
 {"21.9",21.9,21,21,0,4,0,4},
 {"22",22,22,22,0,4,0,4},
 {"22.49",22.49,22,22,0,4,0,4},
 {"22.5",22.5,22,22,0,4,0,4},
 {"22.9",22.9,22,22,0,4,0,4},
 {"33",33,33,33,0,4,0,4},
 {"33.9",33.9,33,33,0,4,0,4},
 {"34",34,34,34,0,4,0,4},
 {"-0.49",-0.49,0,0,0,0,0,0},
 {"-0.5",-0.5,0,0,0,0,0,0},
 {"-0.9",-0.9,0,0,0,0,0,0},
 {"-1",-1,-1,-1,0,0,0,0},
 {"-1.9",-1.9,-1,-1,0,0,0,0},
 {"2147483647",2147483647,INT32_MAX,INT32_MAX,0,4,0,4},
 {"2147483647.1",2147483647.1,INT32_MAX,INT32_MAX,0,4,0,4},
 {"2147483647.5",2147483647.5,INT32_MAX,INT32_MAX,0,4,0,4},
 {"2147483647.9",2147483647.9,INT32_MAX,INT32_MAX,0,4,0,4},
 {"2147483648",2147483648,{},INT32_MIN,0,0,0,4},
 {"-2147483648",-2147483648,INT32_MIN,INT32_MIN,0,0,0,4},
 {"-2147483648.9",-2147483648.9,INT32_MIN,INT32_MIN,0,0,0,4},
 {"-2147483649",-2147483649,{},INT32_MAX,0,0,0,4},
 {"-2147483649.9",-2147483649.9,{},INT32_MAX,0,0,0,4},
 {"4294967295",4294967295,{},-1,0,0,0,0},
 {"4294967296",4294967296,{},{},0,0,0,0},
 {"4294967297",4294967297,{},{},0,0,0,0},
 {"4294967297.9",4294967297.9,{},{},0,0,0,0},
 {"4294967307",4294967307,{},{},0,0,0,0},
 {"4294967318.9",4294967318.9,{},{},0,0,0,0},
 {"-4294967295",-4294967295,{},1,0,0,1,1},
 {"-4294967295.9",-4294967295.9,{},1,0,0,1,1},
 {"-4294967294.5",-4294967294.5,{},2,0,0,2,2},
 {"-4294967293",-4294967293,{},3,0,0,3,3},
 {"-4294967285",-4294967285,{},11,0,0,0,4},
 {"-4294967274.9",-4294967274.9,{},22,0,0,0,4},
 {"-4294967263",-4294967263,{},33,0,0,0,4},
 {"-4294967296",-4294967296,{},0,0,0,0,0},
 {"-4294967296.5",-4294967296.5,{},0,0,0,0,0},
 {"-4294967297",-4294967297,{},-1,0,0,0,0},
 {"9007199254740992",9007199254740992,{},{},0,0,0,0},
 {"1E20",1E20,{},{},0,0,0,0},
 {"1E300",1E300,{},{},0,0,0,0},
 {"-9007199254740992",-9007199254740992,{},0,0,0,0,0},
 {"-1E20",-1E20,{},0,0,0,0,0},
 {"-1E300",-1E300,{},0,0,0,0,0},
 {"EXP(1000)",std::numeric_limits<double>::infinity(),{},{},0,0,0,0},
 {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{},0,0,0,0,0},
 {"1E-300",1E-300,0,0,0,0,0,0},
 {"-1E-300",-1E-300,0,0,0,0,0,0},
 {"2147483648.9",2147483648.9,{},INT32_MIN,0,0,0,4},
 {"2147483649",2147483649,{},INT32_MIN+1,0,0,0,0},
 {"2147483649.9",2147483649.9,{},INT32_MIN+1,0,0,0,0},
 {"4294967293",4294967293,{},-3,0,0,0,0},
 {"4294967294",4294967294,{},-2,0,0,0,0},
 {"4294967295.1",4294967295.1,{},{},0,0,0,0},
 {"4294967295.5",4294967295.5,{},{},0,0,0,0},
 {"4294967295.9",4294967295.9,{},{},0,0,0,0},
 {"-2147483647",-2147483647,-INT32_MAX,-INT32_MAX,0,0,0,0},
 {"-6442450944",-6442450944,{},INT32_MIN,0,0,0,4},
 {"-6442450943",-6442450943,{},INT32_MIN+1,0,0,0,0},
 {"-6442450945",-6442450945,{},INT32_MAX,0,0,0,4},
 {"-8589934590",-8589934590,{},2,0,0,2,2},
 {"-8589934591.9",-8589934591.9,{},1,0,0,1,1},
 {"8589934593",8589934593,{},{},0,0,0,0},
};

int direct_count=0,public_count=0;
void direct_cases() {
 expect(rows.size()==86,"ListIndex frozen Numeric count");
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  const auto check=[&](const PrgValue& value,O modern,O legacy,const char* label) {
   ++direct_count;
   expect(checked_listindex_numeric_argument(value,mode)==(mode==NumericBehavior::vfp9?legacy:modern),
          std::string("ListIndex direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+label);
  };
  for(const auto& row:rows) check(make_number_value(row.raw),row.modern,row.legacy,row.expression);
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},{},"derived NaN");
  check(make_number_value(std::nextafter(2147483648.,0.)),INT32_MAX,INT32_MAX,"signed upper inner");
  check(make_number_value(std::nextafter(2147483648.,INFINITY)),{},INT32_MIN,"signed upper outer");
  check(make_number_value(std::nextafter(-2147483649.,0.)),INT32_MIN,INT32_MIN,"signed lower inner");
  check(make_number_value(std::nextafter(-2147483649.,-INFINITY)),{},INT32_MAX,"signed lower outer");
  check(make_number_value(std::nextafter(4294967295.,0.)),{},-2,"raw uint32 ceiling inner");
  check(make_number_value(std::nextafter(4294967295.,INFINITY)),{},{},"raw uint32 ceiling outer");
  check(make_number_value(std::nextafter(1.,0.)),0,0,"one inner");
  check(make_number_value(std::nextafter(-1.,0.)),0,0,"negative one inner");
  check(make_number_value(std::nextafter(0.,1.)),0,0,"positive subnormal");
  check(make_number_value(std::nextafter(0.,-1.)),0,0,"negative subnormal");
  check(make_number_value(std::nextafter(3.,0.)),2,2,"three inner");
  struct Signed {std::int64_t raw;O modern,legacy;};
  for(const auto& row:std::vector<Signed>{
   {0,0,0},{1,1,1},{2,2,2},{3,3,3},{11,11,11},{22,22,22},
   {-1,-1,-1},{INT32_MAX,INT32_MAX,INT32_MAX},
   {2147483648LL,{},INT32_MIN},{INT32_MIN,INT32_MIN,INT32_MIN},
   {-2147483649LL,{},INT32_MAX},{4294967295LL,{},-1},{4294967296LL,{},{}},
   {-4294967294LL,{},2},{-9007199254740990LL,{},2},
   {INT64_MIN,{},0},{INT64_MIN+2,{},2},{INT64_MIN+3,{},3},
   {INT64_MIN+2147483648LL,{},INT32_MIN},{INT64_MAX,{},{}},
   {-6442450943LL,{},INT32_MIN+1},{-8589934590LL,{},2}})
   check(make_int64_value(row.raw),row.modern,row.legacy,"exact signed64");
  struct Unsigned {std::uint64_t raw;O modern,legacy;};
  for(const auto& row:std::vector<Unsigned>{
   {0,0,0},{1,1,1},{2,2,2},{3,3,3},{11,11,11},{22,22,22},
   {2147483647ULL,INT32_MAX,INT32_MAX},{2147483648ULL,{},INT32_MIN},
   {4294967295ULL,{},-1},{4294967296ULL,{},{}},
   {9223372036854775810ULL,{},{}},{UINT64_MAX,{},{}}})
   check(make_uint64_value(row.raw),row.modern,row.legacy,"exact unsigned64");
  auto typed_null=make_number_value(2.9);typed_null.is_null=true;
  check(typed_null,{},{},"typed Numeric NULL adapter");
  check(make_currency_value(29000),{},{},"Currency adapter");
  check(make_boolean_value(true),{},{},"Logical adapter");
  check(make_string_value("2.9"),{},{},"Character adapter");
  check(make_null_value(),{},{},"NULL adapter");
 }
}
struct Exact {const char* expression;O modern,legacy;
 int modern_list,modern_combo,legacy_list,legacy_combo;};
const std::vector<Exact> exacts{
 {"CAST(0 AS INT64)",0,0,0,0,0,0},
 {"CAST(1 AS INT64)",1,1,1,1,1,1},
 {"CAST(2 AS INT64)",2,2,2,2,2,2},
 {"CAST(3 AS INT64)",3,3,3,3,3,3},
 {"CAST(11 AS INT64)",11,11,0,4,0,4},
 {"CAST(22 AS INT64)",22,22,0,4,0,4},
 {"CAST(-1 AS INT64)",-1,-1,0,0,0,0},
 {"CAST(2147483647 AS INT64)",INT32_MAX,INT32_MAX,0,4,0,4},
 {"CAST(2147483648 AS INT64)",{},INT32_MIN,0,0,0,4},
 {"CAST(-2147483648 AS INT64)",INT32_MIN,INT32_MIN,0,0,0,4},
 {"(CAST(-2147483648 AS INT64)-CAST(1 AS INT64))",{},INT32_MAX,0,0,0,4},
 {"CAST(4294967295 AS INT64)",{},-1,0,0,0,0},
 {"CAST(4294967296 AS INT64)",{},{},0,0,0,0},
 {"CAST(-4294967294 AS INT64)",{},2,0,0,2,2},
 {"(CAST(-9007199254740992 AS INT64)+CAST(2 AS INT64))",{},2,0,0,2,2},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(2 AS INT64))",{},2,0,0,2,2},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(3 AS INT64))",{},3,0,0,3,3},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(2147483648 AS INT64))",{},INT32_MIN,0,0,0,4},
 {"CAST(0 AS UINT64)",0,0,0,0,0,0},
 {"CAST(2 AS UINT64)",2,2,2,2,2,2},
 {"CAST(4294967295 AS UINT64)",{},-1,0,0,0,0},
 {"CAST(4294967296 AS UINT64)",{},{},0,0,0,0},
 {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{},{},0,0,0,0},
 {"(CAST(9223372036854775808 AS UINT64)+CAST(2 AS UINT64))",{},{},0,0,0,0},
};
std::string get(const RuntimePauseState& state,const char* key) {
 const auto it=state.globals.find(key);
 return it==state.globals.end()?"<missing>":format_value(it->second);
}
void public_case(const fs::path& path,const std::string& mode,int selected,const std::string& cls,
                 bool dense,bool reflection,const std::string& input,O projection,int final_index,
                 bool empty=false,bool multi=false,const std::string& message={},int reject_code=0) {
 ++public_count;
 const auto other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto tag=mode+" session"+std::to_string(selected)+" "+cls+(dense?" dense":" sparse")+
  (empty?" empty":"")+(multi?" multi":" single")+(reflection?" SETPEM ":" direct ")+input;
 const std::string populate=empty?"":(dense?
  "oList.AddItem('first')\noList.AddItem('second')\noList.AddItem('third')\n":
  "oList.AddListItem('first',11)\noList.AddListItem('second',22)\noList.AddListItem('third',33)\n");
 const auto assignment=reflection?"lSetter=SETPEM(oList,'ListIndex',"+input+")\n":
  "oList.ListIndex="+input+"\nlSetter=.T.\n";
 write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+std::string(other)+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC nCode,cMessage,lAfter,cBefore,cAfter,cOther,cReset,nGuard,oList,lSetter\n"
  "nCode=0\ncMessage=''\nlAfter=.F.\nlSetter='sentinel'\n"
  "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "oList=CREATEOBJECT('"+cls+"')\n"+populate+
  (empty?"":"oList.ItemData(1)=101\noList.ItemData(2)=202\noList.ItemData(3)=303\n")+
  (multi?"oList.MultiSelect=.T.\noList.Selected(1)=.T.\noList.Selected(2)=.T.\n":"")+
  "oList.ListIndex="+(empty?std::string("0"):std::string("2"))+"\n"
  "cBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\n"+assignment+"CATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\n"
  "lAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\n"
  "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
  "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
 const auto state=session.run(DebugResumeAction::continue_run);
 const int code=projection?0:(reject_code?reject_code:(mode=="VFP9"?46:31));
 expect(state.completed,"ListIndex completion "+tag+": "+state.message);
 expect(get(state,"ncode")==std::to_string(code),"ListIndex error "+tag+" got "+get(state,"ncode"));
 expect(get(state,"lsetter")==(projection?"true":"sentinel"),"ListIndex write result/sentinel "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","ListIndex continuation "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"cbefore")==context&&get(state,"cafter")==context,"ListIndex context "+tag);
 expect(get(state,"cother")==(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|",
        "ListIndex isolation/reset "+tag);
 if(projection) expect(get(state,"cmessage").empty(),"ListIndex successful message "+tag);
 if(!message.empty()) expect(get(state,"cmessage")==message,"ListIndex localized error "+tag);
 const int wanted=projection?final_index:(empty?0:2);
 const int row=!empty&&wanted>=1&&wanted<=3?wanted:0;
 const std::string cell=row==1?"first":row==2?"second":row==3?"third":"";
 bool seen=false;
 for(const auto& object:state.ole_objects) if(copperfin::runtime::lowercase_copy(object.base_class_name)==copperfin::runtime::lowercase_copy(cls)) {
  seen=true;
  const auto prop=[&](const char* key,const std::string& value,PrgValueKind kind) {
   const auto found=object.properties.find(key);
   expect(found!=object.properties.end()&&found->second.kind==kind&&format_value(found->second)==value,
          std::string("ListIndex state ")+key+" "+tag+(found==object.properties.end()?" missing":" got "+format_value(found->second)));
  };
  prop("listindex",std::to_string(wanted),PrgValueKind::number);
  prop("listitemid",std::to_string(row?(dense?row:row*11):0),PrgValueKind::number);
  prop("value",cell,PrgValueKind::string);
  prop("displayvalue",cell,PrgValueKind::string);
  prop("listcount",empty?"0":"3",PrgValueKind::number);
  prop("newindex",empty?"0":"3",PrgValueKind::number);
  prop("newitemid",empty?"0":dense?"3":"33",PrgValueKind::number);
  expect(object.list_rows.size()==(empty?0U:3U)&&object.list_item_data.size()==(empty?0U:3U),
         "ListIndex bounded count "+tag);
  if(!empty&&object.list_rows.size()==3) {
   const std::vector<std::string> cells{"first","second","third"};
   for(std::size_t slot=0;slot<3;++slot)
    expect(object.list_rows[slot].size()==1&&format_value(object.list_rows[slot][0])==cells[slot],
           "ListIndex preserved cell "+tag);
  }
  if(!empty&&object.list_item_data.size()==3)
   expect(format_value(object.list_item_data[0])=="101"&&format_value(object.list_item_data[1])=="202"&&
          format_value(object.list_item_data[2])=="303","ListIndex preserved payload "+tag);
  std::vector<bool> bits=empty?std::vector<bool>{}:multi?std::vector<bool>{true,true,false}:std::vector<bool>{false,false,false};
  if(row>0) bits[static_cast<std::size_t>(row-1)]=true;
  expect(object.list_selected==bits,"ListIndex exact selection bits "+tag);
  const auto keys=empty?std::vector<std::string>{}:dense?std::vector<std::string>{"1","2","3"}:std::vector<std::string>{"11","22","33"};
  expect(object.collection_item_keys==keys,"ListIndex preserved row identities "+tag);
 }
 expect(seen,"ListIndex native object captured "+tag);
 for(const auto& event:state.events)
  expect(event.category!="ole.invoke","ListIndex no external invocation "+tag);
}
void public_cases(const std::string& mode,int selected) {
 expect(rows.size()==86&&exacts.size()==24,"ListIndex frozen public input counts");
 const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
 const auto dir=fs::temp_directory_path()/("copperfin_listindex_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
 fs::create_directories(dir);
 const auto path=dir/"query.prg";
 for(const std::string cls:{"ListBox","ComboBox"}) for(const bool dense:{false,true}) for(const bool reflection:{false,true}) {
  const bool combo=cls=="ComboBox",legacy=mode=="VFP9";
  for(const auto& r:rows)
   public_case(path,mode,selected,cls,dense,reflection,r.expression,legacy?r.legacy:r.modern,
               legacy?(combo?r.legacy_combo:r.legacy_list):(combo?r.modern_combo:r.modern_list));
  for(const auto& r:exacts)
   public_case(path,mode,selected,cls,dense,reflection,r.expression,legacy?r.legacy:r.modern,
               legacy?(combo?r.legacy_combo:r.legacy_list):(combo?r.modern_combo:r.modern_list));
  // Parent-derived public NaN, not a native NaN-syntax observation.
  public_case(path,mode,selected,cls,dense,reflection,"(EXP(1000)-EXP(1000))",{},0,false,false,{},31);
 }
 // Independently observed edge source: all literal boundary/control inputs,
 // empty controls and both MultiSelect identity shapes on both existing routes.
 const std::vector<std::string> edges{"0","0.9","1","1.9","2.9","3.9","4","22.9","-0.9","-1.9",
  "2147483647.9","2147483648","4294967294","4294967295","4294967295.1","4294967295.5",
  "4294967295.9","4294967296","-2147483649","-4294967293","-4294967294.5","-4294967295.9",
  "-4294967296","-4294967297","-8589934590","-8589934591.9","8589934593","-1E300","-EXP(1000)"};
 expect(edges.size()==29,"ListIndex frozen edge count");
 for(const auto& text:edges) {
  const Row* chosen=nullptr;for(const auto& r:rows) if(r.expression==text) chosen=&r;
  expect(chosen!=nullptr,"ListIndex enumerated edge "+text);
  if(!chosen) continue;
  const auto& r=*chosen;const bool legacy=mode=="VFP9";
  const auto projection=legacy?r.legacy:r.modern;
  for(const bool reflection:{false,true}) {
   for(const bool dense:{false,true})
    public_case(path,mode,selected,"ListBox",dense,reflection,text,projection,
                legacy?r.legacy_list:r.modern_list,false,true);
   public_case(path,mode,selected,"ListBox",true,reflection,text,projection,0,true);
   const int combo=legacy?r.legacy_combo:r.modern_combo;
   public_case(path,mode,selected,"ComboBox",true,reflection,text,projection,combo>0?1:0,true);
  }
 }
 for(const auto& locale:std::vector<std::vector<std::string>>{
  {"en-US","Subscript is outside defined range.","Numeric overflow."},
  {"es-419","El subíndice está fuera del intervalo definido.","Desbordamiento numérico."},
  {"pt-BR","O subscrito está fora do intervalo definido.","Estouro numérico."},
  {"qps-ploc","[!! Šµþšçŕïþţ ïš øµţšïðë ðëƒïñëð ŕåñğë. !!]","[!! Ñµmëŕïç øṽëŕƒlöω. !!]"}}) {
  set_env_value("COPPERFIN_LOCALE",locale[0],true);
  for(const std::string cls:{"ListBox","ComboBox"}) for(const bool dense:{false,true}) for(const bool reflection:{false,true}) {
   public_case(path,mode,selected,cls,dense,reflection,mode=="VFP9"?"4294967295.1":"2147483648",{},0,
               false,false,locale[mode=="VFP9"?2:1]);
   public_case(path,mode,selected,cls,dense,reflection,"2.9",2,2);
  }
 }
 set_env_value("COPPERFIN_LOCALE","en-US",true);
 std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv) {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 if(argc==1) {
  direct_cases();
  expect(direct_count==274&&public_count==0,"ListIndex complete direct count");
 } else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
           (std::string(argv[2])=="1"||std::string(argv[2])=="2")) {
  public_cases(argv[1],std::string(argv[2])=="1"?1:2);
  expect(direct_count==0&&public_count==1184,"ListIndex complete public count");
 } else {std::cerr<<"Expected no args or COPPERFIN|VFP9 1|2\n";return 2;}
 std::cout<<"ListIndex frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

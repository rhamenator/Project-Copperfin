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
#include <utility>
#include <vector>

namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;
// RQ-CF-PRG-LISTITEM-NUMERIC-001 / VR-5611-LISTITEM-NUMERIC-001/002.
// Independently recovered native/help and parent-derived literals, not helper output.
struct Row {const char* text; double raw; std::optional<std::int32_t> modern,old;};
const std::vector<Row> rows{
 {"0",0,0,0},
 {"-0",-0.,0,0},
 {"0.49",.49,0,0},
 {"0.5",.5,0,0},
 {"0.9",.9,0,0},
 {"1",1,1,1},
 {"1.49",1.49,1,1},
 {"1.5",1.5,1,1},
 {"1.9",1.9,1,1},
 {"2",2,2,2},
 {"2.49",2.49,2,2},
 {"2.5",2.5,2,2},
 {"2.9",2.9,2,2},
 {"3",3,3,3},
 {"3.49",3.49,3,3},
 {"3.5",3.5,3,3},
 {"3.9",3.9,3,3},
 {"4",4,4,4},
 {"10.9",10.9,10,10},
 {"11",11,11,11},
 {"11.49",11.49,11,11},
 {"11.5",11.5,11,11},
 {"11.9",11.9,11,11},
 {"21.9",21.9,21,21},
 {"22",22,22,22},
 {"22.49",22.49,22,22},
 {"22.5",22.5,22,22},
 {"22.9",22.9,22,22},
 {"33",33,33,33},
 {"33.9",33.9,33,33},
 {"34",34,34,34},
 {"-0.49",-.49,0,0},
 {"-0.5",-.5,0,0},
 {"-0.9",-.9,0,0},
 {"-1",-1,{},{}},
 {"-1.9",-1.9,{},{}},
 {"2147483647",2147483647.,INT32_MAX,INT32_MAX},
 {"2147483647.1",2147483647.1,INT32_MAX,{}},
 {"2147483647.5",2147483647.5,INT32_MAX,{}},
 {"2147483647.9",2147483647.9,INT32_MAX,{}},
 {"2147483648",2147483648.,{},{}},
 {"-2147483648",-2147483648.,{},{}},
 {"-2147483648.9",-2147483648.9,{},{}},
 {"-2147483649",-2147483649.,{},INT32_MAX},
 {"-2147483649.9",-2147483649.9,{},INT32_MAX},
 {"4294967295",4294967295.,{},{}},
 {"4294967296",4294967296.,{},{}},
 {"4294967297",4294967297.,{},{}},
 {"4294967297.9",4294967297.9,{},{}},
 {"4294967307",4294967307.,{},{}},
 {"4294967318.9",4294967318.9,{},{}},
 {"-4294967295",-4294967295.,{},1},
 {"-4294967295.9",-4294967295.9,{},1},
 {"-4294967294.5",-4294967294.5,{},2},
 {"-4294967293",-4294967293.,{},3},
 {"-4294967285",-4294967285.,{},11},
 {"-4294967274.9",-4294967274.9,{},22},
 {"-4294967263",-4294967263.,{},33},
 {"-4294967296",-4294967296.,{},0},
 {"-4294967296.5",-4294967296.5,{},0},
 {"-4294967297",-4294967297.,{},{}},
 {"9007199254740992",9007199254740992.,{},{}},
 {"-9007199254740992",-9007199254740992.,{},0},
 {"1E20",1E20,{},{}},
 {"-1E20",-1E20,{},0},
 {"1E300",1E300,{},{}},
 {"-1E300",-1E300,{},0},
 {"EXP(1000)",std::numeric_limits<double>::infinity(),{},{}},
 {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{},0},
 {"1E-300",1E-300,0,0},
 {"-1E-300",-1E-300,0,0}
};
int direct_count=0,public_count=0;
void direct_cases() {
 expect(rows.size()==71,"ListItem frozen Numeric count");
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  const auto check=[&](const PrgValue& value,std::optional<std::int32_t> modern,
                       std::optional<std::int32_t> old,const char* label) {
   ++direct_count;
   expect(checked_listitem_numeric_argument(value,mode)==(mode==NumericBehavior::vfp9?old:modern),
          std::string("ListItem direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+label);
  };
  for(const auto& row:rows) check(make_number_value(row.raw),row.modern,row.old,row.text);
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},{},"derived NaN");
  check(make_number_value(std::nextafter(2147483648.,0.)),INT32_MAX,{},"converted upper inner");
  check(make_number_value(std::nextafter(2147483647.,INFINITY)),INT32_MAX,{},"native raw ceiling gate");
  check(make_number_value(std::nextafter(2147483648.,INFINITY)),{},{},"converted upper outer");
  check(make_number_value(std::nextafter(-2147483649.,0.)),{},{},"converted lower inner");
  check(make_number_value(std::nextafter(-2147483649.,-INFINITY)),{},INT32_MAX,"converted lower outer");
  check(make_number_value(std::nextafter(22.,0.)),21,21,"sparse identity inner");
  check(make_number_value(std::nextafter(22.,INFINITY)),22,22,"sparse identity outer");
  check(make_number_value(std::nextafter(1.,0.)),0,0,"one inner");
  check(make_number_value(std::nextafter(-1.,0.)),0,0,"negative inner");
  check(make_number_value(std::nextafter(0.,1.)),0,0,"positive subnormal");
  check(make_number_value(std::nextafter(0.,-1.)),0,0,"negative subnormal");
  struct Signed {std::int64_t raw;std::optional<std::int32_t> modern,old;};
  for(const auto& row:std::vector<Signed>{
   {0,0,0},
   {1,1,1},
   {3,3,3},
   {11,11,11},
   {22,22,22},
   {33,33,33},
   {2147483647,INT32_MAX,INT32_MAX},
   {2147483648LL,{},{}},
   {-1,{},{}},
   {-2147483648LL,{},{}},
   {-2147483649LL,{},INT32_MAX},
   {-4294967285LL,{},11},
   {-4294967274LL,{},22},
   {-4294967263LL,{},33},
   {-4294967296LL,{},0},
   {-4294967297LL,{},{}},
   {9007199254741003LL,{},{}},
   {-9007199254740970LL,{},22},
   {INT64_MIN,{},0},
   {INT64_MIN+22,{},22},
   {INT64_MIN+33,{},33},
   {INT64_MAX,{},{}},
   {4294967307LL,{},{}},
   {4294967318LL,{},{}}})
   check(make_int64_value(row.raw),row.modern,row.old,"exact signed64");
  struct Unsigned {std::uint64_t raw;std::optional<std::int32_t> modern,old;};
  for(const auto& row:std::vector<Unsigned>{
   {0,0,0},
   {1,1,1},
   {3,3,3},
   {11,11,11},
   {22,22,22},
   {33,33,33},
   {2147483647ULL,INT32_MAX,INT32_MAX},
   {2147483648ULL,{},{}},
   {4294967296ULL,{},{}},
   {9007199254741003ULL,{},{}},
   {9223372036854775830ULL,{},{}},
   {UINT64_MAX,{},{}}})
   check(make_uint64_value(row.raw),row.modern,row.old,"exact unsigned64");
  PrgValue typed_null=make_number_value(22.9);
  typed_null.is_null=true;
  PrgValue currency{};
  currency.kind=PrgValueKind::currency;
  currency.currency_value=229000;
  check(typed_null,{},{},"typed Numeric NULL adapter");
  check(currency,{},{},"Currency adapter");
  check(make_boolean_value(true),{},{},"Logical adapter");
  check(make_string_value("22.9"),{},{},"Character adapter");
  check(make_null_value(),{},{},"NULL adapter");
 }
}
struct Exact {const char* expression;std::optional<std::int32_t> modern,old;};
const std::vector<Exact> exacts{
 {"CAST(0 AS INT64)",0,0},
 {"CAST(1 AS INT64)",1,1},
 {"CAST(3 AS INT64)",3,3},
 {"CAST(11 AS INT64)",11,11},
 {"CAST(22 AS INT64)",22,22},
 {"CAST(33 AS INT64)",33,33},
 {"CAST(-1 AS INT64)",{},{}},
 {"CAST(2147483647 AS INT64)",INT32_MAX,INT32_MAX},
 {"CAST(2147483648 AS INT64)",{},{}},
 {"CAST(-2147483648 AS INT64)",{},{}},
 {"(CAST(-2147483648 AS INT64)-CAST(1 AS INT64))",{},INT32_MAX},
 {"CAST(4294967307 AS INT64)",{},{}},
 {"CAST(-4294967285 AS INT64)",{},11},
 {"CAST(-4294967274 AS INT64)",{},22},
 {"CAST(-4294967263 AS INT64)",{},33},
 {"(CAST(9007199254740992 AS INT64)+CAST(11 AS INT64))",{},{}},
 {"(CAST(-9007199254740992 AS INT64)+CAST(22 AS INT64))",{},22},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(22 AS INT64))",{},22},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(33 AS INT64))",{},33},
 {"CAST(11 AS UINT64)",11,11},
 {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{},{}},
 {"(CAST(9223372036854775808 AS UINT64)+CAST(22 AS UINT64))",{},{}}
};
std::string get(const RuntimePauseState& state,const char* key) {
 const auto it=state.globals.find(key);
 return it==state.globals.end()?"<missing>":format_value(it->second);
}
void public_case(const fs::path& path,const std::string& mode,int selected,const std::string& cls,
                 bool dense,bool bracket,const std::string& input,std::optional<std::int32_t> id,
                 const std::string& message={},bool full_expression=false) {
 ++public_count;
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto expression=full_expression?input:"oList.ListItem"+std::string(bracket?"[":"(")+input+(bracket?"]":")");
 const auto tag=mode+" session"+std::to_string(selected)+" "+cls+(dense?" dense ":" sparse ")+expression;
 write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC cResult,nCode,cMessage,lAfter,cBefore,cAfter,cOther,cReset,nGuard,oList\n"
  "cResult='sentinel'\nnCode=0\ncMessage=''\nlAfter=.F.\n"
  "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "oList=CREATEOBJECT('"+cls+"')\n"+
  (dense?"oList.AddItem('first')\noList.AddItem('second')\noList.AddItem('third')\n":
         "oList.AddListItem('first',11)\noList.AddListItem('second',22)\noList.AddListItem('third',33)\n")+
  "oList.ItemData(1)=101\noList.ItemData(2)=202\noList.ItemData(3)=303\noList.ListIndex=2\n"
  "cBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\ncResult="+expression+"\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\n"
  "lAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\n"
  "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
  "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"ListItem completion "+tag+": "+state.message);
 expect(get(state,"ncode")==(id?"0":"31"),"ListItem error "+tag+" got "+get(state,"ncode"));
 int row=0;
 if(id) {
  if(dense && *id>=1 && *id<=3) row=*id;
  else if(!dense && *id==11) row=1;
  else if(!dense && *id==22) row=2;
  else if(!dense && *id==33) row=3;
 }
 expect(get(state,"cresult")==(id?(row==1?"first":row==2?"second":row==3?"third":""):"sentinel"),"ListItem result "+tag+" got "+get(state,"cresult"));
 const auto found=state.globals.find("cresult");
 expect(found!=state.globals.end()&&found->second.kind==PrgValueKind::string,
        "ListItem kind "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","ListItem continuation "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"cbefore")==context&&get(state,"cafter")==context,"ListItem context "+tag);
 expect(get(state,"cother")==(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|",
        "ListItem isolation/reset "+tag);
 if(id) expect(get(state,"cmessage").empty(),"ListItem successful message "+tag);
 if(!message.empty()) expect(get(state,"cmessage")==message,"ListItem localized31 "+tag);
 bool object_seen=false;
 for(const auto& object:state.ole_objects) if(copperfin::runtime::lowercase_copy(object.base_class_name)==copperfin::runtime::lowercase_copy(cls)) {
  object_seen=true;
  expect(object.list_rows.size()==3&&object.list_item_data.size()==3,"ListItem count "+tag);
  if(object.list_item_data.size()==3)
   expect(format_value(object.list_item_data[0])=="101"&&format_value(object.list_item_data[1])=="202"&&
          format_value(object.list_item_data[2])=="303","ListItem preserved payload "+tag);
  expect(object.list_selected==std::vector<bool>({false,true,false}),"ListItem preserved bits "+tag);
  const std::vector<std::string> keys=dense?std::vector<std::string>{"1","2","3"}:std::vector<std::string>{"11","22","33"};
  expect(object.collection_item_keys==keys,"ListItem stable identities "+tag);
  const auto item_id=object.properties.find("listitemid");
  expect(item_id!=object.properties.end()&&format_value(item_id->second)==(dense?"2":"22"),
         "ListItem preserved item ID "+tag);
  const auto selected_row=object.properties.find("listindex");
  expect(selected_row!=object.properties.end()&&format_value(selected_row->second)=="2","ListItem preserved selection "+tag);
 }
 expect(object_seen,"ListItem native object captured "+tag);
 for(const auto& event:state.events)
  expect(event.category!="prg.object.selected"&&event.category!="ole.invoke","ListItem no external/success event "+tag);
}
void public_cases(const std::string& mode,int selected) {
 expect(rows.size()==71&&exacts.size()==22,"ListItem frozen public input counts");
 const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
 const auto dir=fs::temp_directory_path()/("copperfin_listitem_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
 fs::create_directories(dir);
 const auto path=dir/"query.prg";
 for(const std::string cls:{"ListBox","ComboBox"}) for(const bool dense:{false,true}) for(const bool bracket:{false,true}) {
  for(const auto& row:rows) public_case(path,mode,selected,cls,dense,bracket,row.text,mode=="VFP9"?row.old:row.modern);
  for(const auto& row:exacts) public_case(path,mode,selected,cls,dense,bracket,row.expression,mode=="VFP9"?row.old:row.modern);
  // Preserved legacy coercions only, NOT native type-admission expectations.
  public_case(path,mode,selected,cls,dense,bracket,".T.",1);
  public_case(path,mode,selected,cls,dense,bracket,".F.",0);
  public_case(path,mode,selected,cls,dense,bracket,".NULL.",0);
  public_case(path,mode,selected,cls,dense,bracket,"'22.9'",23);
  public_case(path,mode,selected,cls,dense,bracket,"$22.9",23);
  for(const auto& locale:std::vector<std::pair<std::string,std::string>>{
   {"en-US","Subscript is outside defined range."},
   {"es-419","El subíndice está fuera del intervalo definido."},
   {"pt-BR","O subscrito está fora do intervalo definido."},
   {"qps-ploc","[!! Šµþšçŕïþţ ïš øµţšïðë ðëƒïñëð ŕåñğë. !!]"}}) {
    set_env_value("COPPERFIN_LOCALE",locale.first,true);
    public_case(path,mode,selected,cls,dense,bracket,".NULL.",0);
    if(mode=="COPPERFIN") public_case(path,mode,selected,cls,dense,bracket,"2147483648",{},locale.second);
    else public_case(path,mode,selected,cls,dense,bracket,"4294967307",{},locale.second);
   }
  set_env_value("COPPERFIN_LOCALE","en-US",true);
 }
 const std::vector<Exact> postfix{
  {"22.9",22,22},
  {"2.9",2,2},
  {"33.9",33,33},
  {"0.5",0,0},
  {"-1",{},{}},
  {"4294967307",{},{}},
  {"-4294967274.9",{},22},
  {"2147483647.5",INT32_MAX,{}},
  {"2147483648",{},{}},
  {"(CAST(-9223372036854775808 AS INT64)+CAST(22 AS INT64))",{},22},
  {"(CAST(9007199254740992 AS INT64)+CAST(11 AS INT64))",{},{}},
  {"EXP(1000)",{},{}}
 };
 expect(postfix.size()==12,"ListItem frozen postfix count");
 for(const std::string cls:{"ListBox","ComboBox"}) for(const bool dense:{false,true}) for(const bool bracket:{false,true})
  for(const auto& row:postfix)
   public_case(path,mode,selected,cls,dense,bracket,
    "IIF(.T.,oList,.NULL.).ListItem"+std::string(bracket?"[":"(")+row.expression+(bracket?"]":")"),
    mode=="VFP9"?row.old:row.modern,{},true);
 std::error_code ignored;
 fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv) {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
 set_env_value("COPPERFIN_LOCALE","en-US",true);
 if(argc==1) {
  direct_cases();
  expect(direct_count==248&&public_count==0,"ListItem complete direct count");
 } else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
           (std::string(argv[2])=="1"||std::string(argv[2])=="2")) {
  public_cases(argv[1],std::string(argv[2])=="1"?1:2);
  expect(direct_count==0&&public_count==944,"ListItem complete public count");
 } else {std::cerr<<"Expected no args or COPPERFIN|VFP9 1|2\n";return 2;}
 std::cout<<"ListItem frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

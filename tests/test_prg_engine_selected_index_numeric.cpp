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
// RQ-CF-PRG-SELECTED-INDEX-NUMERIC-001 / VR-5611-SELECTED-INDEX-NUMERIC-001/002.
// Independent native/help/parent-derived literals, never helper output.
struct Row {const char* text; double raw; std::optional<std::int32_t> modern,old;};
const std::vector<Row> rows{
 {"0",0,0,0},{"-0",-0.,0,0},{"0.49",.49,0,0},{"0.5",.5,0,0},{"0.9",.9,0,0},
 {"1",1,1,1},{"1.49",1.49,1,1},{"1.5",1.5,1,1},{"1.9",1.9,1,1},
 {"2",2,2,2},{"2.49",2.49,2,2},{"2.5",2.5,2,2},{"2.9",2.9,2,2},
 {"3",3,3,3},{"3.49",3.49,3,3},{"3.5",3.5,3,3},{"3.9",3.9,3,3},{"4",4,4,4},
 {"-0.49",-.49,0,0},{"-0.5",-.5,0,0},{"-0.9",-.9,0,0},
 {"-1",-1,{},{}},{"-1.9",-1.9,{},{}},
 {"2147483647",2147483647.,2147483647,2147483647},
 {"2147483647.1",2147483647.1,2147483647,{}},
 {"2147483647.5",2147483647.5,2147483647,{}},
 {"2147483647.9",2147483647.9,2147483647,{}},
 {"2147483648",2147483648.,{},{}},{"-2147483648",-2147483648.,{},{}},
 {"-2147483648.9",-2147483648.9,{},{}},
 {"-2147483649",-2147483649.,{},2147483647},
 {"-2147483649.9",-2147483649.9,{},2147483647},
 {"4294967295",4294967295.,{},{}},{"4294967296",4294967296.,{},{}},
 {"4294967297",4294967297.,{},{}},{"4294967297.9",4294967297.9,{},{}},
 {"-4294967295",-4294967295.,{},1},{"-4294967295.9",-4294967295.9,{},1},
 {"-4294967294.5",-4294967294.5,{},2},{"-4294967293",-4294967293.,{},3},
 {"-4294967296",-4294967296.,{},0},{"-4294967296.5",-4294967296.5,{},0},
 {"-4294967297",-4294967297.,{},{}},
 {"9007199254740992",9007199254740992.,{},{}},
 {"-9007199254740992",-9007199254740992.,{},0},
 {"1E20",1E20,{},{}},{"-1E20",-1E20,{},0},
 {"1E300",1E300,{},{}},{"-1E300",-1E300,{},0},
 {"EXP(1000)",std::numeric_limits<double>::infinity(),{},{}},
 {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{},0},
 {"1E-300",1E-300,0,0},{"-1E-300",-1E-300,0,0}
};
int direct_count=0,public_count=0;
void direct_cases() {
 expect(rows.size()==53,"Selected frozen Numeric count");
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  const auto check=[&](const PrgValue& value,std::optional<std::int32_t> modern,
                       std::optional<std::int32_t> old,const char* label) {
   ++direct_count;
   expect(checked_selected_index_numeric_argument(value,mode)==(mode==NumericBehavior::vfp9?old:modern),
          std::string("Selected direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+label);
  };
  for(const auto& row:rows) check(make_number_value(row.raw),row.modern,row.old,row.text);
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},{},"derived NaN");
  check(make_number_value(std::nextafter(2147483648.,0.)),2147483647,{},"converted upper inner");
  check(make_number_value(std::nextafter(2147483647.,INFINITY)),2147483647,{},"raw ceiling outer");
  check(make_number_value(std::nextafter(2147483648.,INFINITY)),{},{},"converted upper outer");
  check(make_number_value(std::nextafter(1.,0.)),0,0,"binary one inner");
  check(make_number_value(std::nextafter(-1.,0.)),0,0,"negative inner");
  check(make_number_value(std::nextafter(-1.,-INFINITY)),{},{},"negative outer");
  check(make_number_value(std::nextafter(0.,1.)),0,0,"positive subnormal");
  check(make_number_value(std::nextafter(0.,-1.)),0,0,"negative subnormal");
  struct Signed {std::int64_t raw;std::optional<std::int32_t> modern,old;};
  for(const auto& row:std::vector<Signed>{
   {0,0,0},{1,1,1},{3,3,3},{2147483647,2147483647,2147483647},
   {2147483648LL,{},{}},{-1,{},{}},{-2147483648LL,{},{}},{-2147483649LL,{},2147483647},
   {-4294967295LL,{},1},{-4294967294LL,{},2},{-4294967293LL,{},3},{-4294967296LL,{},0},
   {-4294967297LL,{},{}},{9007199254740993LL,{},{}},{-9007199254740991LL,{},1},
   {INT64_MIN,{},0},{INT64_MIN+2,{},2},{INT64_MIN+3,{},3},{INT64_MAX,{},{}}})
    check(make_int64_value(row.raw),row.modern,row.old,"exact signed64");
  struct Unsigned {std::uint64_t raw;std::optional<std::int32_t> modern,old;};
  for(const auto& row:std::vector<Unsigned>{
   {0,0,0},{1,1,1},{3,3,3},{2147483647ULL,2147483647,2147483647},
   {2147483648ULL,{},{}},{4294967296ULL,{},{}},{9007199254740993ULL,{},{}},
   {9223372036854775808ULL,{},{}},{UINT64_MAX,{},{}}})
    check(make_uint64_value(row.raw),row.modern,row.old,"exact unsigned64");
  PrgValue typed_null=make_number_value(1.9);
  typed_null.is_null=true;
  check(typed_null,{},{},"typed Numeric NULL adapter");
  check(PrgValue{.kind=PrgValueKind::currency,.currency_value=19000},{},{},"Currency adapter");
  check(make_boolean_value(true),{},{},"Logical adapter");
  check(make_string_value("1.9"),{},{},"Character adapter");
  check(make_null_value(),{},{},"NULL adapter");
 }
}
struct Exact {const char* expression;std::optional<std::int32_t> modern,old;};
const std::vector<Exact> exacts{
 {"CAST(0 AS INT64)",0,0},{"CAST(1 AS INT64)",1,1},{"CAST(3 AS INT64)",3,3},
 {"CAST(-1 AS INT64)",{},{}},{"CAST(2147483647 AS INT64)",2147483647,2147483647},
 {"CAST(2147483648 AS INT64)",{},{}},{"CAST(-2147483648 AS INT64)",{},{}},
 {"(CAST(-2147483648 AS INT64)-CAST(1 AS INT64))",{},2147483647},
 {"CAST(-4294967295 AS INT64)",{},1},{"CAST(-4294967294 AS INT64)",{},2},
 {"CAST(-4294967293 AS INT64)",{},3},{"CAST(-4294967296 AS INT64)",{},0},
 {"(CAST(9007199254740992 AS INT64)+CAST(1 AS INT64))",{},{}},
 {"(CAST(-9007199254740992 AS INT64)+CAST(1 AS INT64))",{},1},
 {"CAST(-9223372036854775808 AS INT64)",{},0},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(2 AS INT64))",{},2},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(3 AS INT64))",{},3},
 {"CAST(3 AS UINT64)",3,3},{"CAST(2147483647 AS UINT64)",2147483647,2147483647},
 {"CAST(2147483648 AS UINT64)",{},{}},
 {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{},{}}
};
std::string get(const RuntimePauseState& state,const char* key) {
 const auto it=state.globals.find(key);
 return it==state.globals.end()?"<missing>":format_value(it->second);
}
void public_case(const fs::path& path,const std::string& mode,int selected,const std::string& cls,
                 bool bracket,const std::string& input,std::optional<std::int32_t> index,
                 const std::string& message={},bool full_expression=false) {
 ++public_count;
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto expression=full_expression?input:"oList.Selected"+std::string(bracket?"[":"(")+input+(bracket?"]":")");
 const auto tag=mode+" session"+std::to_string(selected)+" "+cls+" "+expression;
 write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC cResult,nCode,cMessage,lAfter,cBefore,cAfter,cOther,cReset,nGuard,oList\n"
  "cResult='sentinel'\nnCode=0\ncMessage=''\nlAfter=.F.\n"
  "CREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "oList=CREATEOBJECT('"+cls+"')\noList.AddItem('first')\noList.AddItem('second')\noList.AddItem('third')\n"
  "oList.ItemData(1)=101\noList.ItemData(2)=202\noList.ItemData(3)=303\noList.ListIndex=2\n"
  "cBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\ncResult="+expression+"\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\n"
  "lAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\n"
  "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
  "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"Selected completion "+tag+": "+state.message);
 expect(get(state,"ncode")==(index?"0":"31"),"Selected error "+tag+" got "+get(state,"ncode"));
 const auto result=index && *index==2 ? "true" : "false";
 expect(get(state,"cresult")==(index?result:"sentinel"),"Selected result "+tag+" got "+get(state,"cresult"));
 const auto found=state.globals.find("cresult");
 expect(found!=state.globals.end()&&found->second.kind==(index?PrgValueKind::boolean:PrgValueKind::string),"Selected kind "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","Selected continuation "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"cbefore")==context&&get(state,"cafter")==context,"Selected context "+tag);
 expect(get(state,"cother")==(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|","Selected isolation/reset "+tag);
 if(index) expect(get(state,"cmessage").empty(),"Selected successful message "+tag);
 if(!message.empty()) expect(get(state,"cmessage")==message,"Selected localized31 "+tag);
 bool object_seen=false;
 for(const auto& object:state.ole_objects) if(copperfin::runtime::lowercase_copy(object.base_class_name)==copperfin::runtime::lowercase_copy(cls)) {
  object_seen=true;
  expect(object.list_rows.size()==3&&object.list_item_data.size()==3,"Selected count "+tag);
  if(object.list_item_data.size()==3)
   expect(format_value(object.list_item_data[0])=="101"&&format_value(object.list_item_data[1])=="202"&&
          format_value(object.list_item_data[2])=="303","Selected preserved payload "+tag);
  expect(object.list_selected==std::vector<bool>({false,true,false}),"Selected preserved bits "+tag);
  const auto selected_row=object.properties.find("listindex");
  expect(selected_row!=object.properties.end()&&format_value(selected_row->second)=="2","Selected preserved selection "+tag);
 }
 expect(object_seen,"Selected native object captured "+tag);
 for(const auto& event:state.events)
  expect(event.category!="prg.object.selected"&&event.category!="ole.invoke","Selected no external/success event "+tag);
}
void public_cases(const std::string& mode,int selected) {
 expect(rows.size()==53&&exacts.size()==21,"Selected frozen public input counts");
 const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
 const auto dir=fs::temp_directory_path()/("copperfin_selected_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
 fs::create_directories(dir);
 const auto path=dir/"query.prg";
 for(const std::string cls:{"ListBox","ComboBox"}) for(const bool bracket:{false,true}) {
  for(const auto& row:rows) public_case(path,mode,selected,cls,bracket,row.text,mode=="VFP9"?row.old:row.modern);
  for(const auto& row:exacts) public_case(path,mode,selected,cls,bracket,row.expression,mode=="VFP9"?row.old:row.modern);
  // Preserved coercions, not native type admission claims.
  public_case(path,mode,selected,cls,bracket,".T.",1);
  public_case(path,mode,selected,cls,bracket,".F.",0);
  public_case(path,mode,selected,cls,bracket,".NULL.",0);
  public_case(path,mode,selected,cls,bracket,"'1.9'",2);
  public_case(path,mode,selected,cls,bracket,"$1.9",2);
  for(const auto& locale:std::vector<std::pair<std::string,std::string>>{
   {"en-US","Subscript is outside defined range."},
   {"es-419","El subíndice está fuera del intervalo definido."},
   {"pt-BR","O subscrito está fora do intervalo definido."},
   {"qps-ploc","[!! Šµþšçŕïþţ ïš øµţšïðë ðëƒïñëð ŕåñğë. !!]"}}) {
    set_env_value("COPPERFIN_LOCALE",locale.first,true);
    public_case(path,mode,selected,cls,bracket,"-1",{},locale.second);
   }
  set_env_value("COPPERFIN_LOCALE","en-US",true);
 }
 // VR-5611-SELECTED-INDEX-NUMERIC-002: postfix routes must not reformat a
 // Numeric/exact selector or bypass its error boundary. Independent literals.
 const std::vector<Exact> postfix{
  {"1.9",1,1},{"0.5",0,0},{"-4294967295",{},1},
  {"2147483647.5",2147483647,{}},{"2147483648",{},{}},{"-1",{},{}},
  {"(CAST(-9223372036854775808 AS INT64)+CAST(2 AS INT64))",{},2},
  {"(CAST(-9223372036854775808 AS INT64)+CAST(3 AS INT64))",{},3},
  {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{},{}}
 };
 expect(postfix.size()==9,"Selected frozen postfix count");
 for(const std::string cls:{"ListBox","ComboBox"}) for(const bool bracket:{false,true})
  for(const auto& row:postfix)
   public_case(path,mode,selected,cls,bracket,
    "IIF(.T.,oList,.NULL.).Selected"+std::string(bracket?"[":"(")+row.expression+(bracket?"]":")"),
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
  expect(direct_count==190&&public_count==0,"Selected complete direct count");
 } else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
           (std::string(argv[2])=="1"||std::string(argv[2])=="2")) {
  public_cases(argv[1],std::string(argv[2])=="1"?1:2);
  expect(direct_count==0&&public_count==368,"Selected complete public count");
 } else {std::cerr<<"Expected no args or COPPERFIN|VFP9 1|2\n";return 2;}
 std::cout<<"Selected frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

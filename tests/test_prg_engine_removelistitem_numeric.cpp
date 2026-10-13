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
// RQ-CF-PRG-REMOVELISTITEM-ID-NUMERIC-001 / VR-5611-REMOVELISTITEM-ID-NUMERIC-001/002.
// Literal independently recovered argument domain/stable-ID address, NOT helper output.
// Numeric precision/binary edges and exact64 are named-parent derivations.
// Successful shared focus/text/top/return are excluded gaps #7163; types #7164.
struct Row { const char* expression; double raw; O modern, legacy; };
const std::vector<Row> rows{
 {"0",0,{},{}},
 {"-0.9",-.9,{},{}},
 {"0.9",.9,{},{}},
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
 {"-1",-1,{},{}},
 {"-1.9",-1.9,{},{}},
 {"32766",32766,32766,32766},
 {"32766.9",32766.9,32766,32766},
 {"32767",32767,32767,32767},
 {"32767.1",32767.1,32767,32767},
 {"32767.9",32767.9,32767,32767},
 {"32768",32768,{},{}},
 {"32768.9",32768.9,{},{}},
 {"32769",32769,{},{}},
 {"32769.9",32769.9,{},{}},
 {"65535",65535,{},{}},
 {"65537",65537,{},{}},
 {"2147483647",2147483647,{},{}},
 {"2147483647.9",2147483647.9,{},{}},
 {"2147483648",2147483648,{},{}},
 {"4294967295",4294967295,{},{}},
 {"4294967295.9",4294967295.9,{},{}},
 {"4294967296",4294967296,{},{}},
 {"4294967297",4294967297,{},1},
 {"4294967298.9",4294967298.9,{},2},
 {"4294967299",4294967299,{},3},
 {"4294967307.9",4294967307.9,{},11},
 {"8589934593",8589934593,{},1},
 {"-4294967295.9",-4294967295.9,{},1},
 {"-4294967294.5",-4294967294.5,{},2},
 {"-4294967293",-4294967293,{},3},
 {"-4294967285.5",-4294967285.5,{},11},
 {"-4294967296",-4294967296,{},{}},
 {"-4294967297",-4294967297,{},{}},
 {"-8589934590",-8589934590,{},2},
 {"-8589934591.9",-8589934591.9,{},1},
 {"4295000063",4295000063,{},32767},
 {"4295000063.9",4295000063.9,{},32767},
 {"-4294934529",-4294934529,{},32767},
 {"-4294934529.9",-4294934529.9,{},32767},
 {"4295000064",4295000064,{},{}},
 {"-4294934528",-4294934528,{},{}},
 {"4295032831",4295032831,{},{}},
 {"-4294901761",-4294901761,{},{}},
 {"9007199254740991",9007199254740991,{},{}},
 {"-9007199254740991",-9007199254740991,{},1},
 {"9007199254740992",9007199254740992,{},{}},
 {"-9007199254740992",-9007199254740992,{},{}},
 {"1E300",1E300,{},{}},
 {"-1E300",-1E300,{},{}},
 {"EXP(1000)",std::numeric_limits<double>::infinity(),{},{}},
 {"-EXP(1000)",-std::numeric_limits<double>::infinity(),{},{}},
 {"1E-300",1E-300,{},{}},
 {"-1E-300",-1E-300,{},{}}
};
struct Exact {const char* expression; O modern,legacy;};
const std::vector<Exact> exacts{
 {"CAST(0 AS INT64)",{},{}},
 {"CAST(1 AS INT64)",1,1},
 {"CAST(2 AS INT64)",2,2},
 {"CAST(3 AS INT64)",3,3},
 {"CAST(11 AS INT64)",11,11},
 {"CAST(22 AS INT64)",22,22},
 {"CAST(33 AS INT64)",33,33},
 {"CAST(32767 AS INT64)",32767,32767},
 {"CAST(32768 AS INT64)",{},{}},
 {"CAST(4294967307 AS INT64)",{},11},
 {"CAST(-4294967285 AS INT64)",{},11},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(11 AS INT64))",{},11},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(22 AS INT64))",{},22},
 {"CAST(11 AS UINT64)",11,11},
 {"CAST(4294967307 AS UINT64)",{},11},
 {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",{},{}},
 {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64)-CAST(4294967284 AS UINT64))",{},11},
 {"(CAST(9223372036854775808 AS UINT64)+CAST(11 AS UINT64))",{},11}
};
int direct_count=0,public_count=0;
void direct_cases() {
 expect(rows.size()==69 && exacts.size()==18,"RemoveListItem frozen literal counts");
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}) {
  const auto check=[&](const PrgValue& v,O modern,O legacy,const char* label) {
   ++direct_count;
   expect(checked_removelistitem_id_argument(v,mode)==(mode==NumericBehavior::vfp9?legacy:modern),
    std::string("RemoveListItem direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+label);
  };
  for(const auto& r:rows) check(make_number_value(r.raw),r.modern,r.legacy,r.expression);
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{}, {},"derived NaN");
  check(make_number_value(std::nextafter(1.,0.)),{}, {},"one below");
  check(make_number_value(std::nextafter(1.,2.)),1,1,"one above");
  check(make_number_value(std::nextafter(3.,0.)),2,2,"three below");
  check(make_number_value(std::nextafter(32768.,0.)),32767,32767,"upper inner");
  check(make_number_value(std::nextafter(32768.,INFINITY)),{}, {},"upper outer");
  check(make_number_value(std::nextafter(0.,1.)),{}, {},"positive subnormal");
  check(make_number_value(std::nextafter(0.,-1.)),{}, {},"negative subnormal");
  struct S {std::int64_t raw; O modern,legacy;};
  for(const auto& r:std::vector<S>{
   {0,{},{}},
   {1,1,1},
   {2,2,2},
   {3,3,3},
   {11,11,11},
   {22,22,22},
   {33,33,33},
   {32767,32767,32767},
   {32768,{},{}},
   {INT32_MAX,{},{}},
   {INT32_MIN,{},{}},
   {4294967307LL,{},11},
   {-4294967285LL,{},11},
   {INT64_MIN,{},{}},
   {INT64_MIN+11,{},11},
   {INT64_MIN+22,{},22},
   {INT64_MIN+32767,{},32767},
   {INT64_MIN+32768,{},{}},
   {INT64_MAX,{},{}}})
   check(make_int64_value(r.raw),r.modern,r.legacy,"exact signed64");
  struct U {std::uint64_t raw; O modern,legacy;};
  for(const auto& r:std::vector<U>{
   {0,{},{}},
   {1,1,1},
   {2,2,2},
   {3,3,3},
   {11,11,11},
   {32767,32767,32767},
   {32768,{},{}},
   {4294967307ULL,{},11},
   {9223372036854775819ULL,{},11},
   {UINT64_MAX,{},{}},
   {UINT64_MAX-4294967284ULL,{},11}})
   check(make_uint64_value(r.raw),r.modern,r.legacy,"exact unsigned64");
  auto typed_null=make_number_value(2.9);typed_null.is_null=true;
  check(typed_null,{}, {},"typed Numeric NULL");
  check(make_null_value(),{}, {},"NULL guard");
  check(make_currency_value(25000),{}, {},"Currency guard");
  check(make_boolean_value(true),{}, {},"Logical guard");
  check(make_string_value("2"),{}, {},"Character guard");
 }
}
std::string get(const RuntimePauseState& s,const char* key) {
 const auto it=s.globals.find(key);return it==s.globals.end()?"<missing>":format_value(it->second);
}
void public_case(const fs::path& path,const std::string& mode,int selected,
 const std::string& cls,bool dense,const std::string& input,O projection,
 bool empty=false,bool multi=false,const std::string& message={},
 const std::string& configuration={},bool high=false) {
 ++public_count;const auto other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const std::string tag=mode+" session"+std::to_string(selected)+" "+cls+
  (dense?" dense ":" sparse ")+(empty?"empty ":"")+(multi?"multi ":"")+(high?"high-ID ":"")+configuration+input;
 const auto populate=empty?std::string{}:high?std::string("o.AddListItem('first',32767)\n"):dense?
  "o.AddItem('first')\no.AddItem('second')\no.AddItem('third')\n":
  "o.AddListItem('first',11)\no.AddListItem('second',22)\no.AddListItem('third',33)\n";
 write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+std::string(other)+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC o,nCode,cMessage,lReturned,lAfter,cBefore,cAfter,cOther,cReset,nGuard,nFocusBefore,nIDBefore,cValueBefore,cDisplayBefore,nTopBefore,nTopIDBefore,nBefore\n"
  "nCode=0\ncMessage=''\nlReturned=.F.\nlAfter=.F.\nCREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "o=CREATEOBJECT('"+cls+"')\no.ColumnCount=2\n"+configuration+populate+
  (empty?"":high?"o.List(1,2)='cell1'\no.ItemData(1)=101\n":"o.List(1,2)='cell1'\no.List(2,2)='cell2'\no.List(3,2)='cell3'\no.ItemData(1)=101\no.ItemData(2)=202\no.ItemData(3)=303\n")+
  (multi?"o.MultiSelect=.T.\no.Selected(1)=.T.\no.Selected(2)=.T.\n":"")+
  (empty?"":high?"o.ListIndex=1\n":"o.ListIndex=2\n")+
  (cls=="ListBox"&&!empty?(high?"o.TopIndex=1\n":"o.TopIndex=2\n"):"")+
  "nBefore=o.ListCount\nnFocusBefore=o.ListIndex\nnIDBefore=o.ListItemID\ncValueBefore=o.Value\ncDisplayBefore=o.DisplayValue\n"+
  (cls=="ListBox"?"nTopBefore=o.TopIndex\nnTopIDBefore=o.TopItemID\n":"")+
  "cBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\no.RemoveListItem("+input+")\nlReturned=.T.\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\nlAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\n"
  "cAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\ncReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
 const auto state=session.run(DebugResumeAction::continue_run);
 // Independent stable-ID lookup expectations, never helper output.
 const int slot=!projection||empty?0:high?(*projection==32767?1:0):
  dense?(*projection>=1&&*projection<=3?*projection:0):
  *projection==11?1:*projection==22?2:*projection==33?3:0;
 const int code=!projection?11:slot==0?1234:0;
 expect(state.completed,"RemoveListItem completion "+tag+": "+state.message);
 expect(get(state,"nbefore")==std::to_string(empty?0:high?1:3),"RemoveListItem explicit populated precondition "+tag);
 expect(get(state,"ncode")==std::to_string(code),"RemoveListItem error "+tag+" got "+get(state,"ncode"));
 expect(get(state,"lreturned")==(code?"false":"true"),"RemoveListItem statement continuation "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","RemoveListItem catchability "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"cbefore")==context&&get(state,"cafter")==context,"RemoveListItem context "+tag);
 expect(get(state,"cother")==(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|","RemoveListItem session/reset "+tag);
 if(!message.empty()) expect(get(state,"cmessage")==message,"RemoveListItem localized message "+tag);
 const int removed=code==0?slot:0;bool seen=false;
 for(const auto& object:state.ole_objects) if(copperfin::runtime::lowercase_copy(object.base_class_name)==copperfin::runtime::lowercase_copy(cls)) {
  seen=true;const auto prop=[&](const char* key,const std::string& expected) {
   const auto it=object.properties.find(key);expect(it!=object.properties.end()&&format_value(it->second)==expected,
    std::string("RemoveListItem state ")+key+" "+tag+(it==object.properties.end()?" missing":" got "+format_value(it->second)));
  };
  const std::size_t before_count=empty?0:high?1:3;
  const std::size_t count=before_count-(removed?1:0);
  prop("listcount",std::to_string(count));
  expect(object.list_rows.size()==count&&object.collection_items.size()==count&&object.list_item_data.size()==count,
   "RemoveListItem coherent bounded count "+tag);
  std::vector<std::string> keys,cells,data;
  for(int row=1;row<=(high?1:3)&&!empty;++row) if(row!=removed) {
   keys.push_back(std::to_string(high?32767:row*(dense?1:11)));
   cells.push_back(row==1?"first":row==2?"second":"third");
   data.push_back(std::to_string(row*101));
  }
  expect(object.collection_item_keys==keys,"RemoveListItem independent surviving IDs "+tag);
  if(object.list_rows.size()==count&&object.list_item_data.size()==count) {
   std::size_t slot=0;
   for(int row=1;row<=(high?1:3)&&!empty;++row) if(row!=removed) {
    expect(object.list_rows[slot].size()==2&&format_value(object.list_rows[slot][0])==cells[slot]&&
      format_value(object.list_rows[slot][1])=="cell"+std::to_string(row),"RemoveListItem independent cells "+tag);
    expect(format_value(object.list_item_data[slot])==data[slot],"RemoveListItem independent payload "+tag);++slot;
   }
  }
  if(code!=0) {
   prop("listindex",get(state,"nfocusbefore"));prop("listitemid",get(state,"nidbefore"));
   prop("value",get(state,"cvaluebefore"));prop("displayvalue",get(state,"cdisplaybefore"));
   if(cls=="ListBox") {prop("topindex",get(state,"ntopbefore"));prop("topitemid",get(state,"ntopidbefore"));}
   expect(object.list_selected==(empty?std::vector<bool>{}:high?std::vector<bool>{true}:multi?std::vector<bool>{true,true,false}:
    std::vector<bool>{false,true,false}),"RemoveListItem rejection preserves selection "+tag);
  }
  // NO successful shared focus/text/top/return claim: #7163 remains excluded.
 }
 expect(seen,"RemoveListItem native state captured "+tag);
 for(const auto& event:state.events) expect(event.category!="ole.invoke","RemoveListItem no external invocation "+tag);
}
void public_cases(const std::string& mode,int selected) {
 const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
 const auto dir=fs::temp_directory_path()/("copperfin_removelistitem_numeric_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
 fs::create_directories(dir);const auto path=dir/"query.prg";const bool legacy=mode=="VFP9";
 for(const std::string cls:{"ListBox","ComboBox"}) for(const bool dense:{false,true}) {
  for(const auto& r:rows) public_case(path,mode,selected,cls,dense,r.expression,legacy?r.legacy:r.modern);
  for(const auto& r:exacts) public_case(path,mode,selected,cls,dense,r.expression,legacy?r.legacy:r.modern);
  public_case(path,mode,selected,cls,dense,"(EXP(1000)-EXP(1000))",{});
 }
 for(const std::string cls:{"ListBox","ComboBox"}) for(const auto& r:rows)
  public_case(path,mode,selected,cls,false,r.expression,legacy?r.legacy:r.modern,true);
 for(const bool dense:{false,true}) for(const auto& r:rows)
  public_case(path,mode,selected,"ListBox",dense,r.expression,legacy?r.legacy:r.modern,false,true);
 // Exact typed-zero configuration is an owner-derived extension boundary.
 const std::vector<Row> guards{
  {"11.9",11.9,11,11},{"22.9",22.9,22,22},{"2.9",2.9,2,2},{"0",0,{},{}},
  {"32768",32768,{},{}},{"4294967307.9",4294967307.9,{},11},
  {"-4294967285.5",-4294967285.5,{},11}};
 for(const std::string cls:{"ListBox","ComboBox"})
 for(const std::string configuration:{"","o.RowSourceType=0\n",
  "o.RowSourceType=CAST(0 AS INT64)\n","o.RowSourceType=CAST(0 AS UINT64)\n"})
 for(const std::string binding:{"o.ControlSource=''\n","o.ControlSource='   '\n"})
 for(const auto& r:guards) public_case(path,mode,selected,cls,false,r.expression,
  legacy?r.legacy:r.modern,false,false,{},configuration+binding);
 const std::vector<Row> high_cases{
  {"32767",32767,32767,32767},{"32767.9",32767.9,32767,32767},
  {"32768",32768,{},{}},{"4295000063.9",4295000063.9,{},32767},
  {"-4294934529.9",-4294934529.9,{},32767},{"1",1,1,1}};
 for(const std::string cls:{"ListBox","ComboBox"}) for(const auto& r:high_cases)
  public_case(path,mode,selected,cls,false,r.expression,legacy?r.legacy:r.modern,
   false,false,{},"o.RowSourceType=0\n",true);
 for(const auto& locale:std::vector<std::vector<std::string>>{
  {"en-US","Function argument value, type, or count is invalid.","Subscript is outside defined range."},
  {"es-419","El valor, tipo o numero de un argumento de funcion no es valido.","El subíndice está fuera del intervalo definido."},
  {"pt-BR","O valor, tipo ou numero de um argumento de funcao e invalido.","O subscrito está fora do intervalo definido."},
  {"qps-ploc","[!! Füñçţïøñ årĝümëñţ vålüë, ţÿþë, ør çøüñţ ïš ïñvålïð. !!]","[!! Šµþšçŕïþţ ïš øµţšïðë ðëƒïñëð ŕåñğë. !!]"}}) {
  set_env_value("COPPERFIN_LOCALE",locale[0],true);
  for(const std::string cls:{"ListBox","ComboBox"}) {
   public_case(path,mode,selected,cls,false,"0",{},false,false,locale[1]);
   public_case(path,mode,selected,cls,false,"4",4,false,false,locale[2]);
   public_case(path,mode,selected,cls,false,"11.9",11);
  }
 }
 set_env_value("COPPERFIN_LOCALE","en-US",true);std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv) {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 if(argc==1) {direct_cases();expect(direct_count==224&&public_count==0,"RemoveListItem complete direct count");}
 else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
  (std::string(argv[2])=="1"||std::string(argv[2])=="2")) {
  public_cases(argv[1],std::string(argv[2])=="1"?1:2);
  expect(direct_count==0&&public_count==776,"RemoveListItem complete public count");
 } else {std::cerr<<"Expected no args or COPPERFIN|VFP9 1|2\n";return 2;}
 std::cout<<"RemoveListItem frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

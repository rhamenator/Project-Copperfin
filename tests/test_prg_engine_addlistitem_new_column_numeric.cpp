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
using Candidate=std::optional<std::size_t>;
// RQ-CF-PRG-ADDLISTITEM-NEW-COLUMN-NUMERIC-001 / VR-5611-ADDLISTITEM-NEW-COLUMN-NUMERIC-001/002.
// Independent literals from THIS consumer's retained780/100/384/144 native
// observations plus named default/allocation/exact64/NaN numeric parents.
// null rejects11; engaged0 ADMITS without a candidate. This is NOT the existing
// ID no-low16 table. Only candidate1 reaches the preserved shared new-row arm;
// all other admitted columns retain #7186's no-row fallback, NOT native parity.
// No helper-generated expectations. Unsafe ORIGINAL public reach is excluded
// explicitly; fixed-only vectors below are authored BEFORE caller migration.
struct NumberCase {const char* source;double value;Candidate modern,legacy;bool originalSafe;};
const std::vector<NumberCase> numbers{
 {"-3",-3,{},{},true},{"-1.9",-1.9,{},{},true},
 {"-0.9",-0.9,{},{},true},{"0",0,{},{},true},
 {"0.49",0.49,{},{},true},{"0.5",0.5,{},{},true},{"0.99",0.99,{},{},true},
 {"1-2^(-53)",1.-std::ldexp(1.,-53),{},{},true},
 {"1",1,1,1,true},{"1+2^(-52)",1.+std::ldexp(1.,-52),1,1,true},
 {"1.5",1.5,1,1,true},{"1.9",1.9,1,1,true},
 {"2.1",2.1,2,2,true},{"2.5",2.5,2,2,true},{"2.9",2.9,2,2,true},
 {"8.9",8.9,8,8,true},{"9",9,9,9,true},
 {"32744.9",32744.9,32744,32744,true},
 {"32745",32745,32745,32745,true},{"32745.49",32745.49,32745,32745,true},
 {"32745.5",32745.5,32745,32745,true},{"32745.9",32745.9,32745,32745,true},
 {"32746-2^(-38)",32746.-std::ldexp(1.,-38),32745,32745,true},
 {"32746",32746,{},0,true},{"32746.5",32746.5,{},0,true},
 {"32767.9",32767.9,{},0,true},{"32768",32768,{},0,true},
 {"65535.9",65535.9,{},0,true},{"65536",65536,{},0,true},
 {"65536.9",65536.9,{},0,true},{"65537",65537,{},1,true},
 {"65537.9",65537.9,{},1,true},{"65538",65538,{},2,true},
 {"65538.9",65538.9,{},2,false},{"131073.9",131073.9,{},1,false},
 {"2147483647.9",2147483647.9,{},0,false},
 {"2147483648",2147483648,{},0,false},{"2147483648.9",2147483648.9,{},0,false},
 {"2147483649",2147483649,{},{},false},
 {"4294967295.9",4294967295.9,{},{},false},
 {"4294967296",4294967296,{},{},false},
 {"4294967297.9",4294967297.9,{},1,false},
 {"4294967298.9",4294967298.9,{},2,false},
 {"4294967296+32745.9",4294967296.+32745.9,{},32745,false},
 {"4294967296+32746",4294967296.+32746,{},0,false},
 {"-2147483648",-2147483648,{},0,true},
 {"-2147483648.9",-2147483648.9,{},0,true},
 {"-2147483649",-2147483649,{},0,true},
 {"-4294967295.9",-4294967295.9,{},1,true},
 {"-4294967294.9",-4294967294.9,{},2,true},
 {"-4294967296",-4294967296,{},{},true},
 {"-4294967296+32745",-4294967296.+32745,{},32745,true},
 {"-4294967296+32746",-4294967296.+32746,{},0,true},
 {"6442450944",6442450944,{},0,false},
 {"6442450945",6442450945,{},{},false},
 {"-6442450944",-6442450944,{},0,true},
 {"-6442450943",-6442450943,{},{},true},
 {"1E-300",1E-300,{},{},true},
 {"9007199254740992",9007199254740992,{},{},false},
 {"1E20",1E20,{},{},false},{"-1E20",-1E20,{},{},false},
 {"1E300",1E300,{},{},false},{"-1E300",-1E300,{},{},false},
 {"(1E300*1E300)",INFINITY,{},{},false},
 {"(-1E300*1E300)",-INFINITY,{},{},false}
};
struct ExactCase {const char* source;PrgValue value;Candidate modern,legacy;bool originalSafe;};
const std::vector<ExactCase> exacts{
 {"CAST(0 AS INT64)",make_int64_value(0),{},{},true},
 {"CAST(1 AS INT64)",make_int64_value(1),1,1,true},
 {"CAST(32745 AS INT64)",make_int64_value(32745),32745,32745,true},
 {"CAST(32746 AS INT64)",make_int64_value(32746),{},0,true},
 {"CAST(65537 AS INT64)",make_int64_value(65537),{},1,true},
 {"CAST(-2147483648 AS INT64)",make_int64_value(-2147483648LL),{},0,true},
 {"CAST(-4294967295 AS INT64)",make_int64_value(-4294967295LL),{},1,true},
 {"CAST(4294967297 AS INT64)",make_int64_value(4294967297LL),{},1,false},
 {"CAST(-9223372036854775808 AS INT64)",make_int64_value(INT64_MIN),{},{},false},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(65537 AS INT64))",make_int64_value(INT64_MIN+65537),{},1,false},
 {"(CAST(9223372036854774784 AS INT64)+CAST(1023 AS INT64))",make_int64_value(INT64_MAX),{},{},false},
 {"CAST(0 AS UINT64)",make_uint64_value(0),{},{},true},
 {"CAST(2 AS UINT64)",make_uint64_value(2),2,2,true},
 {"CAST(32745 AS UINT64)",make_uint64_value(32745),32745,32745,true},
 {"CAST(32746 AS UINT64)",make_uint64_value(32746),{},0,true},
 {"CAST(65537 AS UINT64)",make_uint64_value(65537),{},1,true},
 {"(CAST(9007199254740992 AS UINT64)+CAST(1 AS UINT64))",make_uint64_value(9007199254740993ULL),{},1,false},
 {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",make_uint64_value(UINT64_MAX),{},{},false}
};
std::size_t direct_count=0,public_count=0;
void direct_cases(){
 expect(numbers.size()==65&&exacts.size()==18,"AddListItem NEW COLUMN independent table counts");
 for(const auto& c:numbers)if(c.originalSafe)
  expect(std::isfinite(c.value)&&c.value>=-6442450944.&&c.value<=65538.,std::string("AddListItem NEW COLUMN safe reach ")+c.source);
 for(const auto& c:exacts)if(c.originalSafe)
  expect((c.value.kind==PrgValueKind::int64&&c.value.int64_value>=-4294967295LL&&c.value.int64_value<=65538LL)||
   (c.value.kind==PrgValueKind::uint64&&c.value.uint64_value<=65538ULL),std::string("AddListItem NEW COLUMN safe exact reach ")+c.source);
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}){
  const auto check=[&](const PrgValue& value,Candidate modern,Candidate legacy,const char* tag){
   ++direct_count;expect(checked_addlistitem_new_column_argument(value,mode)==(mode==NumericBehavior::vfp9?legacy:modern),
    std::string("AddListItem NEW COLUMN direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+tag);
  };
  for(const auto& c:numbers)check(make_number_value(c.value),c.modern,c.legacy,c.source);
  for(const auto& c:exacts)check(c.value,c.modern,c.legacy,c.source);
  check(make_number_value(std::nextafter(1.,0.)),{},{},"binary below1");
  check(make_number_value(std::nextafter(32746.,0.)),32745,32745,"binary below32746");
  check(make_number_value(std::nextafter(32746.,INFINITY)),{},0,"binary above32746");
  check(make_number_value(std::ldexp(1.,63)),{},{},"positive2^63");
  check(make_number_value(-std::ldexp(1.,63)),{},{},"negative2^63");
  check(make_number_value(std::nextafter(-std::ldexp(1.,63),-INFINITY)),{},{},"below negative2^63");
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},{},"derived NaN");
  check(make_number_value(std::numeric_limits<double>::denorm_min()),{},{},"positive subnormal");
  auto typedNull=make_number_value(1.9);typedNull.is_null=true;
  check(typedNull,{},{},"typed NULL");check(make_null_value(),{},{},"NULL");
  check(make_string_value("1.9"),{},{},"Character");check(make_boolean_value(true),{},{},"Logical");
  check(make_currency_value(19000),{},{},"Currency");
 }
}
std::string get(const RuntimePauseState& state,const char* key){
 const auto found=state.globals.find(key);return found==state.globals.end()?"<missing>":format_value(found->second);
}
void invoke_case(const fs::path& path,const std::string& mode,int selected,const std::string& cls,
 bool sparse,int columns,const std::string& source,Candidate candidate,
 const std::string& config={},const std::string& localized={}){
 ++public_count;
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const int target=sparse?44:11;
 const auto tag=mode+" session"+std::to_string(selected)+" "+cls+" missingID"+std::to_string(target)+
  " columns"+std::to_string(columns)+" "+config+" "+source;
 std::string setup;
 if(sparse){
  for(int row=1;row<=3;++row){const auto s=std::to_string(row);
   setup+="o.AddListItem('seed"+s+"',"+std::to_string(row*11)+")\no.List("+s+",2)='cell"+s+"'\no.ItemData("+s+")="+std::to_string(row*101)+"\n";
  }
  setup+="o.ListIndex=2\n";if(cls=="ListBox")setup+="o.TopIndex=2\n";
 }
 const std::string snapshot="TRANSFORM(o.ListCount)+'|'+TRANSFORM(o.ColumnCount)+'|'+TRANSFORM(o.RowSourceType)+'|'+o.ControlSource+'|'+TRANSFORM(o.ListIndex)+'|'+TRANSFORM(o.ListItemID)+'|'+TRANSFORM(o.Value)+'|'+TRANSFORM(o.DisplayValue)+'|'+TRANSFORM(o.NewIndex)+'|'+TRANSFORM(o.NewItemID)"+
  (cls=="ListBox"?std::string("+'|'+TRANSFORM(o.TopIndex)+'|'+TRANSFORM(o.TopItemID)"):"");
 write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC o,nCode,cMessage,lReturned,lAfter,nResult,cBefore,cAfter,cContextBefore,cContextAfter,cOther,cReset,nBefore,nGuard,nMissing\n"
  "nCode=0\ncMessage=''\nlReturned=.F.\nlAfter=.F.\nnResult=-99\nCREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "o=CREATEOBJECT('"+cls+"')\no.ColumnCount="+std::to_string(columns)+"\n"+config+setup+
  "nBefore=o.ListCount\nnMissing=o.ItemIDToIndex("+std::to_string(target)+")\ncBefore="+snapshot+"\n"
  "cContextBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\nnResult=o.AddListItem('candidate',"+std::to_string(target)+","+source+")\nlReturned=.T.\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\nlAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\ncAfter="+snapshot+"\n"
  "cContextAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\ncReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"AddListItem NEW COLUMN completion "+tag+": "+state.message);
 expect(get(state,"nbefore")==std::string(sparse?"3":"0")&&get(state,"nmissing")=="0","AddListItem NEW COLUMN missing precondition "+tag);
 expect(get(state,"ncode")==std::string(candidate?"0":"11"),"AddListItem NEW COLUMN admission "+tag+" got "+get(state,"ncode"));
 expect(get(state,"lreturned")==std::string(candidate?"true":"false"),"AddListItem NEW COLUMN continuation "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","AddListItem NEW COLUMN catchability "+tag);
 if(!localized.empty())expect(get(state,"cmessage")==localized,"AddListItem NEW COLUMN localized11 "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"ccontextbefore")==context&&get(state,"ccontextafter")==context,"AddListItem NEW COLUMN context "+tag);
 expect(get(state,"cother")==(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|","AddListItem NEW COLUMN session/reset "+tag);
 const bool creates=candidate&&*candidate==1U;
 const auto expectedResult=!candidate?"-99":creates?std::to_string(target):"0";
 expect(get(state,"nresult")==expectedResult,"AddListItem NEW COLUMN preserved result "+tag);
 // NON-NATIVE modeled preservation: #7186 is explicitly NOT repaired. Native
 // recovery creates a row for other admitted columns; no such claim here.
 if(!creates)expect(get(state,"cbefore")==get(state,"cafter"),"AddListItem NEW COLUMN retained no-row metadata "+tag);
 bool seen=false;
 for(const auto& object:state.ole_objects)if(copperfin::runtime::lowercase_copy(object.base_class_name)==copperfin::runtime::lowercase_copy(cls)){
  seen=true;const std::size_t count=(sparse?3U:0U)+(creates?1U:0U);
  const bool coherent=object.list_rows.size()==count&&object.collection_items.size()==count&&
   object.collection_item_keys.size()==count&&object.list_item_data.size()==count&&object.list_selected.size()==count;
  expect(coherent,"AddListItem NEW COLUMN modeled count/coherence "+tag);
  if(coherent){std::size_t candidates=0;
   for(std::size_t row=0;row<count;++row){
    const auto& id=object.collection_item_keys[row];const auto& cells=object.list_rows[row];
    if(id==std::to_string(target)){
     ++candidates;expect(creates&&!cells.empty()&&format_value(cells[0])=="candidate"&&
      format_value(object.collection_items[row])=="candidate"&&format_value(object.list_item_data[row])=="0"&&
      !object.list_selected[row]&&cells.size()<=32745U,"AddListItem NEW COLUMN modeled candidate row "+tag);
    }else{
     const int seed=id=="11"?1:id=="22"?2:id=="33"?3:0;const auto s=std::to_string(seed);
     expect(sparse&&seed!=0&&format_value(object.collection_items[row])=="seed"+s&&
      format_value(object.list_item_data[row])==std::to_string(seed*101),"AddListItem NEW COLUMN retained seed identity/data "+tag);
     bool allCellsMatch=cells.size()>=2U;
     for(std::size_t col=0;col<cells.size();++col)
      if(format_value(cells[col])!=(col==0?"seed"+s:col==1?"cell"+s:""))allCellsMatch=false;
     expect(allCellsMatch,"AddListItem NEW COLUMN complete retained seed cells "+tag);
    }
   }
   expect(candidates==(creates?1U:0U),"AddListItem NEW COLUMN candidate count "+tag);
  }
 }
 expect(seen,"AddListItem NEW COLUMN captured control "+tag);
 for(const auto& event:state.events)expect(event.category!="ole.invoke","AddListItem NEW COLUMN no external invocation "+tag);
}
void public_cases(const std::string& mode,int selected,bool hostile){
 const auto dir=fs::temp_directory_path()/("copperfin_addlistitem_new_column_5611_"+mode+"_"+std::to_string(selected)+"_"+
  std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 fs::create_directories(dir);const auto path=dir/"query.prg";const bool legacy=mode=="VFP9";
 for(const std::string cls:{"ListBox","ComboBox"})for(const bool sparse:{false,true}){
  for(const auto& c:numbers)if(c.originalSafe!=hostile)
   invoke_case(path,mode,selected,cls,sparse,2,c.source,legacy?c.legacy:c.modern);
  for(const auto& c:exacts)if(c.originalSafe!=hostile)
   invoke_case(path,mode,selected,cls,sparse,2,c.source,legacy?c.legacy:c.modern);
 }
 if(hostile){
  for(const std::string cls:{"ListBox","ComboBox"})for(const bool sparse:{false,true})
   invoke_case(path,mode,selected,cls,sparse,2,"(EXP(1000)-EXP(1000))",{});
 }else{
  // Supported configuration BEFORE population; no type/RST/binding expansion.
  const std::vector<NumberCase> guards{{"1.9",1.9,1,1,true},{"0.5",0.5,{},{},true},
   {"32746",32746,{},0,true},{"65537.9",65537.9,{},1,true}};
  const std::vector<std::string> configs{"", "o.RowSourceType=0\no.ControlSource=''\n",
   "o.RowSourceType=0\no.ControlSource='   '\n",
   "o.RowSourceType=CAST(0 AS INT64)\no.ControlSource=CHR(9)+'  '\n",
   "o.RowSourceType=CAST(0 AS UINT64)\no.ControlSource=''\n",
   "o.RowSourceType=0\no.ControlSource=''\no.Sorted=.T.\n"};
  for(const std::string cls:{"ListBox","ComboBox"})for(const bool sparse:{false,true})
   for(const int columns:{0,9})for(const auto& config:configs)for(const auto& c:guards)
    invoke_case(path,mode,selected,cls,sparse,columns,c.source,legacy?c.legacy:c.modern,config);
  for(const auto& locale:std::vector<std::vector<std::string>>{
   {"en-US","Function argument value, type, or count is invalid."},
   {"es-419","El valor, tipo o numero de un argumento de funcion no es valido."},
   {"pt-BR","O valor, tipo ou numero de um argumento de funcao e invalido."},
   {"qps-ploc","[!! Füñçţïøñ årĝümëñţ vålüë, ţÿþë, ør çøüñţ ïš ïñvålïð. !!]"}}){
   set_env_value("COPPERFIN_LOCALE",locale[0],true);
   for(const std::string cls:{"ListBox","ComboBox"}){
    invoke_case(path,mode,selected,cls,true,2,"0.5",{},{},locale[1]);
    invoke_case(path,mode,selected,cls,false,2,"1.9",1);
   }
  }
 }
 set_env_value("COPPERFIN_LOCALE","en-US",true);std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv){
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 if(argc==1){direct_cases();expect(direct_count==192&&public_count==0,"AddListItem NEW COLUMN complete direct count");}
 else if((argc==3||argc==4)&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
  (std::string(argv[2])=="1"||std::string(argv[2])=="2")&&(argc==3||std::string(argv[3])=="--fixed-only-hostile")){
  public_cases(argv[1],std::string(argv[2])=="1"?1:2,argc==4);
  expect(direct_count==0&&public_count==(argc==4?112:432),"AddListItem NEW COLUMN complete public count");
 }else{std::cerr<<"Expected no args or COPPERFIN|VFP9 1|2 [--fixed-only-hostile]\n";return 2;}
 std::cout<<"AddListItem NEW COLUMN frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

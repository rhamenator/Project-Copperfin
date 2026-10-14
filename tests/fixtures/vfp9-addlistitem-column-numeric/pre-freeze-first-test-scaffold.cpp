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
using Column=std::optional<std::size_t>;
// RQ-CF-PRG-ADDLISTITEM-COLUMN-NUMERIC-001 / VR-5611-ADDLISTITEM-COLUMN-NUMERIC-001/002.
// Independent literals from the nine retained VFP9 pairs and named numeric/
// allocation parents, NOT a helper-generated table or AddItem's low16 model.
// Engaged0 means NO CANDIDATE WRITE to an existing ID. Caption1 duplicate and
// shared success state/return remain untouched; no absolute native parity claim.
// originalSafe bounds the old public resize. Unsafe public vectors are separate
// PRE-WIRE-authored fixed-only corroboration, NEVER original qualification.
struct NumberCase {const char* source;double value;Column modern,legacy;bool originalSafe=false;};
const std::vector<NumberCase> numbers{
 {"-3",-3,{},{},true},{"-1.9",-1.9,{},{},true},{"-0.9",-0.9,{},{},true},
 {"0",0,{},{},true},{"0.5",0.5,{},{},true},{"0.9",0.9,{},{},true},
 {"1",1,1,1,true},{"1.4",1.4,1,1,true},{"1.5",1.5,1,1,true},
 {"1.9",1.9,1,1,true},{"2",2,2,2,true},{"2.5",2.5,2,2,true},
 {"2.9",2.9,2,2,true},{"3.9",3.9,3,3,true},{"8.9",8.9,8,8,true},
 {"9",9,9,9,true},{"127",127,127,127,true},{"255",255,255,255,true},
 {"256",256,256,256,true},
 {"32744.1",32744.1,32744,32744,true},{"32744.9",32744.9,32744,32744,true},
 {"32745",32745,32745,32745,true},{"32745.1",32745.1,32745,32745,true},
 {"32745.5",32745.5,32745,32745,true},{"32745.9",32745.9,32745,32745,true},
 {"32746",32746,{},0,true},{"32746.1",32746.1,{},0,true},
 {"32746.9",32746.9,{},0,true},{"32767",32767,{},0,true},
 {"32768",32768,{},0,true},{"65535",65535,{},0,true},
 {"65536",65536,{},0,true},{"65537",65537,{},0,true},
 {"65537.9",65537.9,{},0},
 {"2147483647",2147483647,{},0},{"2147483647.9",2147483647.9,{},0},
 {"2147483648",2147483648,{},0},{"2147483648.9",2147483648.9,{},0},
 {"2147483649",2147483649,{},{}},
 {"-2147483648",-2147483648,{},0,true},
 {"-2147483648.9",-2147483648.9,{},0,true},
 {"-2147483649",-2147483649,{},0,true},
 {"-2147483647.9",-2147483647.9,{},{},true},
 {"4294967296",4294967296,{},{}},
 {"4294967297.9",4294967297.9,{},1},
 {"4294967298.9",4294967298.9,{},2},
 {"-4294967296",-4294967296,{},{},true},
 {"-4294967295.9",-4294967295.9,{},1,true},
 {"-4294967294.9",-4294967294.9,{},2,true},
 {"4294967296+32744.9",4294967296.+32744.9,{},32744},
 {"4294967296+32745.9",4294967296.+32745.9,{},32745},
 {"4294967296+32746.9",4294967296.+32746.9,{},0},
 {"-4294967296+32744.9",-4294967296.+32744.9,{},32745,true},
 {"-4294967296+32745.9",-4294967296.+32745.9,{},0,true},
 {"6442450944",6442450944,{},0},{"6442450945",6442450945,{},{}},
 {"-6442450944",-6442450944,{},0,true},
 {"-6442450945",-6442450945,{},0,true},
 {"8589934592+32745.9",8589934592.+32745.9,{},32745},
 {"-8589934592+32745.9",-8589934592.+32745.9,{},0,true},
 {"9007199254740991",9007199254740991,{},{}},
 {"9007199254740992",9007199254740992,{},{}},
 {"1E300",1E300,{},{}},{"-1E300",-1E300,{},{}},
 {"1E20",1E20,{},{}},{"-1E20",-1E20,{},{}},
 {"1E-300",1E-300,{},{},true},
 {"(1E300*1E300)",INFINITY,{},{}},{"(-1E300*1E300)",-INFINITY,{},{}}
};
struct ExactCase {const char* source;PrgValue value;Column modern,legacy;bool originalSafe=false;};
const std::vector<ExactCase> exacts{
 {"CAST(0 AS INT64)",make_int64_value(0),{},{},true},
 {"CAST(1 AS INT64)",make_int64_value(1),1,1,true},
 {"CAST(32745 AS INT64)",make_int64_value(32745),32745,32745,true},
 {"CAST(32746 AS INT64)",make_int64_value(32746),{},0,true},
 {"CAST(65537 AS INT64)",make_int64_value(65537),{},0,true},
 {"CAST(-2147483648 AS INT64)",make_int64_value(-2147483648LL),{},0,true},
 {"CAST(-4294967295 AS INT64)",make_int64_value(-4294967295LL),{},1,true},
 {"CAST(4294967297 AS INT64)",make_int64_value(4294967297LL),{},1},
 {"CAST(-9223372036854775808 AS INT64)",make_int64_value(INT64_MIN),{},{}},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(1 AS INT64))",make_int64_value(INT64_MIN+1),{},1},
 {"(CAST(9223372036854774784 AS INT64)+CAST(1023 AS INT64))",make_int64_value(INT64_MAX),{},{}},
 {"CAST(0 AS UINT64)",make_uint64_value(0),{},{},true},
 {"CAST(2 AS UINT64)",make_uint64_value(2),2,2,true},
 {"CAST(32745 AS UINT64)",make_uint64_value(32745),32745,32745,true},
 {"CAST(32746 AS UINT64)",make_uint64_value(32746),{},0,true},
 {"CAST(65537 AS UINT64)",make_uint64_value(65537),{},0,true},
 {"(CAST(9007199254740992 AS UINT64)+CAST(1 AS UINT64))",make_uint64_value(9007199254740993ULL),{},1},
 {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",make_uint64_value(UINT64_MAX),{},{}},
 {"(CAST(9223372036854775808 AS UINT64)+CAST(2147483648 AS UINT64))",make_uint64_value(9223372039002259456ULL),{},0}
};
int direct_count=0,public_count=0;
void direct_cases(){
 for(const auto& c:numbers)if(c.originalSafe)
  expect(std::isfinite(c.value)&&c.value>=-std::ldexp(1.,63)&&c.value<=65537.,
   std::string("AddListItem COLUMN original-safe bound ")+c.source);
 for(const auto& c:exacts)if(c.originalSafe)
  expect((c.value.kind==PrgValueKind::int64&&c.value.int64_value>=-4294967295LL&&c.value.int64_value<=65537LL)||
   (c.value.kind==PrgValueKind::uint64&&c.value.uint64_value<=65537ULL),
   std::string("AddListItem COLUMN original-safe exact bound ")+c.source);
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}){
  const auto check=[&](const PrgValue& value,Column modern,Column legacy,const char* label){
   ++direct_count;
   expect(checked_addlistitem_column_argument(value,mode)==(mode==NumericBehavior::vfp9?legacy:modern),
    std::string("AddListItem COLUMN direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+label);
  };
  for(const auto& c:numbers)check(make_number_value(c.value),c.modern,c.legacy,c.source);
  for(const auto& c:exacts)check(c.value,c.modern,c.legacy,c.source);
  check(make_number_value(std::nextafter(32745.,0.)),32744,32744,"binary below32745");
  check(make_number_value(std::nextafter(32745.,INFINITY)),32745,32745,"binary above32745");
  check(make_number_value(std::nextafter(32746.,0.)),32745,32745,"binary below32746");
  check(make_number_value(std::nextafter(32746.,INFINITY)),{},0,"binary above32746");
  check(make_number_value(std::nextafter(1.,0.)),{}, {},"binary below1");
  check(make_number_value(std::nextafter(1.,INFINITY)),1,1,"binary above1");
  check(make_number_value(std::ldexp(1.,63)),{}, {},"binary positive2^63");
  check(make_number_value(-std::ldexp(1.,63)),{}, {},"binary negative2^63");
  check(make_number_value(std::nextafter(-std::ldexp(1.,63),-INFINITY)),{}, {},"binary belownegative2^63");
  check(make_number_value(-std::ldexp(1.,64)),{}, {},"actual negative2^64, not native source spelling");
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{}, {},"derived NaN");
  check(make_number_value(std::numeric_limits<double>::denorm_min()),{}, {},"positive subnormal");
  auto null=make_number_value(2.9);null.is_null=true;
  check(null,{}, {},"typed NULL");check(make_null_value(),{}, {},"NULL");
  check(make_string_value("2"),{}, {},"Character");check(make_boolean_value(true),{}, {},"Logical");
  check(make_currency_value(29000),{}, {},"Currency");
 }
}
std::string get(const RuntimePauseState& state,const char* key){
 const auto found=state.globals.find(key);return found==state.globals.end()?"<missing>":format_value(found->second);
}
void invoke_case(const fs::path& path,const std::string& mode,int selected,const std::string& cls,
 int target,int columns,const std::string& source,Column column,
 const std::string& config={},const std::string& localized={}){
 ++public_count;
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto tag=mode+" session"+std::to_string(selected)+" "+cls+" existingID"+
  std::to_string(target*11)+" columns"+std::to_string(columns)+" "+config+" "+source;
 std::string setup;
 for(int row=1;row<=3;++row){const auto s=std::to_string(row);
  setup+="o.AddListItem('seed"+s+"',"+std::to_string(row*11)+")\no.List("+s+",2)='cell"+s+
   "'\no.ItemData("+s+")="+std::to_string(row*101)+"\n";
 }
 setup+="o.ListIndex=2\n";
 if(cls=="ListBox")setup+="o.TopIndex=2\n";
 const std::string snapshot="TRANSFORM(o.ListCount)+'|'+TRANSFORM(o.ColumnCount)+'|'+TRANSFORM(o.RowSourceType)+'|'+o.ControlSource+'|'+TRANSFORM(o.ListIndex)+'|'+TRANSFORM(o.ListItemID)+'|'+TRANSFORM(o.Value)+'|'+TRANSFORM(o.DisplayValue)+'|'+TRANSFORM(o.NewIndex)+'|'+TRANSFORM(o.NewItemID)"+
  (cls=="ListBox"?std::string("+'|'+TRANSFORM(o.TopIndex)+'|'+TRANSFORM(o.TopItemID)"):"");
 write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC o,nCode,cMessage,lReturned,lAfter,cBefore,cAfter,cContextBefore,cContextAfter,cOther,cReset,nBefore,nGuard\n"
  "nCode=0\ncMessage=''\nlReturned=.F.\nlAfter=.F.\nCREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "o=CREATEOBJECT('"+cls+"')\no.ColumnCount="+std::to_string(columns)+"\n"+config+setup+
  "nBefore=o.ListCount\ncBefore="+snapshot+"\n"
  "cContextBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\no.AddListItem('candidate',"+std::to_string(target*11)+","+source+")\nlReturned=.T.\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\nlAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\ncAfter="+snapshot+"\n"
  "cContextAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\ncReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"AddListItem COLUMN completion "+tag+": "+state.message);
 expect(get(state,"nbefore")=="3","AddListItem COLUMN existing-ID precondition "+tag);
 expect(get(state,"ncode")==std::string(column?"0":"11"),"AddListItem COLUMN admission "+tag+" got "+get(state,"ncode"));
 expect(get(state,"lreturned")==std::string(column?"true":"false"),"AddListItem COLUMN invocation continuation "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","AddListItem COLUMN catchability "+tag);
 if(!localized.empty())expect(get(state,"cmessage")==localized,"AddListItem COLUMN localized11 "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"ccontextbefore")==context&&get(state,"ccontextafter")==context,"AddListItem COLUMN context "+tag);
 expect(get(state,"cother")==(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|","AddListItem COLUMN session/reset "+tag);
 if(!column)expect(get(state,"cbefore")==get(state,"cafter"),"AddListItem COLUMN rejected modeled metadata "+tag);
 bool seen=false;
 for(const auto& object:state.ole_objects)if(lowercase_copy(object.base_class_name)==lowercase_copy(cls)){
  seen=true;
  const bool coherent=object.list_rows.size()==3&&object.collection_items.size()==3&&
   object.collection_item_keys.size()==3&&object.list_item_data.size()==3;
  expect(coherent,"AddListItem COLUMN retained row count/coherence "+tag);
  if(coherent)for(std::size_t row=0;row<3;++row){
   const auto s=std::to_string(row+1),&cells=object.list_rows[row];
   expect(object.collection_item_keys[row]==std::to_string((row+1)*11),"AddListItem COLUMN retained IDs "+tag);
   expect(format_value(object.collection_items[row])=="seed"+s,"AddListItem COLUMN retained caption/duplicate1 "+tag);
   expect(format_value(object.list_item_data[row])==std::to_string((row+1)*101),"AddListItem COLUMN retained payload "+tag);
   expect(cells.size()>=2,"AddListItem COLUMN retained prepared cells "+tag);
   if(column&&row==static_cast<std::size_t>(target-1))
    expect(cells.size()<=32745U,"AddListItem COLUMN derived argument-driven row bound "+tag);
   std::size_t candidates=0;
   for(std::size_t col=1;col<=cells.size();++col){
    const bool write=column&&*column>1&&row==static_cast<std::size_t>(target-1)&&col==*column;
    const std::string wanted=write?"candidate":col==1?"seed"+s:col==2?"cell"+s:"";
    const auto actual=format_value(cells[col-1]);
    if(actual=="candidate")++candidates;
    // Existing-row cell model, not native absolute metadata/return/storage layout.
    expect(actual==wanted,"AddListItem COLUMN retained/selected cell "+tag+" col"+std::to_string(col));
   }
   const bool expectedWrite=column&&*column>1&&row==static_cast<std::size_t>(target-1);
   expect(candidates==(expectedWrite?1U:0U),"AddListItem COLUMN candidate/no-write count "+tag);
   if(expectedWrite)expect(cells.size()>=*column,"AddListItem COLUMN selected stored column "+tag);
  }
  if(!column)expect(object.list_selected==std::vector<bool>({false,true,false}),"AddListItem COLUMN rejected selection "+tag);
 }
 expect(seen,"AddListItem COLUMN captured control "+tag);
 for(const auto& event:state.events)expect(event.category!="ole.invoke","AddListItem COLUMN no external invocation "+tag);
}
void public_cases(const std::string& mode,int selected,bool hostile){
 const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
 const auto dir=fs::temp_directory_path()/("copperfin_addlistitem_column_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
 fs::create_directories(dir);const auto path=dir/"query.prg";const bool legacy=mode=="VFP9";
 for(const std::string cls:{"ListBox","ComboBox"})for(const int target:{1,2,3}){
  for(const auto& c:numbers)if(c.originalSafe!=hostile)
   invoke_case(path,mode,selected,cls,target,2,c.source,legacy?c.legacy:c.modern);
  for(const auto& c:exacts)if(c.originalSafe!=hostile)
   invoke_case(path,mode,selected,cls,target,2,c.source,legacy?c.legacy:c.modern);
 }
 if(hostile){
  for(const std::string cls:{"ListBox","ComboBox"})
   invoke_case(path,mode,selected,cls,2,2,"(EXP(1000)-EXP(1000))",{});
 }else{
  const std::vector<NumberCase> guards{
   {"1.9",1.9,1,1,true},{"2.9",2.9,2,2,true},{"8.9",8.9,8,8,true},
   {"0",0,{},{},true},{"32746",32746,{},0,true},{"65537",65537,{},0,true}};
  for(const std::string cls:{"ListBox","ComboBox"})for(const int columns:{0,8})
   for(const std::string rst:{"","o.RowSourceType=0\n","o.RowSourceType=CAST(0 AS INT64)\n","o.RowSourceType=CAST(0 AS UINT64)\n"})
    for(const std::string binding:{"o.ControlSource=''\n","o.ControlSource='   '\n","o.ControlSource=CHR(9)+'  '\n"})
     for(const auto& c:guards)invoke_case(path,mode,selected,cls,2,columns,c.source,legacy?c.legacy:c.modern,rst+binding);
  for(const auto& locale:std::vector<std::vector<std::string>>{
   {"en-US","Function argument value, type, or count is invalid."},
   {"es-419","El valor, tipo o numero de un argumento de funcion no es valido."},
   {"pt-BR","O valor, tipo ou numero de um argumento de funcao e invalido."},
   {"qps-ploc","[!! Füñçţïøñ årĝümëñţ vålüë, ţÿþë, ør çøüñţ ïš ïñvålïð. !!]"}}){
   set_env_value("COPPERFIN_LOCALE",locale[0],true);
   for(const std::string cls:{"ListBox","ComboBox"}){
    invoke_case(path,mode,selected,cls,2,2,"0",{}, {},locale[1]);
    invoke_case(path,mode,selected,cls,2,2,"-1.9",{}, {},locale[1]);
    invoke_case(path,mode,selected,cls,2,2,"2.9",2);
   }
  }
 }
 set_env_value("COPPERFIN_LOCALE","en-US",true);
 std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv){
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 if(argc==1){direct_cases();}
 else if((argc==3||argc==4)&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
  (std::string(argv[2])=="1"||std::string(argv[2])=="2")&&(argc==3||std::string(argv[3])=="--fixed-only-hostile")){
  public_cases(argv[1],std::string(argv[2])=="1"?1:2,argc==4);
 }else{std::cerr<<"Expected no args or COPPERFIN|VFP9 1|2 [--fixed-only-hostile]\n";return 2;}
 std::cout<<"AddListItem COLUMN frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

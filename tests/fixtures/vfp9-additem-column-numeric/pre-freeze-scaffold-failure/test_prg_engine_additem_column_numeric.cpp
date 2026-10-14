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
// RQ-CF-PRG-ADDITEM-COLUMN-NUMERIC-001 / VR-5611-ADDITEM-COLUMN-NUMERIC-001/002.
// Independently authored THIRD-column literals from retained installed VFP9
// captures and named owner numeric/allocation parent. Engaged0 is admission.
// Explicit originalSafe separates bounded original public probes from a
// PRE-WIRE-authored fixed-only hostile corpus; never a phase assertion skip.
// No helper-generated expectations, decimal-nearone parser oracle or hidden-
// storage/native global-capacity/shared-success/generated-ID parity claim.
struct NumberCase {const char* source;double value;Column modern,legacy;bool originalSafe=false;};
const std::vector<NumberCase> numbers{
 {"-3",-3,{},{},true},{"-1.9",-1.9,{},{},true},
 {"-0.9",-0.9,{},{},true},{"0",0,{},{},true},{"0.9",0.9,{},{},true},
 {"1",1,1,1,true},{"1.4",1.4,1,1,true},{"1.5",1.5,1,1,true},
 {"1.9",1.9,1,1,true},{"2",2,2,2,true},{"2.5",2.5,2,2,true},
 {"2.9",2.9,2,2,true},{"3",3,3,3,true},{"3.9",3.9,3,3,true},
 {"4",4,4,4,true},{"4.9",4.9,4,4,true},{"8",8,8,8,true},
 {"8.9",8.9,8,8,true},{"9",9,9,9,true},{"11",11,11,11,true},
 {"33",33,33,33,true},{"127",127,127,127,true},{"128",128,128,128,true},
 {"255",255,255,255,true},{"256",256,256,256,true},
 {"32767",32767,32767,32767,true},{"32768",32768,32768,32768,true},
 {"65535",65535,65535,65535,true},{"65535.9",65535.9,65535,65535,true},
 {"65536",65536,{},0,true},{"65536.9",65536.9,{},0,true},
 {"65537",65537,{},1,true},{"65537.9",65537.9,{},1,true},
 {"65538.9",65538.9,{},2}, // unsafe positive original: exceeds65538.
 {"65538",65538,{},2,true},
 {"-65535.9",-65535.9,{},{},true},{"-65534.9",-65534.9,{},{},true},
 {"2147483647",2147483647,{},65535},
 {"2147483647.9",2147483647.9,{},65535},
 {"2147483648",2147483648,{},0},
 {"2147483648.9",2147483648.9,{},0},
 {"2147483649",2147483649,{},{}},{"3221225472",3221225472,{},{}},
 {"4294967294",4294967294,{},{}},{"4294967295",4294967295,{},{}},
 {"4294967296",4294967296,{},{}},
 {"4294967297.9",4294967297.9,{},1},
 {"4294967298.9",4294967298.9,{},2},
 {"4294967299.9",4294967299.9,{},3},
 {"4294967300.9",4294967300.9,{},4},
 {"-4294967295.9",-4294967295.9,{},1,true},
 {"-4294967294.9",-4294967294.9,{},2,true},
 {"-4294967293.9",-4294967293.9,{},3,true},
 {"-4294967292.9",-4294967292.9,{},4,true},
 {"-2147483648",-2147483648,{},0,true},
 {"-2147483648.9",-2147483648.9,{},0,true},
 {"-2147483649",-2147483649,{},65535,true},
 {"-3221225472",-3221225472,{},0,true},
 {"-4294967293",-4294967293,{},3,true},
 {"-4294967294",-4294967294,{},2,true},
 {"-4294967295",-4294967295,{},1,true},
 {"-4294967296",-4294967296,{},{},true},
 {"-4294967297",-4294967297,{},{},true},
 {"8589934593",8589934593,{},1},
 {"-8589934591",-8589934591,{},1,true},
 {"9007199254740991",9007199254740991,{},{}},
 {"-9007199254740991",-9007199254740991,{},1,true},
 {"9007199254740992",9007199254740992,{},{}},
 {"1E300",1E300,{},{}},{"-1E300",-1E300,{},{}},
 {"1E20",1E20,{},{}},{"-1E20",-1E20,{},{}},
 {"1E-300",1E-300,{},{},true},
 {"(1E300*1E300)",INFINITY,{},{}},{"(-1E300*1E300)",-INFINITY,{},{}},
 {"1-2^(-52)",1.-std::ldexp(1.,-52),{},{},true},
 {"1-2^(-53)",1.-std::ldexp(1.,-53),{},{},true},
 {"1+2^(-52)",1.+std::ldexp(1.,-52),1,1,true},
 {"2-2^(-52)",2.-std::ldexp(1.,-52),1,1,true},
 {"3.9999999999999996",3.9999999999999996,3,3,true},
 {"4-2^(-51)",4.-std::ldexp(1.,-51),3,3,true},
 {"8.999999999999998",8.999999999999998,8,8,true},
 {"2147483648-2^(-22)",2147483648.-std::ldexp(1.,-22),{},65535},
 {"-2147483648+0.5",-2147483648.+0.5,{},{},true},
 {"6442450944",6442450944,{},0},{"6442450944.9",6442450944.9,{},0},
 {"6442450945",6442450945,{},{}},
 {"-6442450944",-6442450944,{},0,true},
 {"-6442450944.9",-6442450944.9,{},0,true},
 {"-6442450945",-6442450945,{},65535,true},
 {"4294967295.9999995",4294967295.9999995,{},{}},
 {"-4294967295.9999995",-4294967295.9999995,{},1,true},
 {"2^63-1024",std::ldexp(1.,63)-1024,{},{}},
 {"2^63",std::ldexp(1.,63),{},{}},
 {"2^63+2048",std::ldexp(1.,63)+2048,{},{}},
 {"-2^63",-std::ldexp(1.,63),{},{}},
 {"-2^63-2048",-std::ldexp(1.,63)-2048,{},{}},
 {"2^64",std::ldexp(1.,64),{},{}}
};
struct ExactCase {const char* source;PrgValue value;Column modern,legacy;bool originalSafe=false;};
const std::vector<ExactCase> exacts{
 {"CAST(0 AS INT64)",make_int64_value(0),{},{},true},
 {"CAST(1 AS INT64)",make_int64_value(1),1,1,true},
 {"CAST(65535 AS INT64)",make_int64_value(65535),65535,65535,true},
 {"CAST(65536 AS INT64)",make_int64_value(65536),{},0,true},
 {"CAST(65537 AS INT64)",make_int64_value(65537),{},1,true},
 {"CAST(-2147483648 AS INT64)",make_int64_value(-2147483648LL),{},0,true},
 {"CAST(-4294967295 AS INT64)",make_int64_value(-4294967295LL),{},1,true},
 {"CAST(4294967297 AS INT64)",make_int64_value(4294967297LL),{},1},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(1 AS INT64))",make_int64_value(INT64_MIN+1),{},1},
 {"CAST(-9223372036854775808 AS INT64)",make_int64_value(INT64_MIN),{},{}},
 {"(CAST(-9223372036854775808 AS INT64)+CAST(2147483648 AS INT64))",make_int64_value(INT64_MIN+2147483648LL),{},0},
 {"(CAST(9223372036854774784 AS INT64)+CAST(1023 AS INT64))",make_int64_value(INT64_MAX),{},{}},
 {"CAST(0 AS UINT64)",make_uint64_value(0),{},{},true},
 {"CAST(2 AS UINT64)",make_uint64_value(2),2,2,true},
 {"CAST(65535 AS UINT64)",make_uint64_value(65535),65535,65535,true},
 {"CAST(65536 AS UINT64)",make_uint64_value(65536),{},0,true},
 {"CAST(65538 AS UINT64)",make_uint64_value(65538),{},2,true},
 {"CAST(4294967297 AS UINT64)",make_uint64_value(4294967297ULL),{},1},
 {"(CAST(9007199254740992 AS UINT64)+CAST(1 AS UINT64))",make_uint64_value(9007199254740993ULL),{},1},
 {"(CAST(9223372036854775808 AS UINT64)+CAST(1 AS UINT64))",make_uint64_value(9223372036854775809ULL),{},1},
 {"(CAST(9223372036854775808 AS UINT64)+CAST(2147483648 AS UINT64))",make_uint64_value(9223372039002259456ULL),{},0},
 {"(CAST(18446744073709549568 AS UINT64)+CAST(2047 AS UINT64))",make_uint64_value(UINT64_MAX),{},{}},
 {"(CAST(18446744073709549568 AS UINT64)-CAST(2147481600 AS UINT64))",make_uint64_value(UINT64_MAX-2147483647ULL),{},0}
};
int direct_count=0,public_count=0;
void direct_cases(){
 for(const auto mode:{NumericBehavior::copperfin,NumericBehavior::vfp9}){
  const auto check=[&](const PrgValue& value,Column modern,Column legacy,const char* label){
   ++direct_count;expect(checked_additem_column_argument(value,mode)==(mode==NumericBehavior::vfp9?legacy:modern),
    std::string("AddItem COLUMN direct ")+(mode==NumericBehavior::vfp9?"VFP9 ":"COPPERFIN ")+label);
  };
  for(const auto& r:numbers)check(make_number_value(r.value),r.modern,r.legacy,r.source);
  for(const auto& r:exacts)check(r.value,r.modern,r.legacy,r.source);
  check(make_number_value(std::numeric_limits<double>::quiet_NaN()),{},{},"derived NaN");
  check(make_number_value(-std::ldexp(1.,64)),{},{},"actual negative2^64 binary, NOT ambiguous native source");
  check(make_number_value(std::nextafter(1.,0.)),{},{},"binary belowone");
  check(make_number_value(std::nextafter(1.,2.)),1,1,"binary aboveone");
  check(make_number_value(std::nextafter(65536.,0.)),65535,65535,"binary below65536");
  check(make_number_value(std::nextafter(65536.,INFINITY)),{},0,"binary above65536");
  check(make_number_value(std::nextafter(2147483648.,0.)),{},65535,"binary belowINT32 edge");
  check(make_number_value(std::nextafter(2147483648.,INFINITY)),{},0,"binary aboveINT32 edge");
  check(make_number_value(std::nextafter(-2147483648.,0.)),{},{},"negative innerINT32 edge");
  check(make_number_value(std::nextafter(-2147483648.,-INFINITY)),{},0,"negative outerINT32 edge");
  check(make_number_value(std::numeric_limits<double>::denorm_min()),{},{},"positive subnormal");
  check(make_number_value(-std::numeric_limits<double>::denorm_min()),{},{},"negative subnormal");
  auto null=make_number_value(1.9);null.is_null=true;
  check(null,{},{},"typed NULL");check(make_null_value(),{},{},"NULL");
  check(make_string_value("1"),{},{},"Character");check(make_boolean_value(true),{},{},"Logical");
  check(make_currency_value(19000),{},{},"Currency");
 }
}
std::string get(const RuntimePauseState& state,const char* key){
 const auto it=state.globals.find(key);return it==state.globals.end()?"<missing>":format_value(it->second);
}
void invoke_case(const fs::path& path,const std::string& mode,int selected,const std::string& cls,
 int count,int position,int columns,const std::string& source,Column column,
 const std::string& config={},const std::string& message={}){
 ++public_count;
 const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const auto tag=mode+" session"+std::to_string(selected)+" "+cls+" count"+std::to_string(count)+
  " position"+std::to_string(position)+" columns"+std::to_string(columns)+" "+config+" "+source;
 std::string setup;
 for(int row=1;row<=count;++row){const auto s=std::to_string(row);
  setup+="o.AddListItem('seed"+s+"',"+std::to_string(row*11)+")\no.List("+s+",2)='cell"+s+
   "'\no.ItemData("+s+")="+std::to_string(row*101)+"\n";
 }
 if(count)setup+="o.ListIndex="+std::to_string(count==1?1:2)+"\n";
 if(count&&cls=="ListBox")setup+="o.TopIndex="+std::to_string(count==1?1:2)+"\n";
 const std::string snapshot="TRANSFORM(o.ListCount)+'|'+TRANSFORM(o.ColumnCount)+'|'+TRANSFORM(o.RowSourceType)+'|'+o.ControlSource+'|'+TRANSFORM(o.ListIndex)+'|'+TRANSFORM(o.ListItemID)+'|'+TRANSFORM(o.Value)+'|'+TRANSFORM(o.DisplayValue)+'|'+TRANSFORM(o.NewIndex)+'|'+TRANSFORM(o.NewItemID)"+
  (cls=="ListBox"?std::string("+'|'+TRANSFORM(o.TopIndex)+'|'+TRANSFORM(o.TopItemID)"):"");
 write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC o,nCode,cMessage,lReturned,lAfter,cBefore,cAfter,cContextBefore,cContextAfter,cOther,cReset,nBefore,nGuard\n"
  "nCode=0\ncMessage=''\nlReturned=.F.\nlAfter=.F.\nCREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "o=CREATEOBJECT('"+cls+"')\no.ColumnCount="+std::to_string(columns)+"\n"+config+setup+
  "nBefore=o.ListCount\ncBefore="+snapshot+"\n"
  "cContextBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\no.AddItem('candidate',"+std::to_string(position)+","+source+")\nlReturned=.T.\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\nlAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\ncAfter="+snapshot+"\n"
  "cContextAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\ncReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"AddItem COLUMN completion "+tag+": "+state.message);
 expect(get(state,"nbefore")==std::to_string(count),"AddItem COLUMN populated precondition "+tag);
 expect(get(state,"ncode")==std::string(column?"0":"11"),"AddItem COLUMN admission error "+tag+" got "+get(state,"ncode"));
 expect(get(state,"lreturned")==std::string(column?"true":"false"),"AddItem COLUMN invocation continuation "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","AddItem COLUMN catchability "+tag);
 if(!message.empty())expect(get(state,"cmessage")==message,"AddItem COLUMN localized11 "+tag);
 const auto context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"ccontextbefore")==context&&get(state,"ccontextafter")==context,"AddItem COLUMN context "+tag);
 expect(get(state,"cother")==(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|","AddItem COLUMN session isolation/reset "+tag);
 if(!column)expect(get(state,"cbefore")==get(state,"cafter"),"AddItem COLUMN complete rejection property state "+tag);
 bool seen=false;
 for(const auto& object:state.ole_objects)if(lowercase_copy(object.base_class_name)==lowercase_copy(cls)){
  seen=true;const auto wanted=static_cast<std::size_t>(count+(column?1:0));
  const bool coherent=object.list_rows.size()==wanted&&object.collection_items.size()==wanted&&
   object.collection_item_keys.size()==wanted&&object.list_item_data.size()==wanted;
  expect(coherent,"AddItem COLUMN coherent count "+tag);
  if(coherent){
   const auto slot=static_cast<std::size_t>(position-1);
   for(int row=1;row<=count;++row){const auto old=static_cast<std::size_t>(row-1);
    const auto at=old+(column&&old>=slot?1U:0U);const auto s=std::to_string(row);
    expect(object.collection_item_keys[at]==std::to_string(row*11),"AddItem COLUMN retained ID "+tag);
    expect(format_value(object.collection_items[at])=="seed"+s,"AddItem COLUMN retained primary cell "+tag);
    expect(object.list_rows[at].size()>=2&&format_value(object.list_rows[at][0])=="seed"+s&&
     format_value(object.list_rows[at][1])=="cell"+s,"AddItem COLUMN retained cells "+tag);
    expect(format_value(object.list_item_data[at])==std::to_string(row*101),"AddItem COLUMN retained payload "+tag);
   }
   if(column){
    // Sampled visible cells only; high hidden storage is NOT a native oracle.
    for(std::size_t c=1;c<=static_cast<std::size_t>(columns==0?1:columns);++c){
     const auto& cells=object.list_rows[slot];
     const std::string got=c<=cells.size()?format_value(cells[c-1]):"";
     expect(got==(c==*column?"candidate":""),"AddItem COLUMN independently observed visible placement "+tag+" cell"+std::to_string(c));
    }
   }else{
    std::vector<bool> bits(static_cast<std::size_t>(count),false);
    if(count)bits[count==1?0:1]=true;
    expect(object.list_selected==bits,"AddItem COLUMN rejected selection retained "+tag);
   }
  }
 }
 expect(seen,"AddItem COLUMN captured control "+tag);
 for(const auto& event:state.events)expect(event.category!="ole.invoke","AddItem COLUMN no external invocation "+tag);
}
void public_cases(const std::string& mode,int selected,bool hostile){
 const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
 const auto dir=fs::temp_directory_path()/("copperfin_additem_column_5611_"+mode+"_"+std::to_string(selected)+"_"+unique);
 fs::create_directories(dir);const auto path=dir/"query.prg";const bool legacy=mode=="VFP9";
 for(const std::string cls:{"ListBox","ComboBox"})for(const int count:{0,3}){
  for(const int position:count==0?std::vector<int>{1}:std::vector<int>{1,2,4}){
   for(const auto& r:numbers)if(r.originalSafe!=hostile)
    invoke_case(path,mode,selected,cls,count,position,2,r.source,legacy?r.legacy:r.modern);
   for(const auto& r:exacts)if(r.originalSafe!=hostile)
    invoke_case(path,mode,selected,cls,count,position,2,r.source,legacy?r.legacy:r.modern);
  }
 }
 if(hostile){
  for(const std::string cls:{"ListBox","ComboBox"})
   invoke_case(path,mode,selected,cls,3,2,2,"(EXP(1000)-EXP(1000))",{});
 }else{
  const std::vector<NumberCase> guards{
   {"1.9",1.9,1,1,true},{"2.9",2.9,2,2,true},{"8.9",8.9,8,8,true},
   {"0",0,{},{},true},{"65536",65536,{},0,true},{"65537",65537,{},1,true}};
  for(const std::string cls:{"ListBox","ComboBox"})for(const int columns:{0,8})
   for(const std::string rst:{"","o.RowSourceType=0\n","o.RowSourceType=CAST(0 AS INT64)\n","o.RowSourceType=CAST(0 AS UINT64)\n"})
    for(const std::string binding:{"o.ControlSource=''\n","o.ControlSource='   '\n","o.ControlSource=CHR(9)+'  '\n"})
     for(const auto& r:guards)invoke_case(path,mode,selected,cls,3,2,columns,r.source,legacy?r.legacy:r.modern,rst+binding);
  for(const auto& locale:std::vector<std::vector<std::string>>{
   {"en-US","Function argument value, type, or count is invalid."},
   {"es-419","El valor, tipo o numero de un argumento de funcion no es valido."},
   {"pt-BR","O valor, tipo ou numero de um argumento de funcao e invalido."},
   {"qps-ploc","[!! Füñçţïøñ årĝümëñţ vålüë, ţÿþë, ør çøüñţ ïš ïñvålïð. !!]"}}){
   set_env_value("COPPERFIN_LOCALE",locale[0],true);
   for(const std::string cls:{"ListBox","ComboBox"}){
    invoke_case(path,mode,selected,cls,3,2,2,"0",{}, {},locale[1]);
    invoke_case(path,mode,selected,cls,3,2,2,"-1.9",{}, {},locale[1]);
    invoke_case(path,mode,selected,cls,3,2,2,"2.9",2);
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
 std::cout<<"AddItem COLUMN frozen cases: direct="<<direct_count<<" public="<<public_count<<'\n';
 return test_failures()==0?0:1;
}

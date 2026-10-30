// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>
#include <vector>
namespace {
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs=std::filesystem;
using Column=std::optional<std::int32_t>;
// RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR-5611-LISTITEM-COLUMN-NUMERIC-001/002.
// Independent safe-public literals from the retained native supplied-column
// captures and named exact64 parent. Missing Column means native error31;
// positive columns beyond the prepared row are valid empty Character reads.
struct Case {const char* source;Column modern,legacy;};
const std::vector<Case> numbers{
 {"0.9",{},{}},{"1",1,1},{"2.5",2,2},{"10",10,10},
 {"2147483647.1",INT32_MAX,{}},{"1-1E-14",{},{}},{"1-90*2^(-53)",{},1},
 {"2-45*2^(-52)",1,2},{"9-563*2^(-49)",8,9},
 {"-4294967295.9",{},1},{"-4294967294.1",{},2}
};
const std::vector<Case> exacts{
 {"CAST(1 AS INT64)",1,1},{"CAST(10 AS INT64)",10,10},
 {"CAST(-4294967295 AS INT64)",{},1},{"CAST(4294967297 AS UINT64)",{},1}
};
int public_count=0;
std::string get(const RuntimePauseState& state,const char* key){
 const auto found=state.globals.find(key);return found==state.globals.end()?"<missing>":format_value(found->second);
}
void invoke(const fs::path& path,const std::string& mode,int selected,const std::string& cls,int target,
 const std::string& source,Column column,const std::string& config={},const std::string& localized={}){
 ++public_count;const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
 const std::string tag=mode+" session"+std::to_string(selected)+" "+cls+" id"+std::to_string(target)+" "+source;
 std::string setup;
 for(int row=1;row<=3;++row){const int id=row*11;setup+="o.AddListItem('id"+std::to_string(id)+"c1',"+std::to_string(id)+")\n";
  for(int col=2;col<=9;++col)setup+="o.List("+std::to_string(row)+","+std::to_string(col)+")='id"+
   std::to_string(id)+"c"+std::to_string(col)+"'\n";
  setup+="o.ItemData("+std::to_string(row)+")="+std::to_string(row*101)+"\n";}
 setup+="o.ListIndex=2\n";if(cls=="ListBox")setup+="o.TopIndex=2\n";
 const std::string snapshot="TRANSFORM(o.ListCount)+'|'+TRANSFORM(o.ColumnCount)+'|'+TRANSFORM(o.RowSourceType)+'|'"+
  std::string("+o.ControlSource+'|'+TRANSFORM(o.ListIndex)+'|'+TRANSFORM(o.ListItemID)+'|'+TRANSFORM(o.Value)+'|'+TRANSFORM(o.DisplayValue)")+
  (cls=="ListBox"?"+'|'+TRANSFORM(o.TopIndex)+'|'+TRANSFORM(o.TopItemID)":"");
 write_text(path,(selected==2?"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO 2\n":"")+
  "SET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC o,cResult,nCode,cMessage,lAfter,cBefore,cAfter,cContextBefore,cContextAfter,cOther,cReset,nGuard\n"
  "cResult='sentinel'\nnCode=0\ncMessage=''\nlAfter=.F.\nCREATE CURSOR guard (label C(8))\nINSERT INTO guard VALUES ('guard')\n"
  "o=CREATEOBJECT('"+cls+"')\no.ColumnCount=9\n"+config+setup+"cBefore="+snapshot+"\n"
  "cContextBefore=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "TRY\ncResult=o.ListItem("+std::to_string(target)+","+source+")\nCATCH TO problem\nnCode=problem.ErrorNo\ncMessage=problem.Message\nENDTRY\n"
  "lAfter=.T.\nnGuard=CAST(1.9 AS INTEGER)\ncAfter="+snapshot+"\n"
  "cContextAfter=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())+'|'+TRANSFORM(RECNO())+'|'+ALLTRIM(label)\n"
  "USE IN guard\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nSET NUMERICBEHAVIOR TO COPPERFIN\n"
  "cReset=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
 auto session=PrgRuntimeSession::create(make_runtime_session_options(path,path.parent_path(),false));
 const auto state=session.run(DebugResumeAction::continue_run);
 expect(state.completed,"ListItem COLUMN completion "+tag+": "+state.message);
 expect(get(state,"ncode")==std::string(column?"0":"31"),"ListItem COLUMN error "+tag+" got "+get(state,"ncode"));
 const std::string wanted=!column||*column>9?"":"id"+std::to_string(target)+"c"+std::to_string(*column);
 expect(get(state,"cresult")==std::string(column?wanted:"sentinel"),"ListItem COLUMN result "+tag+" got "+get(state,"cresult"));
 const auto result=state.globals.find("cresult");expect(result!=state.globals.end()&&result->second.kind==PrgValueKind::string,
  "ListItem COLUMN Character result "+tag);
 expect(get(state,"lafter")=="true"&&get(state,"nguard")=="1","ListItem COLUMN continuation "+tag);
 const std::string context=std::to_string(selected)+"|"+mode+"|GUARD|1|guard";
 expect(get(state,"ccontextbefore")==context&&get(state,"ccontextafter")==context,"ListItem COLUMN context "+tag);
 expect(get(state,"cother")==std::string(selected==2?other:mode)&&get(state,"creset")=="1|COPPERFIN|","ListItem COLUMN isolation "+tag);
 expect(get(state,"cbefore")==get(state,"cafter"),"ListItem COLUMN read-only state "+tag);
 if(column)expect(get(state,"cmessage").empty(),"ListItem COLUMN success message "+tag);
 if(!localized.empty())expect(get(state,"cmessage")==localized,"ListItem COLUMN localized31 "+tag);
 bool seen=false;for(const auto& object:state.ole_objects)if(copperfin::runtime::lowercase_copy(object.base_class_name)==
  copperfin::runtime::lowercase_copy(cls)){
  seen=true;expect(object.list_rows.size()==3&&object.collection_item_keys==std::vector<std::string>{"11","22","33"}&&
   object.list_item_data.size()==3&&object.list_selected==std::vector<bool>({false,true,false}),"ListItem COLUMN coherent state "+tag);
  if(object.list_rows.size()==3)for(std::size_t row=0;row<3;++row){expect(object.list_rows[row].size()==9,"ListItem COLUMN width "+tag);
   for(std::size_t col=0;col<object.list_rows[row].size();++col)expect(format_value(object.list_rows[row][col])==
    "id"+std::to_string((row+1)*11)+"c"+std::to_string(col+1),"ListItem COLUMN retained cell "+tag);}}
 expect(seen,"ListItem COLUMN captured control "+tag);
 for(const auto& event:state.events)expect(event.category!="prg.object.selected"&&event.category!="ole.invoke","ListItem COLUMN no external event "+tag);
}
void public_cases(const std::string& mode,int selected){
 expect(numbers.size()==11&&exacts.size()==4,"ListItem COLUMN frozen tables");
 const auto dir=fs::temp_directory_path()/("copperfin_listitem_column_5611_"+mode+"_"+std::to_string(selected)+"_"+
  std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));fs::create_directories(dir);const auto path=dir/"query.prg";
 const bool legacy=mode=="VFP9";for(const std::string cls:{"ListBox","ComboBox"})for(int target:{11,22}){
  for(const auto& c:numbers)invoke(path,mode,selected,cls,target,c.source,legacy?c.legacy:c.modern);
  for(const auto& c:exacts)invoke(path,mode,selected,cls,target,c.source,legacy?c.legacy:c.modern);}
 for(const std::string cls:{"ListBox","ComboBox"})for(const std::string& config:{std::string{},
  std::string{"o.RowSourceType=0\no.ControlSource=''\n"},std::string{"o.RowSourceType=CAST(0 AS INT64)\no.ControlSource='   '\n"},
  std::string{"o.RowSourceType=CAST(0 AS UINT64)\no.ControlSource=CHR(9)+'  '\n"}})
  for(const auto& c:std::vector<Case>{{"0.9",{},{}},{"2.5",2,2},{"1-90*2^(-53)",{},1}})
   invoke(path,mode,selected,cls,22,c.source,legacy?c.legacy:c.modern,config);
 for(const std::string cls:{"ListBox","ComboBox"})for(const auto& locale:std::vector<std::pair<std::string,std::string>>{
  {"en-US","Subscript is outside defined range."},{"es-419","El subíndice está fuera del intervalo definido."},
  {"pt-BR","O subscrito está fora do intervalo definido."},{"qps-ploc","[!! Šµþšçŕïþţ ïš øµţšïðë ðëƒïñëð ŕåñğë. !!]"}}){
  set_env_value("COPPERFIN_LOCALE",locale.first,true);invoke(path,mode,selected,cls,22,"0.9",{}, {},locale.second);}
 set_env_value("COPPERFIN_LOCALE","en-US",true);std::error_code ignored;fs::remove_all(dir,ignored);
}
}
int main(int argc,char** argv){ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 if(argc==1){expect(numbers.size()==11&&exacts.size()==4,"ListItem COLUMN frozen tables");}
 else if(argc==3&&(std::string(argv[1])=="COPPERFIN"||std::string(argv[1])=="VFP9")&&
  (std::string(argv[2])=="1"||std::string(argv[2])=="2")){public_cases(argv[1],std::string(argv[2])=="1"?1:2);
  expect(public_count==92,"ListItem COLUMN complete public count");}
 else{std::cerr<<"Expected COPPERFIN|VFP9 1|2\n";return 2;}
 std::cout<<"ListItem supplied-column frozen public cases="<<public_count<<'\n';return test_failures()==0?0:1;}

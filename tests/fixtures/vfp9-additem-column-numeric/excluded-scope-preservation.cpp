// NON-NATIVE source-audited baseline/fixed comparison, never a requirement oracle.
// RQ-CF-PRG-ADDITEM-COLUMN-NUMERIC-001 / VR-5611-ADDITEM-COLUMN-NUMERIC-002.
// Sample excluded routes and unchanged shared-success state; no native parity.
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
struct Case {std::string group,argument,configuration,call;};
int main(int argc,char** argv){
 if(argc!=2)return 2;
 using namespace copperfin::runtime;
 using namespace copperfin::test_support;
 const std::filesystem::path root=argv[1];std::filesystem::create_directories(root);
 const auto path=root/"scope.prg";int calls=0,bad=0;
 std::vector<Case> cases;
 for(const std::string first:{".T.",".NULL.","4.9","$4.9"})
  cases.push_back({"first-type",first,"","o.AddItem("+first+",2,2.9)"});
 for(const std::string third:{".T.",".NULL.","'2.9'","$2.9"})
  cases.push_back({"third-type",third,"","o.AddItem('candidate',2,"+third+")"});
 for(const std::string second:{".T.",".NULL.","'2'","$2","CAST(2 AS INT64)","CAST(2 AS UINT64)","1.9","0","-1"})
  cases.push_back({"nonordinary-second",second,"","o.AddItem('candidate',"+second+",2.9)"});
 cases.push_back({"arity","one","","o.AddItem('candidate')"});
 cases.push_back({"arity","two","","o.AddItem('candidate',2.9)"});
 cases.push_back({"arity","four","","o.AddItem('candidate',2,2.9,7)"});
 // Existing RST1 crash #7177 is expressly excluded, not retested or repaired.
 cases.push_back({"binding-rst2","2.9",
  "CREATE CURSOR srcrows (caption C(8))\nINSERT INTO srcrows VALUES ('first')\nINSERT INTO srcrows VALUES ('second')\nINSERT INTO srcrows VALUES ('third')\no.RowSourceType=2\no.RowSource='srcrows'\no.Requery()\n",
  "o.AddItem('candidate',2,2.9)"});
 cases.push_back({"binding-rst5","2.9",
  "PUBLIC ARRAY aSource[3]\naSource[1]='first'\naSource[2]='second'\naSource[3]='third'\no.RowSourceType=5\no.RowSource='aSource'\no.Requery()\n",
  "o.AddItem('candidate',2,2.9)"});
 cases.push_back({"binding-nonempty","2.9","PUBLIC cBound\ncBound=''\no.ControlSource='cBound'\n",
  "o.AddItem('candidate',2,2.9)"});
 for(const std::string rst:{".F.",".NULL.","$0","'0'","0.4","-0.4"})
  cases.push_back({"nonexact-rst",rst,"o.RowSourceType="+rst+"\n","o.AddItem('candidate',2,2.9)"});
 for(const std::string third:{"1","2","8"})
  cases.push_back({"accepted-integer-shared-state",third,"","o.AddItem('candidate',2,"+third+")"});
 for(const std::string third:{"1","2","8"})
  cases.push_back({"accepted-sorted-shared-state",third,"o.Sorted=.T.\n","o.AddItem('candidate',2,"+third+")"});
 for(const std::string sorted:{"","o.Sorted=.T.\n"})
  cases.push_back({"accepted-high-id-shared-state","2",sorted,"o.AddItem('candidate',1,2)"});
 for(const std::string cls:{"ListBox","ComboBox"})for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const int selected:{1,2})for(const auto& c:cases){
  ++calls;const bool source_rows=c.group=="binding-rst2"||c.group=="binding-rst5";
  const std::string population=c.group=="accepted-high-id-shared-state"?
   "o.AddListItem('first',32767)\n":
   "o.AddListItem('first',11)\no.AddListItem('second',22)\no.AddListItem('third',33)\n";
  const auto other=mode=="VFP9"?"COPPERFIN":"VFP9";
  write_text(path,"SET NUMERICBEHAVIOR TO "+std::string(other)+"\nSET DATASESSION TO "+std::to_string(selected)+
   "\nSET NUMERICBEHAVIOR TO "+mode+"\nPUBLIC o,nBefore,nCount,nFocus,nID,cValue,cDisplay,nTop,nTopID,nNewIndex,nNewID,nCode,vResult,cContext,cPhase,cOther\n"
   "nCode=0\nvResult='not-returned'\ncPhase='setup'\nTRY\no=CREATEOBJECT('"+cls+"')\no.ColumnCount=2\n"+
   (source_rows?c.configuration:c.configuration+population)+
   "o.ListIndex="+std::string(c.group=="accepted-high-id-shared-state"?"1":"2")+"\n"+
   (cls=="ListBox"?"o.TopIndex="+std::string(c.group=="accepted-high-id-shared-state"?"1":"2")+"\n":"")+
   "nBefore=o.ListCount\ncPhase='invoke'\nvResult="+c.call+
   "\nCATCH TO problem\nnCode=problem.ErrorNo\nENDTRY\nnCount=o.ListCount\nnFocus=o.ListIndex\nnID=o.ListItemID\ncValue=o.Value\ncDisplay=o.DisplayValue\nnNewIndex=o.NewIndex\nnNewID=o.NewItemID\n"+
   (cls=="ListBox"?"nTop=o.TopIndex\nnTopID=o.TopItemID\n":"")+
   "cContext=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nRETURN\n");
  auto session=PrgRuntimeSession::create(make_runtime_session_options(path,root,false));
  const auto state=session.run(DebugResumeAction::continue_run);
  const auto get=[&](const char* key){const auto i=state.globals.find(key);return i==state.globals.end()?"<missing>":format_value(i->second);};
  if(!state.completed||get("cphase")!="invoke")++bad;
  std::cout<<cls<<'|'<<mode<<'|'<<selected<<'|'<<c.group<<'|'<<c.argument<<"|completed="<<state.completed;
  for(const auto key:{"cphase","nbefore","ncode","vresult","ncount","nfocus","nid","cvalue","cdisplay","ntop","ntopid","nnewindex","nnewid","ccontext","cother"})
   std::cout<<'|'<<key<<'='<<get(key);
  for(const auto& object:state.ole_objects)if(object.base_class_name==cls){
   std::cout<<"|ids=";for(const auto& key:object.collection_item_keys)std::cout<<key<<',';
   std::cout<<"|primary=";for(const auto& value:object.collection_items)std::cout<<format_value(value)<<',';
   std::cout<<"|bits=";for(bool bit:object.list_selected)std::cout<<bit;
   std::cout<<"|cells=";for(const auto& row:object.list_rows)for(const auto& cell:row)std::cout<<format_value(cell)<<',';
   std::cout<<"|data=";for(const auto& value:object.list_item_data)std::cout<<format_value(value)<<',';
  }
  std::cout<<'\n';
 }
 std::cout<<"DONE|calls="<<calls<<"|scaffold_failures="<<bad<<'\n';
 return bad==0&&calls==296?0:1;
}

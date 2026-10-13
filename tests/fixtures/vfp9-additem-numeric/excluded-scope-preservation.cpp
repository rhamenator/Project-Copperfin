// Verification-only current-main preservation; NEVER a native requirement oracle.
// RQ-CF-PRG-ADDITEM-POSITION-NUMERIC-001 / VR-5611-ADDITEM-POSITION-NUMERIC-002.
// Diagnostic only: preserve sampled fallback/success/automatic-ID state, NOT requirements.
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
 for(int scope:{1,2,5,0})for(const std::string argument:{"1.9","4.9","11"}){
  const std::string configuration=scope==1?
   "o.RowSourceType=1\no.RowSource='first,second,third'\no.Requery()\n":
   scope==2?"CREATE CURSOR srcrows (caption C(8))\nINSERT INTO srcrows VALUES ('first')\nINSERT INTO srcrows VALUES ('second')\nINSERT INTO srcrows VALUES ('third')\no.RowSourceType=2\no.RowSource='srcrows'\no.Requery()\n":
   scope==5?"PUBLIC ARRAY aSource[3]\naSource[1]='first'\naSource[2]='second'\naSource[3]='third'\no.RowSourceType=5\no.RowSource='aSource'\no.Requery()\n":
   "PUBLIC cBound\ncBound=''\no.ControlSource='cBound'\n";
  cases.push_back({"binding-rst"+std::to_string(scope),argument,configuration,
   "o.AddItem('candidate',"+argument+")"});
 }
 for(const std::string argument:{".T.",".NULL.","'4'","$4.9"})
  cases.push_back({"second-type",argument,"","o.AddItem('candidate',"+argument+")"});
 cases.push_back({"arity","omitted","","o.AddItem('candidate')"});
 cases.push_back({"arity","extra","","o.AddItem('candidate',4.9,1,7)"});
 for(const std::string call:{"o.AddItem('candidate',4.9,1)","o.AddItem('candidate',1.9,2)","o.AddItem('candidate',0,2)"})
  cases.push_back({"explicit-column",call,"",call});
 for(const std::string first:{".NULL.",".T.","4.9","$4.9"})
  cases.push_back({"first-type",first,"","o.AddItem("+first+",4.9)"});
 for(const std::string rst:{".F.",".NULL.","$0","'0'","0.4","-0.4"})
  cases.push_back({"nonexact-rst",rst,"o.RowSourceType="+rst+"\n","o.AddItem('candidate',4.9)"});
 for(const std::string argument:{"1","2","4"})
  cases.push_back({"accepted-integer-shared-state",argument,"","o.AddItem('candidate',"+argument+")"});
 for(const std::string argument:{"1","2","4"})
  cases.push_back({"accepted-sorted-shared-state",argument,"o.Sorted=.T.\n",
   "o.AddItem('candidate',"+argument+")"});
 for(const std::string sorted:{"","o.Sorted=.T.\n"})
  cases.push_back({"accepted-high-id-shared-state","1",sorted,"o.AddItem('candidate',1)"});
 for(const std::string cls:{"ListBox","ComboBox"})for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const int selected:{1,2})for(const auto& c:cases){
  ++calls;
  const bool source_rows=c.group=="binding-rst1"||c.group=="binding-rst2"||c.group=="binding-rst5";
  const std::string population=c.group=="accepted-high-id-shared-state"?
   "o.AddListItem('first',32767)\n":
   "o.AddListItem('first',1)\no.AddListItem('second',2)\no.AddListItem('third',3)\n";
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
 return bad==0&&calls==312?0:1;
}

// Verification-only current-main preservation, NOT a native requirement oracle.
// RQ-CF-PRG-REMOVELISTITEM-ID-NUMERIC-001 excludes these routes and #7163/#7164.
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv) {
 if(argc!=2) return 2;
 using namespace copperfin::runtime;
 using namespace copperfin::test_support;
 const std::filesystem::path root=argv[1];std::filesystem::create_directories(root);
 const auto path=root/"scope.prg";int calls=0,bad=0;
 for(const std::string cls:{"ListBox","ComboBox"}) for(const std::string mode:{"COPPERFIN","VFP9"})
 for(const int selected:{1,2}) for(const int scope:{1,2,5,0,-1,-2}) {
  const std::vector<std::string> inputs=scope==-1?std::vector<std::string>{".T.",".NULL.","'2'","$2.5"}:
   scope==-2?std::vector<std::string>{"omitted","extra"}:std::vector<std::string>{"1.9","2.9","99"};
  for(const auto& input:inputs) {
   ++calls;
   const std::string setup=scope==1?"o.RowSourceType=1\no.RowSource='first,second,third'\no.Requery()\n":
    scope==2?"CREATE CURSOR srcrows (caption C(8))\nINSERT INTO srcrows VALUES ('first')\nINSERT INTO srcrows VALUES ('second')\nINSERT INTO srcrows VALUES ('third')\no.RowSourceType=2\no.RowSource='srcrows'\no.Requery()\n":
    scope==5?"PUBLIC ARRAY aSource[3]\naSource[1]='first'\naSource[2]='second'\naSource[3]='third'\no.RowSourceType=5\no.RowSource='aSource'\no.Requery()\n":
    "o.AddItem('first')\no.AddItem('second')\no.AddItem('third')\n"+
      std::string(scope==0?"PUBLIC cBound\ncBound=''\no.ControlSource='cBound'\n":"");
   const auto call=input=="omitted"?"o.RemoveListItem()":input=="extra"?"o.RemoveListItem(2.9,7)":"o.RemoveListItem("+input+")";
   write_text(path,"SET DATASESSION TO "+std::to_string(selected)+"\nSET NUMERICBEHAVIOR TO "+mode+
    "\nPUBLIC o,nBefore,nCount,nFocus,nID,cValue,cDisplay,nTop,nTopID,nCode,vResult,cContext\n"
    "o=CREATEOBJECT('"+cls+"')\n"+setup+"o.ListIndex=2\n"+
    (cls=="ListBox"?"o.TopIndex=2\n":"")+
    "nBefore=o.ListCount\nnCode=0\nvResult='not-returned'\nTRY\nvResult="+call+
    "\nCATCH TO problem\nnCode=problem.ErrorNo\nENDTRY\nnCount=o.ListCount\nnFocus=o.ListIndex\nnID=o.ListItemID\ncValue=o.Value\ncDisplay=o.DisplayValue\n"+
    (cls=="ListBox"?"nTop=o.TopIndex\nnTopID=o.TopItemID\n":"")+
    "cContext=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+ALIAS()\nRETURN\n");
   auto session=PrgRuntimeSession::create(make_runtime_session_options(path,root,false));
   const auto state=session.run(DebugResumeAction::continue_run);
   const auto get=[&](const char* key){const auto i=state.globals.find(key);return i==state.globals.end()?"<missing>":format_value(i->second);};
   if(!state.completed||get("nbefore")!="3")++bad;
   std::cout<<cls<<'|'<<mode<<'|'<<selected<<'|'<<scope<<'|'<<input<<"|completed="<<state.completed;
   for(const auto key:{"nbefore","ncode","vresult","ncount","nfocus","nid","cvalue","cdisplay","ntop","ntopid","ccontext"})
    std::cout<<'|'<<key<<'='<<get(key);
   for(const auto& object:state.ole_objects) if(object.base_class_name==cls) {
    std::cout<<"|ids=";for(const auto& key:object.collection_item_keys)std::cout<<key<<',';
    std::cout<<"|bits=";for(bool bit:object.list_selected)std::cout<<bit;
    std::cout<<"|cells=";for(const auto& row:object.list_rows)for(const auto& cell:row)std::cout<<format_value(cell)<<',';
    std::cout<<"|data=";for(const auto& v:object.list_item_data)std::cout<<format_value(v)<<',';
   }
   std::cout<<'\n';
  }
 }
 std::cout<<"DONE|calls="<<calls<<"|scaffold_failures="<<bad<<'\n';return bad==0&&calls==144?0:1;
}


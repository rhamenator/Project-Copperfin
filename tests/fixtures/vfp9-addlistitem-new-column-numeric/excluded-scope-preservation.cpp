// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// NON-NATIVE baseline/fixed preservation, NEVER a requirement oracle.
// Missing44 EXCLUDED caption/column/ID/arity/RST/binding routes, unchanged
// ordinary integral1/2/8/32745 shared paths and existing22 controls. RST1/#7177
// and unsafe original huge/nonfinite calls are NOT executed or repaired.
// Authored while BOTH-original270 is frozen and the caller is still UNUSED.
// RQ-CF-PRG-ADDLISTITEM-NEW-COLUMN-NUMERIC-001 / VR-5611-ADDLISTITEM-NEW-COLUMN-NUMERIC-002.
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
namespace {
struct Case {std::string group,argument,configuration,call;};
}
int main(int argc,char** argv){
 if(argc!=2)return 2;
 using namespace copperfin::runtime;
 using namespace copperfin::test_support;
 const std::filesystem::path root=argv[1];std::filesystem::create_directories(root);
 const auto script=root/"preservation.prg";
 std::vector<Case> cases;
 for(const std::string caption:{".T.",".NULL.","4.9","$4.9"})
  cases.push_back({"caption-type",caption,"","o.AddListItem("+caption+",44,1.9)"});
 for(const std::string column:{".T.",".NULL.","'1.9'","$1.9"})
  cases.push_back({"column-type",column,"","o.AddListItem('candidate',44,"+column+")"});
 for(const std::string id:{".T.",".NULL.","'44'","$44","CAST(44 AS INT64)","CAST(44 AS UINT64)","43.9","0","-1"})
  cases.push_back({"nonordinary-id",id,"","o.AddListItem('candidate',"+id+",1.9)"});
 cases.push_back({"arity","one","","o.AddListItem('candidate')"});
 cases.push_back({"arity","two","","o.AddListItem('candidate',44)"});
 cases.push_back({"arity","four","","o.AddListItem('candidate',44,1.9,7)"});
 // Registered known RST1 crash #7177 is excluded, not retested/repaired.
 cases.push_back({"binding-rst2","1.9","CREATE CURSOR source (caption C(8))\nINSERT INTO source VALUES ('one')\nINSERT INTO source VALUES ('two')\nINSERT INTO source VALUES ('three')\no.RowSourceType=2\no.RowSource='source'\no.Requery()\n","o.AddListItem('candidate',44,1.9)"});
 cases.push_back({"binding-rst5","1.9","PUBLIC ARRAY aRows[3]\naRows[1]='one'\naRows[2]='two'\naRows[3]='three'\no.RowSourceType=5\no.RowSource='aRows'\no.Requery()\n","o.AddListItem('candidate',44,1.9)"});
 cases.push_back({"nonempty-binding","1.9","PUBLIC cBound\ncBound=''\no.ControlSource='cBound'\n","o.AddListItem('candidate',44,1.9)"});
 for(const std::string rst:{".F.",".NULL.","$0","'0'","0.4","-0.4"})
  cases.push_back({"nonexact-rst",rst,"o.RowSourceType="+rst+"\n","o.AddListItem('candidate',44,1.9)"});
 for(const std::string sorted:{"","o.Sorted=.T.\n"})for(const std::string column:{"1","2","8","32745"})
  cases.push_back({sorted.empty()?"ordinary-shared-state":"sorted-shared-state",column,sorted,"o.AddListItem('candidate',44,"+column+")"});
 for(const std::string column:{"1.9","2.9","65537.9","32746"})
  cases.push_back({"existing-id-control",column,"","o.AddListItem('candidate',22,"+column+")"});
 int calls=0,scaffoldFailures=0;
 for(const std::string cls:{"ListBox","ComboBox"})for(const std::string mode:{"COPPERFIN","VFP9"})for(const int selected:{1,2})for(const auto& c:cases){
  ++calls;const std::string other=mode=="VFP9"?"COPPERFIN":"VFP9";
  const bool bound=c.group=="binding-rst2"||c.group=="binding-rst5";
  const std::string population="o.AddListItem('seed1',11)\no.AddListItem('seed2',22)\no.AddListItem('seed3',33)\no.List(1,2)='cell1'\no.List(2,2)='cell2'\no.List(3,2)='cell3'\no.ItemData(1)=101\no.ItemData(2)=202\no.ItemData(3)=303\n";
  write_text(script,"SET NUMERICBEHAVIOR TO "+other+"\nSET DATASESSION TO "+std::to_string(selected)+"\nSET NUMERICBEHAVIOR TO "+mode+
   "\nPUBLIC o,cPhase,nCode,vResult,nBefore,nCount,nFocus,nID,cValue,cDisplay,nNewIndex,nNewID,nTop,nTopID,cContext,cOther\ncPhase='setup'\nnCode=0\nvResult='not-returned'\nTRY\no=CREATEOBJECT('"+cls+"')\no.ColumnCount=2\n"+c.configuration+(bound?"":population)+
   "o.ListIndex=2\n"+(cls=="ListBox"?"o.TopIndex=2\n":"")+"nBefore=o.ListCount\ncPhase='invoke'\nvResult="+c.call+
   "\nCATCH TO problem\nnCode=problem.ErrorNo\nENDTRY\nnCount=o.ListCount\nnFocus=o.ListIndex\nnID=o.ListItemID\ncValue=o.Value\ncDisplay=o.DisplayValue\nnNewIndex=o.NewIndex\nnNewID=o.NewItemID\n"+(cls=="ListBox"?"nTop=o.TopIndex\nnTopID=o.TopItemID\n":"")+
   "cContext=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())\nSET DATASESSION TO 1\ncOther=SET('NUMERICBEHAVIOR')\nRETURN\n");
  auto session=PrgRuntimeSession::create(make_runtime_session_options(script,root,false));
  const auto state=session.run(DebugResumeAction::continue_run);
  const auto get=[&](const char* key){const auto i=state.globals.find(key);return i==state.globals.end()?"<missing>":format_value(i->second);};
  if(!state.completed||get("cphase")!="invoke")++scaffoldFailures;
  std::cout<<cls<<'|'<<mode<<'|'<<selected<<'|'<<c.group<<'|'<<c.argument<<"|completed="<<state.completed;
  for(const auto key:{"cphase","nbefore","ncode","vresult","ncount","nfocus","nid","cvalue","cdisplay","nnewindex","nnewid","ntop","ntopid","ccontext","cother"})std::cout<<'|'<<key<<'='<<get(key);
  for(const auto& o:state.ole_objects)if(o.base_class_name==cls){
   for(const auto* property:{"rowsourcetype","controlsource"}){
    const auto found=o.properties.find(property);std::cout<<'|'<<property<<'=';
    if(found==o.properties.end())std::cout<<"<absent>";
    else std::cout<<static_cast<int>(found->second.kind)<<':'<<found->second.is_null<<':'<<format_value(found->second);
   }
   std::cout<<"|ids=";for(const auto& id:o.collection_item_keys)std::cout<<id<<',';
   std::cout<<"|primary=";for(const auto& value:o.collection_items)std::cout<<format_value(value)<<',';
   std::cout<<"|selected=";for(bool bit:o.list_selected)std::cout<<bit;
   std::cout<<"|rows=";for(const auto& row:o.list_rows){std::cout<<'['<<row.size()<<':';for(const auto& cell:row)std::cout<<format_value(cell)<<',';std::cout<<']';}
   std::cout<<"|data=";for(const auto& value:o.list_item_data)std::cout<<format_value(value)<<',';
  }
  std::cout<<'\n';
 }
 std::cout<<"DONE|calls="<<calls<<"|scaffold_failures="<<scaffoldFailures<<'\n';
 return cases.size()==41&&calls==328&&scaffoldFailures==0?0:1;
}

// Current-main diagnostic only: observations, NOT requirements/acceptance assertions.
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include <filesystem>
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 if(argc!=2) return 2;
 using namespace copperfin::runtime;
 using namespace copperfin::test_support;
 const std::filesystem::path root=argv[1];std::filesystem::create_directories(root);
 for(const std::string cls:{"ListBox","ComboBox"}) {
  const auto path=root/"allocation.prg";
  write_text(path,"PUBLIC o,vReturn,nCode\no=CREATEOBJECT('"+cls+"')\no.RowSourceType=0\no.ControlSource=''\no.ColumnCount=2\n"
   "o.AddListItem('first',32767)\no.List(1,2)='cell1'\no.ItemData(1)=101\no.ListIndex=1\n"+
   (cls=="ListBox"?"o.TopIndex=1\n":"")+
   "nCode=0\nTRY\nvReturn=o.AddItem('candidate',1)\nCATCH TO problem\nnCode=problem.ErrorNo\nENDTRY\nRETURN\n");
  auto session=PrgRuntimeSession::create(make_runtime_session_options(path,root,false));
  const auto state=session.run(DebugResumeAction::continue_run);
  std::cout<<cls<<"|completed="<<state.completed<<"|message="<<state.message;
  for(const auto key:{"ncode","vreturn"}) {
   const auto it=state.globals.find(key);
   std::cout<<'|'<<key<<'='<<(it==state.globals.end()?"<missing>":format_value(it->second));
  }
  for(const auto& object:state.ole_objects) if(object.base_class_name==cls) {
   std::cout<<"|rows="<<object.list_rows.size()<<"|ids=";
   for(const auto& id:object.collection_item_keys)std::cout<<id<<',';
   std::cout<<"|cells=";
   for(const auto& row:object.list_rows)for(const auto& cell:row)std::cout<<format_value(cell)<<',';
   std::cout<<"|data=";
   for(const auto& value:object.list_item_data)std::cout<<format_value(value)<<',';
  }
  std::cout<<'\n';
 }
 return 0;
}

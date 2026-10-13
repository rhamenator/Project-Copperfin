// Current-main supporting diagnostic, NOT a requirement oracle.
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include <filesystem>
#include <iostream>
#include <string>
int main() {
 using namespace copperfin::runtime;
 using namespace copperfin::test_support;
 const std::filesystem::path root="/home/rich/.codex/worktrees/removeitem-numeric-5611/Project-Copperfin/build/removeitem-native/rst0-main-diagnostic-scripts";
 std::filesystem::create_directories(root);
 int completed=0;
 for(const std::string cls:{"ListBox","ComboBox"}) {
  const auto path=root/(cls+".prg");
  write_text(path,"o=CREATEOBJECT('"+cls+"')\no.AddListItem('first',11)\no.AddListItem('second',22)\no.AddListItem('third',33)\nnBefore=o.ListCount\no.RowSourceType=0\nnAfter=o.ListCount\nRETURN\n");
  const auto state=PrgRuntimeSession::create(make_runtime_session_options(path,root,false)).run(DebugResumeAction::continue_run);
  if(state.completed) ++completed;
  std::cout<<cls<<"|completed="<<state.completed;
  for(const auto key:{"nbefore","nafter"}) {
   const auto it=state.globals.find(key);
   std::cout<<'|'<<key<<'='<<(it==state.globals.end()?"<missing>":format_value(it->second));
  }
  std::cout<<'\n';
 }
 std::cout<<"DONE|"<<completed<<'\n';return completed==2?0:1;
}

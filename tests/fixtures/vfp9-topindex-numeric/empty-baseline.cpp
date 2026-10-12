#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include <iostream>
int main() {
 const std::filesystem::path p="/home/rich/.codex/worktrees/topindex-numeric-5611/Project-Copperfin/build/topindex-native/empty-baseline.prg";
 auto session=copperfin::runtime::PrgRuntimeSession::create(copperfin::test_support::make_runtime_session_options(p,p.parent_path(),false));
 const auto state=session.run(copperfin::runtime::DebugResumeAction::continue_run);
 std::cout<<"completed="<<state.completed<<" message="<<state.message<<'\n';
 for(const auto* key:{"nbeforetop","nbeforeid","naftertop","nafterid"}) {
  auto i=state.globals.find(key);
  std::cout<<key<<'='<<(i==state.globals.end()?"<missing>":copperfin::runtime::format_value(i->second))<<'\n';
 }
 return state.completed?0:1;
}

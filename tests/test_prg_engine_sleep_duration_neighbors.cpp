// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "test_prg_engine_control_flow_support.h"
int main() {
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 using namespace cf_test_prg_engine_control_flow;
 test_sleep_command_emits_runtime_sleep_event();
 test_sleep_duration_uses_heap_backed_frame_continuations();
 test_critical_section_blocking_policy_rejects_sleep_inside_section();
 test_spawn_task_supervision_requests_cooperative_cancellation();
 return test_failures()==0?0:1;
}

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#if !defined(COPPERFIN_ENABLE_PRG_STRING_FUNCTION_TEST_HOOKS)
#error "PRG string-function test hooks are available only in test builds."
#endif

namespace copperfin::runtime::test_hooks {

// Installs a single-shot pause hook at the next wildcard matcher cancellation
// poll. The hook runs synchronously on the evaluating thread and is compiled
// only into test builds so session cancellation can be synchronized without a
// wall-clock race.
void set_wildcard_match_poll_test_hook(void (*hook)());

}  // namespace copperfin::runtime::test_hooks

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <functional>
#include <map>
#include <string>

namespace copperfin::runtime::detail {

// This host-private seam deliberately contains all COM-native values. Runtime
// admission metadata remains pointer/cookie/HRESULT-free.
using WindowsComEventSubscriptionFactory =
    std::function<std::function<void()>(const std::function<bool(std::string)>&)>;

// Creates a local IConnectionPoint subscription factory for an owned source.
// Only zero-argument IDispatch method notifications listed in dispatch_methods
// can cross the boundary, and delivery is limited to the supplied method name.
[[nodiscard]] WindowsComEventSubscriptionFactory make_windows_com_event_subscription_factory(
    IUnknown* source,
    REFIID outgoing_interface,
    std::map<DISPID, std::string> dispatch_methods);

} // namespace copperfin::runtime::detail

#endif

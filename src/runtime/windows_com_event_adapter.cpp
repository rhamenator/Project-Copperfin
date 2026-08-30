// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "windows_com_event_adapter.h"

#if defined(_WIN32)

#include <atomic>
#include <memory>
#include <mutex>
#include <utility>

#include <ocidl.h>

namespace copperfin::runtime::detail {
namespace {

class DispatchSink final : public IDispatch {
public:
    DispatchSink(std::map<DISPID, std::string> methods, std::function<bool(std::string)> deliver)
        : methods_(std::move(methods)), deliver_(std::move(deliver)) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) override {
        if (result == nullptr) return E_POINTER;
        *result = nullptr;
        if (iid == IID_IUnknown || iid == IID_IDispatch) {
            *result = static_cast<IDispatch*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++references_; }
    ULONG STDMETHODCALLTYPE Release() override {
        const ULONG remaining = --references_;
        if (remaining == 0U) delete this;
        return remaining;
    }
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* count) override {
        if (count == nullptr) return E_POINTER;
        *count = 0U;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT, LCID, ITypeInfo**) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) override { return DISP_E_UNKNOWNNAME; }
    HRESULT STDMETHODCALLTYPE Invoke(DISPID id, REFIID, LCID, WORD flags, DISPPARAMS* parameters,
                                     VARIANT*, EXCEPINFO*, UINT*) override {
        if ((flags & DISPATCH_METHOD) == 0U || parameters == nullptr || parameters->cArgs != 0U ||
            parameters->cNamedArgs != 0U) return DISP_E_BADPARAMCOUNT;
        std::lock_guard<std::mutex> lock(mutex_);
        const auto found = methods_.find(id);
        if (!accepting_ || found == methods_.end() || !deliver_) {
            // S_FALSE belongs to the successful HRESULT range. Returning it
            // here would let a source that checks SUCCEEDED() mistake a
            // rejected or quiesced callback for admitted delivery.
            return DISP_E_MEMBERNOTFOUND;
        }
        return deliver_(found->second) ? S_OK : E_FAIL;
    }
    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        accepting_ = false;
        deliver_ = {};
    }
private:
    std::atomic<ULONG> references_{1U};
    std::mutex mutex_;
    bool accepting_ = true;
    std::map<DISPID, std::string> methods_;
    std::function<bool(std::string)> deliver_;
};

struct Subscription final {
    IConnectionPoint* point = nullptr;
    DispatchSink* sink = nullptr;
    DWORD cookie = 0U;
    std::mutex mutex;
    bool disconnected = false;
    ~Subscription() { disconnect(); }
    void disconnect() noexcept {
        std::lock_guard<std::mutex> lock(mutex);
        if (disconnected) return;
        disconnected = true;
        if (sink != nullptr) sink->stop();
        if (point != nullptr && cookie != 0U) (void)point->Unadvise(cookie);
        if (sink != nullptr) sink->Release();
        if (point != nullptr) point->Release();
        sink = nullptr; point = nullptr; cookie = 0U;
    }
};

struct OwnedSource final {
    explicit OwnedSource(IUnknown* value) : value(value) { value->AddRef(); }
    ~OwnedSource() { value->Release(); }
    IUnknown* value;
};

} // namespace

WindowsComEventSubscriptionFactory make_windows_com_event_subscription_factory(
    IUnknown* source, REFIID outgoing_interface, std::map<DISPID, std::string> dispatch_methods) {
    if (source == nullptr || dispatch_methods.empty()) return {};
    const auto owned_source = std::make_shared<OwnedSource>(source);
    return [owned_source, outgoing_interface, methods = std::move(dispatch_methods)](const std::function<bool(std::string)>& deliver) {
        if (!deliver) return std::function<void()>{};
        IConnectionPointContainer* container = nullptr;
        if (FAILED(owned_source->value->QueryInterface(IID_PPV_ARGS(&container)))) return std::function<void()>{};
        IConnectionPoint* point = nullptr;
        const HRESULT located = container->FindConnectionPoint(outgoing_interface, &point);
        container->Release();
        if (FAILED(located) || point == nullptr) return std::function<void()>{};
        DispatchSink* sink = new DispatchSink(methods, deliver);
        DWORD cookie = 0U;
        if (FAILED(point->Advise(sink, &cookie)) || cookie == 0U) {
            sink->Release(); point->Release(); return std::function<void()>{};
        }
        auto subscription = std::make_shared<Subscription>();
        subscription->point = point; subscription->sink = sink; subscription->cookie = cookie;
        return std::function<void()>([subscription]() { subscription->disconnect(); });
    };
}

} // namespace copperfin::runtime::detail

#endif

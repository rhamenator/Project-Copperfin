// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "windows_com_event_adapter.h"

#include "copperfin/runtime/prg_engine.h"

#include <windows.h>
#include <ocidl.h>

#include <array>
#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

class OwnedDispatchSource final : public IConnectionPointContainer, public IConnectionPoint {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) override {
        if (result == nullptr) return E_POINTER;
        *result = nullptr;
        if (iid == IID_IUnknown || iid == IID_IConnectionPointContainer) {
            *result = static_cast<IConnectionPointContainer*>(this);
        } else if (iid == IID_IConnectionPoint) {
            *result = static_cast<IConnectionPoint*>(this);
        } else return E_NOINTERFACE;
        AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++references_; }
    ULONG STDMETHODCALLTYPE Release() override {
        const ULONG remaining = --references_;
        if (remaining == 0U) delete this;
        return remaining;
    }
    HRESULT STDMETHODCALLTYPE EnumConnectionPoints(IEnumConnectionPoints**) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE FindConnectionPoint(REFIID iid, IConnectionPoint** result) override {
        if (result == nullptr) return E_POINTER;
        *result = nullptr;
        if (iid != IID_IDispatch) return E_NOINTERFACE;
        *result = static_cast<IConnectionPoint*>(this); AddRef(); return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetConnectionInterface(IID* iid) override {
        if (iid == nullptr) return E_POINTER;
        *iid = IID_IDispatch; return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetConnectionPointContainer(IConnectionPointContainer** result) override {
        if (result == nullptr) return E_POINTER;
        *result = static_cast<IConnectionPointContainer*>(this); AddRef(); return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Advise(IUnknown* unknown, DWORD* cookie) override {
        if (unknown == nullptr || cookie == nullptr) return E_POINTER;
        IDispatch* dispatch = nullptr;
        if (FAILED(unknown->QueryInterface(IID_PPV_ARGS(&dispatch)))) return E_NOINTERFACE;
        std::lock_guard<std::mutex> lock(mutex_);
        *cookie = next_cookie_++;
        observers_.emplace(*cookie, dispatch);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Unadvise(DWORD cookie) override {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto found = observers_.find(cookie);
        if (found == observers_.end()) return E_FAIL;
        found->second->Release(); observers_.erase(found); return S_OK;
    }
    HRESULT STDMETHODCALLTYPE EnumConnections(IEnumConnections**) override { return E_NOTIMPL; }
    bool fire(DISPID dispatch_id) {
        std::vector<IDispatch*> observers;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [_, observer] : observers_) { observer->AddRef(); observers.push_back(observer); }
        }
        bool delivered = !observers.empty();
        DISPPARAMS parameters{};
        for (IDispatch* observer : observers) {
            delivered = SUCCEEDED(observer->Invoke(dispatch_id, IID_NULL, LOCALE_INVARIANT, DISPATCH_METHOD,
                                                   &parameters, nullptr, nullptr, nullptr)) && delivered;
            observer->Release();
        }
        return delivered;
    }
    bool fire_changed() { return fire(1); }
    std::size_t observer_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return observers_.size();
    }
private:
    ~OwnedDispatchSource() {
        for (auto& [_, observer] : observers_) observer->Release();
    }
    std::atomic<ULONG> references_{1U};
    mutable std::mutex mutex_;
    DWORD next_cookie_ = 1U;
    std::map<DWORD, IDispatch*> observers_;
};

void write_text(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream output(path, std::ios::binary);
    output << contents;
}

void test_prg_eventhandler_dispatches_owned_com_source() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_windows_eventhandler_com_adapter";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path program = root / "owned_source.prg";
    write_text(program,
        "nCalls = 0\n"
        "oSource = CREATEOBJECT('AdmittedSource')\n"
        "oHandler = CREATEOBJECT('Handler')\n"
        "lBound = EVENTHANDLER(oSource, oHandler)\n"
        "READ EVENTS\n"
        "RETURN\n"
        "DEFINE CLASS AdmittedSource AS Custom\n"
        "ENDDEFINE\n"
        "DEFINE CLASS Handler AS Custom\n"
        "    PROCEDURE OnChanged\n"
        "        nCalls = nCalls + 1\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    auto* source = new OwnedDispatchSource();
    copperfin::runtime::RuntimeSessionOptions options;
    options.startup_path = program.string();
    options.working_directory = root.string();
    options.temp_directory = (root / "runtime-temp").string();
    options.com_event_source_admission_callback = [source](const copperfin::runtime::RuntimeOleObjectState& object)
        -> std::optional<copperfin::runtime::RuntimeComEventSourceAdmission> {
        if (object.prog_id != "AdmittedSource") return std::nullopt;
        return copperfin::runtime::RuntimeComEventSourceAdmission{
            .source_identity = "owned-windows-com-fixture",
            .handler_interface_id = "IID_IDispatch",
            .required_handler_methods = {"OnChanged"},
            .subscribe_local_event_source =
                copperfin::runtime::detail::make_windows_com_event_subscription_factory(
                    static_cast<IConnectionPointContainer*>(source), IID_IDispatch, {{1, "OnChanged"}})};
    };

    {
        auto session = copperfin::runtime::PrgRuntimeSession::create(options);
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop && state.waiting_for_events,
               "owned Windows COM source fixture should pause in READ EVENTS");
        expect(source->fire_changed(), "owned Windows COM source should dispatch to the admitted runtime sink");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const auto calls = state.globals.find("ncalls");
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop && state.waiting_for_events &&
                   calls != state.globals.end() && copperfin::runtime::format_value(calls->second) == "1",
               "owned Windows COM callback should reach the existing iterative PRG handler frame");
    }
    expect(!source->fire_changed(), "runtime teardown should disconnect the owned Windows COM source");
    source->Release();
    fs::remove_all(root, ignored);
}

void test_prg_eventhandler_explicitly_unbinds_owned_com_source() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_windows_eventhandler_com_adapter_unbind";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path program = root / "owned_source_unbind.prg";
    write_text(program,
        "oSource = CREATEOBJECT('AdmittedSource')\n"
        "oHandler = CREATEOBJECT('Handler')\n"
        "lBound = EVENTHANDLER(oSource, oHandler)\n"
        "lUnbound = EVENTHANDLER(oSource, oHandler, .T.)\n"
        "READ EVENTS\n"
        "RETURN\n"
        "DEFINE CLASS AdmittedSource AS Custom\n"
        "ENDDEFINE\n"
        "DEFINE CLASS Handler AS Custom\n"
        "    PROCEDURE OnChanged\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    auto* source = new OwnedDispatchSource();
    copperfin::runtime::RuntimeSessionOptions options;
    options.startup_path = program.string();
    options.working_directory = root.string();
    options.temp_directory = (root / "runtime-temp").string();
    options.com_event_source_admission_callback = [source](const copperfin::runtime::RuntimeOleObjectState& object)
        -> std::optional<copperfin::runtime::RuntimeComEventSourceAdmission> {
        if (object.prog_id != "AdmittedSource") return std::nullopt;
        return copperfin::runtime::RuntimeComEventSourceAdmission{
            .source_identity = "owned-windows-com-fixture",
            .handler_interface_id = "IID_IDispatch",
            .required_handler_methods = {"OnChanged"},
            .subscribe_local_event_source =
                copperfin::runtime::detail::make_windows_com_event_subscription_factory(
                    static_cast<IConnectionPointContainer*>(source), IID_IDispatch, {{1, "OnChanged"}})};
    };

    {
        auto session = copperfin::runtime::PrgRuntimeSession::create(options);
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop && state.waiting_for_events,
               "explicitly unbound owned COM fixture should pause in READ EVENTS");
        const auto bound = state.globals.find("lbound");
        const auto unbound = state.globals.find("lunbound");
        expect(bound != state.globals.end() && copperfin::runtime::format_value(bound->second) == "true" &&
                   unbound != state.globals.end() && copperfin::runtime::format_value(unbound->second) == "true",
               "owned COM source should report successful bind and explicit unbind");
        expect(source->observer_count() == 0U && !source->fire_changed(),
               "explicit EVENTHANDLER unbind should deterministically remove the COM subscription");
    }
    source->Release();
    fs::remove_all(root, ignored);
}

void test_prg_eventhandler_release_and_fault_disconnect_owned_com_source() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_windows_eventhandler_com_adapter_lifecycle";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    for (const auto& [file_name, release_statement] :
         std::array<std::pair<const char*, const char*>, 2U>{{
             {"owned_source_release.prg", "oSource.Release()\n"},
             {"owned_handler_release.prg", "oHandler.Release()\n"}}}) {
        expect(std::string(release_statement).find('\n') != std::string::npos &&
                   std::string(release_statement).find("\\n") == std::string::npos,
               "lifecycle fixture must write an actual PRG line ending after Release()");
        const fs::path program = root / file_name;
        write_text(program,
            "oSource = CREATEOBJECT('AdmittedSource')\n"
            "oHandler = CREATEOBJECT('Handler')\n"
            "lBound = EVENTHANDLER(oSource, oHandler)\n" + std::string(release_statement) +
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS AdmittedSource AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS Handler AS Custom\n"
            "    PROCEDURE OnChanged\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        auto* source = new OwnedDispatchSource();
        copperfin::runtime::RuntimeSessionOptions options;
        options.startup_path = program.string();
        options.working_directory = root.string();
        options.temp_directory = (root / "runtime-temp").string();
        options.com_event_source_admission_callback = [source](const copperfin::runtime::RuntimeOleObjectState& object)
            -> std::optional<copperfin::runtime::RuntimeComEventSourceAdmission> {
            if (object.prog_id != "AdmittedSource") return std::nullopt;
            return copperfin::runtime::RuntimeComEventSourceAdmission{
                .source_identity = "owned-windows-com-fixture",
                .handler_interface_id = "IID_IDispatch",
                .required_handler_methods = {"OnChanged"},
                .subscribe_local_event_source =
                    copperfin::runtime::detail::make_windows_com_event_subscription_factory(
                        static_cast<IConnectionPointContainer*>(source), IID_IDispatch, {{1, "OnChanged"}})};
        };
        {
            auto session = copperfin::runtime::PrgRuntimeSession::create(options);
            const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop && state.waiting_for_events &&
                       source->observer_count() == 0U && !source->fire_changed(),
                   "source or handler release should unadvise the owned COM source before a later callback");
        }
        source->Release();
    }

    const fs::path fault_program = root / "owned_handler_fault.prg";
    write_text(fault_program,
        "oSource = CREATEOBJECT('AdmittedSource')\n"
        "oHandler = CREATEOBJECT('FaultingHandler')\n"
        "lBound = EVENTHANDLER(oSource, oHandler)\n"
        "READ EVENTS\n"
        "RETURN\n"
        "DEFINE CLASS AdmittedSource AS Custom\n"
        "ENDDEFINE\n"
        "DEFINE CLASS FaultingHandler AS Custom\n"
        "    PROCEDURE OnChanged\n"
        "        THROW 'owned-com-handler-fault'\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");
    auto* source = new OwnedDispatchSource();
    copperfin::runtime::RuntimeSessionOptions options;
    options.startup_path = fault_program.string();
    options.working_directory = root.string();
    options.temp_directory = (root / "runtime-temp").string();
    options.com_event_source_admission_callback = [source](const copperfin::runtime::RuntimeOleObjectState& object)
        -> std::optional<copperfin::runtime::RuntimeComEventSourceAdmission> {
        if (object.prog_id != "AdmittedSource") return std::nullopt;
        return copperfin::runtime::RuntimeComEventSourceAdmission{
            .source_identity = "owned-windows-com-fixture",
            .handler_interface_id = "IID_IDispatch",
            .required_handler_methods = {"OnChanged"},
            .subscribe_local_event_source =
                copperfin::runtime::detail::make_windows_com_event_subscription_factory(
                    static_cast<IConnectionPointContainer*>(source), IID_IDispatch, {{1, "OnChanged"}})};
    };
    {
        auto session = copperfin::runtime::PrgRuntimeSession::create(options);
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop && state.waiting_for_events &&
                   source->fire_changed(),
               "fault fixture should accept exactly one declared owned-source callback");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::error && source->observer_count() == 0U &&
                   !source->fire_changed(),
               "an escaping owned-source handler fault should unadvise before returning the contained error");
    }
    source->Release();
    fs::remove_all(root, ignored);
}

} // namespace

int main() {
    const HRESULT initialized = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    expect(SUCCEEDED(initialized), "test COM apartment should initialize");
    auto* source = new OwnedDispatchSource();
    const auto factory = copperfin::runtime::detail::make_windows_com_event_subscription_factory(
        static_cast<IConnectionPointContainer*>(source), IID_IDispatch, {{1, "OnChanged"}});
    expect(static_cast<bool>(factory), "owned local source should produce a subscription factory");
    expect(!copperfin::runtime::detail::make_windows_com_event_subscription_factory(
               static_cast<IConnectionPointContainer*>(source), IID_ITypeInfo, {{1, "OnChanged"}})(
               [](std::string) { return true; }),
           "adapter should fail closed when the owned source lacks the requested interface");
    expect(!copperfin::runtime::detail::make_windows_com_event_subscription_factory(
               static_cast<IConnectionPointContainer*>(source), IID_IDispatch, {}),
           "adapter should reject an empty dispatch-method contract");
    std::vector<std::string> delivered;
    const auto disconnect = factory([&delivered](std::string method) {
        delivered.push_back(std::move(method)); return true;
    });
    expect(static_cast<bool>(disconnect), "owned local source should accept an IDispatch sink");
    expect(source->fire_changed(), "advised source should call its mapped sink");
    expect(delivered == std::vector<std::string>{"OnChanged"}, "adapter should forward only the mapped method token");
    expect(!source->fire(2), "adapter should reject an undeclared dispatch identifier");
    disconnect();
    expect(!source->fire_changed(), "disconnect should unadvise and quiesce the source callback");
    source->Release();
    test_prg_eventhandler_dispatches_owned_com_source();
    test_prg_eventhandler_explicitly_unbinds_owned_com_source();
    test_prg_eventhandler_release_and_fault_disconnect_owned_com_source();
    ::CoUninitialize();
    return 0;
}

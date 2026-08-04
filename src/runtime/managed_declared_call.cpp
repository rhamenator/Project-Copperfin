// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "managed_declared_call.h"

#if defined(_WIN32)

#include <metahost.h>

#include <cstddef>
#include <mutex>

#if defined(_MSC_VER)
#import "mscorlib.tlb" raw_interfaces_only \
    high_property_prefixes("_get", "_put", "_putref") \
    rename("ReportEvent", "InteropServices_ReportEvent") \
    rename("or", "InteropServices_or")
#endif

namespace copperfin::runtime
{
    namespace
    {
        template <typename Interface>
        class ComOwner final
        {
        public:
            ComOwner() = default;

            ~ComOwner()
            {
                reset();
            }

            ComOwner(const ComOwner &) = delete;
            ComOwner &operator=(const ComOwner &) = delete;

            [[nodiscard]] Interface *get() const noexcept
            {
                return value_;
            }

            [[nodiscard]] Interface **put() noexcept
            {
                reset();
                return &value_;
            }

            void reset(Interface *value = nullptr) noexcept
            {
                if (value_ != nullptr)
                {
                    value_->Release();
                }
                value_ = value;
            }

            [[nodiscard]] Interface *detach() noexcept
            {
                Interface *value = value_;
                value_ = nullptr;
                return value;
            }

        private:
            Interface *value_ = nullptr;
        };

        class BstrOwner final
        {
        public:
            explicit BstrOwner(const std::wstring &value)
                : value_(SysAllocStringLen(value.data(), static_cast<UINT>(value.size())))
            {
            }

            ~BstrOwner()
            {
                SysFreeString(value_);
            }

            BstrOwner(const BstrOwner &) = delete;
            BstrOwner &operator=(const BstrOwner &) = delete;

            [[nodiscard]] BSTR get() const noexcept
            {
                return value_;
            }

        private:
            BSTR value_ = nullptr;
        };

        class SafeArrayOwner final
        {
        public:
            explicit SafeArrayOwner(SAFEARRAY *value = nullptr) noexcept
                : value_(value)
            {
            }

            ~SafeArrayOwner()
            {
                if (value_ != nullptr)
                {
                    SafeArrayDestroy(value_);
                }
            }

            SafeArrayOwner(const SafeArrayOwner &) = delete;
            SafeArrayOwner &operator=(const SafeArrayOwner &) = delete;

            [[nodiscard]] SAFEARRAY *get() const noexcept
            {
                return value_;
            }

            void reset(SAFEARRAY *value = nullptr) noexcept
            {
                if (value_ != nullptr)
                {
                    SafeArrayDestroy(value_);
                }
                value_ = value;
            }

        private:
            SAFEARRAY *value_ = nullptr;
        };

        class VariantOwner final
        {
        public:
            VariantOwner()
            {
                VariantInit(&value_);
            }

            ~VariantOwner()
            {
                VariantClear(&value_);
            }

            VariantOwner(const VariantOwner &) = delete;
            VariantOwner &operator=(const VariantOwner &) = delete;

            [[nodiscard]] VARIANT *put() noexcept
            {
                VariantClear(&value_);
                return &value_;
            }

            [[nodiscard]] VARIANT &get() noexcept
            {
                return value_;
            }

        private:
            VARIANT value_{};
        };

        [[nodiscard]] std::wstring widen_utf8(const std::string &value)
        {
            if (value.empty())
            {
                return {};
            }
            const int count = MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                value.data(),
                static_cast<int>(value.size()),
                nullptr,
                0);
            if (count <= 0)
            {
                return {};
            }
            std::wstring result(static_cast<std::size_t>(count), L'\0');
            const int converted = MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                value.data(),
                static_cast<int>(value.size()),
                result.data(),
                count);
            return converted == count ? result : std::wstring{};
        }

#if defined(_MSC_VER)
        void discard_thread_error_info() noexcept
        {
            IErrorInfo *error_info = nullptr;
            if (GetErrorInfo(0U, &error_info) == S_OK && error_info != nullptr)
            {
                error_info->Release();
            }
        }

        class ClrRuntime final
        {
        public:
            [[nodiscard]] HRESULT ensure_started()
            {
                const std::lock_guard lock(mutex_);
                if (SUCCEEDED(status_) && host_.get() != nullptr)
                {
                    return status_;
                }

                host_.reset();
                ComOwner<ICLRMetaHost> metahost;
                status_ = CLRCreateInstance(
                    CLSID_CLRMetaHost,
                    IID_ICLRMetaHost,
                    reinterpret_cast<void **>(metahost.put()));
                if (FAILED(status_))
                {
                    stage_ = ManagedInvocationStage::create_runtime;
                    return status_;
                }

                ComOwner<ICLRRuntimeInfo> runtime_info;
                status_ = metahost.get()->GetRuntime(
                    L"v4.0.30319",
                    IID_ICLRRuntimeInfo,
                    reinterpret_cast<void **>(runtime_info.put()));
                if (FAILED(status_))
                {
                    ComOwner<IEnumUnknown> runtimes;
                    if (SUCCEEDED(metahost.get()->EnumerateInstalledRuntimes(runtimes.put())) &&
                        runtimes.get() != nullptr)
                    {
                        IUnknown *unknown = nullptr;
                        ULONG fetched = 0U;
                        while (runtimes.get()->Next(1U, &unknown, &fetched) == S_OK &&
                               fetched == 1U)
                        {
                            ComOwner<IUnknown> runtime_unknown;
                            runtime_unknown.reset(unknown);
                            unknown = nullptr;
                            ComOwner<ICLRRuntimeInfo> candidate;
                            if (SUCCEEDED(runtime_unknown.get()->QueryInterface(
                                    IID_ICLRRuntimeInfo,
                                    reinterpret_cast<void **>(candidate.put()))))
                            {
                                BOOL loadable = FALSE;
                                if (SUCCEEDED(candidate.get()->IsLoadable(&loadable)) && loadable != FALSE)
                                {
                                    runtime_info.reset(candidate.detach());
                                }
                            }
                            fetched = 0U;
                        }
                    }
                    if (runtime_info.get() == nullptr)
                    {
                        stage_ = ManagedInvocationStage::locate_runtime;
                        return status_;
                    }
                }

                status_ = runtime_info.get()->GetInterface(
                    CLSID_CorRuntimeHost,
                    IID_ICorRuntimeHost,
                    reinterpret_cast<void **>(host_.put()));
                if (FAILED(status_) || host_.get() == nullptr)
                {
                    stage_ = ManagedInvocationStage::acquire_runtime_host;
                    return status_;
                }

                status_ = host_.get()->Start();
                if (FAILED(status_))
                {
                    stage_ = ManagedInvocationStage::start_runtime;
                    return status_;
                }
                stage_ = ManagedInvocationStage::none;
                return status_;
            }

            [[nodiscard]] ManagedInvocationStage stage() const noexcept
            {
                return stage_;
            }

            [[nodiscard]] HRESULT default_domain(mscorlib::_AppDomain **domain) const
            {
                *domain = nullptr;
                if (FAILED(status_) || host_.get() == nullptr)
                {
                    return status_;
                }

                ComOwner<IUnknown> unknown_domain;
                HRESULT hr = host_.get()->GetDefaultDomain(unknown_domain.put());
                if (FAILED(hr) || unknown_domain.get() == nullptr)
                {
                    return hr;
                }
                return unknown_domain.get()->QueryInterface(
                    __uuidof(mscorlib::_AppDomain),
                    reinterpret_cast<void **>(domain));
            }

        private:
            std::mutex mutex_;
            ComOwner<ICorRuntimeHost> host_;
            HRESULT status_ = E_FAIL;
            ManagedInvocationStage stage_ = ManagedInvocationStage::create_runtime;
        };

        [[nodiscard]] HRESULT has_public_static_method(
            mscorlib::_Type *type,
            const std::wstring &method_name,
            bool *found)
        {
            *found = false;
            constexpr auto flags = static_cast<mscorlib::BindingFlags>(
                mscorlib::BindingFlags_Public |
                mscorlib::BindingFlags_Static |
                mscorlib::BindingFlags_FlattenHierarchy);
            SAFEARRAY *methods = nullptr;
            HRESULT hr = type->GetMethods(flags, &methods);
            SafeArrayOwner method_array(methods);
            if (FAILED(hr) || method_array.get() == nullptr)
            {
                return FAILED(hr) ? hr : E_NOINTERFACE;
            }

            LONG lower_bound = 0;
            LONG upper_bound = -1;
            hr = SafeArrayGetLBound(method_array.get(), 1U, &lower_bound);
            if (SUCCEEDED(hr))
            {
                hr = SafeArrayGetUBound(method_array.get(), 1U, &upper_bound);
            }
            if (FAILED(hr))
            {
                return hr;
            }

            for (LONG index = lower_bound; index <= upper_bound; ++index)
            {
                IUnknown *unknown = nullptr;
                hr = SafeArrayGetElement(method_array.get(), &index, &unknown);
                if (FAILED(hr) || unknown == nullptr)
                {
                    continue;
                }
                ComOwner<IUnknown> method_unknown;
                method_unknown.reset(unknown);
                ComOwner<mscorlib::_MethodInfo> method;
                hr = method_unknown.get()->QueryInterface(
                    __uuidof(mscorlib::_MethodInfo),
                    reinterpret_cast<void **>(method.put()));
                if (FAILED(hr) || method.get() == nullptr)
                {
                    continue;
                }

                BSTR candidate_name = nullptr;
                hr = method.get()->get_name(&candidate_name);
                if (FAILED(hr))
                {
                    SysFreeString(candidate_name);
                    continue;
                }
                const bool matches =
                    std::wstring(candidate_name, SysStringLen(candidate_name)) == method_name;
                SysFreeString(candidate_name);
                if (matches)
                {
                    *found = true;
                    return S_OK;
                }
            }
            return S_OK;
        }

        [[nodiscard]] HRESULT query_assembly(VARIANT &value, mscorlib::_Assembly **assembly)
        {
            *assembly = nullptr;
            IUnknown *unknown = nullptr;
            if (value.vt == VT_UNKNOWN)
            {
                unknown = value.punkVal;
            }
            else if (value.vt == VT_DISPATCH)
            {
                unknown = value.pdispVal;
            }
            if (unknown == nullptr)
            {
                return E_NOINTERFACE;
            }
            return unknown->QueryInterface(
                __uuidof(mscorlib::_Assembly),
                reinterpret_cast<void **>(assembly));
        }

        [[nodiscard]] HRESULT load_from(
            mscorlib::_AppDomain *domain,
            const std::wstring &assembly_path,
            mscorlib::_Assembly **assembly)
        {
            *assembly = nullptr;
            BstrOwner core_name(L"mscorlib");
            if (core_name.get() == nullptr)
            {
                return E_OUTOFMEMORY;
            }
            ComOwner<mscorlib::_Assembly> core_library;
            HRESULT hr = domain->Load_2(core_name.get(), core_library.put());
            if (FAILED(hr) || core_library.get() == nullptr)
            {
                return FAILED(hr) ? hr : E_NOINTERFACE;
            }

            BstrOwner assembly_type_name(L"System.Reflection.Assembly");
            ComOwner<mscorlib::_Type> assembly_type;
            hr = core_library.get()->GetType_2(assembly_type_name.get(), assembly_type.put());
            if (FAILED(hr) || assembly_type.get() == nullptr)
            {
                return FAILED(hr) ? hr : E_NOINTERFACE;
            }

            VARIANT path_argument{};
            VariantInit(&path_argument);
            path_argument.vt = VT_BSTR;
            path_argument.bstrVal = SysAllocStringLen(
                assembly_path.data(),
                static_cast<UINT>(assembly_path.size()));
            if (path_argument.bstrVal == nullptr)
            {
                return E_OUTOFMEMORY;
            }
            BstrOwner load_from_name(L"LoadFrom");
            if (load_from_name.get() == nullptr)
            {
                VariantClear(&path_argument);
                return E_OUTOFMEMORY;
            }

            SafeArrayOwner invocation_arguments(SafeArrayCreateVector(VT_VARIANT, 0, 1U));
            if (invocation_arguments.get() == nullptr)
            {
                VariantClear(&path_argument);
                return E_OUTOFMEMORY;
            }
            LONG index = 0;
            hr = SafeArrayPutElement(invocation_arguments.get(), &index, &path_argument);
            VariantClear(&path_argument);
            if (FAILED(hr))
            {
                return hr;
            }

            VARIANT target{};
            VariantInit(&target);
            VariantOwner loaded_assembly;
            constexpr auto flags = static_cast<mscorlib::BindingFlags>(
                mscorlib::BindingFlags_InvokeMethod |
                mscorlib::BindingFlags_Public |
                mscorlib::BindingFlags_Static);
            hr = assembly_type.get()->InvokeMember_3(
                load_from_name.get(),
                flags,
                nullptr,
                target,
                invocation_arguments.get(),
                loaded_assembly.put());
            if (FAILED(hr))
            {
                discard_thread_error_info();
                return hr;
            }
            return query_assembly(loaded_assembly.get(), assembly);
        }
#endif
    }

    ManagedInvocationResult invoke_managed_declared_method(
        const std::string &assembly_path_utf8,
        const std::string &type_name_utf8,
        const std::string &method_name_utf8,
        const std::vector<VARIANT> &arguments,
        VARIANT *return_value)
    {
        if (return_value == nullptr)
        {
            return {E_POINTER, ManagedInvocationStage::invoke_method};
        }
        VariantInit(return_value);

#if !defined(_MSC_VER)
        (void)assembly_path_utf8;
        (void)type_name_utf8;
        (void)method_name_utf8;
        (void)arguments;
        return {E_NOTIMPL, ManagedInvocationStage::acquire_runtime_host};
#else
        if (assembly_path_utf8.empty())
        {
            return {E_INVALIDARG, ManagedInvocationStage::load_assembly};
        }
        if (type_name_utf8.empty())
        {
            return {E_INVALIDARG, ManagedInvocationStage::find_type};
        }
        if (method_name_utf8.empty())
        {
            return {E_INVALIDARG, ManagedInvocationStage::find_method};
        }
        const std::wstring assembly_path = widen_utf8(assembly_path_utf8);
        if (assembly_path.empty())
        {
            return {HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION),
                    ManagedInvocationStage::load_assembly};
        }
        const std::wstring type_name = widen_utf8(type_name_utf8);
        if (type_name.empty())
        {
            return {HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION),
                    ManagedInvocationStage::find_type};
        }
        const std::wstring method_name = widen_utf8(method_name_utf8);
        if (method_name.empty())
        {
            return {HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION),
                    ManagedInvocationStage::find_method};
        }

        static ClrRuntime runtime;
        const HRESULT runtime_status = runtime.ensure_started();
        if (FAILED(runtime_status))
        {
            return {runtime_status, runtime.stage()};
        }

        ComOwner<mscorlib::_AppDomain> domain;
        HRESULT hr = runtime.default_domain(domain.put());
        if (FAILED(hr) || domain.get() == nullptr)
        {
            return {FAILED(hr) ? hr : E_NOINTERFACE,
                    ManagedInvocationStage::acquire_app_domain};
        }

        ComOwner<mscorlib::_Assembly> assembly;
        hr = load_from(domain.get(), assembly_path, assembly.put());
        if (FAILED(hr) || assembly.get() == nullptr)
        {
            return {FAILED(hr) ? hr : E_NOINTERFACE,
                    ManagedInvocationStage::load_assembly};
        }

        BstrOwner type_name_bstr(type_name);
        ComOwner<mscorlib::_Type> type;
        hr = assembly.get()->GetType_2(type_name_bstr.get(), type.put());
        if (FAILED(hr) || type.get() == nullptr)
        {
            return {FAILED(hr) ? hr : E_NOINTERFACE,
                    ManagedInvocationStage::find_type};
        }

        bool method_found = false;
        hr = has_public_static_method(type.get(), method_name, &method_found);
        if (FAILED(hr) || !method_found)
        {
            return {FAILED(hr) ? hr : DISP_E_MEMBERNOTFOUND,
                    ManagedInvocationStage::find_method};
        }

        SafeArrayOwner invocation_arguments;
        SAFEARRAY *argument_array = nullptr;
        if (!arguments.empty())
        {
            argument_array = SafeArrayCreateVector(
                VT_VARIANT,
                0,
                static_cast<ULONG>(arguments.size()));
            if (argument_array == nullptr)
            {
                return {E_OUTOFMEMORY, ManagedInvocationStage::invoke_method};
            }
            invocation_arguments.reset(argument_array);
            for (LONG index = 0; index < static_cast<LONG>(arguments.size()); ++index)
            {
                VARIANT copy{};
                VariantInit(&copy);
                hr = VariantCopy(
                    &copy,
                    const_cast<VARIANT *>(&arguments[static_cast<std::size_t>(index)]));
                if (SUCCEEDED(hr))
                {
                    hr = SafeArrayPutElement(argument_array, &index, &copy);
                }
                VariantClear(&copy);
                if (FAILED(hr))
                {
                    return {hr, ManagedInvocationStage::invoke_method};
                }
            }
        }

        VARIANT target{};
        VariantInit(&target);
        BstrOwner method_name_bstr(method_name);
        if (method_name_bstr.get() == nullptr)
        {
            return {E_OUTOFMEMORY, ManagedInvocationStage::invoke_method};
        }
        constexpr auto flags = static_cast<mscorlib::BindingFlags>(
            mscorlib::BindingFlags_InvokeMethod |
            mscorlib::BindingFlags_Public |
            mscorlib::BindingFlags_Static |
            mscorlib::BindingFlags_FlattenHierarchy);
        hr = type.get()->InvokeMember_3(
            method_name_bstr.get(),
            flags,
            nullptr,
            target,
            argument_array,
            return_value);
        if (FAILED(hr))
        {
            VariantClear(return_value);
            discard_thread_error_info();
            return {hr, ManagedInvocationStage::invoke_method};
        }
        return {S_OK, ManagedInvocationStage::none};
#endif
    }
}

#endif

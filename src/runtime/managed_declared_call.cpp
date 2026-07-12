// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "managed_declared_call.h"

#if defined(_WIN32)

#include <metahost.h>

#include <cstddef>

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
            ClrRuntime()
            {
                ComOwner<ICLRMetaHost> metahost;
                status_ = CLRCreateInstance(
                    CLSID_CLRMetaHost,
                    IID_ICLRMetaHost,
                    reinterpret_cast<void **>(metahost.put()));
                if (FAILED(status_))
                {
                    stage_ = ManagedInvocationStage::create_runtime;
                    return;
                }

                ComOwner<ICLRRuntimeInfo> runtime_info;
                status_ = metahost.get()->GetRuntime(
                    L"v4.0.30319",
                    IID_ICLRRuntimeInfo,
                    reinterpret_cast<void **>(runtime_info.put()));
                if (FAILED(status_))
                {
                    stage_ = ManagedInvocationStage::locate_runtime;
                    return;
                }

                status_ = runtime_info.get()->GetInterface(
                    CLSID_CorRuntimeHost,
                    IID_ICorRuntimeHost,
                    reinterpret_cast<void **>(host_.put()));
                if (FAILED(status_) || host_.get() == nullptr)
                {
                    stage_ = ManagedInvocationStage::acquire_runtime_host;
                    return;
                }

                status_ = host_.get()->Start();
                if (FAILED(status_))
                {
                    stage_ = ManagedInvocationStage::start_runtime;
                    return;
                }
                stage_ = ManagedInvocationStage::none;
            }

            [[nodiscard]] HRESULT status() const noexcept
            {
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
            ComOwner<ICorRuntimeHost> host_;
            HRESULT status_ = E_FAIL;
            ManagedInvocationStage stage_ = ManagedInvocationStage::create_runtime;
        };

        [[nodiscard]] const wchar_t *system_type_name(VARTYPE type) noexcept
        {
            switch (type)
            {
            case VT_BSTR:
                return L"System.String";
            case VT_R8:
                return L"System.Double";
            case VT_R4:
                return L"System.Single";
            case VT_I8:
            case VT_UI8:
                return L"System.Int64";
            case VT_BOOL:
                return L"System.Boolean";
            case VT_I1:
            case VT_UI1:
                return L"System.Byte";
            case VT_I2:
            case VT_UI2:
                return L"System.Int16";
            case VT_I4:
            case VT_UI4:
            default:
                return L"System.Int32";
            }
        }

        [[nodiscard]] HRESULT get_method(
            mscorlib::_Assembly *core_library,
            mscorlib::_Type *type,
            const std::wstring &method_name,
            const std::vector<VARIANT> &arguments,
            mscorlib::_MethodInfo **method)
        {
            *method = nullptr;
            SafeArrayOwner parameter_types(
                SafeArrayCreateVector(VT_UNKNOWN, 0, static_cast<ULONG>(arguments.size())));
            if (parameter_types.get() == nullptr)
            {
                return E_OUTOFMEMORY;
            }

            for (LONG index = 0; index < static_cast<LONG>(arguments.size()); ++index)
            {
                BstrOwner type_name(system_type_name(arguments[static_cast<std::size_t>(index)].vt));
                if (type_name.get() == nullptr)
                {
                    return E_OUTOFMEMORY;
                }
                ComOwner<mscorlib::_Type> parameter_type;
                HRESULT hr = core_library->GetType_2(type_name.get(), parameter_type.put());
                if (FAILED(hr) || parameter_type.get() == nullptr)
                {
                    return FAILED(hr) ? hr : E_NOINTERFACE;
                }
                hr = SafeArrayPutElement(parameter_types.get(), &index, parameter_type.get());
                if (FAILED(hr))
                {
                    return hr;
                }
            }

            BstrOwner method_name_bstr(method_name);
            if (method_name_bstr.get() == nullptr)
            {
                return E_OUTOFMEMORY;
            }
            constexpr auto flags = static_cast<mscorlib::BindingFlags>(
                mscorlib::BindingFlags_Public | mscorlib::BindingFlags_Static);
            return type->GetMethod(
                method_name_bstr.get(),
                flags,
                nullptr,
                parameter_types.get(),
                nullptr,
                method);
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
            std::vector<VARIANT> method_arguments{path_argument};
            ComOwner<mscorlib::_MethodInfo> load_method;
            hr = get_method(
                core_library.get(),
                assembly_type.get(),
                L"LoadFrom",
                method_arguments,
                load_method.put());
            if (FAILED(hr) || load_method.get() == nullptr)
            {
                VariantClear(&path_argument);
                return FAILED(hr) ? hr : E_NOINTERFACE;
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
            hr = load_method.get()->Invoke_3(
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
        const std::wstring assembly_path = widen_utf8(assembly_path_utf8);
        const std::wstring type_name = widen_utf8(type_name_utf8);
        const std::wstring method_name = widen_utf8(method_name_utf8);
        if (assembly_path.empty() || type_name.empty() || method_name.empty())
        {
            return {HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION),
                    ManagedInvocationStage::load_assembly};
        }

        static const ClrRuntime runtime;
        if (FAILED(runtime.status()))
        {
            return {runtime.status(), runtime.stage()};
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

        BstrOwner core_name(L"mscorlib");
        ComOwner<mscorlib::_Assembly> core_library;
        hr = domain.get()->Load_2(core_name.get(), core_library.put());
        if (FAILED(hr) || core_library.get() == nullptr)
        {
            return {FAILED(hr) ? hr : E_NOINTERFACE,
                    ManagedInvocationStage::find_method};
        }

        ComOwner<mscorlib::_MethodInfo> method;
        hr = get_method(
            core_library.get(),
            type.get(),
            method_name,
            arguments,
            method.put());
        if (FAILED(hr) || method.get() == nullptr)
        {
            return {FAILED(hr) ? hr : E_NOINTERFACE,
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
        hr = method.get()->Invoke_3(target, argument_array, return_value);
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

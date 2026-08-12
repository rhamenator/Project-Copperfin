// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "native_declared_call.h"

#if defined(_WIN32)

#include "win64_native_call.h"

#include <cstddef>
#include <cstdint>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <oleauto.h>

namespace copperfin::runtime
{
    namespace
    {
        struct NativeArgumentStorage
        {
            std::int32_t signed_integer32 = 0;
            std::int64_t signed_integer64 = 0;
            float floating_point32 = 0.0F;
            double floating_point64 = 0.0;
            std::string string_value;
        };

        struct NativeAbiArgument
        {
            std::uint64_t integer_value = 0U;
            float floating_point32 = 0.0F;
            double floating_point64 = 0.0;
            NativeDeclaredArgumentKind kind = NativeDeclaredArgumentKind::signed_integer32;
            bool is_pointer = false;
        };

        [[nodiscard]] NativeDeclaredCallResult failed_call(HRESULT result)
        {
            NativeDeclaredCallResult failure;
            failure.compatible_error_code = static_cast<std::int32_t>(result);
            return failure;
        }

        void copy_by_reference_results(
            const NativeDeclaredCallRequest &request,
            const std::vector<NativeArgumentStorage> &storage,
            NativeDeclaredCallResult &result)
        {
            result.arguments = request.arguments;
            for (std::size_t index = 0U; index < request.arguments.size(); ++index)
            {
                NativeDeclaredArgument &updated = result.arguments[index];
                if (!updated.by_reference)
                {
                    continue;
                }

                switch (updated.kind)
                {
                case NativeDeclaredArgumentKind::signed_integer32:
                    updated.signed_integer_value = storage[index].signed_integer32;
                    break;
                case NativeDeclaredArgumentKind::signed_integer64:
                    updated.signed_integer_value = storage[index].signed_integer64;
                    break;
                case NativeDeclaredArgumentKind::floating_point32:
                    updated.floating_point_value = storage[index].floating_point32;
                    break;
                case NativeDeclaredArgumentKind::floating_point64:
                    updated.floating_point_value = storage[index].floating_point64;
                    break;
                case NativeDeclaredArgumentKind::string:
                    updated.string_value = storage[index].string_value.c_str();
                    break;
                }
            }
        }

        [[nodiscard]] VARTYPE argument_variant_type(const NativeAbiArgument &argument)
        {
            if (argument.kind == NativeDeclaredArgumentKind::floating_point32 && !argument.is_pointer)
            {
                return VT_R4;
            }
            if (argument.kind == NativeDeclaredArgumentKind::floating_point64 && !argument.is_pointer)
            {
                return VT_R8;
            }
#if defined(_WIN64)
            if (argument.is_pointer)
            {
                return VT_I8;
            }
#else
            if (argument.is_pointer)
            {
                return VT_I4;
            }
#endif
            return argument.kind == NativeDeclaredArgumentKind::signed_integer64 ? VT_I8 : VT_I4;
        }

        [[nodiscard]] VARTYPE return_variant_type(NativeDeclaredReturnKind kind)
        {
            switch (kind)
            {
            case NativeDeclaredReturnKind::signed_integer16:
                return VT_I2;
            case NativeDeclaredReturnKind::signed_integer32:
                return VT_I4;
            case NativeDeclaredReturnKind::signed_integer64:
                return VT_I8;
            case NativeDeclaredReturnKind::floating_point32:
                return VT_R4;
            case NativeDeclaredReturnKind::floating_point64:
                return VT_R8;
            case NativeDeclaredReturnKind::string:
#if defined(_WIN64)
                return VT_I8;
#else
                return VT_I4;
#endif
            }
            return VT_I4;
        }

        [[nodiscard]] NativeDeclaredCallResult invoke_with_disp_call_func(
            const NativeDeclaredCallRequest &request,
            const std::vector<NativeAbiArgument> &arguments,
            const std::vector<NativeArgumentStorage> &storage)
        {
            std::vector<VARTYPE> argument_types(arguments.size());
            std::vector<VARIANTARG> argument_values(arguments.size());
            std::vector<VARIANTARG *> argument_pointers(arguments.size());
            for (std::size_t index = 0U; index < arguments.size(); ++index)
            {
                VariantInit(&argument_values[index]);
                const VARTYPE type = argument_variant_type(arguments[index]);
                argument_types[index] = type;
                argument_values[index].vt = type;
                if (type == VT_R4)
                {
                    argument_values[index].fltVal = arguments[index].floating_point32;
                }
                else if (type == VT_R8)
                {
                    argument_values[index].dblVal = arguments[index].floating_point64;
                }
                else if (type == VT_I8)
                {
                    argument_values[index].llVal = static_cast<LONGLONG>(arguments[index].integer_value);
                }
                else
                {
                    argument_values[index].lVal = static_cast<LONG>(arguments[index].integer_value);
                }
                argument_pointers[index] = &argument_values[index];
            }

            VARIANT native_result;
            VariantInit(&native_result);
#if defined(_WIN64)
            constexpr CALLCONV calling_convention = CC_STDCALL;
#else
            const CALLCONV calling_convention = request.use_cdecl ? CC_CDECL : CC_STDCALL;
#endif
            const HRESULT invoke_result = DispCallFunc(
                nullptr,
                static_cast<ULONG_PTR>(request.function_address),
                calling_convention,
                return_variant_type(request.return_kind),
                static_cast<UINT>(arguments.size()),
                argument_types.empty() ? nullptr : argument_types.data(),
                argument_pointers.empty() ? nullptr : argument_pointers.data(),
                &native_result);
            if (FAILED(invoke_result))
            {
                return failed_call(invoke_result);
            }

            NativeDeclaredCallResult result;
            result.succeeded = true;
            switch (request.return_kind)
            {
            case NativeDeclaredReturnKind::signed_integer16:
                result.signed_integer_value = static_cast<std::int16_t>(native_result.iVal);
                break;
            case NativeDeclaredReturnKind::signed_integer32:
                result.signed_integer_value = static_cast<std::int32_t>(native_result.lVal);
                break;
            case NativeDeclaredReturnKind::signed_integer64:
                result.signed_integer_value = static_cast<std::int64_t>(native_result.llVal);
                break;
            case NativeDeclaredReturnKind::floating_point32:
                result.floating_point_value = static_cast<double>(native_result.fltVal);
                break;
            case NativeDeclaredReturnKind::floating_point64:
                result.floating_point_value = native_result.dblVal;
                break;
            case NativeDeclaredReturnKind::string:
            {
#if defined(_WIN64)
                const auto pointer_value = static_cast<std::uintptr_t>(native_result.llVal);
#else
                const auto pointer_value = static_cast<std::uintptr_t>(
                    static_cast<std::uint32_t>(native_result.lVal));
#endif
                const char *text = reinterpret_cast<const char *>(pointer_value);
                result.string_value = text == nullptr ? std::string{} : std::string(text);
                break;
            }
            }
            copy_by_reference_results(request, storage, result);
            return result;
        }
    }

    NativeDeclaredCallResult invoke_native_declared_function(const NativeDeclaredCallRequest &request)
    {
        if (request.function_address == 0U || request.arguments.size() > 8U)
        {
            return failed_call(E_INVALIDARG);
        }

        std::vector<NativeArgumentStorage> storage(request.arguments.size());
        std::vector<NativeAbiArgument> arguments(request.arguments.size());
        bool has_floating_point32_argument = false;
        for (std::size_t index = 0U; index < request.arguments.size(); ++index)
        {
            const NativeDeclaredArgument &source = request.arguments[index];
            NativeArgumentStorage &backing = storage[index];
            NativeAbiArgument &target = arguments[index];
            target.kind = source.kind;
            target.is_pointer = source.by_reference || source.kind == NativeDeclaredArgumentKind::string;

            switch (source.kind)
            {
            case NativeDeclaredArgumentKind::signed_integer32:
                backing.signed_integer32 = static_cast<std::int32_t>(source.signed_integer_value);
                target.integer_value = source.by_reference
                                           ? reinterpret_cast<std::uintptr_t>(&backing.signed_integer32)
                                           : static_cast<std::uint64_t>(
                                                 static_cast<std::int64_t>(backing.signed_integer32));
                break;
            case NativeDeclaredArgumentKind::signed_integer64:
                backing.signed_integer64 = source.signed_integer_value;
                target.integer_value = source.by_reference
                                           ? reinterpret_cast<std::uintptr_t>(&backing.signed_integer64)
                                           : static_cast<std::uint64_t>(backing.signed_integer64);
                break;
            case NativeDeclaredArgumentKind::floating_point32:
                has_floating_point32_argument = true;
                backing.floating_point32 = static_cast<float>(source.floating_point_value);
                if (source.by_reference)
                {
                    target.integer_value = reinterpret_cast<std::uintptr_t>(&backing.floating_point32);
                }
                else
                {
                    target.floating_point32 = backing.floating_point32;
                }
                break;
            case NativeDeclaredArgumentKind::floating_point64:
                backing.floating_point64 = source.floating_point_value;
                if (source.by_reference)
                {
                    target.integer_value = reinterpret_cast<std::uintptr_t>(&backing.floating_point64);
                }
                else
                {
                    target.floating_point64 = backing.floating_point64;
                }
                break;
            case NativeDeclaredArgumentKind::string:
                backing.string_value = source.string_value;
                if (source.by_reference)
                {
                    backing.string_value.push_back('\0');
                    target.integer_value = reinterpret_cast<std::uintptr_t>(backing.string_value.data());
                }
                else
                {
                    target.integer_value = reinterpret_cast<std::uintptr_t>(backing.string_value.c_str());
                }
                break;
            }
        }

#if defined(_WIN64)
        if (!has_floating_point32_argument &&
            request.return_kind != NativeDeclaredReturnKind::floating_point32 &&
            request.return_kind != NativeDeclaredReturnKind::signed_integer16)
        {
            std::vector<detail::Win64NativeCallArgument> win64_arguments(arguments.size());
            for (std::size_t index = 0U; index < arguments.size(); ++index)
            {
                if (arguments[index].kind == NativeDeclaredArgumentKind::floating_point64 &&
                    !arguments[index].is_pointer)
                {
                    win64_arguments[index].double_value = arguments[index].floating_point64;
                    win64_arguments[index].is_double = true;
                }
                else
                {
                    win64_arguments[index].integer_value = arguments[index].integer_value;
                }
            }

            detail::Win64NativeReturnKind return_kind = detail::Win64NativeReturnKind::integer32;
            if (request.return_kind == NativeDeclaredReturnKind::signed_integer64)
            {
                return_kind = detail::Win64NativeReturnKind::integer64;
            }
            else if (request.return_kind == NativeDeclaredReturnKind::floating_point64)
            {
                return_kind = detail::Win64NativeReturnKind::floating64;
            }
            else if (request.return_kind == NativeDeclaredReturnKind::string)
            {
                return_kind = detail::Win64NativeReturnKind::string_pointer;
            }

            const detail::Win64NativeCallResult native_result = detail::invoke_win64_native_function(
                request.function_address,
                win64_arguments,
                return_kind);
            NativeDeclaredCallResult result;
            result.succeeded = true;
            if (request.return_kind == NativeDeclaredReturnKind::floating_point64)
            {
                result.floating_point_value = native_result.double_value;
            }
            else if (request.return_kind == NativeDeclaredReturnKind::string)
            {
                result.string_value = native_result.string_pointer == nullptr
                                          ? std::string{}
                                          : std::string(native_result.string_pointer);
            }
            else if (request.return_kind == NativeDeclaredReturnKind::signed_integer64)
            {
                result.signed_integer_value = static_cast<std::int64_t>(native_result.integer_value);
            }
            else
            {
                result.signed_integer_value = static_cast<std::int32_t>(native_result.integer_value);
            }
            copy_by_reference_results(request, storage, result);
            return result;
        }
#endif

        return invoke_with_disp_call_func(request, arguments, storage);
    }
}

#endif

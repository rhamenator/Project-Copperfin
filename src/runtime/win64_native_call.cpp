// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "win64_native_call.h"

#if defined(_WIN64)

#include <cstddef>
#include <type_traits>
#include <utility>

namespace copperfin::runtime::detail
{
    namespace
    {
        template <std::size_t Mask, std::size_t Index>
        using NativeArgumentType = std::conditional_t<
            (Mask & (std::size_t{1U} << Index)) != 0U,
            double,
            std::uint64_t>;

        // Microsoft x64 gives each non-floating integer or pointer argument one
        // eight-byte GP/stack slot; the callee consumes its declared low-width value.

        template <typename ArgumentType>
        [[nodiscard]] ArgumentType native_argument_value(const Win64NativeCallArgument &argument)
        {
            if constexpr (std::is_same_v<ArgumentType, double>)
            {
                return argument.double_value;
            }
            else
            {
                return argument.integer_value;
            }
        }

        template <typename ReturnType, std::size_t Mask, std::size_t... Indices>
        [[nodiscard]] ReturnType invoke_with_mask(
            std::uintptr_t function_address,
            std::span<const Win64NativeCallArgument> arguments,
            std::index_sequence<Indices...>)
        {
            using Function = ReturnType (*)(NativeArgumentType<Mask, Indices>...);
            const auto function = reinterpret_cast<Function>(function_address);
            (void)arguments;
            return function(native_argument_value<NativeArgumentType<Mask, Indices>>(arguments[Indices])...);
        }

        template <
            typename ReturnType,
            std::size_t ArgumentCount,
            std::size_t BeginMask,
            std::size_t EndMask>
        [[nodiscard]] ReturnType dispatch_mask_range(
            std::uintptr_t function_address,
            std::span<const Win64NativeCallArgument> arguments,
            std::size_t mask)
        {
            if constexpr (EndMask - BeginMask == 1U)
            {
                return invoke_with_mask<ReturnType, BeginMask>(
                    function_address,
                    arguments,
                    std::make_index_sequence<ArgumentCount>{});
            }
            else
            {
                constexpr std::size_t middle = BeginMask + ((EndMask - BeginMask) / 2U);
                if (mask < middle)
                {
                    return dispatch_mask_range<ReturnType, ArgumentCount, BeginMask, middle>(
                        function_address,
                        arguments,
                        mask);
                }
                return dispatch_mask_range<ReturnType, ArgumentCount, middle, EndMask>(
                    function_address,
                    arguments,
                    mask);
            }
        }

        template <typename ReturnType, std::size_t ArgumentCount>
        [[nodiscard]] ReturnType dispatch_typed_call(
            std::uintptr_t function_address,
            std::span<const Win64NativeCallArgument> arguments,
            std::size_t mask)
        {
            return dispatch_mask_range<
                ReturnType,
                ArgumentCount,
                0U,
                (std::size_t{1U} << ArgumentCount)>(
                    function_address,
                    arguments,
                    mask);
        }

        template <typename ReturnType>
        [[nodiscard]] ReturnType dispatch_argument_count(
            std::uintptr_t function_address,
            std::span<const Win64NativeCallArgument> arguments,
            std::size_t mask)
        {
            switch (arguments.size())
            {
            case 0U:
                return dispatch_typed_call<ReturnType, 0U>(function_address, arguments, mask);
            case 1U:
                return dispatch_typed_call<ReturnType, 1U>(function_address, arguments, mask);
            case 2U:
                return dispatch_typed_call<ReturnType, 2U>(function_address, arguments, mask);
            case 3U:
                return dispatch_typed_call<ReturnType, 3U>(function_address, arguments, mask);
            case 4U:
                return dispatch_typed_call<ReturnType, 4U>(function_address, arguments, mask);
            case 5U:
                return dispatch_typed_call<ReturnType, 5U>(function_address, arguments, mask);
            case 6U:
                return dispatch_typed_call<ReturnType, 6U>(function_address, arguments, mask);
            case 7U:
                return dispatch_typed_call<ReturnType, 7U>(function_address, arguments, mask);
            case 8U:
                return dispatch_typed_call<ReturnType, 8U>(function_address, arguments, mask);
            default:
                return ReturnType{};
            }
        }
    }

    Win64NativeCallResult invoke_win64_native_function(
        std::uintptr_t function_address,
        std::span<const Win64NativeCallArgument> arguments,
        Win64NativeReturnKind return_kind)
    {
        std::size_t double_mask = 0U;
        for (std::size_t index = 0U; index < arguments.size(); ++index)
        {
            if (arguments[index].is_double)
            {
                double_mask |= std::size_t{1U} << index;
            }
        }

        Win64NativeCallResult result;
        if (return_kind == Win64NativeReturnKind::floating64)
        {
            result.double_value = dispatch_argument_count<double>(
                function_address,
                arguments,
                double_mask);
        }
        else if (return_kind == Win64NativeReturnKind::string_pointer)
        {
            result.string_pointer = dispatch_argument_count<const char *>(
                function_address,
                arguments,
                double_mask);
        }
        else if (return_kind == Win64NativeReturnKind::integer64)
        {
            const std::int64_t integer_result = dispatch_argument_count<std::int64_t>(
                function_address,
                arguments,
                double_mask);
            result.integer_value = static_cast<std::uint64_t>(integer_result);
        }
        else
        {
            const std::int32_t integer_result = dispatch_argument_count<std::int32_t>(
                function_address,
                arguments,
                double_mask);
            result.integer_value = static_cast<std::uint64_t>(static_cast<std::int64_t>(integer_result));
        }
        return result;
    }
}

#endif

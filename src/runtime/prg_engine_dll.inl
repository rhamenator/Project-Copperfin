// prg_engine_dll.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        bool is_library_loaded(const std::string &library_name) const
        {
            return loaded_libraries.contains(normalize_identifier(library_name));
        }

        int register_api_function(
            const std::string &variant,
            const std::string &function_name,
            const std::string &argument_types,
            const std::string &return_type,
            const std::string &dll_name)
        {
            if (!is_library_loaded("foxtools"))
            {
                last_error_message = runtime_text("Runtime.Prg.Dll.Error.FoxtoolsNotLoaded");
                return -1;
            }

            const int handle = current_api_handle_counter()++;
            current_registered_api_functions().emplace(handle, RegisteredApiFunction{
                                                                   .handle = handle,
                                                                   .variant = variant,
                                                                   .function_name = function_name,
                                                                   .argument_types = argument_types,
                                                                   .return_type = return_type,
                                                                   .dll_name = dll_name});
            const std::string normalized_argument_types = trim_copy(argument_types);
            const std::string arg_summary = normalized_argument_types.empty() ? "void" : normalized_argument_types;
            const std::string normalized_return_type = trim_copy(return_type);
            events.push_back({.category = "interop.regfn",
                              .detail = variant + ":" + function_name + "@" + dll_name +
                                        " -> " + std::to_string(handle) + " returns " +
                                        (normalized_return_type.empty() ? "ANY" : normalized_return_type) +
                                        " args=" + arg_summary,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return handle;
        }

        PrgValue call_registered_api_function(int handle, const std::vector<PrgValue> &arguments)
        {
            const auto &registered_functions = current_registered_api_functions();
            const auto found = registered_functions.find(handle);
            if (found == registered_functions.end())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Dll.Error.RegisteredApiHandleNotFound",
                    {{"handle", std::to_string(handle)}});
                return make_number_value(-1.0);
            }

            const RegisteredApiFunction &function = found->second;
            events.push_back({.category = "interop.callfn",
                              .detail = function.function_name + "#" + std::to_string(function.handle) + " "
                                        "(" + std::to_string(arguments.size()) + " args)" +
                                        " expects " + function.argument_types +
                                        " returns " + function.return_type,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});

            const std::string normalized_name = normalize_identifier(function.function_name);
            if (normalized_name == "getcurrentprocessid")
            {
                return make_number_value(static_cast<double>(current_process_id()));
            }
            if ((normalized_name == "lstrlena" || normalized_name == "lstrlenw") && !arguments.empty())
            {
                return make_number_value(static_cast<double>(value_as_string(arguments.front()).size()));
            }
            if ((normalized_name == "messageboxa" || normalized_name == "messageboxw"))
            {
                return make_number_value(1.0);
            }
            if ((normalized_name == "getmodulehandlea" || normalized_name == "getmodulehandlew"))
            {
                return make_number_value(1.0);
            }

            const std::string normalized_return = normalize_identifier(function.return_type);
            if (normalized_return == "c")
            {
                return make_string_value({});
            }
            if (normalized_return == "f" || normalized_return == "d")
            {
                return make_number_value(0.0);
            }
            return make_number_value(0.0);
        }

        // ---------------------------------------------------------------------------
        // invoke_declared_dll_function
        // Called from ExpressionParser when declared_dll_invoke_callback_ is set.
        // ---------------------------------------------------------------------------
        std::optional<PrgValue> invoke_declared_dll_function(
            const std::string &fn_key,
            const std::vector<PrgValue> &args,
            const std::vector<std::optional<std::string>> &argument_references)
        {
            const std::string key = normalize_identifier(fn_key);
            const auto found = declared_dll_functions.find(key);
            if (found == declared_dll_functions.end())
                return std::nullopt;
            const DeclaredDllFunction &declfn = found->second;

#if defined(_WIN32)
            // Split comma-separated param_types string into a vector for indexed access.
            struct DeclaredDllParamType
            {
                std::string type;
                bool by_ref = false;
            };
            const auto parse_declared_param_type = [](const std::string &raw_type)
            {
                std::string token = trim_copy(raw_type);
                bool by_ref = false;
                if (!token.empty() && token.back() == '@')
                {
                    by_ref = true;
                    token = trim_copy(token.substr(0U, token.size() - 1U));
                }

                std::size_t type_end = 0U;
                while (type_end < token.size() &&
                       std::isspace(static_cast<unsigned char>(token[type_end])) == 0 &&
                       token[type_end] != '@' &&
                       token[type_end] != '(')
                {
                    ++type_end;
                }

                const std::string type = token.substr(0U, type_end);
                const std::string suffix = trim_copy(token.substr(type_end));
                by_ref = by_ref || suffix.find('@') != std::string::npos;

                return DeclaredDllParamType{normalize_identifier(type), by_ref};
            };

            std::vector<decltype(parse_declared_param_type(std::string{}))> declared_param_types;
            {
                std::istringstream ss(declfn.param_types);
                std::string token;
                while (std::getline(ss, token, ','))
                {
                    declared_param_types.push_back(parse_declared_param_type(token));
                }
            }

            if (args.size() != declared_param_types.size())
            {
                const bool too_few = args.size() < declared_param_types.size();
                throw PrgCompatibilityError(
                    runtime_text(
                        too_few
                            ? "Runtime.Prg.Dll.Error.TooFewArguments"
                            : "Runtime.Prg.Dll.Error.TooManyArguments"),
                    too_few ? 1229 : 1230);
            }

            auto param_type_at = [&](std::size_t i) -> std::string
            {
                return i < declared_param_types.size() ? declared_param_types[i].type : std::string("integer");
            };
            auto param_is_by_ref = [&](std::size_t i) -> bool
            {
                return i < declared_param_types.size() && declared_param_types[i].by_ref;
            };
            // Declared 64-bit integer extensions must not pass through the
            // binary64 representation used by ordinary VFP numeric values.
            const auto exact_declared_integer_value = [](const PrgValue &value) -> std::int64_t
            {
                if (value.kind == PrgValueKind::int64)
                {
                    return value.int64_value;
                }
                if (value.kind == PrgValueKind::uint64)
                {
                    return static_cast<std::int64_t>(value.uint64_value);
                }
                return static_cast<std::int64_t>(value_as_number(value));
            };
            // Convert interpreter values to the portable CLR-host boundary.
            auto to_managed_argument = [&](const PrgValue &v, const std::string &ptype) -> ManagedDeclaredArgument
            {
                ManagedDeclaredArgument argument;
                const std::string base = normalize_identifier(ptype);
                if (base == "string" || base == "c")
                {
                    argument.kind = ManagedDeclaredArgumentKind::string;
                    argument.string_value = value_as_string(v);
                }
                else if (base == "double" || base == "d" || base == "f")
                {
                    argument.kind = ManagedDeclaredArgumentKind::floating_point64;
                    argument.floating_point_value = value_as_number(v);
                }
                else if (declared_dll_type_is_single(base))
                {
                    argument.kind = ManagedDeclaredArgumentKind::floating_point32;
                    argument.floating_point_value = static_cast<float>(value_as_number(v));
                }
                else if (declared_dll_type_uses_64_bit_integer(base))
                {
                    argument.kind = ManagedDeclaredArgumentKind::signed_integer64;
                    argument.signed_integer_value = exact_declared_integer_value(v);
                }
                else
                {
                    // VFP9 parameters permit INTEGER/LONG, not SHORT.
                    argument.kind = ManagedDeclaredArgumentKind::signed_integer32;
                    argument.signed_integer_value = static_cast<std::int32_t>(value_as_number(v));
                }
                return argument;
            };

            // Convert the portable CLR-host result back to an interpreter value.
            auto from_managed_value = [&](const ManagedDeclaredValue &value) -> PrgValue
            {
                const std::string rt = normalize_identifier(declfn.return_type);
                if (rt == "c" || rt == "string")
                {
                    return make_string_value(
                        value.kind == ManagedDeclaredValueKind::string
                            ? value.string_value
                            : std::string{});
                }
                switch (value.kind)
                {
                case ManagedDeclaredValueKind::signed_integer64:
                    return make_int64_value(value.signed_integer_value);
                case ManagedDeclaredValueKind::unsigned_integer64:
                    return make_uint64_value(value.unsigned_integer_value);
                case ManagedDeclaredValueKind::floating_point:
                    return make_number_value(value.floating_point_value);
                case ManagedDeclaredValueKind::boolean:
                    return make_boolean_value(value.boolean_value);
                case ManagedDeclaredValueKind::empty:
                case ManagedDeclaredValueKind::string:
                    return make_number_value(0.0);
                }
                return make_number_value(0.0);
            };

            if (declfn.is_dotnet)
            {
                std::vector<ManagedDeclaredArgument> managed_arguments;
                managed_arguments.reserve(args.size());
                for (std::size_t index = 0U; index < args.size(); ++index)
                {
                    managed_arguments.push_back(to_managed_argument(args[index], param_type_at(index)));
                }

                const std::string &managed_load_path = declfn.loaded_module_path.empty()
                                                           ? declfn.dll_path
                                                           : declfn.loaded_module_path;
                const ManagedInvocationResult invocation = invoke_managed_declared_method(
                    managed_load_path,
                    declfn.dotnet_type_name,
                    declfn.dotnet_method_name,
                    managed_arguments);

                if (!invocation.succeeded)
                {
                    if (!stack.empty() && stack.back().routine != nullptr && stack.back().pc > 0U)
                    {
                        const std::size_t statement_index = stack.back().pc - 1U;
                        if (statement_index < stack.back().routine->statements.size())
                        {
                            const Statement &statement =
                                stack.back().routine->statements[statement_index];
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                        }
                    }
                    const std::int32_t compatible_hresult = invocation.compatible_error_code;
                    switch (invocation.stage)
                    {
                    case ManagedInvocationStage::create_runtime:
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dll.Error.ClrCreateInstanceFailed",
                            {{"hresult", std::to_string(compatible_hresult)}});
                        break;
                    case ManagedInvocationStage::locate_runtime:
                        last_error_message = runtime_text("Runtime.Prg.Dll.Error.ClrRuntimeNotFound");
                        break;
                    case ManagedInvocationStage::acquire_runtime_host:
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dll.Error.CorRuntimeHostGetFailed",
                            {{"hresult", std::to_string(compatible_hresult)}});
                        break;
                    case ManagedInvocationStage::start_runtime:
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dll.Error.ClrRuntimeStartFailed",
                            {{"hresult", std::to_string(compatible_hresult)}});
                        break;
                    case ManagedInvocationStage::acquire_app_domain:
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dll.Error.GetDefaultDomainFailed",
                            {{"hresult", std::to_string(compatible_hresult)}});
                        break;
                    case ManagedInvocationStage::load_assembly:
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dll.Error.DotNetAssemblyLoadFailed",
                            {
                                {"hresult", std::to_string(compatible_hresult)},
                                {"path", declfn.dll_path},
                            });
                        break;
                    case ManagedInvocationStage::find_type:
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dll.Error.DotNetTypeNotFound",
                            {{"typeName", declfn.dotnet_type_name}});
                        break;
                    case ManagedInvocationStage::find_method:
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dll.Error.DotNetMethodNotFound",
                            {{"methodName", declfn.dotnet_method_name}});
                        break;
                    case ManagedInvocationStage::none:
                    case ManagedInvocationStage::invoke_method:
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dll.Error.DotNetMethodInvokeFailed",
                            {{"hresult", std::to_string(compatible_hresult)}});
                        break;
                    }
                    return make_empty_value();
                }

                return from_managed_value(invocation.value);
            }
            else
            {
                // ---------------------------------------------------------------
                // Native DLL invocation via proc_address
                // ---------------------------------------------------------------
                if (declfn.native_function_address == 0U)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dll.Error.NativeProcAddressMissing",
                        {{"functionName", declfn.function_name}});
                    return make_empty_value();
                }

                for (std::size_t index = 0U; index < declared_param_types.size(); ++index)
                {
                    const bool has_callsite_reference =
                        index < argument_references.size() && argument_references[index].has_value();
                    if (declared_param_types[index].by_ref &&
                        declared_dll_type_is_numeric_parameter(declared_param_types[index].type) &&
                        !has_callsite_reference)
                    {
                        throw PrgCompatibilityError(
                            runtime_text(
                                "Runtime.Prg.Dll.Error.NumericByReferenceArgumentRequired",
                                {
                                    {"functionName", declfn.alias},
                                    {"position", std::to_string(index + 1U)},
                                }),
                            11);
                    }
                }

                // Build typed argument storage for common calling conventions.
                // We support the same limited set as VFP's DECLARE: up to 8 args,
                // typed as INTEGER/LONG/SINGLE/DOUBLE/STRING.
                // For STRING params we pass a pointer to the UTF-8 buffer.
                // We do not attempt to pack varargs generically; instead we use a
                // dispatch table keyed on arg count (0-8), which covers the vast
                // majority of real-world DLL calls.

                // Convert arguments into stable integer/pointer and floating-point
                // backing storage before selecting the bounded native dispatcher.
                struct ByRefBinding
                {
                    std::optional<std::string> reference_name;
                    std::string base_type;
                    bool by_ref = false;
                    bool is_single = false;
                    bool is_double = false;
                    bool is_string = false;
                    bool is_integer64 = false;
                    std::string string_buffer;
                    std::int32_t int32_value = 0;
                    std::int64_t int64_value = 0;
                    float single_value = 0.0F;
                    double double_value = 0.0;
                };

                std::vector<ByRefBinding> byref_bindings(args.size());
                std::vector<std::string> string_buffers;
                string_buffers.reserve(args.size());
                struct Arg64
                {
                    __int64 i = 0;
                    float single_value = 0.0F;
                    double d = 0.0;
                    bool is_single = false;
                    bool is_double = false;
                    bool is_integer32 = false;
                    bool is_pointer = false;
                };
                std::vector<Arg64> flat;
                flat.reserve(args.size());
                const bool has_single_parameter = std::any_of(
                    declared_param_types.begin(),
                    declared_param_types.end(),
                    [](const auto &parameter)
                    {
                        return declared_dll_type_is_single(parameter.type);
                    });
                for (std::size_t idx = 0; idx < args.size(); ++idx)
                {
                    const std::string ptype = normalize_identifier(param_type_at(idx));
                    const bool by_ref_param = param_is_by_ref(idx) &&
                                             idx < argument_references.size() &&
                                             argument_references[idx].has_value();
                    const std::string base_pt = ptype;
                    const bool is_integer64 = declared_dll_type_uses_64_bit_integer(base_pt);
                    ByRefBinding &binding = byref_bindings[idx];
                    binding.base_type = base_pt;
                    binding.reference_name = by_ref_param ? argument_references[idx] : std::nullopt;
                    binding.by_ref = by_ref_param;
                    binding.is_integer64 = is_integer64;

                    Arg64 a{};
                    if (declared_dll_type_is_single(base_pt))
                    {
                        if (binding.by_ref)
                        {
                            binding.is_single = true;
                            binding.single_value = static_cast<float>(value_as_number(args[idx]));
                            a.i = reinterpret_cast<__int64>(&binding.single_value);
                            a.is_pointer = true;
                        }
                        else
                        {
                            a.single_value = static_cast<float>(value_as_number(args[idx]));
                            a.is_single = true;
                        }
                    }
                    else if (base_pt == "double" || base_pt == "d" || base_pt == "f")
                    {
                        if (binding.by_ref)
                        {
                            binding.is_double = true;
                            binding.double_value = value_as_number(args[idx]);
                            a.i = reinterpret_cast<__int64>(&binding.double_value);
                            a.is_pointer = true;
                        }
                        else
                        {
                            a.d = value_as_number(args[idx]);
                            a.is_double = true;
                        }
                    }
                    else if (base_pt == "string" || base_pt == "c")
                    {
                        if (binding.by_ref)
                        {
                            binding.is_string = true;
                            binding.string_buffer = value_as_string(args[idx]);
                            binding.string_buffer.push_back('\0');
                            a.i = reinterpret_cast<__int64>(binding.string_buffer.data());
                            a.is_pointer = true;
                        }
                        else
                        {
                            string_buffers.push_back(value_as_string(args[idx]));
                            a.i = reinterpret_cast<__int64>(string_buffers.back().c_str());
                            a.is_pointer = true;
                        }
                    }
                    else
                    {
                        a.i = static_cast<__int64>(
                            binding.is_integer64
                                ? exact_declared_integer_value(args[idx])
                                : static_cast<std::int64_t>(value_as_number(args[idx])));
                        if (binding.by_ref)
                        {
                            if (binding.is_integer64)
                            {
                                binding.int64_value = a.i;
                                a.i = reinterpret_cast<__int64>(&binding.int64_value);
                                a.is_pointer = true;
                            }
                            else
                            {
                                binding.int32_value = static_cast<std::int32_t>(a.i);
                                a.i = reinterpret_cast<__int64>(&binding.int32_value);
                                a.is_pointer = true;
                            }
                        }
                        else
                        {
                            a.is_integer32 = !is_integer64;
                        }
                    }
                    flat.push_back(a);
                }

                const auto sync_byref_bindings = [&]()
                {
                    if (stack.empty())
                    {
                        return;
                    }
                    for (std::size_t idx = 0; idx < std::min(flat.size(), byref_bindings.size()); ++idx)
                    {
                        const auto &binding = byref_bindings[idx];
                        if (!binding.by_ref || !binding.reference_name.has_value())
                        {
                            continue;
                        }
                        if (binding.reference_name.value().empty())
                        {
                            continue;
                        }
                        PrgValue value;
                        if (binding.is_string)
                        {
                            value = make_string_value(binding.string_buffer.c_str());
                        }
                        else if (binding.is_single)
                        {
                            value = make_number_value(static_cast<double>(binding.single_value));
                        }
                        else if (binding.is_double)
                        {
                            value = make_number_value(binding.double_value);
                        }
                        else
                        {
                            value = binding.is_integer64
                                        ? make_int64_value(binding.int64_value)
                                        : make_number_value(static_cast<double>(binding.int32_value));
                        }
                        assign_variable(stack.back(), binding.reference_name.value(), value);
                    }
                };

                // Call the function. We cast to a stdcall prototype (VFP default on x86
                // for DECLARE; on x64 there is only one calling convention).
                // Return storage follows the declared SHORT, integer, SINGLE,
                // DOUBLE, or STRING contract.
                const std::string rt = normalize_identifier(declfn.return_type);
                const bool ret_short = declared_dll_type_is_short(rt);
                const bool ret_single = declared_dll_type_is_single(rt);
                const bool ret_double = (rt == "double" || rt == "d" || rt == "f");
                const bool ret_string = (rt == "c" || rt == "string");
                const bool ret_integer64 = declared_dll_type_uses_64_bit_integer(rt);
                const std::size_t nargs = flat.size();
                const auto finalize_integer_result = [&](const auto result) -> PrgValue
                {
                    sync_byref_bindings();
                    if (ret_string)
                    {
                        const char *p = reinterpret_cast<const char *>(result);
                        return make_string_value(p ? std::string(p) : std::string{});
                    }
                    if (declared_dll_type_uses_64_bit_integer(declfn.return_type))
                    {
                        return make_int64_value(static_cast<std::int64_t>(result));
                    }
                    return make_number_value(static_cast<double>(result));
                };
                const auto finalize_double_result = [&](double result) -> PrgValue
                {
                    sync_byref_bindings();
                    return make_number_value(result);
                };

                if (nargs > 8U)
                {
                    throw PrgCompatibilityError(
                        runtime_text(
                            "Runtime.Prg.Dll.Error.NativeArgumentLimitExceeded",
                            {
                                {"count", std::to_string(nargs)},
                                {"maximum", "8"}
                            }),
                        1230);
                }

                const auto invoke_via_disp_call_func = [&]() -> PrgValue
                {
                    std::vector<VARTYPE> native_argument_types(nargs);
                    std::vector<VARIANTARG> native_argument_values(nargs);
                    std::vector<VARIANTARG *> native_argument_pointers(nargs);
                    for (std::size_t index = 0U; index < nargs; ++index)
                    {
                        VariantInit(&native_argument_values[index]);
                        if (flat[index].is_single)
                        {
                            native_argument_types[index] = VT_R4;
                            native_argument_values[index].vt = VT_R4;
                            native_argument_values[index].fltVal = flat[index].single_value;
                        }
                        else if (flat[index].is_double)
                        {
                            native_argument_types[index] = VT_R8;
                            native_argument_values[index].vt = VT_R8;
                            native_argument_values[index].dblVal = flat[index].d;
                        }
                        else
                        {
#if defined(_WIN64)
                            const bool use_integer32 = flat[index].is_integer32;
#else
                            const bool use_integer32 = flat[index].is_integer32 || flat[index].is_pointer;
#endif
                            native_argument_types[index] = use_integer32 ? VT_I4 : VT_I8;
                            native_argument_values[index].vt = native_argument_types[index];
                            if (use_integer32)
                            {
                                native_argument_values[index].lVal = static_cast<LONG>(flat[index].i);
                            }
                            else
                            {
                                native_argument_values[index].llVal = flat[index].i;
                            }
                        }
                        native_argument_pointers[index] = &native_argument_values[index];
                    }

#if defined(_WIN64)
                    const VARTYPE pointer_return_type = VT_I8;
#else
                    const VARTYPE pointer_return_type = VT_I4;
#endif
                    VARTYPE native_return_type = VT_I4;
                    if (ret_short)
                        native_return_type = VT_I2;
                    else if (ret_single)
                        native_return_type = VT_R4;
                    else if (ret_double)
                        native_return_type = VT_R8;
                    else if (ret_string)
                        native_return_type = pointer_return_type;
                    else if (ret_integer64)
                        native_return_type = VT_I8;
                    VARIANT native_result;
                    VariantInit(&native_result);
#if defined(_WIN64)
                    constexpr CALLCONV native_calling_convention = CC_STDCALL;
#else
                    const CALLCONV native_calling_convention = declfn.native_cdecl ? CC_CDECL : CC_STDCALL;
#endif
                    const HRESULT invoke_result = DispCallFunc(
                        nullptr,
                        static_cast<ULONG_PTR>(declfn.native_function_address),
                        native_calling_convention,
                        native_return_type,
                        static_cast<UINT>(nargs),
                        native_argument_types.empty() ? nullptr : native_argument_types.data(),
                        native_argument_pointers.empty() ? nullptr : native_argument_pointers.data(),
                        &native_result);
                    if (FAILED(invoke_result))
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dll.Error.NativeInvokeFailed",
                            {
                                {"functionName", declfn.function_name},
                                {"hresult", std::to_string(invoke_result)}
                            });
                        return make_empty_value();
                    }
                    if (ret_short)
                    {
                        return finalize_integer_result(static_cast<std::int16_t>(native_result.iVal));
                    }
                    if (ret_single)
                    {
                        return finalize_double_result(static_cast<double>(native_result.fltVal));
                    }
                    if (ret_double)
                    {
                        return finalize_double_result(native_result.dblVal);
                    }
                    if (ret_string)
                    {
#if defined(_WIN64)
                        return finalize_integer_result(static_cast<std::uintptr_t>(native_result.llVal));
#else
                        return finalize_integer_result(static_cast<std::uintptr_t>(
                            static_cast<std::uint32_t>(native_result.lVal)));
#endif
                    }
                    return ret_integer64
                               ? finalize_integer_result(native_result.llVal)
                               : finalize_integer_result(native_result.lVal);
                };

#if defined(_WIN64)

                if (has_single_parameter || ret_single || ret_short)
                {
                    return invoke_via_disp_call_func();
                }

                std::vector<detail::Win64NativeCallArgument> native_arguments(nargs);
                for (std::size_t index = 0U; index < nargs; ++index)
                {
                    if (flat[index].is_double)
                    {
                        native_arguments[index].double_value = flat[index].d;
                        native_arguments[index].is_double = true;
                    }
                    else
                    {
                        native_arguments[index].integer_value = static_cast<std::uint64_t>(flat[index].i);
                    }
                }

                const detail::Win64NativeCallResult native_result = detail::invoke_win64_native_function(
                    declfn.native_function_address,
                    native_arguments,
                    ret_double
                        ? detail::Win64NativeReturnKind::floating64
                        : (ret_string
                               ? detail::Win64NativeReturnKind::string_pointer
                               : (ret_integer64
                                      ? detail::Win64NativeReturnKind::integer64
                                      : detail::Win64NativeReturnKind::integer32)));
                return ret_double
                           ? finalize_double_result(native_result.double_value)
                           : (ret_string
                                  ? finalize_integer_result(reinterpret_cast<std::uintptr_t>(native_result.string_pointer))
                                  : (ret_integer64
                                         ? finalize_integer_result(static_cast<std::int64_t>(native_result.integer_value))
                                         : finalize_integer_result(static_cast<std::int32_t>(native_result.integer_value))));
#else
                // Win32 uses stdcall for native DECLARE and requires type-aware
                // stack slots for every signature, not only those containing SINGLE.
                return invoke_via_disp_call_func();
#endif
            }
#else
            (void)declfn;
            (void)args;
            (void)argument_references;
            return make_empty_value();
#endif
        }

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

                // Preserve VFP's bounded native-call contract before crossing
                // the portable request/result boundary.
                if (args.size() > 8U)
                {
                    throw PrgCompatibilityError(
                        runtime_text(
                            "Runtime.Prg.Dll.Error.NativeArgumentLimitExceeded",
                            {
                                {"count", std::to_string(args.size())},
                                {"maximum", "8"}
                            }),
                        1230);
                }

                NativeDeclaredCallRequest request;
                request.function_address = declfn.native_function_address;
                request.use_cdecl = declfn.native_cdecl;
                request.arguments.reserve(args.size());
                std::vector<std::optional<std::string>> writeback_references(args.size());

                for (std::size_t index = 0U; index < args.size(); ++index)
                {
                    const std::string parameter_type = normalize_identifier(param_type_at(index));
                    NativeDeclaredArgument argument;
                    argument.by_reference =
                        param_is_by_ref(index) &&
                        index < argument_references.size() &&
                        argument_references[index].has_value();
                    if (argument.by_reference)
                    {
                        writeback_references[index] = argument_references[index];
                    }

                    if (parameter_type == "string" || parameter_type == "c")
                    {
                        argument.kind = NativeDeclaredArgumentKind::string;
                        argument.string_value = value_as_string(args[index]);
                    }
                    else if (declared_dll_type_is_single(parameter_type))
                    {
                        argument.kind = NativeDeclaredArgumentKind::floating_point32;
                        argument.floating_point_value =
                            static_cast<float>(value_as_number(args[index]));
                    }
                    else if (
                        parameter_type == "double" ||
                        parameter_type == "d" ||
                        parameter_type == "f")
                    {
                        argument.kind = NativeDeclaredArgumentKind::floating_point64;
                        argument.floating_point_value = value_as_number(args[index]);
                    }
                    else if (declared_dll_type_uses_64_bit_integer(parameter_type))
                    {
                        argument.kind = NativeDeclaredArgumentKind::signed_integer64;
                        argument.signed_integer_value = exact_declared_integer_value(args[index]);
                    }
                    else
                    {
                        argument.kind = NativeDeclaredArgumentKind::signed_integer32;
                        argument.signed_integer_value =
                            static_cast<std::int32_t>(value_as_number(args[index]));
                    }
                    request.arguments.push_back(std::move(argument));
                }

                const std::string return_type = normalize_identifier(declfn.return_type);
                if (declared_dll_type_is_short(return_type))
                {
                    request.return_kind = NativeDeclaredReturnKind::signed_integer16;
                }
                else if (declared_dll_type_is_single(return_type))
                {
                    request.return_kind = NativeDeclaredReturnKind::floating_point32;
                }
                else if (
                    return_type == "double" ||
                    return_type == "d" ||
                    return_type == "f")
                {
                    request.return_kind = NativeDeclaredReturnKind::floating_point64;
                }
                else if (return_type == "c" || return_type == "string")
                {
                    request.return_kind = NativeDeclaredReturnKind::string;
                }
                else if (declared_dll_type_uses_64_bit_integer(return_type))
                {
                    request.return_kind = NativeDeclaredReturnKind::signed_integer64;
                }
                else
                {
                    request.return_kind = NativeDeclaredReturnKind::signed_integer32;
                }

                const NativeDeclaredCallResult native_result =
                    invoke_native_declared_function(request);
                if (!native_result.succeeded)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dll.Error.NativeInvokeFailed",
                        {
                            {"functionName", declfn.function_name},
                            {"hresult", std::to_string(native_result.compatible_error_code)}
                        });
                    return make_empty_value();
                }

                if (!stack.empty())
                {
                    const std::size_t writeback_count =
                        std::min(native_result.arguments.size(), writeback_references.size());
                    for (std::size_t index = 0U; index < writeback_count; ++index)
                    {
                        if (!writeback_references[index].has_value() ||
                            writeback_references[index].value().empty())
                        {
                            continue;
                        }

                        const NativeDeclaredArgument &updated =
                            native_result.arguments[index];
                        PrgValue value;
                        switch (updated.kind)
                        {
                        case NativeDeclaredArgumentKind::string:
                            value = make_string_value(updated.string_value);
                            break;
                        case NativeDeclaredArgumentKind::floating_point32:
                        case NativeDeclaredArgumentKind::floating_point64:
                            value = make_number_value(updated.floating_point_value);
                            break;
                        case NativeDeclaredArgumentKind::signed_integer64:
                            value = make_int64_value(updated.signed_integer_value);
                            break;
                        case NativeDeclaredArgumentKind::signed_integer32:
                            value = make_number_value(
                                static_cast<double>(updated.signed_integer_value));
                            break;
                        }
                        assign_variable(
                            stack.back(),
                            writeback_references[index].value(),
                            value);
                    }
                }

                switch (request.return_kind)
                {
                case NativeDeclaredReturnKind::floating_point32:
                case NativeDeclaredReturnKind::floating_point64:
                    return make_number_value(native_result.floating_point_value);
                case NativeDeclaredReturnKind::string:
                    return make_string_value(native_result.string_value);
                case NativeDeclaredReturnKind::signed_integer64:
                    return make_int64_value(native_result.signed_integer_value);
                case NativeDeclaredReturnKind::signed_integer16:
                case NativeDeclaredReturnKind::signed_integer32:
                    return make_number_value(
                        static_cast<double>(native_result.signed_integer_value));
                }
                return make_number_value(0.0);
            }
#else
            (void)declfn;
            (void)args;
            (void)argument_references;
            return make_empty_value();
#endif
        }

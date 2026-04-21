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
                last_error_message = "FOXTOOLS is not loaded";
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
            events.push_back({.category = "interop.regfn",
                              .detail = variant + ":" + function_name + "@" + dll_name + " -> " + std::to_string(handle),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return handle;
        }

        PrgValue call_registered_api_function(int handle, const std::vector<PrgValue> &arguments)
        {
            const auto &registered_functions = current_registered_api_functions();
            const auto found = registered_functions.find(handle);
            if (found == registered_functions.end())
            {
                last_error_message = "Registered API handle not found: " + std::to_string(handle);
                return make_number_value(-1.0);
            }

            const RegisteredApiFunction &function = found->second;
            events.push_back({.category = "interop.callfn",
                              .detail = function.function_name + " (" + std::to_string(arguments.size()) + " args)",
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
        PrgValue invoke_declared_dll_function(const std::string &fn_key, const std::vector<PrgValue> &args)
        {
            const std::string key = normalize_identifier(fn_key);
            const auto found = declared_dll_functions.find(key);
            if (found == declared_dll_functions.end())
                return make_empty_value();
            const DeclaredDllFunction &declfn = found->second;

#if defined(_WIN32)
            // Split comma-separated param_types string into a vector for indexed access
            std::vector<std::string> param_type_list;
            {
                std::istringstream ss(declfn.param_types);
                std::string tok;
                while (std::getline(ss, tok, ','))
                {
                    while (!tok.empty() && tok.front() == ' ')
                        tok.erase(tok.begin());
                    while (!tok.empty() && tok.back() == ' ')
                        tok.pop_back();
                    if (!tok.empty())
                        param_type_list.push_back(tok);
                }
            }

            auto param_type_at = [&](std::size_t i) -> std::string
            {
                return i < param_type_list.size() ? param_type_list[i] : std::string("integer");
            };

            // Helper: convert PrgValue → VARIANT
            auto to_variant = [&](const PrgValue &v, const std::string &ptype) -> VARIANT
            {
                VARIANT var;
                VariantInit(&var);
                const std::string pt = normalize_identifier(ptype);
                // by-ref marker stripped for marshalling
                const bool by_ref = !pt.empty() && pt.back() == '@';
                const std::string base = by_ref ? pt.substr(0, pt.size() - 1) : pt;
                if (base == "string" || base == "c")
                {
                    std::string s = value_as_string(v);
                    var.vt = VT_BSTR;
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
                    std::wstring ws(wlen, 0);
                    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, ws.data(), wlen);
                    var.bstrVal = SysAllocString(ws.c_str());
                }
                else if (base == "double" || base == "d" || base == "f")
                {
                    var.vt = VT_R8;
                    var.dblVal = value_as_number(v);
                }
                else if (base == "long" || base == "longlong" || base == "integer64" || base == "i64")
                {
                    var.vt = VT_I8;
                    var.llVal = static_cast<LONGLONG>(value_as_number(v));
                }
                else
                {
                    // Default: INTEGER / SHORT / WORD → VT_I4
                    var.vt = VT_I4;
                    var.lVal = static_cast<LONG>(value_as_number(v));
                }
                return var;
            };

            // Helper: VARIANT → PrgValue based on return_type
            auto from_variant = [&](const VARIANT &var) -> PrgValue
            {
                const std::string rt = normalize_identifier(declfn.return_type);
                if (rt == "c" || rt == "string")
                {
                    if (var.vt == VT_BSTR && var.bstrVal)
                    {
                        int len = WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1, nullptr, 0, nullptr, nullptr);
                        std::string s(len, 0);
                        WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1, s.data(), len, nullptr, nullptr);
                        if (!s.empty() && s.back() == '\0')
                            s.pop_back();
                        return make_string_value(s);
                    }
                    return make_string_value({});
                }
                if (var.vt == VT_I8 || var.vt == VT_UI8)
                    return make_number_value(static_cast<double>(var.llVal));
                if (var.vt == VT_R4)
                    return make_number_value(static_cast<double>(var.fltVal));
                if (var.vt == VT_R8)
                    return make_number_value(var.dblVal);
                if (var.vt == VT_BOOL)
                    return make_boolean_value(var.boolVal != VARIANT_FALSE);
                // Integer family
                if (var.vt == VT_I4 || var.vt == VT_UI4)
                    return make_number_value(static_cast<double>(var.lVal));
                if (var.vt == VT_I2 || var.vt == VT_UI2)
                    return make_number_value(static_cast<double>(var.iVal));
                if (var.vt == VT_I1 || var.vt == VT_UI1)
                    return make_number_value(static_cast<double>(var.bVal));
                return make_number_value(static_cast<double>(var.intVal));
            };

            if (declfn.is_dotnet)
            {
                // ---------------------------------------------------------------
                // .NET CLR hosting via COM (ICLRMetaHost / ICorRuntimeHost)
                // ---------------------------------------------------------------
                // Single-init static COM state (process-wide, non-thread-safe for
                // simplicity; adequate for VFP-style single-threaded programs).
                static ICorRuntimeHost *s_runtime_host = nullptr;
                static bool s_clr_started = false;
                if (!s_clr_started)
                {
                    ICLRMetaHost *metahost = nullptr;
                    HRESULT hr2 = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, reinterpret_cast<void **>(&metahost));
                    if (FAILED(hr2))
                    {
                        last_error_message = "CLRCreateInstance failed: " + std::to_string(hr2);
                        return make_empty_value();
                    }
                    ICLRRuntimeInfo *runtime_info = nullptr;
                    hr2 = metahost->GetRuntime(L"v4.0.30319", IID_ICLRRuntimeInfo, reinterpret_cast<void **>(&runtime_info));
                    if (FAILED(hr2))
                    {
                        IEnumUnknown *enumerator = nullptr;
                        metahost->EnumerateInstalledRuntimes(&enumerator);
                        if (enumerator)
                        {
                            IUnknown *rt_unk = nullptr;
                            ULONG fetched = 0;
                            while (enumerator->Next(1, &rt_unk, &fetched) == S_OK && fetched > 0)
                            {
                                if (runtime_info)
                                    runtime_info->Release();
                                rt_unk->QueryInterface(IID_ICLRRuntimeInfo, reinterpret_cast<void **>(&runtime_info));
                                rt_unk->Release();
                            }
                            enumerator->Release();
                        }
                    }
                    if (runtime_info == nullptr)
                    {
                        metahost->Release();
                        last_error_message = "No CLR runtime found";
                        return make_empty_value();
                    }
                    hr2 = runtime_info->GetInterface(CLSID_CorRuntimeHost, IID_ICorRuntimeHost, reinterpret_cast<void **>(&s_runtime_host));
                    runtime_info->Release();
                    metahost->Release();
                    if (FAILED(hr2) || s_runtime_host == nullptr)
                    {
                        last_error_message = "Failed to get ICorRuntimeHost: " + std::to_string(hr2);
                        return make_empty_value();
                    }
                    s_runtime_host->Start();
                    s_clr_started = true;
                }

                // IDispatch late-binding helper: call a named method on a COM object
                auto dispatch_call = [](IDispatch *obj, const wchar_t *method_name,
                                        std::vector<VARIANT> args, VARIANT *ret_out) -> HRESULT
                {
                    BSTR bname = SysAllocString(method_name);
                    DISPID dispid = DISPID_UNKNOWN;
                    HRESULT hr = obj->GetIDsOfNames(IID_NULL, &bname, 1, LOCALE_USER_DEFAULT, &dispid);
                    SysFreeString(bname);
                    if (FAILED(hr))
                        return hr;
                    // IDispatch args must be in reverse order
                    std::reverse(args.begin(), args.end());
                    DISPPARAMS dp{};
                    dp.rgvarg = args.empty() ? nullptr : args.data();
                    dp.cArgs = static_cast<UINT>(args.size());
                    EXCEPINFO exc{};
                    return obj->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                                       &dp, ret_out, &exc, nullptr);
                };

                // Get default AppDomain as IDispatch
                IUnknown *app_domain_unk = nullptr;
                HRESULT hr = s_runtime_host->GetDefaultDomain(&app_domain_unk);
                if (FAILED(hr) || app_domain_unk == nullptr)
                {
                    last_error_message = "GetDefaultDomain failed: " + std::to_string(hr);
                    return make_empty_value();
                }
                IDispatch *app_domain_disp = nullptr;
                hr = app_domain_unk->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&app_domain_disp));
                app_domain_unk->Release();
                if (FAILED(hr) || app_domain_disp == nullptr)
                {
                    last_error_message = "AppDomain QueryInterface IDispatch failed";
                    return make_empty_value();
                }

                // Load assembly: AppDomain.LoadFrom(path)
                std::wstring asm_path_w(declfn.dll_path.begin(), declfn.dll_path.end());
                VARIANT vpath;
                VariantInit(&vpath);
                vpath.vt = VT_BSTR;
                vpath.bstrVal = SysAllocString(asm_path_w.c_str());
                VARIANT v_assembly;
                VariantInit(&v_assembly);
                hr = dispatch_call(app_domain_disp, L"Load", {vpath}, &v_assembly);
                VariantClear(&vpath);
                app_domain_disp->Release();

                // Try LoadFrom on AppDomain.CurrentDomain if Load fails
                if (FAILED(hr) || v_assembly.vt == VT_EMPTY || v_assembly.vt == VT_NULL)
                {
                    // We couldn't load: return a graceful empty
                    VariantClear(&v_assembly);
                    last_error_message = "Could not load .NET assembly: " + declfn.dll_path + " hr=" + std::to_string(hr);
                    return make_empty_value();
                }

                IDispatch *assembly_disp = nullptr;
                if (v_assembly.vt == VT_DISPATCH)
                    assembly_disp = v_assembly.pdispVal;
                else if (v_assembly.vt == (VT_DISPATCH | VT_BYREF) && v_assembly.ppdispVal)
                    assembly_disp = *v_assembly.ppdispVal;
                if (!assembly_disp)
                {
                    VariantClear(&v_assembly);
                    last_error_message = "Assembly is not IDispatch";
                    return make_empty_value();
                }
                assembly_disp->AddRef();
                VariantClear(&v_assembly);

                // GetType(type_name)
                std::wstring type_name_w(declfn.dotnet_type_name.begin(), declfn.dotnet_type_name.end());
                VARIANT vtn;
                VariantInit(&vtn);
                vtn.vt = VT_BSTR;
                vtn.bstrVal = SysAllocString(type_name_w.c_str());
                VARIANT v_type;
                VariantInit(&v_type);
                hr = dispatch_call(assembly_disp, L"GetType", {vtn}, &v_type);
                VariantClear(&vtn);
                assembly_disp->Release();
                IDispatch *type_disp = nullptr;
                if (SUCCEEDED(hr) && v_type.vt == VT_DISPATCH)
                    type_disp = v_type.pdispVal;
                if (!type_disp)
                {
                    VariantClear(&v_type);
                    last_error_message = "Type not found: " + declfn.dotnet_type_name;
                    return make_empty_value();
                }
                type_disp->AddRef();
                VariantClear(&v_type);

                // GetMethod(method_name)
                std::wstring method_name_w(declfn.dotnet_method_name.begin(), declfn.dotnet_method_name.end());
                VARIANT vmn;
                VariantInit(&vmn);
                vmn.vt = VT_BSTR;
                vmn.bstrVal = SysAllocString(method_name_w.c_str());
                VARIANT v_method;
                VariantInit(&v_method);
                hr = dispatch_call(type_disp, L"GetMethod", {vmn}, &v_method);
                VariantClear(&vmn);
                type_disp->Release();
                IDispatch *method_disp = nullptr;
                if (SUCCEEDED(hr) && v_method.vt == VT_DISPATCH)
                    method_disp = v_method.pdispVal;
                if (!method_disp)
                {
                    VariantClear(&v_method);
                    last_error_message = "Method not found: " + declfn.dotnet_method_name;
                    return make_empty_value();
                }
                method_disp->AddRef();
                VariantClear(&v_method);

                // Build args SAFEARRAY wrapped in VARIANT for Invoke(null, args[])
                SAFEARRAY *sa = SafeArrayCreateVector(VT_VARIANT, 0, static_cast<ULONG>(args.size()));
                for (LONG idx = 0; idx < static_cast<LONG>(args.size()); ++idx)
                {
                    const std::string ptype = param_type_at(static_cast<std::size_t>(idx));
                    VARIANT v = to_variant(args[static_cast<std::size_t>(idx)], ptype);
                    SafeArrayPutElement(sa, &idx, &v);
                    VariantClear(&v);
                }
                VARIANT vsa;
                VariantInit(&vsa);
                vsa.vt = VT_ARRAY | VT_VARIANT;
                vsa.parray = sa;

                VARIANT vnull;
                VariantInit(&vnull);
                vnull.vt = VT_NULL; // static method — null target

                VARIANT ret_var;
                VariantInit(&ret_var);
                hr = dispatch_call(method_disp, L"Invoke", {vnull, vsa}, &ret_var);
                VariantClear(&vsa); // also destroys sa
                VariantClear(&vnull);
                method_disp->Release();

                if (FAILED(hr))
                {
                    last_error_message = "Method invoke failed: " + std::to_string(hr);
                    return make_empty_value();
                }
                PrgValue result = from_variant(ret_var);
                VariantClear(&ret_var);
                return result;
            }
            else
            {
                // ---------------------------------------------------------------
                // Native DLL invocation via proc_address
                // ---------------------------------------------------------------
                if (declfn.proc_address == nullptr)
                {
                    last_error_message = "No proc address for: " + declfn.function_name;
                    return make_empty_value();
                }

                // Build integer/double argument lists for common calling conventions.
                // We support the same limited set as VFP's DECLARE: up to 8 args,
                // typed as INTEGER/LONG/DOUBLE/STRING.
                // For STRING params we pass a pointer to the UTF-8 buffer.
                // We do not attempt to pack varargs generically; instead we use a
                // dispatch table keyed on arg count (0-8), which covers the vast
                // majority of real-world DLL calls.

                // Convert args to a flat array of 64-bit values (integers/pointers)
                // and a parallel doubles array.
                std::vector<std::string> string_buffers; // keep alive through the call
                struct Arg64
                {
                    __int64 i;
                    double d;
                    bool is_double;
                };
                std::vector<Arg64> flat;
                flat.reserve(args.size());
                for (std::size_t idx = 0; idx < args.size(); ++idx)
                {
                    const std::string ptype = normalize_identifier(param_type_at(idx));
                    const std::string base_pt = (!ptype.empty() && ptype.back() == '@')
                                                    ? ptype.substr(0, ptype.size() - 1)
                                                    : ptype;
                    Arg64 a{};
                    if (base_pt == "double" || base_pt == "d" || base_pt == "f")
                    {
                        a.d = value_as_number(args[idx]);
                        a.is_double = true;
                    }
                    else if (base_pt == "string" || base_pt == "c")
                    {
                        std::string s = value_as_string(args[idx]);
                        string_buffers.push_back(std::move(s));
                        a.i = reinterpret_cast<__int64>(string_buffers.back().c_str());
                    }
                    else
                    {
                        a.i = static_cast<__int64>(value_as_number(args[idx]));
                    }
                    flat.push_back(a);
                }

                // Extract all args as __int64 (works for int, ptr, and bitcast of double)
                auto iarg = [&](std::size_t i) -> __int64
                {
                    if (i >= flat.size())
                        return 0LL;
                    return flat[i].is_double
                               ? *reinterpret_cast<const __int64 *>(&flat[i].d)
                               : flat[i].i;
                };

                // Call the function. We cast to a stdcall prototype (VFP default on x86
                // for DECLARE; on x64 there is only one calling convention).
                // Return value is either integer or double based on return_type.
                const std::string rt = normalize_identifier(declfn.return_type);
                const bool ret_double = (rt == "double" || rt == "d" || rt == "f");
                const bool ret_string = (rt == "c" || rt == "string");
                const std::size_t nargs = flat.size();

#if defined(_WIN64)
                // On x64 Windows, calling convention is unified.
                typedef __int64 (*FnI_0)();
                typedef __int64 (*FnI_1)(__int64);
                typedef __int64 (*FnI_2)(__int64, __int64);
                typedef __int64 (*FnI_3)(__int64, __int64, __int64);
                typedef __int64 (*FnI_4)(__int64, __int64, __int64, __int64);
                typedef __int64 (*FnI_5)(__int64, __int64, __int64, __int64, __int64);
                typedef __int64 (*FnI_6)(__int64, __int64, __int64, __int64, __int64, __int64);
                typedef __int64 (*FnI_7)(__int64, __int64, __int64, __int64, __int64, __int64, __int64);
                typedef __int64 (*FnI_8)(__int64, __int64, __int64, __int64, __int64, __int64, __int64, __int64);
                typedef double (*FnD_0)();
                typedef double (*FnD_1)(__int64);
                typedef double (*FnD_2)(__int64, __int64);
                typedef double (*FnD_3)(__int64, __int64, __int64);
                typedef double (*FnD_4)(__int64, __int64, __int64, __int64);

                FARPROC fn = declfn.proc_address;
                __int64 iret = 0;
                double dret = 0.0;
                if (ret_double)
                {
                    switch (nargs)
                    {
                    case 0:
                        dret = reinterpret_cast<FnD_0>(fn)();
                        break;
                    case 1:
                        dret = reinterpret_cast<FnD_1>(fn)(iarg(0));
                        break;
                    case 2:
                        dret = reinterpret_cast<FnD_2>(fn)(iarg(0), iarg(1));
                        break;
                    case 3:
                        dret = reinterpret_cast<FnD_3>(fn)(iarg(0), iarg(1), iarg(2));
                        break;
                    default:
                        dret = reinterpret_cast<FnD_4>(fn)(iarg(0), iarg(1), iarg(2), iarg(3));
                        break;
                    }
                    return make_number_value(dret);
                }
                else
                {
                    switch (nargs)
                    {
                    case 0:
                        iret = reinterpret_cast<FnI_0>(fn)();
                        break;
                    case 1:
                        iret = reinterpret_cast<FnI_1>(fn)(iarg(0));
                        break;
                    case 2:
                        iret = reinterpret_cast<FnI_2>(fn)(iarg(0), iarg(1));
                        break;
                    case 3:
                        iret = reinterpret_cast<FnI_3>(fn)(iarg(0), iarg(1), iarg(2));
                        break;
                    case 4:
                        iret = reinterpret_cast<FnI_4>(fn)(iarg(0), iarg(1), iarg(2), iarg(3));
                        break;
                    case 5:
                        iret = reinterpret_cast<FnI_5>(fn)(iarg(0), iarg(1), iarg(2), iarg(3), iarg(4));
                        break;
                    case 6:
                        iret = reinterpret_cast<FnI_6>(fn)(iarg(0), iarg(1), iarg(2), iarg(3), iarg(4), iarg(5));
                        break;
                    case 7:
                        iret = reinterpret_cast<FnI_7>(fn)(iarg(0), iarg(1), iarg(2), iarg(3), iarg(4), iarg(5), iarg(6));
                        break;
                    default:
                        iret = reinterpret_cast<FnI_8>(fn)(iarg(0), iarg(1), iarg(2), iarg(3), iarg(4), iarg(5), iarg(6), iarg(7));
                        break;
                    }
                    if (ret_string)
                    {
                        const char *p = reinterpret_cast<const char *>(iret);
                        return make_string_value(p ? std::string(p) : std::string{});
                    }
                    return make_number_value(static_cast<double>(iret));
                }
#else
                // x86: use __stdcall by default (VFP DECLARE default)
                typedef __int32(__stdcall * FnSI_0)();
                typedef __int32(__stdcall * FnSI_1)(__int32);
                typedef __int32(__stdcall * FnSI_2)(__int32, __int32);
                typedef __int32(__stdcall * FnSI_3)(__int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_4)(__int32, __int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_5)(__int32, __int32, __int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_6)(__int32, __int32, __int32, __int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_7)(__int32, __int32, __int32, __int32, __int32, __int32, __int32);
                typedef __int32(__stdcall * FnSI_8)(__int32, __int32, __int32, __int32, __int32, __int32, __int32, __int32);

                auto i32 = [&](std::size_t i) -> __int32
                { return static_cast<__int32>(iarg(i)); };

                FARPROC fn = declfn.proc_address;
                __int32 iret = 0;
                switch (nargs)
                {
                case 0:
                    iret = reinterpret_cast<FnSI_0>(fn)();
                    break;
                case 1:
                    iret = reinterpret_cast<FnSI_1>(fn)(i32(0));
                    break;
                case 2:
                    iret = reinterpret_cast<FnSI_2>(fn)(i32(0), i32(1));
                    break;
                case 3:
                    iret = reinterpret_cast<FnSI_3>(fn)(i32(0), i32(1), i32(2));
                    break;
                case 4:
                    iret = reinterpret_cast<FnSI_4>(fn)(i32(0), i32(1), i32(2), i32(3));
                    break;
                case 5:
                    iret = reinterpret_cast<FnSI_5>(fn)(i32(0), i32(1), i32(2), i32(3), i32(4));
                    break;
                case 6:
                    iret = reinterpret_cast<FnSI_6>(fn)(i32(0), i32(1), i32(2), i32(3), i32(4), i32(5));
                    break;
                case 7:
                    iret = reinterpret_cast<FnSI_7>(fn)(i32(0), i32(1), i32(2), i32(3), i32(4), i32(5), i32(6));
                    break;
                default:
                    iret = reinterpret_cast<FnSI_8>(fn)(i32(0), i32(1), i32(2), i32(3), i32(4), i32(5), i32(6), i32(7));
                    break;
                }
                if (ret_string)
                {
                    const char *p = reinterpret_cast<const char *>(iret);
                    return make_string_value(p ? std::string(p) : std::string{});
                }
                return make_number_value(static_cast<double>(iret));
#endif
            }
#else
            (void)declfn;
            (void)args;
            return make_empty_value();
#endif
        }


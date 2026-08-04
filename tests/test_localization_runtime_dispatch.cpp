// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_localization_support.h"

void test_runtime_record_precondition_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap replace_placeholders{{"command", "REPLACE"}};
    const copperfin::localization::PlaceholderMap field_placeholders{{"fieldName", "CustomerID"}};
    const copperfin::localization::PlaceholderMap append_blank_placeholders{{"command", "APPEND BLANK"}};
    const copperfin::localization::PlaceholderMap constraint_placeholders{
        {"constraint", "NOT NULL"},
        {"fieldName", "CustomerID"}
    };
    const copperfin::localization::PlaceholderMap insert_placeholders{{"command", "INSERT INTO"}};

    expect(
        english.translate("Runtime.Prg.Records.Error.RequiresLocalTableBackedCursor") ==
            "This command requires a local table-backed cursor",
        "#2542: generic local table-backed cursor error should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Records.Error.CommandRequiresCurrentRemoteRecord", replace_placeholders) ==
            "REPLACE requires a current remote record",
        "#2542: command-specific remote record error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.CommandRequiresCurrentLocalRecord", replace_placeholders) ==
            "REPLACE requires a current local record",
        "#2542: command-specific local record error should preserve command placeholder");
    expect(
        english.translate(
            "Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor",
            append_blank_placeholders) == "APPEND BLANK requires a local table-backed cursor",
        "#2547: command-specific local table-backed cursor error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.RemoteSqlFieldNotFound", field_placeholders) ==
            "Field not found on remote SQL cursor: CustomerID",
        "#2542: remote SQL field error should preserve field-name placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.LockRetryCancelled") == "Lock retry cancelled.",
        "#2542: lock retry cancellation should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Records.Error.ConstraintFieldNotFound", constraint_placeholders) ==
            "NOT NULL field not found: CustomerID",
        "#2546: NOT NULL field-not-found error should preserve constraint and field placeholders");
    expect(
        english.translate("Runtime.Prg.Records.Error.ConstraintFailedForField", constraint_placeholders) ==
            "NOT NULL constraint failed for field: CustomerID",
        "#2546: NOT NULL constraint error should preserve constraint and field placeholders");
    expect(
        english.translate("Runtime.Prg.Records.Error.InsertTargetFieldsResolveFailed", insert_placeholders) ==
            "INSERT INTO could not resolve target field names",
        "#2546: INSERT INTO field resolution error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.InsertRequiresTargetField", insert_placeholders) ==
            "INSERT INTO requires at least one target field",
        "#2546: INSERT INTO target-field error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.InsertFieldValueCountMismatch", insert_placeholders) ==
            "INSERT INTO field/value counts do not match",
        "#2546: INSERT INTO count mismatch error should preserve command placeholder");

    const std::string spanish_field =
        spanish.translate("Runtime.Prg.Records.Error.RemoteSqlFieldNotFound", field_placeholders);
    expect(
        spanish_field.find("CustomerID") != std::string::npos &&
            spanish_field.find("Field not found") == std::string::npos,
        "#2542: es-419 remote SQL field error should preserve field name without falling back to English");

    const std::string portuguese_replace =
        portuguese.translate("Runtime.Prg.Records.Error.CommandRequiresCurrentLocalRecord", replace_placeholders);
    expect(
        portuguese_replace.find("REPLACE") != std::string::npos &&
            portuguese_replace.find("requires a current local record") == std::string::npos,
        "#2542: pt-BR command-specific record error should preserve command without falling back to English");
    const std::string spanish_append =
        spanish.translate("Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor", append_blank_placeholders);
    expect(
        spanish_append.find("APPEND BLANK") != std::string::npos &&
            spanish_append.find("requires a local table-backed cursor") == std::string::npos,
        "#2547: es-419 table-backed cursor error should preserve command without falling back to English");
    const std::string spanish_insert =
        spanish.translate("Runtime.Prg.Records.Error.InsertFieldValueCountMismatch", insert_placeholders);
    expect(
        spanish_insert.find("INSERT INTO") != std::string::npos &&
            spanish_insert.find("field/value counts do not match") == std::string::npos,
        "#2546: es-419 INSERT INTO error should preserve command without falling back to English");
    const std::string portuguese_constraint =
        portuguese.translate("Runtime.Prg.Records.Error.ConstraintFailedForField", constraint_placeholders);
    expect(
        portuguese_constraint.find("NOT NULL") != std::string::npos &&
            portuguese_constraint.find("CustomerID") != std::string::npos &&
            portuguese_constraint.find("constraint failed") == std::string::npos,
        "#2546: pt-BR NOT NULL error should preserve invariant placeholders without falling back to English");

    const std::string pseudo_message =
        pseudo.translate("Runtime.Prg.Records.Error.RemoteSqlFieldNotFound", field_placeholders);
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message.find("CustomerID") != std::string::npos &&
            pseudo_message.find("{fieldName}") == std::string::npos,
        "#2542: qps-ploc record precondition error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_constraint =
        pseudo.translate("Runtime.Prg.Records.Error.ConstraintFieldNotFound", constraint_placeholders);
    expect(
        pseudo_constraint.find("[!! ") == 0U &&
            pseudo_constraint.find("NOT NULL") != std::string::npos &&
            pseudo_constraint.find("CustomerID") != std::string::npos &&
            pseudo_constraint.find("{constraint}") == std::string::npos &&
            pseudo_constraint.find("{fieldName}") == std::string::npos,
        "#2546: qps-ploc NOT NULL error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_append =
        pseudo.translate("Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor", append_blank_placeholders);
    expect(
        pseudo_append.find("[!! ") == 0U &&
            pseudo_append.find("APPEND BLANK") != std::string::npos &&
            pseudo_append.find("{command}") == std::string::npos,
        "#2547: qps-ploc table-backed cursor error should pseudo-localize prose while preserving command");
}

void test_runtime_dll_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap handle_placeholders{{"handle", "42"}};
    const copperfin::localization::PlaceholderMap hresult_placeholders{{"hresult", "-2146232576"}};
    const copperfin::localization::PlaceholderMap assembly_placeholders{
        {"hresult", "-2147024894"},
        {"path", "bin/Interop.dll"}
    };
    const copperfin::localization::PlaceholderMap type_placeholders{{"typeName", "Copperfin.Tools.Loader"}};
    const copperfin::localization::PlaceholderMap function_placeholders{{"functionName", "GetVersion"}};
    const copperfin::localization::PlaceholderMap native_limit_placeholders{
        {"count", "9"},
        {"maximum", "8"}
    };
    const copperfin::localization::PlaceholderMap native_invoke_placeholders{
        {"functionName", "pow"},
        {"hresult", "-2147352568"}
    };
    const copperfin::localization::PlaceholderMap numeric_byref_placeholders{
        {"functionName", "NumericByRef"},
        {"position", "4"}
    };
    const std::vector<std::string> native_keys{
        "Runtime.Prg.Dll.Error.NativeArgumentLimitExceeded",
        "Runtime.Prg.Dll.Error.NativeInvokeFailed",
        "Runtime.Prg.Dll.Error.NumericByReferenceArgumentRequired",
        "Runtime.Prg.Dll.Error.TooFewArguments",
        "Runtime.Prg.Dll.Error.TooManyArguments"
    };

    expect(
        english.translate("Runtime.Prg.Dll.Error.FoxtoolsNotLoaded") == "FOXTOOLS is not loaded",
        "#2548: FOXTOOLS load precondition should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Dll.Error.RegisteredApiHandleNotFound", handle_placeholders) ==
            "Registered API handle not found: 42",
        "#2548: API handle error should preserve handle placeholder");
    expect(
        english.translate("Runtime.Prg.Dll.Error.ClrCreateInstanceFailed", hresult_placeholders) ==
            "CLRCreateInstance failed: -2146232576",
        "#2549: CLRCreateInstance error should preserve HRESULT placeholder");
    expect(
        english.translate("Runtime.Prg.Dll.Error.DotNetAssemblyLoadFailed", assembly_placeholders) ==
            "Could not load .NET assembly: bin/Interop.dll hr=-2147024894",
        "#2549: .NET assembly load error should preserve path and HRESULT placeholders");
    expect(
        english.translate("Runtime.Prg.Dll.Error.DotNetTypeNotFound", type_placeholders) ==
            "Type not found: Copperfin.Tools.Loader",
        "#2549: .NET type lookup error should preserve type-name placeholder");
    expect(
        english.translate("Runtime.Prg.Dll.Error.NativeProcAddressMissing", function_placeholders) ==
            "No proc address for: GetVersion",
        "#2550: native proc-address error should preserve function-name placeholder");
    expect(
        english.translate("Runtime.Prg.Dll.Error.NativeArgumentLimitExceeded", native_limit_placeholders) ==
            "Native DLL call has 9 arguments; the maximum is 8",
        "#3895: native argument-limit error should preserve count placeholders");
    expect(
        english.translate("Runtime.Prg.Dll.Error.NativeInvokeFailed", native_invoke_placeholders) ==
            "Native DLL function pow failed to invoke: -2147352568",
        "#3895: native invocation error should preserve function and HRESULT placeholders");
    expect(
        english.translate(
            "Runtime.Prg.Dll.Error.NumericByReferenceArgumentRequired",
            numeric_byref_placeholders) ==
            "Native DLL function NumericByRef argument 4 is declared numeric by reference and requires a call-site @ variable.",
        "#3944: numeric by-reference rejection should preserve alias and position placeholders");
    expect(
        english.translate("Runtime.Prg.Dll.Error.TooFewArguments") == "Too few arguments." &&
            english.translate("Runtime.Prg.Dll.Error.TooManyArguments") == "Too many arguments.",
        "#3946: DECLARE arity errors should preserve the grounded VFP9 en-US wording");
    for (const std::string &native_key : native_keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(native_key),
            "#3895: es-419 should define every native invocation key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(native_key),
            "#3895: pt-BR should define every native invocation key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(native_key),
            "#3895: qps-ploc should define every native invocation key");
    }
    expect(
        spanish.translate("Runtime.Prg.Dll.Error.FoxtoolsNotLoaded").find("FOXTOOLS") != std::string::npos &&
            spanish.translate("Runtime.Prg.Dll.Error.FoxtoolsNotLoaded").find("is not loaded") == std::string::npos,
        "#2548: es-419 FOXTOOLS error should preserve invariant product token without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dll.Error.DotNetTypeNotFound", type_placeholders)
                    .find("Copperfin.Tools.Loader") != std::string::npos &&
            spanish.translate("Runtime.Prg.Dll.Error.DotNetTypeNotFound", type_placeholders)
                    .find("Type not found") == std::string::npos,
        "#2549: es-419 .NET type error should preserve type name without falling back to English");
    expect(
        spanish.translate(
            "Runtime.Prg.Dll.Error.NumericByReferenceArgumentRequired",
            numeric_byref_placeholders).find("NumericByRef") != std::string::npos &&
            spanish.translate(
                "Runtime.Prg.Dll.Error.NumericByReferenceArgumentRequired",
                numeric_byref_placeholders).find("declared numeric by reference") == std::string::npos,
        "#3944: es-419 numeric by-reference rejection should preserve the alias without English fallback");

    const std::string pseudo_handle =
        pseudo.translate("Runtime.Prg.Dll.Error.RegisteredApiHandleNotFound", handle_placeholders);
    expect(
        pseudo_handle.find("[!! ") == 0U &&
            pseudo_handle.find("42") != std::string::npos &&
            pseudo_handle.find("{handle}") == std::string::npos,
        "#2548: qps-ploc API handle error should pseudo-localize prose while preserving handle");
    const std::string pseudo_assembly =
        pseudo.translate("Runtime.Prg.Dll.Error.DotNetAssemblyLoadFailed", assembly_placeholders);
    expect(
        pseudo_assembly.find("[!! ") == 0U &&
            pseudo_assembly.find("bin/Interop.dll") != std::string::npos &&
            pseudo_assembly.find("-2147024894") != std::string::npos &&
            pseudo_assembly.find("{path}") == std::string::npos &&
            pseudo_assembly.find("{hresult}") == std::string::npos,
        "#2549: qps-ploc .NET assembly error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_function =
        pseudo.translate("Runtime.Prg.Dll.Error.NativeProcAddressMissing", function_placeholders);
    expect(
        pseudo_function.find("[!! ") == 0U &&
            pseudo_function.find("GetVersion") != std::string::npos &&
            pseudo_function.find("{functionName}") == std::string::npos,
        "#2550: qps-ploc native proc-address error should pseudo-localize prose while preserving function name");
    const std::string pseudo_limit =
        pseudo.translate("Runtime.Prg.Dll.Error.NativeArgumentLimitExceeded", native_limit_placeholders);
    expect(
        pseudo_limit.find("[!! ") == 0U &&
            pseudo_limit.find("9") != std::string::npos &&
            pseudo_limit.find("8") != std::string::npos &&
            pseudo_limit.find("{count}") == std::string::npos &&
            pseudo_limit.find("{maximum}") == std::string::npos,
        "#3895: qps-ploc native argument-limit error should preserve replaced counts");
    const std::string pseudo_invoke =
        pseudo.translate("Runtime.Prg.Dll.Error.NativeInvokeFailed", native_invoke_placeholders);
    expect(
        pseudo_invoke.find("[!! ") == 0U &&
            pseudo_invoke.find("pow") != std::string::npos &&
            pseudo_invoke.find("-2147352568") != std::string::npos &&
            pseudo_invoke.find("{functionName}") == std::string::npos &&
            pseudo_invoke.find("{hresult}") == std::string::npos,
        "#3895: qps-ploc native invocation error should preserve replaced identifiers");
    const std::string pseudo_numeric_byref = pseudo.translate(
        "Runtime.Prg.Dll.Error.NumericByReferenceArgumentRequired",
        numeric_byref_placeholders);
    expect(
        pseudo_numeric_byref.find("[!! ") == 0U &&
            pseudo_numeric_byref.find("NumericByRef") != std::string::npos &&
            pseudo_numeric_byref.find("4") != std::string::npos &&
            pseudo_numeric_byref.find("{functionName}") == std::string::npos &&
            pseudo_numeric_byref.find("{position}") == std::string::npos,
        "#3944: qps-ploc numeric by-reference rejection should preserve replaced alias and position");
    expect(
        pseudo.translate("Runtime.Prg.Dll.Error.TooFewArguments") ==
                copperfin::localization::pseudo_localize("Too few arguments.") &&
            pseudo.translate("Runtime.Prg.Dll.Error.TooManyArguments") ==
                copperfin::localization::pseudo_localize("Too many arguments."),
        "#3946: DECLARE arity errors should pseudo-localize without changing machine contracts");
}

void test_runtime_core_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap detail_placeholders{{"detail", "disk full"}};
    const copperfin::localization::PlaceholderMap path_placeholders{{"path", "forms/customer.scx"}};
    const copperfin::localization::PlaceholderMap guardrail_placeholders{{"limit", "37"}};

    expect(
        english.translate("Runtime.Prg.Core.Error.AsyncTaskCancelled") == "Async task cancelled.",
        "#2551: async task cancellation should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Core.Error.ResourceOutOfMemory") ==
            "Runtime resource fault: out of memory. Execution paused safely.",
        "#2551: out-of-memory resource fault should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Core.Error.ResourceFilesystemFailure", detail_placeholders) ==
            "Runtime resource fault: filesystem failure: disk full",
        "#2551: filesystem resource fault should preserve detail placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.ResourceSystemError", detail_placeholders) ==
            "Runtime resource fault: system error: disk full",
        "#2551: system resource fault should preserve detail placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.RuntimeFault", detail_placeholders) == "Runtime fault: disk full",
        "#2551: generic runtime fault should preserve detail placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.UnknownRuntimeFault") == "Runtime fault: unknown exception",
        "#2551: unknown runtime fault should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Core.Error.XAssetBootstrapMaterializeFailed", path_placeholders) ==
            "Unable to materialize xAsset bootstrap for: forms/customer.scx",
        "#2552: xAsset bootstrap error should preserve path placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.GuardrailCallDepthExceeded", guardrail_placeholders) ==
            "Runtime guardrail: maximum call depth (37) exceeded.",
        "#2720: call-depth guardrail fault should preserve the numeric limit placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.GuardrailExecutedStatementsExceeded", guardrail_placeholders) ==
            "Runtime guardrail: maximum executed statements (37) exceeded.",
        "#2720: executed-statements guardrail fault should preserve the numeric limit placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.GuardrailLoopIterationsExceeded", guardrail_placeholders) ==
            "Runtime guardrail: maximum loop iterations (37) exceeded.",
        "#2720: loop-iterations guardrail fault should preserve the numeric limit placeholder");
    expect(
        spanish.translate("Runtime.Prg.Core.Error.RuntimeFault", detail_placeholders).find("disk full") !=
                std::string::npos &&
            spanish.translate("Runtime.Prg.Core.Error.RuntimeFault", detail_placeholders).find("Runtime fault") ==
                std::string::npos,
        "#2551: es-419 runtime fault should preserve detail without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Core.Error.GuardrailExecutedStatementsExceeded", guardrail_placeholders)
                .find("maximum executed statements") == std::string::npos,
        "#2720: es-419 executed-statements guardrail fault should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Core.Error.GuardrailLoopIterationsExceeded", guardrail_placeholders)
                .find("maximum loop iterations") == std::string::npos,
        "#2720: pt-BR loop-iterations guardrail fault should not fall back to raw English");

    const std::string pseudo_fault =
        pseudo.translate("Runtime.Prg.Core.Error.ResourceFilesystemFailure", detail_placeholders);
    expect(
        pseudo_fault.find("[!! ") == 0U &&
            pseudo_fault.find("disk full") != std::string::npos &&
            pseudo_fault.find("{detail}") == std::string::npos,
        "#2551: qps-ploc runtime resource fault should pseudo-localize prose while preserving detail");
    const std::string pseudo_xasset =
        pseudo.translate("Runtime.Prg.Core.Error.XAssetBootstrapMaterializeFailed", path_placeholders);
    expect(
        pseudo_xasset.find("[!! ") == 0U &&
        pseudo_xasset.find("forms/customer.scx") != std::string::npos &&
            pseudo_xasset.find("{path}") == std::string::npos,
        "#2552: qps-ploc xAsset bootstrap error should pseudo-localize prose while preserving path");
    const std::string pseudo_guardrail =
        pseudo.translate("Runtime.Prg.Core.Error.GuardrailCallDepthExceeded", guardrail_placeholders);
    expect(
        pseudo_guardrail.find("[!! ") == 0U &&
            pseudo_guardrail.find("37") != std::string::npos &&
            pseudo_guardrail.find("maximum call depth") == std::string::npos,
        "#2720: qps-ploc call-depth guardrail fault should pseudo-localize prose while preserving the numeric limit");
}

void test_runtime_pause_and_session_messages_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap path_placeholders{{"path", "forms/customer.scx"}};

    expect(
        english.translate("Runtime.Prg.Session.Message.StoppedOnEntry") == "Stopped on entry.",
        "#2589: stopped-on-entry pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.WaitingInReadEvents") ==
            "The runtime is waiting in READ EVENTS.",
        "#2589: READ EVENTS pause message should preserve the invariant token in en-US");
    expect(
        english.translate("Runtime.Prg.Session.Message.ExecutionCompleted") == "Execution completed.",
        "#2589: execution-completed pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.BreakpointHit") == "Breakpoint hit.",
        "#2589: breakpoint-hit pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.StepCompleted") == "Step completed.",
        "#2589: step-completed pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.StepOverCompleted") == "Step-over completed.",
        "#2589: step-over-completed pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.StepOutCompleted") == "Step-out completed.",
        "#2589: step-out-completed pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Error.NoRunnableStartupMethodsFoundInAsset", path_placeholders) ==
            "No runnable startup methods were found in asset: forms/customer.scx",
        "#2589: no-runnable-startup message should preserve the asset path placeholder in en-US");

    const std::string spanish_waiting =
        spanish.translate("Runtime.Prg.Session.Message.WaitingInReadEvents");
    expect(
        spanish_waiting.find("READ EVENTS") != std::string::npos &&
            spanish_waiting.find("The runtime is waiting") == std::string::npos,
        "#2589: es-419 READ EVENTS pause message should preserve invariant tokens without falling back to English");

    const std::string portuguese_breakpoint =
        portuguese.translate("Runtime.Prg.Session.Message.BreakpointHit");
    expect(
        portuguese_breakpoint == "Um breakpoint foi atingido.",
        "#2589: pt-BR breakpoint-hit pause message should localize the prose");

    const std::string pseudo_step =
        pseudo.translate("Runtime.Prg.Session.Message.StepOverCompleted");
    expect(
        pseudo_step.find("[!! ") == 0U &&
            pseudo_step.find("Step-over completed.") == std::string::npos,
        "#2589: qps-ploc step-over pause message should pseudo-localize the prose");

    const std::string pseudo_no_runnable =
        pseudo.translate("Runtime.Prg.Session.Error.NoRunnableStartupMethodsFoundInAsset", path_placeholders);
    expect(
        pseudo_no_runnable.find("[!! ") == 0U &&
            pseudo_no_runnable.find("forms/customer.scx") != std::string::npos &&
            pseudo_no_runnable.find("{path}") == std::string::npos,
        "#2589: qps-ploc no-runnable-startup message should pseudo-localize prose while preserving the asset path");
}

void test_runtime_watch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Watch.Error.EmptyExpression") == "Watch expression is empty.",
        "#2590: empty watch-expression error should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Watch.Error.RequiresPausedFrame") ==
            "Watch evaluation requires a paused runtime frame.",
        "#2590: paused-frame watch error should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Watch.Error.OutOfMemory") ==
            "Watch evaluation ran out of memory.",
        "#2590: out-of-memory watch error should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Watch.Error.Failed") == "Watch evaluation failed.",
        "#2590: generic watch failure should preserve the en-US default output");

    const std::string spanish_empty = spanish.translate("Runtime.Prg.Watch.Error.EmptyExpression");
    expect(
        spanish_empty == "La expresion de watch esta vacia.",
        "#2590: es-419 empty watch-expression error should localize the prose");
    expect(
        spanish_empty.find("Watch expression is empty.") == std::string::npos,
        "#2590: es-419 empty watch-expression error should not fall back to raw English prose");

    const std::string portuguese_paused =
        portuguese.translate("Runtime.Prg.Watch.Error.RequiresPausedFrame");
    expect(
        portuguese_paused == "A avaliacao de watch exige um frame de runtime pausado.",
        "#2590: pt-BR paused-frame watch error should localize the prose");

    const std::string pseudo_failed = pseudo.translate("Runtime.Prg.Watch.Error.Failed");
    expect(
        pseudo_failed.find("[!! ") == 0U &&
            pseudo_failed.find("Watch evaluation failed.") == std::string::npos,
        "#2590: qps-ploc generic watch failure should pseudo-localize the prose");
}

void test_runtime_dispatch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap yield_placeholders{{"command", "YIELD"}};
    const copperfin::localization::PlaceholderMap go_placeholders{{"command", "GO"}};
    const copperfin::localization::PlaceholderMap replace_placeholders{{"command", "REPLACE"}};
    const copperfin::localization::PlaceholderMap update_placeholders{{"command", "UPDATE"}};
    const copperfin::localization::PlaceholderMap append_blank_placeholders{{"command", "APPEND BLANK"}};
    const copperfin::localization::PlaceholderMap delete_placeholders{{"command", "DELETE"}};
    const copperfin::localization::PlaceholderMap delete_from_placeholders{{"command", "DELETE FROM"}};
    const copperfin::localization::PlaceholderMap recall_placeholders{{"command", "RECALL"}};
    const copperfin::localization::PlaceholderMap insert_into_placeholders{{"command", "INSERT INTO"}};
    const copperfin::localization::PlaceholderMap pack_placeholders{{"command", "PACK"}};
    const copperfin::localization::PlaceholderMap zap_placeholders{{"command", "ZAP"}};
    const copperfin::localization::PlaceholderMap unlock_placeholders{{"command", "UNLOCK"}};
    const copperfin::localization::PlaceholderMap seek_placeholders{{"command", "SEEK"}};
    const copperfin::localization::PlaceholderMap skip_placeholders{{"command", "SKIP"}};
    const copperfin::localization::PlaceholderMap browse_placeholders{{"command", "BROWSE"}};
    const copperfin::localization::PlaceholderMap set_order_placeholders{{"command", "SET ORDER"}};
    const copperfin::localization::PlaceholderMap select_placeholders{{"command", "SELECT"}};
    const copperfin::localization::PlaceholderMap scan_placeholders{{"command", "SCAN"}};
    const copperfin::localization::PlaceholderMap do_target_placeholders{
        {"command", "DO"},
        {"target", "legacy/startup.prg"}
    };
    const copperfin::localization::PlaceholderMap call_target_placeholders{
        {"command", "CALL"},
        {"target", "NativeEntry"}
    };
    const copperfin::localization::PlaceholderMap spawn_placeholders{{"command", "SPAWN"}};
    const copperfin::localization::PlaceholderMap spawn_target_placeholders{
        {"command", "SPAWN"},
        {"target", "workers/process.prg"}
    };
    const copperfin::localization::PlaceholderMap await_placeholders{{"command", "AWAIT"}};
    const copperfin::localization::PlaceholderMap handle_placeholders{{"handle", "42"}};

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandDoesNotTakeArguments", yield_placeholders) ==
            "YIELD does not take arguments",
        "#2553: YIELD argument error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", go_placeholders) ==
            "GO target work area not found",
        "#2556: GO work-area error should localize through catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", skip_placeholders) ==
            "SKIP target work area not found",
        "#2556: SKIP work-area error should localize through catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", browse_placeholders) ==
            "BROWSE target work area not found",
        "#2556: BROWSE work-area error should localize through catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", set_order_placeholders) ==
            "SET ORDER target work area not found",
        "#2556: SET ORDER work-area error should localize through catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", select_placeholders) +
            ": MISSING_ALIAS" ==
            "SELECT target work area not found: MISSING_ALIAS",
        "#2556: SELECT work-area error should localize through catalog and preserve selection");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", seek_placeholders) ==
            "SEEK target work area not found",
        "#2555: SEEK work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", replace_placeholders) ==
            "REPLACE target work area not found",
        "#2557: REPLACE work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", update_placeholders) ==
            "UPDATE target work area not found",
        "#2557: UPDATE work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", append_blank_placeholders) ==
            "APPEND BLANK target work area not found",
        "#2557: APPEND BLANK work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", delete_placeholders) ==
            "DELETE target work area not found",
        "#2557: DELETE work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", delete_from_placeholders) ==
            "DELETE FROM target work area not found",
        "#2557: DELETE FROM work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", recall_placeholders) ==
            "RECALL target work area not found",
        "#2557: RECALL work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", insert_into_placeholders) ==
            "INSERT INTO target work area not found",
        "#2557: INSERT INTO work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", pack_placeholders) ==
            "PACK target work area not found",
        "#2557: PACK work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", zap_placeholders) ==
            "ZAP target work area not found",
        "#2557: ZAP work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", unlock_placeholders) ==
            "UNLOCK target work area not found",
        "#2557: UNLOCK work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", do_target_placeholders) ==
            "Unable to resolve DO target: legacy/startup.prg",
        "#2554: DO target resolution error should preserve command and target placeholders");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", call_target_placeholders) ==
            "Unable to resolve CALL target: NativeEntry",
        "#2554: CALL target resolution error should preserve command and target placeholders");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SpawnRequiresTarget", spawn_placeholders) ==
            "SPAWN requires a target routine or file",
        "#2553: SPAWN missing-target error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SpawnTargetResolveFailed", spawn_target_placeholders) ==
            "Unable to resolve SPAWN target: workers/process.prg",
        "#2553: SPAWN target resolution error should preserve command and target placeholders");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AwaitRequiresTaskHandle", await_placeholders) ==
            "AWAIT requires a task handle",
        "#2553: AWAIT missing-handle error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.UnknownTaskHandle", handle_placeholders) ==
            "Unknown task handle: 42",
        "#2553: unknown task-handle error should preserve handle placeholder");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.SpawnTargetResolveFailed", spawn_target_placeholders)
                    .find("workers/process.prg") != std::string::npos &&
            spanish.translate("Runtime.Prg.Dispatch.Error.SpawnTargetResolveFailed", spawn_target_placeholders)
                    .find("Unable to resolve") == std::string::npos,
        "#2553: es-419 SPAWN target error should preserve target without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", do_target_placeholders)
                    .find("legacy/startup.prg") != std::string::npos &&
            spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", do_target_placeholders)
                    .find("Unable to resolve") == std::string::npos,
        "#2554: es-419 DO target error should preserve target without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", seek_placeholders)
                    .find("SEEK") != std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", seek_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2555: es-419 SEEK work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", delete_placeholders).find("DELETE") !=
            std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", delete_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2557: es-419 DELETE work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", go_placeholders).find("GO") !=
            std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", go_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 GO work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", skip_placeholders).find("SKIP") !=
            std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", skip_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 SKIP work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", browse_placeholders).find("BROWSE") !=
            std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", browse_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 BROWSE work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", set_order_placeholders)
                    .find("SET ORDER") != std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", set_order_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 SET ORDER work-area error should preserve command without falling back to English");
    expect(
        (spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", select_placeholders) +
         ": MISSING_ALIAS")
                .find("MISSING_ALIAS") != std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", select_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 SELECT work-area error should preserve selection without falling back to English");

    const std::string pseudo_spawn =
        pseudo.translate("Runtime.Prg.Dispatch.Error.SpawnTargetResolveFailed", spawn_target_placeholders);
    expect(
        pseudo_spawn.find("[!! ") == 0U &&
            pseudo_spawn.find("SPAWN") != std::string::npos &&
            pseudo_spawn.find("workers/process.prg") != std::string::npos &&
            pseudo_spawn.find("{command}") == std::string::npos &&
            pseudo_spawn.find("{target}") == std::string::npos,
        "#2553: qps-ploc SPAWN target error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_do =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", do_target_placeholders);
    expect(
        pseudo_do.find("[!! ") == 0U &&
            pseudo_do.find("DO") != std::string::npos &&
            pseudo_do.find("legacy/startup.prg") != std::string::npos &&
            pseudo_do.find("{command}") == std::string::npos &&
            pseudo_do.find("{target}") == std::string::npos,
        "#2554: qps-ploc DO target error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_scan =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", scan_placeholders);
    expect(
        pseudo_scan.find("[!! ") == 0U &&
            pseudo_scan.find("SCAN") != std::string::npos &&
            pseudo_scan.find("{command}") == std::string::npos,
        "#2555: qps-ploc SCAN work-area error should pseudo-localize prose while preserving command");
    const std::string pseudo_go =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", go_placeholders);
    expect(
        pseudo_go.find("[!! ") == 0U &&
            pseudo_go.find("GO") != std::string::npos &&
            pseudo_go.find("{command}") == std::string::npos,
        "#2556: qps-ploc GO work-area error should pseudo-localize prose while preserving command");
    const std::string pseudo_replace =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", replace_placeholders);
    expect(
        pseudo_replace.find("[!! ") == 0U &&
            pseudo_replace.find("REPLACE") != std::string::npos &&
            pseudo_replace.find("target work area not found") == std::string::npos &&
            pseudo_replace.find("{command}") == std::string::npos,
        "#2557: qps-ploc REPLACE work-area error should pseudo-localize prose while preserving command");
    const std::string pseudo_select =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", select_placeholders) +
        ": MISSING_ALIAS";
    expect(
        pseudo_select.find("[!! ") == 0U &&
            pseudo_select.find("SELECT") != std::string::npos &&
            pseudo_select.find("MISSING_ALIAS") != std::string::npos &&
            pseudo_select.find("{command}") == std::string::npos,
        "#2556: qps-ploc SELECT work-area error should pseudo-localize prose while preserving selection");
}

void test_runtime_surface_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap bounds{
        {"maximum", "31"},
        {"minimum", "0"}
    };

    expect(
        english.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds) ==
            "Bit position must be between 0 and 31",
        "#2543: bit-position range error should preserve en-US default output");
    expect(
        spanish.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds).find("0") !=
                std::string::npos &&
            spanish.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds).find("31") !=
                std::string::npos &&
            spanish.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds).find("Bit position") ==
                std::string::npos,
        "#2543: es-419 bit-position range error should preserve bounds without falling back to English");

    const std::string pseudo_message =
        pseudo.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds);
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message.find("0") != std::string::npos &&
            pseudo_message.find("31") != std::string::npos &&
            pseudo_message.find("{minimum}") == std::string::npos,
        "#2543: qps-ploc bit-position range error should pseudo-localize prose while preserving bounds");

    const copperfin::localization::PlaceholderMap object_array_placeholders{
        {"capability", "object/array"},
        {"function", "AMEMBERS()"}
    };
    const copperfin::localization::PlaceholderMap function_placeholders{{"function", "GETPEM()"}};

    expect(
        english.translate(
            "Runtime.Prg.RuntimeSurface.Warning.StubCapabilityCallback",
            object_array_placeholders) == "AMEMBERS() uses stub behavior (no object/array callback)",
        "#2544: object/array runtime-surface warning should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback", function_placeholders) ==
            "GETPEM() uses stub behavior (no runtime object callback)",
        "#2544: runtime object callback warning should preserve en-US default output");
    expect(
        english.translate(
            "Runtime.Prg.RuntimeSurface.Warning.AmembersFallback",
            {{"function", "AMEMBERS()"}}) ==
            "AMEMBERS() fallback: unable to enumerate members, returning empty array",
        "#2544: AMEMBERS fallback warning should preserve en-US default output");
    expect(
        spanish.translate("Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback", function_placeholders)
                .find("GETPEM()") != std::string::npos &&
            spanish.translate("Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback", function_placeholders)
                    .find("uses stub behavior") == std::string::npos,
        "#2544: es-419 runtime object callback warning should preserve function name without falling back to English");

    const std::string pseudo_warning = pseudo.translate(
        "Runtime.Prg.RuntimeSurface.Warning.StubCapabilityCallback",
        object_array_placeholders);
    expect(
        pseudo_warning.find("[!! ") == 0U &&
            pseudo_warning.find("AMEMBERS()") != std::string::npos &&
            pseudo_warning.find("object/array") != std::string::npos &&
            pseudo_warning.find("{function}") == std::string::npos &&
            pseudo_warning.find("{capability}") == std::string::npos,
        "#2544: qps-ploc runtime-surface warning should pseudo-localize prose while preserving placeholders");

    const copperfin::localization::PlaceholderMap cursor_snapshot_placeholders{
        {"capability", "cursor snapshot"},
        {"function", "CURSORTOXML()"}
    };
    const copperfin::localization::PlaceholderMap xml_to_cursor_placeholders{{"function", "XMLTOCURSOR()"}};

    expect(
        english.translate("Runtime.Prg.RuntimeSurface.Warning.UnavailableCallback", cursor_snapshot_placeholders) ==
            "CURSORTOXML() unavailable (no cursor snapshot callback)",
        "#2545: cursor snapshot warning should preserve en-US default output");
    expect(
        english.translate(
            "Runtime.Prg.RuntimeSurface.Warning.CursorToXmlWriteFailed",
            {{"function", "CURSORTOXML()"}}) == "CURSORTOXML() failed to write target path",
        "#2545: CURSORTOXML write warning should preserve en-US default output");
    expect(
        english.translate(
            "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorInputAndAliasRequired",
            xml_to_cursor_placeholders) == "XMLTOCURSOR() requires XML input and destination alias",
        "#2545: XMLTOCURSOR input warning should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.RuntimeSurface.Warning.XmlToCursorParseFailed", xml_to_cursor_placeholders) ==
            "XMLTOCURSOR() could not parse the provided XML payload",
        "#2545: XMLTOCURSOR parse warning should preserve en-US default output");
    expect(
        spanish.translate("Runtime.Prg.RuntimeSurface.Warning.XmlToCursorParseFailed", xml_to_cursor_placeholders)
                .find("XMLTOCURSOR()") != std::string::npos &&
            spanish.translate("Runtime.Prg.RuntimeSurface.Warning.XmlToCursorParseFailed", xml_to_cursor_placeholders)
                    .find("could not parse") == std::string::npos,
        "#2545: es-419 XMLTOCURSOR warning should preserve function name without falling back to English");

    const std::string pseudo_cursor_warning =
        pseudo.translate("Runtime.Prg.RuntimeSurface.Warning.UnavailableCallback", cursor_snapshot_placeholders);
    expect(
        pseudo_cursor_warning.find("[!! ") == 0U &&
            pseudo_cursor_warning.find("CURSORTOXML()") != std::string::npos &&
            pseudo_cursor_warning.find("cursor snapshot") != std::string::npos &&
            pseudo_cursor_warning.find("{function}") == std::string::npos &&
            pseudo_cursor_warning.find("{capability}") == std::string::npos,
        "#2545: qps-ploc cursor XML warning should pseudo-localize prose while preserving placeholders");
}

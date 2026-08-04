// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "runtime_pipeline_support.h"

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

std::map<std::string, std::map<std::string, std::string>> build_generated_launcher_localized_messages() {
    static const std::vector<std::string_view> locales{
        "en-US",
        "es-419",
        "pt-BR",
        "qps-ploc"
    };
    static const std::vector<std::string_view> keys{
        "Runtime.Package.Launcher.Error.RuntimeHostMissing",
        "Runtime.Package.Launcher.Error.ManifestMissing",
        "Runtime.Package.Launcher.Error.RuntimeHostStartFailed"
    };

    const std::filesystem::path catalog_root = copperfin::localization::resolve_catalog_root();
    std::map<std::string, std::map<std::string, std::string>> localized_messages;
    for (const auto locale : locales) {
        const auto catalog = copperfin::localization::load_catalogs(catalog_root, locale);
        auto& locale_messages = localized_messages[std::string(locale)];
        for (const auto key : keys) {
            locale_messages[std::string(key)] = catalog.translate(key);
        }
    }
    return localized_messages;
}

void append_generated_launcher_localization_helpers(std::ostringstream& stream) {
    const auto localized_messages = build_generated_launcher_localized_messages();
    stream << "    private static readonly Dictionary<string, Dictionary<string, string>> LocalizedMessages =\n";
    stream << "        new(StringComparer.OrdinalIgnoreCase)\n";
    stream << "        {\n";
    for (const auto& [locale, messages] : localized_messages) {
        stream << "            [\"" << json_escape(locale) << "\"] = new(StringComparer.OrdinalIgnoreCase)\n";
        stream << "            {\n";
        for (const auto& [key, value] : messages) {
            stream << "                [\"" << json_escape(key) << "\"] = \"" << json_escape(value) << "\",\n";
        }
        stream << "            },\n";
    }
    stream << "        };\n\n";

    stream << "    private static string SelectLocale(string[] args)\n";
    stream << "    {\n";
    stream << "        for (var index = 0; index + 1 < args.Length; ++index)\n";
    stream << "        {\n";
    stream << "            if (string.Equals(args[index], \"--locale\", StringComparison.OrdinalIgnoreCase) ||\n";
    stream << "                string.Equals(args[index], \"/locale\", StringComparison.OrdinalIgnoreCase))\n";
    stream << "            {\n";
    stream << "                return NormalizeLocale(args[index + 1]);\n";
    stream << "            }\n";
    stream << "        }\n\n";
    stream << "        var configured = Environment.GetEnvironmentVariable(\"COPPERFIN_LOCALE\");\n";
    stream << "        if (!string.IsNullOrWhiteSpace(configured))\n";
    stream << "        {\n";
    stream << "            return NormalizeLocale(configured);\n";
    stream << "        }\n";
    stream << "        return \"en-US\";\n";
    stream << "    }\n\n";

    stream << "    private static string NormalizeLocale(string value)\n";
    stream << "    {\n";
    stream << "        if (string.IsNullOrWhiteSpace(value))\n";
    stream << "        {\n";
    stream << "            return \"en-US\";\n";
    stream << "        }\n\n";
    stream << "        var dotSuffix = value.IndexOf('.');\n";
    stream << "        var modifierSuffix = value.IndexOf('@');\n";
    stream << "        var suffix = dotSuffix >= 0 && modifierSuffix >= 0\n";
    stream << "            ? Math.Min(dotSuffix, modifierSuffix)\n";
    stream << "            : Math.Max(dotSuffix, modifierSuffix);\n";
    stream << "        if (suffix >= 0)\n";
    stream << "        {\n";
    stream << "            value = value.Substring(0, suffix).Trim();\n";
    stream << "        }\n\n";
    stream << "        var parts = value.Trim().Replace('_', '-').Split('-', StringSplitOptions.RemoveEmptyEntries);\n";
    stream << "        if (parts.Length == 0)\n";
    stream << "        {\n";
    stream << "            return \"en-US\";\n";
    stream << "        }\n\n";
    stream << "        for (var index = 0; index < parts.Length; ++index)\n";
    stream << "        {\n";
    stream << "            parts[index] = index == 0\n";
    stream << "                ? parts[index].ToLowerInvariant()\n";
    stream << "                : (string.Equals(parts[index], \"419\", StringComparison.Ordinal) ? \"419\" : parts[index].ToUpperInvariant());\n";
    stream << "        }\n";
    stream << "        return string.Join(\"-\", parts);\n";
    stream << "    }\n\n";

    stream << "    private static IEnumerable<string> LocaleFallbackChain(string locale)\n";
    stream << "    {\n";
    stream << "        var normalized = NormalizeLocale(locale);\n";
    stream << "        yield return normalized;\n";
    stream << "        if (normalized.StartsWith(\"es-\", StringComparison.OrdinalIgnoreCase) &&\n";
    stream << "            !string.Equals(normalized, \"es-419\", StringComparison.OrdinalIgnoreCase))\n";
    stream << "        {\n";
    stream << "            yield return \"es-419\";\n";
    stream << "        }\n\n";
    stream << "        var separator = normalized.IndexOf('-');\n";
    stream << "        if (separator >= 0)\n";
    stream << "        {\n";
    stream << "            yield return normalized.Substring(0, separator);\n";
    stream << "        }\n";
    stream << "        yield return \"en-US\";\n";
    stream << "    }\n\n";

    stream << "    private static string Translate(string key, string locale)\n";
    stream << "    {\n";
    stream << "        foreach (var candidate in LocaleFallbackChain(locale))\n";
    stream << "        {\n";
    stream << "            if (LocalizedMessages.TryGetValue(candidate, out var localeMessages) &&\n";
    stream << "                localeMessages.TryGetValue(key, out var value))\n";
    stream << "            {\n";
    stream << "                return value;\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        return key;\n";
    stream << "    }\n\n";
}

std::map<std::string, std::map<std::string, std::string>> build_generated_csharp_localized_messages() {
    static const std::vector<std::string_view> locales{
        "en-US",
        "es-419",
        "pt-BR",
        "qps-ploc"
    };
    static const std::vector<std::string_view> keys{
        "Runtime.Package.Transpilation.Error.UnsupportedFoxProStatement",
        "Runtime.Package.Transpilation.Error.ManualPortRequiredForXAssetMethod"
    };

    const std::filesystem::path catalog_root = copperfin::localization::resolve_catalog_root();
    std::map<std::string, std::map<std::string, std::string>> localized_messages;
    for (const auto locale : locales) {
        const auto catalog = copperfin::localization::load_catalogs(catalog_root, locale);
        auto& locale_messages = localized_messages[std::string(locale)];
        for (const auto key : keys) {
            locale_messages[std::string(key)] = catalog.translate(key);
        }
    }
    return localized_messages;
}

void append_generated_csharp_localization_helpers(std::ostringstream& stream) {
    const auto localized_messages = build_generated_csharp_localized_messages();
    stream << "    internal static class GeneratedLocalization\n";
    stream << "    {\n";
    stream << "        private static readonly Dictionary<string, Dictionary<string, string>> LocalizedMessages =\n";
    stream << "            new(StringComparer.OrdinalIgnoreCase)\n";
    stream << "            {\n";
    for (const auto& [locale, messages] : localized_messages) {
        stream << "                [\"" << json_escape(locale) << "\"] = new(StringComparer.OrdinalIgnoreCase)\n";
        stream << "                {\n";
        for (const auto& [key, value] : messages) {
            stream << "                    [\"" << json_escape(key) << "\"] = \"" << json_escape(value) << "\",\n";
        }
        stream << "                },\n";
    }
    stream << "            };\n\n";

    stream << "        internal static string Translate(string key, Dictionary<string, string>? placeholders = null)\n";
    stream << "        {\n";
    stream << "            var pattern = TranslatePattern(key);\n";
    stream << "            if (placeholders is null || placeholders.Count == 0)\n";
    stream << "            {\n";
    stream << "                return pattern;\n";
    stream << "            }\n\n";
    stream << "            foreach (var placeholder in placeholders)\n";
    stream << "            {\n";
    stream << "                pattern = pattern.Replace(\"{\" + placeholder.Key + \"}\", placeholder.Value, StringComparison.Ordinal);\n";
    stream << "            }\n";
    stream << "            return pattern;\n";
    stream << "        }\n\n";

    stream << "        private static string TranslatePattern(string key)\n";
    stream << "        {\n";
    stream << "            foreach (var locale in LocaleFallbackChain(SelectLocale()))\n";
    stream << "            {\n";
    stream << "                if (LocalizedMessages.TryGetValue(locale, out var localeMessages) &&\n";
    stream << "                    localeMessages.TryGetValue(key, out var value))\n";
    stream << "                {\n";
    stream << "                    return value;\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            return key;\n";
    stream << "        }\n\n";

    stream << "        private static string SelectLocale()\n";
    stream << "        {\n";
    stream << "            var args = Environment.GetCommandLineArgs();\n";
    stream << "            for (var index = 0; index + 1 < args.Length; ++index)\n";
    stream << "            {\n";
    stream << "                if (string.Equals(args[index], \"--locale\", StringComparison.OrdinalIgnoreCase) ||\n";
    stream << "                    string.Equals(args[index], \"/locale\", StringComparison.OrdinalIgnoreCase))\n";
    stream << "                {\n";
    stream << "                    return NormalizeLocale(args[index + 1]);\n";
    stream << "                }\n";
    stream << "            }\n\n";
    stream << "            var configured = Environment.GetEnvironmentVariable(\"COPPERFIN_LOCALE\");\n";
    stream << "            if (!string.IsNullOrWhiteSpace(configured))\n";
    stream << "            {\n";
    stream << "                return NormalizeLocale(configured);\n";
    stream << "            }\n";
    stream << "            return \"en-US\";\n";
    stream << "        }\n\n";

    stream << "        private static string NormalizeLocale(string value)\n";
    stream << "        {\n";
    stream << "            if (string.IsNullOrWhiteSpace(value))\n";
    stream << "            {\n";
    stream << "                return \"en-US\";\n";
    stream << "            }\n\n";
    stream << "            var parts = value.Trim().Replace('_', '-').Split('-', StringSplitOptions.RemoveEmptyEntries);\n";
    stream << "            if (parts.Length == 0)\n";
    stream << "            {\n";
    stream << "                return \"en-US\";\n";
    stream << "            }\n\n";
    stream << "            for (var index = 0; index < parts.Length; ++index)\n";
    stream << "            {\n";
    stream << "                parts[index] = index == 0\n";
    stream << "                    ? parts[index].ToLowerInvariant()\n";
    stream << "                    : (string.Equals(parts[index], \"419\", StringComparison.Ordinal) ? \"419\" : parts[index].ToUpperInvariant());\n";
    stream << "            }\n";
    stream << "            return string.Join(\"-\", parts);\n";
    stream << "        }\n\n";

    stream << "        private static IEnumerable<string> LocaleFallbackChain(string locale)\n";
    stream << "        {\n";
    stream << "            var normalized = NormalizeLocale(locale);\n";
    stream << "            yield return normalized;\n";
    stream << "            if (normalized.StartsWith(\"es-\", StringComparison.OrdinalIgnoreCase) &&\n";
    stream << "                !string.Equals(normalized, \"es-419\", StringComparison.OrdinalIgnoreCase))\n";
    stream << "            {\n";
    stream << "                yield return \"es-419\";\n";
    stream << "            }\n\n";
    stream << "            var separator = normalized.IndexOf('-');\n";
    stream << "            if (separator >= 0)\n";
    stream << "            {\n";
    stream << "                yield return normalized.Substring(0, separator);\n";
    stream << "            }\n";
    stream << "            yield return \"en-US\";\n";
    stream << "        }\n";
    stream << "    }\n\n";
}

std::string transpile_statement_to_csharp(
    const Statement& statement,
    const std::map<std::string, std::string>& routine_name_map) {
    switch (statement.kind) {
        case StatementKind::local_declaration: {
            std::ostringstream stream;
            for (const auto& name : statement.names) {
                stream << "dynamic " << sanitize_csharp_identifier(name, "localValue") << " = null;\n";
            }
            return stream.str();
        }
        case StatementKind::assignment:
            return statement.text + ";\n";
        case StatementKind::do_command: {
            const std::string routine_name = lowercase_copy(trim_copy(statement.identifier));
            const auto found = routine_name_map.find(routine_name);
            if (found != routine_name_map.end()) {
                return found->second + "();\n";
            }
            break;
        }
        case StatementKind::wait_command:
            if (!statement.expression.empty()) {
                return "Console.WriteLine(\"" + json_escape(unquote_literal(statement.expression)) + "\");\n";
            }
            break;
        case StatementKind::return_statement:
            return "return;\n";
        default:
            break;
    }

    return "throw new NotSupportedException(GeneratedLocalization.Translate(\"Runtime.Package.Transpilation.Error.UnsupportedFoxProStatement\", new Dictionary<string, string> { [\"statementText\"] = \"" + json_escape(statement.text) + "\" }));\n";
}

std::string build_xasset_csharp_method_identifier(
    const XAssetExecutableModel& model,
    const XAssetMethod& method) {
    const std::string normalized_root = lowercase_copy(trim_copy(model.root_object_path));
    const std::string normalized_object = lowercase_copy(trim_copy(method.object_path));
    if (!normalized_root.empty() && normalized_object == normalized_root) {
        return sanitize_csharp_compound_identifier(method.method_name, "Method");
    }

    std::string method_prefix = method.object_path;
    if (!model.root_object_path.empty() &&
        normalized_object.size() > normalized_root.size() &&
        normalized_object.rfind(normalized_root + ".", 0U) == 0U) {
        method_prefix = method.object_path.substr(model.root_object_path.size() + 1U);
    }
    if (!method_prefix.empty()) {
        method_prefix += ".";
    }
    method_prefix += method.method_name;
    return sanitize_csharp_compound_identifier(method_prefix, "Method");
}

void append_xasset_csharp_type(
    std::ostringstream& stream,
    const studio::StudioDocumentModel& document) {
    const XAssetExecutableModel model = build_xasset_executable_model(document);
    if (!model.ok || model.root_object_path.empty()) {
        return;
    }

    const std::string type_name = sanitize_csharp_compound_identifier(model.root_object_path, "XAssetObject");
    std::map<std::string, std::string> method_name_map;
    for (const auto& method : model.methods) {
        method_name_map.emplace(method.routine_name, build_xasset_csharp_method_identifier(model, method));
    }

    stream << "    public sealed class " << type_name << "\n";
    stream << "    {\n";

    for (const auto& method : model.methods) {
        const auto mapped_name = method_name_map.find(method.routine_name);
        if (mapped_name == method_name_map.end()) {
            continue;
        }

        const std::string method_identity = method.object_path.empty()
            ? method.method_name
            : method.object_path + "." + method.method_name;
        stream << "        public void " << mapped_name->second << "()\n";
        stream << "        {\n";
        stream << "            throw new NotSupportedException(GeneratedLocalization.Translate("
               << "\"Runtime.Package.Transpilation.Error.ManualPortRequiredForXAssetMethod\", "
               << "new Dictionary<string, string> { [\"methodIdentity\"] = \""
               << json_escape(method_identity)
               << "\" }));\n";
        stream << "        }\n\n";
    }

    if (!model.startup_routines.empty()) {
        stream << "        public void RunStartup()\n";
        stream << "        {\n";
        for (const auto& routine_name : model.startup_routines) {
            const auto found = method_name_map.find(routine_name);
            if (found != method_name_map.end()) {
                stream << "            " << found->second << "();\n";
            }
        }
        stream << "        }\n\n";
    }

    if (!model.shutdown_routines.empty()) {
        stream << "        public void RunShutdown()\n";
        stream << "        {\n";
        for (const auto& routine_name : model.shutdown_routines) {
            const auto found = method_name_map.find(routine_name);
            if (found != method_name_map.end()) {
                stream << "            " << found->second << "();\n";
            }
        }
        stream << "        }\n\n";
    }

    stream << "    }\n\n";
}

std::string build_csharp_transpilation_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "using System;\n";
    stream << "using System.Collections.Generic;\n\n";
    stream << "namespace Copperfin.Generated\n";
    stream << "{\n";
    append_generated_csharp_localization_helpers(stream);
    stream << "    public static class TranspiledProgram\n";
    stream << "    {\n";

    for (const auto& asset : plan.assets) {
        if (!should_stage_asset(asset) ||
            lowercase_copy(trim_copy(copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(asset.source_path).extension()))) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        std::map<std::string, std::string> routine_name_map;
        for (const auto& routine_entry : program.routines) {
            routine_name_map.emplace(lowercase_copy(routine_entry.first),
                                     sanitize_csharp_routine_identifier(routine_entry.first, "Routine"));
        }

        stream << "        public static void MainRoutine()\n";
        stream << "        {\n";
        for (const auto& statement : program.main.statements) {
            stream << "            " << transpile_statement_to_csharp(statement, routine_name_map);
        }
        stream << "        }\n\n";

        for (const auto& routine_entry : program.routines) {
            stream << "        public static void "
                   << sanitize_csharp_routine_identifier(routine_entry.first, "Routine")
                   << "()\n";
            stream << "        {\n";
            for (const auto& statement : routine_entry.second.statements) {
                stream << "            " << transpile_statement_to_csharp(statement, routine_name_map);
            }
            stream << "        }\n\n";
        }
    }

    stream << "    }\n\n";

    for (const auto& asset : plan.assets) {
        if (!should_stage_asset(asset)) {
            continue;
        }
        const std::string extension = lowercase_copy(trim_copy(
            copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(asset.source_path).extension())));
        if (extension != ".scx" && extension != ".vcx") {
            continue;
        }

        const auto open_result = studio::open_document({
            .path = asset.source_path,
            .load_full_table = true
        });
        if (!open_result.ok) {
            continue;
        }

        append_xasset_csharp_type(stream, open_result.document);
    }

    stream << "}\n";
    return stream.str();
}

std::string build_launcher_program_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const std::string runtime_host_name = copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(plan.runtime_host_destination_path).filename());
    stream << "using System;\n";
    stream << "using System.Collections.Generic;\n";
    stream << "using System.Diagnostics;\n";
    stream << "using System.IO;\n";
    stream << "using System.Text;\n\n";
    stream << "internal static class Program\n";
    stream << "{\n";
    stream << "    private static int Main(string[] args)\n";
    stream << "    {\n";
    stream << "        Console.OutputEncoding = Encoding.UTF8;\n";
    stream << "        var locale = SelectLocale(args);\n";
    stream << "        var baseDir = AppContext.BaseDirectory;\n";
    stream << "        var runtimeHost = Path.Combine(baseDir, \"" << runtime_host_name << "\");\n";
    stream << "        var manifest = Path.Combine(baseDir, \"app.cfmanifest\");\n";
    stream << "        var debugManifest = Path.Combine(baseDir, \"app.cfdebug\");\n";
    stream << "        if (!File.Exists(runtimeHost))\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(Translate(\"Runtime.Package.Launcher.Error.RuntimeHostMissing\", locale));\n";
    stream << "            return 3;\n";
    stream << "        }\n";
    stream << "        if (!File.Exists(manifest))\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(Translate(\"Runtime.Package.Launcher.Error.ManifestMissing\", locale));\n";
    stream << "            return 4;\n";
    stream << "        }\n\n";
    stream << "        var debugRequested = false;\n";
    stream << "        var forwarded = new List<string>();\n";
    stream << "        for (var index = 0; index < args.Length; ++index)\n";
    stream << "        {\n";
    stream << "            var arg = args[index];\n";
    stream << "            if (string.Equals(arg, \"--debug\", StringComparison.OrdinalIgnoreCase) ||\n";
    stream << "                string.Equals(arg, \"/debug\", StringComparison.OrdinalIgnoreCase))\n";
    stream << "            {\n";
    stream << "                debugRequested = true;\n";
    stream << "                forwarded.Add(\"--debug\");\n";
    stream << "                continue;\n";
    stream << "            }\n";
    stream << "            if (string.Equals(arg, \"/locale\", StringComparison.OrdinalIgnoreCase) &&\n";
    stream << "                (index + 1) < args.Length &&\n";
    stream << "                !args[index + 1].StartsWith(\"-\", StringComparison.Ordinal) &&\n";
    stream << "                !args[index + 1].StartsWith(\"/\", StringComparison.Ordinal))\n";
    stream << "            {\n";
    stream << "                forwarded.Add(\"--locale\");\n";
    stream << "                continue;\n";
    stream << "            }\n";
    stream << "            forwarded.Add(arg);\n";
    stream << "        }\n";
    stream << "        var selectedManifest = debugRequested && File.Exists(debugManifest) ? debugManifest : manifest;\n";
    stream << "        forwarded.Insert(0, selectedManifest);\n";
    stream << "        forwarded.Insert(0, \"--manifest\");\n\n";
    stream << "        var startInfo = new ProcessStartInfo\n";
    stream << "        {\n";
    stream << "            FileName = runtimeHost,\n";
    stream << "            WorkingDirectory = baseDir,\n";
    stream << "            UseShellExecute = false\n";
    stream << "        };\n\n";
    stream << "        foreach (var argument in forwarded)\n";
    stream << "        {\n";
    stream << "            startInfo.ArgumentList.Add(argument);\n";
    stream << "        }\n\n";
    stream << "        Process? process;\n";
    stream << "        try\n";
    stream << "        {\n";
    stream << "            process = Process.Start(startInfo);\n";
    stream << "        }\n";
    stream << "        catch (Exception)\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(Translate(\"Runtime.Package.Launcher.Error.RuntimeHostStartFailed\", locale));\n";
    stream << "            return 5;\n";
    stream << "        }\n";
    stream << "        if (process is null)\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(Translate(\"Runtime.Package.Launcher.Error.RuntimeHostStartFailed\", locale));\n";
    stream << "            return 5;\n";
    stream << "        }\n";
    stream << "        using (process)\n";
    stream << "        {\n";
    stream << "            process.WaitForExit();\n";
    stream << "            return process.ExitCode;\n";
    stream << "        }\n";
    stream << "    }\n\n";
    append_generated_launcher_localization_helpers(stream);
    stream << "}\n";
    return stream.str();
}

std::string build_launcher_project_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const std::string assembly_name =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.launcher_project_path).stem());
    stream << "<Project Sdk=\"Microsoft.NET.Sdk\">\n";
    stream << "  <PropertyGroup>\n";
    stream << "    <OutputType>Exe</OutputType>\n";
#if defined(_WIN32)
    stream << "    <TargetFramework>net8.0-windows</TargetFramework>\n";
#else
    stream << "    <TargetFramework>net8.0</TargetFramework>\n";
#endif
    stream << "    <ImplicitUsings>enable</ImplicitUsings>\n";
    stream << "    <Nullable>enable</Nullable>\n";
    stream << "    <UseWindowsForms>false</UseWindowsForms>\n";
    stream << "    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>\n";
    stream << "    <AssemblyName>" << assembly_name << "</AssemblyName>\n";
    stream << "    <RootNamespace>Copperfin.Generated</RootNamespace>\n";
    stream << "    <PublishSingleFile>false</PublishSingleFile>\n";
    stream << "  </PropertyGroup>\n";
    stream << "  <ItemGroup>\n";
    stream << "    <Compile Include=\"Program.cs\" />\n";
    stream << "  </ItemGroup>\n";
    stream << "</Project>\n";
    return stream.str();
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime

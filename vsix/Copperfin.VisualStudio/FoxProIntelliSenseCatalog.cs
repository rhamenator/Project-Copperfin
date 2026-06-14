using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace Copperfin.VisualStudio;

internal sealed class FoxProCompletionEntry
{
    public string DisplayText { get; set; } = string.Empty;
    public string InsertionText { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string Kind { get; set; } = string.Empty;
    public int Priority { get; set; }
}

internal sealed class FoxProParameterEntry
{
    public string Name { get; set; } = string.Empty;
    public string Documentation { get; set; } = string.Empty;
}

internal sealed class FoxProSignatureEntry
{
    public string Name { get; set; } = string.Empty;
    public string Content { get; set; } = string.Empty;
    public string Documentation { get; set; } = string.Empty;
    public IReadOnlyList<FoxProParameterEntry> Parameters { get; set; } = Array.Empty<FoxProParameterEntry>();
}

internal sealed class FoxProDefinitionLocation
{
    public string Name { get; set; } = string.Empty;
    public string Kind { get; set; } = string.Empty;
    public string FilePath { get; set; } = string.Empty;
    public int LineNumber { get; set; }
    public int ColumnNumber { get; set; }
    public string Description { get; set; } = string.Empty;
}

internal static class FoxProIntelliSenseCatalog
{
    private static readonly string[] TextExtensions = { ".prg", ".h", ".hpp", ".ch", ".qpr", ".mpr", ".spr" };
    private static readonly string[] FormExtensions = { ".scx" };
    private static readonly string[] ReportExtensions = { ".frx" };
    private static readonly string[] LabelExtensions = { ".lbx" };
    private static readonly string[] MenuExtensions = { ".mnx" };
    private static readonly string[] TableExtensions = { ".dbf", ".dbc" };
    private static readonly string[] IgnoredDirectories = { ".git", ".vs", "bin", "obj", "build", "packages", "node_modules" };

    private static readonly Regex ProcedureRegex = new(@"^\s*PROCEDURE\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex FunctionRegex = new(@"^\s*FUNCTION\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex DefineClassRegex = new(@"^\s*DEFINE\s+CLASS\s+([A-Za-z0-9_\.]+)\s+AS\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex EndDefineRegex = new(@"^\s*ENDDEFINE\b", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex DefineRegex = new(@"^\s*#DEFINE\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex IncludeRegex = new(@"^\s*#INCLUDE\s+[\""<]([^\"">]+)[\"">]", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex UseAliasRegex = new(@"^\s*USE\s+.+?\s+ALIAS\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex UseStatementRegex = new(@"^\s*USE\s+(""[^""]+""|'[^']+'|[^\s]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex CreateCursorRegex = new(@"^\s*CREATE\s+CURSOR\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex IntoCursorRegex = new(@"\bINTO\s+CURSOR\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex SqlExecInvocationRegex = new(@"\bSQLEXEC\s*\(", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex ParametersRegex = new(@"^\s*(?:L?PARAMETERS)\s+(.+)$", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex MemberAccessRegex = new(@"([A-Za-z_][A-Za-z0-9_]*)\.$", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex AliasIdentifierRegex = new(@"^[A-Za-z_][A-Za-z0-9_\.]*$", RegexOptions.Compiled);

    private static readonly (string Name, string Description)[] Keywords =
    {
        ("SELECT", "Command: selects a work area or evaluates a SELECT() call depending on context."),
        ("USE", "Command: opens a table, free table, cursor, or closes one with USE IN."),
        ("IN", "Keyword: targets a work area or alias in commands like USE IN and GO ... IN."),
        ("ALIAS", "Keyword: assigns or references a work-area alias."),
        ("DO", "Command: runs a procedure, program, or form."),
        ("DO FORM", "Command: runs a form and enters its event lifecycle."),
        ("REPORT FORM", "Command: runs a report, often with PREVIEW."),
        ("LABEL FORM", "Command: runs a label layout."),
        ("SET", "Command: updates runtime state such as SAFETY, DELETED, EXCLUSIVE, and TALK."),
        ("SET DATASESSION TO", "Command: switches into a specific data session."),
        ("READ EVENTS", "Command: enters the event loop."),
        ("CLEAR EVENTS", "Command: exits the event loop."),
        ("PUBLIC", "Command: declares global variables."),
        ("LOCAL", "Command: declares local variables."),
        ("RETURN", "Command: returns from a procedure or function."),
        ("DEFINE CLASS", "Command: defines a class in source."),
        ("ENDDEFINE", "Command: ends a class definition."),
        ("IF", "Keyword: starts a conditional block."),
        ("ELSE", "Keyword: alternate branch within IF."),
        ("ENDIF", "Keyword: ends a conditional block."),
        ("FOR", "Keyword: starts a numeric loop."),
        ("ENDFOR", "Keyword: ends a numeric loop."),
        ("ON ERROR", "Command: installs an error handler."),
        ("CREATEOBJECT", "Function-style activation of a COM or Copperfin automation object."),
        ("GETOBJECT", "Function-style bind to an existing COM or moniker-based automation object."),
        ("SQLCONNECT", "Function: opens a SQL pass-through connection."),
        ("SQLSTRINGCONNECT", "Function: opens SQL pass-through with a raw connection string."),
        ("SQLEXEC", "Function: executes SQL pass-through and optionally materializes a cursor alias."),
        ("SQLDISCONNECT", "Function: closes a SQL pass-through connection.")
    };

    private static readonly (string Name, string Description)[] Functions =
    {
        ("ALIAS()", "Returns the current alias, or a named work area's alias when passed an argument."),
        ("SELECT()", "Returns the current work area, or resolves/selects a requested work area."),
        ("RECCOUNT()", "Returns the number of records in the current or named work area."),
        ("RECNO()", "Returns the current record number in the current or named work area."),
        ("EOF()", "Returns .T. when positioned after the last record."),
        ("BOF()", "Returns .T. when positioned before the first record."),
        ("ALLTRIM()", "Trims leading and trailing spaces."),
        ("JUSTPATH()", "Returns the directory portion of a path."),
        ("MESSAGE()", "Returns the current error message text."),
        ("MESSAGEBOX()", "Displays a dialog and returns the button choice."),
        ("SYS()", "Accesses VFP system services and metadata."),
        ("STR()", "Converts a number to a string."),
        ("DATE()", "Returns the current date."),
        ("DAY()", "Returns the day part of a date."),
        ("MONTH()", "Returns the month part of a date."),
        ("YEAR()", "Returns the year part of a date."),
        ("CMONTH()", "Returns the month name for a date."),
        ("IIF()", "Evaluates one of two expressions based on a condition."),
        ("FILE()", "Checks whether a file exists."),
        ("CURSORGETPROP()", "Returns metadata for an open cursor or remote cursor adapter."),
        ("CREATEOBJECT()", "Creates a COM or Copperfin automation object."),
        ("GETOBJECT()", "Binds to a running automation object or moniker.")
    };

    private static readonly (string Name, string Description)[] SetKeywords =
    {
        ("DELETED", "SET option controlling deleted-record visibility."),
        ("SAFETY", "SET option controlling overwrite prompts."),
        ("EXCLUSIVE", "SET option controlling shared/exclusive opens."),
        ("TALK", "SET option controlling command chatter."),
        ("EXACT", "SET option controlling exact string matching."),
        ("MULTILOCKS", "SET option controlling optimistic locking behavior."),
        ("PATH", "SET option controlling search paths."),
        ("DEFAULT TO", "SET subcommand for the default directory."),
        ("DATASESSION TO", "SET subcommand for the current data session.")
    };

    private static readonly (string Name, string Description)[] GenericObjectMembers =
    {
        ("Visible", "Common automation property controlling object visibility."),
        ("Caption", "Common automation property for title text."),
        ("Value", "Common automation property for current value."),
        ("Count", "Common automation property for collection size."),
        ("Item()", "Common automation method for retrieving a child item."),
        ("Add()", "Common automation method for creating or appending a child object."),
        ("Open()", "Common automation method for opening a child object or document."),
        ("Close()", "Common automation method for closing an object or document."),
        ("Execute()", "Common automation method for command-style invocation."),
        ("Refresh()", "Common automation method for refreshing state."),
        ("Quit()", "Common automation method for shutting down an automation server."),
        ("Workbooks", "Common Excel automation collection."),
        ("ActiveWorkbook", "Common Excel automation property."),
        ("Documents", "Common Office automation collection.")
    };

    private static readonly FoxProSignatureEntry[] SignatureEntries =
    {
        CreateSignature("ALIAS", "ALIAS([nWorkArea | cAlias])", "Returns the alias for the current or specified work area.", ("nWorkArea | cAlias", "Optional work area number or alias to resolve.")),
        CreateSignature("SELECT", "SELECT([nWorkArea | cAlias])", "Returns or resolves a work area by number or alias.", ("nWorkArea | cAlias", "Optional work area number or alias.")),
        CreateSignature("RECCOUNT", "RECCOUNT([cAlias])", "Returns the number of records in the current or named work area.", ("cAlias", "Optional alias to inspect.")),
        CreateSignature("RECNO", "RECNO([cAlias])", "Returns the current record number in the current or named work area.", ("cAlias", "Optional alias to inspect.")),
        CreateSignature("EOF", "EOF([cAlias])", "Returns .T. when positioned after the last record.", ("cAlias", "Optional alias to inspect.")),
        CreateSignature("BOF", "BOF([cAlias])", "Returns .T. when positioned before the first record.", ("cAlias", "Optional alias to inspect.")),
        CreateSignature("CREATEOBJECT", "CREATEOBJECT(cClass [, eInitParameter1 [, eInitParameterN]])", "Creates a COM or Copperfin automation object.", ("cClass", "ProgID or class name to instantiate."), ("eInitParameter1", "Optional constructor-style argument."), ("eInitParameterN", "Additional optional constructor-style arguments.")),
        CreateSignature("GETOBJECT", "GETOBJECT([cFileName] [, cClass])", "Binds to an existing automation object or document moniker.", ("cFileName", "Optional document path or moniker."), ("cClass", "Optional class or ProgID filter.")),
        CreateSignature("SQLCONNECT", "SQLCONNECT(cDataSourceName [, cUserId [, cPassword [, lShared]]])", "Opens an ODBC or connection-manager session for SQL pass-through.", ("cDataSourceName", "DSN or connection identifier."), ("cUserId", "Optional user name."), ("cPassword", "Optional password."), ("lShared", "Optional shared-connection flag.")),
        CreateSignature("SQLSTRINGCONNECT", "SQLSTRINGCONNECT(cConnectString [, lShared])", "Opens SQL pass-through with a raw connection string.", ("cConnectString", "Raw ODBC-style connection string."), ("lShared", "Optional shared-connection flag.")),
        CreateSignature("SQLEXEC", "SQLEXEC(nConnectionHandle, cCommand [, cCursorName])", "Executes SQL pass-through and optionally materializes a result cursor.", ("nConnectionHandle", "Connection handle returned by SQLCONNECT or SQLSTRINGCONNECT."), ("cCommand", "SQL text to execute."), ("cCursorName", "Optional target cursor alias.")),
        CreateSignature("SQLDISCONNECT", "SQLDISCONNECT([nConnectionHandle])", "Closes one SQL pass-through connection or all of them.", ("nConnectionHandle", "Optional connection handle to close.")),
        CreateSignature("CURSORGETPROP", "CURSORGETPROP(cProperty [, cCursorName])", "Returns metadata for a cursor or remote view.", ("cProperty", "Property name to query."), ("cCursorName", "Optional cursor alias.")),
        CreateSignature("MESSAGEBOX", "MESSAGEBOX(cMessage [, nDialogBoxType [, cTitleBarText]])", "Displays a modal dialog and returns the pressed button.", ("cMessage", "Message text to display."), ("nDialogBoxType", "Optional button/icon/style flags."), ("cTitleBarText", "Optional dialog title.")),
        CreateSignature("SYS", "SYS(nFunction [, eExpression1 [, eExpressionN]])", "Calls a Visual FoxPro system service by numeric identifier.", ("nFunction", "System function number."), ("eExpression1", "Optional first argument."), ("eExpressionN", "Additional optional arguments.")),
        CreateSignature("IIF", "IIF(lExpression, eTrueValue, eFalseValue)", "Returns one of two expressions based on a condition.", ("lExpression", "Condition to evaluate."), ("eTrueValue", "Value when the condition is true."), ("eFalseValue", "Value when the condition is false."))
    };

    private static readonly Dictionary<string, IReadOnlyList<FoxProSignatureEntry>> SignatureLookup =
        SignatureEntries.GroupBy(entry => NormalizeLookupToken(entry.Name), StringComparer.OrdinalIgnoreCase)
            .ToDictionary(group => group.Key, group => (IReadOnlyList<FoxProSignatureEntry>)group.ToList(), StringComparer.OrdinalIgnoreCase);

    private static readonly ConcurrentDictionary<string, ProjectSymbolIndex> Cache = new(StringComparer.OrdinalIgnoreCase);

    public static IReadOnlyList<FoxProCompletionEntry> BuildEntries(string? filePath, string linePrefix, string tokenPrefix)
    {
        var prefix = tokenPrefix ?? string.Empty;
        var completions = new Dictionary<string, FoxProCompletionEntry>(StringComparer.OrdinalIgnoreCase);
        AddEntries(completions, Keywords, "keyword", priority: 200);
        AddEntries(completions, Functions, "function", priority: 180);

        if (LooksLikeSetContext(linePrefix))
        {
            AddEntries(completions, SetKeywords, "set", priority: 20);
        }

        if (!string.IsNullOrWhiteSpace(filePath))
        {
            var index = GetProjectIndex(filePath!);
            if (LooksLikeMemberAccess(linePrefix))
            {
                AddProjectMemberEntries(completions, index);
            }
            AddContextualProjectEntries(completions, index, linePrefix);
            AddSymbolEntries(completions, index);
        }

        if (LooksLikeMemberAccess(linePrefix))
        {
            AddEntries(completions, GenericObjectMembers, "member", priority: 20);
        }

        return completions.Values
            .Where(entry => string.IsNullOrWhiteSpace(prefix) || entry.DisplayText.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
            .OrderBy(entry => entry.Priority)
            .ThenBy(entry => RankKind(entry.Kind))
            .ThenBy(entry => entry.DisplayText, StringComparer.OrdinalIgnoreCase)
            .ToList();
    }

    public static string DescribeToken(string token)
    {
        return DescribeToken(null, token);
    }

    public static string DescribeToken(string? filePath, string token)
    {
        if (string.IsNullOrWhiteSpace(token))
        {
            return string.Empty;
        }

        var key = NormalizeLookupToken(token);
        foreach (var item in Keywords)
        {
            if (TokenMatches(item.Name, key))
            {
                return item.Description;
            }
        }

        foreach (var item in Functions)
        {
            if (TokenMatches(item.Name, key))
            {
                return item.Description;
            }
        }

        foreach (var item in SetKeywords)
        {
            if (TokenMatches(item.Name, key))
            {
                return item.Description;
            }
        }

        foreach (var item in GenericObjectMembers)
        {
            if (TokenMatches(item.Name, key))
            {
                return item.Description;
            }
        }

        if (!string.IsNullOrWhiteSpace(filePath))
        {
            var index = GetProjectIndex(filePath!);
            if (TryResolveDefinitionCandidate(index, key, out var definition))
            {
                return definition.Description;
            }

            if (index.Procedures.Contains(key))
            {
                return "Project procedure or function symbol.";
            }

            if (index.Classes.Contains(key))
            {
                return "Project class symbol.";
            }

            if (index.Defines.Contains(key))
            {
                return "Project preprocessor symbol.";
            }

            if (index.Aliases.Contains(key))
            {
                return "Known work-area alias discovered in project source.";
            }

            if (index.Tables.Contains(key))
            {
                return "Project table or database asset.";
            }

            if (index.Forms.Contains(key))
            {
                return "Project form asset.";
            }

            if (index.Reports.Contains(key))
            {
                return "Project report asset.";
            }

            if (index.Labels.Contains(key))
            {
                return "Project label asset.";
            }

            if (index.Menus.Contains(key))
            {
                return "Project menu asset.";
            }
        }

        return string.Empty;
    }

    public static IReadOnlyList<FoxProSignatureEntry> GetSignatures(string? filePath, string invocationName)
    {
        var key = NormalizeLookupToken(invocationName);
        if (SignatureLookup.TryGetValue(key, out var signatures))
        {
            return signatures;
        }

        if (invocationName.IndexOf('.') >= 0)
        {
            var memberName = invocationName.Split('.').Last();
            if (SignatureLookup.TryGetValue(NormalizeLookupToken(memberName), out signatures))
            {
                return signatures;
            }
        }

        if (!string.IsNullOrWhiteSpace(filePath))
        {
            var index = GetProjectIndex(filePath!);
            var projectSignatures = TryResolveProjectSignatures(index, key);
            if (projectSignatures.Count != 0)
            {
                return projectSignatures;
            }

            if (index.Classes.Contains(key) || index.Procedures.Contains(key))
            {
                return Array.Empty<FoxProSignatureEntry>();
            }
        }

        return Array.Empty<FoxProSignatureEntry>();
    }

    public static bool TryResolveDefinition(string? filePath, string token, out FoxProDefinitionLocation definition)
    {
        definition = new FoxProDefinitionLocation();
        if (string.IsNullOrWhiteSpace(filePath) || string.IsNullOrWhiteSpace(token))
        {
            return false;
        }

        var index = GetProjectIndex(filePath!);
        return TryResolveDefinitionCandidate(index, NormalizeLookupToken(token), out definition);
    }

    private static void AddContextualProjectEntries(
        IDictionary<string, FoxProCompletionEntry> completions,
        ProjectSymbolIndex index,
        string linePrefix)
    {
        var upper = linePrefix.TrimStart().ToUpperInvariant();
        if (upper.StartsWith("DO FORM ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Forms.Select(name => (name, "Project form asset.")), "asset", priority: 0);
            return;
        }

        if (upper.StartsWith("REPORT FORM ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Reports.Select(name => (name, "Project report asset.")), "asset", priority: 0);
            return;
        }

        if (upper.StartsWith("LABEL FORM ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Labels.Select(name => (name, "Project label asset.")), "asset", priority: 0);
            return;
        }

        if (upper.StartsWith("USE ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Tables.Select(name => (name, "Project table or database asset.")), "asset", priority: 0);
            AddEntries(completions, index.Aliases.Select(name => (name, "Known alias from project source.")), "alias", priority: 0);
            return;
        }

        if (upper.StartsWith("SELECT ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Aliases.Select(name => (name, "Known work-area alias from project source.")), "alias", priority: 0);
            AddEntries(completions, index.Tables.Select(name => (name, "Project table or database asset.")), "asset", priority: 0);
            return;
        }

        if (upper.StartsWith("DO ", StringComparison.Ordinal) && !upper.StartsWith("DO FORM ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Procedures.Select(name => (name, "Procedure/function/program symbol in the active project.")), "symbol", priority: 0);
            AddEntries(completions, index.Menus.Select(name => (name, "Project menu asset.")), "asset", priority: 0);
        }
    }

    private static void AddSymbolEntries(IDictionary<string, FoxProCompletionEntry> completions, ProjectSymbolIndex index)
    {
        AddEntries(completions, index.Procedures.Select(name => (name, "Procedure/function symbol in the active project.")), "symbol", priority: 100);
        AddEntries(completions, index.Classes.Select(name => (name, "Class symbol in the active project.")), "class", priority: 100);
        AddEntries(completions, index.Defines.Select(name => (name, "Preprocessor symbol in the active project.")), "define", priority: 100);
        AddEntries(completions, index.Aliases.Select(name => (name, "Known alias from USE ... ALIAS statements.")), "alias", priority: 100);
    }

    private static void AddProjectMemberEntries(IDictionary<string, FoxProCompletionEntry> completions, ProjectSymbolIndex index)
    {
        AddEntries(
            completions,
            index.Methods.Select(method => (ExtractMethodName(method), $"Project method member from {ExtractContainingType(method)}.")),
            "member",
            priority: 0);
    }

    private static bool LooksLikeSetContext(string linePrefix)
    {
        return linePrefix.TrimStart().StartsWith("SET ", StringComparison.OrdinalIgnoreCase);
    }

    private static bool LooksLikeMemberAccess(string linePrefix)
    {
        return MemberAccessRegex.IsMatch(linePrefix);
    }

    private static string ExtractMethodName(string qualifiedMethodName)
    {
        var separator = qualifiedMethodName.LastIndexOf('.');
        return separator >= 0 ? qualifiedMethodName[(separator + 1)..] : qualifiedMethodName;
    }

    private static string ExtractContainingType(string qualifiedMethodName)
    {
        var separator = qualifiedMethodName.LastIndexOf('.');
        return separator >= 0 ? qualifiedMethodName[..separator] : "the active project";
    }

    private static void AddEntries(
        IDictionary<string, FoxProCompletionEntry> completions,
        IEnumerable<(string Name, string Description)> source,
        string kind,
        int priority)
    {
        foreach (var item in source)
        {
            if (string.IsNullOrWhiteSpace(item.Name))
            {
                continue;
            }

            var candidate = new FoxProCompletionEntry
            {
                DisplayText = item.Name,
                InsertionText = item.Name,
                Description = item.Description,
                Kind = kind,
                Priority = priority
            };

            if (completions.TryGetValue(item.Name, out var existing) && existing.Priority <= candidate.Priority)
            {
                continue;
            }

            completions[item.Name] = candidate;
        }
    }

    private static ProjectSymbolIndex GetProjectIndex(string filePath)
    {
        var root = ResolveProjectRoot(filePath);
        return Cache.AddOrUpdate(
            root,
            _ => BuildProjectIndex(root),
            (_, existing) => existing.ShouldRefresh ? BuildProjectIndex(root) : existing);
    }

    private static ProjectSymbolIndex BuildProjectIndex(string root)
    {
        var index = new ProjectSymbolIndex
        {
            Root = root,
            BuiltAtUtc = DateTime.UtcNow
        };
        var scannedFiles = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

        foreach (var file in EnumerateProjectFiles(root))
        {
            var extension = Path.GetExtension(file);
            if (FormExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                AddAsset(index.Forms, index, Path.GetFileNameWithoutExtension(file), "form asset", file, "Project form asset.");
                continue;
            }
            if (ReportExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                AddAsset(index.Reports, index, Path.GetFileNameWithoutExtension(file), "report asset", file, "Project report asset.");
                continue;
            }
            if (LabelExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                AddAsset(index.Labels, index, Path.GetFileNameWithoutExtension(file), "label asset", file, "Project label asset.");
                continue;
            }
            if (MenuExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                AddAsset(index.Menus, index, Path.GetFileNameWithoutExtension(file), "menu asset", file, "Project menu asset.");
                continue;
            }
            if (TableExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                AddAsset(index.Tables, index, Path.GetFileNameWithoutExtension(file), "table asset", file, "Project table or database asset.");
                continue;
            }
            if (!TextExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                continue;
            }

            ScanTextFile(file, root, index, scannedFiles);
        }

        return index;
    }

    private static void ScanTextFile(string path, string root, ProjectSymbolIndex index, ISet<string> scannedFiles)
    {
        string normalizedPath;
        try
        {
            normalizedPath = Path.GetFullPath(path);
        }
        catch
        {
            return;
        }

        if (!scannedFiles.Add(normalizedPath))
        {
            return;
        }

        string[] lines;
        try
        {
            lines = File.ReadAllLines(normalizedPath);
        }
        catch
        {
            return;
        }

        string? currentClassName = null;
        for (var lineIndex = 0; lineIndex < lines.Length; lineIndex++)
        {
            var line = lines[lineIndex];

            var classMatch = DefineClassRegex.Match(line);
            if (classMatch.Success)
            {
                currentClassName = classMatch.Groups[1].Value;
                index.Classes.Add(currentClassName);
                TryAddDefinition(index.Definitions, currentClassName, "class", normalizedPath, lineIndex + 1, classMatch.Groups[1].Index + 1, $"Project class symbol deriving from {classMatch.Groups[2].Value}.");
                continue;
            }

            if (currentClassName is not null)
            {
                if (EndDefineRegex.IsMatch(line))
                {
                    currentClassName = null;
                    continue;
                }

                var methodProcedureMatch = ProcedureRegex.Match(line);
                if (methodProcedureMatch.Success)
                {
                    var methodName = $"{currentClassName}.{methodProcedureMatch.Groups[1].Value}";
                    index.Methods.Add(methodName);
                    TryAddDefinition(index.Definitions, methodName, "method", normalizedPath, lineIndex + 1, methodProcedureMatch.Groups[1].Index + 1, $"Project method symbol on class {currentClassName}.");
                    TryAddProjectSignature(index.Signatures, methodName, lines, lineIndex, "Project method signature discovered in source.");
                    continue;
                }

                var methodFunctionMatch = FunctionRegex.Match(line);
                if (methodFunctionMatch.Success)
                {
                    var methodName = $"{currentClassName}.{methodFunctionMatch.Groups[1].Value}";
                    index.Methods.Add(methodName);
                    TryAddDefinition(index.Definitions, methodName, "method", normalizedPath, lineIndex + 1, methodFunctionMatch.Groups[1].Index + 1, $"Project method symbol on class {currentClassName}.");
                    TryAddProjectSignature(index.Signatures, methodName, lines, lineIndex, "Project method signature discovered in source.");
                    continue;
                }
            }

            var procedureMatch = ProcedureRegex.Match(line);
            if (procedureMatch.Success)
            {
                var name = procedureMatch.Groups[1].Value;
                index.Procedures.Add(name);
                TryAddDefinition(index.Definitions, name, "procedure", normalizedPath, lineIndex + 1, procedureMatch.Groups[1].Index + 1, "Project procedure symbol.");
                TryAddProjectSignature(index.Signatures, name, lines, lineIndex, "Project procedure signature discovered in source.");
            }

            var functionMatch = FunctionRegex.Match(line);
            if (functionMatch.Success)
            {
                var name = functionMatch.Groups[1].Value;
                index.Procedures.Add(name);
                TryAddDefinition(index.Definitions, name, "function", normalizedPath, lineIndex + 1, functionMatch.Groups[1].Index + 1, "Project function symbol.");
                TryAddProjectSignature(index.Signatures, name, lines, lineIndex, "Project function signature discovered in source.");
            }

            AddMatch(index.Defines, index.Definitions, DefineRegex, line, normalizedPath, lineIndex + 1, "define", "Project preprocessor symbol.");
            AddMatch(index.Aliases, index.Definitions, UseAliasRegex, line, normalizedPath, lineIndex + 1, "alias", "Known work-area alias discovered in project source.");
            TryAddImplicitUseAlias(index, line, normalizedPath, lineIndex + 1);
            AddMatch(index.Aliases, index.Definitions, CreateCursorRegex, line, normalizedPath, lineIndex + 1, "alias", "Known cursor alias discovered in project source.");
            AddMatch(index.Aliases, index.Definitions, IntoCursorRegex, line, normalizedPath, lineIndex + 1, "alias", "Known cursor alias discovered in project source.");
            TryAddSqlExecCursorAlias(index, line, normalizedPath, lineIndex + 1);

            var includeMatch = IncludeRegex.Match(line);
            if (includeMatch.Success)
            {
                var includePath = ResolveIncludePath(normalizedPath, root, includeMatch.Groups[1].Value);
                if (!string.IsNullOrWhiteSpace(includePath) &&
                    TextExtensions.Contains(Path.GetExtension(includePath), StringComparer.OrdinalIgnoreCase))
                {
                    ScanTextFile(includePath, root, index, scannedFiles);
                }
            }
        }
    }

    private static void TryAddImplicitUseAlias(ProjectSymbolIndex index, string line, string path, int lineNumber)
    {
        if (UseAliasRegex.IsMatch(line))
        {
            return;
        }

        var match = UseStatementRegex.Match(line);
        if (!match.Success)
        {
            return;
        }

        if (!TryInferAliasFromUseOperand(match.Groups[1].Value, out var alias))
        {
            return;
        }

        index.Aliases.Add(alias);
        TryAddDefinition(
            index.Definitions,
            alias,
            "alias",
            path,
            lineNumber,
            match.Groups[1].Index + 1,
            "Known work-area alias discovered in project source.");
    }

    private static void TryAddSqlExecCursorAlias(ProjectSymbolIndex index, string line, string path, int lineNumber)
    {
        var match = SqlExecInvocationRegex.Match(line);
        if (!match.Success)
        {
            return;
        }

        var openParenIndex = line.IndexOf('(', match.Index);
        if (openParenIndex < 0 || !TryReadInvocationArguments(line, openParenIndex, out var arguments) || arguments.Count < 3)
        {
            return;
        }

        if (!TryParseQuotedAlias(arguments[2].Text, out var alias))
        {
            return;
        }

        index.Aliases.Add(alias);
        TryAddDefinition(
            index.Definitions,
            alias,
            "alias",
            path,
            lineNumber,
            arguments[2].ColumnNumber,
            "Known cursor alias discovered in project source.");
    }

    private static bool TryInferAliasFromUseOperand(string operand, out string alias)
    {
        alias = string.Empty;
        var normalized = TrimQuotedToken(operand);
        if (string.IsNullOrWhiteSpace(normalized) ||
            string.Equals(normalized, "IN", StringComparison.OrdinalIgnoreCase) ||
            normalized.IndexOf('&') >= 0 ||
            normalized.IndexOf('(') >= 0)
        {
            return false;
        }

        var hasDirectorySeparator = normalized.Contains(Path.DirectorySeparatorChar) || normalized.Contains(Path.AltDirectorySeparatorChar);
        var extension = Path.GetExtension(normalized);
        if (!hasDirectorySeparator &&
            normalized.IndexOf('.') >= 0 &&
            !TableExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
        {
            return false;
        }

        var inferred = Path.GetFileNameWithoutExtension(normalized);
        if (string.IsNullOrWhiteSpace(inferred))
        {
            inferred = Path.GetFileName(normalized);
        }

        if (!AliasIdentifierRegex.IsMatch(inferred))
        {
            return false;
        }

        alias = inferred;
        return true;
    }

    private static bool TryParseQuotedAlias(string token, out string alias)
    {
        alias = string.Empty;
        var normalized = TrimQuotedToken(token);
        if (string.IsNullOrWhiteSpace(normalized) || string.Equals(normalized, token, StringComparison.Ordinal))
        {
            return false;
        }

        if (!AliasIdentifierRegex.IsMatch(normalized))
        {
            return false;
        }

        alias = normalized;
        return true;
    }

    private static string TrimQuotedToken(string token)
    {
        var trimmed = token.Trim();
        if (trimmed.Length >= 2)
        {
            if ((trimmed[0] == '"' && trimmed[^1] == '"') || (trimmed[0] == '\'' && trimmed[^1] == '\''))
            {
                return trimmed[1..^1];
            }
        }

        return trimmed;
    }

    private static bool TryReadInvocationArguments(string line, int openParenIndex, out List<InvocationArgument> arguments)
    {
        arguments = new List<InvocationArgument>();
        var depth = 0;
        char quote = '\0';
        var argumentStart = openParenIndex + 1;

        for (var index = openParenIndex; index < line.Length; index++)
        {
            var current = line[index];
            if (quote != '\0')
            {
                if (current == quote)
                {
                    quote = '\0';
                }

                continue;
            }

            if (current == '"' || current == '\'')
            {
                quote = current;
                continue;
            }

            if (current == '(')
            {
                depth++;
                continue;
            }

            if (current == ')')
            {
                depth--;
                if (depth == 0)
                {
                    AddInvocationArgument(arguments, line, argumentStart, index);
                    return true;
                }

                continue;
            }

            if (current == ',' && depth == 1)
            {
                AddInvocationArgument(arguments, line, argumentStart, index);
                argumentStart = index + 1;
            }
        }

        return false;
    }

    private static void AddInvocationArgument(List<InvocationArgument> arguments, string line, int startIndex, int endIndex)
    {
        if (endIndex <= startIndex)
        {
            arguments.Add(new InvocationArgument(string.Empty, Math.Max(1, startIndex + 1)));
            return;
        }

        var raw = line[startIndex..endIndex];
        var trimmed = raw.Trim();
        var leadingWhitespace = raw.Length - raw.TrimStart().Length;
        arguments.Add(new InvocationArgument(trimmed, startIndex + leadingWhitespace + 1));
    }

    private static string ResolveIncludePath(string sourcePath, string root, string includePath)
    {
        if (string.IsNullOrWhiteSpace(includePath))
        {
            return string.Empty;
        }

        try
        {
            if (Path.IsPathRooted(includePath))
            {
                return File.Exists(includePath) ? Path.GetFullPath(includePath) : string.Empty;
            }

            var sourceRelative = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(sourcePath) ?? string.Empty, includePath));
            if (File.Exists(sourceRelative))
            {
                return sourceRelative;
            }

            var rootRelative = Path.GetFullPath(Path.Combine(root, includePath));
            return File.Exists(rootRelative) ? rootRelative : string.Empty;
        }
        catch
        {
            return string.Empty;
        }
    }

    private static void AddMatch(
        ISet<string> bucket,
        IDictionary<string, FoxProDefinitionLocation> definitions,
        Regex regex,
        string line,
        string path,
        int lineNumber,
        string kind,
        string description)
    {
        var match = regex.Match(line);
        if (match.Success)
        {
            var name = match.Groups[1].Value;
            bucket.Add(name);
            TryAddDefinition(definitions, name, kind, path, lineNumber, match.Groups[1].Index + 1, description);
        }
    }

    private static IEnumerable<string> EnumerateProjectFiles(string root)
    {
        var pending = new Stack<string>();
        pending.Push(root);
        var count = 0;
        while (pending.Count > 0 && count < 2000)
        {
            var current = pending.Pop();
            IEnumerable<string> directories;
            try
            {
                directories = Directory.EnumerateDirectories(current);
            }
            catch
            {
                continue;
            }

            foreach (var directory in directories)
            {
                var name = Path.GetFileName(directory);
                if (IgnoredDirectories.Contains(name, StringComparer.OrdinalIgnoreCase))
                {
                    continue;
                }
                pending.Push(directory);
            }

            IEnumerable<string> files;
            try
            {
                files = Directory.EnumerateFiles(current);
            }
            catch
            {
                continue;
            }

            foreach (var file in files)
            {
                ++count;
                yield return file;
                if (count >= 2000)
                {
                    yield break;
                }
            }
        }
    }

    private static string ResolveProjectRoot(string filePath)
    {
        var directory = Path.GetDirectoryName(filePath);
        if (string.IsNullOrWhiteSpace(directory))
        {
            return Path.GetDirectoryName(filePath) ?? string.Empty;
        }

        var current = new DirectoryInfo(directory);
        for (var depth = 0; current is not null && depth < 8; depth++, current = current.Parent)
        {
            if (current.EnumerateFiles("*.pjx").Any() || current.EnumerateFiles("*.sln").Any())
            {
                return current.FullName;
            }
        }

        return directory;
    }

    private static int RankKind(string kind)
    {
        return kind switch
        {
            "symbol" => 0,
            "class" => 1,
            "alias" => 2,
            "asset" => 3,
            "function" => 4,
            "keyword" => 5,
            "set" => 6,
            "define" => 7,
            "member" => 8,
            _ => 9
        };
    }

    private static string NormalizeLookupToken(string token)
    {
        var normalized = token.Trim().TrimEnd('(', ')');
        return normalized.StartsWith("#", StringComparison.Ordinal) ? normalized : normalized.TrimStart('&');
    }

    private static bool TokenMatches(string candidate, string key)
    {
        if (string.Equals(candidate, key, StringComparison.OrdinalIgnoreCase))
        {
            return true;
        }

        var normalizedCandidate = NormalizeLookupToken(candidate);
        return string.Equals(normalizedCandidate, key, StringComparison.OrdinalIgnoreCase);
    }

    private static bool TryResolveDefinition(ProjectSymbolIndex index, string key, out FoxProDefinitionLocation definition)
    {
        if (index.Definitions.TryGetValue(key, out definition!))
        {
            return true;
        }

        definition = new FoxProDefinitionLocation();
        return false;
    }

    private static bool TryResolveDefinitionCandidate(ProjectSymbolIndex index, string token, out FoxProDefinitionLocation definition)
    {
        if (TryResolveDefinition(index, token, out definition))
        {
            return true;
        }

        if (token.IndexOf('.') < 0)
        {
            return false;
        }

        var segments = token.Split('.', StringSplitOptions.RemoveEmptyEntries);
        for (var segmentCount = segments.Length - 1; segmentCount > 0; segmentCount--)
        {
            var prefix = string.Join(".", segments.Take(segmentCount));
            if (TryResolveDefinition(index, prefix, out definition))
            {
                return true;
            }
        }

        if (TryResolveUniqueProjectMethodDefinition(index, segments[^1], out definition))
        {
            return true;
        }

        return TryResolveDefinition(index, segments[^1], out definition);
    }

    private static IReadOnlyList<FoxProSignatureEntry> TryResolveProjectSignatures(ProjectSymbolIndex index, string invocationName)
    {
        if (index.Signatures.TryGetValue(invocationName, out var signatures))
        {
            return signatures;
        }

        if (invocationName.IndexOf('.') < 0)
        {
            return Array.Empty<FoxProSignatureEntry>();
        }

        var segments = invocationName.Split('.', StringSplitOptions.RemoveEmptyEntries);
        for (var segmentCount = segments.Length - 1; segmentCount > 0; segmentCount--)
        {
            var prefix = string.Join(".", segments.Take(segmentCount));
            if (index.Signatures.TryGetValue(prefix, out signatures))
            {
                return signatures;
            }
        }

        if (TryResolveUniqueProjectMethodSignature(index, segments[^1], out signatures))
        {
            return signatures;
        }

        return index.Signatures.TryGetValue(segments[^1], out signatures)
            ? signatures
            : Array.Empty<FoxProSignatureEntry>();
    }

    private static bool TryResolveUniqueProjectMethodSignature(
        ProjectSymbolIndex index,
        string methodName,
        out IReadOnlyList<FoxProSignatureEntry> signatures)
    {
        signatures = Array.Empty<FoxProSignatureEntry>();
        var matches = index.Methods
            .Where(method => string.Equals(ExtractMethodName(method), methodName, StringComparison.OrdinalIgnoreCase))
            .Take(2)
            .ToList();
        if (matches.Count != 1)
        {
            return false;
        }

        if (!index.Signatures.TryGetValue(matches[0], out var resolvedSignatures))
        {
            return false;
        }

        signatures = resolvedSignatures;
        return true;
    }

    private static bool TryResolveUniqueProjectMethodDefinition(
        ProjectSymbolIndex index,
        string methodName,
        out FoxProDefinitionLocation definition)
    {
        definition = new FoxProDefinitionLocation();
        var matches = index.Methods
            .Where(method => string.Equals(ExtractMethodName(method), methodName, StringComparison.OrdinalIgnoreCase))
            .Take(2)
            .ToList();
        if (matches.Count != 1)
        {
            return false;
        }

        return TryResolveDefinition(index, matches[0], out definition);
    }

    private static void AddAsset(
        ISet<string> bucket,
        ProjectSymbolIndex index,
        string name,
        string kind,
        string path,
        string description)
    {
        bucket.Add(name);
        TryAddDefinition(index.Definitions, name, kind, path, 1, 1, description);
    }

    private static void TryAddDefinition(
        IDictionary<string, FoxProDefinitionLocation> definitions,
        string name,
        string kind,
        string path,
        int lineNumber,
        int columnNumber,
        string description)
    {
        if (string.IsNullOrWhiteSpace(name) || definitions.ContainsKey(name))
        {
            return;
        }

        definitions[name] = new FoxProDefinitionLocation
        {
            Name = name,
            Kind = kind,
            FilePath = path,
            LineNumber = lineNumber,
            ColumnNumber = Math.Max(1, columnNumber),
            Description = description
        };
    }

    private static void TryAddProjectSignature(
        IDictionary<string, IReadOnlyList<FoxProSignatureEntry>> signatures,
        string name,
        IReadOnlyList<string> lines,
        int definitionLineIndex,
        string documentation)
    {
        if (string.IsNullOrWhiteSpace(name) || signatures.ContainsKey(name))
        {
            return;
        }

        var rawParameters = TryReadProjectParameterList(lines, definitionLineIndex);
        var parameters = ParseProjectParameters(rawParameters);
        var content = parameters.Count == 0
            ? $"{name}()"
            : $"{name}({string.Join(", ", parameters.Select(parameter => parameter.Documentation))})";

        signatures[name] = new[]
        {
            new FoxProSignatureEntry
            {
                Name = name,
                Content = content,
                Documentation = documentation,
                Parameters = parameters
            }
        };
    }

    private static string TryReadProjectParameterList(IReadOnlyList<string> lines, int definitionLineIndex)
    {
        for (var lineIndex = definitionLineIndex + 1; lineIndex < lines.Count; lineIndex++)
        {
            var candidate = lines[lineIndex].Trim();
            if (string.IsNullOrWhiteSpace(candidate) || candidate.StartsWith("*", StringComparison.Ordinal) || candidate.StartsWith("&&", StringComparison.Ordinal))
            {
                continue;
            }

            var match = ParametersRegex.Match(candidate);
            return match.Success ? match.Groups[1].Value.Trim() : string.Empty;
        }

        return string.Empty;
    }

    private static IReadOnlyList<FoxProParameterEntry> ParseProjectParameters(string rawParameters)
    {
        if (string.IsNullOrWhiteSpace(rawParameters))
        {
            return Array.Empty<FoxProParameterEntry>();
        }

        return rawParameters
            .Split(',')
            .Select(parameter => parameter.Trim())
            .Where(parameter => !string.IsNullOrWhiteSpace(parameter))
            .Select(parameter => new FoxProParameterEntry
            {
                Name = NormalizeProjectParameterName(parameter),
                Documentation = parameter
            })
            .ToList();
    }

    private static string NormalizeProjectParameterName(string parameter)
    {
        var separatorIndex = parameter.IndexOf('=');
        var candidate = separatorIndex >= 0 ? parameter[..separatorIndex] : parameter;
        var asIndex = candidate.IndexOf(" AS ", StringComparison.OrdinalIgnoreCase);
        if (asIndex >= 0)
        {
            candidate = candidate[..asIndex];
        }

        return candidate.Trim();
    }

    private static FoxProSignatureEntry CreateSignature(string name, string content, string documentation, params (string Name, string Documentation)[] parameters)
    {
        return new FoxProSignatureEntry
        {
            Name = name,
            Content = content,
            Documentation = documentation,
            Parameters = parameters.Select(parameter => new FoxProParameterEntry
            {
                Name = parameter.Name,
                Documentation = parameter.Documentation
            }).ToList()
        };
    }

    private sealed class ProjectSymbolIndex
    {
        public string Root { get; set; } = string.Empty;
        public DateTime BuiltAtUtc { get; set; }
        public HashSet<string> Procedures { get; } = new(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Classes { get; } = new(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Methods { get; } = new(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Defines { get; } = new(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Aliases { get; } = new(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Tables { get; } = new(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Forms { get; } = new(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Reports { get; } = new(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Labels { get; } = new(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Menus { get; } = new(StringComparer.OrdinalIgnoreCase);
        public Dictionary<string, FoxProDefinitionLocation> Definitions { get; } = new(StringComparer.OrdinalIgnoreCase);
        public Dictionary<string, IReadOnlyList<FoxProSignatureEntry>> Signatures { get; } = new(StringComparer.OrdinalIgnoreCase);

        public bool ShouldRefresh => (DateTime.UtcNow - BuiltAtUtc) > TimeSpan.FromSeconds(15);
    }

    private readonly record struct InvocationArgument(string Text, int ColumnNumber);
}

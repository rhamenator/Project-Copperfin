// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
    private static readonly Regex IncludeRegex = new(@"^\s*#INCLUDE\s+(?:""([^""]+)""|<([^>]+)>|([^\s&]+))", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex UseAliasRegex = new(@"^\s*USE\s+.+?\s+ALIAS\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex UseStatementRegex = new(@"^\s*USE\s+(""[^""]+""|'[^']+'|[^\s]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex CreateCursorRegex = new(@"^\s*CREATE\s+CURSOR\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex IntoCursorRegex = new(@"\bINTO\s+CURSOR\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex SqlExecInvocationRegex = new(@"\bSQLEXEC\s*\(", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex ParametersRegex = new(@"^\s*(?:LPARAMETERS?|PARAMETERS)\s+(.+)$", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex MemberAccessRegex = new(@"([A-Za-z_][A-Za-z0-9_]*)\.$", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex AliasIdentifierRegex = new(@"^[A-Za-z_][A-Za-z0-9_\.]*$", RegexOptions.Compiled);

    private static readonly (string Name, string DescriptionKey)[] Keywords =
    {
        ("SELECT", "LanguageService.IntelliSense.Keyword.Select"),
        ("USE", "LanguageService.IntelliSense.Keyword.Use"),
        ("IN", "LanguageService.IntelliSense.Keyword.In"),
        ("ALIAS", "LanguageService.IntelliSense.Keyword.Alias"),
        ("DO", "LanguageService.IntelliSense.Keyword.Do"),
        ("DO FORM", "LanguageService.IntelliSense.Keyword.DoForm"),
        ("REPORT FORM", "LanguageService.IntelliSense.Keyword.ReportForm"),
        ("LABEL FORM", "LanguageService.IntelliSense.Keyword.LabelForm"),
        ("SET", "LanguageService.IntelliSense.Keyword.Set"),
        ("SET DATASESSION TO", "LanguageService.IntelliSense.Keyword.SetDataSessionTo"),
        ("READ EVENTS", "LanguageService.IntelliSense.Keyword.ReadEvents"),
        ("CLEAR EVENTS", "LanguageService.IntelliSense.Keyword.ClearEvents"),
        ("PUBLIC", "LanguageService.IntelliSense.Keyword.Public"),
        ("LOCAL", "LanguageService.IntelliSense.Keyword.Local"),
        ("RETURN", "LanguageService.IntelliSense.Keyword.Return"),
        ("DEFINE CLASS", "LanguageService.IntelliSense.Keyword.DefineClass"),
        ("ENDDEFINE", "LanguageService.IntelliSense.Keyword.EndDefine"),
        ("IF", "LanguageService.IntelliSense.Keyword.If"),
        ("ELSE", "LanguageService.IntelliSense.Keyword.Else"),
        ("ENDIF", "LanguageService.IntelliSense.Keyword.EndIf"),
        ("FOR", "LanguageService.IntelliSense.Keyword.For"),
        ("ENDFOR", "LanguageService.IntelliSense.Keyword.EndFor"),
        ("ON ERROR", "LanguageService.IntelliSense.Keyword.OnError"),
        ("CREATEOBJECT", "LanguageService.IntelliSense.Keyword.CreateObject"),
        ("GETOBJECT", "LanguageService.IntelliSense.Keyword.GetObject"),
        ("SQLCONNECT", "LanguageService.IntelliSense.Keyword.SqlConnect"),
        ("SQLSTRINGCONNECT", "LanguageService.IntelliSense.Keyword.SqlStringConnect"),
        ("SQLEXEC", "LanguageService.IntelliSense.Keyword.SqlExec"),
        ("SQLDISCONNECT", "LanguageService.IntelliSense.Keyword.SqlDisconnect")
    };

    private static readonly (string Name, string DescriptionKey)[] Functions =
    {
        ("ALIAS()", "LanguageService.IntelliSense.Function.Alias"),
        ("SELECT()", "LanguageService.IntelliSense.Function.Select"),
        ("RECCOUNT()", "LanguageService.IntelliSense.Function.RecCount"),
        ("RECNO()", "LanguageService.IntelliSense.Function.RecNo"),
        ("EOF()", "LanguageService.IntelliSense.Function.Eof"),
        ("BOF()", "LanguageService.IntelliSense.Function.Bof"),
        ("ALLTRIM()", "LanguageService.IntelliSense.Function.AllTrim"),
        ("JUSTPATH()", "LanguageService.IntelliSense.Function.JustPath"),
        ("MESSAGE()", "LanguageService.IntelliSense.Function.Message"),
        ("MESSAGEBOX()", "LanguageService.IntelliSense.Function.MessageBox"),
        ("SYS()", "LanguageService.IntelliSense.Function.Sys"),
        ("STR()", "LanguageService.IntelliSense.Function.Str"),
        ("DATE()", "LanguageService.IntelliSense.Function.Date"),
        ("DAY()", "LanguageService.IntelliSense.Function.Day"),
        ("MONTH()", "LanguageService.IntelliSense.Function.Month"),
        ("YEAR()", "LanguageService.IntelliSense.Function.Year"),
        ("CMONTH()", "LanguageService.IntelliSense.Function.CMonth"),
        ("IIF()", "LanguageService.IntelliSense.Function.Iif"),
        ("FILE()", "LanguageService.IntelliSense.Function.File"),
        ("CURSORGETPROP()", "LanguageService.IntelliSense.Function.CursorGetProp"),
        ("CREATEOBJECT()", "LanguageService.IntelliSense.Function.CreateObject"),
        ("GETOBJECT()", "LanguageService.IntelliSense.Function.GetObject")
    };

    private static readonly (string Name, string DescriptionKey)[] SetKeywords =
    {
        ("DELETED", "LanguageService.IntelliSense.Set.Deleted"),
        ("SAFETY", "LanguageService.IntelliSense.Set.Safety"),
        ("EXCLUSIVE", "LanguageService.IntelliSense.Set.Exclusive"),
        ("TALK", "LanguageService.IntelliSense.Set.Talk"),
        ("EXACT", "LanguageService.IntelliSense.Set.Exact"),
        ("MULTILOCKS", "LanguageService.IntelliSense.Set.MultiLocks"),
        ("PATH", "LanguageService.IntelliSense.Set.Path"),
        ("DEFAULT TO", "LanguageService.IntelliSense.Set.DefaultTo"),
        ("DATASESSION TO", "LanguageService.IntelliSense.Set.DataSessionTo")
    };

    private static readonly (string Name, string DescriptionKey)[] GenericObjectMembers =
    {
        ("Visible", "LanguageService.IntelliSense.Member.Visible"),
        ("Caption", "LanguageService.IntelliSense.Member.Caption"),
        ("Value", "LanguageService.IntelliSense.Member.Value"),
        ("Count", "LanguageService.IntelliSense.Member.Count"),
        ("Item()", "LanguageService.IntelliSense.Member.Item"),
        ("Add()", "LanguageService.IntelliSense.Member.Add"),
        ("Open()", "LanguageService.IntelliSense.Member.Open"),
        ("Close()", "LanguageService.IntelliSense.Member.Close"),
        ("Execute()", "LanguageService.IntelliSense.Member.Execute"),
        ("Refresh()", "LanguageService.IntelliSense.Member.Refresh"),
        ("Quit()", "LanguageService.IntelliSense.Member.Quit"),
        ("Workbooks", "LanguageService.IntelliSense.Member.Workbooks"),
        ("ActiveWorkbook", "LanguageService.IntelliSense.Member.ActiveWorkbook"),
        ("Documents", "LanguageService.IntelliSense.Member.Documents")
    };

    private static readonly SignatureTemplate[] SignatureTemplates =
    {
        CreateSignatureTemplate("ALIAS", "ALIAS([nWorkArea | cAlias])", "LanguageService.IntelliSense.Signature.Alias.Documentation", ("nWorkArea | cAlias", "LanguageService.IntelliSense.Signature.Alias.Parameter.WorkAreaOrAlias")),
        CreateSignatureTemplate("SELECT", "SELECT([nWorkArea | cAlias])", "LanguageService.IntelliSense.Signature.Select.Documentation", ("nWorkArea | cAlias", "LanguageService.IntelliSense.Signature.Select.Parameter.WorkAreaOrAlias")),
        CreateSignatureTemplate("RECCOUNT", "RECCOUNT([cAlias])", "LanguageService.IntelliSense.Signature.RecCount.Documentation", ("cAlias", "LanguageService.IntelliSense.Signature.RecCount.Parameter.Alias")),
        CreateSignatureTemplate("RECNO", "RECNO([cAlias])", "LanguageService.IntelliSense.Signature.RecNo.Documentation", ("cAlias", "LanguageService.IntelliSense.Signature.RecNo.Parameter.Alias")),
        CreateSignatureTemplate("EOF", "EOF([cAlias])", "LanguageService.IntelliSense.Signature.Eof.Documentation", ("cAlias", "LanguageService.IntelliSense.Signature.Eof.Parameter.Alias")),
        CreateSignatureTemplate("BOF", "BOF([cAlias])", "LanguageService.IntelliSense.Signature.Bof.Documentation", ("cAlias", "LanguageService.IntelliSense.Signature.Bof.Parameter.Alias")),
        CreateSignatureTemplate("CREATEOBJECT", "CREATEOBJECT(cClass [, eInitParameter1 [, eInitParameterN]])", "LanguageService.IntelliSense.Signature.CreateObject.Documentation", ("cClass", "LanguageService.IntelliSense.Signature.CreateObject.Parameter.Class"), ("eInitParameter1", "LanguageService.IntelliSense.Signature.CreateObject.Parameter.Init1"), ("eInitParameterN", "LanguageService.IntelliSense.Signature.CreateObject.Parameter.InitN")),
        CreateSignatureTemplate("GETOBJECT", "GETOBJECT([cFileName] [, cClass])", "LanguageService.IntelliSense.Signature.GetObject.Documentation", ("cFileName", "LanguageService.IntelliSense.Signature.GetObject.Parameter.FileName"), ("cClass", "LanguageService.IntelliSense.Signature.GetObject.Parameter.Class")),
        CreateSignatureTemplate("SQLCONNECT", "SQLCONNECT(cDataSourceName [, cUserId [, cPassword [, lShared]]])", "LanguageService.IntelliSense.Signature.SqlConnect.Documentation", ("cDataSourceName", "LanguageService.IntelliSense.Signature.SqlConnect.Parameter.DataSourceName"), ("cUserId", "LanguageService.IntelliSense.Signature.SqlConnect.Parameter.UserId"), ("cPassword", "LanguageService.IntelliSense.Signature.SqlConnect.Parameter.Password"), ("lShared", "LanguageService.IntelliSense.Signature.SqlConnect.Parameter.Shared")),
        CreateSignatureTemplate("SQLSTRINGCONNECT", "SQLSTRINGCONNECT(cConnectString [, lShared])", "LanguageService.IntelliSense.Signature.SqlStringConnect.Documentation", ("cConnectString", "LanguageService.IntelliSense.Signature.SqlStringConnect.Parameter.ConnectString"), ("lShared", "LanguageService.IntelliSense.Signature.SqlStringConnect.Parameter.Shared")),
        CreateSignatureTemplate("SQLEXEC", "SQLEXEC(nConnectionHandle, cCommand [, cCursorName])", "LanguageService.IntelliSense.Signature.SqlExec.Documentation", ("nConnectionHandle", "LanguageService.IntelliSense.Signature.SqlExec.Parameter.ConnectionHandle"), ("cCommand", "LanguageService.IntelliSense.Signature.SqlExec.Parameter.Command"), ("cCursorName", "LanguageService.IntelliSense.Signature.SqlExec.Parameter.CursorName")),
        CreateSignatureTemplate("SQLDISCONNECT", "SQLDISCONNECT([nConnectionHandle])", "LanguageService.IntelliSense.Signature.SqlDisconnect.Documentation", ("nConnectionHandle", "LanguageService.IntelliSense.Signature.SqlDisconnect.Parameter.ConnectionHandle")),
        CreateSignatureTemplate("CURSORGETPROP", "CURSORGETPROP(cProperty [, cCursorName])", "LanguageService.IntelliSense.Signature.CursorGetProp.Documentation", ("cProperty", "LanguageService.IntelliSense.Signature.CursorGetProp.Parameter.Property"), ("cCursorName", "LanguageService.IntelliSense.Signature.CursorGetProp.Parameter.CursorName")),
        CreateSignatureTemplate("MESSAGEBOX", "MESSAGEBOX(cMessage [, nDialogBoxType [, cTitleBarText]])", "LanguageService.IntelliSense.Signature.MessageBox.Documentation", ("cMessage", "LanguageService.IntelliSense.Signature.MessageBox.Parameter.Message"), ("nDialogBoxType", "LanguageService.IntelliSense.Signature.MessageBox.Parameter.DialogBoxType"), ("cTitleBarText", "LanguageService.IntelliSense.Signature.MessageBox.Parameter.TitleBarText")),
        CreateSignatureTemplate("SYS", "SYS(nFunction [, eExpression1 [, eExpressionN]])", "LanguageService.IntelliSense.Signature.Sys.Documentation", ("nFunction", "LanguageService.IntelliSense.Signature.Sys.Parameter.Function"), ("eExpression1", "LanguageService.IntelliSense.Signature.Sys.Parameter.Expression1"), ("eExpressionN", "LanguageService.IntelliSense.Signature.Sys.Parameter.ExpressionN")),
        CreateSignatureTemplate("IIF", "IIF(lExpression, eTrueValue, eFalseValue)", "LanguageService.IntelliSense.Signature.Iif.Documentation", ("lExpression", "LanguageService.IntelliSense.Signature.Iif.Parameter.Expression"), ("eTrueValue", "LanguageService.IntelliSense.Signature.Iif.Parameter.TrueValue"), ("eFalseValue", "LanguageService.IntelliSense.Signature.Iif.Parameter.FalseValue"))
    };

    private static readonly Dictionary<string, IReadOnlyList<SignatureTemplate>> SignatureTemplateLookup =
        SignatureTemplates.GroupBy(entry => NormalizeLookupToken(entry.Name), StringComparer.OrdinalIgnoreCase)
            .ToDictionary(group => group.Key, group => (IReadOnlyList<SignatureTemplate>)group.ToList(), StringComparer.OrdinalIgnoreCase);

    private static readonly ConcurrentDictionary<string, ProjectSymbolIndex> Cache = new(StringComparer.OrdinalIgnoreCase);

    public static IReadOnlyList<FoxProCompletionEntry> BuildEntries(string? filePath, string linePrefix, string tokenPrefix)
    {
        var localization = CopperfinLocalization.FromVisualStudioUiCulture();
        var prefix = tokenPrefix ?? string.Empty;
        var completions = new Dictionary<string, FoxProCompletionEntry>(StringComparer.OrdinalIgnoreCase);
        AddEntries(completions, Keywords, "keyword", priority: 200, localization);
        AddEntries(completions, Functions, "function", priority: 180, localization);

        if (LooksLikeSetContext(linePrefix))
        {
            AddEntries(completions, SetKeywords, "set", priority: 20, localization);
        }

        if (!string.IsNullOrWhiteSpace(filePath))
        {
            var index = GetProjectIndex(filePath!);
            if (LooksLikeMemberAccess(linePrefix))
            {
                AddProjectMemberEntries(completions, index, localization);
            }
            AddContextualProjectEntries(completions, index, linePrefix, localization);
            AddSymbolEntries(completions, index, localization);
        }

        if (LooksLikeMemberAccess(linePrefix))
        {
            AddEntries(completions, GenericObjectMembers, "member", priority: 20, localization);
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

        var localization = CopperfinLocalization.FromVisualStudioUiCulture();
        var key = NormalizeLookupToken(token);
        foreach (var item in Keywords)
        {
            if (TokenMatches(item.Name, key))
            {
                return localization.Text(item.DescriptionKey);
            }
        }

        foreach (var item in Functions)
        {
            if (TokenMatches(item.Name, key))
            {
                return localization.Text(item.DescriptionKey);
            }
        }

        foreach (var item in SetKeywords)
        {
            if (TokenMatches(item.Name, key))
            {
                return localization.Text(item.DescriptionKey);
            }
        }

        foreach (var item in GenericObjectMembers)
        {
            if (TokenMatches(item.Name, key))
            {
                return localization.Text(item.DescriptionKey);
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
                return localization.Text("LanguageService.IntelliSense.Project.ProcedureOrFunctionSymbol");
            }

            if (index.Classes.Contains(key))
            {
                return localization.Text("LanguageService.IntelliSense.Project.ClassSymbol");
            }

            if (index.Defines.Contains(key))
            {
                return localization.Text("LanguageService.IntelliSense.Project.PreprocessorSymbol");
            }

            if (index.Aliases.Contains(key))
            {
                return localization.Text("LanguageService.IntelliSense.Project.WorkAreaAliasDiscovered");
            }

            if (index.Tables.Contains(key))
            {
                return localization.Text("LanguageService.IntelliSense.Project.TableAsset");
            }

            if (index.Forms.Contains(key))
            {
                return localization.Text("LanguageService.IntelliSense.Project.FormAsset");
            }

            if (index.Reports.Contains(key))
            {
                return localization.Text("LanguageService.IntelliSense.Project.ReportAsset");
            }

            if (index.Labels.Contains(key))
            {
                return localization.Text("LanguageService.IntelliSense.Project.LabelAsset");
            }

            if (index.Menus.Contains(key))
            {
                return localization.Text("LanguageService.IntelliSense.Project.MenuAsset");
            }
        }

        return string.Empty;
    }

    public static IReadOnlyList<FoxProSignatureEntry> GetSignatures(string? filePath, string invocationName)
    {
        var localization = CopperfinLocalization.FromVisualStudioUiCulture();
        var key = NormalizeLookupToken(invocationName);
        if (SignatureTemplateLookup.TryGetValue(key, out var signatureTemplates))
        {
            return LocalizeSignatures(signatureTemplates, localization);
        }

        if (invocationName.IndexOf('.') >= 0)
        {
            var memberName = invocationName.Split('.').Last();
            if (SignatureTemplateLookup.TryGetValue(NormalizeLookupToken(memberName), out signatureTemplates))
            {
                return LocalizeSignatures(signatureTemplates, localization);
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

    internal static void ClearCacheForTests()
    {
        Cache.Clear();
    }

    private static void AddContextualProjectEntries(
        IDictionary<string, FoxProCompletionEntry> completions,
        ProjectSymbolIndex index,
        string linePrefix,
        CopperfinLocalization localization)
    {
        var upper = linePrefix.TrimStart().ToUpperInvariant();
        if (upper.StartsWith("DO FORM ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Forms.Select(name => (name, "LanguageService.IntelliSense.Project.FormAsset")), "asset", priority: 0, localization);
            return;
        }

        if (upper.StartsWith("REPORT FORM ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Reports.Select(name => (name, "LanguageService.IntelliSense.Project.ReportAsset")), "asset", priority: 0, localization);
            return;
        }

        if (upper.StartsWith("LABEL FORM ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Labels.Select(name => (name, "LanguageService.IntelliSense.Project.LabelAsset")), "asset", priority: 0, localization);
            return;
        }

        if (upper.StartsWith("USE ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Tables.Select(name => (name, "LanguageService.IntelliSense.Project.TableAsset")), "asset", priority: 0, localization);
            AddEntries(completions, index.Aliases.Select(name => (name, "LanguageService.IntelliSense.Project.AliasFromProjectSource")), "alias", priority: 0, localization);
            return;
        }

        if (upper.StartsWith("SELECT ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Aliases.Select(name => (name, "LanguageService.IntelliSense.Project.WorkAreaAliasFromProjectSource")), "alias", priority: 0, localization);
            AddEntries(completions, index.Tables.Select(name => (name, "LanguageService.IntelliSense.Project.TableAsset")), "asset", priority: 0, localization);
            return;
        }

        if (upper.StartsWith("DO ", StringComparison.Ordinal) && !upper.StartsWith("DO FORM ", StringComparison.Ordinal))
        {
            AddEntries(completions, index.Procedures.Select(name => (name, "LanguageService.IntelliSense.Project.ProcedureFunctionProgramSymbol")), "symbol", priority: 0, localization);
            AddEntries(completions, index.Menus.Select(name => (name, "LanguageService.IntelliSense.Project.MenuAsset")), "asset", priority: 0, localization);
        }
    }

    private static void AddSymbolEntries(IDictionary<string, FoxProCompletionEntry> completions, ProjectSymbolIndex index, CopperfinLocalization localization)
    {
        AddEntries(completions, index.Procedures.Select(name => (name, "LanguageService.IntelliSense.Project.ProcedureFunctionSymbol")), "symbol", priority: 100, localization);
        AddEntries(completions, index.Classes.Select(name => (name, "LanguageService.IntelliSense.Project.ClassSymbolInActiveProject")), "class", priority: 100, localization);
        AddEntries(completions, index.Defines.Select(name => (name, "LanguageService.IntelliSense.Project.PreprocessorSymbolInActiveProject")), "define", priority: 100, localization);
        AddEntries(completions, index.Aliases.Select(name => (name, "LanguageService.IntelliSense.Project.AliasFromUseAlias")), "alias", priority: 100, localization);
    }

    private static void AddProjectMemberEntries(IDictionary<string, FoxProCompletionEntry> completions, ProjectSymbolIndex index, CopperfinLocalization localization)
    {
        AddEntries(
            completions,
            index.Methods.Select(method => (ExtractMethodName(method), localization.Format("LanguageService.IntelliSense.Project.MethodMemberFromType", ExtractContainingType(method)))),
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
        return separator >= 0 ? qualifiedMethodName.Substring(separator + 1) : qualifiedMethodName;
    }

    private static string ExtractContainingType(string qualifiedMethodName)
    {
        var separator = qualifiedMethodName.LastIndexOf('.');
        return separator >= 0
            ? qualifiedMethodName.Substring(0, separator)
            : CopperfinLocalization.FromVisualStudioUiCulture().Text("LanguageService.IntelliSense.Project.ActiveProject");
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

    private static void AddEntries(
        IDictionary<string, FoxProCompletionEntry> completions,
        IEnumerable<(string Name, string DescriptionKey)> source,
        string kind,
        int priority,
        CopperfinLocalization localization)
    {
        AddEntries(completions, source.Select(item => (item.Name, localization.Text(item.DescriptionKey))), kind, priority);
    }

    private static ProjectSymbolIndex GetProjectIndex(string filePath)
    {
        var root = ResolveProjectRoot(filePath);
        var localization = CopperfinLocalization.FromVisualStudioUiCulture();
        var cacheKey = root + "|" + localization.Locale;
        return Cache.AddOrUpdate(
            cacheKey,
            _ => BuildProjectIndex(root, localization),
            (_, existing) => existing.ShouldRefresh ? BuildProjectIndex(root, localization) : existing);
    }

    private static ProjectSymbolIndex BuildProjectIndex(string root, CopperfinLocalization localization)
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
                AddAsset(index.Forms, index, Path.GetFileNameWithoutExtension(file), "form asset", file, localization.Text("LanguageService.IntelliSense.Project.FormAsset"));
                continue;
            }
            if (ReportExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                AddAsset(index.Reports, index, Path.GetFileNameWithoutExtension(file), "report asset", file, localization.Text("LanguageService.IntelliSense.Project.ReportAsset"));
                continue;
            }
            if (LabelExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                AddAsset(index.Labels, index, Path.GetFileNameWithoutExtension(file), "label asset", file, localization.Text("LanguageService.IntelliSense.Project.LabelAsset"));
                continue;
            }
            if (MenuExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                AddAsset(index.Menus, index, Path.GetFileNameWithoutExtension(file), "menu asset", file, localization.Text("LanguageService.IntelliSense.Project.MenuAsset"));
                continue;
            }
            if (TableExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                AddAsset(index.Tables, index, Path.GetFileNameWithoutExtension(file), "table asset", file, localization.Text("LanguageService.IntelliSense.Project.TableAsset"));
                continue;
            }
            if (!TextExtensions.Contains(extension, StringComparer.OrdinalIgnoreCase))
            {
                continue;
            }

            ScanTextFile(file, root, index, scannedFiles, localization);
        }

        return index;
    }

    private static void ScanTextFile(string path, string root, ProjectSymbolIndex index, ISet<string> scannedFiles, CopperfinLocalization localization)
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
                TryAddDefinition(index.Definitions, currentClassName, "class", normalizedPath, lineIndex + 1, classMatch.Groups[1].Index + 1, localization.Format("LanguageService.IntelliSense.Project.ClassSymbolDerivingFrom", classMatch.Groups[2].Value));
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
                    TryAddDefinition(index.Definitions, methodName, "method", normalizedPath, lineIndex + 1, methodProcedureMatch.Groups[1].Index + 1, localization.Format("LanguageService.IntelliSense.Project.MethodSymbolOnClass", currentClassName));
                    TryAddProjectSignature(index.Signatures, methodName, lines, lineIndex, localization.Text("LanguageService.IntelliSense.Project.MethodSignatureDiscovered"));
                    continue;
                }

                var methodFunctionMatch = FunctionRegex.Match(line);
                if (methodFunctionMatch.Success)
                {
                    var methodName = $"{currentClassName}.{methodFunctionMatch.Groups[1].Value}";
                    index.Methods.Add(methodName);
                    TryAddDefinition(index.Definitions, methodName, "method", normalizedPath, lineIndex + 1, methodFunctionMatch.Groups[1].Index + 1, localization.Format("LanguageService.IntelliSense.Project.MethodSymbolOnClass", currentClassName));
                    TryAddProjectSignature(index.Signatures, methodName, lines, lineIndex, localization.Text("LanguageService.IntelliSense.Project.MethodSignatureDiscovered"));
                    continue;
                }
            }

            var procedureMatch = ProcedureRegex.Match(line);
            if (procedureMatch.Success)
            {
                var name = procedureMatch.Groups[1].Value;
                index.Procedures.Add(name);
                TryAddDefinition(index.Definitions, name, "procedure", normalizedPath, lineIndex + 1, procedureMatch.Groups[1].Index + 1, localization.Text("LanguageService.IntelliSense.Project.ProcedureSymbol"));
                TryAddProjectSignature(index.Signatures, name, lines, lineIndex, localization.Text("LanguageService.IntelliSense.Project.ProcedureSignatureDiscovered"));
            }

            var functionMatch = FunctionRegex.Match(line);
            if (functionMatch.Success)
            {
                var name = functionMatch.Groups[1].Value;
                index.Procedures.Add(name);
                TryAddDefinition(index.Definitions, name, "function", normalizedPath, lineIndex + 1, functionMatch.Groups[1].Index + 1, localization.Text("LanguageService.IntelliSense.Project.FunctionSymbol"));
                TryAddProjectSignature(index.Signatures, name, lines, lineIndex, localization.Text("LanguageService.IntelliSense.Project.FunctionSignatureDiscovered"));
            }

            AddMatch(index.Defines, index.Definitions, DefineRegex, line, normalizedPath, lineIndex + 1, "define", localization.Text("LanguageService.IntelliSense.Project.PreprocessorSymbol"));
            AddMatch(index.Aliases, index.Definitions, UseAliasRegex, line, normalizedPath, lineIndex + 1, "alias", localization.Text("LanguageService.IntelliSense.Project.WorkAreaAliasDiscovered"));
            TryAddImplicitUseAlias(index, line, normalizedPath, lineIndex + 1, localization);
            AddMatch(index.Aliases, index.Definitions, CreateCursorRegex, line, normalizedPath, lineIndex + 1, "alias", localization.Text("LanguageService.IntelliSense.Project.CursorAliasDiscovered"));
            AddMatch(index.Aliases, index.Definitions, IntoCursorRegex, line, normalizedPath, lineIndex + 1, "alias", localization.Text("LanguageService.IntelliSense.Project.CursorAliasDiscovered"));
            TryAddSqlExecCursorAlias(index, line, normalizedPath, lineIndex + 1, localization);

            var includeMatch = IncludeRegex.Match(line);
            if (includeMatch.Success)
            {
                var includeOperand = includeMatch.Groups
                    .Cast<Group>()
                    .Skip(1)
                    .First(group => group.Success)
                    .Value;
                var includePath = ResolveIncludePath(normalizedPath, root, includeOperand);
                if (!string.IsNullOrWhiteSpace(includePath) &&
                    TextExtensions.Contains(Path.GetExtension(includePath), StringComparer.OrdinalIgnoreCase))
                {
                    ScanTextFile(includePath, root, index, scannedFiles, localization);
                }
            }
        }
    }

    private static void TryAddImplicitUseAlias(ProjectSymbolIndex index, string line, string path, int lineNumber, CopperfinLocalization localization)
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
            localization.Text("LanguageService.IntelliSense.Project.WorkAreaAliasDiscovered"));
    }

    private static void TryAddSqlExecCursorAlias(ProjectSymbolIndex index, string line, string path, int lineNumber, CopperfinLocalization localization)
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
            localization.Text("LanguageService.IntelliSense.Project.CursorAliasDiscovered"));
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
            var last = trimmed[trimmed.Length - 1];
            if ((trimmed[0] == '"' && last == '"') || (trimmed[0] == '\'' && last == '\''))
            {
                return trimmed.Substring(1, trimmed.Length - 2);
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

        var raw = line.Substring(startIndex, endIndex - startIndex);
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
                return ResolveExistingFilePath(includePath);
            }

            var sourceRelative = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(sourcePath) ?? string.Empty, includePath));
            var resolvedSourceRelative = ResolveExistingFilePath(sourceRelative);
            if (!string.IsNullOrWhiteSpace(resolvedSourceRelative))
            {
                return resolvedSourceRelative;
            }

            var rootRelative = Path.GetFullPath(Path.Combine(root, includePath));
            return ResolveExistingFilePath(rootRelative);
        }
        catch
        {
            return string.Empty;
        }
    }

    private static string ResolveExistingFilePath(string candidate)
    {
        var fullPath = Path.GetFullPath(candidate);
        if (File.Exists(fullPath))
        {
            return fullPath;
        }

        var pathRoot = Path.GetPathRoot(fullPath);
        if (string.IsNullOrEmpty(pathRoot))
        {
            return string.Empty;
        }

        var currentPath = pathRoot;
        var relativePath = fullPath.Substring(pathRoot.Length);
        var segments = relativePath.Split(
            new[] { Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar },
            StringSplitOptions.RemoveEmptyEntries);

        foreach (var segment in segments)
        {
            string? exactMatch = null;
            string? insensitiveMatch = null;
            var insensitiveMatchCount = 0;
            foreach (var entry in Directory.EnumerateFileSystemEntries(currentPath))
            {
                var entryName = Path.GetFileName(entry);
                if (string.Equals(entryName, segment, StringComparison.Ordinal))
                {
                    exactMatch = entry;
                    break;
                }

                if (string.Equals(entryName, segment, StringComparison.OrdinalIgnoreCase))
                {
                    insensitiveMatch = entry;
                    insensitiveMatchCount++;
                }
            }

            if (exactMatch is not null)
            {
                currentPath = exactMatch;
                continue;
            }

            if (insensitiveMatchCount != 1 || insensitiveMatch is null)
            {
                return string.Empty;
            }

            currentPath = insensitiveMatch;
        }

        return File.Exists(currentPath) ? Path.GetFullPath(currentPath) : string.Empty;
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

        var segments = token.Split(new[] { '.' }, StringSplitOptions.RemoveEmptyEntries);
        for (var segmentCount = segments.Length - 1; segmentCount > 0; segmentCount--)
        {
            var prefix = string.Join(".", segments.Take(segmentCount));
            if (TryResolveDefinition(index, prefix, out definition))
            {
                return true;
            }
        }

        var methodName = segments[segments.Length - 1];
        if (TryResolveUniqueProjectMethodDefinition(index, methodName, out definition))
        {
            return true;
        }

        return TryResolveDefinition(index, methodName, out definition);
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

        var segments = invocationName.Split(new[] { '.' }, StringSplitOptions.RemoveEmptyEntries);
        for (var segmentCount = segments.Length - 1; segmentCount > 0; segmentCount--)
        {
            var prefix = string.Join(".", segments.Take(segmentCount));
            if (index.Signatures.TryGetValue(prefix, out signatures))
            {
                return signatures;
            }
        }

        var methodName = segments[segments.Length - 1];
        if (TryResolveUniqueProjectMethodSignature(index, methodName, out signatures))
        {
            return signatures;
        }

        return index.Signatures.TryGetValue(methodName, out signatures)
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

    private static IReadOnlyList<FoxProSignatureEntry> LocalizeSignatures(
        IReadOnlyList<SignatureTemplate> templates,
        CopperfinLocalization localization)
    {
        return templates
            .Select(template => new FoxProSignatureEntry
            {
                Name = template.Name,
                Content = template.Content,
                Documentation = localization.Text(template.DocumentationKey),
                Parameters = template.Parameters.Select(parameter => new FoxProParameterEntry
                {
                    Name = parameter.Name,
                    Documentation = localization.Text(parameter.DocumentationKey)
                }).ToList()
            })
            .ToList();
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
        var candidate = separatorIndex >= 0 ? parameter.Substring(0, separatorIndex) : parameter;
        var asIndex = candidate.IndexOf(" AS ", StringComparison.OrdinalIgnoreCase);
        if (asIndex >= 0)
        {
            candidate = candidate.Substring(0, asIndex);
        }

        return candidate.Trim();
    }

    private static SignatureTemplate CreateSignatureTemplate(
        string name,
        string content,
        string documentationKey,
        params (string Name, string DocumentationKey)[] parameters)
    {
        return new SignatureTemplate
        {
            Name = name,
            Content = content,
            DocumentationKey = documentationKey,
            Parameters = parameters.Select(parameter => new SignatureParameterTemplate(parameter.Name, parameter.DocumentationKey)).ToList()
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

    private sealed class SignatureTemplate
    {
        public string Name { get; set; } = string.Empty;
        public string Content { get; set; } = string.Empty;
        public string DocumentationKey { get; set; } = string.Empty;
        public IReadOnlyList<SignatureParameterTemplate> Parameters { get; set; } = Array.Empty<SignatureParameterTemplate>();
    }

    private readonly struct SignatureParameterTemplate
    {
        public SignatureParameterTemplate(string name, string documentationKey)
        {
            Name = name;
            DocumentationKey = documentationKey;
        }

        public string Name { get; }

        public string DocumentationKey { get; }
    }

    private readonly struct InvocationArgument
    {
        public InvocationArgument(string text, int columnNumber)
        {
            Text = text;
            ColumnNumber = columnNumber;
        }

        public string Text { get; }

        public int ColumnNumber { get; }
    }
}

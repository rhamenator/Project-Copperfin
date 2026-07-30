// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace Copperfin.VisualStudio;

internal static class CopperfinProjectInsightClient
{
    private const RegexOptions ProjectRegexOptions =
        RegexOptions.IgnoreCase | RegexOptions.Compiled | RegexOptions.CultureInvariant;

    private static CopperfinLocalization Localization => CopperfinLocalization.FromVisualStudioUiCulture();

    private static readonly string[] TaskMarkers = { "TODO", "FIXME", "HACK", "BUG", "UNDONE" };
    private static readonly Regex ProcedureRegex = new(@"^\s*PROC(?:EDURE)?\s+([A-Za-z0-9_\.]+)", ProjectRegexOptions);
    private static readonly Regex FunctionRegex = new(@"^\s*FUNCTION\s+([A-Za-z0-9_\.]+)", ProjectRegexOptions);
    private static readonly Regex MethodProcedureRegex = new(@"^\s*(?:(?:PROTECTED|HIDDEN)\s+)?PROC(?:EDURE)?\s+([A-Za-z0-9_\.]+)", ProjectRegexOptions);
    private static readonly Regex MethodFunctionRegex = new(@"^\s*(?:(?:PROTECTED|HIDDEN)\s+)?FUNCTION\s+([A-Za-z0-9_\.]+)", ProjectRegexOptions);
    private static readonly Regex DefineClassRegex = new(@"^\s*DEFINE\s+CLASS\s+([A-Za-z0-9_\.]+)\s+AS\s+([A-Za-z0-9_\.]+)", ProjectRegexOptions);
    private static readonly Regex EndDefineRegex = new(@"^\s*ENDDEFINE\b", ProjectRegexOptions);
    private static readonly Regex EndRoutineRegex = new(@"^\s*END(?:PROC|FUNC)\b", ProjectRegexOptions);
    private static readonly Regex ClassPropertyRegex = new(@"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=", ProjectRegexOptions);
    private static readonly Regex DefineRegex = new(@"^\s*#DEFINE\s+([A-Za-z0-9_\.]+)", ProjectRegexOptions);
    private static readonly Regex DoFormRegex = new(@"^\s*DO\s+FORM\s+(.+)$", ProjectRegexOptions);
    private static readonly Regex ReportFormRegex = new(@"^\s*REPORT\s+FORM\s+(.+)$", ProjectRegexOptions);
    private static readonly Regex LabelFormRegex = new(@"^\s*LABEL\s+FORM\s+(.+)$", ProjectRegexOptions);
    private static readonly Regex DoRegex = new(@"^\s*DO\s+([A-Za-z0-9_\.]+)", ProjectRegexOptions);
    private static readonly Regex InvocationRegex = new(@"(?<![A-Za-z0-9_#])([A-Za-z_][A-Za-z0-9_\.]*)\s*\(", ProjectRegexOptions);
    private static readonly Regex DottedMemberRegex = new(@"(?<![A-Za-z0-9_#])([A-Za-z_][A-Za-z0-9_]*(?:\.[A-Za-z_][A-Za-z0-9_]*)+)(?![A-Za-z0-9_])", ProjectRegexOptions);
    private static readonly HashSet<string> TextExtensions = new(StringComparer.OrdinalIgnoreCase)
    {
        ".prg", ".h", ".hpp", ".ch", ".qpr", ".mpr", ".spr", ".ini", ".xml", ".txt"
    };

    public static CopperfinProjectInsights BuildInsights(CopperfinStudioSnapshotDocument snapshot)
    {
        var insights = new CopperfinProjectInsights();
        var workspace = snapshot.ProjectWorkspace;
        if (workspace is null)
        {
            insights.Warnings.Add(Localization.Text("AssetEditor.ProjectInsights.Warning.MetadataUnavailable"));
            return insights;
        }

        var loadedTextFiles = new Dictionary<string, string[]>(StringComparer.OrdinalIgnoreCase);
        var projectRoot = ResolveProjectRoot(snapshot);
        insights.ProjectRoot = projectRoot;
        foreach (var entry in workspace.Entries)
        {
            CollectWorkspaceArtifacts(entry, projectRoot, insights);

            var resolvedPath = ResolveEntryPath(projectRoot, entry);
            if (string.IsNullOrWhiteSpace(resolvedPath))
            {
                insights.Warnings.Add(Localization.Format("AssetEditor.ProjectInsights.Warning.ResolveProjectEntry", entry.Name));
                continue;
            }

            if (!File.Exists(resolvedPath))
            {
                insights.Warnings.Add(Localization.Format("AssetEditor.ProjectInsights.Warning.MissingProjectEntry", resolvedPath));
                continue;
            }

            if (!TextExtensions.Contains(Path.GetExtension(resolvedPath)))
            {
                continue;
            }

            var lines = TryReadTextFile(resolvedPath, insights);
            if (lines is null)
            {
                continue;
            }

            loadedTextFiles[resolvedPath] = lines;
            CollectTaskItemsAndDefinitions(resolvedPath, lines, insights);
        }

        var knownCallableSymbols = new HashSet<string>(
            insights.DefinedSymbols
                .Where(symbol => string.Equals(symbol.Kind, "procedure", StringComparison.OrdinalIgnoreCase) ||
                                 string.Equals(symbol.Kind, "function", StringComparison.OrdinalIgnoreCase) ||
                                 string.Equals(symbol.Kind, "method", StringComparison.OrdinalIgnoreCase))
                .Select(symbol => symbol.Name),
            StringComparer.OrdinalIgnoreCase);
        var knownMethodSymbols = knownCallableSymbols
            .Where(symbol => symbol.IndexOf('.') >= 0)
            .ToList();
        var knownPropertySymbols = insights.DefinedSymbols
            .Where(symbol => string.Equals(symbol.Kind, "property", StringComparison.OrdinalIgnoreCase))
            .Select(symbol => symbol.Name)
            .ToList();

        foreach (var loadedTextFile in loadedTextFiles)
        {
            CollectRuntimeReferences(
                loadedTextFile.Key,
                loadedTextFile.Value,
                knownCallableSymbols,
                knownMethodSymbols,
                knownPropertySymbols,
                insights);
        }

        return insights;
    }

    public static CopperfinProjectRenamePreview BuildRenamePreview(CopperfinStudioSnapshotDocument snapshot, string symbolName)
    {
        var insights = BuildInsights(snapshot);
        var preview = new CopperfinProjectRenamePreview
        {
            SymbolName = NormalizeRenameSymbol(symbolName, insights)
        };

        if (string.IsNullOrWhiteSpace(preview.SymbolName))
        {
            preview.Warnings.AddRange(insights.Warnings);
            return preview;
        }

        preview.Warnings.AddRange(insights.Warnings);

        foreach (var definition in insights.DefinedSymbols
                     .Where(symbol => string.Equals(symbol.Name, preview.SymbolName, StringComparison.OrdinalIgnoreCase))
                     .OrderBy(symbol => symbol.FilePath, StringComparer.OrdinalIgnoreCase)
                     .ThenBy(symbol => symbol.Line))
        {
            preview.Occurrences.Add(new CopperfinProjectRenameOccurrence
            {
                Kind = "definition",
                FilePath = definition.FilePath,
                Line = definition.Line,
                Detail = definition.Detail
            });
        }

        foreach (var reference in insights.RuntimeReferences
                     .Where(symbol => string.Equals(symbol.Name, preview.SymbolName, StringComparison.OrdinalIgnoreCase))
                     .OrderBy(symbol => symbol.FilePath, StringComparer.OrdinalIgnoreCase)
                     .ThenBy(symbol => symbol.Line))
        {
            preview.Occurrences.Add(new CopperfinProjectRenameOccurrence
            {
                Kind = "reference",
                FilePath = reference.FilePath,
                Line = reference.Line,
                Detail = reference.Detail
            });
        }

        return preview;
    }

    private static void CollectWorkspaceArtifacts(
        CopperfinStudioProjectEntry entry,
        string projectRoot,
        CopperfinProjectInsights insights)
    {
        var resolvedPath = ResolveEntryPath(projectRoot, entry);
        if (string.IsNullOrWhiteSpace(resolvedPath))
        {
            resolvedPath = entry.RelativePath;
        }

        if (entry.GroupId is "databases" or "tables" or "queries")
        {
            insights.DataAssets.Add(new CopperfinProjectDataAsset
            {
                Kind = entry.TypeTitle,
                Title = Path.GetFileName(string.IsNullOrWhiteSpace(entry.Name) ? resolvedPath : entry.Name),
                FilePath = resolvedPath,
                GroupTitle = entry.GroupTitle,
                Excluded = entry.Excluded
            });
        }

        if (entry.GroupId is "forms" or "classes" or "reports" or "labels" or "menus" or "programs")
        {
            insights.ObjectNodes.Add(new CopperfinProjectObjectNode
            {
                Kind = entry.TypeTitle,
                Title = Path.GetFileName(string.IsNullOrWhiteSpace(entry.Name) ? resolvedPath : entry.Name),
                FilePath = resolvedPath,
                GroupTitle = entry.GroupTitle,
                Excluded = entry.Excluded,
                Detail = entry.GroupTitle + (entry.Excluded ? Localization.Text("AssetEditor.Summary.ExcludedSuffix") : string.Empty)
            });
        }
    }

    private static string[]? TryReadTextFile(string path, CopperfinProjectInsights insights)
    {
        try
        {
            return File.ReadAllLines(path);
        }
        catch (IOException)
        {
            insights.Warnings.Add(Localization.Format("AssetEditor.ProjectInsights.Warning.ReadProjectFile", path));
            return null;
        }
        catch (UnauthorizedAccessException)
        {
            insights.Warnings.Add(Localization.Format("AssetEditor.ProjectInsights.Warning.ReadProjectFileAccessDenied", path));
            return null;
        }
    }

    private static void CollectTaskItemsAndDefinitions(string path, IReadOnlyList<string> lines, CopperfinProjectInsights insights)
    {
        string? currentClassName = null;
        var insideClassRoutine = false;
        for (var index = 0; index < lines.Count; index++)
        {
            var line = lines[index];
            var lineNumber = index + 1;
            CollectTaskItems(path, lineNumber, line, insights);
            CollectDefinitions(path, lineNumber, line, ref currentClassName, ref insideClassRoutine, insights);
        }
    }

    private static void CollectTaskItems(string path, int lineNumber, string line, CopperfinProjectInsights insights)
    {
        foreach (var marker in TaskMarkers)
        {
            var markerIndex = line.IndexOf(marker, StringComparison.OrdinalIgnoreCase);
            if (markerIndex < 0)
            {
                continue;
            }

            insights.TaskItems.Add(new CopperfinProjectTaskItem
            {
                Category = marker,
                FilePath = path,
                Line = lineNumber,
                Message = line.Trim()
            });
            return;
        }
    }

    private static void CollectDefinitions(
        string path,
        int lineNumber,
        string line,
        ref string? currentClassName,
        ref bool insideClassRoutine,
        CopperfinProjectInsights insights)
    {
        var classMatch = DefineClassRegex.Match(line);
        if (classMatch.Success)
        {
            currentClassName = classMatch.Groups[1].Value;
            insideClassRoutine = false;
            insights.DefinedSymbols.Add(new CopperfinProjectCodeSymbol
            {
                Kind = "class",
                Name = classMatch.Groups[1].Value,
                FilePath = path,
                Line = lineNumber,
                Detail = "AS " + classMatch.Groups[2].Value
            });

            insights.ObjectNodes.Add(new CopperfinProjectObjectNode
            {
                Kind = "Class",
                Title = classMatch.Groups[1].Value,
                FilePath = path,
                Detail = "AS " + classMatch.Groups[2].Value
            });

            return;
        }

        if (currentClassName is not null)
        {
            if (EndDefineRegex.IsMatch(line))
            {
                currentClassName = null;
                insideClassRoutine = false;
                return;
            }

            if (EndRoutineRegex.IsMatch(line))
            {
                insideClassRoutine = false;
                return;
            }

            if (AddScopedMethodDefinitionIfMatch(path, lineNumber, line, MethodProcedureRegex, currentClassName, insights) ||
                AddScopedMethodDefinitionIfMatch(path, lineNumber, line, MethodFunctionRegex, currentClassName, insights))
            {
                insideClassRoutine = true;
                return;
            }

            if (!insideClassRoutine && AddScopedPropertyDefinitionIfMatch(path, lineNumber, line, currentClassName, insights))
            {
                return;
            }
        }

        AddDefinitionIfMatch(path, lineNumber, line, ProcedureRegex, "procedure", insights);
        AddDefinitionIfMatch(path, lineNumber, line, FunctionRegex, "function", insights);
        AddDefinitionIfMatch(path, lineNumber, line, DefineRegex, "define", insights);
    }

    private static void CollectRuntimeReferences(
        string path,
        IReadOnlyList<string> lines,
        ISet<string> knownCallableSymbols,
        IReadOnlyList<string> knownMethodSymbols,
        IReadOnlyList<string> knownPropertySymbols,
        CopperfinProjectInsights insights)
    {
        for (var index = 0; index < lines.Count; index++)
        {
            var line = lines[index];
            var lineNumber = index + 1;

            AddReferenceIfMatch(path, lineNumber, line, DoFormRegex, "do form", insights);
            AddReferenceIfMatch(path, lineNumber, line, ReportFormRegex, "report form", insights);
            AddReferenceIfMatch(path, lineNumber, line, LabelFormRegex, "label form", insights);

            var doMatch = DoRegex.Match(line);
            if (doMatch.Success &&
                line.IndexOf("DO FORM", StringComparison.OrdinalIgnoreCase) < 0)
            {
                insights.RuntimeReferences.Add(new CopperfinProjectCodeSymbol
                {
                    Kind = "do",
                    Name = doMatch.Groups[1].Value,
                    FilePath = path,
                    Line = lineNumber,
                    Detail = line.Trim()
                });
            }

            CollectCallableReferences(path, lineNumber, line, knownCallableSymbols, knownMethodSymbols, insights);
            CollectPropertyReferences(path, lineNumber, line, knownPropertySymbols, insights);
        }
    }

    private static void CollectCallableReferences(
        string path,
        int lineNumber,
        string line,
        ISet<string> knownCallableSymbols,
        IReadOnlyList<string> knownMethodSymbols,
        CopperfinProjectInsights insights)
    {
        if (knownCallableSymbols.Count == 0 ||
            ProcedureRegex.IsMatch(line) ||
            FunctionRegex.IsMatch(line) ||
            DefineClassRegex.IsMatch(line) ||
            line.TrimStart().StartsWith("ENDDEFINE", StringComparison.OrdinalIgnoreCase))
        {
            return;
        }

        foreach (Match match in InvocationRegex.Matches(line))
        {
            var invocation = match.Groups[1].Value;
            var resolvedName = ResolveCallableReferenceName(invocation, knownCallableSymbols, knownMethodSymbols);
            if (string.IsNullOrWhiteSpace(resolvedName))
            {
                continue;
            }

            insights.RuntimeReferences.Add(new CopperfinProjectCodeSymbol
            {
                Kind = invocation.IndexOf('.') >= 0 ? "call.member" : "call",
                Name = resolvedName,
                FilePath = path,
                Line = lineNumber,
                Detail = line.Trim()
            });
        }
    }

    private static string ResolveCallableReferenceName(
        string invocation,
        ISet<string> knownCallableSymbols,
        IReadOnlyList<string> knownMethodSymbols)
    {
        if (knownCallableSymbols.Contains(invocation))
        {
            return invocation;
        }

        if (invocation.IndexOf('.') < 0)
        {
            return string.Empty;
        }

        var memberName = invocation.Split('.').Last();
        if (knownCallableSymbols.Contains(memberName))
        {
            return memberName;
        }

        return TryResolveUniqueProjectMethodName(memberName, knownMethodSymbols, out var resolvedMethodName)
            ? resolvedMethodName
            : string.Empty;
    }

    private static void CollectPropertyReferences(
        string path,
        int lineNumber,
        string line,
        IReadOnlyList<string> knownPropertySymbols,
        CopperfinProjectInsights insights)
    {
        if (line.TrimStart().StartsWith("*", StringComparison.Ordinal))
        {
            return;
        }

        foreach (Match match in DottedMemberRegex.Matches(line))
        {
            if (IsInsideCommentOrString(line, match.Index))
            {
                continue;
            }

            var suffix = line.Substring(match.Index + match.Length).TrimStart();
            if (suffix.StartsWith("(", StringComparison.Ordinal))
            {
                continue;
            }

            var reference = match.Groups[1].Value;
            var resolvedName = ResolvePropertyReferenceName(reference, knownPropertySymbols);
            if (string.IsNullOrWhiteSpace(resolvedName))
            {
                continue;
            }

            insights.RuntimeReferences.Add(new CopperfinProjectCodeSymbol
            {
                Kind = "member.property",
                Name = resolvedName,
                FilePath = path,
                Line = lineNumber,
                Detail = line.Trim()
            });
        }
    }

    private static bool IsInsideCommentOrString(string line, int position)
    {
        var quote = '\0';
        var bracketString = false;
        for (var index = 0; index < position; index++)
        {
            var value = line[index];
            if (quote != '\0')
            {
                if (value == quote)
                {
                    if (index + 1 < position && line[index + 1] == quote)
                    {
                        index++;
                    }
                    else
                    {
                        quote = '\0';
                    }
                }
                continue;
            }

            if (bracketString)
            {
                if (value == ']')
                {
                    if (index + 1 < position && line[index + 1] == ']')
                    {
                        index++;
                    }
                    else
                    {
                        bracketString = false;
                    }
                }
                continue;
            }

            if (value == '\'' || value == '"')
            {
                quote = value;
            }
            else if (value == '[' && !IsBracketSubscriptStart(line, index))
            {
                bracketString = true;
            }
            else if (value == '&' && index + 1 < position && line[index + 1] == '&')
            {
                return true;
            }
        }

        return quote != '\0' || bracketString;
    }

    private static bool IsBracketSubscriptStart(string line, int index)
    {
        if (index <= 0)
        {
            return false;
        }

        var previous = line[index - 1];
        return (previous >= 'A' && previous <= 'Z') ||
               (previous >= 'a' && previous <= 'z') ||
               (previous >= '0' && previous <= '9') ||
               previous == '_' ||
               previous == ')' ||
               previous == ']';
    }

    private static string ResolvePropertyReferenceName(string reference, IReadOnlyList<string> knownPropertySymbols)
    {
        var exact = knownPropertySymbols.FirstOrDefault(
            property => string.Equals(property, reference, StringComparison.OrdinalIgnoreCase));
        if (!string.IsNullOrWhiteSpace(exact))
        {
            return exact;
        }

        var propertyName = reference.Split('.').Last();
        return TryResolveUniqueProjectPropertyName(propertyName, knownPropertySymbols, out var resolvedPropertyName)
            ? resolvedPropertyName
            : string.Empty;
    }

    private static string NormalizeRenameSymbol(string symbolName, CopperfinProjectInsights insights)
    {
        var trimmed = symbolName.Trim();
        if (string.IsNullOrWhiteSpace(trimmed))
        {
            return string.Empty;
        }

        if (insights.DefinedSymbols.Exists(symbol => string.Equals(symbol.Name, trimmed, StringComparison.OrdinalIgnoreCase)))
        {
            return trimmed;
        }

        var knownCallableSymbols = new HashSet<string>(
            insights.DefinedSymbols
                .Where(symbol => string.Equals(symbol.Kind, "procedure", StringComparison.OrdinalIgnoreCase) ||
                                 string.Equals(symbol.Kind, "function", StringComparison.OrdinalIgnoreCase) ||
                                 string.Equals(symbol.Kind, "method", StringComparison.OrdinalIgnoreCase))
                .Select(symbol => symbol.Name),
            StringComparer.OrdinalIgnoreCase);
        var knownMethodSymbols = knownCallableSymbols
            .Where(symbol => symbol.IndexOf('.') >= 0)
            .ToList();
        var resolvedName = ResolveCallableReferenceName(trimmed, knownCallableSymbols, knownMethodSymbols);
        if (!string.IsNullOrWhiteSpace(resolvedName))
        {
            return resolvedName;
        }

        var knownPropertySymbols = insights.DefinedSymbols
            .Where(symbol => string.Equals(symbol.Kind, "property", StringComparison.OrdinalIgnoreCase))
            .Select(symbol => symbol.Name)
            .ToList();
        resolvedName = ResolvePropertyReferenceName(trimmed, knownPropertySymbols);
        if (!string.IsNullOrWhiteSpace(resolvedName))
        {
            return resolvedName;
        }

        return trimmed.IndexOf('.') >= 0
            ? trimmed.Split('.').Last()
            : trimmed;
    }

    private static void AddDefinitionIfMatch(string path, int lineNumber, string line, Regex regex, string kind, CopperfinProjectInsights insights)
    {
        var match = regex.Match(line);
        if (!match.Success)
        {
            return;
        }

        insights.DefinedSymbols.Add(new CopperfinProjectCodeSymbol
        {
            Kind = kind,
            Name = match.Groups[1].Value,
            FilePath = path,
            Line = lineNumber,
            Detail = line.Trim()
        });

        insights.ObjectNodes.Add(new CopperfinProjectObjectNode
        {
            Kind = kind,
            Title = match.Groups[1].Value,
            FilePath = path,
            Detail = line.Trim()
        });
    }

    private static bool AddScopedMethodDefinitionIfMatch(
        string path,
        int lineNumber,
        string line,
        Regex regex,
        string currentClassName,
        CopperfinProjectInsights insights)
    {
        var match = regex.Match(line);
        if (!match.Success)
        {
            return false;
        }

        var qualifiedMethodName = currentClassName + "." + match.Groups[1].Value;
        insights.DefinedSymbols.Add(new CopperfinProjectCodeSymbol
        {
            Kind = "method",
            Name = qualifiedMethodName,
            FilePath = path,
            Line = lineNumber,
            Detail = line.Trim()
        });

        insights.ObjectNodes.Add(new CopperfinProjectObjectNode
        {
            Kind = "method",
            Title = qualifiedMethodName,
            FilePath = path,
            Detail = line.Trim()
        });

        return true;
    }

    private static bool AddScopedPropertyDefinitionIfMatch(
        string path,
        int lineNumber,
        string line,
        string currentClassName,
        CopperfinProjectInsights insights)
    {
        var match = ClassPropertyRegex.Match(line);
        if (!match.Success)
        {
            return false;
        }

        var qualifiedPropertyName = currentClassName + "." + match.Groups[1].Value;
        insights.DefinedSymbols.Add(new CopperfinProjectCodeSymbol
        {
            Kind = "property",
            Name = qualifiedPropertyName,
            FilePath = path,
            Line = lineNumber,
            Detail = line.Trim()
        });

        insights.ObjectNodes.Add(new CopperfinProjectObjectNode
        {
            Kind = "property",
            Title = qualifiedPropertyName,
            FilePath = path,
            Detail = line.Trim()
        });

        return true;
    }

    private static void AddReferenceIfMatch(string path, int lineNumber, string line, Regex regex, string kind, CopperfinProjectInsights insights)
    {
        var match = regex.Match(line);
        if (!match.Success)
        {
            return;
        }

        insights.RuntimeReferences.Add(new CopperfinProjectCodeSymbol
        {
            Kind = kind,
            Name = match.Groups[1].Value.Trim(),
            FilePath = path,
            Line = lineNumber,
            Detail = line.Trim()
        });
    }

    private static bool TryResolveUniqueProjectMethodName(
        string methodName,
        IReadOnlyList<string> knownMethodSymbols,
        out string resolvedMethodName)
    {
        resolvedMethodName = string.Empty;
        var matches = knownMethodSymbols
            .Where(symbol => string.Equals(symbol.Split('.').Last(), methodName, StringComparison.OrdinalIgnoreCase))
            .Take(2)
            .ToList();
        if (matches.Count != 1)
        {
            return false;
        }

        resolvedMethodName = matches[0];
        return true;
    }

    private static bool TryResolveUniqueProjectPropertyName(
        string propertyName,
        IReadOnlyList<string> knownPropertySymbols,
        out string resolvedPropertyName)
    {
        resolvedPropertyName = string.Empty;
        var matches = knownPropertySymbols
            .Where(symbol => string.Equals(symbol.Split('.').Last(), propertyName, StringComparison.OrdinalIgnoreCase))
            .Take(2)
            .ToList();
        if (matches.Count != 1)
        {
            return false;
        }

        resolvedPropertyName = matches[0];
        return true;
    }

    private static string ResolveProjectRoot(CopperfinStudioSnapshotDocument snapshot)
    {
        var projectDirectory = Path.GetDirectoryName(snapshot.Path) ?? string.Empty;
        var homeDirectory = snapshot.ProjectWorkspace?.HomeDirectory;
        if (string.IsNullOrWhiteSpace(homeDirectory))
        {
            return projectDirectory;
        }

        if (Path.IsPathRooted(homeDirectory) && Directory.Exists(homeDirectory))
        {
            return homeDirectory!;
        }

        var combined = Path.GetFullPath(Path.Combine(projectDirectory, homeDirectory));
        return Directory.Exists(combined) ? combined : projectDirectory;
    }

    private static string ResolveEntryPath(string projectRoot, CopperfinStudioProjectEntry entry)
    {
        if (!string.IsNullOrWhiteSpace(entry.Name) && Path.IsPathRooted(entry.Name))
        {
            return string.Empty;
        }

        if (!string.IsNullOrWhiteSpace(entry.Name) &&
            CopperfinDocumentPathIdentity.LooksWindowsRooted(entry.Name))
        {
            return string.Empty;
        }

        if (!string.IsNullOrWhiteSpace(entry.RelativePath))
        {
            var relativePath = entry.RelativePath.Trim()
                .Replace('\\', Path.DirectorySeparatorChar)
                .Replace('/', Path.DirectorySeparatorChar);
            if (Path.IsPathRooted(relativePath) ||
                CopperfinDocumentPathIdentity.LooksWindowsRooted(relativePath))
            {
                return string.Empty;
            }

            try
            {
                var candidate = Path.GetFullPath(Path.Combine(projectRoot, relativePath));
                return CopperfinDocumentPathIdentity.IsWithinRoot(projectRoot, candidate)
                    ? candidate
                    : string.Empty;
            }
            catch (ArgumentException)
            {
                return string.Empty;
            }
            catch (NotSupportedException)
            {
                return string.Empty;
            }
        }

        if (!string.IsNullOrWhiteSpace(entry.Name))
        {
            try
            {
                var candidate = Path.GetFullPath(Path.Combine(projectRoot, Path.GetFileName(entry.Name) ?? string.Empty));
                return CopperfinDocumentPathIdentity.IsWithinRoot(projectRoot, candidate)
                    ? candidate
                    : string.Empty;
            }
            catch (ArgumentException)
            {
                return string.Empty;
            }
            catch (NotSupportedException)
            {
                return string.Empty;
            }
        }

        return string.Empty;
    }
}

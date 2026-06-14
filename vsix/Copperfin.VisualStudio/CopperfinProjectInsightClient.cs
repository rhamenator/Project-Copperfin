using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace Copperfin.VisualStudio;

internal static class CopperfinProjectInsightClient
{
    private static readonly string[] TaskMarkers = { "TODO", "FIXME", "HACK", "BUG", "UNDONE" };
    private static readonly Regex ProcedureRegex = new(@"^\s*PROCEDURE\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex FunctionRegex = new(@"^\s*FUNCTION\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex DefineClassRegex = new(@"^\s*DEFINE\s+CLASS\s+([A-Za-z0-9_\.]+)\s+AS\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex DefineRegex = new(@"^\s*#DEFINE\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex DoFormRegex = new(@"^\s*DO\s+FORM\s+(.+)$", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex ReportFormRegex = new(@"^\s*REPORT\s+FORM\s+(.+)$", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex LabelFormRegex = new(@"^\s*LABEL\s+FORM\s+(.+)$", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex DoRegex = new(@"^\s*DO\s+([A-Za-z0-9_\.]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    private static readonly Regex InvocationRegex = new(@"(?<![A-Za-z0-9_#])([A-Za-z_][A-Za-z0-9_\.]*)\s*\(", RegexOptions.IgnoreCase | RegexOptions.Compiled);
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
            insights.Warnings.Add("Project workspace metadata is unavailable.");
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
                insights.Warnings.Add($"Could not resolve project entry: {entry.Name}");
                continue;
            }

            if (!File.Exists(resolvedPath))
            {
                insights.Warnings.Add($"Missing project entry on disk: {resolvedPath}");
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
                                 string.Equals(symbol.Kind, "function", StringComparison.OrdinalIgnoreCase))
                .Select(symbol => symbol.Name),
            StringComparer.OrdinalIgnoreCase);

        foreach (var loadedTextFile in loadedTextFiles)
        {
            CollectRuntimeReferences(loadedTextFile.Key, loadedTextFile.Value, knownCallableSymbols, insights);
        }

        return insights;
    }

    public static CopperfinProjectRenamePreview BuildRenamePreview(CopperfinStudioSnapshotDocument snapshot, string symbolName)
    {
        var preview = new CopperfinProjectRenamePreview
        {
            SymbolName = NormalizeRenameSymbol(symbolName)
        };

        if (string.IsNullOrWhiteSpace(preview.SymbolName))
        {
            return preview;
        }

        var insights = BuildInsights(snapshot);
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
                Detail = entry.GroupTitle + (entry.Excluded ? " (excluded)" : string.Empty)
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
            insights.Warnings.Add($"Could not read project file: {path}");
            return null;
        }
        catch (UnauthorizedAccessException)
        {
            insights.Warnings.Add($"Access denied while reading project file: {path}");
            return null;
        }
    }

    private static void CollectTaskItemsAndDefinitions(string path, IReadOnlyList<string> lines, CopperfinProjectInsights insights)
    {
        for (var index = 0; index < lines.Count; index++)
        {
            var line = lines[index];
            var lineNumber = index + 1;
            CollectTaskItems(path, lineNumber, line, insights);
            CollectDefinitions(path, lineNumber, line, insights);
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

    private static void CollectDefinitions(string path, int lineNumber, string line, CopperfinProjectInsights insights)
    {
        AddDefinitionIfMatch(path, lineNumber, line, ProcedureRegex, "procedure", insights);
        AddDefinitionIfMatch(path, lineNumber, line, FunctionRegex, "function", insights);
        AddDefinitionIfMatch(path, lineNumber, line, DefineRegex, "define", insights);

        var classMatch = DefineClassRegex.Match(line);
        if (classMatch.Success)
        {
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
        }
    }

    private static void CollectRuntimeReferences(
        string path,
        IReadOnlyList<string> lines,
        ISet<string> knownCallableSymbols,
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

            CollectCallableReferences(path, lineNumber, line, knownCallableSymbols, insights);
        }
    }

    private static void CollectCallableReferences(
        string path,
        int lineNumber,
        string line,
        ISet<string> knownCallableSymbols,
        CopperfinProjectInsights insights)
    {
        if (knownCallableSymbols.Count == 0 ||
            ProcedureRegex.IsMatch(line) ||
            FunctionRegex.IsMatch(line) ||
            DefineClassRegex.IsMatch(line))
        {
            return;
        }

        foreach (Match match in InvocationRegex.Matches(line))
        {
            var invocation = match.Groups[1].Value;
            var resolvedName = ResolveCallableReferenceName(invocation, knownCallableSymbols);
            if (string.IsNullOrWhiteSpace(resolvedName))
            {
                continue;
            }

            insights.RuntimeReferences.Add(new CopperfinProjectCodeSymbol
            {
                Kind = invocation.Contains('.') ? "call.member" : "call",
                Name = resolvedName,
                FilePath = path,
                Line = lineNumber,
                Detail = line.Trim()
            });
        }
    }

    private static string ResolveCallableReferenceName(string invocation, ISet<string> knownCallableSymbols)
    {
        if (knownCallableSymbols.Contains(invocation))
        {
            return invocation;
        }

        if (!invocation.Contains('.'))
        {
            return string.Empty;
        }

        var memberName = invocation.Split('.').Last();
        return knownCallableSymbols.Contains(memberName) ? memberName : string.Empty;
    }

    private static string NormalizeRenameSymbol(string symbolName)
    {
        var trimmed = symbolName.Trim();
        if (string.IsNullOrWhiteSpace(trimmed))
        {
            return string.Empty;
        }

        return trimmed.Contains('.')
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
            return entry.Name;
        }

        if (!string.IsNullOrWhiteSpace(entry.RelativePath))
        {
            return Path.GetFullPath(Path.Combine(projectRoot, entry.RelativePath));
        }

        if (!string.IsNullOrWhiteSpace(entry.Name))
        {
            return Path.GetFullPath(Path.Combine(projectRoot, Path.GetFileName(entry.Name) ?? string.Empty));
        }

        return string.Empty;
    }
}

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Collections.Generic;
using System.IO;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinStudioStartupDocument
{
    public string Path { get; set; } = string.Empty;
    public string? ObjectName { get; set; }
    public string? UniqueId { get; set; }
}

internal static class CopperfinStudioStartupArguments
{
    public static bool TryParse(
        string[] args,
        CopperfinLocalization localization,
        out IReadOnlyList<CopperfinStudioStartupDocument> documents,
        out string? error)
    {
        var paths = new List<string>();
        string? objectName = null;
        string? uniqueId = null;
        var objectNameProvided = false;
        var uniqueIdProvided = false;

        for (var index = 0; index < args.Length; ++index)
        {
            var argument = args[index];
            if (string.Equals(argument, "--locale", StringComparison.OrdinalIgnoreCase))
            {
                if (!TryReadValue(args, ref index, argument, localization, out _ , out error))
                {
                    documents = Array.Empty<CopperfinStudioStartupDocument>();
                    return false;
                }

                continue;
            }

            if (string.Equals(argument, "--path", StringComparison.OrdinalIgnoreCase))
            {
                if (!TryReadValue(args, ref index, argument, localization, out var path, out error))
                {
                    documents = Array.Empty<CopperfinStudioStartupDocument>();
                    return false;
                }

                paths.Add(path!);
                continue;
            }

            if (string.Equals(argument, "--object-name", StringComparison.OrdinalIgnoreCase) ||
                string.Equals(argument, "--unique-id", StringComparison.OrdinalIgnoreCase))
            {
                var isObjectName = string.Equals(argument, "--object-name", StringComparison.OrdinalIgnoreCase);
                if ((isObjectName && objectNameProvided) || (!isObjectName && uniqueIdProvided))
                {
                    documents = Array.Empty<CopperfinStudioStartupDocument>();
                    error = localization.Format("Studio.Startup.DuplicateSelector", argument);
                    return false;
                }

                if (!TryReadValue(args, ref index, argument, localization, out var value, out error))
                {
                    documents = Array.Empty<CopperfinStudioStartupDocument>();
                    return false;
                }

                if (isObjectName)
                {
                    objectNameProvided = true;
                    objectName = value;
                }
                else
                {
                    uniqueIdProvided = true;
                    uniqueId = value;
                }

                continue;
            }

            if (argument.StartsWith("--", StringComparison.Ordinal))
            {
                documents = Array.Empty<CopperfinStudioStartupDocument>();
                error = localization.Format("Studio.Startup.UnknownArgument", argument);
                return false;
            }

            if (!string.IsNullOrWhiteSpace(argument))
            {
                paths.Add(argument);
            }
        }

        var selectorProvided = objectNameProvided || uniqueIdProvided;
        if (!selectorProvided)
        {
            documents = CreateDocuments(paths);
            error = null;
            return true;
        }

        if (paths.Count != 1)
        {
            documents = Array.Empty<CopperfinStudioStartupDocument>();
            error = localization.Text("Studio.Startup.SelectorRequiresSingleAsset");
            return false;
        }

        var extension = System.IO.Path.GetExtension(paths[0]);
        if (!string.Equals(extension, ".scx", StringComparison.OrdinalIgnoreCase) &&
            !string.Equals(extension, ".vcx", StringComparison.OrdinalIgnoreCase))
        {
            documents = Array.Empty<CopperfinStudioStartupDocument>();
            error = localization.Text("Studio.Startup.SelectorUnsupportedAsset");
            return false;
        }

        documents = new[]
        {
            new CopperfinStudioStartupDocument
            {
                Path = paths[0],
                ObjectName = objectName,
                UniqueId = uniqueId
            }
        };
        error = null;
        return true;
    }

    private static IReadOnlyList<CopperfinStudioStartupDocument> CreateDocuments(IReadOnlyList<string> paths)
    {
        var documents = new List<CopperfinStudioStartupDocument>(paths.Count);
        foreach (var path in paths)
        {
            documents.Add(new CopperfinStudioStartupDocument { Path = path });
        }

        return documents;
    }

    private static bool TryReadValue(
        IReadOnlyList<string> args,
        ref int index,
        string option,
        CopperfinLocalization localization,
        out string? value,
        out string? error)
    {
        if (index + 1 >= args.Count || string.IsNullOrWhiteSpace(args[index + 1]))
        {
            value = null;
            error = localization.Format("Studio.Startup.MissingArgument", option);
            return false;
        }

        value = args[++index];
        error = null;
        return true;
    }
}

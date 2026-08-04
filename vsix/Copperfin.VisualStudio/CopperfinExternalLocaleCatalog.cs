// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Xml;

namespace Copperfin.VisualStudio;

internal static class CopperfinExternalLocaleCatalog
{
    public static IReadOnlyDictionary<string, string>? Load(string locale, string? configuredRoot)
    {
        return LoadFromBaseDirectory(locale, configuredRoot, AppContext.BaseDirectory);
    }

    internal static IReadOnlyDictionary<string, string>? LoadFromBaseDirectory(
        string locale,
        string? configuredRoot,
        string baseDirectory)
    {
        foreach (var root in CandidateRoots(configuredRoot, baseDirectory))
        {
            var catalogPath = FindCatalogPath(root, locale);
            if (catalogPath is null)
            {
                continue;
            }

            try
            {
                using var stream = File.OpenRead(catalogPath);
                var serializer = new DataContractJsonSerializer(
                    typeof(Dictionary<string, string>),
                    new DataContractJsonSerializerSettings { UseSimpleDictionaryFormat = true });
                if (serializer.ReadObject(stream) is Dictionary<string, string> catalog)
                {
                    return new Dictionary<string, string>(catalog, StringComparer.Ordinal);
                }
            }
            catch (IOException)
            {
                // A missing or unreadable optional catalog uses the compiled fallback.
            }
            catch (UnauthorizedAccessException)
            {
                // A missing or unreadable optional catalog uses the compiled fallback.
            }
            catch (SerializationException)
            {
                // A malformed optional catalog uses the compiled fallback.
            }
            catch (InvalidDataContractException)
            {
                // A malformed optional catalog uses the compiled fallback.
            }
            catch (XmlException)
            {
                // A malformed optional catalog uses the compiled fallback.
            }
        }

        return null;
    }

    private static IEnumerable<string> CandidateRoots(string? configuredRoot, string baseDirectory)
    {
        if (configuredRoot is { } root && !string.IsNullOrWhiteSpace(root))
        {
            yield return root.Trim();
            yield break;
        }

        var executableDirectory = new DirectoryInfo(baseDirectory);
        yield return Path.Combine(executableDirectory.FullName, "..", "share", "copperfin", "locales");
        yield return Path.Combine(executableDirectory.FullName, "share", "copperfin", "locales");
        yield return Path.Combine(executableDirectory.FullName, "..", "resources", "locales");
        yield return Path.Combine(executableDirectory.FullName, "..", "..", "resources", "locales");
        yield return Path.Combine(executableDirectory.FullName, "..", "..", "share", "copperfin", "locales");

        var ancestor = new DirectoryInfo(Environment.CurrentDirectory);
        while (ancestor is not null)
        {
            yield return Path.Combine(ancestor.FullName, "resources", "locales");
            ancestor = ancestor.Parent;
        }
    }

    private static string? FindCatalogPath(string root, string locale)
    {
        var localeDirectory = string.Equals(locale, CopperfinLocalization.DefaultLocale, StringComparison.OrdinalIgnoreCase)
            ? "en-US"
            : locale;
        var directPath = Path.Combine(root, "strings.json");
        if (Directory.Exists(root) &&
            string.Equals(new DirectoryInfo(root).Name, localeDirectory, StringComparison.OrdinalIgnoreCase) &&
            File.Exists(directPath))
        {
            return directPath;
        }

        var nestedPath = Path.Combine(root, localeDirectory, "strings.json");
        return File.Exists(nestedPath) ? nestedPath : null;
    }
}

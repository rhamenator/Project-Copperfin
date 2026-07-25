using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using Copperfin.VisualStudio;

if (args.Length != 1 && args.Length != 2)
{
    Console.Error.WriteLine("Usage: Copperfin.LocalizationCatalogGenerator <repository-root> [--check]");
    return 2;
}

var repositoryRoot = Path.GetFullPath(args[0]);
var checkOnly = args.Length == 2 && string.Equals(args[1], "--check", StringComparison.Ordinal);
var locales = new Dictionary<string, string>(StringComparer.Ordinal)
{
    ["en-US"] = CopperfinLocalization.DefaultLocale,
    ["es-419"] = CopperfinLocalization.SpanishLatinAmericaLocale,
    ["pt-BR"] = CopperfinLocalization.PortugueseBrazilLocale,
    ["qps-ploc"] = CopperfinLocalization.PseudoLocale
};

var options = new JsonSerializerOptions { WriteIndented = true };
var managedCatalogSourcePath = Path.Combine(
    repositoryRoot,
    "vsix",
    "Copperfin.VisualStudio",
    "CopperfinLocalization.cs");
var managedCatalogSection = string.Empty;
var managedCatalogKeys = new Dictionary<string, HashSet<string>>(StringComparer.Ordinal)
{
    [CopperfinLocalization.DefaultLocale] = new HashSet<string>(StringComparer.Ordinal),
    [CopperfinLocalization.SpanishLatinAmericaLocale] = new HashSet<string>(StringComparer.Ordinal),
    [CopperfinLocalization.PortugueseBrazilLocale] = new HashSet<string>(StringComparer.Ordinal)
};
var managedKeyPattern = new Regex("^\\s*\\[\\\"(?<key>[^\\\"]+)\\\"\\]\\s*=", RegexOptions.Compiled);
foreach (var line in File.ReadLines(managedCatalogSourcePath))
{
    if (line.Contains("[DefaultLocale] = new Dictionary", StringComparison.Ordinal))
    {
        managedCatalogSection = CopperfinLocalization.DefaultLocale;
    }
    else if (line.Contains("[SpanishLatinAmericaLocale] = new Dictionary", StringComparison.Ordinal))
    {
        managedCatalogSection = CopperfinLocalization.SpanishLatinAmericaLocale;
    }
    else if (line.Contains("[PortugueseBrazilLocale] = new Dictionary", StringComparison.Ordinal))
    {
        managedCatalogSection = CopperfinLocalization.PortugueseBrazilLocale;
    }

    var match = managedKeyPattern.Match(line);
    if (match.Success &&
        managedCatalogKeys.TryGetValue(managedCatalogSection, out var keys) &&
        !keys.Add(match.Groups["key"].Value))
    {
        throw new InvalidDataException(
            $"Managed catalog contains a duplicate key: {managedCatalogSection}/{match.Groups["key"].Value}");
    }
}

foreach (var locale in locales)
{
    var isPseudoLocale = string.Equals(locale.Key, "qps-ploc", StringComparison.Ordinal);
    var catalogPath = Path.Combine(repositoryRoot, "resources", "locales", locale.Key, "strings.json");
    var catalogText = File.ReadAllText(catalogPath);
    var existingValues = new Dictionary<string, string>(StringComparer.Ordinal);
    var seenKeys = new HashSet<string>(StringComparer.Ordinal);
    using (var document = JsonDocument.Parse(catalogText))
    {
        if (document.RootElement.ValueKind != JsonValueKind.Object)
        {
            throw new InvalidDataException($"Catalog root must be an object: {catalogPath}");
        }

        foreach (var property in document.RootElement.EnumerateObject())
        {
            if (!seenKeys.Add(property.Name))
            {
                throw new InvalidDataException($"Catalog contains a duplicate key: {locale.Key}/{property.Name}");
            }

            if (property.Value.ValueKind != JsonValueKind.String)
            {
                throw new InvalidDataException($"Catalog value must be a string: {locale.Key}/{property.Name}");
            }

            existingValues[property.Name] = property.Value.GetString()!;
        }
    }
    var managedEntries = CopperfinLocalization.CatalogEntries(locale.Value);
    var missingEntries = new List<KeyValuePair<string, string>>();

    foreach (var entry in managedEntries)
    {
        if (existingValues.TryGetValue(entry.Key, out var existingValue))
        {
            if ((!isPseudoLocale && !string.Equals(existingValue, entry.Value, StringComparison.Ordinal)) ||
                (isPseudoLocale && string.IsNullOrWhiteSpace(existingValue)))
            {
                throw new InvalidDataException($"Catalog value drift for {locale.Key}/{entry.Key}");
            }

            continue;
        }

        if (checkOnly)
        {
            throw new InvalidDataException($"Catalog is missing managed key {locale.Key}/{entry.Key}");
        }

        missingEntries.Add(isPseudoLocale
            ? new KeyValuePair<string, string>(entry.Key, $"[{entry.Value}]")
            : entry);
    }

    if (missingEntries.Count > 0)
    {
        var closingBrace = catalogText.LastIndexOf('}');
        if (closingBrace < 0)
        {
            throw new InvalidDataException($"Catalog is missing its closing brace: {catalogPath}");
        }

        var prefix = catalogText[..closingBrace].TrimEnd();
        var separator = prefix.EndsWith("{", StringComparison.Ordinal) ? "\n" : ",\n";
        var additions = string.Join(
            ",\n",
            missingEntries.Select(entry =>
                $"  {JsonSerializer.Serialize(entry.Key, options)}: {JsonSerializer.Serialize(entry.Value, options)}"));
        File.WriteAllText(
            catalogPath,
            prefix + separator + additions + "\n}\n",
            new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
    }
}

Console.WriteLine(checkOnly
    ? "Managed localization catalog parity passed."
    : "Managed localization entries synchronized into shared catalogs.");
return 0;

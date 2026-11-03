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
var localeRoot = Path.Combine(repositoryRoot, "resources", "locales");
var inventoryLines = File.ReadAllLines(Path.Combine(localeRoot, "inventory.txt"));
if (inventoryLines.Length < 3 || inventoryLines[0] != "version 1")
{
    throw new InvalidDataException("Locale inventory requires version 1 and declared catalogs.");
}
var locales = new Dictionary<string, string>(StringComparer.Ordinal);
var inventoryPattern = new Regex("^(?<locale>[a-z][A-Za-z0-9-]*) (?<role>source|unreviewed|pseudo)$");
foreach (var line in inventoryLines.Skip(1))
{
    var match = inventoryPattern.Match(line);
    if (!match.Success)
    {
        throw new InvalidDataException($"Malformed locale inventory entry: {line}");
    }
    var locale = match.Groups["locale"].Value;
    var role = match.Groups["role"].Value;
    if (locale.Contains("--", StringComparison.Ordinal) || locale.EndsWith("-", StringComparison.Ordinal) ||
        !locales.TryAdd(locale, role))
    {
        throw new InvalidDataException($"Invalid or duplicate locale inventory entry: {locale}");
    }
    if ((role == "source" && locale != "en-US") || (role == "pseudo" && locale != "qps-ploc"))
    {
        throw new InvalidDataException($"Unexpected source or pseudo locale: {locale}");
    }
}
if (locales.Count(entry => entry.Value == "source") != 1 ||
    locales.Count(entry => entry.Value == "pseudo") != 1 ||
    !locales.ContainsKey("en-US") || !locales.ContainsKey("qps-ploc"))
{
    throw new InvalidDataException("Locale inventory requires one en-US source and one qps-ploc pseudo catalog.");
}
var catalogDirectories = Directory.EnumerateDirectories(localeRoot)
    .Select(Path.GetFileName).ToHashSet(StringComparer.Ordinal);
if (!catalogDirectories.SetEquals(locales.Keys))
{
    throw new InvalidDataException("Locale directories do not match the declared inventory.");
}
var strictUtf8 = new UTF8Encoding(encoderShouldEmitUTF8Identifier: false, throwOnInvalidBytes: true);
var sourceEntries = JsonSerializer.Deserialize<Dictionary<string, string>>(
    File.ReadAllText(Path.Combine(localeRoot, "en-US", "strings.json"), strictUtf8))
    ?? throw new InvalidDataException("Source catalog is missing or malformed.");
var placeholderPattern = new Regex("\\{(?<name>[^{}]+)\\}");
string[] Placeholders(string value) => placeholderPattern.Matches(value)
    .Select(match => match.Groups["name"].Value).OrderBy(name => name, StringComparer.Ordinal).ToArray();

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
    var catalogText = File.ReadAllText(catalogPath, strictUtf8);
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
    var managedLocale = locale.Key switch
    {
        "en-US" => CopperfinLocalization.DefaultLocale,
        "es-419" => CopperfinLocalization.SpanishLatinAmericaLocale,
        "pt-BR" => CopperfinLocalization.PortugueseBrazilLocale,
        "qps-ploc" => CopperfinLocalization.PseudoLocale,
        _ => null
    };
    if (managedLocale is null && !checkOnly)
    {
        throw new InvalidDataException(
            $"Generating an unmanaged locale catalog is unsupported: {locale.Key}; use --check to validate it");
    }
    var managedEntries = managedLocale is null
        ? new Dictionary<string, string>(StringComparer.Ordinal)
        : CopperfinLocalization.CatalogEntries(managedLocale);
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

    if (checkOnly)
    {
        if (existingValues.Count != sourceEntries.Count)
        {
            throw new InvalidDataException($"Catalog key count differs from en-US: {locale.Key}");
        }
        foreach (var source in sourceEntries)
        {
            if (!existingValues.TryGetValue(source.Key, out var localized) ||
                string.IsNullOrWhiteSpace(localized))
            {
                throw new InvalidDataException($"Catalog key is missing or blank: {locale.Key}/{source.Key}");
            }
            if (!Placeholders(source.Value).SequenceEqual(Placeholders(localized), StringComparer.Ordinal))
            {
                throw new InvalidDataException($"Catalog placeholder drift: {locale.Key}/{source.Key}");
            }
        }
        foreach (var key in existingValues.Keys)
        {
            if (!sourceEntries.ContainsKey(key))
            {
                throw new InvalidDataException($"Catalog contains undeclared key: {locale.Key}/{key}");
            }
        }
    }
}

Console.WriteLine(checkOnly
    ? "Managed localization catalog parity passed."
    : "Managed localization entries synchronized into shared catalogs.");
return 0;

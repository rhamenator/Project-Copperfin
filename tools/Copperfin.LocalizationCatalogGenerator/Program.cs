using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using Copperfin.VisualStudio;
using Microsoft.Win32.SafeHandles;

if (args.Length != 1 && args.Length != 2)
{
    Console.Error.WriteLine("Usage: Copperfin.LocalizationCatalogGenerator <repository-root> [--check]");
    return 2;
}

var repositoryRoot = Path.GetFullPath(args[0]);
var checkOnly = args.Length == 2 && string.Equals(args[1], "--check", StringComparison.Ordinal);
var localeRoot = Path.Combine(repositoryRoot, "resources", "locales");
var strictUtf8 = new UTF8Encoding(encoderShouldEmitUTF8Identifier: false, throwOnInvalidBytes: true);
const int maximumManagedInputBytes = 64 * 1024 * 1024;

void EnsureNoLinkedAncestors(string path, string description)
{
    var relative = Path.GetRelativePath(repositoryRoot, Path.GetFullPath(path));
    if (Path.IsPathRooted(relative) || relative == ".." ||
        relative.StartsWith($"..{Path.DirectorySeparatorChar}", StringComparison.Ordinal) ||
        relative.StartsWith($"..{Path.AltDirectorySeparatorChar}", StringComparison.Ordinal))
    {
        throw new InvalidDataException($"{description} must remain inside the repository root: {path}");
    }

    var rootAttributes = File.GetAttributes(repositoryRoot);
    if ((rootAttributes & (FileAttributes.Directory | FileAttributes.ReparsePoint)) != FileAttributes.Directory)
    {
        throw new InvalidDataException($"Repository root must be a regular directory, not a link: {repositoryRoot}");
    }

    var parent = Path.GetDirectoryName(relative);
    if (string.IsNullOrEmpty(parent))
    {
        return;
    }

    var current = repositoryRoot;
    foreach (var component in parent.Split(
                 [Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar],
                 StringSplitOptions.RemoveEmptyEntries))
    {
        current = Path.Combine(current, component);
        var attributes = File.GetAttributes(current);
        if ((attributes & (FileAttributes.Directory | FileAttributes.ReparsePoint)) != FileAttributes.Directory)
        {
            throw new InvalidDataException(
                $"{description} ancestor must be a regular directory, not a link: {current}");
        }
    }
}

FileStream OpenRegularFileForRead(string path, string description)
{
    EnsureNoLinkedAncestors(path, description);
    var attributes = File.GetAttributes(path);
    if ((attributes & (FileAttributes.Directory | FileAttributes.ReparsePoint | FileAttributes.Device)) != 0)
    {
        throw new InvalidDataException($"{description} must be a regular file, not a link: {path}");
    }

    if (OperatingSystem.IsLinux() || OperatingSystem.IsMacOS())
    {
        var linuxNoFollow = RuntimeInformation.ProcessArchitecture is
            Architecture.Arm or Architecture.Arm64 or Architecture.Armv6 or Architecture.Ppc64le
            ? NativeMethods.LinuxArmOpenNoFollow
            : NativeMethods.LinuxOpenNoFollow;
        var flags = OperatingSystem.IsMacOS()
            ? NativeMethods.MacOpenNonBlocking | NativeMethods.MacOpenNoFollow | NativeMethods.MacOpenCloseOnExec
            : NativeMethods.LinuxOpenNonBlocking | linuxNoFollow | NativeMethods.LinuxOpenCloseOnExec;
        var descriptor = NativeMethods.Open(path, flags, 0);
        if (descriptor < 0)
        {
            var error = Marshal.GetLastPInvokeError();
            throw new InvalidDataException(
                $"{description} must be an existing regular file, not a link: {path}",
                new Win32Exception(error));
        }

        var handle = new SafeFileHandle((IntPtr)descriptor, ownsHandle: true);
        var regular = OperatingSystem.IsLinux()
            ? NativeMethods.IsLinuxRegularFile(descriptor)
            : NativeMethods.IsMacRegularFile(descriptor);
        if (!regular)
        {
            handle.Dispose();
            throw new InvalidDataException($"{description} must be a regular file: {path}");
        }

        return new FileStream(handle, FileAccess.Read);
    }

    return new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
}

string ReadStrictUtf8(string path, string description)
{
    using var stream = OpenRegularFileForRead(path, description);
    using var buffer = new MemoryStream();
    var block = new byte[81920];
    while (true)
    {
        var count = stream.Read(block, 0, block.Length);
        if (count == 0)
        {
            break;
        }
        if (buffer.Length + count > maximumManagedInputBytes)
        {
            throw new InvalidDataException(
                $"{description} exceeds the {maximumManagedInputBytes}-byte input limit: {path}");
        }
        buffer.Write(block, 0, count);
    }
    var bytes = buffer.ToArray();
    var offset = bytes.Length >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF ? 3 : 0;
    return strictUtf8.GetString(bytes, offset, bytes.Length - offset);
}

void WriteCatalogAtomically(string path, string content)
{
    using (OpenRegularFileForRead(path, "Locale catalog"))
    {
    }

    var directory = Path.GetDirectoryName(path)
        ?? throw new InvalidDataException($"Catalog has no parent directory: {path}");
    var temporaryPath = Path.Combine(directory, $".{Path.GetFileName(path)}.{Guid.NewGuid():N}.tmp");
    UnixFileMode? unixMode = null;
    if (!OperatingSystem.IsWindows())
    {
        unixMode = File.GetUnixFileMode(path);
        const UnixFileMode writable = UnixFileMode.UserWrite | UnixFileMode.GroupWrite | UnixFileMode.OtherWrite;
        if ((unixMode & writable) == 0)
        {
            throw new UnauthorizedAccessException($"Locale catalog is read-only: {path}");
        }
    }
    else if ((File.GetAttributes(path) & FileAttributes.ReadOnly) != 0)
    {
        throw new UnauthorizedAccessException($"Locale catalog is read-only: {path}");
    }

    try
    {
        var bytes = new UTF8Encoding(encoderShouldEmitUTF8Identifier: false).GetBytes(content);
        using (var stream = new FileStream(
                   temporaryPath,
                   FileMode.CreateNew,
                   FileAccess.Write,
                   FileShare.None,
                   bufferSize: 4096,
                   FileOptions.WriteThrough))
        {
            stream.Write(bytes);
            stream.Flush(flushToDisk: true);
        }
        if (!OperatingSystem.IsWindows() && unixMode is { } mode)
        {
            File.SetUnixFileMode(temporaryPath, mode);
        }

        // Re-admit the destination immediately before replacement. File.Move
        // replaces a raced link entry rather than opening its outside target.
        using (OpenRegularFileForRead(path, "Locale catalog"))
        {
        }
        File.Move(temporaryPath, path, overwrite: true);
        temporaryPath = string.Empty;
    }
    finally
    {
        if (temporaryPath.Length > 0)
        {
            File.Delete(temporaryPath);
        }
    }
}
var inventoryLines = new List<string>();
using (var inventoryReader = new StringReader(ReadStrictUtf8(
           Path.Combine(localeRoot, "inventory.txt"), "Locale inventory")))
{
    while (inventoryReader.ReadLine() is { } line)
    {
        inventoryLines.Add(line);
    }
}
if (inventoryLines.Count < 3 || inventoryLines[0] != "version 1")
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
        !string.Equals(CopperfinLocalization.NormalizeLocale(locale), locale, StringComparison.Ordinal) ||
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
var localeRootFiles = Directory.EnumerateFiles(localeRoot)
    .Select(Path.GetFileName).ToHashSet(StringComparer.Ordinal);
if (!localeRootFiles.SetEquals(new[] { "inventory.txt" }))
{
    throw new InvalidDataException("Locale root files do not match inventory contract.");
}
foreach (var locale in locales.Keys)
{
    var localeDirectory = Path.Combine(localeRoot, locale);
    if ((File.GetAttributes(localeDirectory) & FileAttributes.ReparsePoint) != 0)
    {
        throw new InvalidDataException($"Locale directory must not be a link: {locale}");
    }
    var localeEntries = Directory.EnumerateFileSystemEntries(localeDirectory)
        .Select(Path.GetFileName).ToHashSet(StringComparer.Ordinal);
    if (!localeEntries.SetEquals(new[] { "strings.json" }))
    {
        throw new InvalidDataException($"Locale directory content does not match inventory contract: {locale}");
    }
}
var sourceEntries = JsonSerializer.Deserialize<Dictionary<string, string>>(
    ReadStrictUtf8(Path.Combine(localeRoot, "en-US", "strings.json"), "Locale catalog"))
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
using var managedCatalogReader = new StringReader(
    ReadStrictUtf8(managedCatalogSourcePath, "Managed catalog source"));
while (managedCatalogReader.ReadLine() is { } line)
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
    var catalogText = ReadStrictUtf8(catalogPath, "Locale catalog");
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
        WriteCatalogAtomically(
            catalogPath,
            prefix + separator + additions + "\n}\n");
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

internal static class NativeMethods
{
    internal const int LinuxOpenNonBlocking = 0x00000800;
    internal const int LinuxOpenNoFollow = 0x00020000;
    internal const int LinuxArmOpenNoFollow = 0x00008000;
    internal const int LinuxOpenCloseOnExec = 0x00080000;
    internal const int MacOpenNonBlocking = 0x00000004;
    internal const int MacOpenNoFollow = 0x00000100;
    internal const int MacOpenCloseOnExec = 0x01000000;
    private const int AtEmptyPath = 0x00001000;
    private const uint StatxType = 0x00000001;
    private const ushort FileTypeMask = 0xF000;
    private const ushort RegularFileType = 0x8000;

    [DllImport("libc", EntryPoint = "open", SetLastError = true)]
    internal static extern int Open(string path, int flags, int mode);

    [DllImport("libc", EntryPoint = "statx", SetLastError = true)]
    private static extern int Statx(
        int directoryDescriptor,
        string path,
        int flags,
        uint mask,
        out LinuxStatx result);

    [DllImport("libc", EntryPoint = "fstat", SetLastError = true)]
    private static extern int MacFstat(int descriptor, IntPtr result);

    internal static bool IsLinuxRegularFile(int descriptor)
    {
        return Statx(descriptor, string.Empty, AtEmptyPath, StatxType, out var result) == 0 &&
            (result.Mode & FileTypeMask) == RegularFileType;
    }

    internal static bool IsMacRegularFile(int descriptor)
    {
        var result = Marshal.AllocHGlobal(256);
        try
        {
            return MacFstat(descriptor, result) == 0 &&
                ((ushort)Marshal.ReadInt16(result, 4) & FileTypeMask) == RegularFileType;
        }
        finally
        {
            Marshal.FreeHGlobal(result);
        }
    }

    [StructLayout(LayoutKind.Explicit, Size = 256)]
    private struct LinuxStatx
    {
        [FieldOffset(28)]
        internal ushort Mode;
    }
}

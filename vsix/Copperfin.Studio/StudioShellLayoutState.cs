// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Runtime.InteropServices;

namespace Copperfin.VisualStudio;

internal interface IStudioShellLayoutStore
{
    StudioShellLayoutState? Load();

    void Save(StudioShellLayoutState state);
}

[DataContract]
internal sealed class StudioShellLayoutState
{
    internal const int CurrentVersion = 3;
    internal const string CommandWindowKey = "command";
    internal const string TerminalWindowKey = "terminal";

    [DataMember(Order = 0)]
    public int Version { get; set; } = CurrentVersion;

    [DataMember(Order = 1)]
    public bool CommandWindowVisible { get; set; } = true;

    [DataMember(Order = 2)]
    public bool TerminalWindowVisible { get; set; } = true;

    [DataMember(Order = 3)]
    public string SelectedToolWindow { get; set; } = CommandWindowKey;

    [DataMember(Order = 4)]
    public int SplitterDistance { get; set; } = 720;

    [DataMember(Order = 5)]
    public bool CommandWindowFloating { get; set; }

    [DataMember(Order = 6)]
    public bool TerminalWindowFloating { get; set; }

    [DataMember(Order = 7)]
    public int? CommandWindowFloatingX { get; set; }

    [DataMember(Order = 8)]
    public int? CommandWindowFloatingY { get; set; }

    [DataMember(Order = 9)]
    public int? CommandWindowFloatingWidth { get; set; }

    [DataMember(Order = 10)]
    public int? CommandWindowFloatingHeight { get; set; }

    [DataMember(Order = 11)]
    public int? TerminalWindowFloatingX { get; set; }

    [DataMember(Order = 12)]
    public int? TerminalWindowFloatingY { get; set; }

    [DataMember(Order = 13)]
    public int? TerminalWindowFloatingWidth { get; set; }

    [DataMember(Order = 14)]
    public int? TerminalWindowFloatingHeight { get; set; }
}

internal sealed class StudioShellLayoutFileStore : IStudioShellLayoutStore
{
    private readonly string filePath;

    internal StudioShellLayoutFileStore(string filePath)
    {
        this.filePath = filePath;
    }

    internal static StudioShellLayoutFileStore CreateDefault()
    {
        var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
        var baseDirectory = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        var isWindows = Environment.OSVersion.Platform == PlatformID.Win32NT ||
                        RuntimeInformation.IsOSPlatform(OSPlatform.Windows);
        var isMacOS = Environment.OSVersion.Platform == PlatformID.MacOSX ||
                      RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
        if (isWindows)
        {
            if (string.IsNullOrWhiteSpace(baseDirectory))
            {
                baseDirectory = home;
            }
        }
        else if (isMacOS)
        {
            baseDirectory = Path.Combine(home, "Library", "Application Support");
        }
        else
        {
            var xdgConfigHome = Environment.GetEnvironmentVariable("XDG_CONFIG_HOME");
            baseDirectory = string.IsNullOrWhiteSpace(xdgConfigHome)
                ? Path.Combine(home, ".config")
                : xdgConfigHome;
        }

        return new StudioShellLayoutFileStore(
            Path.Combine(baseDirectory, "Copperfin", "Studio", "shell-layout.json"));
    }

    public StudioShellLayoutState? Load()
    {
        if (!File.Exists(filePath))
        {
            return null;
        }

        try
        {
            using var stream = File.OpenRead(filePath);
            return new DataContractJsonSerializer(typeof(StudioShellLayoutState)).ReadObject(stream)
                as StudioShellLayoutState;
        }
        catch (Exception exception) when (
            exception is IOException ||
            exception is UnauthorizedAccessException ||
            exception is SerializationException ||
            exception is System.Xml.XmlException)
        {
            return null;
        }
    }

    public void Save(StudioShellLayoutState state)
    {
        var directory = Path.GetDirectoryName(filePath);
        if (string.IsNullOrWhiteSpace(directory))
        {
            return;
        }

        var temporaryPath = filePath + "." + Guid.NewGuid().ToString("N") + ".tmp";
        try
        {
            Directory.CreateDirectory(directory);
            using (var stream = File.Create(temporaryPath))
            {
                new DataContractJsonSerializer(typeof(StudioShellLayoutState)).WriteObject(stream, state);
                stream.Flush();
            }

            if (File.Exists(filePath))
            {
                try
                {
                    File.Replace(temporaryPath, filePath, null);
                }
                catch (PlatformNotSupportedException)
                {
                    File.Delete(filePath);
                    File.Move(temporaryPath, filePath);
                }
            }
            else
            {
                File.Move(temporaryPath, filePath);
            }
        }
        catch (Exception exception) when (
            exception is IOException ||
            exception is UnauthorizedAccessException ||
            exception is PlatformNotSupportedException)
        {
            // Layout persistence is best effort and must not prevent the shell from closing.
        }
        finally
        {
            try
            {
                if (File.Exists(temporaryPath))
                {
                    File.Delete(temporaryPath);
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }
    }
}

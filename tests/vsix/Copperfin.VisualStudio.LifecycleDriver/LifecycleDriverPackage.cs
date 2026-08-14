// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Traceability: RQ-CF-REL-003; DQ-windows-vsix-lifecycle-scope;
// DV-windows-vsix-lifecycle-contract; HZ-system-failure-01;
// HZ-data-corruption-01; HZ-doc-command-01.

using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;

namespace Copperfin.VisualStudio.LifecycleDriver;

[PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
[ProvideAutoLoad(UIContextGuids80.NoSolution, PackageAutoLoadFlags.BackgroundLoad)]
[Guid(PackageGuid)]
public sealed class LifecycleDriverPackage : AsyncPackage
{
    private const string PackageGuid = "9c9f5ea2-4bb2-4c82-a88d-66c0be2149d1";
    private const string ProductCommandSet = "4b56ff76-d352-4027-bb18-ef4c759d260b";
    private const uint ShowCommandWindowId = 0x0300;
    private const string ResultVariable = "COPPERFIN_VSIX_LIFECYCLE_DRIVER_RESULT";
    private const string PrgVariable = "COPPERFIN_VSIX_LIFECYCLE_DRIVER_PRG";

    protected override async Task InitializeAsync(
        CancellationToken cancellationToken,
        IProgress<ServiceProgressData> progress)
    {
        var resultPath = Environment.GetEnvironmentVariable(ResultVariable);
        var prgPath = Environment.GetEnvironmentVariable(PrgVariable);
        if (string.IsNullOrWhiteSpace(resultPath) || string.IsNullOrWhiteSpace(prgPath))
        {
            return;
        }

        var result = new Dictionary<string, object>
        {
            ["schema_version"] = 1,
            ["kind"] = "copperfin-windows-vsix-lifecycle-driver",
            ["product_command_group"] = ProductCommandSet,
            ["product_command_id"] = ShowCommandWindowId,
            ["command_post_hresult"] = -1,
            ["prg_open_requested"] = false,
            ["outcome"] = "ERROR",
            ["diagnostic"] = string.Empty,
        };

        try
        {
            resultPath = Path.GetFullPath(resultPath);
            prgPath = Path.GetFullPath(prgPath);
            var resultDirectory = Path.GetDirectoryName(resultPath);
            if (string.IsNullOrWhiteSpace(resultDirectory) || !Directory.Exists(resultDirectory))
            {
                throw new InvalidOperationException("Lifecycle-driver result directory is unavailable.");
            }
            if (!File.Exists(prgPath))
            {
                throw new FileNotFoundException("Lifecycle-driver PRG fixture is unavailable.", prgPath);
            }

            await JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
            var dte = await GetServiceAsync(typeof(EnvDTE.DTE)) as EnvDTE.DTE
                ?? throw new InvalidOperationException("Visual Studio DTE service is unavailable in-process.");
            _ = dte.ItemOperations.OpenFile(prgPath, EnvDTE.Constants.vsViewKindTextView);
            result["prg_open_requested"] = true;

            var uiShell = await GetServiceAsync(typeof(SVsUIShell)) as IVsUIShell
                ?? throw new InvalidOperationException("Visual Studio UI shell service is unavailable in-process.");
            var commandGroup = new Guid(ProductCommandSet);
            object commandArgument = null!;
            var hresult = uiShell.PostExecCommand(
                ref commandGroup,
                ShowCommandWindowId,
                (uint)OLECMDEXECOPT.OLECMDEXECOPT_DODEFAULT,
                ref commandArgument);
            result["command_post_hresult"] = hresult;
            ErrorHandler.ThrowOnFailure(hresult);
            result["outcome"] = "PASS";
        }
        catch (Exception exception)
        {
            result["diagnostic"] = exception.GetType().Name + ": " + exception.Message;
            WriteResult(resultPath, result);
            throw;
        }

        WriteResult(resultPath, result);
    }

    private static void WriteResult(string path, Dictionary<string, object> result)
    {
        var json = new JavaScriptSerializer().Serialize(result);
        File.WriteAllText(path, json, new System.Text.UTF8Encoding(false));
    }
}

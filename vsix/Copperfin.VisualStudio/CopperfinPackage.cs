using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.Shell;

namespace Copperfin.VisualStudio;

[PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
[InstalledProductRegistration(
    "Copperfin for Visual Studio",
    "Launches Copperfin Studio for Visual FoxPro-style assets and prepares the path toward native asset designers.",
    "0.1.0")]
[ProvideMenuResource("Menus.ctmenu", 1)]
[Guid(PackageGuids.PackageString)]
public sealed class CopperfinPackage : AsyncPackage
{
    protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress)
    {
        await JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
        await OpenInCopperfinStudioCommand.InitializeAsync(this);
    }
}

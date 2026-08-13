// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Linq;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static void SmokeStandaloneStudioWorkspaceAgentPolicySurface()
    {
        var parsed = CopperfinWorkspaceAgentPolicyClient.TryParse(WorkspaceAgentPolicyJson);
        Expect(parsed.Success && parsed.Descriptor is not null,
            "standalone workspace-agent policy smoke requires the validated descriptor fixture");
        if (parsed.Descriptor is null)
        {
            return;
        }

        var spanish = new CopperfinLocalization("es-419");
        using var form = new StudioMainForm(spanish, new InMemoryStudioShellLayoutStore());
        Expect(form.WorkspaceAgentPolicyMenuText == "Acceso del asistente del área de trabajo...",
            "standalone Studio should expose localized workspace-assistant policy access");
        var invalidPolicy = CopperfinWorkspaceAgentPolicyClient.TryParse("not-json");
        Expect(form.WorkspaceAgentPolicyErrorTextForTest(invalidPolicy) ==
               "Copperfin no pudo verificar la política de acceso del asistente del área de trabajo.",
            "standalone Studio should not expose raw invariant parser details as user-facing policy errors");
        var failedHost = new CopperfinWorkspaceAgentPolicyResult
        {
            Success = false,
            DiagnosticCode = "workspace-agent-policy.host-failed",
            Error = "untrusted host stderr"
        };
        Expect(form.WorkspaceAgentPolicyErrorTextForTest(failedHost) ==
               "Copperfin no pudo verificar la política de acceso del asistente del área de trabajo.",
            "standalone Studio should not expose raw host output as user-facing policy errors");

        using var dialog = form.CreateWorkspaceAgentPolicyDialogForTest(parsed.Descriptor);
        var dialogButtons = FindButtons(dialog).ToList();
        Expect(dialog.Text == "Acceso del asistente del área de trabajo" &&
               dialog.ActivationStatusText == "Vista previa de solo lectura; la activación del asistente aún no está disponible." &&
               dialog.ModeCount == 3 && dialog.SelectedModeName == "advisory" &&
               dialog.ModeSelectorAccessibleName == "Modo de acceso:" &&
               dialog.DetailsAccessibleName == "Capacidades" &&
               dialogButtons.Count == 1 && dialogButtons[0].Text == "Cerrar" &&
               dialogButtons[0].AccessibleName == "Cerrar",
            "workspace-assistant policy surface should localize its chrome and default to advisory without activation");

        dialog.SelectModeForTest("unrestricted_local");
        Expect(dialog.SelectedModeName == "unrestricted_local" &&
               dialog.DetailsText.Contains("[!! localized title !!]", StringComparison.Ordinal) &&
               dialog.DetailsText.Contains("Elevar privilegios: Falso", StringComparison.Ordinal),
            "unrestricted policy preview should show host-provided warning prose and permanent no-elevation state");

        var pseudo = new CopperfinLocalization(CopperfinLocalization.PseudoLocale);
        using var pseudoDialog = new StudioWorkspaceAgentPolicyDialog(parsed.Descriptor, pseudo);
        Expect(pseudoDialog.Text == pseudo.Text("Studio.WorkspaceAgent.Title") &&
               pseudoDialog.ActivationStatusText == pseudo.Text("Studio.WorkspaceAgent.ActivationUnavailable"),
            "workspace-assistant policy surface should remain pseudo-localization ready");
    }
}

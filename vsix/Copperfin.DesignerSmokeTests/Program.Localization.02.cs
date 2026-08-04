
// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;
internal static partial class Program
{
    private static void SmokeLocalizedProjectInsightArtifactKindLabels()
    {
        var insights = new CopperfinProjectInsights
        {
            ProjectRoot = @"C:\src\sample",
            DataAssets = new List<CopperfinProjectDataAsset>
            {
                new() { Kind = "Table", Title = "customer.dbf", FilePath = @"C:\src\sample\customer.dbf" },
                new() { Kind = "Query", Title = "orders.qpr", FilePath = @"C:\src\sample\orders.qpr" }
            },
            ObjectNodes = new List<CopperfinProjectObjectNode>
            {
                new() { Kind = "Program", Title = "main.prg", FilePath = @"C:\src\sample\main.prg", Detail = "Programs" },
                new() { Kind = "Class", Title = "app.customer.editor", FilePath = @"C:\src\sample\editor.prg", Detail = "AS custom" }
            },
            DefinedSymbols = new List<CopperfinProjectCodeSymbol>
            {
                new() { Kind = "class", Name = "app.customer.editor", FilePath = @"C:\src\sample\editor.prg", Line = 1 }
            }
        };

        var snapshot = new CopperfinStudioSnapshotDocument
        {
            ProjectWorkspace = new CopperfinStudioProjectWorkspace
            {
                ProjectTitle = "sample.pjx"
            }
        };

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        var spanishObjectSummary = InvokeAssetEditorString(spanishControl, "BuildObjectBrowserSummary", snapshot, insights, string.Empty, false);
        var spanishDataSummary = InvokeAssetEditorString(spanishControl, "BuildDataExplorerSummary", snapshot, insights, string.Empty);
        Expect(spanishObjectSummary.IndexOf("[Programa] main.prg", StringComparison.Ordinal) >= 0 &&
               spanishObjectSummary.IndexOf("[Clase] app.customer.editor", StringComparison.Ordinal) >= 0 &&
               spanishDataSummary.IndexOf("[Tabla] customer.dbf", StringComparison.Ordinal) >= 0 &&
               spanishDataSummary.IndexOf("[Consulta] orders.qpr", StringComparison.Ordinal) >= 0,
            "Spanish project summaries should localize displayed object and data kind labels");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        var portugueseObjectSummary = InvokeAssetEditorString(portugueseControl, "BuildObjectBrowserSummary", snapshot, insights, string.Empty, false);
        var portugueseDataSummary = InvokeAssetEditorString(portugueseControl, "BuildDataExplorerSummary", snapshot, insights, string.Empty);
        Expect(portugueseObjectSummary.IndexOf("[Programa] main.prg", StringComparison.Ordinal) >= 0 &&
               portugueseObjectSummary.IndexOf("[Classe] app.customer.editor", StringComparison.Ordinal) >= 0 &&
               portugueseDataSummary.IndexOf("[Tabela] customer.dbf", StringComparison.Ordinal) >= 0 &&
               portugueseDataSummary.IndexOf("[Consulta] orders.qpr", StringComparison.Ordinal) >= 0,
            "Portuguese project summaries should localize displayed object and data kind labels");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        var pseudoObjectSummary = InvokeAssetEditorString(pseudoControl, "BuildObjectBrowserSummary", snapshot, insights, string.Empty, false);
        var pseudoDataSummary = InvokeAssetEditorString(pseudoControl, "BuildDataExplorerSummary", snapshot, insights, string.Empty);
        Expect(pseudoObjectSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.ArtifactKind.Program"), StringComparison.Ordinal) >= 0 &&
               pseudoObjectSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.ArtifactKind.Class"), StringComparison.Ordinal) >= 0 &&
               pseudoDataSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.ArtifactKind.Table"), StringComparison.Ordinal) >= 0 &&
               pseudoDataSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.ArtifactKind.Query"), StringComparison.Ordinal) >= 0 &&
               pseudoObjectSummary.IndexOf("[Program]", StringComparison.Ordinal) < 0 &&
               pseudoObjectSummary.IndexOf("[Class]", StringComparison.Ordinal) < 0 &&
               pseudoDataSummary.IndexOf("[Table]", StringComparison.Ordinal) < 0 &&
               pseudoDataSummary.IndexOf("[Query]", StringComparison.Ordinal) < 0,
            "Pseudo-localized project summaries should route displayed object and data kind labels through the shared catalog instead of leaking raw tokens");
    }

    private static void SmokeLocalizedBuilderSummaryArtifactKindLabels()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            ProjectWorkspace = new CopperfinStudioProjectWorkspace
            {
                ProjectTitle = "sample.pjx"
            }
        };

        var insights = new CopperfinProjectInsights
        {
            ProjectRoot = @"C:\src\sample",
            ObjectNodes = new List<CopperfinProjectObjectNode>
            {
                new() { Kind = "Program", Title = "main.prg", FilePath = @"C:\src\sample\main.prg" },
                new() { Kind = "Class", Title = "app.customer.editor", FilePath = @"C:\src\sample\editor.prg" }
            }
        };

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        var spanishSummary = InvokeAssetEditorString(spanishControl, "BuildBuilderSummary", snapshot, insights);
        Expect(spanishSummary.IndexOf("[Programa] main.prg", StringComparison.Ordinal) >= 0 &&
               spanishSummary.IndexOf("[Clase] app.customer.editor", StringComparison.Ordinal) >= 0,
            "Spanish builder summaries should localize displayed current-target artifact kinds");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        var portugueseSummary = InvokeAssetEditorString(portugueseControl, "BuildBuilderSummary", snapshot, insights);
        Expect(portugueseSummary.IndexOf("[Programa] main.prg", StringComparison.Ordinal) >= 0 &&
               portugueseSummary.IndexOf("[Classe] app.customer.editor", StringComparison.Ordinal) >= 0,
            "Portuguese builder summaries should localize displayed current-target artifact kinds");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        var pseudoSummary = InvokeAssetEditorString(pseudoControl, "BuildBuilderSummary", snapshot, insights);
        Expect(pseudoSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.ArtifactKind.Program"), StringComparison.Ordinal) >= 0 &&
               pseudoSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.ArtifactKind.Class"), StringComparison.Ordinal) >= 0 &&
               pseudoSummary.IndexOf("[Program]", StringComparison.Ordinal) < 0 &&
               pseudoSummary.IndexOf("[Class]", StringComparison.Ordinal) < 0,
            "Pseudo-localized builder summaries should route current-target artifact kinds through the shared catalog instead of leaking raw tokens");
    }

    private static void SmokeLocalizedWorkspaceGroupTitles()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            ProjectWorkspace = new CopperfinStudioProjectWorkspace
            {
                ProjectTitle = "sample.pjx",
                Groups = new List<CopperfinStudioProjectGroup>
                {
                    new() { Id = "forms", Title = "Forms", ItemCount = 2, ExcludedCount = 0 },
                    new() { Id = "programs", Title = "Programs", ItemCount = 1, ExcludedCount = 0 },
                    new() { Id = "classes", Title = "Class Libraries", ItemCount = 3, ExcludedCount = 1 }
                }
            }
        };

        var insights = new CopperfinProjectInsights
        {
            ProjectRoot = @"C:\src\sample",
            ObjectNodes = new List<CopperfinProjectObjectNode>
            {
                new()
                {
                    Kind = "Program",
                    Title = "main.prg",
                    FilePath = @"C:\src\sample\main.prg",
                    GroupTitle = "Programs",
                    Excluded = true,
                    Detail = "Programs [excluded]"
                }
            },
            DefinedSymbols = new List<CopperfinProjectCodeSymbol>
            {
                new() { Kind = "class", Name = "app.customer.editor", FilePath = @"C:\src\sample\editor.prg", Line = 1 }
            }
        };

        snapshot.ProjectWorkspace.Groups[2].Title = "Bibliotecas de clases";
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        var spanishWorkspaceSummary = InvokeAssetEditorString(spanishControl, "BuildProjectWorkspaceSummary", snapshot);
        var spanishToolboxSummary = InvokeAssetEditorString(spanishControl, "BuildToolboxSummary", snapshot, insights);
        var spanishObjectSummary = InvokeAssetEditorString(spanishControl, "BuildObjectBrowserSummary", snapshot, insights, string.Empty, false);
        Expect(spanishWorkspaceSummary.IndexOf("Formularios", StringComparison.Ordinal) >= 0 &&
               spanishWorkspaceSummary.IndexOf("Programas", StringComparison.Ordinal) >= 0 &&
               spanishWorkspaceSummary.IndexOf("Bibliotecas de clases", StringComparison.Ordinal) >= 0 &&
               spanishToolboxSummary.IndexOf("Formularios", StringComparison.Ordinal) >= 0 &&
               spanishToolboxSummary.IndexOf("Programas", StringComparison.Ordinal) >= 0 &&
               spanishToolboxSummary.IndexOf("Bibliotecas de clases", StringComparison.Ordinal) >= 0 &&
               spanishObjectSummary.IndexOf("Programas [excluido]", StringComparison.Ordinal) >= 0,
            "Spanish project summaries should localize workspace group titles and excluded suffixes without changing machine-readable project metadata");

        snapshot.ProjectWorkspace.Groups[2].Title = "Bibliotecas de classes";
        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        var portugueseWorkspaceSummary = InvokeAssetEditorString(portugueseControl, "BuildProjectWorkspaceSummary", snapshot);
        var portugueseToolboxSummary = InvokeAssetEditorString(portugueseControl, "BuildToolboxSummary", snapshot, insights);
        var portugueseObjectSummary = InvokeAssetEditorString(portugueseControl, "BuildObjectBrowserSummary", snapshot, insights, string.Empty, false);
        Expect(portugueseWorkspaceSummary.IndexOf("Formulários", StringComparison.Ordinal) >= 0 &&
               portugueseWorkspaceSummary.IndexOf("Programas", StringComparison.Ordinal) >= 0 &&
               portugueseWorkspaceSummary.IndexOf("Bibliotecas de classes", StringComparison.Ordinal) >= 0 &&
               portugueseToolboxSummary.IndexOf("Formulários", StringComparison.Ordinal) >= 0 &&
               portugueseToolboxSummary.IndexOf("Programas", StringComparison.Ordinal) >= 0 &&
               portugueseToolboxSummary.IndexOf("Bibliotecas de classes", StringComparison.Ordinal) >= 0 &&
               portugueseObjectSummary.IndexOf("Programas [excluído]", StringComparison.Ordinal) >= 0,
            "Portuguese project summaries should localize workspace group titles and excluded suffixes without changing machine-readable project metadata");

        snapshot.ProjectWorkspace.Groups[2].Title = "Class Libraries";
        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        var pseudoWorkspaceSummary = InvokeAssetEditorString(pseudoControl, "BuildProjectWorkspaceSummary", snapshot);
        var pseudoToolboxSummary = InvokeAssetEditorString(pseudoControl, "BuildToolboxSummary", snapshot, insights);
        var pseudoObjectSummary = InvokeAssetEditorString(pseudoControl, "BuildObjectBrowserSummary", snapshot, insights, string.Empty, false);
        Expect(pseudoWorkspaceSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.Forms"), StringComparison.Ordinal) >= 0 &&
               pseudoWorkspaceSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.Programs"), StringComparison.Ordinal) >= 0 &&
               pseudoWorkspaceSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.ClassLibraries"), StringComparison.Ordinal) >= 0 &&
               pseudoToolboxSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.Forms"), StringComparison.Ordinal) >= 0 &&
               pseudoToolboxSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.Programs"), StringComparison.Ordinal) >= 0 &&
               pseudoToolboxSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.ClassLibraries"), StringComparison.Ordinal) >= 0 &&
               pseudoObjectSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.Programs"), StringComparison.Ordinal) >= 0 &&
               pseudoObjectSummary.IndexOf("Forms", StringComparison.Ordinal) < 0 &&
               pseudoObjectSummary.IndexOf("Programs [excluded]", StringComparison.Ordinal) < 0 &&
               pseudoWorkspaceSummary.IndexOf("Class Libraries", StringComparison.Ordinal) < 0 &&
               pseudoToolboxSummary.IndexOf("Programs", StringComparison.Ordinal) < 0,
            "Pseudo-localized project summaries should route workspace group titles through the shared catalog instead of leaking raw English labels");
    }

    private static void SmokeLocalizedProjectWorkspaceBooleanValues()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            ProjectWorkspace = new CopperfinStudioProjectWorkspace
            {
                ProjectTitle = "sample.pjx",
                BuildPlan = new CopperfinStudioProjectBuildPlan
                {
                    DebugEnabled = true,
                    EncryptEnabled = false,
                    SaveCode = true,
                    NoLogo = false
                }
            }
        };

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        var spanishSummary = InvokeAssetEditorString(spanishControl, "BuildProjectWorkspaceSummary", snapshot);
        Expect(spanishSummary.IndexOf("Depurar: Verdadero", StringComparison.Ordinal) >= 0 &&
               spanishSummary.IndexOf("Cifrar: Falso", StringComparison.Ordinal) >= 0 &&
               spanishSummary.IndexOf("Guardar código: Verdadero", StringComparison.Ordinal) >= 0 &&
               spanishSummary.IndexOf("Sin logotipo: Falso", StringComparison.Ordinal) >= 0,
            "Spanish project workspace summaries should localize build-plan boolean display values");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        var portugueseSummary = InvokeAssetEditorString(portugueseControl, "BuildProjectWorkspaceSummary", snapshot);
        Expect(portugueseSummary.IndexOf("Depurar: Verdadeiro", StringComparison.Ordinal) >= 0 &&
               portugueseSummary.IndexOf("Criptografar: Falso", StringComparison.Ordinal) >= 0 &&
               portugueseSummary.IndexOf("Salvar código: Verdadeiro", StringComparison.Ordinal) >= 0 &&
               portugueseSummary.IndexOf("Sem logotipo: Falso", StringComparison.Ordinal) >= 0,
            "Portuguese project workspace summaries should localize build-plan boolean display values");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        var pseudoSummary = InvokeAssetEditorString(pseudoControl, "BuildProjectWorkspaceSummary", snapshot);
        Expect(pseudoSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.Boolean.True"), StringComparison.Ordinal) >= 0 &&
               pseudoSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.Boolean.False"), StringComparison.Ordinal) >= 0 &&
               pseudoSummary.IndexOf(": True\n", StringComparison.Ordinal) < 0 &&
               pseudoSummary.IndexOf(": False\n", StringComparison.Ordinal) < 0,
            "Pseudo-localized project workspace summaries should route build-plan boolean display values through the shared catalog");
    }

    private static void SmokeLocalizedProjectWorkspaceExplorerGroupTitles()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "project",
            ProjectWorkspace = new CopperfinStudioProjectWorkspace
            {
                Groups = new List<CopperfinStudioProjectGroup>
                {
                    new() { Id = "forms", Title = "Forms", ItemCount = 2, ExcludedCount = 0 },
                    new() { Id = "programs", Title = "Programs", ItemCount = 1, ExcludedCount = 0, RecordIndexes = new List<int> { 10 } },
                    new() { Id = "classes", Title = "Class Libraries", ItemCount = 3, ExcludedCount = 1 }
                },
                Entries = new List<CopperfinStudioProjectEntry>
                {
                    new()
                    {
                        RecordIndex = 10,
                        RelativePath = @"src\main.prg",
                        GroupId = "programs",
                        GroupTitle = "Programs"
                    }
                }
            },
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 10,
                    Title = "main.prg",
                    Subtitle = "Programs"
                }
            }
        };

        snapshot.ProjectWorkspace.Groups[2].Title = "Bibliotecas de clases";
        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        ApplyProjectSnapshotForExplorerGroupTitleSmoke(spanishControl, snapshot);
        AssertProjectWorkspaceGroupTitles(
            spanishControl,
            new[] { "Formularios", "Programas", "Bibliotecas de clases" },
            "Programas",
            "Spanish project explorer rows and project object-list subtitles should localize workspace group titles");

        snapshot.ProjectWorkspace.Groups[2].Title = "Bibliotecas de classes";
        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        ApplyProjectSnapshotForExplorerGroupTitleSmoke(portugueseControl, snapshot);
        AssertProjectWorkspaceGroupTitles(
            portugueseControl,
            new[] { "Formulários", "Programas", "Bibliotecas de classes" },
            "Programas",
            "Portuguese project explorer rows and project object-list subtitles should localize workspace group titles");

        snapshot.ProjectWorkspace.Groups[2].Title = "Class Libraries";
        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        ApplyProjectSnapshotForExplorerGroupTitleSmoke(pseudoControl, snapshot);
        AssertProjectWorkspaceGroupTitles(
            pseudoControl,
            new[]
            {
                pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.Forms"),
                pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.Programs"),
                pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.ClassLibraries")
            },
            pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.Programs"),
            "Pseudo-localized project explorer rows and project object-list subtitles should route workspace group titles through the shared catalog instead of leaking raw English labels",
            rawLeakChecks: new[] { "Forms", "Programs", "Class Libraries" });
    }

    private static void SmokeLocalizedProjectFallbackKindAndGroupLabels()
    {
        var snapshot = new CopperfinStudioSnapshotDocument
        {
            AssetFamily = "project",
            ProjectWorkspace = new CopperfinStudioProjectWorkspace
            {
                ProjectTitle = "sample.pjx",
                Groups = new List<CopperfinStudioProjectGroup>
                {
                    new() { Id = "project_items", Title = "Project Items", ItemCount = 1, ExcludedCount = 1, RecordIndexes = new List<int> { 10 } },
                    new() { Id = "other_records", Title = "Other Records", ItemCount = 1, ExcludedCount = 0 }
                },
                Entries = new List<CopperfinStudioProjectEntry>
                {
                    new()
                    {
                        RecordIndex = 10,
                        RelativePath = @"docs\README",
                        GroupId = "project_items",
                        GroupTitle = "Project Items"
                    }
                }
            },
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new()
                {
                    RecordIndex = 10,
                    Title = "README",
                    Subtitle = "Project Items"
                }
            }
        };

        var insights = new CopperfinProjectInsights
        {
            ProjectRoot = @"C:\src\sample",
            DataAssets = new List<CopperfinProjectDataAsset>
            {
                new() { Kind = "Project Record", Title = "TYPE=Z", FilePath = @"C:\src\sample\sample.pjx", GroupTitle = "Other Records" }
            },
            ObjectNodes = new List<CopperfinProjectObjectNode>
            {
                new()
                {
                    Kind = "Project Item",
                    Title = "README",
                    FilePath = @"C:\src\sample\README",
                    GroupTitle = "Project Items",
                    Excluded = true,
                    Detail = "Project Items [excluded]"
                }
            }
        };

        using var spanishControl = new CopperfinAssetEditorControl(new CopperfinLocalization("es-419"));
        ApplyProjectSnapshotForExplorerGroupTitleSmoke(spanishControl, snapshot);
        AssertProjectWorkspaceGroupTitles(
            spanishControl,
            new[] { "Elementos del proyecto", "Otros registros" },
            "Elementos del proyecto",
            "Spanish project fallback explorer rows and project object-list subtitles should localize managed Project Items and Other Records group titles");
        var spanishObjectSummary = InvokeAssetEditorString(spanishControl, "BuildObjectBrowserSummary", snapshot, insights, string.Empty, false);
        var spanishDataSummary = InvokeAssetEditorString(spanishControl, "BuildDataExplorerSummary", snapshot, insights, string.Empty);
        Expect(spanishObjectSummary.IndexOf("[Elemento del proyecto] README", StringComparison.Ordinal) >= 0 &&
               spanishObjectSummary.IndexOf("Elementos del proyecto [excluido]", StringComparison.Ordinal) >= 0 &&
               spanishDataSummary.IndexOf("[Registro del proyecto] TYPE=Z", StringComparison.Ordinal) >= 0,
            "Spanish project fallback summaries should localize Project Item, Project Record, and Project Items display text");

        using var portugueseControl = new CopperfinAssetEditorControl(new CopperfinLocalization("pt-BR"));
        ApplyProjectSnapshotForExplorerGroupTitleSmoke(portugueseControl, snapshot);
        AssertProjectWorkspaceGroupTitles(
            portugueseControl,
            new[] { "Itens do projeto", "Outros registros" },
            "Itens do projeto",
            "Portuguese project fallback explorer rows and project object-list subtitles should localize managed Project Items and Other Records group titles");
        var portugueseObjectSummary = InvokeAssetEditorString(portugueseControl, "BuildObjectBrowserSummary", snapshot, insights, string.Empty, false);
        var portugueseDataSummary = InvokeAssetEditorString(portugueseControl, "BuildDataExplorerSummary", snapshot, insights, string.Empty);
        Expect(portugueseObjectSummary.IndexOf("[Item do projeto] README", StringComparison.Ordinal) >= 0 &&
               portugueseObjectSummary.IndexOf("Itens do projeto [excluído]", StringComparison.Ordinal) >= 0 &&
               portugueseDataSummary.IndexOf("[Registro do projeto] TYPE=Z", StringComparison.Ordinal) >= 0,
            "Portuguese project fallback summaries should localize Project Item, Project Record, and Project Items display text");

        var pseudoLocalization = new CopperfinLocalization("qps-ploc");
        using var pseudoControl = new CopperfinAssetEditorControl(pseudoLocalization);
        ApplyProjectSnapshotForExplorerGroupTitleSmoke(pseudoControl, snapshot);
        AssertProjectWorkspaceGroupTitles(
            pseudoControl,
            new[]
            {
                pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.ProjectItems"),
                pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.OtherRecords")
            },
            pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.ProjectItems"),
            "Pseudo-localized project fallback explorer rows and project object-list subtitles should route managed Project Items and Other Records group titles through the shared catalog",
            rawLeakChecks: new[] { "Project Items", "Other Records" });
        var pseudoObjectSummary = InvokeAssetEditorString(pseudoControl, "BuildObjectBrowserSummary", snapshot, insights, string.Empty, false);
        var pseudoDataSummary = InvokeAssetEditorString(pseudoControl, "BuildDataExplorerSummary", snapshot, insights, string.Empty);
        Expect(pseudoObjectSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.ArtifactKind.ProjectItem"), StringComparison.Ordinal) >= 0 &&
               pseudoObjectSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.GroupTitle.ProjectItems"), StringComparison.Ordinal) >= 0 &&
               pseudoDataSummary.IndexOf(pseudoLocalization.Text("AssetEditor.Summary.ArtifactKind.ProjectRecord"), StringComparison.Ordinal) >= 0 &&
               pseudoObjectSummary.IndexOf("Project Item", StringComparison.Ordinal) < 0 &&
               pseudoObjectSummary.IndexOf("Project Items", StringComparison.Ordinal) < 0 &&
               pseudoDataSummary.IndexOf("Project Record", StringComparison.Ordinal) < 0,
            "Pseudo-localized project fallback summaries should route Project Item, Project Record, and Project Items display text through the shared catalog instead of leaking raw English labels");
    }

}

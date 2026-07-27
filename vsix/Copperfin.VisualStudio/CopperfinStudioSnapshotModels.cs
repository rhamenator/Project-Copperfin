// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System.Collections.Generic;

namespace Copperfin.VisualStudio;

internal sealed class CopperfinStudioSnapshotEnvelope
{
    public string Status { get; set; } = string.Empty;
    public CopperfinStudioSnapshotDocument Document { get; set; } = new();
}

internal sealed class CopperfinStudioToolboxPaletteEnvelope
{
    public string Status { get; set; } = string.Empty;
    public string Error { get; set; } = string.Empty;
    public CopperfinStudioToolboxPalettePayload? ToolboxPaletteQuery { get; set; }
}

internal sealed class CopperfinStudioToolboxPalettePayload
{
    public bool Ok { get; set; }
    public string Error { get; set; } = string.Empty;
    public string ToolboxContext { get; set; } = string.Empty;
    public List<CopperfinStudioToolboxItem> Items { get; set; } = new();
}

internal sealed class CopperfinStudioToolboxItem
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Category { get; set; } = string.Empty;
    public string VfpClass { get; set; } = string.Empty;
    public string BaseClass { get; set; } = string.Empty;
    public string DefaultNamePrefix { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioToolboxCreateEnvelope
{
    public string Status { get; set; } = string.Empty;
    public string Error { get; set; } = string.Empty;
    public CopperfinStudioToolboxCreatePayload? ToolboxCreate { get; set; }
}

internal sealed class CopperfinStudioToolboxCreatePayload
{
    public bool Ok { get; set; }
    public string Error { get; set; } = string.Empty;
    public string ObjectName { get; set; } = string.Empty;
    public string UniqueId { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioToolboxPaletteResult
{
    public bool Success { get; set; }
    public string Error { get; set; } = string.Empty;
    public string Context { get; set; } = string.Empty;
    public List<CopperfinStudioToolboxItem> Items { get; set; } = new();
}

internal sealed class CopperfinStudioToolboxCreateResult
{
    public bool Success { get; set; }
    public string Error { get; set; } = string.Empty;
    public string ObjectName { get; set; } = string.Empty;
    public string UniqueId { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioBuilderCatalogEnvelope
{
    public string Status { get; set; } = string.Empty;
    public string Error { get; set; } = string.Empty;
    public CopperfinStudioBuilderCatalogPayload? BuilderLaunchCatalog { get; set; }
}

internal sealed class CopperfinStudioBuilderCatalogPayload
{
    public bool Ok { get; set; }
    public string Error { get; set; } = string.Empty;
    public string Context { get; set; } = string.Empty;
    public List<CopperfinStudioBuilderCatalogEntry> Entries { get; set; } = new();
}

internal sealed class CopperfinStudioBuilderCatalogEntry
{
    public string BuilderId { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Kind { get; set; } = string.Empty;
    public string Context { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string Vfp9EquivalentDisplay { get; set; } = string.Empty;
    public bool LaunchOk { get; set; }
}

internal sealed class CopperfinStudioBuilderCatalogResult
{
    public bool Success { get; set; }
    public string Error { get; set; } = string.Empty;
    public List<CopperfinStudioBuilderCatalogEntry> Entries { get; set; } = new();
}

internal sealed class CopperfinStudioBuilderLaunchPlanEnvelope
{
    public string Status { get; set; } = string.Empty;
    public string Error { get; set; } = string.Empty;
    public CopperfinStudioBuilderLaunchPlanPayload? BuilderLaunchPlan { get; set; }
}

internal sealed class CopperfinStudioBuilderLaunchPlanPayload
{
    public bool Ok { get; set; }
    public string Error { get; set; } = string.Empty;
    public string BuilderId { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Kind { get; set; } = string.Empty;
    public string SelectionContext { get; set; } = string.Empty;
    public string Context { get; set; } = string.Empty;
    public string Vfp9EquivalentDisplay { get; set; } = string.Empty;
    public string CopperfinComponent { get; set; } = string.Empty;
    public string EntryPoint { get; set; } = string.Empty;
    public string AssetPath { get; set; } = string.Empty;
    public int RecordIndex { get; set; }
    public string ObjectName { get; set; } = string.Empty;
    public string UniqueId { get; set; } = string.Empty;
    public bool LaunchReady { get; set; }
    public List<string> LaunchReadyBuilderIds { get; set; } = new();
    public List<string> LaunchBlockedBuilderIds { get; set; } = new();
    public List<string> LaunchBlockedErrors { get; set; } = new();
    public string Description { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioBuilderLaunchPlanResult
{
    public bool Success { get; set; }
    public string Error { get; set; } = string.Empty;
    public CopperfinStudioBuilderLaunchPlanPayload Plan { get; set; } = new();
}

internal sealed class CopperfinStudioBuilderExecutionEnvelope
{
    public string Status { get; set; } = string.Empty;
    public string Error { get; set; } = string.Empty;
    public CopperfinStudioBuilderExecutionPayload? BuilderExecution { get; set; }
    public int ObservedExitCode { get; set; }
    public bool Executed { get; set; }
    public bool DryRun { get; set; }
    public bool ExecutionAdmitted { get; set; }
    public string LaunchCommand { get; set; } = string.Empty;
    public string ExecutedCommand { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioBuilderExecutionPayload
{
    public bool Ok { get; set; }
    public string Error { get; set; } = string.Empty;
    public string BuilderId { get; set; } = string.Empty;
    public string Context { get; set; } = string.Empty;
    public string EntryPoint { get; set; } = string.Empty;
    public bool Executed { get; set; }
    public bool DryRun { get; set; }
    public bool ExecutionAdmitted { get; set; }
    public int ObservedExitCode { get; set; }
    public string LaunchCommand { get; set; } = string.Empty;
    public string ExecutedCommand { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioBuilderExecutionResult
{
    public bool Success { get; set; }
    public string Error { get; set; } = string.Empty;
    public int ObservedExitCode { get; set; }
    public bool Executed { get; set; }
}

internal sealed class CopperfinStudioSnapshotDocument
{
    public string Path { get; set; } = string.Empty;
    public string DisplayName { get; set; } = string.Empty;
    public string Kind { get; set; } = string.Empty;
    public bool ReadOnly { get; set; }
    public bool LaunchedFromVisualStudio { get; set; }
    public bool HasSidecar { get; set; }
    public string SidecarPath { get; set; } = string.Empty;
    public string AssetFamily { get; set; } = string.Empty;
    public int IndexCount { get; set; }
    public string HeaderVersionDescription { get; set; } = string.Empty;
    public int FieldCount { get; set; }
    public int RecordCount { get; set; }
    public bool CommandUndoAvailable { get; set; }
    public string CommandUndoLabel { get; set; } = string.Empty;
    public bool SelectedReportSettingsAvailable { get; set; }
    public string SelectedReportSelectionKind { get; set; } = string.Empty;
    public List<CopperfinStudioNamedValue>? SelectedReportSettings { get; set; }
    public List<CopperfinStudioSnapshotField> Fields { get; set; } = new();
    public CopperfinStudioReportLayout? ReportLayout { get; set; }
    public CopperfinStudioProjectWorkspace? ProjectWorkspace { get; set; }
    public CopperfinStudioSecurityProfile SecurityProfile { get; set; } = new();
    public CopperfinStudioExtensibilityProfile ExtensibilityProfile { get; set; } = new();
    public CopperfinStudioDatabaseFederationProfile DatabaseProfile { get; set; } = new();
    public CopperfinStudioLicenseProfile LicenseProfile { get; set; } = new();
    public List<CopperfinStudioSnapshotObject> Objects { get; set; } = new();
}

internal sealed class CopperfinStudioLicenseProfile
{
    public string State { get; set; } = string.Empty;
    public string LicenseType { get; set; } = string.Empty;
    public string Licensee { get; set; } = string.Empty;
    public int Seats { get; set; }
    public string SubscriptionExpires { get; set; } = string.Empty;
    public int PerpetualMaxMajorVersion { get; set; }
}

internal sealed class CopperfinStudioSnapshotField
{
    public string Name { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty;
    public int Length { get; set; }
    public int DecimalCount { get; set; }
}

internal sealed class CopperfinStudioSnapshotObject
{
    public int RecordIndex { get; set; }
    public bool Deleted { get; set; }
    public string Title { get; set; } = string.Empty;
    public string Subtitle { get; set; } = string.Empty;
    public List<CopperfinStudioSnapshotProperty> Properties { get; set; } = new();
}

internal sealed class CopperfinStudioSnapshotProperty
{
    public string Name { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty;
    public bool IsNull { get; set; }
    public string Value { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioReportLayout
{
    public bool IsLabel { get; set; }
    public string DocumentTitle { get; set; } = string.Empty;
    public int? DocumentTitleFieldIndex { get; set; }
    public int DocumentTitleMemoBlockNumber { get; set; }
    public bool PreviewBoundsAvailable { get; set; }
    public int PreviewBoundsLeft { get; set; }
    public int PreviewBoundsTop { get; set; }
    public int PreviewBoundsRight { get; set; }
    public int PreviewBoundsBottom { get; set; }
    public int PreviewBoundsWidth { get; set; }
    public int PreviewBoundsHeight { get; set; }
    public bool DeletedPreviewBoundsAvailable { get; set; }
    public int DeletedPreviewBoundsLeft { get; set; }
    public int DeletedPreviewBoundsTop { get; set; }
    public int DeletedPreviewBoundsRight { get; set; }
    public int DeletedPreviewBoundsBottom { get; set; }
    public int DeletedPreviewBoundsWidth { get; set; }
    public int DeletedPreviewBoundsHeight { get; set; }
    public List<CopperfinStudioReportGrouping> Groupings { get; set; } = new();
    public List<CopperfinStudioNamedValue> Settings { get; set; } = new();
    public List<CopperfinStudioNamedValue> DeletedSettings { get; set; } = new();
    public List<CopperfinStudioReportSection> Sections { get; set; } = new();
    public List<CopperfinStudioReportSection> DeletedSections { get; set; } = new();
    public List<CopperfinStudioReportLayoutObject> UnplacedObjects { get; set; } = new();
    public List<CopperfinStudioReportLayoutObject> DeletedObjects { get; set; } = new();
}

internal sealed class CopperfinStudioReportGrouping
{
    public int GroupingIndex { get; set; }
    public int NestingDepth { get; set; }
    public string Expression { get; set; } = string.Empty;
    public int? ExpressionFieldIndex { get; set; }
    public int ExpressionMemoBlockNumber { get; set; }
    public string HeaderSectionId { get; set; } = string.Empty;
    public int? HeaderRecordIndex { get; set; }
    public bool HeaderDeleted { get; set; }
    public string FooterSectionId { get; set; } = string.Empty;
    public int? FooterRecordIndex { get; set; }
    public bool FooterDeleted { get; set; }
}

internal sealed class CopperfinStudioNamedValue
{
    public string Name { get; set; } = string.Empty;
    public string Value { get; set; } = string.Empty;
    public int RecordIndex { get; set; }
    public int? FieldIndex { get; set; }
    public int? SourceLineIndex { get; set; }
    public int MemoBlockNumber { get; set; }
}

internal sealed class CopperfinStudioReportSection
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string BandKind { get; set; } = string.Empty;
    public int RecordIndex { get; set; }
    public bool Deleted { get; set; }
    public string? Expression { get; set; }
    public int? ExpressionFieldIndex { get; set; }
    public int ExpressionMemoBlockNumber { get; set; }
    public string Comment { get; set; } = string.Empty;
    public int? CommentFieldIndex { get; set; }
    public int CommentMemoBlockNumber { get; set; }
    public string UserComment { get; set; } = string.Empty;
    public int? UserCommentFieldIndex { get; set; }
    public int UserCommentMemoBlockNumber { get; set; }
    public string NoRepeat { get; set; } = string.Empty;
    public int? NoRepeatFieldIndex { get; set; }
    public int NoRepeatMemoBlockNumber { get; set; }
    public int? SectionIndex { get; set; }
    public int? SectionCount { get; set; }
    public bool GroupingContextAvailable { get; set; }
    public int? GroupingIndex { get; set; }
    public int? GroupingNestingDepth { get; set; }
    public string? GroupRole { get; set; }
    public string? GroupingExpression { get; set; }
    public int? GroupingExpressionFieldIndex { get; set; }
    public int GroupingExpressionMemoBlockNumber { get; set; }
    public string? GroupPartnerSectionId { get; set; }
    public int? GroupPartnerRecordIndex { get; set; }
    public bool GroupPartnerDeleted { get; set; }
    public int Top { get; set; }
    public int Height { get; set; }
    public string PageBreak { get; set; } = string.Empty;
    public int? PageBreakFieldIndex { get; set; }
    public int PageBreakMemoBlockNumber { get; set; }
    public string ColumnBreak { get; set; } = string.Empty;
    public int? ColumnBreakFieldIndex { get; set; }
    public int ColumnBreakMemoBlockNumber { get; set; }
    public string ResetPage { get; set; } = string.Empty;
    public int? ResetPageFieldIndex { get; set; }
    public int ResetPageMemoBlockNumber { get; set; }
    public string EjectBefore { get; set; } = string.Empty;
    public int? EjectBeforeFieldIndex { get; set; }
    public int EjectBeforeMemoBlockNumber { get; set; }
    public string EjectAfter { get; set; } = string.Empty;
    public int? EjectAfterFieldIndex { get; set; }
    public int EjectAfterMemoBlockNumber { get; set; }
    public string Plain { get; set; } = string.Empty;
    public int? PlainFieldIndex { get; set; }
    public int PlainMemoBlockNumber { get; set; }
    public string OnEntryExpression { get; set; } = string.Empty;
    public int? OnEntryExpressionFieldIndex { get; set; }
    public int OnEntryExpressionMemoBlockNumber { get; set; }
    public string OnExitExpression { get; set; } = string.Empty;
    public int? OnExitExpressionFieldIndex { get; set; }
    public int OnExitExpressionMemoBlockNumber { get; set; }
    public int DeletedObjectCount { get; set; }
    public List<CopperfinStudioReportLayoutObject> Objects { get; set; } = new();
}

internal sealed class CopperfinStudioReportLayoutObject
{
    public int RecordIndex { get; set; }
    public bool Deleted { get; set; }
    public string ContainingSectionId { get; set; } = string.Empty;
    public int? ContainingSectionRecordIndex { get; set; }
    public int SectionRelativeTop { get; set; }
    public int SectionRelativeBottom { get; set; }
    public int? SectionObjectIndex { get; set; }
    public int SectionObjectCount { get; set; }
    public string ObjectKind { get; set; } = string.Empty;
    public int? ObjectKindFieldIndex { get; set; }
    public int ObjectKindMemoBlockNumber { get; set; }
    public string Title { get; set; } = string.Empty;
    public string Expression { get; set; } = string.Empty;
    public int? ExpressionFieldIndex { get; set; }
    public int ExpressionMemoBlockNumber { get; set; }
    public string Picture { get; set; } = string.Empty;
    public int? PictureFieldIndex { get; set; }
    public int PictureMemoBlockNumber { get; set; }
    public string PictureAlignment { get; set; } = string.Empty;
    public int Left { get; set; }
    public int Top { get; set; }
    public int Width { get; set; }
    public int Height { get; set; }
    public List<CopperfinStudioNamedValue> Highlights { get; set; } = new();
}

internal sealed class CopperfinStudioProjectWorkspace
{
    public string ProjectTitle { get; set; } = string.Empty;
    public string ProjectKey { get; set; } = string.Empty;
    public string HomeDirectory { get; set; } = string.Empty;
    public string OutputPath { get; set; } = string.Empty;
    public List<CopperfinStudioProjectGroup> Groups { get; set; } = new();
    public List<CopperfinStudioProjectEntry> Entries { get; set; } = new();
    public CopperfinStudioProjectBuildPlan BuildPlan { get; set; } = new();
}

internal sealed class CopperfinStudioProjectGroup
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public int ItemCount { get; set; }
    public int ExcludedCount { get; set; }
    public List<int> RecordIndexes { get; set; } = new();
}

internal sealed class CopperfinStudioProjectEntry
{
    public int RecordIndex { get; set; }
    public string Name { get; set; } = string.Empty;
    public string RelativePath { get; set; } = string.Empty;
    public string TypeCode { get; set; } = string.Empty;
    public string TypeTitle { get; set; } = string.Empty;
    public string GroupId { get; set; } = string.Empty;
    public string GroupTitle { get; set; } = string.Empty;
    public string Key { get; set; } = string.Empty;
    public string Comments { get; set; } = string.Empty;
    public bool Excluded { get; set; }
    public bool MainProgram { get; set; }
    public bool Local { get; set; }
}

internal sealed class CopperfinStudioProjectBuildPlan
{
    public bool Available { get; set; }
    public bool CanBuild { get; set; }
    public string ProjectTitle { get; set; } = string.Empty;
    public string ProjectKey { get; set; } = string.Empty;
    public string HomeDirectory { get; set; } = string.Empty;
    public string OutputPath { get; set; } = string.Empty;
    public string BuildTarget { get; set; } = string.Empty;
    public string StartupItem { get; set; } = string.Empty;
    public int StartupRecordIndex { get; set; }
    public int TotalItems { get; set; }
    public int ExcludedItems { get; set; }
    public bool DebugEnabled { get; set; }
    public bool EncryptEnabled { get; set; }
    public bool SaveCode { get; set; }
    public bool NoLogo { get; set; }
}

internal sealed class CopperfinStudioSecurityProfile
{
    public bool Available { get; set; }
    public bool Optional { get; set; }
    public string Mode { get; set; } = string.Empty;
    public string PackagePolicy { get; set; } = string.Empty;
    public string ManagedInteropPolicy { get; set; } = string.Empty;
    public List<CopperfinStudioSecurityRole> Roles { get; set; } = new();
    public List<CopperfinStudioIdentityProvider> IdentityProviders { get; set; } = new();
    public List<CopperfinStudioSecurityFeature> Features { get; set; } = new();
    public List<string> AuditEvents { get; set; } = new();
    public List<string> HardeningProfiles { get; set; } = new();
}

internal sealed class CopperfinStudioSecurityRole
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public bool DefaultAssignment { get; set; }
    public List<string> PermissionIds { get; set; } = new();
}

internal sealed class CopperfinStudioIdentityProvider
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Kind { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public bool EnabledByDefault { get; set; }
}

internal sealed class CopperfinStudioSecurityFeature
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public bool EnabledByDefault { get; set; }
    public bool Optional { get; set; }
}

internal sealed class CopperfinStudioExtensibilityProfile
{
    public bool Available { get; set; }
    public List<CopperfinStudioLanguageIntegration> Languages { get; set; } = new();
    public List<CopperfinStudioAiFeature> AiFeatures { get; set; } = new();
    public CopperfinStudioDotNetOutputProfile DotNetOutput { get; set; } = new();
    public List<string> Guardrails { get; set; } = new();
}

internal sealed class CopperfinStudioLanguageIntegration
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string IntegrationMode { get; set; } = string.Empty;
    public string TrustBoundary { get; set; } = string.Empty;
    public string OutputStory { get; set; } = string.Empty;
    public bool EnabledByDefault { get; set; }
}

internal sealed class CopperfinStudioAiFeature
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string TrustBoundary { get; set; } = string.Empty;
    public bool EnabledByDefault { get; set; }
}

internal sealed class CopperfinStudioDotNetOutputProfile
{
    public bool Available { get; set; }
    public bool NativeHostExecutables { get; set; }
    public bool ManagedWrappers { get; set; }
    public bool NugetSdk { get; set; }
    public string PrimaryStory { get; set; } = string.Empty;
}

internal sealed class CopperfinStudioDatabaseFederationProfile
{
    public bool Available { get; set; }
    public List<CopperfinStudioDatabaseConnector> Connectors { get; set; } = new();
    public List<CopperfinStudioQueryTranslationPath> QueryPaths { get; set; } = new();
    public List<string> Guardrails { get; set; } = new();
}

internal sealed class CopperfinStudioDatabaseConnector
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Family { get; set; } = string.Empty;
    public string AccessMode { get; set; } = string.Empty;
    public string SchemaShape { get; set; } = string.Empty;
    public string TranslationStory { get; set; } = string.Empty;
    public bool XbaseCommandsFirstClass { get; set; }
    public bool FoxSqlTranslationDirect { get; set; }
    public bool AiQueryPlanningOptional { get; set; }
}

internal sealed class CopperfinStudioQueryTranslationPath
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string SourceShape { get; set; } = string.Empty;
    public string TargetShape { get; set; } = string.Empty;
    public string Complexity { get; set; } = string.Empty;
    public string Strategy { get; set; } = string.Empty;
    public bool DeterministicFirst { get; set; }
    public bool AiOptional { get; set; }
}

internal sealed class CopperfinStudioSnapshotResult
{
    public bool Success { get; set; }
    public string Error { get; set; } = string.Empty;
    public CopperfinStudioSnapshotDocument? Document { get; set; }
}

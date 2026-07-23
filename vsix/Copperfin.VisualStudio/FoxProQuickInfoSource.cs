// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.ComponentModel.Composition;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.Language.Intellisense;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Utilities;

namespace Copperfin.VisualStudio;

[Export(typeof(IAsyncQuickInfoSourceProvider))]
[ContentType(FoxProContentTypeDefinitions.ContentTypeName)]
[Name("Copperfin FoxPro Quick Info")]
internal sealed class FoxProQuickInfoSourceProvider : IAsyncQuickInfoSourceProvider
{
    [Import]
    internal Microsoft.VisualStudio.Text.ITextDocumentFactoryService TextDocumentFactoryService = null!;

    public IAsyncQuickInfoSource TryCreateQuickInfoSource(ITextBuffer textBuffer)
    {
        return new FoxProQuickInfoSource(textBuffer, TextDocumentFactoryService);
    }
}

internal sealed class FoxProQuickInfoSource : IAsyncQuickInfoSource
{
    private readonly ITextBuffer textBuffer;
    private readonly Microsoft.VisualStudio.Text.ITextDocumentFactoryService textDocumentFactoryService;
    private bool disposed;

    public FoxProQuickInfoSource(
        ITextBuffer textBuffer,
        Microsoft.VisualStudio.Text.ITextDocumentFactoryService textDocumentFactoryService)
    {
        this.textBuffer = textBuffer;
        this.textDocumentFactoryService = textDocumentFactoryService;
    }

    public Task<QuickInfoItem?> GetQuickInfoItemAsync(
        IAsyncQuickInfoSession session,
        CancellationToken cancellationToken)
    {
        if (disposed || cancellationToken.IsCancellationRequested)
        {
            return Task.FromResult<QuickInfoItem?>(null);
        }

        var triggerPoint = session.GetTriggerPoint(textBuffer.CurrentSnapshot);
        if (triggerPoint is null)
        {
            return Task.FromResult<QuickInfoItem?>(null);
        }

        var snapshot = triggerPoint.Value.Snapshot;
        var position = triggerPoint.Value.Position;
        var span = FoxProTextUtilities.FindTokenSpan(snapshot, position);
        if (span.Length == 0)
        {
            return Task.FromResult<QuickInfoItem?>(null);
        }

        var token = snapshot.GetText(span);
        if (string.IsNullOrWhiteSpace(token))
        {
            return Task.FromResult<QuickInfoItem?>(null);
        }

        cancellationToken.ThrowIfCancellationRequested();
        var description = FoxProIntelliSenseCatalog.DescribeToken(TryGetFilePath(), token);
        if (string.IsNullOrWhiteSpace(description))
        {
            return Task.FromResult<QuickInfoItem?>(null);
        }

        var applicableToSpan = snapshot.CreateTrackingSpan(span, SpanTrackingMode.EdgeInclusive);
        return Task.FromResult<QuickInfoItem?>(new QuickInfoItem(
            applicableToSpan,
            $"{token}\r\n{description}"));
    }

    public void Dispose()
    {
        disposed = true;
    }

    private string? TryGetFilePath()
    {
        return textDocumentFactoryService.TryGetTextDocument(textBuffer, out var document)
            ? document.FilePath
            : null;
    }
}

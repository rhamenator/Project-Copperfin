using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio.Language.Intellisense;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Utilities;

namespace Copperfin.VisualStudio;

[Export(typeof(IQuickInfoSourceProvider))]
[ContentType(FoxProContentTypeDefinitions.ContentTypeName)]
[Name("Copperfin FoxPro Quick Info")]
internal sealed class FoxProQuickInfoSourceProvider : IQuickInfoSourceProvider
{
    [Import]
    internal Microsoft.VisualStudio.Text.ITextDocumentFactoryService TextDocumentFactoryService = null!;

    public IQuickInfoSource TryCreateQuickInfoSource(ITextBuffer textBuffer)
    {
        return new FoxProQuickInfoSource(textBuffer, TextDocumentFactoryService);
    }
}

internal sealed class FoxProQuickInfoSource : IQuickInfoSource
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

    public void AugmentQuickInfoSession(IQuickInfoSession session, IList<object> quickInfoContent, out ITrackingSpan applicableToSpan)
    {
        applicableToSpan = null!;
        if (disposed)
        {
            return;
        }

        var triggerPoint = session.GetTriggerPoint(textBuffer.CurrentSnapshot);
        if (triggerPoint is null)
        {
            return;
        }

        var snapshot = triggerPoint.Value.Snapshot;
        var position = triggerPoint.Value.Position;
        var span = FindTokenSpan(snapshot, position);
        if (span.Length == 0)
        {
            return;
        }

        var token = snapshot.GetText(span);
        if (string.IsNullOrWhiteSpace(token))
        {
            return;
        }

        var description = FoxProIntelliSenseCatalog.DescribeToken(TryGetFilePath(), token);
        if (string.IsNullOrWhiteSpace(description))
        {
            return;
        }

        applicableToSpan = snapshot.CreateTrackingSpan(span, SpanTrackingMode.EdgeInclusive);
        quickInfoContent.Add($"{token}\r\n{description}");
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

    private static Span FindTokenSpan(ITextSnapshot snapshot, int position)
    {
        if (snapshot.Length == 0)
        {
            return new Span(0, 0);
        }

        var adjustedPosition = Math.Max(0, Math.Min(position, snapshot.Length - 1));
        if (!IsTokenCharacter(snapshot[adjustedPosition]) && adjustedPosition > 0 && IsTokenCharacter(snapshot[adjustedPosition - 1]))
        {
            adjustedPosition--;
        }

        if (!IsTokenCharacter(snapshot[adjustedPosition]))
        {
            return new Span(position, 0);
        }

        var start = adjustedPosition;
        while (start > 0 && IsTokenCharacter(snapshot[start - 1]))
        {
            start--;
        }

        var end = adjustedPosition;
        while (end < snapshot.Length && IsTokenCharacter(snapshot[end]))
        {
            end++;
        }

        return Span.FromBounds(start, end);
    }

    private static bool IsTokenCharacter(char value)
    {
        return char.IsLetterOrDigit(value) || value == '_' || value == '.' || value == '#';
    }
}

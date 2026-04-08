using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Linq;
using Microsoft.VisualStudio.Editor;
using Microsoft.VisualStudio.Language.Intellisense;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Text.Operations;
using Microsoft.VisualStudio.Utilities;

namespace Copperfin.VisualStudio;

[Export(typeof(ICompletionSourceProvider))]
[ContentType(FoxProContentTypeDefinitions.ContentTypeName)]
[Name("Copperfin FoxPro Completion")]
internal sealed class FoxProCompletionSourceProvider : ICompletionSourceProvider
{
    [Import]
    internal ITextDocumentFactoryService TextDocumentFactoryService = null!;

    public ICompletionSource TryCreateCompletionSource(ITextBuffer textBuffer)
    {
        return new FoxProCompletionSource(textBuffer, TextDocumentFactoryService);
    }
}

internal sealed class FoxProCompletionSource : ICompletionSource
{
    private readonly ITextBuffer textBuffer;
    private readonly ITextDocumentFactoryService textDocumentFactoryService;
    private bool disposed;

    public FoxProCompletionSource(ITextBuffer textBuffer, ITextDocumentFactoryService textDocumentFactoryService)
    {
        this.textBuffer = textBuffer;
        this.textDocumentFactoryService = textDocumentFactoryService;
    }

    public void AugmentCompletionSession(ICompletionSession session, IList<CompletionSet> completionSets)
    {
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
        var tokenPrefix = snapshot.GetText(span);
        var line = snapshot.GetLineFromPosition(position);
        var linePrefix = snapshot.GetText(Span.FromBounds(line.Start.Position, position));
        var filePath = TryGetFilePath();

        var completions = FoxProIntelliSenseCatalog.BuildEntries(filePath, linePrefix, tokenPrefix)
            .Select(entry => new Completion(entry.DisplayText, entry.InsertionText, entry.Description, null, entry.Kind))
            .ToList();

        if (completions.Count == 0)
        {
            return;
        }

        var applicableTo = snapshot.CreateTrackingSpan(span, SpanTrackingMode.EdgeInclusive);
        completionSets.Add(new CompletionSet(
            "CopperfinFoxPro",
            "Copperfin FoxPro",
            applicableTo,
            completions,
            Enumerable.Empty<Completion>()));
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
        var start = position;
        while (start > 0 && IsTokenCharacter(snapshot[start - 1]))
        {
            start--;
        }

        var end = position;
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

[Export(typeof(IVsTextViewCreationListener))]
[ContentType(FoxProContentTypeDefinitions.ContentTypeName)]
[TextViewRole(PredefinedTextViewRoles.Editable)]
internal sealed class FoxProCompletionHandlerProvider : IVsTextViewCreationListener
{
    [Import]
    internal IVsEditorAdaptersFactoryService AdapterService = null!;

    [Import]
    internal ICompletionBroker CompletionBroker = null!;

    public void VsTextViewCreated(Microsoft.VisualStudio.TextManager.Interop.IVsTextView textViewAdapter)
    {
        var textView = AdapterService.GetWpfTextView(textViewAdapter);
        if (textView is null || textView.Properties.ContainsProperty(typeof(FoxProCompletionCommandHandler)))
        {
            return;
        }

        var commandHandler = new FoxProCompletionCommandHandler(textViewAdapter, textView, CompletionBroker);
        textView.Properties.AddProperty(typeof(FoxProCompletionCommandHandler), commandHandler);
    }
}

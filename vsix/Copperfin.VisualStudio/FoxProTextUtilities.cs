// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

using System;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;

namespace Copperfin.VisualStudio;

internal readonly struct FoxProInvocationContext
{
    public FoxProInvocationContext(string invocationName, Span invocationSpan, int parameterIndex)
    {
        InvocationName = invocationName;
        InvocationSpan = invocationSpan;
        ParameterIndex = parameterIndex;
    }

    public string InvocationName { get; }
    public Span InvocationSpan { get; }
    public int ParameterIndex { get; }
}

internal static class FoxProTextUtilities
{
    public static Span FindTokenSpan(ITextSnapshot snapshot, int position)
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

    public static string? TryGetTokenAtCaret(ITextView textView, out Span span)
    {
        var snapshot = textView.TextSnapshot;
        var caretPosition = textView.Caret.Position.BufferPosition.Position;
        span = FindTokenSpan(snapshot, caretPosition);
        if (span.Length == 0)
        {
            return null;
        }

        var token = snapshot.GetText(span);
        return string.IsNullOrWhiteSpace(token) ? null : token;
    }

    public static bool TryFindInvocationContext(ITextSnapshot snapshot, int position, out FoxProInvocationContext context)
    {
        context = default;
        if (snapshot.Length == 0)
        {
            return false;
        }

        var text = snapshot.GetText();
        if (!FoxProInvocationParser.TryParse(text, position, out var parsed))
        {
            return false;
        }

        context = new FoxProInvocationContext(
            parsed.InvocationName,
            new Span(parsed.InvocationStart, parsed.InvocationLength),
            parsed.ParameterIndex);
        return true;
    }

    public static bool IsTokenCharacter(char value)
    {
        return char.IsLetterOrDigit(value) || value == '_' || value == '.' || value == '#';
    }

}

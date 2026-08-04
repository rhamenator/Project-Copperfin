// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;

namespace Copperfin.VisualStudio;

internal readonly struct FoxProInvocationParseResult
{
    public FoxProInvocationParseResult(string invocationName, int invocationStart, int invocationLength, int parameterIndex)
    {
        InvocationName = invocationName;
        InvocationStart = invocationStart;
        InvocationLength = invocationLength;
        ParameterIndex = parameterIndex;
    }

    public string InvocationName { get; }
    public int InvocationStart { get; }
    public int InvocationLength { get; }
    public int ParameterIndex { get; }
}

internal static class FoxProInvocationParser
{
    public static bool TryParse(string text, int position, out FoxProInvocationParseResult result)
    {
        result = default;
        if (string.IsNullOrEmpty(text))
        {
            return false;
        }

        var scanPosition = Math.Max(0, Math.Min(position, text.Length));
        var parentheses = new Stack<int>();
        var state = LexicalState.Code;
        var lineHasOnlyWhitespace = true;
        for (var index = 0; index < scanPosition; index++)
        {
            var value = text[index];
            if (state != LexicalState.Code)
            {
                if (state == LexicalState.LineComment)
                {
                    if (value == '\r' || value == '\n')
                    {
                        state = LexicalState.Code;
                        lineHasOnlyWhitespace = true;
                    }
                    continue;
                }

                if (value == '\r' || value == '\n')
                {
                    lineHasOnlyWhitespace = true;
                }
                if (value == QuoteCharacter(state))
                {
                    if (index + 1 < scanPosition && text[index + 1] == value)
                    {
                        index++;
                    }
                    else
                    {
                        state = LexicalState.Code;
                    }
                }
                continue;
            }

            if (value == '\r' || value == '\n')
            {
                lineHasOnlyWhitespace = true;
                continue;
            }
            if (char.IsWhiteSpace(value))
            {
                continue;
            }
            if ((value == '\'' || value == '"'))
            {
                state = QuotedLiteral(value);
                lineHasOnlyWhitespace = false;
                continue;
            }
            if (value == '&' && index + 1 < scanPosition && text[index + 1] == '&')
            {
                state = LexicalState.LineComment;
                index++;
                continue;
            }
            if (value == '*' && lineHasOnlyWhitespace)
            {
                state = LexicalState.LineComment;
                continue;
            }

            lineHasOnlyWhitespace = false;
            if (value == '(')
            {
                parentheses.Push(index);
            }
            else if (value == ')' && parentheses.Count > 0)
            {
                parentheses.Pop();
            }
        }

        if (state != LexicalState.Code || parentheses.Count == 0)
        {
            return false;
        }

        var openParen = parentheses.Peek();
        var end = openParen;
        while (end > 0 && char.IsWhiteSpace(text[end - 1]))
        {
            end--;
        }

        var start = end;
        while (start > 0 && IsTokenCharacter(text[start - 1]))
        {
            start--;
        }

        if (start == end)
        {
            return false;
        }

        result = new FoxProInvocationParseResult(
            text.Substring(start, end - start),
            start,
            end - start,
            CountParameters(text, openParen + 1, scanPosition));
        return true;
    }

    private static bool IsTokenCharacter(char value)
    {
        return char.IsLetterOrDigit(value) || value == '_' || value == '.' || value == '#';
    }

    private static int CountParameters(string text, int start, int end)
    {
        var parameterIndex = 0;
        var depth = 0;
        var state = LexicalState.Code;
        var lineHasOnlyWhitespace = true;
        for (var index = start; index < end && index < text.Length; index++)
        {
            var value = text[index];
            if (state != LexicalState.Code)
            {
                if (state == LexicalState.LineComment)
                {
                    if (value == '\r' || value == '\n')
                    {
                        state = LexicalState.Code;
                        lineHasOnlyWhitespace = true;
                    }
                    continue;
                }

                if (value == QuoteCharacter(state))
                {
                    if (index + 1 < end && text[index + 1] == value)
                    {
                        index++;
                    }
                    else
                    {
                        state = LexicalState.Code;
                    }
                }
                continue;
            }

            if (value == '\r' || value == '\n')
            {
                lineHasOnlyWhitespace = true;
                continue;
            }
            if (char.IsWhiteSpace(value))
            {
                continue;
            }
            if (value == '\'' || value == '"')
            {
                state = QuotedLiteral(value);
                lineHasOnlyWhitespace = false;
                continue;
            }
            if (value == '&' && index + 1 < end && text[index + 1] == '&')
            {
                state = LexicalState.LineComment;
                index++;
                continue;
            }
            if (value == '*' && lineHasOnlyWhitespace)
            {
                state = LexicalState.LineComment;
                continue;
            }

            lineHasOnlyWhitespace = false;
            if (value == '(')
            {
                depth++;
            }
            else if (value == ')' && depth > 0)
            {
                depth--;
            }
            else if (value == ',' && depth == 0)
            {
                parameterIndex++;
            }
        }

        return parameterIndex;
    }

    private enum LexicalState
    {
        Code,
        LineComment,
        SingleQuoted,
        DoubleQuoted
    }

    private static LexicalState QuotedLiteral(char quote)
    {
        return quote == '\'' ? LexicalState.SingleQuoted : LexicalState.DoubleQuoted;
    }

    private static char QuoteCharacter(LexicalState state)
    {
        return state == LexicalState.SingleQuoted ? '\'' : '"';
    }
}

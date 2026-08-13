// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.Text;

namespace Copperfin.VisualStudio;

// RQ-CF-AGENT-003 requires duplicate JSON members to fail before a serializer
// can collapse them. This bounded recognizer validates member uniqueness at
// every nesting level; JavaScriptSerializer still owns typed deserialization.
internal sealed class CopperfinStrictJsonMemberValidator
{
    private const int MaximumDepth = 64;
    private readonly string _json;
    private int _offset;

    private CopperfinStrictJsonMemberValidator(string json) => _json = json;

    public static bool HasValidUniqueMembers(string? json)
    {
        if (json is null || json.Length == 0 || json.Length > 1024 * 1024) return false;
        var validator = new CopperfinStrictJsonMemberValidator(json);
        return validator.ParseValue(0) && validator.SkipWhitespace() &&
               validator._offset == validator._json.Length;
    }

    private bool ParseValue(int depth)
    {
        if (depth > MaximumDepth || !SkipWhitespace() || _offset >= _json.Length) return false;
        return _json[_offset] switch
        {
            '{' => ParseObject(depth + 1),
            '[' => ParseArray(depth + 1),
            '"' => TryParseString(out _),
            't' => ParseLiteral("true"),
            'f' => ParseLiteral("false"),
            'n' => ParseLiteral("null"),
            '-' => ParseNumber(),
            >= '0' and <= '9' => ParseNumber(),
            _ => false
        };
    }

    private bool ParseObject(int depth)
    {
        ++_offset;
        if (!SkipWhitespace()) return false;
        if (Consume('}')) return true;
        var members = new HashSet<string>(StringComparer.Ordinal);
        while (true)
        {
            if (!TryParseString(out var member) || !members.Add(member) ||
                !SkipWhitespace() || !Consume(':') || !ParseValue(depth) || !SkipWhitespace()) return false;
            if (Consume('}')) return true;
            if (!Consume(',') || !SkipWhitespace()) return false;
        }
    }

    private bool ParseArray(int depth)
    {
        ++_offset;
        if (!SkipWhitespace()) return false;
        if (Consume(']')) return true;
        while (true)
        {
            if (!ParseValue(depth) || !SkipWhitespace()) return false;
            if (Consume(']')) return true;
            if (!Consume(',') || !SkipWhitespace()) return false;
        }
    }

    private bool TryParseString(out string value)
    {
        value = string.Empty;
        if (!Consume('"')) return false;
        var decoded = new StringBuilder();
        while (_offset < _json.Length)
        {
            var current = _json[_offset++];
            if (current == '"')
            {
                value = decoded.ToString();
                return true;
            }
            if (current < 0x20) return false;
            if (current != '\\')
            {
                decoded.Append(current);
                continue;
            }
            if (_offset >= _json.Length) return false;
            switch (_json[_offset++])
            {
                case '"': decoded.Append('"'); break;
                case '\\': decoded.Append('\\'); break;
                case '/': decoded.Append('/'); break;
                case 'b': decoded.Append('\b'); break;
                case 'f': decoded.Append('\f'); break;
                case 'n': decoded.Append('\n'); break;
                case 'r': decoded.Append('\r'); break;
                case 't': decoded.Append('\t'); break;
                case 'u':
                    if (!TryParseHexCodeUnit(out var codeUnit)) return false;
                    decoded.Append(codeUnit);
                    break;
                default: return false;
            }
        }
        return false;
    }

    private bool TryParseHexCodeUnit(out char value)
    {
        value = '\0';
        if (_offset + 4 > _json.Length) return false;
        var result = 0;
        for (var index = 0; index < 4; ++index)
        {
            var digit = HexValue(_json[_offset++]);
            if (digit < 0) return false;
            result = (result << 4) | digit;
        }
        value = (char)result;
        return true;
    }

    private static int HexValue(char value)
    {
        if (value >= '0' && value <= '9') return value - '0';
        if (value >= 'a' && value <= 'f') return value - 'a' + 10;
        if (value >= 'A' && value <= 'F') return value - 'A' + 10;
        return -1;
    }

    private bool ParseNumber()
    {
        Consume('-');
        if (_offset >= _json.Length) return false;
        if (_json[_offset] == '0') ++_offset;
        else if (_json[_offset] >= '1' && _json[_offset] <= '9')
        {
            do { ++_offset; }
            while (_offset < _json.Length && _json[_offset] >= '0' && _json[_offset] <= '9');
        }
        else return false;
        if (Consume('.') && !ConsumeDigits()) return false;
        if (_offset < _json.Length && (_json[_offset] == 'e' || _json[_offset] == 'E'))
        {
            ++_offset;
            if (_offset < _json.Length && (_json[_offset] == '+' || _json[_offset] == '-')) ++_offset;
            if (!ConsumeDigits()) return false;
        }
        return true;
    }

    private bool ConsumeDigits()
    {
        var start = _offset;
        while (_offset < _json.Length && _json[_offset] >= '0' && _json[_offset] <= '9') ++_offset;
        return _offset > start;
    }

    private bool ParseLiteral(string literal)
    {
        if (_offset + literal.Length > _json.Length ||
            !string.Equals(_json.Substring(_offset, literal.Length), literal, StringComparison.Ordinal)) return false;
        _offset += literal.Length;
        return true;
    }

    private bool SkipWhitespace()
    {
        while (_offset < _json.Length)
        {
            var value = _json[_offset];
            if (value != ' ' && value != '\t' && value != '\r' && value != '\n') break;
            ++_offset;
        }
        return true;
    }

    private bool Consume(char expected)
    {
        if (_offset >= _json.Length || _json[_offset] != expected) return false;
        ++_offset;
        return true;
    }
}

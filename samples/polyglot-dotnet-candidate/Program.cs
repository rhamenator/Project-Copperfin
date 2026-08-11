// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace Copperfin.PolyglotCandidate;

internal static class Program
{
    private const int MaximumDocumentBytes = 1024 * 1024;
    private const int MaximumDepth = 64;
    private const string EnvelopeVersion = "1.0";
    private const string ProtocolVersion = "1.0.0";
    private const string CapabilityId = "samples.dotnet.add-v1";

    private readonly record struct InvocationIdentity(
        string CapabilityId,
        string CorrelationId,
        string ProtocolVersion);

    public static int Main()
    {
        try
        {
            var documentBytes = ReadBoundedStandardInput();
            if (documentBytes is null ||
                !TryParseInvocation(
                    documentBytes,
                    out var identity,
                    out var left,
                    out var right,
                    out var argumentsValid))
            {
                return 2;
            }

            if (!argumentsValid)
            {
                WriteError(identity, "sample.dotnet.invalid_arguments",
                    "The sample requires exact signed 64-bit left and right values.");
                return 0;
            }

            try
            {
                WriteSuccess(identity, checked(left + right));
                WriteBenchmarkMetricsIfRequested();
            }
            catch (OverflowException)
            {
                WriteError(identity, "sample.dotnet.overflow",
                    "The requested signed 64-bit addition overflowed.");
            }
            return 0;
        }
        catch (JsonException)
        {
            return 2;
        }
        catch
        {
            Console.Error.Write("polyglot candidate failed");
            return 3;
        }
    }

    private static void WriteBenchmarkMetricsIfRequested()
    {
        if (Environment.GetEnvironmentVariable(
                "COPPERFIN_BENCHMARK_SELF_METRICS") != "1")
        {
            return;
        }
        var workingSetKiB = checked((Environment.WorkingSet + 1023L) / 1024L);
        Console.Error.Write($"COPPERFIN_WORKING_SET_KIB={workingSetKiB}\n");
    }

    private static byte[]? ReadBoundedStandardInput()
    {
        using var input = Console.OpenStandardInput();
        using var document = new MemoryStream();
        var buffer = new byte[8192];
        while (true)
        {
            var count = input.Read(buffer, 0, buffer.Length);
            if (count == 0)
            {
                break;
            }
            if (document.Length + count > MaximumDocumentBytes)
            {
                return null;
            }
            document.Write(buffer, 0, count);
        }
        return document.ToArray();
    }

    private static bool TryParseInvocation(
        byte[] documentBytes,
        out InvocationIdentity identity,
        out long left,
        out long right,
        out bool argumentsValid)
    {
        identity = default;
        left = 0;
        right = 0;
        argumentsValid = false;

        using JsonDocument document = JsonDocument.Parse(
            documentBytes,
            new JsonDocumentOptions
            {
                AllowTrailingCommas = false,
                CommentHandling = JsonCommentHandling.Disallow,
                MaxDepth = MaximumDepth,
            });
        var root = document.RootElement;
        if (root.ValueKind != JsonValueKind.Object ||
            !HasExactUniqueProperties(
                root,
                "envelope_version",
                "kind",
                "capability_id",
                "correlation_id",
                "protocol_version",
                "arguments"))
        {
            return false;
        }

        var envelopeVersion = RequiredString(root, "envelope_version");
        var kind = RequiredString(root, "kind");
        var capabilityId = RequiredString(root, "capability_id");
        var correlationId = RequiredString(root, "correlation_id");
        var protocolVersion = RequiredString(root, "protocol_version");
        if (capabilityId is null || correlationId is null || protocolVersion is null ||
            !string.Equals(envelopeVersion, EnvelopeVersion, StringComparison.Ordinal) ||
            !string.Equals(kind, "invocation", StringComparison.Ordinal) ||
            !string.Equals(capabilityId, CapabilityId, StringComparison.Ordinal) ||
            correlationId.Length == 0 ||
            !string.Equals(protocolVersion, ProtocolVersion, StringComparison.Ordinal))
        {
            return false;
        }

        identity = new InvocationIdentity(capabilityId, correlationId, protocolVersion);
        var arguments = root.GetProperty("arguments");
        if (arguments.ValueKind != JsonValueKind.Object ||
            !HasExactUniqueProperties(arguments, "left", "right"))
        {
            return true;
        }

        argumentsValid =
            arguments.GetProperty("left").TryGetInt64(out left) &&
            arguments.GetProperty("right").TryGetInt64(out right);
        return true;
    }

    private static string? RequiredString(JsonElement value, string propertyName)
    {
        var property = value.GetProperty(propertyName);
        return property.ValueKind == JsonValueKind.String ? property.GetString() : null;
    }

    private static bool HasExactUniqueProperties(
        JsonElement value,
        params string[] requiredNames)
    {
        var required = new HashSet<string>(requiredNames, StringComparer.Ordinal);
        var found = new HashSet<string>(StringComparer.Ordinal);
        foreach (var property in value.EnumerateObject())
        {
            if (!required.Contains(property.Name) || !found.Add(property.Name))
            {
                return false;
            }
        }
        return found.Count == required.Count;
    }

    private static void WriteSuccess(InvocationIdentity identity, long sum)
    {
        WriteEnvelope(writer =>
        {
            WriteIdentity(writer, identity, "success");
            writer.WritePropertyName("payload");
            writer.WriteStartObject();
            writer.WriteNumber("sum", sum);
            writer.WriteEndObject();
        });
    }

    private static void WriteError(
        InvocationIdentity identity,
        string code,
        string message)
    {
        WriteEnvelope(writer =>
        {
            WriteIdentity(writer, identity, "error");
            writer.WritePropertyName("error");
            writer.WriteStartObject();
            writer.WriteString("code", code);
            writer.WriteString("message", message);
            writer.WriteBoolean("retryable", false);
            writer.WriteEndObject();
        });
    }

    private static void WriteIdentity(
        Utf8JsonWriter writer,
        InvocationIdentity identity,
        string kind)
    {
        writer.WriteString("envelope_version", EnvelopeVersion);
        writer.WriteString("kind", kind);
        writer.WriteString("capability_id", identity.CapabilityId);
        writer.WriteString("correlation_id", identity.CorrelationId);
        writer.WriteString("protocol_version", identity.ProtocolVersion);
    }

    private static void WriteEnvelope(Action<Utf8JsonWriter> writeBody)
    {
        using var output = new MemoryStream();
        using (var writer = new Utf8JsonWriter(output))
        {
            writer.WriteStartObject();
            writeBody(writer);
            writer.WriteEndObject();
        }
        Console.OpenStandardOutput().Write(output.GetBuffer(), 0, checked((int)output.Length));
    }
}

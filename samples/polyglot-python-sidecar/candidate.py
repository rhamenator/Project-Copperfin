# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

"""One-request Python sidecar for the Copperfin v1 polyglot envelope."""

import json
import sys


MAXIMUM_DOCUMENT_BYTES = 1024 * 1024
SIGNED_64_MINIMUM = -(2**63)
SIGNED_64_MAXIMUM = 2**63 - 1
CAPABILITY_ID = "samples.python.add-v1"
PROTOCOL_VERSION = "1.0.0"


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("duplicate JSON member")
        result[key] = value
    return result


def exact_keys(value, expected):
    return isinstance(value, dict) and set(value) == set(expected)


def signed_64(value):
    return (
        isinstance(value, int)
        and not isinstance(value, bool)
        and SIGNED_64_MINIMUM <= value <= SIGNED_64_MAXIMUM
    )


def emit(document):
    encoded = json.dumps(
        document,
        ensure_ascii=False,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    sys.stdout.buffer.write(encoded)


def main():
    raw = sys.stdin.buffer.read(MAXIMUM_DOCUMENT_BYTES + 1)
    if len(raw) > MAXIMUM_DOCUMENT_BYTES:
        return 2
    try:
        request = json.loads(
            raw.decode("utf-8", errors="strict"),
            object_pairs_hook=unique_object,
            parse_constant=lambda _value: (_ for _ in ()).throw(
                ValueError("non-finite number")
            ),
        )
    except (UnicodeDecodeError, json.JSONDecodeError, ValueError, RecursionError):
        return 2

    required = (
        "envelope_version",
        "kind",
        "capability_id",
        "correlation_id",
        "protocol_version",
        "arguments",
    )
    if (
        not exact_keys(request, required)
        or request["envelope_version"] != "1.0"
        or request["kind"] != "invocation"
        or request["capability_id"] != CAPABILITY_ID
        or not isinstance(request["correlation_id"], str)
        or not request["correlation_id"]
        or request["protocol_version"] != PROTOCOL_VERSION
    ):
        return 2

    identity = {
        "envelope_version": "1.0",
        "capability_id": CAPABILITY_ID,
        "correlation_id": request["correlation_id"],
        "protocol_version": PROTOCOL_VERSION,
    }
    arguments = request["arguments"]
    if (
        not exact_keys(arguments, ("left", "right"))
        or not signed_64(arguments["left"])
        or not signed_64(arguments["right"])
    ):
        emit(
            {
                **identity,
                "kind": "error",
                "error": {
                    "code": "sample.python.invalid_arguments",
                    "message": "The sample requires exact signed 64-bit left and right values.",
                    "retryable": False,
                },
            }
        )
        return 0

    total = arguments["left"] + arguments["right"]
    if not signed_64(total):
        emit(
            {
                **identity,
                "kind": "error",
                "error": {
                    "code": "sample.python.overflow",
                    "message": "The requested signed 64-bit addition overflowed.",
                    "retryable": False,
                },
            }
        )
        return 0

    emit({**identity, "kind": "success", "payload": {"sum": total}})
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

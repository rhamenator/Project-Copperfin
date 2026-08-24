#!/usr/bin/env python3
"""Post and read race-safe Project Copperfin agent-channel messages.

Messages are individual immutable files, rather than competing appends to a
shared JSONL file.  Git can therefore merge independent messages without
renumbering or replacing another agent's work.
"""

from __future__ import annotations

import argparse
import datetime as dt
import json
import os
from pathlib import Path
import re
import sys
import time
from typing import Any, Dict, List, Optional
import uuid


def repository_root(argument: Optional[str]) -> Path:
    return Path(argument).resolve() if argument else Path(__file__).resolve().parents[1]


def message_directory(root: Path) -> Path:
    return root / ".agent-channel" / "messages"


def cursor_path(root: Path, agent: str) -> Path:
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_.-]*", agent):
        raise ValueError(f"invalid agent name: {agent!r}")
    return root / ".agent-channel" / "cursors" / f"{agent}.json"


def cursor_lock_path(root: Path, agent: str) -> Path:
    return cursor_path(root, agent).with_suffix(".lock")


def parse_recipients(value: str) -> List[str]:
    return [recipient.strip() for recipient in value.split(",") if recipient.strip()]


def validate_message(path: Path, message: Any) -> Dict[str, Any]:
    if not isinstance(message, dict):
        raise ValueError(f"agent-channel message must be an object: {path}")
    required = {"message_id", "ts", "from", "to", "type", "text"}
    if set(message) != required:
        raise ValueError(f"invalid agent-channel message fields: {path}")
    if not isinstance(message["message_id"], str):
        raise ValueError(f"agent-channel message_id must be a string: {path}")
    try:
        canonical_id = str(uuid.UUID(message["message_id"]))
    except ValueError as error:
        raise ValueError(f"invalid agent-channel message_id: {path}") from error
    if message["message_id"] != canonical_id or path.stem != canonical_id:
        raise ValueError(f"agent-channel filename does not match message_id: {path}")
    if not isinstance(message["ts"], str):
        raise ValueError(f"agent-channel timestamp must be a string: {path}")
    try:
        dt.datetime.fromisoformat(message["ts"].replace("Z", "+00:00"))
    except ValueError as error:
        raise ValueError(f"invalid agent-channel timestamp: {path}") from error
    for field in ("from", "type", "text"):
        if not isinstance(message[field], str) or (field != "text" and not message[field]):
            raise ValueError(f"agent-channel {field} must be a valid string: {path}")
    recipients = message["to"]
    if not isinstance(recipients, list) or not recipients or any(
        not isinstance(recipient, str) or not recipient for recipient in recipients
    ):
        raise ValueError(f"agent-channel recipients must be a non-empty string list: {path}")
    if len(set(recipients)) != len(recipients):
        raise ValueError(f"agent-channel recipients must be unique: {path}")
    return message


def read_messages(root: Path) -> List[Dict[str, Any]]:
    messages: List[Dict[str, Any]] = []
    for path in message_directory(root).glob("*.json"):
        try:
            message = json.loads(path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as error:
            raise ValueError(f"invalid agent-channel message: {path}") from error
        messages.append(validate_message(path, message))
    return sorted(messages, key=lambda message: (str(message["ts"]), str(message["message_id"])))


def read_cursor(root: Path, agent: str) -> List[str]:
    path = cursor_path(root, agent)
    if not path.exists():
        return []
    try:
        cursor = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        raise ValueError(f"invalid agent-channel cursor: {path}") from error
    if not isinstance(cursor, dict) or set(cursor) != {"schema_version", "processed_message_ids"} or cursor["schema_version"] != 1:
        raise ValueError(f"invalid agent-channel cursor schema: {path}")
    identifiers = cursor["processed_message_ids"]
    if not isinstance(identifiers, list) or any(not isinstance(identifier, str) for identifier in identifiers):
        raise ValueError(f"invalid agent-channel cursor identifiers: {path}")
    return identifiers


def write_cursor(root: Path, agent: str, identifiers: List[str]) -> None:
    path = cursor_path(root, agent)
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = {"schema_version": 1, "processed_message_ids": identifiers}
    temporary_path = path.with_suffix(".tmp")
    temporary_path.write_text(json.dumps(payload, ensure_ascii=False, sort_keys=True) + "\n", encoding="utf-8")
    temporary_path.replace(path)


def acquire_cursor_lock(root: Path, agent: str) -> Path:
    path = cursor_lock_path(root, agent)
    path.parent.mkdir(parents=True, exist_ok=True)
    for _ in range(100):
        try:
            descriptor = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL)
        except FileExistsError:
            try:
                if time.time() - path.stat().st_mtime > 30:
                    path.unlink()
                    continue
            except FileNotFoundError:
                continue
            time.sleep(0.05)
            continue
        with os.fdopen(descriptor, "w", encoding="utf-8") as output:
            json.dump({"pid": os.getpid()}, output)
            output.write("\n")
        return path
    raise OSError(f"timed out waiting for agent-channel cursor lock: {path}")


def record_processed(root: Path, agent: str, delivered: List[str]) -> None:
    lock = acquire_cursor_lock(root, agent)
    try:
        known = read_cursor(root, agent)
        write_cursor(root, agent, known + [identifier for identifier in delivered if identifier not in known])
    finally:
        lock.unlink(missing_ok=True)


def command_post(arguments: argparse.Namespace) -> int:
    root = repository_root(arguments.root)
    destination = message_directory(root)
    destination.mkdir(parents=True, exist_ok=True)
    recipients = parse_recipients(arguments.recipient)
    if not recipients:
        raise ValueError("agent-channel recipients must not be empty")
    message_id = str(uuid.uuid4())
    message = {
        "message_id": message_id,
        "ts": dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z"),
        "from": arguments.sender,
        "to": recipients,
        "type": arguments.message_type,
        "text": arguments.text,
    }
    path = destination / f"{message_id}.json"
    validate_message(path, message)
    temporary_path = destination / f".{message_id}.{uuid.uuid4().hex}.tmp"
    try:
        with temporary_path.open("x", encoding="utf-8") as output:
            json.dump(message, output, ensure_ascii=False, sort_keys=True)
            output.write("\n")
        os.link(temporary_path, path)
    finally:
        temporary_path.unlink(missing_ok=True)
    print(path.relative_to(root))
    return 0


def command_read(arguments: argparse.Namespace) -> int:
    root = repository_root(arguments.root)
    processed = set(read_cursor(root, arguments.agent)) if arguments.only_unread else set()
    delivered: List[str] = []
    for message in read_messages(root):
        recipients = message["to"]
        if (arguments.agent in recipients or "both" in recipients) and message["message_id"] not in processed:
            print(json.dumps(message, ensure_ascii=False, sort_keys=True))
            delivered.append(message["message_id"])
    if arguments.mark_read and delivered:
        record_processed(root, arguments.agent, delivered)
    return 0


def command_verify(arguments: argparse.Namespace) -> int:
    for _ in read_messages(repository_root(arguments.root)):
        pass
    return 0


def command_cursor(arguments: argparse.Namespace) -> int:
    root = repository_root(arguments.root)
    print(json.dumps({"processed_message_ids": read_cursor(root, arguments.agent), "schema_version": 1}, sort_keys=True))
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", help="repository root (default: script parent)")
    commands = parser.add_subparsers(dest="command", required=True)

    post = commands.add_parser("post", help="create one immutable message file")
    post.add_argument("--from", dest="sender", required=True)
    post.add_argument("--to", dest="recipient", required=True)
    post.add_argument("--type", dest="message_type", required=True)
    post.add_argument("--text", required=True)
    post.set_defaults(handler=command_post)

    read = commands.add_parser("read", help="print messages addressed to one agent")
    read.add_argument("--agent", required=True)
    read.add_argument("--only-unread", action="store_true", help="omit locally processed messages")
    read.add_argument("--mark-read", action="store_true", help="record delivered messages in the local cursor")
    read.set_defaults(handler=command_read)

    verify = commands.add_parser("verify", help="validate message-file schema and identity")
    verify.set_defaults(handler=command_verify)

    cursor = commands.add_parser("cursor", help="show one agent's local processed-message cursor")
    cursor.add_argument("--agent", required=True)
    cursor.set_defaults(handler=command_cursor)

    arguments = parser.parse_args()
    try:
        return arguments.handler(arguments)
    except (OSError, ValueError) as error:
        print(f"agent-channel: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

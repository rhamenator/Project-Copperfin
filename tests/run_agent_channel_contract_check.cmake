# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(require_text relative_path expected_text description)
    file(READ "${SOURCE_DIR}/${relative_path}" contents)
    string(FIND "${contents}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing ${description}")
    endif()
endfunction()

require_text(".agent-channel/README.md" "messages/<message_id>.json"
    "immutable message-file protocol")
require_text(".agent-channel/README.md" "message_id"
    "stable message identity")
require_text(".agent-channel/README.md" "not commit cursor state"
    "local cursor policy")
require_text("scripts/agent_channel.py" "uuid.uuid4()"
    "unique message identity generator")
require_text("scripts/agent_channel.py" "validate_message(path, message)"
    "post-time message validation")
require_text("scripts/agent_channel.py" "os.link(temporary_path, path)"
    "atomic immutable message publication")
require_text("scripts/agent_channel.py" "def validate_message"
    "strict message schema validation")
require_text("scripts/agent_channel.py" "not isinstance(cursor, dict)"
    "clean invalid-cursor rejection")
require_text("scripts/agent_channel.py" "def record_processed"
    "cursor update serialization")
require_text("scripts/agent_channel.py" "def acquire_cursor_lock"
    "per-agent cursor lock")
require_text("scripts/agent_channel.py" "commands.add_parser(\"cursor\""
    "managed cursor command")
require_text("scripts/agent_channel.py" "--only-unread"
    "unread-message filtering")
require_text(".gitignore" "/.agent-channel/cursors/"
    "untracked cursor state")

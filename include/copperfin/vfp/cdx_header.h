// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

struct CdxHeader {
    std::array<std::uint16_t, 8> raw_words{};
    std::uint32_t root_node_offset = 0;
    std::uint32_t next_free_node_offset = 0;
    std::uint16_t key_length_hint = 0;
    std::uint16_t key_pool_length_hint = 0;
    std::uint16_t page_size = 512;
    std::uint64_t file_size = 0;

    [[nodiscard]] bool looks_like_cdx() const;
};

struct CdxTagDescriptor {
    std::string name_hint;
    std::string key_expression_hint;
    std::string for_expression_hint;
    std::uint32_t tag_page_offset_hint = 0;
    std::uint32_t name_offset_hint = 0;
    std::uint32_t key_expression_offset_hint = 0;
    std::uint32_t for_expression_offset_hint = 0;
    bool inferred_name = false;
    // Persisted creation direction (issue #5358): a single byte at a fixed
    // offset within the tag's own header page, 0x00 for a tag created with
    // ASCENDING (the default) and 0x01 for one created with DESCENDING.
    // Confirmed empirically against a real, fully patched VFP9 install
    // (09.00.0000.7423): two otherwise byte-identical tag header pages
    // (same table, field, and tag name, differing only in the ASCENDING/
    // DESCENDING keyword used to create them) differ at exactly one byte,
    // reproduced twice across unrelated field types/tag names/table
    // content -- not inferred from decompilation or undocumented sources.
    // See docs/32-recovered-requirements-traceability.md for the recorded
    // evidence.
    //
    // Scope note: reliably populated only for single-tag structural CDX
    // files (one tag per .cdx, the common "INDEX ... TAG name OF file.cdx"
    // case), where the tag's own header page can be found from
    // root_node_offset + page_size. Real multi-tag (entry_count > 1)
    // compound CDX files' per-entry page association has not been
    // recovered, so this defaults to false (not guessed) for those.
    bool descending_hint = false;
};

struct CdxParseResult {
    bool ok = false;
    CdxHeader header{};
    std::vector<CdxTagDescriptor> tags;
    std::string error;
};

CdxParseResult parse_cdx_header(const std::vector<std::uint8_t>& bytes, std::uint64_t file_size);
CdxParseResult parse_cdx_header_from_file(const std::string& path);

}  // namespace copperfin::vfp

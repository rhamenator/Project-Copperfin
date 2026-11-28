void test_parse_index_probe_for_cdx() {
    const auto bytes = make_synthetic_cdx_family_bytes(true, true);

    const auto result = copperfin::vfp::parse_index_probe(bytes, 16U * 512U, copperfin::vfp::IndexKind::cdx);
    expect(result.ok, "parse_index_probe should succeed for a plausible synthetic CDX header");
    expect(result.probe.root_node_offset_hint == 1024U, "CDX root node offset should be parsed");
    expect(result.probe.key_length_hint == 10U, "CDX key length hint should be parsed");
    expect(result.probe.group_length_hint == 480U, "CDX key pool length hint should be parsed");
    expect(result.probe.multi_tag, "CDX should be treated as multi-tag");
    expect(result.probe.tags.size() == 2U, "CDX probe should enumerate tags from directory leaf pages");
    expect(result.probe.for_expression_hint == "DELETED() = .F.", "CDX probe should surface the first tag FOR expression");
    if (result.probe.tags.size() >= 2U) {
        expect(result.probe.tags[0].name_hint == "CUSTOMER_I", "directory leaf parsing should preserve the first stored tag name");
        expect(result.probe.tags[0].tag_page_offset_hint == (11U * 512U), "CDX probe should surface the first tag page hint");
        expect(result.probe.tags[0].tag_sort_marker_hint == "flags:0x0001,entries:1", "CDX probe should expose per-tag page marker hints");
        expect(
            result.probe.tags[0].key_expression_hint == "customer_id",
            "directory tag names should still bind to the matching plain-field expression");
        expect(result.probe.tags[0].for_expression_hint.empty(), "tags should not borrow FOR expressions from a different key-expression span");
        expect(!result.probe.tags[0].inferred_name, "directory-derived tag names should not be marked as inferred");
        expect(result.probe.tags[0].direction_known && !result.probe.tags[0].descending_hint,
            "the first synthetic tag direction should be explicitly ascending");
        expect(result.probe.tags[1].name_hint == "COMPANY_NA", "directory leaf parsing should preserve the second stored tag name");
        expect(result.probe.tags[1].tag_page_offset_hint == (12U * 512U), "CDX probe should surface the second tag root page hint");
        expect(result.probe.tags[0].tag_header_offset_hint == (3U * 512U), "CDX probe should preserve the first tag header page association");
        expect(result.probe.tags[1].tag_header_offset_hint == (5U * 512U), "CDX probe should preserve the second tag header page association");
        expect(result.probe.tags[1].tag_sort_marker_hint == "flags:0x0003,entries:2", "CDX probe should expose per-tag page marker hints");
        expect(
            result.probe.tags[1].key_expression_hint == "UPPER(company_name)",
            "directory tag names should still bind to the matching functional expression");
        expect(result.probe.tags[1].normalization_hint == "upper", "CDX probe should expose first-pass normalization hints");
        expect(result.probe.tags[1].collation_hint == "case-folded", "CDX probe should expose first-pass collation hints");
        expect(
            result.probe.tags[1].for_expression_hint == "DELETED() = .F.",
            "page-local FOR expressions should attach to the matching CDX tag");
        expect(!result.probe.tags[1].inferred_name, "directory-derived tag names should not be marked as inferred");
        expect(result.probe.tags[1].direction_known && !result.probe.tags[1].descending_hint,
            "the second synthetic tag direction should be explicitly ascending");
    }
}

void test_parse_synthetic_vfp9_compound_cdx_preserves_tag_tuples() {
    const auto bytes = make_synthetic_structural_cdx({
        {"PERSON_I", "UPPER(last_name)", "", 11U, 0x0003U, 1U, false},
        {"ZIP_DESC", "postal_code", "NOT DELETED()", 12U, 0x0003U, 1U, true}
    });
    const auto result = copperfin::vfp::parse_index_probe(
        bytes, bytes.size(), copperfin::vfp::IndexKind::cdx);
    expect(result.ok && result.probe.tags.size() == 2U,
           "a structurally well-formed synthetic compound CDX should parse both tags");
    if (result.probe.tags.size() == 2U) {
        const auto& first = result.probe.tags[0];
        const auto& second = result.probe.tags[1];
        expect(first.name_hint == "PERSON_I" && first.key_expression_hint == "UPPER(last_name)" &&
                   first.for_expression_hint.empty() && first.direction_known && !first.descending_hint,
               "the first synthetic directory entry must retain its exact ascending tag tuple");
        expect(second.name_hint == "ZIP_DESC" && second.key_expression_hint == "postal_code" &&
                   second.for_expression_hint == "NOT DELETED()" && second.direction_known &&
                   second.descending_hint,
               "the second synthetic directory entry must retain its exact filtered descending tuple");
        expect(first.tag_header_offset_hint == 1536U && second.tag_header_offset_hint == 2560U,
               "synthetic directory entries must bind distinct tag header pages");
    }
}

void test_parse_cdx_expression_preserves_code_page_bytes() {
    std::string expression = "UPPER('CAF";
    expression.push_back(static_cast<char>(static_cast<unsigned char>(0xE9U)));
    expression += "')";
    const auto bytes = make_synthetic_structural_cdx({
        {.name = "ACCENTED", .key_expression = expression, .for_expression = "", .root_page = 9U}
    });
    const auto result = copperfin::vfp::parse_index_probe(
        bytes, bytes.size(), copperfin::vfp::IndexKind::cdx);
    expect(result.ok && result.probe.tags.size() == 1U,
           "a CDX expression with a high-bit code-page byte should remain parseable");
    if (result.ok && result.probe.tags.size() == 1U) {
        expect(result.probe.tags.front().key_expression_hint == expression,
               "CDX expression bytes should be preserved exactly instead of filtered by the host locale");
    }
}

void test_parse_vfp9_cdx_directory_name_width_is_independent_of_key_length() {
    auto bytes = make_synthetic_structural_cdx({
        {.name = "WIDE_TAG", .key_expression = "UPPER(customer_name)",
         .for_expression = "", .root_page = 9U}
    });
    // VFP9 allows compound CDX key lengths through 240 characters; the
    // directory still stores a tag name in its fixed 10-character slot.
    write_le_u16(bytes, 12U, 240U);
    const auto result = copperfin::vfp::parse_index_probe(
        bytes, bytes.size(), copperfin::vfp::IndexKind::cdx);
    expect(result.ok && result.probe.tags.size() == 1U,
           "a long CDX key-length hint must not change the fixed tag-name directory width");
    if (result.probe.tags.size() == 1U) {
        expect(result.probe.tags.front().name_hint == "WIDE_TAG" &&
                   result.probe.tags.front().key_expression_hint == "UPPER(customer_name)",
               "the fixed-width directory name and the longer key expression must remain independently decoded");
    }
}

void test_parse_synthetic_vfp9_compound_cdx_rejects_ambiguous_bindings() {
    const auto duplicate_names = make_synthetic_structural_cdx({
        {.name = "SHARED", .key_expression = "first_key", .for_expression = "", .root_page = 9U},
        {.name = "SHARED", .key_expression = "second_key", .for_expression = "", .root_page = 12U}
    });
    const auto duplicate_name_result = copperfin::vfp::parse_index_probe(
        duplicate_names, duplicate_names.size(), copperfin::vfp::IndexKind::cdx);
    expect(!duplicate_name_result.ok && duplicate_name_result.probe.tags.empty(),
           "duplicate tag names must fail closed without publishing a partial directory");

    const auto duplicate_headers = make_synthetic_structural_cdx({
        {.name = "FIRST_TAG", .key_expression = "first_key", .for_expression = "", .root_page = 9U,
         .root_flags = 0x0003U, .root_entries = 1U},
        {.name = "SECOND_TAG", .key_expression = "second_key", .for_expression = "", .root_page = 12U,
         .root_flags = 0x0003U, .root_entries = 1U, .header_page = 3U}
    });
    const auto duplicate_header_result = copperfin::vfp::parse_index_probe(
        duplicate_headers, duplicate_headers.size(), copperfin::vfp::IndexKind::cdx);
    expect(!duplicate_header_result.ok && duplicate_header_result.probe.tags.empty(),
           "distinct tags cannot bind to the same header page");

    auto invalid_pointer = make_synthetic_structural_cdx({
        {.name = "VALIDTAG", .key_expression = "field_name", .for_expression = "", .root_page = 9U}
    });
    constexpr std::size_t directory_entry_pointer_offset = (2U * 512U) + 24U;
    invalid_pointer[directory_entry_pointer_offset] = 0xFFU;
    invalid_pointer[directory_entry_pointer_offset + 1U] = 0xFFU;
    invalid_pointer[directory_entry_pointer_offset + 2U] = 0x0FU;
    const auto invalid_pointer_result = copperfin::vfp::parse_index_probe(
        invalid_pointer, invalid_pointer.size(), copperfin::vfp::IndexKind::cdx);
    expect(!invalid_pointer_result.ok && invalid_pointer_result.probe.tags.empty(),
           "an unaligned or out-of-range tag-header association must not yield a partial descriptor");

    auto expression_as_root = make_synthetic_structural_cdx({
        {.name = "VALIDTAG", .key_expression = "field_name", .for_expression = "", .root_page = 9U}
    });
    constexpr std::size_t tag_header_offset = 3U * 512U;
    write_le_u32(expression_as_root, tag_header_offset, 4U * 512U);
    const auto expression_as_root_result = copperfin::vfp::parse_index_probe(
        expression_as_root, expression_as_root.size(), copperfin::vfp::IndexKind::cdx);
    expect(!expression_as_root_result.ok && expression_as_root_result.probe.tags.empty(),
           "a tag header pointing at its expression-pool page must not publish a tag root");

    auto unused_page_as_root = make_synthetic_structural_cdx({
        {.name = "VALIDTAG", .key_expression = "field_name", .for_expression = "", .root_page = 9U}
    });
    write_le_u32(unused_page_as_root, tag_header_offset, 14U * 512U);
    const auto unused_page_as_root_result = copperfin::vfp::parse_index_probe(
        unused_page_as_root, unused_page_as_root.size(), copperfin::vfp::IndexKind::cdx);
    expect(!unused_page_as_root_result.ok && unused_page_as_root_result.probe.tags.empty(),
           "an unused in-range page without CDX root attributes must not be published as a tag root");

    auto directory_page_as_root = make_synthetic_structural_cdx({
        {.name = "VALIDTAG", .key_expression = "field_name", .for_expression = "", .root_page = 9U}
    });
    constexpr std::size_t directory_page_offset = 2U * 512U;
    write_le_u32(directory_page_as_root, tag_header_offset, directory_page_offset);
    const auto directory_page_as_root_result = copperfin::vfp::parse_index_probe(
        directory_page_as_root, directory_page_as_root.size(), copperfin::vfp::IndexKind::cdx);
    expect(!directory_page_as_root_result.ok && directory_page_as_root_result.probe.tags.empty(),
           "a tag root pointing to the compound directory page must not publish a tag descriptor");
}

void test_parse_real_vfp9_compound_cdx_recovers_directory_metadata() {
    namespace fs = std::filesystem;
    const fs::path fixture_root = fs::path(COPPERFIN_TEST_SOURCE_DIR) /
        "tests/fixtures/vfp9-descending-observation-output";
    const fs::path cdx_path = fixture_root / "descending.CDX";
    const fs::path dbf_path = fixture_root / "descending.dbf";

    const auto direct_result = copperfin::vfp::parse_index_probe_from_file(
        copperfin::platform::path_to_utf8_string(cdx_path));
    expect(direct_result.ok,
           "the real VFP9 compound CDX directory must parse when each tag is structurally bound");
    expect(direct_result.probe.tags.size() == 2U,
           "the real VFP9 compound CDX must expose both directory entries");
    if (direct_result.probe.tags.size() == 2U) {
        const auto& ascending = direct_result.probe.tags[0];
        const auto& descending = direct_result.probe.tags[1];
        expect(ascending.name_hint == "ASCTAG", "the real VFP9 first tag must decode as ASCTAG");
        expect(ascending.key_expression_hint == "name", "ASCTAG must bind its real key expression");
        expect(ascending.for_expression_hint.empty(), "ASCTAG must not borrow a FOR expression");
        expect(ascending.direction_known && !ascending.descending_hint,
               "ASCTAG creation direction must be known as ascending");
        expect(ascending.tag_header_offset_hint == 1536U && ascending.tag_page_offset_hint == 2560U,
               "ASCTAG must bind the directory entry to its header and root pages");
        expect(descending.name_hint == "DESCTAG", "the real VFP9 second tag must decode as DESCTAG");
        expect(descending.key_expression_hint == "name", "DESCTAG must bind its real key expression");
        expect(descending.for_expression_hint.empty(), "DESCTAG must not borrow a FOR expression");
        expect(descending.direction_known && descending.descending_hint,
               "DESCTAG creation direction must be known as descending");
        expect(descending.tag_header_offset_hint == 3072U && descending.tag_page_offset_hint == 4096U,
               "DESCTAG must bind the directory entry to its header and root pages");
    }

    const auto second_real_cdx = copperfin::vfp::parse_index_probe_from_file(
        copperfin::platform::path_to_utf8_string(
            fs::path(COPPERFIN_TEST_SOURCE_DIR) /
            "tests/fixtures/vfp9-index-order-recovery/indexed_demo.cdx"));
    expect(second_real_cdx.ok && second_real_cdx.probe.tags.size() == 2U,
           "the independent real VFP9 CDX capture must also bind both tags");
    if (second_real_cdx.probe.tags.size() == 2U) {
        expect(second_real_cdx.probe.tags[0].name_hint == "ASCTAG" &&
                   second_real_cdx.probe.tags[0].key_expression_hint == "name" &&
                   second_real_cdx.probe.tags[0].direction_known &&
                   !second_real_cdx.probe.tags[0].descending_hint,
               "the independent capture's first tag tuple must match VFP9's ascending ASCTAG");
        expect(second_real_cdx.probe.tags[1].name_hint == "DESCTAG" &&
                   second_real_cdx.probe.tags[1].key_expression_hint == "name" &&
                   second_real_cdx.probe.tags[1].direction_known &&
                   second_real_cdx.probe.tags[1].descending_hint,
               "the independent capture's second tag tuple must match VFP9's descending DESCTAG");
    }

    const auto table_result = copperfin::vfp::inspect_asset(
        copperfin::platform::path_to_utf8_string(dbf_path));
    expect(table_result.ok, "the DBF table remains inspectable when its CDX metadata is incomplete");
    const bool exposed_cdx = std::any_of(
        table_result.indexes.begin(),
        table_result.indexes.end(),
        [](const copperfin::vfp::AssetInspectionResult::IndexAsset& index) {
            return index.probe.kind == copperfin::vfp::IndexKind::cdx;
        });
    expect(exposed_cdx,
           "table inspection must expose the structurally parsed real VFP9 CDX");
    expect(!has_validation_issue(table_result, "index.companion_parse_failed"),
           "table inspection must not qualify a successfully parsed real VFP9 CDX as failed");
}

void test_parse_cdx_compound_directory_with_unfit_entry_count_fails_closed() {
    auto bytes = make_synthetic_cdx_family_bytes(true, false);
    const std::size_t root_page_offset = static_cast<std::size_t>(
        static_cast<std::uint32_t>(bytes[0]) |
        (static_cast<std::uint32_t>(bytes[1]) << 8U) |
        (static_cast<std::uint32_t>(bytes[2]) << 16U) |
        (static_cast<std::uint32_t>(bytes[3]) << 24U));
    // The declared compact leaf entry array cannot fit beside the page header.
    write_le_u16(bytes, root_page_offset + 2U, 128U);

    const auto result = copperfin::vfp::parse_index_probe(
        bytes, bytes.size(), copperfin::vfp::IndexKind::cdx);
    expect(!result.ok,
           "a compound root directory whose compact entries exceed its page must fail closed");
    expect(result.probe.tags.empty(),
           "an overfull compound root directory must not fall through to heuristic tag extraction");
}

void test_parse_cdx_header_root_offset_beyond_16_bits() {
    std::vector<std::uint8_t> bytes(16U, 0U);
    write_le_u32(bytes, 0U, 128U * 512U);
    write_le_u32(bytes, 4U, 1U * 512U);
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;

    const auto result = copperfin::vfp::parse_cdx_header(bytes, 129U * 512U);
    expect(!result.ok, "a header-only buffer must not claim a complete CDX tag parse");
    expect(
        result.header.root_node_offset == (128U * 512U),
        "CDX root node offset should not be truncated to its low 16 bits");
    expect(
        result.header.next_free_node_offset == (1U * 512U),
        "CDX free node offset should not be truncated to its low 16 bits");
}

void test_parse_index_probe_for_dcx() {
    const auto bytes = make_synthetic_cdx_family_bytes(false, true);

    const auto result = copperfin::vfp::parse_index_probe(bytes, 16U * 512U, copperfin::vfp::IndexKind::dcx);
    expect(result.ok, "parse_index_probe should succeed for a plausible synthetic DCX header");
    expect(result.probe.kind == copperfin::vfp::IndexKind::dcx, "DCX probe kind should be preserved");
    expect(result.probe.multi_tag, "DCX should be treated as multi-tag");
    expect(!result.probe.production_candidate, "DCX should not be flagged as a table production index");
    expect(result.probe.tags.size() == 1U, "DCX probe should reuse the shared CDX-family tag parser");
    if (!result.probe.tags.empty()) {
        expect(result.probe.tags.front().name_hint == "NAME", "DCX probe should preserve the stored tag name");
        expect(result.probe.tags.front().tag_page_offset_hint == (9U * 512U), "DCX probe should preserve the stored tag root page hint");
        expect(result.probe.tags.front().tag_sort_marker_hint == "flags:0x0003,entries:1", "DCX probe should expose per-tag page marker hints");
        expect(result.probe.tags.front().key_expression_hint == "UPPER(NAME)", "DCX probe should expose the key expression hint");
        expect(result.probe.tags.front().normalization_hint == "upper", "DCX probe should expose first-pass normalization hints");
        expect(result.probe.tags.front().collation_hint == "case-folded", "DCX probe should expose first-pass collation hints");
        expect(result.probe.tags.front().for_expression_hint == "DELETED() = .F.", "DCX probe should expose the FOR expression hint");
    }
}

void test_parse_index_probe_for_cdx_prefers_tag_page_local_expressions() {
    const auto bytes = make_synthetic_cdx_bytes_with_decoys();

    const auto result = copperfin::vfp::parse_index_probe(bytes, 16U * 512U, copperfin::vfp::IndexKind::cdx);
    expect(result.ok, "parse_index_probe should still succeed for a plausible CDX with stray printable expressions");
    const auto tag = std::find_if(
        result.probe.tags.begin(),
        result.probe.tags.end(),
        [](const copperfin::vfp::IndexTagProbe& candidate) { return candidate.name_hint == "CUSTOMER_I"; });
    expect(tag != result.probe.tags.end(), "single-tag adversarial CDX probe should still expose the stored tag");
    if (tag != result.probe.tags.end()) {
        expect(tag->tag_page_offset_hint == (11U * 512U), "adversarial CDX probe should preserve the tag page hint");
        expect(
            tag->key_expression_hint == "customer_id",
            "tag-page-local binding should ignore earlier decoy key expressions");
        expect(
            tag->for_expression_hint == "DELETED() = .F.",
            "tag-page-local binding should ignore earlier decoy FOR expressions");
    }
}

void test_parse_index_probe_for_cdx_binds_descriptive_tag_names_from_tag_page_hints() {
    const auto bytes = make_synthetic_cdx_bytes_with_descriptive_tag_name();

    const auto result = copperfin::vfp::parse_index_probe(bytes, 16U * 512U, copperfin::vfp::IndexKind::cdx);
    expect(result.ok, "parse_index_probe should succeed for a plausible CDX with a descriptive tag name");
    expect(result.probe.tags.size() == 1U, "descriptive-tag CDX probe should expose the single tag");
    if (!result.probe.tags.empty()) {
        expect(result.probe.tags.front().name_hint == "FULLNAME", "descriptive tag names should be preserved");
        expect(
            result.probe.tags.front().key_expression_hint == "UPPER(LAST+FIRST)",
            "tag-page-local binding should attach expressions even when the tag name does not resemble the expression");
        expect(result.probe.tags.front().tag_page_offset_hint == (9U * 512U), "descriptive-tag CDX probe should preserve the stored tag root page hint");
        expect(result.probe.tags.front().normalization_hint == "upper", "descriptive-tag CDX probe should still derive normalization hints");
        expect(result.probe.tags.front().collation_hint == "case-folded", "descriptive-tag CDX probe should still derive collation hints");
    }
}

void test_parse_index_probe_for_cdx_preserves_plain_field_expression_tags() {
    const auto bytes = make_synthetic_cdx_bytes_with_plain_field_expression();

    const auto result = copperfin::vfp::parse_index_probe(bytes, 16U * 512U, copperfin::vfp::IndexKind::cdx);
    expect(result.ok, "parse_index_probe should succeed for a plausible CDX with a plain field-name key expression");
    expect(result.probe.tags.size() == 1U, "plain-field CDX probe should expose the single stored tag");
    if (!result.probe.tags.empty()) {
        expect(result.probe.tags.front().name_hint == "NAME", "plain-field CDX probe should preserve the stored tag name");
        expect(
            result.probe.tags.front().key_expression_hint == "NAME",
            "plain-field CDX probe should keep a direct field-name key expression instead of dropping it");
        expect(
            result.probe.tags.front().tag_page_offset_hint == (9U * 512U),
            "plain-field CDX probe should preserve the stored tag root page hint");
    }
}

void test_parse_index_probe_for_idx() {
    std::vector<std::uint8_t> bytes(512U, 0U);
    write_le_u32(bytes, 0U, 512U);
    write_le_u32(bytes, 4U, 0xFFFFFFFFU);
    write_le_u32(bytes, 8U, 1024U);
    write_le_u16(bytes, 12U, 10U);
    bytes[14] = 0x21U;
    bytes[15] = 0x9AU;
    write_ascii(bytes, 16U, "UPPER(NAME)");
    write_ascii(bytes, 236U, "DELETED() = .F.");

    const auto result = copperfin::vfp::parse_index_probe(bytes, 1024U, copperfin::vfp::IndexKind::idx);
    expect(result.ok, "parse_index_probe should succeed for a plausible Visual FoxPro IDX header");
    expect(result.probe.kind == copperfin::vfp::IndexKind::idx, "IDX probe kind should be preserved");
    expect(result.probe.root_node_offset_hint == 512U, "IDX root node offset should be parsed");
    expect(result.probe.end_of_file_offset_hint == 1024U, "IDX end-of-file offset should be parsed");
    expect(result.probe.key_length_hint == 10U, "IDX key length should be parsed");
    expect(result.probe.key_expression_hint == "UPPER(NAME)", "IDX key expression should be extracted");
    expect(result.probe.for_expression_hint == "DELETED() = .F.", "IDX FOR expression should be extracted");
    expect(result.probe.normalization_hint == "upper", "IDX probe should expose first-pass normalization hints");
    expect(result.probe.collation_hint == "case-folded", "IDX probe should expose first-pass collation hints");
    expect(result.probe.header_sort_marker_hint == "sig:0x9A,flags:0x21", "IDX probe should expose an opaque header sort marker");
}

void test_parse_index_probe_for_ndx() {
    std::vector<std::uint8_t> bytes(512U, 0U);
    write_le_u32(bytes, 0U, 1U);
    write_le_u32(bytes, 4U, 2U);
    write_le_u32(bytes, 8U, 0x00000034U);
    write_le_u16(bytes, 12U, 2U);
    write_le_u16(bytes, 14U, 42U);
    write_le_u16(bytes, 16U, 1U);
    write_le_u16(bytes, 18U, 12U);
    write_le_u16(bytes, 22U, 1U);
    write_ascii(bytes, 24U, "CODE");

    const auto result = copperfin::vfp::parse_index_probe(bytes, 1024U, copperfin::vfp::IndexKind::ndx);
    expect(result.ok, "parse_index_probe should succeed for a plausible dBase NDX header");
    expect(result.probe.kind == copperfin::vfp::IndexKind::ndx, "NDX probe kind should be preserved");
    expect(result.probe.root_node_offset_hint == 512U, "NDX root block should convert to a byte offset");
    expect(result.probe.end_of_file_offset_hint == 1024U, "NDX EOF block should convert to a byte offset");
    expect(result.probe.max_keys_hint == 42U, "NDX max keys should be parsed");
    expect(result.probe.group_length_hint == 12U, "NDX group length should be parsed");
    expect(result.probe.flags == 0x01U, "NDX uniqueness flag should be projected into flags");
    expect(result.probe.key_expression_hint == "CODE", "NDX expression should be extracted");
    expect(result.probe.header_sort_marker_hint == "ver:0x34", "NDX probe should expose an opaque header sort marker");
    expect(result.probe.key_domain_hint == "numeric_or_date", "NDX probe should expose the numeric/date key domain hint");
}

void test_parse_index_probe_for_ndx_surfaces_character_domain_without_named_collation() {
    std::vector<std::uint8_t> bytes(512U, 0U);
    write_le_u32(bytes, 0U, 1U);
    write_le_u32(bytes, 4U, 2U);
    write_le_u32(bytes, 8U, 0x0000007FU);
    write_le_u16(bytes, 12U, 4U);
    write_le_u16(bytes, 14U, 42U);
    write_le_u16(bytes, 16U, 0U);
    write_le_u16(bytes, 18U, 12U);
    write_ascii(bytes, 24U, "CODE");

    const auto result = copperfin::vfp::parse_index_probe(bytes, 1024U, copperfin::vfp::IndexKind::ndx);
    expect(result.ok, "parse_index_probe should succeed for a plausible dBase NDX header with a character key domain");
    expect(result.probe.header_sort_marker_hint == "ver:0x7F", "NDX probe should preserve the raw opaque version marker");
    expect(result.probe.key_domain_hint == "character", "NDX probe should expose the character key domain hint");
    expect(result.probe.collation_hint.empty(), "NDX probe should not invent a named collation from the raw header marker alone");
}

void test_parse_index_probe_for_mdx() {
    const auto bytes = make_synthetic_mdx_bytes(true);

    const auto result = copperfin::vfp::parse_index_probe(bytes, bytes.size(), copperfin::vfp::IndexKind::mdx);
    expect(result.ok, "parse_index_probe should succeed for a plausible dBase MDX file");
    expect(result.probe.kind == copperfin::vfp::IndexKind::mdx, "MDX probe kind should be preserved");
    expect(result.probe.multi_tag, "MDX should be treated as multi-tag");
    expect(result.probe.production_candidate, "MDX should be treated as a production index candidate");
    expect(result.probe.header_sort_marker_hint == "slots:48,entry_size:32,in_use:2", "MDX probe should expose header slot metadata");
    expect(result.probe.tags.size() == 2U, "MDX probe should enumerate first-pass tag hints");
    if (result.probe.tags.size() >= 2U) {
        expect(result.probe.tags[0].name_hint == "NAME_TAG", "MDX probe should expose the first tag hint");
        expect(result.probe.tags[1].name_hint == "CITYSTATE", "MDX probe should expose the second tag hint");
        expect(result.probe.tags[0].tag_page_offset_hint == (2U * 512U), "MDX probe should expose the first tag header-page offset hint");
        expect(result.probe.tags[1].tag_page_offset_hint == (3U * 512U), "MDX probe should expose the second tag header-page offset hint");
        expect(result.probe.tags[0].key_expression_hint == "UPPER(NAME)", "MDX probe should extract first-pass key expressions from tag headers");
        expect(result.probe.tags[1].key_expression_hint == "UPPER(CITY+STATE)", "MDX probe should extract first-pass key expressions from tag headers");
        expect(result.probe.tags[0].for_expression_hint == "DELETED() = .F.", "MDX probe should extract first-pass FOR expressions from tag headers");
        expect(result.probe.tags[1].for_expression_hint == "STATE = 'WA'", "MDX probe should extract first-pass FOR expressions from tag headers");
        expect(result.probe.tags[0].tag_sort_marker_hint == "flags:0x0001,entries:9", "MDX probe should expose first-tag page marker metadata");
        expect(result.probe.tags[1].tag_sort_marker_hint == "flags:0x0001,entries:7", "MDX probe should expose second-tag page marker metadata");
        expect(result.probe.tags[0].key_format_marker == static_cast<std::uint8_t>('C'), "MDX probe should preserve the tag key-format marker");
        expect(result.probe.tags[0].key_type_marker == static_cast<std::uint8_t>('C'), "MDX probe should preserve the tag key-type marker");
        expect(result.probe.tags[0].thread_hint == 1U, "MDX probe should preserve the tag thread marker bytes");
        expect(result.probe.tags[1].thread_hint == 2U, "MDX probe should preserve per-tag thread marker bytes");
        expect(result.probe.tags[0].normalization_hint == "upper", "MDX probe should derive first-pass normalization hints from key expressions");
        expect(result.probe.tags[0].collation_hint == "case-folded", "MDX probe should derive first-pass collation hints from key expressions");
    }
}

void test_parse_index_probe_for_mdx_rejects_implausible_header() {
    std::vector<std::uint8_t> bytes(2U * 512U, 0U);
    write_ascii(bytes, 512U + 32U, "NAME_TAG");

    const auto result = copperfin::vfp::parse_index_probe(bytes, bytes.size(), copperfin::vfp::IndexKind::mdx);
    expect(!result.ok, "parse_index_probe should reject MDX files with an implausible all-zero header block");
}

void test_parse_index_probe_for_ntx() {
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_le_u16(bytes, 0U, 0x0006U);
    write_le_u16(bytes, 2U, 0x0517U);
    write_le_u32(bytes, 4U, 1024U);
    write_le_u32(bytes, 8U, 3U * 1024U);
    write_le_u16(bytes, 12U, 18U);
    write_le_u16(bytes, 14U, 10U);
    write_le_u16(bytes, 16U, 0U);
    write_le_u16(bytes, 18U, 46U);
    write_le_u16(bytes, 20U, 23U);
    write_ascii(bytes, 22U, "UPPER(NAME)");

    const auto result = copperfin::vfp::parse_index_probe(bytes, 3U * 1024U, copperfin::vfp::IndexKind::ntx);
    expect(result.ok, "parse_index_probe should succeed for a plausible Clipper NTX header");
    expect(result.probe.kind == copperfin::vfp::IndexKind::ntx, "NTX probe kind should be preserved");
    expect(result.probe.block_size == 1024U, "NTX probe should use the 1024-byte page size");
    expect(result.probe.root_node_offset_hint == 1024U, "NTX root page offset should be parsed");
    expect(result.probe.free_node_offset_hint == (3U * 1024U), "NTX next-unused page offset should be parsed");
    expect(result.probe.key_length_hint == 10U, "NTX key length should be parsed");
    expect(result.probe.group_length_hint == 18U, "NTX group length should be parsed");
    expect(result.probe.max_keys_hint == 46U, "NTX max keys per page should be parsed");
    expect(result.probe.key_expression_hint == "UPPER(NAME)", "NTX key expression should be extracted");
    expect(result.probe.normalization_hint == "upper", "NTX probe should expose first-pass normalization hints");
    expect(result.probe.collation_hint == "case-folded", "NTX probe should expose first-pass collation hints");
    expect(result.probe.header_sort_marker_hint == "ver:0x06", "NTX probe should expose an opaque header sort marker");
}

void test_parse_index_probe_for_ntx_rejects_truncated_header() {
    std::vector<std::uint8_t> bytes(512U, 0U);
    write_le_u32(bytes, 4U, 1024U);

    const auto result = copperfin::vfp::parse_index_probe(bytes, 512U, copperfin::vfp::IndexKind::ntx);
    expect(!result.ok, "parse_index_probe should reject an NTX header shorter than one 1024-byte page");
}

void test_parse_index_probe_for_ntx_rejects_partial_trailing_page() {
    // A file whose declared size (1025 bytes) is not a whole number of
    // 1024-byte pages must fail closed even when the buffered first page
    // and its root offset otherwise look plausible: the on-disk file only
    // has a partial trailing page, so root offset 1024 would not actually
    // contain a complete page.
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_le_u16(bytes, 0U, 0x0006U);
    write_le_u32(bytes, 4U, 1024U);
    write_le_u32(bytes, 8U, 0U);
    write_le_u16(bytes, 12U, 18U);
    write_le_u16(bytes, 14U, 10U);
    write_le_u16(bytes, 18U, 46U);
    write_ascii(bytes, 22U, "NAME");

    const auto result = copperfin::vfp::parse_index_probe(bytes, 1025U, copperfin::vfp::IndexKind::ntx);
    expect(!result.ok, "parse_index_probe should reject an NTX file whose size is not a whole number of 1024-byte pages");
}

void test_parse_index_probe_for_ntx_rejects_inconsistent_group_length() {
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_le_u16(bytes, 0U, 0x0006U);
    write_le_u32(bytes, 4U, 1024U);
    write_le_u16(bytes, 12U, 99U);
    write_le_u16(bytes, 14U, 10U);
    write_le_u16(bytes, 18U, 46U);
    write_ascii(bytes, 22U, "NAME");

    const auto result = copperfin::vfp::parse_index_probe(bytes, 2U * 1024U, copperfin::vfp::IndexKind::ntx);
    expect(!result.ok, "parse_index_probe should reject an NTX header whose group length does not match key length + 8");
}

void test_index_probe_errors_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.IndexProbe.Error.VisualFoxProIdxHeaderTooSmall") ==
            "File is smaller than the 512-byte Visual FoxPro IDX header size.",
        "#2380: IDX short-header error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.IndexProbe.Error.DbaseMdxInvalidValues") ==
            "Header values do not look like a block-oriented dBase MDX file.",
        "#2380: MDX invalid-header error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.IndexProbe.Error.ClipperNtxInvalidValues") ==
            "Header values do not look like a Clipper NTX file.",
        "#5484: NTX invalid-header error should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate("Vfp.IndexProbe.Error.UnknownExtension") ==
            "Extension de indice desconocida.",
        "#2602: index probe extension errors should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Vfp.IndexProbe.Error.VisualFoxProIdxHeaderTooSmall") ==
            "O arquivo e menor que o tamanho do cabecalho Visual FoxPro IDX de 512 bytes.",
        "#2602: Visual FoxPro IDX short-header errors should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Vfp.IndexProbe.Error.VisualFoxProIdxHeaderTooSmall") !=
            english_catalog.translate("Vfp.IndexProbe.Error.VisualFoxProIdxHeaderTooSmall"),
        "#2380: index probe errors should be pseudo-localizable");
    expect(
        pseudo_catalog.translate("Vfp.IndexProbe.Error.UnknownExtension") ==
            copperfin::localization::pseudo_localize("Unknown index extension."),
        "#2602: index probe qps-ploc strings should resolve through the pseudo-localization transform");

    const auto idx_result =
        copperfin::vfp::parse_index_probe({0x00U}, 1U, copperfin::vfp::IndexKind::idx);
    expect(!idx_result.ok, "parse_index_probe should reject short IDX input");
    expect(
        idx_result.error == "File is smaller than the 512-byte Visual FoxPro IDX header size.",
        "#2380: parse_index_probe should preserve the default localized IDX short-header error");

    const auto unknown_result =
        copperfin::vfp::parse_index_probe({}, 0U, copperfin::vfp::IndexKind::unknown);
    expect(!unknown_result.ok, "parse_index_probe should reject unknown index kinds");
    expect(
        unknown_result.error == "Unknown index extension.",
        "#2380: parse_index_probe should preserve the default localized unknown-extension error");
}

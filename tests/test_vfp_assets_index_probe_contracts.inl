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
        expect(result.probe.tags[1].name_hint == "COMPANY_NA", "directory leaf parsing should preserve the second stored tag name");
        expect(result.probe.tags[1].tag_page_offset_hint == (4U * 512U), "CDX probe should surface the second tag page hint");
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
    }
}

void test_parse_cdx_header_root_offset_beyond_16_bits() {
    std::vector<std::uint8_t> bytes(16U, 0U);
    write_le_u32(bytes, 0U, 128U * 512U);
    write_le_u32(bytes, 4U, 1U * 512U);
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;

    const auto result = copperfin::vfp::parse_cdx_header(bytes, 129U * 512U);
    expect(result.ok, "parse_cdx_header should accept a root node offset beyond 16 bits");
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
        expect(result.probe.tags.front().tag_page_offset_hint == (4U * 512U), "DCX probe should preserve the stored tag page hint");
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
        expect(result.probe.tags.front().tag_page_offset_hint == (4U * 512U), "descriptive-tag CDX probe should preserve the stored tag page hint");
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
            result.probe.tags.front().tag_page_offset_hint == (4U * 512U),
            "plain-field CDX probe should preserve the stored tag page hint");
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

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_support.h"
#include "test_environment_support.h"

#include "copperfin/platform/path.h"

namespace cf_test_studio_host {
void test_document_default_catalog_refreshes_when_locale_changes() {
    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    locale_override.set("en-US");

    copperfin::studio::StudioDocumentModel document;
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;
    document.table_preview.records = {
        {.record_index = 7U, .deleted = false, .values = {}}
    };

    const auto english_open = copperfin::studio::open_document({});
    const auto english_objects = copperfin::studio::build_object_snapshot(document);
    locale_override.set("es-419");
    const auto spanish_open = copperfin::studio::open_document({});
    const auto spanish_objects = copperfin::studio::build_object_snapshot(document);
    locale_override.set("qps-ploc");
    const auto pseudo_open = copperfin::studio::open_document({});
    const auto pseudo_objects = copperfin::studio::build_object_snapshot(document);

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    constexpr std::string_view open_key = "Studio.DocumentOpen.Error.PathRequired";
    constexpr std::string_view title_key = "Studio.DocumentModel.Fallback.RecordTitle";
    const auto expected_title = [title_key](const auto& catalog) {
        return catalog.translate(title_key, {{"recordIndex", "7"}});
    };
    expect(!english_open.ok && english_open.error == english_catalog.translate(open_key),
           "#4361: document-open diagnostics should begin in en-US");
    expect(!spanish_open.ok && spanish_open.error == spanish_catalog.translate(open_key),
           "#4361: document-open diagnostics should refresh to es-419");
    expect(!pseudo_open.ok && pseudo_open.error == pseudo_catalog.translate(open_key),
           "#4361: document-open diagnostics should refresh to qps-ploc");
    expect(english_objects.size() == 1U && spanish_objects.size() == 1U && pseudo_objects.size() == 1U,
           "#4361: locale refresh should preserve object-snapshot cardinality");
    if (english_objects.size() == 1U && spanish_objects.size() == 1U && pseudo_objects.size() == 1U) {
        expect(english_objects[0].title == expected_title(english_catalog),
               "#4361: object-snapshot fallback titles should begin in en-US");
        expect(spanish_objects[0].title == expected_title(spanish_catalog),
               "#4361: object-snapshot fallback titles should refresh to es-419");
        expect(pseudo_objects[0].title == expected_title(pseudo_catalog),
               "#4361: object-snapshot fallback titles should refresh to qps-ploc");
        expect(english_objects[0].record_index == spanish_objects[0].record_index &&
                   spanish_objects[0].record_index == pseudo_objects[0].record_index,
               "#4361: locale refresh should preserve object record identity");
    }
}

void test_open_document_path_error_resolves_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Studio.DocumentOpen.Error.PathRequired") == "No path was provided.",
        "#2393: missing document path diagnostic should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate("Studio.DocumentOpen.Error.PathRequired") == "No se proporciono una ruta.",
        "#2645: missing document path diagnostic should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Studio.DocumentOpen.Error.PathRequired") ==
            "Nenhum caminho foi fornecido.",
        "#2645: missing document path diagnostic should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Studio.DocumentOpen.Error.PathRequired") !=
            english_catalog.translate("Studio.DocumentOpen.Error.PathRequired"),
        "#2393: missing document path diagnostic should be pseudo-localizable");

    const auto result = copperfin::studio::open_document({});
    expect(!result.ok, "#2393: open_document should reject missing paths");
    expect(
        result.error == "No path was provided.",
        "#2393: open_document should preserve default localized missing path diagnostic");
}

void test_open_document_uses_supplied_localization_catalog() {
    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    locale_override.set("qps-ploc");

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto catalogs = std::vector<std::pair<std::string_view, copperfin::localization::LocalizedCatalog>>{
        {"en-US", copperfin::localization::load_catalogs(catalog_root, "en-US")},
        {"es-419", copperfin::localization::load_catalogs(catalog_root, "es-419")},
        {"pt-BR", copperfin::localization::load_catalogs(catalog_root, "pt-BR")},
        {"qps-ploc", copperfin::localization::load_catalogs(catalog_root, "qps-ploc")}
    };
    constexpr std::string_view key = "Studio.DocumentOpen.Error.PathRequired";
    for (const auto& [locale, catalog] : catalogs) {
        const auto result = copperfin::studio::open_document({}, catalog);
        expect(!result.ok, "#4734: supplied catalog should preserve missing-path failure");
        expect(result.error == catalog.translate(key),
               std::string("#4734: supplied catalog should control document-open text for ") +
                   std::string(locale));
    }
}

void test_open_document_infers_form_sidecar() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_tests";
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "customer.scx";
    const fs::path sidecar_path = temp_dir / "customer.sct";

    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo-sidecar";
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .symbol = "form1",
        .line = 10U,
        .column = 2U,
        .launched_from_visual_studio = true,
        .read_only = false
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should succeed for a valid synthetic SCX file");
    expect(result.document.kind == copperfin::studio::StudioAssetKind::form, "SCX should map to a form document");
    expect(result.document.display_name == "customer.scx", "document display name should use the file name");
    expect(result.document.has_sidecar, "open_document should detect the SCT sidecar");
    expect(result.document.sidecar_path == sidecar_path.string(), "open_document should infer the SCT sidecar path");
    expect(result.document.launched_from_visual_studio, "launch metadata should flow into the Studio document");
    expect(result.document.selection_symbol == "form1", "#964: launch selection symbol should flow into the Studio document");
    expect(result.document.selection_line == 10U, "#964: launch selection line should flow into the Studio document");
    expect(result.document.selection_column == 2U, "#964: launch selection column should flow into the Studio document");
    expect(result.document.selection_record_index == 0U,
           "#964: launch selection record index should keep the default when none is supplied");
    expect(!result.document.selection_record_available,
           "#967: launch selection record availability should be false when no record is supplied");
    expect(result.document.inspection.header_available, "inspection metadata should be attached to the document");

    const auto objects = copperfin::studio::build_object_snapshot(result.document);
    expect(objects.empty(), "header-only synthetic SCX should not produce object snapshots without parsed records");

    std::error_code ignored;
    fs::remove(form_path, ignored);
    fs::remove(sidecar_path, ignored);
    fs::remove(temp_dir, ignored);
}

void test_open_document_casefold_preserves_utf8_filename_bytes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir =
        fs::temp_directory_path() / "copperfin_studio_host_utf8_casefold_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    struct SidecarCase {
        copperfin::studio::StudioAssetKind kind;
        std::u8string_view primary_name;
        std::u8string_view sidecar_name;
        std::string_view label;
    };
    const SidecarCase cases[]{
        {copperfin::studio::StudioAssetKind::project, u8"caf\u00E9.pjx", u8"caf\u00E9.PJT", "PJX/PJT"},
        {copperfin::studio::StudioAssetKind::form, u8"caf\u00E9.scx", u8"caf\u00E9.SCT", "SCX/SCT"},
        {copperfin::studio::StudioAssetKind::class_library, u8"caf\u00E9.vcx", u8"caf\u00E9.VCT", "VCX/VCT"},
        {copperfin::studio::StudioAssetKind::report, u8"caf\u00E9.frx", u8"caf\u00E9.FRT", "FRX/FRT"},
        {copperfin::studio::StudioAssetKind::label, u8"caf\u00E9.lbx", u8"caf\u00E9.LBT", "LBX/LBT"},
        {copperfin::studio::StudioAssetKind::menu, u8"caf\u00E9.mnx", u8"caf\u00E9.MNT", "MNX/MNT"}
    };

    for (const auto& sidecar_case : cases) {
        const fs::path primary_path = temp_dir / fs::path(sidecar_case.primary_name);
        const fs::path sidecar_path = temp_dir / fs::path(sidecar_case.sidecar_name);
        const auto bytes = make_vfp_header();
        {
            std::ofstream output(primary_path, std::ios::binary);
            output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        }
        {
            std::ofstream output(sidecar_path, std::ios::binary);
            output << "memo-sidecar";
        }

        const std::string primary_path_utf8 = copperfin::platform::path_to_utf8_string(primary_path);
        const std::string sidecar_path_utf8 = copperfin::platform::path_to_utf8_string(sidecar_path);
        const std::string inferred = copperfin::studio::infer_sidecar_path(
            primary_path_utf8,
            sidecar_case.kind);
        expect(inferred == sidecar_path_utf8,
               "#3973: " + std::string(sidecar_case.label) +
                   " recovery should preserve UTF-8 bytes and actual entry spelling");

        const auto result = copperfin::studio::open_document({.path = primary_path_utf8});
        expect(result.ok,
               "#3973: UTF-8 " + std::string(sidecar_case.label) + " paths should remain openable");
        expect(result.document.kind == sidecar_case.kind,
               "#3973: UTF-8 " + std::string(sidecar_case.label) + " assets should retain their family");
        expect(result.document.has_sidecar,
               "#3973: UTF-8 " + std::string(sidecar_case.label) +
                   " paths should discover case-variant sidecars");
        expect(result.document.sidecar_path == sidecar_path_utf8,
               "#3973: opened UTF-8 " + std::string(sidecar_case.label) +
                   " documents should retain actual sidecar filename spelling");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_utf8_paths_across_native_boundary() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string("copperfin_studio_host_utf8_boundary_tests");
    const fs::path form_path = temp_dir /
        copperfin::platform::path_from_utf8_string("caf\xC3\xA9-form.scx");
    const fs::path sidecar_path = temp_dir /
        copperfin::platform::path_from_utf8_string("caf\xC3\xA9-form.SCT");
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto bytes = make_vfp_header();
    {
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }
    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo-sidecar";
    }

    const std::string form_path_utf8 = copperfin::platform::path_to_utf8_string(form_path);
    const std::string sidecar_path_utf8 = copperfin::platform::path_to_utf8_string(sidecar_path);
    expect(
        copperfin::studio::infer_sidecar_path(form_path_utf8, copperfin::studio::StudioAssetKind::form) ==
            sidecar_path_utf8,
        "#4275: inferred UTF-8 sidecar paths should cross the native filesystem boundary losslessly");

    const auto primary_result = copperfin::studio::open_document({.path = form_path_utf8});
    expect(primary_result.ok,
           "#4275: UTF-8 primary document paths should remain openable through the shared Studio model");
    if (primary_result.ok) {
        expect(primary_result.document.path == form_path_utf8,
               "#4275: inferred UTF-8 opens should preserve the primary document path");
        expect(primary_result.document.sidecar_path == sidecar_path_utf8,
               "#4275: inferred UTF-8 opens should preserve the actual sidecar path spelling");
    }

    const auto sidecar_result = copperfin::studio::open_document({.path = sidecar_path_utf8});
    expect(sidecar_result.ok,
           "#4275: direct UTF-8 sidecar paths should remain openable through the shared Studio model");
    if (sidecar_result.ok) {
        expect(sidecar_result.document.path == form_path_utf8,
               "#4275: direct UTF-8 sidecar opens should preserve the canonical primary path");
        expect(sidecar_result.document.sidecar_path == sidecar_path_utf8,
               "#4275: direct UTF-8 sidecar opens should preserve the sidecar path spelling");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_canonicalizes_direct_sidecar_paths() {
    namespace fs = std::filesystem;
    const fs::path temp_dir =
        fs::temp_directory_path() / "copperfin_studio_host_direct_sidecar_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto write_primary = [](const fs::path& path) {
        const auto bytes = make_vfp_header();
        std::ofstream output(path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    };
    const auto write_sidecar = [](const fs::path& path) {
        std::ofstream output(path, std::ios::binary);
        output << "memo-sidecar";
    };

    struct DirectSidecarCase {
        std::string_view stem;
        std::string_view primary_extension;
        std::string_view sidecar_extension;
        copperfin::studio::StudioAssetKind kind;
    };
    const DirectSidecarCase cases[]{
        {"project", ".pjx", ".PJT", copperfin::studio::StudioAssetKind::project},
        {"form", ".scx", ".SCT", copperfin::studio::StudioAssetKind::form},
        {"classes", ".vcx", ".VCT", copperfin::studio::StudioAssetKind::class_library},
        {"report", ".frx", ".FRT", copperfin::studio::StudioAssetKind::report},
        {"label", ".lbx", ".LBT", copperfin::studio::StudioAssetKind::label},
        {"menu", ".mnx", ".MNT", copperfin::studio::StudioAssetKind::menu}
    };

    for (const auto& direct_case : cases) {
        const fs::path primary_path =
            temp_dir / (std::string(direct_case.stem) + std::string(direct_case.primary_extension));
        const fs::path sidecar_path =
            temp_dir / (std::string(direct_case.stem) + std::string(direct_case.sidecar_extension));
        write_primary(primary_path);
        write_sidecar(sidecar_path);

        const auto result = copperfin::studio::open_document({
            .path = sidecar_path.string(),
            .symbol = "selectedSymbol",
            .line = 17U,
            .column = 4U,
            .launched_from_visual_studio = true
        });
        const std::string label =
            std::string(direct_case.primary_extension) + "/" + std::string(direct_case.sidecar_extension);
        expect(result.ok, "#3990: direct " + label + " sidecar open should canonicalize to its primary");
        if (!result.ok) {
            continue;
        }
        expect(result.document.kind == direct_case.kind,
               "#3990: direct " + label + " open should preserve the document family");
        expect(result.document.path == primary_path.string() &&
                   result.document.inspection.path == primary_path.string(),
               "#3990: direct " + label + " open should expose the actual primary path");
        expect(result.document.sidecar_path == sidecar_path.string() && result.document.has_sidecar,
               "#3990: direct " + label + " open should retain the actual sidecar spelling");
        expect(result.document.path != result.document.sidecar_path,
               "#3990: direct " + label + " open must never create a self-sidecar document");
        expect(result.document.display_name == primary_path.filename().string(),
               "#3990: direct " + label + " display name should follow the canonical primary");
        expect(result.document.selection_symbol == "selectedSymbol" &&
                   result.document.selection_line == 17U &&
                   result.document.selection_column == 4U &&
                   result.document.launched_from_visual_studio,
               "#3990: direct " + label + " canonicalization should preserve launch metadata");
    }

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> diagnostic_keys{
        "Studio.DocumentOpen.Error.SidecarPathAmbiguous",
        "Studio.DocumentOpen.Error.SidecarPrimaryAmbiguous",
        "Studio.DocumentOpen.Error.SidecarPrimaryMissing"
    };
    expect(count_missing_locale_keys(english_catalog, "en-US", diagnostic_keys) == 0U &&
               count_missing_locale_keys(
                   copperfin::localization::load_catalogs(catalog_root, "es-419"),
                   "es-419",
                   diagnostic_keys) == 0U &&
               count_missing_locale_keys(
                   copperfin::localization::load_catalogs(catalog_root, "pt-BR"),
                   "pt-BR",
                   diagnostic_keys) == 0U &&
               count_missing_locale_keys(pseudo_catalog, "qps-ploc", diagnostic_keys) == 0U,
           "#3990: direct-sidecar diagnostics should have four-catalog key parity");
    expect(pseudo_catalog.translate(diagnostic_keys[0U]) != english_catalog.translate(diagnostic_keys[0U]) &&
               pseudo_catalog.translate(diagnostic_keys[1U]) != english_catalog.translate(diagnostic_keys[1U]) &&
               pseudo_catalog.translate(diagnostic_keys[2U]) != english_catalog.translate(diagnostic_keys[2U]),
           "#3990: direct-sidecar diagnostics should remain pseudo-localizable");

    const fs::path missing_sidecar = temp_dir / "missing.PJT";
    write_sidecar(missing_sidecar);
    const auto missing_result = copperfin::studio::open_document({.path = missing_sidecar.string()});
    expect(!missing_result.ok &&
               missing_result.error == english_catalog.translate(
                   "Studio.DocumentOpen.Error.SidecarPrimaryMissing",
                   {{"path", missing_sidecar.string()}}),
           "#3990: a direct sidecar without a primary should fail through the localized diagnostic");

    const fs::path case_primary = temp_dir / "CaseProject.PJX";
    const fs::path case_sidecar = temp_dir / "caseproject.PJT";
    write_primary(case_primary);
    write_sidecar(case_sidecar);
    const auto case_result = copperfin::studio::open_document({.path = case_sidecar.string()});
    expect(case_result.ok && case_result.document.path == case_primary.string() &&
               case_result.document.sidecar_path == case_sidecar.string(),
           "#3990: unique case-fold recovery should preserve actual primary and sidecar spelling");

    const fs::path input_case_primary = temp_dir / "InputCase.PJX";
    const fs::path input_case_sidecar = temp_dir / "InputCase.PJT";
    const fs::path input_case_request = temp_dir / "inputcase.pjt";
    write_primary(input_case_primary);
    write_sidecar(input_case_sidecar);
    const auto input_case_result =
        copperfin::studio::open_document({.path = input_case_request.string()});
    expect(input_case_result.ok &&
               input_case_result.document.path == input_case_primary.string() &&
               input_case_result.document.sidecar_path == input_case_sidecar.string(),
           "#3990: direct sidecar inputs should recover one unique case-fold path and actual spelling");

    const fs::path ordinary_primary = temp_dir / "ordinary.PJX";
    const fs::path ordinary_sidecar = temp_dir / "ordinary.PJT";
    write_primary(ordinary_primary);
    write_sidecar(ordinary_sidecar);
    const auto ordinary_result = copperfin::studio::open_document({.path = ordinary_primary.string()});
    expect(ordinary_result.ok && ordinary_result.document.path == ordinary_primary.string() &&
               ordinary_result.document.sidecar_path == ordinary_sidecar.string() &&
               ordinary_result.document.has_sidecar,
           "#3990: ordinary PJX/PJT primary opens should remain unchanged");

    const fs::path ambiguous_sidecar = temp_dir / "ambiguous.PJT";
    const fs::path ambiguous_primary_one = temp_dir / "Ambiguous.PJX";
    const fs::path ambiguous_primary_two = temp_dir / "AMBIGUOUS.pjx";
    write_sidecar(ambiguous_sidecar);
    write_primary(ambiguous_primary_one);
    write_primary(ambiguous_primary_two);
    const auto has_exact_entry = [&](std::string_view filename) {
        for (const auto& entry : fs::directory_iterator(temp_dir)) {
            if (entry.path().filename().string() == filename) {
                return true;
            }
        }
        return false;
    };

    const fs::path payload_primary = temp_dir / "payload.pjx";
    const fs::path inferred_payload_sidecar = temp_dir / "payload.pjt";
    const fs::path opened_payload_sidecar = temp_dir / "payload.PJT";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> payload_fields{
        {.name = "BODY", .type = 'M', .length = 4U}
    };
    const auto payload_create_result = copperfin::vfp::create_dbf_table_file(
        payload_primary.string(), payload_fields, {{"chosen-payload"}});
    expect(payload_create_result.ok,
           "#3990: exact-sidecar payload fixture should create its primary and inferred memo");
    fs::copy_file(
        inferred_payload_sidecar,
        opened_payload_sidecar,
        fs::copy_options::overwrite_existing,
        ignored);
    {
        std::ofstream corrupt_inferred_sidecar(
            inferred_payload_sidecar,
            std::ios::binary | std::ios::trunc);
        corrupt_inferred_sidecar << "bad";
    }
    if (has_exact_entry("payload.pjt") && has_exact_entry("payload.PJT")) {
        const auto payload_result =
            copperfin::studio::open_document({.path = opened_payload_sidecar.string()});
        expect(payload_result.ok && payload_result.document.table_preview_available &&
                   payload_result.document.table_preview.records.size() == 1U,
               "#3990: direct exact-sidecar open should parse through the selected memo file");
        if (payload_result.ok && payload_result.document.table_preview_available &&
            payload_result.document.table_preview.records.size() == 1U &&
            !payload_result.document.table_preview.records[0U].values.empty()) {
            expect(payload_result.document.table_preview.records[0U].values[0U].display_value ==
                       "chosen-payload",
                   "#3990: table preview should consume the exact opened sidecar, not an inferred decoy");
        }
        expect(std::none_of(
                   payload_result.document.inspection.validation_issues.begin(),
                   payload_result.document.inspection.validation_issues.end(),
                   [](const copperfin::vfp::AssetValidationIssue& issue) {
                       return issue.code == "memo.sidecar_header_truncated";
                   }),
               "#3990: inspection should validate the exact opened sidecar, not an inferred decoy");
    }

    const fs::path sidecar_ambiguous_primary = temp_dir / "sidecar.pjx";
    const fs::path sidecar_ambiguous_one = temp_dir / "Sidecar.PJT";
    const fs::path sidecar_ambiguous_two = temp_dir / "SIDECAR.pjt";
    const fs::path sidecar_ambiguous_request = temp_dir / "sidecar.pjt";
    write_primary(sidecar_ambiguous_primary);
    write_sidecar(sidecar_ambiguous_one);
    write_sidecar(sidecar_ambiguous_two);
    if (has_exact_entry("Sidecar.PJT") && has_exact_entry("SIDECAR.pjt")) {
        const auto sidecar_ambiguous_result =
            copperfin::studio::open_document({.path = sidecar_ambiguous_request.string()});
        expect(!sidecar_ambiguous_result.ok &&
                   sidecar_ambiguous_result.error == english_catalog.translate(
                       "Studio.DocumentOpen.Error.SidecarPathAmbiguous",
                       {{"path", sidecar_ambiguous_request.string()}}),
               "#3990: multiple case-fold sidecar inputs should fail through the localized diagnostic");

        write_sidecar(sidecar_ambiguous_request);
        const auto sidecar_exact_result =
            copperfin::studio::open_document({.path = sidecar_ambiguous_request.string()});
        expect(sidecar_exact_result.ok &&
                   sidecar_exact_result.document.path == sidecar_ambiguous_primary.string() &&
                   sidecar_exact_result.document.sidecar_path == sidecar_ambiguous_request.string(),
               "#3990: an exact sidecar filename should win over case-fold siblings");
    }

    const fs::path legacy_primary = temp_dir / "legacy.pjx";
    const fs::path legacy_sidecar_one = temp_dir / "Legacy.PJT";
    const fs::path legacy_sidecar_two = temp_dir / "LEGACY.pjt";
    write_primary(legacy_primary);
    write_sidecar(legacy_sidecar_one);
    write_sidecar(legacy_sidecar_two);
    if (has_exact_entry("Legacy.PJT") && has_exact_entry("LEGACY.pjt")) {
        const auto legacy_result = copperfin::studio::open_document({.path = legacy_primary.string()});
        expect(!legacy_result.ok && legacy_result.error == english_catalog.translate(
                   "Studio.DocumentOpen.Error.SidecarPathAmbiguous",
                   {{"path", (temp_dir / "legacy.pjt").string()}}),
               "#3992: ordinary primary opens should reject ambiguous sidecars");
    }

    if (has_exact_entry("Ambiguous.PJX") && has_exact_entry("AMBIGUOUS.pjx")) {
        const auto ambiguous_result =
            copperfin::studio::open_document({.path = ambiguous_sidecar.string()});
        expect(!ambiguous_result.ok &&
                   ambiguous_result.error == english_catalog.translate(
                       "Studio.DocumentOpen.Error.SidecarPrimaryAmbiguous",
                       {{"path", ambiguous_sidecar.string()}}),
               "#3990: multiple case-fold primary matches should fail through the localized diagnostic");

        const fs::path exact_primary = temp_dir / "ambiguous.pjx";
        write_primary(exact_primary);
        const auto exact_result = copperfin::studio::open_document({.path = ambiguous_sidecar.string()});
        expect(exact_result.ok && exact_result.document.path == exact_primary.string(),
               "#3990: an exact primary filename should win over case-fold siblings");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_infers_read_only_from_asset_family_writability() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_read_only_document_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "customer.scx";
    const fs::path sidecar_path = temp_dir / "customer.sct";

    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo-sidecar";
    }

    const auto writable_result = copperfin::studio::open_document({
        .path = form_path.string(),
        .read_only = false
    });
    expect(writable_result.ok, "#3506: writable sidecar-backed assets should open");
    expect(!writable_result.document.read_only,
           "#3506: writable sidecar-backed assets should remain editable unless explicitly launched read-only");

    const auto main_original_permissions = fs::status(form_path, ignored).permissions();
    const auto sidecar_original_permissions = fs::status(sidecar_path, ignored).permissions();
    fs::permissions(
        sidecar_path,
        fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write,
        fs::perm_options::remove,
        ignored);

    const auto read_only_sidecar_result = copperfin::studio::open_document({
        .path = form_path.string(),
        .read_only = false
    });
    expect(read_only_sidecar_result.ok, "#3506: sidecar read-only assets should still open");
    expect(read_only_sidecar_result.document.read_only,
           "#3506: sidecar-backed assets should report read-only when the existing sidecar is not writable");

    fs::permissions(sidecar_path, sidecar_original_permissions, fs::perm_options::replace, ignored);
    fs::permissions(
        form_path,
        fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write,
        fs::perm_options::remove,
        ignored);

    const auto read_only_main_result = copperfin::studio::open_document({
        .path = form_path.string(),
        .read_only = false
    });
    expect(read_only_main_result.ok, "#3506: main-file read-only assets should still open");
    expect(read_only_main_result.document.read_only,
           "#3506: sidecar-backed assets should report read-only when the main asset file is not writable");

    fs::permissions(form_path, main_original_permissions, fs::perm_options::replace, ignored);

    const auto explicit_read_only_result = copperfin::studio::open_document({
        .path = form_path.string(),
        .read_only = true
    });
    expect(explicit_read_only_result.ok, "#3506: explicit read-only launches should still open");
    expect(explicit_read_only_result.document.read_only,
           "#3506: explicit read-only launches should continue to force read-only document state");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_uses_vfp_filename_for_display_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_vfp_filename_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

#if defined(_WIN32)
    const fs::path asset_dir = temp_dir / "E" / "Forms";
    fs::create_directories(asset_dir);
    const fs::path form_path = asset_dir / "customer.scx";
    const fs::path sidecar_path = asset_dir / "customer.sct";
    expect(form_path.string().find('\\') != std::string::npos,
           "#3906: Windows host fixture should exercise native backslash path text");
    expect(copperfin::studio::infer_sidecar_path(
               R"(E:\Forms\customer.scx)", copperfin::studio::StudioAssetKind::form) ==
               R"(E:\Forms\customer.sct)",
           "#3906: Windows logical VFP paths should infer sidecars without requiring host-file lookup");
#else
    const fs::path form_path = temp_dir / R"(E:\Forms\customer.scx)";
    const fs::path sidecar_path = temp_dir / R"(E:\Forms\customer.sct)";
#endif

    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo-sidecar";
    }

    const auto result = copperfin::studio::open_document({
        .path = form_path.string(),
        .launched_from_visual_studio = true
    });

    expect(result.ok, "#702: open_document should accept synthetic Windows-style VFP path names on the host filesystem");
    expect(result.document.display_name == "customer.scx",
           "#702: Studio display names should use VFP-aware filename parsing for backslash paths");
    expect(result.document.has_sidecar, "#702: sidecar inference should remain compatible with VFP-style path text");
    expect(result.document.sidecar_path == sidecar_path.string(),
           "#702: inferred sidecar path should still replace the extension in the host path");

    const auto direct_sidecar_result = copperfin::studio::open_document({
        .path = sidecar_path.string(),
        .launched_from_visual_studio = true
    });
    expect(direct_sidecar_result.ok,
           "#3990: direct Windows-style sidecar paths should canonicalize to their primary");
    if (direct_sidecar_result.ok) {
        expect(direct_sidecar_result.document.path == form_path.string() &&
                   direct_sidecar_result.document.sidecar_path == sidecar_path.string() &&
                   direct_sidecar_result.document.has_sidecar,
               "#3990: direct Windows-style sidecar canonicalization should preserve path identities");
        expect(direct_sidecar_result.document.display_name == "customer.scx" &&
                   direct_sidecar_result.document.launched_from_visual_studio,
               "#3990: direct Windows-style sidecar canonicalization should preserve display and launch metadata");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_attaches_default_designer_contexts() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_designer_context_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto write_synthetic_asset = [&](const std::string& filename) {
        const fs::path path = temp_dir / filename;
        const auto bytes = make_vfp_header();
        std::ofstream output(path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        return path;
    };

    const auto form_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("customer.scx").string()
    });
    expect(form_result.ok, "#960: synthetic form should open for designer-context checks");
    expect(form_result.document.designer_contexts.size() == 1U,
           "#960: form documents should expose one default designer context");
    if (!form_result.document.designer_contexts.empty()) {
        const auto& context = form_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object,
               "#960: form documents should expose the visual-object designer context");
        expect(context.editor_action_count == context.editor_actions.size(),
               "#1009: form designer context should report editor-action count metadata");
        expect(context.builder_count == context.builders.size(),
               "#1009: form designer context should report builder count metadata");
        expect(context.builder_count == 3U,
               "#1010: form designer context should expose form plus control builders");
        expect(context.toolbox_item_count == context.toolbox_items.size(),
               "#1009: form designer context should report toolbox-item count metadata");
        expect(has_descriptor_id(context.editor_actions, "show-property-grid"),
               "#960: form designer context should include property-grid actions");
        expect(has_descriptor_id(context.builders, "form-builder"),
               "#1010: form designer context should include form builder");
        expect(has_descriptor_id(context.builders, "control-builder"),
               "#960: form designer context should include control builders");
        expect(has_descriptor_id(context.toolbox_items, "textbox"),
               "#960: form designer context should include form toolbox items");
    }

    const auto container_override_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::container_object
        }
    });
    expect(container_override_result.ok, "#1014: synthetic form should open for explicit container context checks");
    expect(container_override_result.document.designer_contexts.size() == 1U,
           "#1014: explicit container contexts should override the form default context list");
    if (!container_override_result.document.designer_contexts.empty()) {
        const auto& context = container_override_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::container_object,
               "#1014: explicit container contexts should be preserved");
        expect(has_descriptor_id(context.editor_actions, "edit-visual-method"),
               "#1014: explicit container contexts should include method-editor actions");
        expect(has_descriptor_id(context.builders, "control-builder"),
               "#1014: explicit container contexts should include control builders");
        expect(!has_descriptor_id(context.builders, "form-builder"),
               "#1014: explicit container contexts should not expose form builders");
        expect(has_descriptor_id(context.toolbox_items, "checkbox"),
               "#1014: explicit container contexts should include container-safe toolbox items");
    }

    const auto class_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("customer.vcx").string()
    });
    expect(class_result.ok, "#1012: synthetic class library should open for designer-context checks");
    expect(class_result.document.designer_contexts.size() == 1U,
           "#1012: class-library documents should expose one default designer context");
    if (!class_result.document.designer_contexts.empty()) {
        const auto& context = class_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::class_designer,
               "#1012: class-library documents should expose the class-designer context");
        expect(has_descriptor_id(context.editor_actions, "show-property-grid"),
               "#1012: class designer context should include property-grid actions");
        expect(has_descriptor_id(context.editor_actions, "edit-visual-method"),
               "#1012: class designer context should include method-editor actions");
        expect(has_descriptor_id(context.builders, "class-builder"),
               "#1012: class designer context should include class builders");
        expect(!has_descriptor_id(context.builders, "form-builder"),
               "#1012: class designer context should not expose form builders");
        expect(!has_descriptor_id(context.builders, "control-builder"),
               "#1012: class designer context should not expose control builders");
        expect(has_descriptor_id(context.toolbox_items, "textbox"),
               "#1012: class designer context should include class-safe toolbox items");
    }

    const auto method_symbol_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .symbol = "cmdSave.Click"
    });
    expect(method_symbol_result.ok, "#963: synthetic form should open for method-symbol context checks");
    expect(method_symbol_result.document.designer_contexts.size() == 1U,
           "#963: method-symbol form documents should expose one inferred designer context");
    if (!method_symbol_result.document.designer_contexts.empty()) {
        const auto& context = method_symbol_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_method,
               "#963: method-like symbols should infer the visual-method designer context for forms");
        expect(has_descriptor_id(context.editor_actions, "edit-visual-method"),
               "#963: inferred visual-method contexts should include method-editor actions");
    }

    const auto data_environment_symbol_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .symbol = "Dataenvironment.OpenTables"
    });
    expect(data_environment_symbol_result.ok, "#965: synthetic form should open for data-environment symbol checks");
    expect(data_environment_symbol_result.document.designer_contexts.size() == 1U,
           "#965: data-environment symbols should expose one inferred designer context");
    if (!data_environment_symbol_result.document.designer_contexts.empty()) {
        const auto& context = data_environment_symbol_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#965: DataEnvironment method symbols should infer the data-environment designer context for forms");
        expect(context.editor_action_count == context.editor_actions.size() &&
                   context.builder_count == context.builders.size() &&
                   context.toolbox_item_count == 0U,
               "#1009: inferred data-environment contexts should report filtered descriptor counts");
        expect(has_descriptor_id(context.editor_actions, "edit-data-environment"),
               "#965: inferred data-environment contexts should include data-environment editor actions");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#965: inferred data-environment contexts should include data-environment builders");
    }

    const fs::path selected_record_path = temp_dir / "selected_record.scx";
    write_synthetic_form_table_with_data_environment(selected_record_path);
    const auto data_environment_record_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 0U,
        .selection_record_available = true
    });
    expect(data_environment_record_result.ok, "#966: synthetic form should open for selected-record context checks");
    expect(data_environment_record_result.document.designer_contexts.size() == 1U,
           "#966: selected DataEnvironment records should expose one inferred designer context");
    if (!data_environment_record_result.document.designer_contexts.empty()) {
        const auto& context = data_environment_record_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#966: selected DataEnvironment records should infer the data-environment designer context");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#966: selected DataEnvironment records should include data-environment builders");
    }

    const auto visual_record_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 1U,
        .selection_record_available = true
    });
    expect(visual_record_result.ok, "#966: synthetic form should open for visual selected-record context checks");
    expect(visual_record_result.document.designer_contexts.size() == 1U,
           "#966: visual selected records should preserve the generic form default context count");
    if (!visual_record_result.document.designer_contexts.empty()) {
        expect(visual_record_result.document.designer_contexts.front().selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::visual_object,
               "#966: non-DataEnvironment selected records should preserve visual-object defaults");
    }

    const auto container_record_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 2U,
        .selection_record_available = true
    });
    expect(container_record_result.ok, "#1015: synthetic form should open for selected container context checks");
    expect(container_record_result.document.designer_contexts.size() == 1U,
           "#1015: selected container records should expose one inferred designer context");
    if (!container_record_result.document.designer_contexts.empty()) {
        const auto& context = container_record_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::container_object,
               "#1015: selected container records should infer the container-object designer context");
        expect(has_descriptor_id(context.builders, "control-builder"),
               "#1015: selected container records should include control builders");
        expect(!has_descriptor_id(context.builders, "form-builder"),
               "#1015: selected container records should not include form builders");
        expect(has_descriptor_id(context.toolbox_items, "checkbox"),
               "#1015: selected container records should include container-safe toolbox items");
    }

    const auto multi_override_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::visual_method,
            copperfin::studio::StudioEditorSelectionContext::report_expression
        }
    });
    expect(multi_override_result.ok, "#962: synthetic form should open for explicit designer-context checks");
    expect(multi_override_result.document.designer_contexts.size() == 2U,
           "#962: explicit selection contexts should override the form default context list");
    if (multi_override_result.document.designer_contexts.size() == 2U) {
        expect(multi_override_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::visual_method,
               "#962: explicit visual_method contexts should be preserved in request order");
        expect(has_descriptor_id(multi_override_result.document.designer_contexts[0].editor_actions, "edit-visual-method"),
               "#962: explicit visual_method contexts should include method-editor actions");
        expect(multi_override_result.document.designer_contexts[1].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#962: explicit report_expression contexts should be preserved in request order");
        expect(has_descriptor_id(multi_override_result.document.designer_contexts[1].editor_actions, "edit-report-expression"),
               "#962: explicit report_expression contexts should include expression-editor actions");
    }

    const auto override_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 0U,
        .selection_record_available = true,
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::report_expression
        }
    });
    expect(override_result.ok, "#962: synthetic form should open for explicit designer-context checks");
    expect(override_result.document.designer_contexts.size() == 1U,
           "#966: explicit selection contexts should override selected-record context defaults");
    if (override_result.document.designer_contexts.size() == 1U) {
        expect(override_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#966: explicit report_expression contexts should win over selected-record data-environment contexts");
        expect(has_descriptor_id(override_result.document.designer_contexts[0].editor_actions, "edit-report-expression"),
               "#962: explicit report_expression contexts should include expression-editor actions");
    }

    const auto container_override_precedence_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 2U,
        .selection_record_available = true,
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::report_expression
        }
    });
    expect(container_override_precedence_result.ok,
           "#1015: synthetic form should open for explicit-over-container checks");
    expect(container_override_precedence_result.document.designer_contexts.size() == 1U,
           "#1015: explicit selection contexts should override selected-record container defaults");
    if (container_override_precedence_result.document.designer_contexts.size() == 1U) {
        expect(container_override_precedence_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#1015: explicit report_expression contexts should win over selected-record container contexts");
    }

    const auto report_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("summary.frx").string()
    });
    expect(report_result.ok, "#960: synthetic report should open for designer-context checks");
    expect(report_result.document.designer_contexts.size() == 1U,
           "#960: report documents should expose one default designer context");
    if (!report_result.document.designer_contexts.empty()) {
        const auto& context = report_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#960: report documents should expose the report-expression designer context");
        expect(context.editor_action_count == context.editor_actions.size() &&
                   context.builder_count == context.builders.size() &&
                   context.toolbox_item_count == context.toolbox_items.size(),
               "#1009: report designer context should report descriptor counts");
        expect(has_descriptor_id(context.editor_actions, "edit-report-expression"),
               "#960: report designer context should include expression editor actions");
        expect(has_descriptor_id(context.builders, "report-builder"),
               "#960: report designer context should include report builders");
        expect(has_descriptor_id(context.toolbox_items, "label"),
               "#960: report designer context should include report-safe toolbox items");
        expect(!has_descriptor_id(context.toolbox_items, "textbox"),
               "#960: report designer context should exclude form-only toolbox items");
    }

    const auto report_data_environment_symbol_result = copperfin::studio::open_document({
        .path = (temp_dir / "summary.frx").string(),
        .symbol = "Dataenvironment.OpenTables"
    });
    expect(report_data_environment_symbol_result.ok,
           "#1016: synthetic report should open for data-environment symbol checks");
    expect(report_data_environment_symbol_result.document.designer_contexts.size() == 1U,
           "#1016: report DataEnvironment symbols should expose one inferred designer context");
    if (!report_data_environment_symbol_result.document.designer_contexts.empty()) {
        const auto& context = report_data_environment_symbol_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#1016: report DataEnvironment symbols should infer the data-environment designer context");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#1016: report DataEnvironment symbols should include data-environment builders");
    }

    const fs::path report_data_environment_path = temp_dir / "report_data_environment.frx";
    write_synthetic_form_table_with_data_environment(report_data_environment_path);
    const auto report_data_environment_record_result = copperfin::studio::open_document({
        .path = report_data_environment_path.string(),
        .record_index = 0U,
        .selection_record_available = true
    });
    expect(report_data_environment_record_result.ok,
           "#1016: synthetic report should open for selected DataEnvironment record checks");
    expect(report_data_environment_record_result.document.designer_contexts.size() == 1U,
           "#1016: selected report DataEnvironment records should expose one inferred designer context");
    if (!report_data_environment_record_result.document.designer_contexts.empty()) {
        expect(report_data_environment_record_result.document.designer_contexts.front().selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#1016: selected report DataEnvironment records should infer the data-environment designer context");
    }

    const auto label_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("mailing.lbx").string()
    });
    expect(label_result.ok, "#1011: synthetic label should open for designer-context checks");
    expect(label_result.document.designer_contexts.size() == 1U,
           "#1011: label documents should expose one default designer context");
    if (!label_result.document.designer_contexts.empty()) {
        const auto& context = label_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::label_expression,
               "#1011: label documents should expose the label-expression designer context");
        expect(has_descriptor_id(context.editor_actions, "edit-report-expression"),
               "#1011: label designer context should include expression editor actions");
        expect(has_descriptor_id(context.builders, "label-wizard"),
               "#1011: label designer context should include label wizard builders");
        expect(!has_descriptor_id(context.builders, "report-builder"),
               "#1011: label designer context should not reuse report builders");
        expect(has_descriptor_id(context.toolbox_items, "label"),
               "#1011: label designer context should include report-safe toolbox items");
    }

    const fs::path label_data_environment_path = temp_dir / "label_data_environment.lbx";
    write_synthetic_form_table_with_data_environment(label_data_environment_path);
    const auto label_data_environment_record_result = copperfin::studio::open_document({
        .path = label_data_environment_path.string(),
        .record_index = 0U,
        .selection_record_available = true
    });
    expect(label_data_environment_record_result.ok,
           "#1016: synthetic label should open for selected DataEnvironment record checks");
    expect(label_data_environment_record_result.document.designer_contexts.size() == 1U,
           "#1016: selected label DataEnvironment records should expose one inferred designer context");
    if (!label_data_environment_record_result.document.designer_contexts.empty()) {
        expect(label_data_environment_record_result.document.designer_contexts.front().selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#1016: selected label DataEnvironment records should infer the data-environment designer context");
    }

    const auto label_override_precedence_result = copperfin::studio::open_document({
        .path = label_data_environment_path.string(),
        .record_index = 0U,
        .selection_record_available = true,
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::label_expression
        }
    });
    expect(label_override_precedence_result.ok,
           "#1016: synthetic label should open for explicit-over-DataEnvironment checks");
    expect(label_override_precedence_result.document.designer_contexts.size() == 1U,
           "#1016: explicit selection contexts should override selected label DataEnvironment defaults");
    if (label_override_precedence_result.document.designer_contexts.size() == 1U) {
        expect(label_override_precedence_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::label_expression,
               "#1016: explicit label_expression contexts should win over selected-record data-environment contexts");
    }

    const auto menu_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("mainmenu.mnx").string()
    });
    expect(menu_result.ok, "#1013: synthetic menu should open for designer-context checks");
    expect(menu_result.document.designer_contexts.size() == 1U,
           "#1013: menu documents should expose one default designer context");
    if (!menu_result.document.designer_contexts.empty()) {
        const auto& context = menu_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item,
               "#1013: menu documents should expose the menu-item designer context");
        expect(has_descriptor_id(context.editor_actions, "show-property-grid"),
               "#1013: menu designer context should include property-grid actions");
        expect(has_descriptor_id(context.editor_actions, "open-builder"),
               "#1013: menu designer context should include builder actions");
        expect(has_descriptor_id(context.builders, "menu-designer"),
               "#1013: menu designer context should include menu designer builders");
        expect(!has_descriptor_id(context.builders, "form-builder"),
               "#1013: menu designer context should not expose form builders");
        expect(context.toolbox_items.empty(),
               "#1013: menu designer context should not expose toolbox items");
    }

    const auto project_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("demo.pjx").string()
    });
    expect(project_result.ok, "#960: synthetic project should open for designer-context checks");
    expect(project_result.document.designer_contexts.size() == 1U,
           "#960: project documents should expose one default designer context");
    if (!project_result.document.designer_contexts.empty()) {
        const auto& context = project_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::project_item,
               "#960: project documents should expose the project-item designer context");
        expect(has_descriptor_id(context.editor_actions, "navigate-project-item"),
               "#960: project designer context should include project navigation actions");
        expect(has_descriptor_id(context.builders, "application-wizard"),
               "#960: project designer context should include application wizard builders");
        expect(context.toolbox_items.empty(), "#960: project designer context should not expose toolbox items");
    }

    const auto database_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("data.dbc").string()
    });
    expect(database_result.ok, "#960: synthetic database container should open for designer-context checks");
    expect(database_result.document.designer_contexts.size() == 1U,
           "#960: database documents should expose one default designer context");
    if (!database_result.document.designer_contexts.empty()) {
        const auto& context = database_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#960: database documents should expose the data-environment designer context");
        expect(has_descriptor_id(context.editor_actions, "edit-data-environment"),
               "#960: data designer context should include data-environment actions");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#960: data designer context should include data-environment builders");
        expect(context.toolbox_items.empty(), "#960: data designer context should not expose toolbox items");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_launch_selection_record_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_selection_metadata_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "customer.scx";
    const auto bytes = make_vfp_header();
    {
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto result = copperfin::studio::open_document({
        .path = form_path.string(),
        .symbol = "cmdSave.Click",
        .line = 42U,
        .column = 7U,
        .record_index = 5U,
        .selection_record_available = true
    });

    expect(result.ok, "#964: synthetic form should open for launch selection metadata checks");
    expect(result.document.selection_symbol == "cmdSave.Click",
           "#964: open_document should preserve launch selection symbols");
    expect(result.document.selection_line == 42U,
           "#964: open_document should preserve launch selection lines");
    expect(result.document.selection_column == 7U,
           "#964: open_document should preserve launch selection columns");
    expect(result.document.selection_record_index == 5U,
           "#964: open_document should preserve launch selection record indexes");
    expect(result.document.selection_record_available,
           "#967: open_document should preserve explicit launch selection record availability");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_loads_full_table_preview_for_report_and_label_assets() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_full_table_preview_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto write_dense_layout_fixture = [](const fs::path& path) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "OBJCODE", .type = 'N', .length = 8U},
            {.name = "EXPR", .type = 'M', .length = 4U},
            {.name = "HPOS", .type = 'N', .length = 10U},
            {.name = "VPOS", .type = 'N', .length = 10U},
            {.name = "WIDTH", .type = 'N', .length = 10U},
            {.name = "HEIGHT", .type = 'N', .length = 10U},
            {.name = "UNIQUEID", .type = 'C', .length = 24U}
        };
        const std::vector<std::vector<std::string>> records{
            {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
            {"9", "1", "", "", "0", "", "1200", "title-section-guid"},
            {"9", "4", "", "", "1200", "", "6000", "detail-section-guid"},
            {"5", "", "\"Header\"", "100", "100", "500", "150", "title-label-guid"},
            {"8", "0", "first.value", "100", "1600", "100", "250", "detail-field-1-guid"},
            {"8", "0", "second.value", "300", "1600", "100", "250", "detail-field-2-guid"},
            {"8", "0", "third.value", "500", "1600", "100", "250", "detail-field-3-guid"},
            {"8", "0", "fourth.value", "700", "1600", "100", "250", "detail-field-4-guid"},
            {"8", "0", "fifth.value", "900", "1600", "100", "250", "detail-field-5-guid"},
            {"8", "0", "sixth.value", "1100", "1600", "100", "250", "detail-field-6-guid"}
        };

        const auto create_result = copperfin::vfp::create_dbf_table_file(path.string(), fields, records);
        expect(create_result.ok, "#2986: dense report/label preview fixture should be created");
    };

    const fs::path report_path = temp_dir / "full_preview.frx";
    const fs::path label_path = temp_dir / "full_preview.lbx";
    write_dense_layout_fixture(report_path);
    write_dense_layout_fixture(label_path);

    const auto report_result = copperfin::studio::open_document({
        .path = report_path.string(),
        .read_only = true
    });
    expect(report_result.ok, "#2986: report documents should open for full-preview checks");
    expect(report_result.document.table_preview_available,
           "#2986: report documents should expose a table preview");
    expect(report_result.document.table_preview.records.size() == 10U,
           "#2986: report documents should load the full FRX table preview by default");
    if (report_result.document.table_preview.records.size() == 10U) {
        expect(report_result.document.table_preview.records.back().record_index == 9U,
               "#2986: report full-table previews should preserve the last FRX record index");
    }

    const auto label_result = copperfin::studio::open_document({
        .path = label_path.string(),
        .read_only = true
    });
    expect(label_result.ok, "#2986: label documents should open for full-preview checks");
    expect(label_result.document.table_preview_available,
           "#2986: label documents should expose a table preview");
    expect(label_result.document.table_preview.records.size() == 10U,
           "#2986: label documents should load the full LBX table preview by default");
    if (label_result.document.table_preview.records.size() == 10U) {
        expect(label_result.document.table_preview.records.back().record_index == 9U,
               "#2986: label full-table previews should preserve the last LBX record index");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_object_snapshot_preserves_empty_and_null_design_fields() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\customer.scx)";
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 7U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'M', .is_null = false, .display_value = "cmdSave", .memo_block_number = 101U},
                {.field_name = "OBJTYPE", .field_type = 'N', .is_null = false, .display_value = "8.000", .memo_block_number = 102U},
                {.field_name = "OBJCODE", .field_type = 'N', .is_null = false, .display_value = "1.000", .memo_block_number = 103U},
                {.field_name = "PLATFORM", .field_type = 'C', .is_null = false, .display_value = "WINDOWS", .memo_block_number = 104U},
                {.field_name = "PARENT", .field_type = 'M', .is_null = false, .display_value = "frmCustomer", .memo_block_number = 105U},
                {.field_name = "HELP", .field_type = 'M', .is_null = true, .display_value = "", .memo_block_number = 0U},
                {.field_name = "TAG", .field_type = 'M', .is_null = false, .display_value = ""},
                {.field_name = "PROPERTIES", .field_type = 'M', .is_null = false, .display_value = "Caption = Save\r\nEnabled = .T.", .memo_block_number = 7U},
                {.field_name = "UNIQUEID", .field_type = 'M', .is_null = false, .display_value = "cmd-save-1", .memo_block_number = 108U},
                {.field_name = "CLASS", .field_type = 'M', .is_null = false, .display_value = "commandbutton", .memo_block_number = 109U},
                {.field_name = "BASECLASS", .field_type = 'M', .is_null = false, .display_value = "commandbutton", .memo_block_number = 110U}
            }
        }
    };

    const auto objects = copperfin::studio::build_object_snapshot(document);
    expect(objects.size() == 1U, "#658: form design snapshot should include the parsed record");
    if (!objects.empty()) {
        expect(objects[0].objtype_code == 8, "#667: object snapshots should expose raw OBJTYPE metadata");
        expect(objects[0].objtype_field_index == 1U, "#671: raw OBJTYPE metadata should retain DBF field provenance");
        expect(objects[0].objtype_memo_block_number == 102U, "#724: raw OBJTYPE metadata should retain memo block provenance");
        expect(objects[0].objcode_code == 1, "#667: object snapshots should expose raw OBJCODE metadata");
        expect(objects[0].objcode_field_index == 2U, "#671: raw OBJCODE metadata should retain DBF field provenance");
        expect(objects[0].objcode_memo_block_number == 103U, "#724: raw OBJCODE metadata should retain memo block provenance");
        expect(objects[0].platform == "WINDOWS", "#667: object snapshots should expose raw PLATFORM metadata");
        expect(objects[0].platform_field_index == 3U, "#671: raw PLATFORM metadata should retain DBF field provenance");
        expect(objects[0].platform_memo_block_number == 104U, "#724: raw PLATFORM metadata should retain memo block provenance");
        expect(objects[0].object_name == "cmdSave", "#660: object snapshots should expose the design object name");
        expect(objects[0].object_name_field_index == 0U, "#672: object name metadata should retain DBF field provenance");
        expect(objects[0].object_name_memo_block_number == 101U, "#717: object names should retain selected memo block provenance");
        expect(objects[0].unique_id == "cmd-save-1", "#660: object snapshots should expose stable UNIQUEID metadata");
        expect(objects[0].unique_id_field_index == 8U, "#672: UNIQUEID metadata should retain DBF field provenance");
        expect(objects[0].unique_id_memo_block_number == 108U, "#717: unique IDs should retain selected memo block provenance");
        expect(objects[0].parent_name == "frmCustomer", "#660: object snapshots should expose parent hierarchy metadata");
        expect(objects[0].parent_name_field_index == 4U, "#672: parent hierarchy metadata should retain DBF field provenance");
        expect(objects[0].parent_name_memo_block_number == 105U, "#717: parent names should retain selected memo block provenance");
        expect(objects[0].class_name == "commandbutton", "#660: object snapshots should expose CLASS metadata");
        expect(objects[0].class_name_field_index == 9U, "#672: CLASS metadata should retain DBF field provenance");
        expect(objects[0].class_name_memo_block_number == 109U, "#717: class names should retain selected memo block provenance");
        expect(objects[0].baseclass_name == "commandbutton", "#660: object snapshots should expose BASECLASS metadata");
        expect(objects[0].baseclass_name_field_index == 10U, "#672: BASECLASS metadata should retain DBF field provenance");
        expect(objects[0].baseclass_name_memo_block_number == 110U, "#717: baseclass names should retain selected memo block provenance");
        expect(objects[0].title == "cmdSave", "#673: friendly titles should keep existing form selection priority");
        expect(objects[0].title_field_index == 0U, "#673: friendly title metadata should retain selected DBF field provenance");
        expect(objects[0].title_memo_block_number == 101U, "#717: friendly titles should inherit selected field memo block provenance");
        expect(objects[0].subtitle == "commandbutton", "#673: friendly subtitles should keep existing form selection priority");
        expect(objects[0].subtitle_field_index == 10U, "#673: friendly subtitle metadata should retain selected DBF field provenance");
        expect(objects[0].subtitle_memo_block_number == 110U, "#717: friendly subtitles should inherit selected field memo block provenance");
        const auto parent = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "PARENT";
        });
        const auto tag = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "TAG";
        });
        const auto help = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "HELP";
        });
        const auto caption = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "Caption";
        });
        const auto enabled = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "Enabled";
        });

        expect(parent != objects[0].properties.end(), "#660: parent design field should stay in object snapshots");
        if (parent != objects[0].properties.end()) {
            expect(parent->value == "frmCustomer", "#660: parent field should remain available as direct property metadata");
            expect(parent->field_index == 4U, "#659: direct design fields should preserve their DBF field ordinal");
            expect(!parent->derived_from_property_blob, "#659: direct DBF fields should not be marked blob-derived");
            expect(parent->source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
                "#684: direct DBF fields should not masquerade as property-blob line metadata");
            expect(parent->memo_block_number == 105U, "#717: memo-backed direct identity properties should retain memo block provenance");
        }
        expect(help != objects[0].properties.end(), "#658: null design fields should stay in object snapshots");
        if (help != objects[0].properties.end()) {
            expect(help->is_null, "#658: null design field metadata should stay attached");
            expect(help->field_index == 5U, "#659: null direct fields should preserve their DBF field ordinal");
            expect(help->memo_block_number == 0U, "#712: null block-zero memo properties should expose memo block zero");
        }
        expect(tag != objects[0].properties.end(), "#658: empty memo-backed design fields should stay in object snapshots");
        if (tag != objects[0].properties.end()) {
            expect(tag->value.empty(), "#658: empty design fields should preserve their empty value");
            expect(tag->field_index == 6U, "#659: empty direct fields should preserve their DBF field ordinal");
            expect(tag->memo_block_number == 0U, "#712: empty direct memo properties should expose memo block zero");
        }
        expect(caption != objects[0].properties.end(), "#658: visual property blob expansion should still work");
        if (caption != objects[0].properties.end()) {
            expect(caption->field_index == 7U, "#659: blob-derived properties should retain the source PROPERTIES field ordinal");
            expect(caption->derived_from_property_blob, "#659: blob-derived properties should expose their provenance");
            expect(caption->source_line_index == 0U, "#684: first blob-derived property should retain its source memo line");
            expect(caption->memo_block_number == 7U, "#712: blob-derived properties should inherit the source PROPERTIES memo block");
        }
        expect(enabled != objects[0].properties.end(), "#684: second visual property blob line should expand into snapshots");
        if (enabled != objects[0].properties.end()) {
            expect(enabled->field_index == 7U, "#684: later blob-derived properties should retain the source PROPERTIES field ordinal");
            expect(enabled->source_line_index == 1U, "#684: later blob-derived properties should retain their source memo line");
            expect(enabled->memo_block_number == 7U, "#712: later blob-derived properties should inherit the source PROPERTIES memo block");
        }
    }
}

void test_object_snapshot_suppresses_unresolved_memo_placeholders() {
    copperfin::studio::StudioDocumentModel form_document;
    form_document.path = R"(E:\Project-Copperfin\samples\placeholder.scx)";
    form_document.kind = copperfin::studio::StudioAssetKind::form;
    form_document.table_preview_available = true;
    form_document.table_preview.records = {
        {
            .record_index = 10U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'M', .is_null = false, .display_value = "<memo block 40>", .memo_block_number = 40U},
                {.field_name = "NAME", .field_type = 'M', .is_null = false, .display_value = "<memo block 41>"},
                {.field_name = "TITLE", .field_type = 'M', .is_null = false, .display_value = "<memo block 42>"},
                {.field_name = "CLASS", .field_type = 'M', .is_null = false, .display_value = "<memo block 43>"},
                {.field_name = "BASECLASS", .field_type = 'M', .is_null = false, .display_value = "<memo block 44>"},
                {.field_name = "PROPERTIES", .field_type = 'M', .is_null = false, .display_value = "<memo block 45>", .memo_block_number = 45U}
            }
        }
    };

    const auto form_objects = copperfin::studio::build_object_snapshot(form_document);
    expect(form_objects.size() == 1U, "#696: unresolved memo placeholders should not prevent object capture");
    if (!form_objects.empty()) {
        expect(form_objects[0].object_name.empty(), "#696: unresolved memo object names should not become active names");
        expect(form_objects[0].object_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#696: active object-name provenance should be missing when usable text is absent");
        expect(form_objects[0].object_name_memo_block_number == 0U,
            "#717: suppressed object-name metadata should expose memo block zero");
        expect(form_objects[0].title == "Record 10", "#696: unresolved memo title sources should use synthetic fallback");
        const auto pseudo_catalog =
            copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "qps-ploc");
        const auto pseudo_form_objects = copperfin::studio::build_object_snapshot(form_document, pseudo_catalog);
        expect(
            pseudo_form_objects.size() == 1U,
            "#2499: pseudo-localized fallback title should preserve object capture");
        if (!pseudo_form_objects.empty()) {
            expect(
                pseudo_form_objects[0].record_index == 10U,
                "#2499: pseudo-localized fallback title should preserve source record index");
            expect(
                pseudo_form_objects[0].title.find("[!! ") != std::string::npos,
                "#2499: document model fallback title should route through pseudo-localization");
            expect(
                pseudo_form_objects[0].title.find("Record 10") == std::string::npos,
                "#2499: pseudo-localized document fallback title should not fall back to raw English prose");
            expect(
                pseudo_form_objects[0].title.find("10") != std::string::npos,
                "#2499: document fallback title should preserve the named recordIndex placeholder value");
            expect(
                pseudo_form_objects[0].object_name == form_objects[0].object_name,
                "#2499: pseudo-localized fallback title should preserve object identity fields");
            expect(
                pseudo_form_objects[0].title_field_index == form_objects[0].title_field_index,
                "#2499: pseudo-localized fallback title should preserve title field provenance");
            expect(
                pseudo_form_objects[0].title_memo_block_number == form_objects[0].title_memo_block_number,
                "#2499: pseudo-localized fallback title should preserve title memo provenance");
        }
        expect(form_objects[0].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#696: synthetic titles should not masquerade as unresolved memo provenance");
        expect(form_objects[0].title_memo_block_number == 0U,
            "#717: synthetic object titles should expose memo block zero");
        expect(form_objects[0].subtitle.empty(), "#696: unresolved memo subtitle sources should remain absent");
        expect(form_objects[0].subtitle_memo_block_number == 0U,
            "#717: suppressed subtitles should expose memo block zero");
        const auto objname = std::find_if(form_objects[0].properties.begin(), form_objects[0].properties.end(), [](const auto& property) {
            return property.name == "OBJNAME";
        });
        expect(objname != form_objects[0].properties.end(), "#696: direct unresolved memo fields should remain visible as properties");
        if (objname != form_objects[0].properties.end()) {
            expect(objname->field_index == 0U, "#696: direct unresolved memo fields should retain field provenance");
            expect(objname->value.empty(), "#696: direct unresolved memo placeholders should not become property values");
            expect(objname->memo_block_number == 40U, "#712: unresolved direct memo fields should retain memo block provenance");
        }
        const auto caption = std::find_if(form_objects[0].properties.begin(), form_objects[0].properties.end(), [](const auto& property) {
            return property.name == "Caption";
        });
        expect(caption == form_objects[0].properties.end(), "#696: unresolved PROPERTIES memo placeholders should not expand blob properties");
    }

    copperfin::studio::StudioDocumentModel menu_document;
    menu_document.path = R"(E:\Project-Copperfin\samples\placeholder.mnx)";
    menu_document.kind = copperfin::studio::StudioAssetKind::menu;
    menu_document.table_preview_available = true;
    menu_document.table_preview.records = {
        {
            .record_index = 11U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "<memo block 46>", .memo_block_number = 46U},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "<memo block 47>", .memo_block_number = 47U},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "<memo block 48>", .memo_block_number = 48U},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "fallback_menu"}
            }
        }
    };

    const auto menu_objects = copperfin::studio::build_object_snapshot(menu_document);
    expect(menu_objects.size() == 1U, "#696: unresolved menu memo placeholders should not prevent object capture");
    if (!menu_objects.empty()) {
        expect(menu_objects[0].menu_prompt.empty(), "#696: unresolved menu PROMPT placeholders should not become prompt text");
        expect(menu_objects[0].menu_prompt_field_index == 0U, "#696: menu PROMPT provenance should remain available");
        expect(menu_objects[0].menu_prompt_memo_block_number == 46U,
            "#718: unresolved menu PROMPT metadata should retain memo block provenance");
        expect(menu_objects[0].menu_command.empty(), "#696: unresolved menu COMMAND placeholders should not become command text");
        expect(menu_objects[0].menu_command_field_index == 1U, "#696: menu COMMAND provenance should remain available");
        expect(menu_objects[0].menu_command_memo_block_number == 47U,
            "#718: unresolved menu COMMAND metadata should retain memo block provenance");
        expect(menu_objects[0].menu_message.empty(), "#696: unresolved menu MESSAGE placeholders should not become message text");
        expect(menu_objects[0].menu_message_field_index == 2U, "#696: menu MESSAGE provenance should remain available");
        expect(menu_objects[0].menu_message_memo_block_number == 48U,
            "#718: unresolved menu MESSAGE metadata should retain memo block provenance");
        expect(menu_objects[0].title == "fallback_menu", "#696: menu titles should skip unresolved PROMPT and use the next usable source");
        expect(menu_objects[0].title_field_index == 3U, "#696: menu title provenance should point at the selected usable source");
    }
}

void test_object_snapshot_trims_normalized_display_metadata() {
    copperfin::studio::StudioDocumentModel form_document;
    form_document.path = R"(E:\Project-Copperfin\samples\trimmed.scx)";
    form_document.kind = copperfin::studio::StudioAssetKind::form;
    form_document.table_preview_available = true;
    form_document.table_preview.records = {
        {
            .record_index = 12U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'C', .is_null = false, .display_value = "   "},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "  cmdSave  "},
                {.field_name = "BASECLASS", .field_type = 'C', .is_null = false, .display_value = "  commandbutton  "},
                {.field_name = "PLATFORM", .field_type = 'C', .is_null = false, .display_value = "  WINDOWS  ", .memo_block_number = 134U},
                {.field_name = "CLASS", .field_type = 'C', .is_null = false, .display_value = "  commandbutton  "}
            }
        }
    };

    const auto form_objects = copperfin::studio::build_object_snapshot(form_document);
    expect(form_objects.size() == 1U, "#705: trimmed form metadata should still produce an object snapshot");
    if (!form_objects.empty()) {
        expect(form_objects[0].object_name == "cmdSave",
            "#705: whitespace-only OBJNAME should be ignored and fallback NAME should be trimmed");
        expect(form_objects[0].object_name_field_index == 1U,
            "#705: object-name provenance should point at the selected fallback field");
        expect(form_objects[0].title == "cmdSave", "#705: form titles should use trimmed display metadata");
        expect(form_objects[0].title_field_index == 1U, "#705: form title provenance should point at trimmed NAME");
        expect(form_objects[0].subtitle == "commandbutton", "#705: form subtitles should use trimmed display metadata");
        expect(form_objects[0].subtitle_field_index == 2U, "#705: form subtitle provenance should point at BASECLASS");
        expect(form_objects[0].platform == "WINDOWS", "#705: platform metadata should be trimmed");
        expect(form_objects[0].platform_field_index == 3U, "#705: platform provenance should still point at PLATFORM");
        expect(form_objects[0].platform_memo_block_number == 134U, "#724: trimmed platform metadata should retain source memo block provenance");
        const auto name = std::find_if(form_objects[0].properties.begin(), form_objects[0].properties.end(), [](const auto& property) {
            return property.name == "NAME";
        });
        expect(name != form_objects[0].properties.end(), "#705: direct NAME property should remain visible");
        if (name != form_objects[0].properties.end()) {
            expect(name->value == "  cmdSave  ", "#705: direct DBF property values should remain source-faithful");
            expect(name->field_index == 1U, "#705: direct DBF property provenance should remain unchanged");
        }
    }

    copperfin::studio::StudioDocumentModel menu_document;
    menu_document.path = R"(E:\Project-Copperfin\samples\trimmed.mnx)";
    menu_document.kind = copperfin::studio::StudioAssetKind::menu;
    menu_document.table_preview_available = true;
    menu_document.table_preview.records = {
        {
            .record_index = 13U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "  Customer  ", .memo_block_number = 130U},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "  MAIN  "},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "  DO FORM customer  ", .memo_block_number = 132U},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "  Open customer  ", .memo_block_number = 133U},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "  customer_menu  "}
            }
        }
    };

    const auto menu_objects = copperfin::studio::build_object_snapshot(menu_document);
    expect(menu_objects.size() == 1U, "#705: trimmed menu metadata should still produce an object snapshot");
    if (!menu_objects.empty()) {
        expect(menu_objects[0].menu_prompt == "Customer", "#705: menu PROMPT metadata should be trimmed");
        expect(menu_objects[0].menu_prompt_memo_block_number == 130U, "#718: decoded menu PROMPT metadata should retain memo block provenance");
        expect(menu_objects[0].menu_level_name == "MAIN", "#705: menu LEVELNAME metadata should be trimmed");
        expect(menu_objects[0].menu_level_name_memo_block_number == 0U, "#718: non-memo menu LEVELNAME metadata should expose memo block zero");
        expect(menu_objects[0].menu_command == "DO FORM customer", "#705: menu COMMAND metadata should be trimmed");
        expect(menu_objects[0].menu_command_memo_block_number == 132U, "#718: decoded menu COMMAND metadata should retain memo block provenance");
        expect(menu_objects[0].menu_message == "Open customer", "#705: menu MESSAGE metadata should be trimmed");
        expect(menu_objects[0].menu_message_memo_block_number == 133U, "#718: decoded menu MESSAGE metadata should retain memo block provenance");
        expect(menu_objects[0].object_name == "customer_menu", "#705: menu object-name fallback should be trimmed");
        expect(menu_objects[0].object_name_field_index == 4U, "#705: menu object-name provenance should stay on NAME");
        expect(menu_objects[0].title == "Customer", "#705: menu title metadata should be trimmed");
        expect(menu_objects[0].title_field_index == 0U, "#705: menu title provenance should stay on PROMPT");
        expect(menu_objects[0].subtitle == "MAIN", "#705: menu subtitle metadata should be trimmed");
        expect(menu_objects[0].subtitle_field_index == 1U, "#705: menu subtitle provenance should stay on LEVELNAME");
        const auto prompt = std::find_if(menu_objects[0].properties.begin(), menu_objects[0].properties.end(), [](const auto& property) {
            return property.name == "PROMPT";
        });
        expect(prompt != menu_objects[0].properties.end(), "#705: direct PROMPT property should remain visible");
        if (prompt != menu_objects[0].properties.end()) {
            expect(prompt->value == "  Customer  ", "#705: direct menu property values should remain source-faithful");
            expect(prompt->field_index == 0U, "#705: direct menu property provenance should remain unchanged");
        }
    }
}

void test_menu_object_snapshot_preserves_normalized_menu_metadata() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\mainmenu.mnx)";
    document.kind = copperfin::studio::StudioAssetKind::menu;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Customer", .memo_block_number = 201U},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "MAIN"},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "DO FORM customer", .memo_block_number = 203U},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "Open customer maintenance", .memo_block_number = 204U},
                {.field_name = "OBJTYPE", .field_type = 'N', .is_null = false, .display_value = "3.000", .memo_block_number = 205U},
                {.field_name = "OBJCODE", .field_type = 'N', .is_null = false, .display_value = "7.000", .memo_block_number = 206U}
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "TOOLS"},
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Tools"},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "DO tools"},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "tools_menu"},
                {.field_name = "PARENTID", .field_type = 'C', .is_null = false, .display_value = "main_menu"}
            }
        },
        {
            .record_index = 4U,
            .deleted = false,
            .values = {
                {.field_name = "COMMENT", .field_type = 'M', .is_null = false, .display_value = "No display fields"}
            }
        },
        {
            .record_index = 5U,
            .deleted = true,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Obsolete"},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "OLD"}
            }
        }
    };

    const auto objects = copperfin::studio::build_object_snapshot(document);
    expect(objects.size() == 4U, "#668: menu snapshot should include parsed menu records");
    if (objects.size() >= 1U) {
        expect(objects[0].menu_prompt == "Customer", "#668: menu snapshots should expose PROMPT metadata");
        expect(objects[0].menu_prompt_field_index == 0U, "#669: menu PROMPT metadata should retain DBF field provenance");
        expect(objects[0].menu_prompt_memo_block_number == 201U, "#718: menu PROMPT metadata should retain source memo block provenance");
        expect(objects[0].menu_level_name == "MAIN", "#668: menu snapshots should expose LEVELNAME metadata");
        expect(objects[0].menu_level_name_field_index == 1U, "#669: menu LEVELNAME metadata should retain DBF field provenance");
        expect(objects[0].menu_level_name_memo_block_number == 0U, "#718: non-memo menu LEVELNAME metadata should expose memo block zero");
        expect(objects[0].menu_command == "DO FORM customer", "#668: menu snapshots should expose COMMAND metadata");
        expect(objects[0].menu_command_field_index == 2U, "#669: menu COMMAND metadata should retain DBF field provenance");
        expect(objects[0].menu_command_memo_block_number == 203U, "#718: menu COMMAND metadata should retain source memo block provenance");
        expect(objects[0].menu_message == "Open customer maintenance", "#668: menu snapshots should expose MESSAGE metadata");
        expect(objects[0].menu_message_field_index == 3U, "#669: menu MESSAGE metadata should retain DBF field provenance");
        expect(objects[0].menu_message_memo_block_number == 204U, "#718: menu MESSAGE metadata should retain source memo block provenance");
        expect(objects[0].title == "Customer", "#668: menu prompt should continue to drive friendly title fallback");
        expect(objects[0].title_field_index == 0U, "#673: menu title metadata should retain selected PROMPT provenance");
        expect(objects[0].subtitle == "MAIN", "#668: menu level name should continue to drive friendly subtitle fallback");
        expect(objects[0].subtitle_field_index == 1U, "#673: menu subtitle metadata should retain selected LEVELNAME provenance");
        expect(objects[0].objtype_code == 3, "#668: menu snapshots should retain raw OBJTYPE metadata");
        expect(objects[0].objtype_memo_block_number == 205U, "#724: menu OBJTYPE metadata should retain source memo block provenance");
        expect(objects[0].objcode_code == 7, "#668: menu snapshots should retain raw OBJCODE metadata");
        expect(objects[0].objcode_memo_block_number == 206U, "#724: menu OBJCODE metadata should retain source memo block provenance");
    }
    if (objects.size() >= 2U) {
        expect(objects[1].menu_prompt == "Tools", "#668: menu snapshots should expose PROMPT metadata when it is not field zero");
        expect(objects[1].menu_prompt_field_index == 1U, "#670: present menu fields should keep their actual DBF ordinal");
        expect(objects[1].object_name == "tools_menu", "#672: object name metadata should fall back to NAME");
        expect(objects[1].object_name_field_index == 3U, "#672: object name fallback should retain selected NAME field provenance");
        expect(objects[1].parent_name == "main_menu", "#672: parent metadata should fall back to PARENTID");
        expect(objects[1].parent_name_field_index == 4U, "#672: parent fallback should retain selected PARENTID field provenance");
        expect(objects[1].title == "Tools", "#673: menu title metadata should preserve existing PROMPT priority");
        expect(objects[1].title_field_index == 1U, "#673: menu title fallback should retain selected PROMPT provenance");
        expect(objects[1].subtitle == "TOOLS", "#673: menu subtitle metadata should preserve existing LEVELNAME priority");
        expect(objects[1].subtitle_field_index == 0U, "#673: menu subtitle fallback should retain selected LEVELNAME provenance");
        expect(objects[1].unique_id_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing UNIQUEID provenance should use the object missing-field sentinel");
        expect(objects[1].class_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing CLASS provenance should use the object missing-field sentinel");
        expect(objects[1].baseclass_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing BASECLASS provenance should use the object missing-field sentinel");
        expect(objects[1].menu_message.empty(), "#670: missing menu MESSAGE values should remain empty");
        expect(objects[1].menu_message_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#670: missing menu MESSAGE provenance should not masquerade as field zero");
        expect(objects[1].menu_message_memo_block_number == 0U,
            "#718: missing menu MESSAGE metadata should expose memo block zero");
        expect(objects[1].objtype_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing OBJTYPE provenance should use the object missing-field sentinel");
        expect(objects[1].objtype_memo_block_number == 0U,
            "#724: missing OBJTYPE metadata should expose memo block zero");
        expect(objects[1].objcode_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing OBJCODE provenance should use the object missing-field sentinel");
        expect(objects[1].objcode_memo_block_number == 0U,
            "#724: missing OBJCODE metadata should expose memo block zero");
        expect(objects[1].platform_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing PLATFORM provenance should use the object missing-field sentinel");
        expect(objects[1].platform_memo_block_number == 0U,
            "#724: missing PLATFORM metadata should expose memo block zero");
    }
    if (objects.size() >= 3U) {
        expect(objects[2].title == "Record 4", "#673: snapshots without display fields should keep synthetic title fallback");
        expect(objects[2].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#673: synthesized titles should use the object missing-field sentinel");
        expect(objects[2].subtitle.empty(), "#673: snapshots without subtitle fields should keep empty subtitle fallback");
        expect(objects[2].subtitle_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#673: missing subtitles should use the object missing-field sentinel");
    }
    if (objects.size() >= 4U) {
        expect(objects[3].deleted, "#688: deleted menu records should stay visible on object snapshots");
        expect(objects[3].title == "Obsolete", "#688: deleted menu records should keep normalized title metadata");
        expect(objects[3].title_field_index == 0U, "#688: deleted menu records should keep title provenance");
    }
}

void test_open_document_preserves_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "missing_sidecar.scx";
    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .launched_from_visual_studio = false,
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for readable assets that carry validation findings");
    expect(
        result.document.inspection.has_validation_issues(),
        "Studio documents should retain validation findings from asset inspection");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "memo.sidecar_missing";
            }),
        "Studio documents should expose the missing-sidecar validation finding");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_memo_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_memo_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "payload_truncated.scx";
    const fs::path sidecar_path = temp_dir / "payload_truncated.sct";

    {
        std::vector<std::uint8_t> table_bytes(115U, 0U);
        table_bytes[0] = 0x30U;
        table_bytes[1] = 126U;
        table_bytes[2] = 4U;
        table_bytes[3] = 11U;
        table_bytes[4] = 0x01U;
        table_bytes[8] = 97U;
        table_bytes[10] = 18U;
        table_bytes[11] = 0U;
        table_bytes[28] = 0x00U;
        table_bytes[29] = 0x03U;
        table_bytes[32] = 'O';
        table_bytes[33] = 'B';
        table_bytes[34] = 'J';
        table_bytes[35] = 'N';
        table_bytes[36] = 'A';
        table_bytes[37] = 'M';
        table_bytes[38] = 'E';
        table_bytes[43] = 'C';
        table_bytes[44] = 1U;
        table_bytes[48] = 12U;
        table_bytes[64] = 'P';
        table_bytes[65] = 'R';
        table_bytes[66] = 'O';
        table_bytes[67] = 'P';
        table_bytes[68] = 'E';
        table_bytes[69] = 'R';
        table_bytes[70] = 'T';
        table_bytes[71] = 'I';
        table_bytes[72] = 'E';
        table_bytes[73] = 'S';
        table_bytes[75] = 'M';
        table_bytes[76] = 13U;
        table_bytes[80] = 4U;
        table_bytes[96] = 0x0DU;
        table_bytes[97] = 0x20U;
        table_bytes[98] = 't';
        table_bytes[99] = 'x';
        table_bytes[100] = 't';
        table_bytes[101] = 'T';
        table_bytes[102] = 'i';
        table_bytes[103] = 't';
        table_bytes[104] = 'l';
        table_bytes[105] = 'e';
        table_bytes[110] = 0x01U;
        table_bytes[114] = 0x1AU;

        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        memo_bytes[3] = 2U;
        memo_bytes[6] = 0x02U;
        memo_bytes[7] = 0x00U;
        memo_bytes[512U + 3U] = 1U;
        memo_bytes[512U + 4U] = 0x00U;
        memo_bytes[512U + 5U] = 0x00U;
        memo_bytes[512U + 6U] = 0x03U;
        memo_bytes[512U + 7U] = 0x84U;
        std::ofstream output(sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for forms with truncated memo payloads");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "memo.payload_truncated";
            }),
        "Studio documents should preserve memo payload validation findings");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_dbf_descriptor_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_descriptor_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "bad_fields.scx";
    {
        std::vector<std::uint8_t> bytes(129U, 0U);
        bytes[0] = 0x30U;
        bytes[1] = 126U;
        bytes[2] = 4U;
        bytes[3] = 11U;
        bytes[4] = 0x01U;
        bytes[8] = 97U;
        bytes[10] = 17U;
        bytes[11] = 0U;
        bytes[28] = 0x00U;
        bytes[29] = 0x03U;
        bytes[32] = '1';
        bytes[33] = '2';
        bytes[34] = '3';
        bytes[35] = 'B';
        bytes[36] = 'A';
        bytes[37] = 'D';
        bytes[38] = 'N';
        bytes[39] = 'A';
        bytes[40] = 'M';
        bytes[41] = 'E';
        bytes[43] = 'C';
        bytes[44] = 1U;
        bytes[48] = 8U;
        bytes[64] = '1';
        bytes[65] = '2';
        bytes[66] = '3';
        bytes[67] = 'B';
        bytes[68] = 'A';
        bytes[69] = 'D';
        bytes[70] = 'N';
        bytes[71] = 'A';
        bytes[72] = 'M';
        bytes[73] = 'E';
        bytes[75] = 'C';
        bytes[76] = 9U;
        bytes[80] = 8U;
        bytes[96] = 0x0DU;
        bytes[97] = 0x20U;
        bytes[128] = 0x1AU;

        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for assets with DBF descriptor validation findings");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "dbf.field_name_duplicate";
            }),
        "Studio documents should preserve DBF descriptor validation findings");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_includes_prg_static_diagnostics() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_prg_diagnostics";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path program_path = temp_dir / "flagged.prg";
    {
        std::ofstream output(program_path, std::ios::binary);
        output << "DO WHILE .T.\n";
        output << "x = 1\n";
        output << "ENDDO\n";
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = program_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should succeed for a PRG file");
    expect(result.document.kind == copperfin::studio::StudioAssetKind::program, "PRG should map to a program document");
    expect(!result.document.static_diagnostics.empty(), "Studio documents should include PRG static diagnostics");
    expect(
        std::any_of(
            result.document.static_diagnostics.begin(),
            result.document.static_diagnostics.end(),
            [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
                return diagnostic.code == "PRG1001";
            }),
        "Studio documents should surface analyzer diagnostics for PRG files");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_studio_host

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/sidecar_path.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_environment_support.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#define _getpid getpid
#endif
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::vector<std::uint8_t> read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

bool has_exact_entry(
    const std::filesystem::path& directory,
    const std::string& filename) {
    std::error_code ignored;
    for (const auto& entry : std::filesystem::directory_iterator(directory, ignored)) {
        if (ignored) {
            return false;
        }
        if (entry.path().filename().string() == filename) {
            return true;
        }
    }
    return false;
}

void write_text_file(const std::filesystem::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
}

struct AssetFamilyCase {
    const char* primary_extension;
    const char* sidecar_extension;
    const char* uppercase_sidecar_extension;
    copperfin::studio::StudioAssetKind studio_kind;
};

void test_six_family_sidecar_resolution_contract() {
    namespace fs = std::filesystem;
    const copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    const auto catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "en-US");
    const fs::path temp_root = fs::temp_directory_path() /
        ("copperfin_vfp_sidecar_path_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::array<AssetFamilyCase, 6U> families{{
        {"pjx", "pjt", "PJT", copperfin::studio::StudioAssetKind::project},
        {"scx", "sct", "SCT", copperfin::studio::StudioAssetKind::form},
        {"vcx", "vct", "VCT", copperfin::studio::StudioAssetKind::class_library},
        {"frx", "frt", "FRT", copperfin::studio::StudioAssetKind::report},
        {"lbx", "lbt", "LBT", copperfin::studio::StudioAssetKind::label},
        {"mnx", "mnt", "MNT", copperfin::studio::StudioAssetKind::menu}
    }};

    for (const auto& family : families) {
        const std::string family_name = family.primary_extension;
        const fs::path directory = temp_root / family_name;
        fs::create_directories(directory);
        const fs::path primary_path = directory / ("sample." + family_name);
        const fs::path exact_sidecar_path =
            directory / ("sample." + std::string(family.sidecar_extension));

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            primary_path.string(),
            {{.name = "OBJNAME", .type = 'M', .length = 4U}},
            {{"sample-object"}});
        expect(create_result.ok, family_name + ": fixture creation should succeed");
        if (!create_result.ok) {
            continue;
        }

        const fs::path folded_sidecar_one =
            directory / ("Sample." + std::string(family.uppercase_sidecar_extension));
        const fs::path folded_sidecar_two =
            directory / ("SAMPLE." + std::string(family.sidecar_extension));
        fs::rename(exact_sidecar_path, folded_sidecar_one, ignored);
        ignored.clear();
        fs::copy_file(
            folded_sidecar_one,
            folded_sidecar_two,
            fs::copy_options::overwrite_existing,
            ignored);

        if (!has_exact_entry(directory, folded_sidecar_one.filename().string()) ||
            !has_exact_entry(directory, folded_sidecar_two.filename().string())) {
            fs::remove_all(directory, ignored);
            continue;
        }

        const std::vector<std::uint8_t> primary_before = read_file(primary_path);
        const std::vector<std::uint8_t> sidecar_one_before = read_file(folded_sidecar_one);
        const std::vector<std::uint8_t> sidecar_two_before = read_file(folded_sidecar_two);
        const auto resolution =
            copperfin::vfp::resolve_vfp_memo_sidecar_path(primary_path);
        expect(resolution.ambiguous && !resolution.path.has_value() &&
                   resolution.requested_path == exact_sidecar_path,
               family_name + ": multiple folded matches should be ambiguous without selecting a file");
        expect(copperfin::studio::infer_sidecar_path(
                   primary_path.string(), family.studio_kind).empty(),
               family_name + ": compatibility inference should fail closed on ambiguity");

        const std::string shared_error = catalog.translate(
            "Vfp.Sidecar.Error.AmbiguousPath",
            {{"path", exact_sidecar_path.string()}});
        const auto open_result = copperfin::studio::open_document({.path = primary_path.string()});
        expect(!open_result.ok && open_result.error == catalog.translate(
                   "Studio.DocumentOpen.Error.SidecarPathAmbiguous",
                   {{"path", exact_sidecar_path.string()}}),
               family_name + ": ordinary Studio open should reject ambiguous sidecars");

        const auto parse_result =
            copperfin::vfp::parse_dbf_table_from_file(primary_path.string(), 1U);
        expect(!parse_result.ok && parse_result.error == shared_error,
               family_name + ": DBF parse should reject ambiguous sidecars");

        const auto write_result = copperfin::vfp::set_record_deleted_flag(
            primary_path.string(), 0U, true);
        expect(!write_result.ok && write_result.error == shared_error,
               family_name + ": DBF write should reject ambiguous sidecars");

        const auto edit_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = primary_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .deleted = true
        });
        expect(!edit_result.ok && edit_result.error == shared_error,
               family_name + ": visual editor mutation should reject ambiguous sidecars");

        const auto inspection = copperfin::vfp::inspect_asset(primary_path.string());
        expect(!inspection.ok && inspection.error == shared_error &&
                   inspection.validation_issues.empty(),
               family_name + ": asset inspection should fail before reading ambiguous assets");

        expect(read_file(primary_path) == primary_before &&
                   read_file(folded_sidecar_one) == sidecar_one_before &&
                   read_file(folded_sidecar_two) == sidecar_two_before,
               family_name + ": rejected parse/edit/write paths should preserve every byte");

        fs::remove(folded_sidecar_two, ignored);
        const auto unique_resolution =
            copperfin::vfp::resolve_vfp_memo_sidecar_path(primary_path);
        expect(!unique_resolution.ambiguous &&
                   unique_resolution.path == folded_sidecar_one,
               family_name + ": one folded match should preserve actual disk spelling");
        const auto unique_open = copperfin::studio::open_document({.path = primary_path.string()});
        expect(unique_open.ok &&
                   unique_open.document.sidecar_path == folded_sidecar_one.string() &&
                   unique_open.document.has_sidecar,
               family_name + ": Studio document data should preserve unique folded spelling");

        fs::copy_file(
            folded_sidecar_one,
            exact_sidecar_path,
            fs::copy_options::overwrite_existing,
            ignored);
        const auto exact_resolution =
            copperfin::vfp::resolve_vfp_memo_sidecar_path(primary_path);
        expect(!exact_resolution.ambiguous && exact_resolution.path == exact_sidecar_path,
               family_name + ": exact spelling should win over folded siblings");
        const auto exact_open = copperfin::studio::open_document({.path = primary_path.string()});
        expect(exact_open.ok && exact_open.document.sidecar_path == exact_sidecar_path.string(),
               family_name + ": Studio should publish the exact sidecar spelling");
    }

    fs::remove_all(temp_root, ignored);
}

void test_non_memo_dbf_ignores_stale_fpt_ambiguity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        ("copperfin_non_memo_dbf_sidecar_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "sample.dbf";
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 12U}},
        {{"before"}});
    expect(create_result.ok, "non-memo DBF fixture creation should succeed");
    write_text_file(temp_root / "Sample.FPT", "stale-one");
    write_text_file(temp_root / "SAMPLE.fpt", "stale-two");

    const auto sidecar_resolution =
        copperfin::vfp::resolve_vfp_memo_sidecar_path(table_path);
    if (has_exact_entry(temp_root, "Sample.FPT") &&
        has_exact_entry(temp_root, "SAMPLE.fpt")) {
        expect(sidecar_resolution.ambiguous,
               "non-memo DBF fixture should contain ambiguous stale FPT files");

        const auto parse_result =
            copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(parse_result.ok,
               "non-memo DBF parse should not depend on stale FPT files");

        const auto field_result = copperfin::vfp::replace_record_field_value(
            table_path.string(), 0U, "NAME", "after");
        expect(field_result.ok,
               "non-memo DBF field writes should not depend on stale FPT files");

        const auto delete_result = copperfin::vfp::set_record_deleted_flag(
            table_path.string(), 0U, true);
        expect(delete_result.ok,
               "table-only DBF writes should not depend on stale FPT files");

        const auto inspection = copperfin::vfp::inspect_asset(table_path.string());
        expect(inspection.ok && inspection.error.empty(),
               "non-memo DBF inspection should ignore stale FPT ambiguity");
        const auto open_result = copperfin::studio::open_document({.path = table_path.string()});
        expect(open_result.ok && open_result.document.sidecar_path.empty() &&
                   !open_result.document.has_sidecar,
               "non-memo Studio DBF opens should ignore stale FPT ambiguity");
    }

    fs::remove_all(temp_root, ignored);
}

void test_memo_backed_dbf_rejects_fpt_ambiguity_for_every_write_shape() {
    namespace fs = std::filesystem;
    const copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    const auto catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "en-US");
    const fs::path temp_root = fs::temp_directory_path() /
        ("copperfin_memo_dbf_sidecar_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "sample.dbf";
    const fs::path exact_sidecar_path = temp_root / "sample.fpt";
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {
            {.name = "NAME", .type = 'C', .length = 12U},
            {.name = "NOTES", .type = 'M', .length = 4U}
        },
        {{"before", "memo-value"}});
    expect(create_result.ok, "memo-backed DBF fixture creation should succeed");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path folded_sidecar_one = temp_root / "Sample.FPT";
    const fs::path folded_sidecar_two = temp_root / "SAMPLE.fpt";
    fs::rename(exact_sidecar_path, folded_sidecar_one, ignored);
    ignored.clear();
    fs::copy_file(
        folded_sidecar_one,
        folded_sidecar_two,
        fs::copy_options::overwrite_existing,
        ignored);
    if (has_exact_entry(temp_root, "Sample.FPT") &&
        has_exact_entry(temp_root, "SAMPLE.fpt")) {
        const std::vector<std::uint8_t> table_before = read_file(table_path);
        const std::vector<std::uint8_t> sidecar_one_before = read_file(folded_sidecar_one);
        const std::vector<std::uint8_t> sidecar_two_before = read_file(folded_sidecar_two);
        const std::string expected_error = catalog.translate(
            "Vfp.Sidecar.Error.AmbiguousPath",
            {{"path", exact_sidecar_path.string()}});

        const auto parse_result =
            copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        const auto field_result = copperfin::vfp::replace_record_field_value(
            table_path.string(), 0U, "NAME", "after");
        const auto append_result =
            copperfin::vfp::append_blank_record_to_file(table_path.string());
        const auto delete_result =
            copperfin::vfp::set_record_deleted_flag(table_path.string(), 0U, true);
        const auto truncate_result =
            copperfin::vfp::truncate_dbf_table_file(table_path.string(), 0U);
        const auto pack_result =
            copperfin::vfp::pack_dbf_table_file(table_path.string());
        const auto pack_memo_result =
            copperfin::vfp::pack_dbf_memo_file(table_path.string());
        const auto zap_result = copperfin::vfp::zap_dbf_table_file(table_path.string());
        const auto inspection = copperfin::vfp::inspect_asset(table_path.string());
        const auto open_result = copperfin::studio::open_document({.path = table_path.string()});

        expect(!parse_result.ok && parse_result.error == expected_error,
               "memo-backed DBF parse should reject ambiguous FPT files");
        for (const auto* result : {
                 &field_result,
                 &append_result,
                 &delete_result,
                 &truncate_result,
                 &pack_result,
                 &pack_memo_result,
                 &zap_result}) {
            expect(!result->ok && result->error == expected_error,
                   "every memo-backed DBF write shape should reject ambiguous FPT files");
        }
        expect(!inspection.ok && inspection.error == expected_error,
               "memo-backed DBF inspection should reject ambiguous FPT files");
        expect(!open_result.ok && open_result.error == expected_error,
               "memo-backed Studio DBF opens should reject ambiguous FPT files");
        expect(read_file(table_path) == table_before &&
                   read_file(folded_sidecar_one) == sidecar_one_before &&
                   read_file(folded_sidecar_two) == sidecar_two_before,
               "memo-backed DBF ambiguity rejection should preserve every byte");
    }

    fs::remove_all(temp_root, ignored);
}

void test_unique_casefold_preserves_actual_spelling() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        ("copperfin_sidecar_spelling_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path primary_path = temp_root / "sample.scx";
    const fs::path inferred_sidecar_path = temp_root / "sample.sct";
    const fs::path temporary_sidecar_path = temp_root / "sidecar.rename";
    const fs::path actual_sidecar_path = temp_root / "Sample.SCT";
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        primary_path.string(),
        {{.name = "OBJNAME", .type = 'M', .length = 4U}},
        {{"sample-object"}});
    expect(create_result.ok, "casefold spelling fixture creation should succeed");
    fs::rename(inferred_sidecar_path, temporary_sidecar_path, ignored);
    ignored.clear();
    fs::rename(temporary_sidecar_path, actual_sidecar_path, ignored);

    expect(has_exact_entry(temp_root, actual_sidecar_path.filename().string()),
           "casefold spelling fixture should retain its mixed-case directory entry");
    const auto resolution =
        copperfin::vfp::resolve_vfp_memo_sidecar_path(primary_path);
    expect(!resolution.ambiguous && resolution.path == actual_sidecar_path,
           "unique casefold resolution should return actual disk spelling");

    const auto open_result = copperfin::studio::open_document({.path = primary_path.string()});
    expect(open_result.ok &&
               open_result.document.sidecar_path == actual_sidecar_path.string(),
           "Studio document and host JSON source data should retain actual sidecar spelling");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_six_family_sidecar_resolution_contract();
    test_non_memo_dbf_ignores_stale_fpt_ambiguity();
    test_memo_backed_dbf_rejects_fpt_ambiguity_for_every_write_shape();
    test_unique_casefold_preserves_actual_spelling();
    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    std::cout << "All VFP sidecar path tests passed\n";
    return 0;
}

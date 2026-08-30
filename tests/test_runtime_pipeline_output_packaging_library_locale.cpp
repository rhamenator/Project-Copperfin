// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_pipeline_output_packaging_support.h"

#include <locale>

namespace cf_test_runtime_pipeline {

namespace {
class manifest_grouped_numpunct final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\1"; }
};

class manifest_global_locale_guard final {
public:
    explicit manifest_global_locale_guard(const std::locale& replacement)
        : previous_(std::locale::global(replacement)) {}

    ~manifest_global_locale_guard() { std::locale::global(previous_); }

    manifest_global_locale_guard(const manifest_global_locale_guard&) = delete;
    manifest_global_locale_guard& operator=(const manifest_global_locale_guard&) = delete;

private:
    std::locale previous_;
};
}  // namespace

void test_library_manifest_source_location_escaping() {
    const std::locale grouping_locale(std::locale::classic(), new manifest_grouped_numpunct());
    manifest_global_locale_guard locale_guard(grouping_locale);
    const std::string source_path =
        "/tmp/copperfin|source\\library\npart-" + std::string("\xC3\xA9") + ".prg";
    const copperfin::runtime::SourceLocation location{source_path, 1234U};
    const std::string encoded = copperfin::runtime::runtime_pipeline_detail::build_manifest_source_location(location);
    const std::size_t separator = encoded.rfind('|');

    expect(separator != std::string::npos,
           "library source-location encoding should retain its final line separator");
    if (separator == std::string::npos) {
        return;
    }

    expect(encoded.substr(0U, separator).find("\\|") != std::string::npos,
           "library source-location encoding should escape embedded pipe delimiters");
    expect(encoded.substr(0U, separator).find("\\\\") != std::string::npos,
           "library source-location encoding should escape backslashes");
    expect(encoded.substr(0U, separator).find("\\n") != std::string::npos,
           "library source-location encoding should escape newlines");
    expect(decode_manifest_value(encoded.substr(0U, separator)) == source_path,
           "library source-location encoding should round-trip path bytes");
    expect(encoded.substr(separator + 1U) == "1234",
           "#4846: library source-location encoding should preserve invariant source lines");
}

void test_library_api_manifest_arities_ignore_grouping_locale() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_runtime_pipeline_library_arity_locale";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "wide.prg";
    write_text(
        source_path,
        "FUNCTION WideCall\n"
        "LPARAMETERS p01, p02, p03, p04, p05, p06, p07, p08, p09, p10, p11, p12\n"
        "RETURN 1\n"
        "ENDFUNC\n");

    copperfin::runtime::RuntimePackagePlan plan;
    plan.output_kind = copperfin::runtime::BuildOutputKind::dll;
    plan.launcher_output_path = (temp_root / "Wide.dll").string();
    plan.exported_symbols = {"WideCall"};
    plan.assets = {{
        .record_index = 1U,
        .source_path = source_path.string(),
        .staged_path = {},
        .relative_path = "wide.prg",
        .type_title = "Program",
        .excluded = false,
        .exists = true,
        .required_for_runtime = true,
        .package_writable = false,
        .copied = true,
        .sha256 = {},
        .source_resolution_error = {}}};

    const std::locale grouping_locale(std::locale::classic(), new manifest_grouped_numpunct());
    manifest_global_locale_guard locale_guard(grouping_locale);
    const std::string library_manifest =
        copperfin::runtime::runtime_pipeline_detail::build_library_api_manifest_source(plan);
    const std::string fll_manifest =
        copperfin::runtime::runtime_pipeline_detail::build_fll_api_manifest_source(plan);

    expect(library_manifest.find("function_arity=WideCall|12") != std::string::npos,
           "#4868: DLL/OCX API function arity should remain invariant under digit grouping");
    expect(fll_manifest.find("function_arity=WideCall|12") != std::string::npos,
           "#4868: FLL API function arity should remain invariant under digit grouping");
    expect(library_manifest.find("function_arity=WideCall|1.2") == std::string::npos &&
               fll_manifest.find("function_arity=WideCall|1.2") == std::string::npos,
           "#4868: generated API manifests should not group machine-readable arity digits");
    expect(library_manifest.find("function_parameters=WideCall|p01|p02|p03|p04|p05|p06|p07|p08|p09|p10|p11|p12") !=
               std::string::npos &&
               fll_manifest.find("function_parameters=WideCall|p01|p02|p03|p04|p05|p06|p07|p08|p09|p10|p11|p12") !=
                   std::string::npos,
           "#4868: invariant arity formatting should preserve parsed parameter order and names");

    fs::remove_all(temp_root, ignored);
}

void test_library_native_wrapper_numeric_literals_ignore_grouping_locale() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_runtime_pipeline_library_wrapper_numeric_locale";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    std::string source_text;
    for (std::size_t line = 1U; line < 1234U; ++line) {
        source_text += "* padding\n";
    }
    source_text +=
        "FUNCTION WideCall\n"
        "LPARAMETERS p01, p02, p03, p04, p05, p06, p07, p08, p09, p10, p11, p12\n"
        "RETURN 1\n"
        "ENDFUNC\n";

    const fs::path source_path = temp_root / "wide.prg";
    write_text(source_path, source_text);

    copperfin::runtime::RuntimePackagePlan plan;
    plan.output_kind = copperfin::runtime::BuildOutputKind::dll;
    plan.launcher_output_path = (temp_root / "Wide.dll").string();
    plan.exported_symbols = {"WideCall"};
    plan.assets = {{
        .record_index = 1U,
        .source_path = source_path.string(),
        .staged_path = {},
        .relative_path = "wide.prg",
        .type_title = "Program",
        .excluded = false,
        .exists = true,
        .required_for_runtime = true,
        .package_writable = false,
        .copied = true,
        .sha256 = {},
        .source_resolution_error = {}}};

    const std::locale grouping_locale(std::locale::classic(), new manifest_grouped_numpunct());
    manifest_global_locale_guard locale_guard(grouping_locale);
    const std::string dll_wrapper =
        copperfin::runtime::runtime_pipeline_detail::build_native_wrapper_source(plan);
    plan.output_kind = copperfin::runtime::BuildOutputKind::fll;
    plan.launcher_output_path = (temp_root / "Wide.fll").string();
    const std::string fll_wrapper =
        copperfin::runtime::runtime_pipeline_detail::build_native_wrapper_source(plan);

    const std::string parameter_names =
        "p01\\|p02\\|p03\\|p04\\|p05\\|p06\\|p07\\|p08\\|p09\\|p10\\|p11\\|p12";
    expect(dll_wrapper.find(", 1234U") != std::string::npos &&
               fll_wrapper.find(", 1234U") != std::string::npos,
           "#4869: DLL/OCX and FLL wrapper source lines should remain invariant C++ literals");
    expect(dll_wrapper.find(", 12U, reinterpret_cast<void*>(&WideCall)") != std::string::npos &&
               fll_wrapper.find(", 12U, reinterpret_cast<void*>(&WideCall)") != std::string::npos,
           "#4869: DLL/OCX and FLL wrapper parameter counts should remain invariant C++ literals");
    expect(dll_wrapper.find(parameter_names) != std::string::npos &&
               fll_wrapper.find(parameter_names) != std::string::npos,
           "#4869: invariant wrapper numeric literals should preserve parameter order and names");
    expect(dll_wrapper.find("1.2.3.4U") == std::string::npos &&
               fll_wrapper.find("1.2.3.4U") == std::string::npos &&
               dll_wrapper.find("1.2U") == std::string::npos &&
               fll_wrapper.find("1.2U") == std::string::npos,
           "#4869: generated wrapper numeric literals should not inherit digit grouping");

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_runtime_pipeline

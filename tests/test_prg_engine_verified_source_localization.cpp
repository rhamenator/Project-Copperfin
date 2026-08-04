// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "prg_engine_test_support.h"
#include "test_locale_catalog_environment_support.h"

#include <filesystem>
#include <exception>
#include <string>

namespace {

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::ScopedTestLocaleCatalogDirectory;
using copperfin::test_support::expect;
using copperfin::test_support::make_runtime_session_options;
using copperfin::test_support::write_text;

std::string run_missing_source_case(const std::string& locale, bool include_source)
{
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / ("copperfin_verified_source_" + locale);
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    const fs::path startup_path = root / "main.prg";
    auto options = make_runtime_session_options(startup_path, root);
    options.require_source_text_overrides = true;
    if (include_source) {
        options.startup_source_text = "#INCLUDE 'missing.h'\nRETURN\n";
    }

    ScopedEnvironmentValue locale_environment("COPPERFIN_LOCALE");
    ScopedTestLocaleCatalogDirectory catalog_directory;
    locale_environment.set(locale);
    std::string message;
    try {
        const auto state = copperfin::runtime::PrgRuntimeSession::create(options).run(
            copperfin::runtime::DebugResumeAction::continue_run);
        message = state.message;
    } catch (const std::exception& error) {
        message = error.what();
    }
    fs::remove_all(root, ignored);
    expect(!message.empty(), locale + " verified source failure should expose an error message");
    return message;
}

}  // namespace

void test_verified_source_errors_are_localized()
{
    const std::string spanish_source = run_missing_source_case("es-419", false);
    expect(spanish_source == "La fuente verificada del paquete no esta disponible: " +
               (std::filesystem::temp_directory_path() / "copperfin_verified_source_es-419" / "main.prg").lexically_normal().string(),
           "strict missing main source should use the Spanish catalog while preserving its path");
    expect(spanish_source.find("verified source text unavailable") == std::string::npos,
           "strict missing main source should not leak the raw English implementation string");

    const std::string portuguese_include = run_missing_source_case("pt-BR", true);
    const auto expected_include_path = (std::filesystem::temp_directory_path() /
                                        "copperfin_verified_source_pt-BR" / "missing.h")
                                           .lexically_normal()
                                           .string();
    expect(portuguese_include == "A origem INCLUDE verificada do pacote nao esta disponivel: " +
               expected_include_path,
           "strict missing include source should use the Portuguese catalog while preserving its path");
    expect(portuguese_include.find("verified include source unavailable") == std::string::npos,
           "strict missing include source should not leak the raw English implementation string");
}

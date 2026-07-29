// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_prg_engine_data_io_support.h"

#include "copperfin/platform/invariant_numeric.h"

namespace cf_test_prg_engine_data_io {
void test_save_to_writes_variables_to_file() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_to";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "session_state.mem";
    const fs::path main_path = temp_root / "save_to_test.prg";
    write_text(
        main_path,
        "x = 'hello'\n"
        "nVal = 42\n"
        "nSavePathCalls = 0\n"
        "SAVE TO save_path('" + mem_path.string() + "')\n"
        "RETURN\n"
        "FUNCTION save_path\n"
        "LPARAMETERS value\n"
        "nSavePathCalls = nSavePathCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE TO script should complete");
    expect(fs::exists(mem_path), "SAVE TO should create a .mem file");
    const auto save_path_calls = state.globals.find("nsavepathcalls");
    expect(save_path_calls != state.globals.end(), "SAVE TO should preserve the path resolver call counter");
    if (save_path_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(save_path_calls->second) == "1",
               "SAVE TO should evaluate the path UDF exactly once");
    }

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("x=C:hello") != std::string::npos,
            "SAVE TO should persist character variable x");
        expect(contents.find("nval=N:42") != std::string::npos,
            "SAVE TO should persist numeric variable nVal");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_loads_variables_from_file() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_from";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "roundtrip.mem";

    const fs::path save_path = temp_root / "save_source.prg";
    write_text(
        save_path,
        "x = 'hello'\n"
        "nVal = 42\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "RETURN\n");

    {
        copperfin::runtime::PrgRuntimeSession save_session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(save_path.string(), temp_root.string(), false));
        const auto save_state = save_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(save_state.completed, "SAVE TO setup script should complete before RESTORE FROM test");
    }

    const fs::path restore_path = temp_root / "restore_target.prg";
    write_text(
        restore_path,
        "nRestorePathCalls = 0\n"
        "RESTORE FROM restore_path('" + mem_path.string() + "') ADDITIVE\n"
        "restored_x = x\n"
        "restored_n = nVal\n"
        "RETURN\n"
        "FUNCTION restore_path\n"
        "LPARAMETERS value\n"
        "nRestorePathCalls = nRestorePathCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession restore_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(restore_path.string(), temp_root.string(), false));

    const auto restore_state = restore_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(restore_state.completed, "RESTORE FROM script should complete");
    const auto restore_path_calls = restore_state.globals.find("nrestorepathcalls");
    expect(restore_path_calls != restore_state.globals.end(), "RESTORE FROM should preserve the path resolver call counter");
    if (restore_path_calls != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restore_path_calls->second) == "1",
               "RESTORE FROM should evaluate the path UDF exactly once");
    }

    const auto restored_x = restore_state.globals.find("restored_x");
    const auto restored_n = restore_state.globals.find("restored_n");
    expect(restored_x != restore_state.globals.end(), "RESTORE FROM should restore character variable x");
    expect(restored_n != restore_state.globals.end(), "RESTORE FROM should restore numeric variable nVal");
    if (restored_x != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_x->second) == "hello",
            "RESTORE FROM should keep x value");
    }
    if (restored_n != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_n->second) == "42",
            "RESTORE FROM should keep nVal value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_restore_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_restore_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path save_path = temp_root / "save_error.prg";
    write_text(
        save_path,
        "SAVE TO ''\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession save_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(save_path.string(), temp_root.string(), false));
    const auto save_state = save_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!save_state.completed, "#2705: qps-ploc SAVE TO missing-filename script should fail");
    expect(
        save_state.message == copperfin::localization::pseudo_localize("SAVE TO: filename required"),
        "#2705: qps-ploc SAVE TO runtime error should route through the pseudo-localization transform");

    const fs::path restore_path = temp_root / "restore_error.prg";
    write_text(
        restore_path,
        "RESTORE FROM 'missing.mem'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession restore_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(restore_path.string(), temp_root.string(), false));
    const auto restore_state = restore_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!restore_state.completed, "#2705: qps-ploc RESTORE FROM missing-file script should fail");
    expect(
        restore_state.message == copperfin::localization::pseudo_localize("RESTORE FROM: unable to open source file"),
        "#2705: qps-ploc RESTORE FROM runtime error should route through the pseudo-localization transform");

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_additive_merges_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_additive";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "additive.mem";
    const fs::path main_path = temp_root / "restore_additive.prg";
    write_text(
        main_path,
        "existing = 'kept'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "another = 'new'\n"
        "RESTORE FROM '" + mem_path.string() + "' ADDITIVE\n"
        "after_existing = existing\n"
        "after_another = another\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESTORE FROM ADDITIVE script should complete");

    const auto after_existing = state.globals.find("after_existing");
    const auto after_another = state.globals.find("after_another");
    expect(after_existing != state.globals.end(), "RESTORE FROM ADDITIVE should retain saved existing variable");
    expect(after_another != state.globals.end(), "RESTORE FROM ADDITIVE should retain non-file variable another");
    if (after_existing != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_existing->second) == "kept",
            "RESTORE FROM ADDITIVE should preserve saved existing value");
    }
    if (after_another != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_another->second) == "new",
            "RESTORE FROM ADDITIVE should preserve additive local global variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_to_like_pattern_filters_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_like";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "filtered.mem";
    const fs::path main_path = temp_root / "save_like.prg";
    write_text(
        main_path,
        "cName = 'Alice'\n"
        "nAge = 30\n"
        "SAVE TO '" + mem_path.string() + "' ALL LIKE c*\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE TO ALL LIKE script should complete");
    expect(fs::exists(mem_path), "SAVE TO ALL LIKE should create output file");

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cname=C:Alice") != std::string::npos,
            "SAVE TO ALL LIKE should include matching cName variable");
        expect(contents.find("nage=N:30") == std::string::npos,
            "SAVE TO ALL LIKE should exclude non-matching nAge variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_to_except_pattern_filters_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "filtered.mem";
    const fs::path main_path = temp_root / "save_except.prg";
    write_text(
        main_path,
        "cName = 'Alice'\n"
        "nAge = 30\n"
        "SAVE TO '" + mem_path.string() + "' ALL EXCEPT c*\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE TO ALL EXCEPT script should complete");
    expect(fs::exists(mem_path), "SAVE TO ALL EXCEPT should create output file");

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cname=C:Alice") == std::string::npos,
            "SAVE TO ALL EXCEPT should exclude matching cName variable");
        expect(contents.find("nage=N:30") != std::string::npos,
            "SAVE TO ALL EXCEPT should include non-matching nAge variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_restore_auto_mem_extension_without_explicit_extension() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_restore_auto_extension";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path base_path = temp_root / "session_state";
    const fs::path mem_path = temp_root / "session_state.mem";
    const fs::path main_path = temp_root / "save_restore_auto_extension.prg";
    write_text(
        main_path,
        "x = 'auto'\n"
        "SAVE TO '" + base_path.string() + "'\n"
        "x = 'changed'\n"
        "RESTORE FROM '" + base_path.string() + "'\n"
        "restored_x = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE/RESTORE without explicit extension should complete");
    expect(fs::exists(mem_path), "SAVE TO without extension should append .mem");

    const auto restored_x = state.globals.find("restored_x");
    expect(restored_x != state.globals.end(), "RESTORE FROM without extension should load saved variable");
    if (restored_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_x->second) == "auto",
            "RESTORE FROM without extension should read from the auto-appended .mem file");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_restore_round_trips_escaped_string_and_types() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_restore_escaped";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "escaped.mem";
    const fs::path save_path = temp_root / "save_escaped.prg";
    write_text(
        save_path,
        "cEscaped = 'left=right:slash\\' + CHR(10) + 'line2' + CHR(9) + 'tail'\n"
        "lFlag = .T.\n"
        "nAmount = 12.5\n"
        "dStamp = '01/15/2026'\n"
        "PUBLIC eOnly\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "RETURN\n");

    {
        copperfin::runtime::PrgRuntimeSession save_session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(save_path.string(), temp_root.string(), false));
        const auto save_state = save_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(save_state.completed, "SAVE TO escaped setup script should complete");
    }

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cescaped=C:") != std::string::npos,
            "SAVE TO should persist escaped string variable");
        expect(contents.find("left\\=right\\:") != std::string::npos,
            "SAVE TO should escape delimiter-sensitive characters");
        expect(contents.find("slash\\\\") != std::string::npos,
            "SAVE TO should escape literal backslashes");
        expect(contents.find("\\nline2\\t") != std::string::npos,
            "SAVE TO should escape newline and tab control characters");
        expect(contents.find("dstamp=D:01/15/2026") != std::string::npos,
            "SAVE TO should persist recognized date values using type code D");
        expect(contents.find("eonly=L,PUBLIC:false") != std::string::npos,
            "SAVE TO should persist bare PUBLIC values as scoped logical false values");
    }

    const fs::path restore_path = temp_root / "restore_escaped.prg";
    write_text(
        restore_path,
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "restored_s = cEscaped\n"
        "restored_l = lFlag\n"
        "restored_n = nAmount\n"
        "restored_d = dStamp\n"
        "restored_e_type = VARTYPE(eOnly)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession restore_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(restore_path.string(), temp_root.string(), false));

    const auto restore_state = restore_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(restore_state.completed, "RESTORE FROM escaped script should complete");

    const auto restored_s = restore_state.globals.find("restored_s");
    const auto restored_l = restore_state.globals.find("restored_l");
    const auto restored_n = restore_state.globals.find("restored_n");
    const auto restored_d = restore_state.globals.find("restored_d");
    const auto restored_e_type = restore_state.globals.find("restored_e_type");
    const auto restored_public = restore_state.globals.find("eonly");

    expect(restored_s != restore_state.globals.end(), "RESTORE FROM should restore escaped string variable");
    expect(restored_l != restore_state.globals.end(), "RESTORE FROM should restore logical variable");
    expect(restored_n != restore_state.globals.end(), "RESTORE FROM should restore numeric variable");
    expect(restored_d != restore_state.globals.end(), "RESTORE FROM should restore date variable");
    expect(restored_e_type != restore_state.globals.end(), "RESTORE FROM should restore bare PUBLIC variable type");
    expect(restored_public != restore_state.globals.end(), "RESTORE FROM should materialize bare PUBLIC variables");

    if (restored_s != restore_state.globals.end()) {
        const std::string expected = std::string("left=right:slash\\") + "\n" + "line2" + "\t" + "tail";
        expect(copperfin::runtime::format_value(restored_s->second) == expected,
            "RESTORE FROM should unescape delimiter-sensitive and control characters");
    }
    if (restored_l != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_l->second) == "true",
            "RESTORE FROM should preserve logical type fidelity");
    }
    if (restored_n != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_n->second) == "12.5",
            "RESTORE FROM should preserve numeric type fidelity");
    }
    if (restored_d != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_d->second) == "01/15/2026",
            "RESTORE FROM should preserve date values");
    }
    if (restored_e_type != restore_state.globals.end()) {
        const std::string restored_type = copperfin::runtime::format_value(restored_e_type->second);
        expect(restored_type == "L",
            "RESTORE FROM should preserve the logical type of a bare PUBLIC declaration");
    }
    if (restored_public != restore_state.globals.end()) {
        expect(restored_public->second.kind == copperfin::runtime::PrgValueKind::boolean &&
                   !restored_public->second.boolean_value,
               "RESTORE FROM should preserve the false value of a bare PUBLIC declaration");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_rejects_numeric_trailing_garbage() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_numeric_garbage";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "numeric.mem";
    write_text(mem_path, "ngood=N:12.5\nnbad=N:12.5oops\n");

    const fs::path main_path = temp_root / "restore_numeric_garbage.prg";
    write_text(
        main_path,
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "after_good = nGood\n"
        "after_bad = nBad\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESTORE FROM numeric garbage script should complete");

    const auto after_good = state.globals.find("after_good");
    const auto after_bad = state.globals.find("after_bad");
    expect(after_good != state.globals.end(), "RESTORE FROM should parse valid numeric values");
    expect(after_bad != state.globals.end(), "RESTORE FROM should still materialize invalid numeric entries");
    if (after_good != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_good->second) == "12.5",
            "RESTORE FROM should keep valid numeric values");
    }
    if (after_bad != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_bad->second) == "0",
            "RESTORE FROM should reject numerics with trailing garbage and fall back to 0");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_parses_numeric_values_invariantly() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_invariant_numeric";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    expect(!copperfin::platform::try_parse_invariant_double("12,5000").has_value(),
        "invariant numeric parser should reject comma-decimal text");

    const fs::path mem_path = temp_root / "numeric.mem";
    write_text(
        mem_path,
        "nperiod=N:12.5\n"
        "ncomma=N:12,5\n"
        "ycurrency=Y:12.5000\n"
        "ycomma=Y:12,5000\n"
        "ymin=Y:-922337203685477.5808\n"
        "ymax=Y:922337203685477.5807\n"
        "yoverflow=Y:922337203685477.5808\n"
        "yunderflow=Y:-922337203685477.5809\n"
        "yroundmax=Y:922337203685477.58075\n"
        "yroundmin=Y:-922337203685477.58075\n"
        "ysmall=Y:0.00005\n"
        "ytiny=Y:0.000005\n");

    const fs::path main_path = temp_root / "restore_invariant_numeric.prg";
    write_text(
        main_path,
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "period = nPeriod\n"
        "comma = nComma\n"
        "currency = yCurrency\n"
        "currency_comma = yComma\n"
        "currency_min = yMin\n"
        "currency_max = yMax\n"
        "currency_overflow = yOverflow\n"
        "currency_underflow = yUnderflow\n"
        "currency_round_max = yRoundMax\n"
        "currency_round_min = yRoundMin\n"
        "currency_small = ySmall\n"
        "currency_tiny = yTiny\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESTORE FROM invariant numeric script should complete");

    const auto period = state.globals.find("period");
    const auto comma = state.globals.find("comma");
    const auto currency = state.globals.find("currency");
    const auto currency_comma = state.globals.find("currency_comma");
    const auto currency_min = state.globals.find("currency_min");
    const auto currency_max = state.globals.find("currency_max");
    const auto currency_overflow = state.globals.find("currency_overflow");
    const auto currency_underflow = state.globals.find("currency_underflow");
    const auto currency_round_max = state.globals.find("currency_round_max");
    const auto currency_round_min = state.globals.find("currency_round_min");
    const auto currency_small = state.globals.find("currency_small");
    const auto currency_tiny = state.globals.find("currency_tiny");
    expect(period != state.globals.end(), "RESTORE FROM should materialize period-decimal numeric values");
    expect(comma != state.globals.end(), "RESTORE FROM should materialize comma-decimal numeric values");
    expect(currency != state.globals.end(), "RESTORE FROM should materialize currency values");
    expect(currency_comma != state.globals.end(), "RESTORE FROM should materialize comma-decimal currency values");
    expect(currency_min != state.globals.end(), "RESTORE FROM should materialize minimum currency values");
    expect(currency_max != state.globals.end(), "RESTORE FROM should materialize maximum currency values");
    expect(currency_overflow != state.globals.end(), "RESTORE FROM should materialize overflowing currency values");
    expect(currency_underflow != state.globals.end(), "RESTORE FROM should materialize underflowing currency values");
    expect(currency_round_max != state.globals.end(), "RESTORE FROM should materialize maximum rounding values");
    expect(currency_round_min != state.globals.end(), "RESTORE FROM should materialize minimum rounding values");
    expect(currency_small != state.globals.end(), "RESTORE FROM should materialize sub-cent rounding values");
    expect(currency_tiny != state.globals.end(), "RESTORE FROM should materialize tiny currency values");
    if (period != state.globals.end()) {
        expect(copperfin::runtime::format_value(period->second) == "12.5",
            "RESTORE FROM should parse period-decimal numeric values");
    }
    if (comma != state.globals.end()) {
        expect(copperfin::runtime::format_value(comma->second) == "0",
            "RESTORE FROM should reject comma-decimal numeric values");
    }
    if (currency != state.globals.end()) {
        expect(currency->second.kind == copperfin::runtime::PrgValueKind::currency &&
                   copperfin::runtime::format_value(currency->second) == "12.5000",
               "RESTORE FROM should parse period-decimal currency values with four-decimal fidelity");
    }
    if (currency_comma != state.globals.end()) {
        expect(copperfin::runtime::format_value(currency_comma->second) == "0.0000",
            "RESTORE FROM should reject comma-decimal currency values");
    }
    if (currency_min != state.globals.end()) {
        expect(copperfin::runtime::format_value(currency_min->second) == "-922337203685477.5808",
            "RESTORE FROM should preserve the minimum currency value");
    }
    if (currency_max != state.globals.end()) {
        expect(copperfin::runtime::format_value(currency_max->second) == "922337203685477.5807",
            "RESTORE FROM should preserve the maximum currency value");
    }
    if (currency_overflow != state.globals.end()) {
        expect(copperfin::runtime::format_value(currency_overflow->second) == "0.0000",
            "RESTORE FROM should reject out-of-range currency values");
    }
    if (currency_underflow != state.globals.end()) {
        expect(copperfin::runtime::format_value(currency_underflow->second) == "0.0000",
            "RESTORE FROM should reject underflowing currency values");
    }
    if (currency_round_max != state.globals.end()) {
        expect(copperfin::runtime::format_value(currency_round_max->second) == "0.0000",
            "RESTORE FROM should reject positive currency rounding carry beyond INT64_MAX");
    }
    if (currency_round_min != state.globals.end()) {
        expect(copperfin::runtime::format_value(currency_round_min->second) == "-922337203685477.5808",
            "RESTORE FROM should allow negative currency rounding carry to INT64_MIN");
    }
    if (currency_small != state.globals.end()) {
        expect(copperfin::runtime::format_value(currency_small->second) == "0.0001",
            "RESTORE FROM should round a half-unit currency value away from zero");
    }
    if (currency_tiny != state.globals.end()) {
        expect(copperfin::runtime::format_value(currency_tiny->second) == "0.0000",
            "RESTORE FROM should not round values below a half-unit currency value up");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_without_additive_clears_prior_globals() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_non_additive";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "state.mem";
    const fs::path main_path = temp_root / "restore_non_additive.prg";
    write_text(
        main_path,
        "from_file = 'saved'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "stale_only = 'drop-me'\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "after_from_file = from_file\n"
        "after_stale_type = VARTYPE(stale_only)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESTORE FROM non-additive script should complete");

    const auto after_from_file = state.globals.find("after_from_file");
    const auto after_stale_type = state.globals.find("after_stale_type");
    expect(after_from_file != state.globals.end(), "non-additive RESTORE should restore file-backed variables");
    expect(after_stale_type != state.globals.end(), "non-additive RESTORE should allow stale variable type inspection");

    if (after_from_file != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_from_file->second) == "saved",
            "non-additive RESTORE should keep file-backed values");
    }
    if (after_stale_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_stale_type->second) == "U",
            "non-additive RESTORE should clear globals not present in the .mem file");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_honors_current_frame_local_bindings() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_locals";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "locals.mem";
    const fs::path main_path = temp_root / "restore_locals.prg";
    write_text(
        main_path,
        "saved_value = 'outer'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "DO restore_proc\n"
        "after_proc_type = TYPE('saved_value')\n"
        "RETURN\n"
        "PROCEDURE restore_proc\n"
        "LOCAL saved_value\n"
        "PUBLIC restored_local\n"
        "saved_value = 'stale'\n"
        "RESTORE FROM '" + mem_path.string() + "' ADDITIVE\n"
        "restored_local = saved_value\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESTORE FROM should complete inside a LOCAL frame");

    const auto restored_local = state.globals.find("restored_local");
    const auto after_proc_type = state.globals.find("after_proc_type");
    expect(restored_local != state.globals.end(), "restored local value should be captured");
    expect(after_proc_type != state.globals.end(), "post-procedure TYPE() should be captured");
    if (restored_local != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_local->second) == "outer",
            "RESTORE FROM ADDITIVE should populate a current-frame LOCAL binding instead of a hidden global");
    }
    if (after_proc_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_proc_type->second) == "C",
            "restoring into a LOCAL should not destroy the visible outer global binding");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_restore_round_trips_arrays() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_restore_arrays";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "arrays.mem";
    const fs::path main_path = temp_root / "save_restore_arrays.prg";
    write_text(
        main_path,
        "DIMENSION aValues[2,2]\n"
        "aValues[1,1] = 'left'\n"
        "aValues[1,2] = 12.5\n"
        "aValues[2,1] = .T.\n"
        "aValues[2,2] = '01/15/2026'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "DIMENSION aValues[1]\n"
        "aValues[1] = 'stale'\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "restored_type = TYPE('aValues')\n"
        "restored_rows = ALEN(aValues, 1)\n"
        "restored_cols = ALEN(aValues, 2)\n"
        "restored_11 = aValues[1,1]\n"
        "restored_12 = aValues[1,2]\n"
        "restored_21 = aValues[2,1]\n"
        "restored_22 = aValues[2,2]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE/RESTORE array roundtrip should complete");

    const auto restored_type = state.globals.find("restored_type");
    const auto restored_rows = state.globals.find("restored_rows");
    const auto restored_cols = state.globals.find("restored_cols");
    const auto restored_11 = state.globals.find("restored_11");
    const auto restored_12 = state.globals.find("restored_12");
    const auto restored_21 = state.globals.find("restored_21");
    const auto restored_22 = state.globals.find("restored_22");

    expect(restored_type != state.globals.end(), "restored array TYPE() should be captured");
    expect(restored_rows != state.globals.end(), "restored array rows should be captured");
    expect(restored_cols != state.globals.end(), "restored array columns should be captured");
    expect(restored_11 != state.globals.end(), "restored array [1,1] should be captured");
    expect(restored_12 != state.globals.end(), "restored array [1,2] should be captured");
    expect(restored_21 != state.globals.end(), "restored array [2,1] should be captured");
    expect(restored_22 != state.globals.end(), "restored array [2,2] should be captured");

    if (restored_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_type->second) == "A",
            "RESTORE FROM should recreate saved arrays");
    }
    if (restored_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_rows->second) == "2",
            "RESTORE FROM should recreate the saved array row count");
    }
    if (restored_cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_cols->second) == "2",
            "RESTORE FROM should recreate the saved array column count");
    }
    if (restored_11 != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_11->second) == "left",
            "RESTORE FROM should preserve string array elements");
    }
    if (restored_12 != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_12->second) == "12.5",
            "RESTORE FROM should preserve numeric array elements");
    }
    if (restored_21 != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_21->second) == "true",
            "RESTORE FROM should preserve logical array elements");
    }
    if (restored_22 != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_22->second) == "01/15/2026",
            "RESTORE FROM should preserve date-like array elements");
    }

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("avalues=A:2,2|") != std::string::npos,
            "SAVE TO should serialize arrays with dimensions");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_restore_round_trips_public_scope() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_restore_public_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "public.mem";
    const fs::path main_path = temp_root / "save_restore_public_scope.prg";
    write_text(
        main_path,
        "PUBLIC cPub, aPub\n"
        "cPub = 'visible'\n"
        "DIMENSION aPub[1]\n"
        "aPub[1] = 'array-visible'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "RELEASE ALL\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "PUBLIC pub_scalar_type, pub_array_type, pub_scalar, pub_array, after_release_scalar, after_release_array\n"
        "pub_scalar_type = TYPE('cPub')\n"
        "pub_array_type = TYPE('aPub')\n"
        "pub_scalar = cPub\n"
        "pub_array = aPub[1]\n"
        "RELEASE ALL\n"
        "after_release_scalar = TYPE('cPub')\n"
        "after_release_array = TYPE('aPub')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE/RESTORE public scope roundtrip should complete");

    const auto pub_scalar_type = state.globals.find("pub_scalar_type");
    const auto pub_array_type = state.globals.find("pub_array_type");
    const auto pub_scalar = state.globals.find("pub_scalar");
    const auto pub_array = state.globals.find("pub_array");
    const auto after_release_scalar = state.globals.find("after_release_scalar");
    const auto after_release_array = state.globals.find("after_release_array");

    expect(pub_scalar_type != state.globals.end(), "restored public scalar TYPE() should be captured");
    expect(pub_array_type != state.globals.end(), "restored public array TYPE() should be captured");
    expect(pub_scalar != state.globals.end(), "restored public scalar should be captured");
    expect(pub_array != state.globals.end(), "restored public array element should be captured");
    expect(after_release_scalar != state.globals.end(), "post-release public scalar TYPE() should be captured");
    expect(after_release_array != state.globals.end(), "post-release public array TYPE() should be captured");

    if (pub_scalar_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_scalar_type->second) == "C",
            "RESTORE FROM should recreate saved PUBLIC scalar bindings");
    }
    if (pub_array_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_array_type->second) == "A",
            "RESTORE FROM should recreate saved PUBLIC array bindings");
    }
    if (pub_scalar != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_scalar->second) == "visible",
            "RESTORE FROM should preserve PUBLIC scalar values");
    }
    if (pub_array != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_array->second) == "array-visible",
            "RESTORE FROM should preserve PUBLIC array values");
    }
    if (after_release_scalar != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_release_scalar->second) == "C",
            "restored PUBLIC scalars should remain pinned across RELEASE ALL");
    }
    if (after_release_array != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_release_array->second) == "A",
            "restored PUBLIC arrays should remain pinned across RELEASE ALL");
    }

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cpub=C,PUBLIC:visible") != std::string::npos,
            "SAVE TO should persist PUBLIC scalar scope markers");
        expect(contents.find("apub=A,PUBLIC:1,1|") != std::string::npos,
            "SAVE TO should persist PUBLIC array scope markers");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_to_shadowed_public_name_does_not_persist_public_scope_marker() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_shadowed_public_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "shadowed.mem";
    const fs::path main_path = temp_root / "save_shadowed_public_scope.prg";
    write_text(
        main_path,
        "PUBLIC cScope\n"
        "cScope = 'public'\n"
        "DO saver\n"
        "RELEASE ALL\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "PUBLIC restored_scope, after_release_type\n"
        "restored_scope = cScope\n"
        "RELEASE ALL\n"
        "after_release_type = TYPE('cScope')\n"
        "RETURN\n"
        "PROCEDURE saver\n"
        "LOCAL cScope\n"
        "cScope = 'local'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE TO shadowed PUBLIC scope script should complete");

    const auto restored_scope = state.globals.find("restored_scope");
    const auto after_release_type = state.globals.find("after_release_type");
    expect(restored_scope != state.globals.end(), "restored_scope should be captured");
    expect(after_release_type != state.globals.end(), "after_release_type should be captured");

    if (restored_scope != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_scope->second) == "local",
            "RESTORE FROM should restore the visible shadowed binding value");
    }
    if (after_release_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_release_type->second) == "U",
            "shadowed local bindings saved through SAVE TO should not come back as PUBLIC after RELEASE ALL");
    }

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cscope=C,PUBLIC:local") == std::string::npos,
            "SAVE TO should not persist a PUBLIC scope marker when the visible binding is a LOCAL shadow");
        expect(contents.find("cscope=C:local") != std::string::npos,
            "SAVE TO should still persist the visible shadowed binding value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_without_additive_clears_stale_arrays() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_clears_arrays";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "arrays.mem";
    const fs::path main_path = temp_root / "restore_clears_arrays.prg";
    write_text(
        main_path,
        "from_file = 'saved'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "DIMENSION stale_arr[1]\n"
        "stale_arr[1] = 'drop-me'\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "after_stale_array_type = TYPE('stale_arr')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "non-additive RESTORE should clear stale arrays");

    const auto after_stale_array_type = state.globals.find("after_stale_array_type");
    expect(after_stale_array_type != state.globals.end(), "stale array TYPE() after restore should be captured");
    if (after_stale_array_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_stale_array_type->second) == "U",
            "non-additive RESTORE should clear arrays not present in the .mem file");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_without_additive_clears_private_shadow_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_clears_private";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "private.mem";
    const fs::path main_path = temp_root / "restore_clears_private.prg";
    write_text(
        main_path,
        "saved_value = 'saved'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "outer = 'outer'\n"
        "DO restore_proc\n"
        "after_type = TYPE('outer')\n"
        "RETURN\n"
        "PROCEDURE restore_proc\n"
        "PRIVATE outer\n"
        "outer = 'shadow'\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "non-additive RESTORE should complete inside PRIVATE shadowing");

    const auto after_type = state.globals.find("after_type");
    expect(after_type != state.globals.end(), "post-restore TYPE() should be captured");
    if (after_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_type->second) == "U",
            "non-additive RESTORE should clear deferred PRIVATE shadow state instead of restoring stale outer bindings");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_data_io

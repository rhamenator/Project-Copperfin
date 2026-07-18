#include "test_prg_engine_seek_index_support.h"

namespace copperfin::seek_index_tests
{
void test_declare_dll_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declare_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "declare_localization.prg";
#if defined(_WIN32)
    write_text(
        main_path,
        "DECLARE INTEGER MissingSymbol() IN 'kernel32.dll'\n"
        "RETURN\n");
#else
    write_text(
        main_path,
        "DECLARE INTEGER lstrcpyA(STRING @, STRING) IN 'kernel32.dll'\n"
        "RETURN\n");
#endif

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#2715: qps-ploc DECLARE error script should fail");
#if defined(_WIN32)
    expect(
        state.message.find("[!! ") == 0U &&
            state.message.find("MissingSymbol") != std::string::npos &&
            state.message.find("kernel32.dll") != std::string::npos &&
            state.message.find("function 'MissingSymbol' not found") == std::string::npos,
        "#2715: qps-ploc DECLARE function-not-found error should pseudo-localize prose while preserving function and path");
#else
    expect(
        state.message == copperfin::localization::pseudo_localize("DECLARE DLL is only supported on Windows."),
        "#2715: qps-ploc DECLARE Windows-only guard should route through the pseudo-localization transform");
#endif

    fs::remove_all(temp_root, ignored);
}


}

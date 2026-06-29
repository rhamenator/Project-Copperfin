#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_launch_max_selection_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_max_selection_target_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--max-width-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2484: default max-width target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --max-width-target-object-name.",
        "#2484: default max-width target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--hide-selection-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2484: default hide-selection target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --hide-selection-target-unique-id.",
        "#2484: default hide-selection target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--continuous-scroll-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2484: pseudo-localized continuous-scroll target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2484: pseudo-localized continuous-scroll target object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--continuous-scroll-target-object-name",
        "#2484: pseudo-localized continuous-scroll target object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --continuous-scroll-target-object-name.",
        "#2484: pseudo-localized continuous-scroll target object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--allow-cell-selection-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2484: pseudo-localized allow-cell-selection target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2484: pseudo-localized allow-cell-selection target unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--allow-cell-selection-target-unique-id",
        "#2484: pseudo-localized allow-cell-selection target unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --allow-cell-selection-target-unique-id.",
        "#2484: pseudo-localized allow-cell-selection target unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

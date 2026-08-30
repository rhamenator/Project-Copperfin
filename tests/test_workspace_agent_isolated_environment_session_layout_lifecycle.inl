void test_secure_generation_layout_preparation() {
    TempTree tree(false);
    const auto boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(boundary.has_value(),
           "RQ-CF-AGENT-014: a private trusted storage root must create the preparation boundary");
    if (!boundary.has_value()) {
        return;
    }

    const auto zero = boundary->prepare_session_layout(0U);
    expect(!zero.prepared && zero.session_generation == 0U &&
               zero.diagnostic_code ==
                   "workspace_agent.environment_invalid_session_generation",
           "RQ-CF-AGENT-014: generation zero must fail without layout authority");

    const auto prepared = boundary->prepare_session_layout(1U);
    expect(prepared.prepared && prepared.session_generation == 1U &&
               prepared.diagnostic_code ==
                   "workspace_agent.environment_session_layout_prepared",
           "RQ-CF-AGENT-014: a new generation must receive one complete private layout");
    const auto session = tree.session_storage / "session-1";
    bool complete_private_layout =
        copperfin::platform::verify_private_directory(session).ok;
    for (const std::string_view leaf :
         {"home", "temp", "config", "cache", "data"}) {
        complete_private_layout = complete_private_layout &&
            copperfin::platform::verify_private_directory(session / leaf).ok;
    }
    expect(complete_private_layout,
           "RQ-CF-AGENT-014: every prepared generation directory must satisfy the platform privacy contract");

    const auto construction = boundary->construct(
        1U, WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1);
    expect(construction.allowed,
           "RQ-CF-AGENT-014: the isolated environment must consume the verified prepared layout");

    const auto repeated = boundary->prepare_session_layout(1U);
    expect(!repeated.prepared && repeated.session_generation == 0U &&
               repeated.diagnostic_code ==
                   "workspace_agent.environment_session_layout_exists",
           "RQ-CF-AGENT-014: preparation must never adopt or overwrite an existing generation");

    TempTree partial_tree(false);
    const auto partial_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            partial_tree.configuration());
    expect(partial_boundary.has_value(),
           "RQ-CF-AGENT-014: the partial-layout fixture must retain a valid private root");
    if (!partial_boundary.has_value()) {
        return;
    }
    TempTree::require_private_directory(
        partial_tree.session_storage / "session-1");
    TempTree::require_private_directory(
        partial_tree.session_storage / "session-1" / "home");
    const auto partial = partial_boundary->prepare_session_layout(1U);
    expect(!partial.prepared && partial.session_generation == 0U &&
               partial.diagnostic_code ==
                   "workspace_agent.environment_session_layout_exists" &&
               std::filesystem::exists(
                   partial_tree.session_storage / "session-1" / "home"),
           "RQ-CF-AGENT-014: a partial preexisting layout must fail without repair or deletion");

    TempTree replaced_tree(false);
    const auto replaced_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            replaced_tree.configuration());
    expect(replaced_boundary.has_value(),
           "RQ-CF-AGENT-014: the replacement fixture must capture a private root");
    if (replaced_boundary.has_value()) {
        const auto replacement = replaced_tree.root / "sessions-replacement";
        TempTree::require_private_directory(replacement);
        std::filesystem::remove(replaced_tree.session_storage);
        std::filesystem::rename(replacement, replaced_tree.session_storage);
        const auto replaced = replaced_boundary->prepare_session_layout(1U);
        expect(!replaced.prepared && replaced.session_generation == 0U &&
                   replaced.diagnostic_code ==
                       "workspace_agent.environment_storage_root_identity_changed" &&
                   !std::filesystem::exists(
                       replaced_tree.session_storage / "session-1"),
               "RQ-CF-AGENT-014: root replacement must fail before creating a generation layout");
    }

    TempTree path_tree(false);
    const auto path_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            path_tree.configuration());
    expect(path_boundary.has_value(),
           "RQ-CF-AGENT-014: the executable-path replacement fixture must create its boundary");
    if (path_boundary.has_value()) {
        const auto replacement = path_tree.root / "approved-replacement";
        std::filesystem::create_directory(replacement);
        std::filesystem::remove(path_tree.approved_two);
        std::filesystem::rename(replacement, path_tree.approved_two);
        const auto replaced = path_boundary->prepare_session_layout(1U);
        expect(!replaced.prepared && replaced.session_generation == 0U &&
                   replaced.diagnostic_code ==
                       "workspace_agent.environment_path_identity_changed" &&
                   !std::filesystem::exists(
                       path_tree.session_storage / "session-1"),
               "RQ-CF-AGENT-014: executable-directory replacement must fail before creating a generation layout");
    }

#if defined(_WIN32)
    TempTree system_tree(false);
    const auto system_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            system_tree.configuration());
    expect(system_boundary.has_value(),
           "RQ-CF-AGENT-014: the system-root replacement fixture must create its boundary");
    if (system_boundary.has_value()) {
        const auto replacement =
            system_tree.root / "windows-root-replacement";
        std::filesystem::create_directory(replacement);
        std::filesystem::remove(system_tree.windows_system_root);
        std::filesystem::rename(replacement, system_tree.windows_system_root);
        const auto replaced = system_boundary->prepare_session_layout(1U);
        expect(!replaced.prepared && replaced.session_generation == 0U &&
                   replaced.diagnostic_code ==
                       "workspace_agent.environment_system_root_identity_changed" &&
                   !std::filesystem::exists(
                       system_tree.session_storage / "session-1"),
               "RQ-CF-AGENT-014: Windows system-root replacement must fail before creating a generation layout");
    }
#endif
}

void test_identity_bound_empty_layout_cleanup() {
    TempTree tree(false);
    const auto boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(boundary.has_value(),
           "RQ-CF-AGENT-020: cleanup fixture must create a trusted environment boundary");
    if (!boundary.has_value()) {
        return;
    }
    const auto prepared = boundary->prepare_session_layout(1U);
    expect(prepared.prepared && prepared.session_generation == 1U,
           "RQ-CF-AGENT-020: successful preparation must return an opaque cleanup receipt");
    const auto other_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(other_boundary.has_value(),
           "RQ-CF-AGENT-020: cross-boundary fixture must capture the same trusted root");
    if (other_boundary.has_value()) {
        const auto denied =
            other_boundary->cleanup_empty_session_layout(prepared);
        expect(!denied.cleaned && denied.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleanup_invalid_receipt" &&
                   std::filesystem::exists(tree.session_storage / "session-1"),
               "RQ-CF-AGENT-020: an opaque receipt must remain bound to the boundary that prepared it");
    }
    const auto cleaned = boundary->cleanup_empty_session_layout(prepared);
    expect(cleaned.cleaned && cleaned.session_generation == 1U &&
               cleaned.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleaned" &&
               !std::filesystem::exists(tree.session_storage / "session-1"),
           "RQ-CF-AGENT-020: an exact empty prepared layout must be removed completely");

    const auto invalid = boundary->cleanup_empty_session_layout({});
    expect(!invalid.cleaned && invalid.diagnostic_code ==
               "workspace_agent.environment_session_layout_cleanup_invalid_receipt",
           "RQ-CF-AGENT-020: generation numbers without a complete preparation receipt must not authorize cleanup");

    TempTree forged_tree(true);
    const auto forged_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            forged_tree.configuration());
    expect(forged_boundary.has_value(),
           "RQ-CF-AGENT-020: forged-receipt fixture must create its boundary");
    if (forged_boundary.has_value()) {
        copperfin::security::WorkspaceAgentSessionLayoutPreparationResult forged;
        forged.prepared = true;
        forged.session_generation = 1U;
        const auto denied =
            forged_boundary->cleanup_empty_session_layout(forged);
        expect(!denied.cleaned && denied.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleanup_invalid_receipt" &&
                   std::filesystem::exists(
                       forged_tree.session_storage / "session-1" / "data"),
               "RQ-CF-AGENT-020: public status fields must not forge cleanup authority for a pre-existing layout");
    }

    TempTree occupied_tree(false);
    const auto occupied_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            occupied_tree.configuration());
    expect(occupied_boundary.has_value(),
           "RQ-CF-AGENT-020: occupied cleanup fixture must create its boundary");
    if (occupied_boundary.has_value()) {
        const auto occupied = occupied_boundary->prepare_session_layout(1U);
        std::ofstream(occupied_tree.session_storage / "session-1" / "data" /
                      "retained.txt") << "retain\n";
        const auto denied =
            occupied_boundary->cleanup_empty_session_layout(occupied);
        expect(!denied.cleaned &&
                   (denied.diagnostic_code ==
                        "workspace_agent.environment_session_layout_cleanup_identity_changed" ||
                    denied.diagnostic_code ==
                        "workspace_agent.environment_session_layout_cleanup_not_empty") &&
                   std::filesystem::exists(
                       occupied_tree.session_storage / "session-1" / "data" /
                       "retained.txt"),
               "RQ-CF-AGENT-020: cleanup must never recurse into or remove session content");
    }

    TempTree replaced_tree(false);
    const auto replaced_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            replaced_tree.configuration());
    expect(replaced_boundary.has_value(),
           "RQ-CF-AGENT-020: replaced-child fixture must create its boundary");
    if (replaced_boundary.has_value()) {
        const auto prepared_replaced =
            replaced_boundary->prepare_session_layout(1U);
        const auto data_path =
            replaced_tree.session_storage / "session-1" / "data";
        std::filesystem::remove(data_path);
        TempTree::require_private_directory(data_path);
        const auto denied = replaced_boundary->cleanup_empty_session_layout(
            prepared_replaced);
        expect(!denied.cleaned && denied.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleanup_identity_changed" &&
                   std::filesystem::exists(data_path),
               "RQ-CF-AGENT-020: a replaced child identity must be preserved and deny cleanup");
    }
}

#if defined(__linux__)
void test_unrepresentable_layout_denied_before_creation() {
    TempTree tree(false);
    std::filesystem::remove(tree.session_storage);

    constexpr std::size_t target_parent_bytes = 4055U;
    std::filesystem::path long_parent = tree.root;
    while (long_parent.native().size() < target_parent_bytes) {
        const std::size_t remaining =
            target_parent_bytes - long_parent.native().size() - 1U;
        const std::size_t component_bytes =
            std::max<std::size_t>(1U, std::min<std::size_t>(200U, remaining));
        long_parent /= std::string(component_bytes, 'a');
        std::filesystem::create_directory(long_parent);
    }
    std::filesystem::permissions(
        long_parent,
        std::filesystem::perms::owner_all,
        std::filesystem::perm_options::replace);
    tree.session_storage = long_parent / "sessions";
    TempTree::require_private_directory(tree.session_storage);

    const auto boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(boundary.has_value(),
           "RQ-CF-AGENT-014: the long-path fixture must admit its private storage root");
    if (!boundary.has_value()) {
        return;
    }
    const auto result = boundary->prepare_session_layout(1U);
    expect(!result.prepared && result.session_generation == 0U &&
               result.diagnostic_code ==
                   "workspace_agent.environment_session_layout_unrepresentable" &&
               !std::filesystem::exists(tree.session_storage / "session-1"),
           "RQ-CF-AGENT-014: an oversized derived environment entry must fail before layout creation");
}
#endif

void test_configuration_and_layout_fail_closed() {
    TempTree tree;
    const auto valid_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(valid_boundary.has_value(),
           "RQ-CF-AGENT-012: valid trusted configuration must create a boundary");
    if (valid_boundary.has_value()) {
        const auto zero_generation = valid_boundary->construct(
            0U, WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1);
        expect(!zero_generation.allowed && zero_generation.entries.empty() &&
                   zero_generation.diagnostic_code ==
                       "workspace_agent.environment_invalid_session_generation",
               "RQ-CF-AGENT-012: zero generation must fail without environment content");
        const auto invalid_policy = valid_boundary->construct(
            1U, static_cast<WorkspaceAgentProcessEnvironmentPolicy>(99U));
        expect(!invalid_policy.allowed && invalid_policy.entries.empty() &&
                   invalid_policy.diagnostic_code ==
                       "workspace_agent.environment_invalid_policy",
               "RQ-CF-AGENT-012: unknown environment policies must fail without content");
    }

    auto invalid_schema = tree.configuration();
    invalid_schema.schema_version = 2U;
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(invalid_schema).has_value(),
           "RQ-CF-AGENT-012: unknown trusted-configuration schemas must fail");

    auto no_path = tree.configuration();
    no_path.trusted_executable_directories.clear();
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(no_path).has_value(),
           "RQ-CF-AGENT-012: an empty approved executable-directory set must fail");

    auto duplicate_path = tree.configuration();
    duplicate_path.trusted_executable_directories.push_back(tree.approved_one);
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(duplicate_path).has_value(),
           "RQ-CF-AGENT-012: duplicate approved directories must fail");

    auto too_many_paths = tree.configuration();
    too_many_paths.trusted_executable_directories.assign(
        copperfin::security::workspace_agent_environment_max_path_directories + 1U,
        tree.approved_one);
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                too_many_paths).has_value(),
           "RQ-CF-AGENT-012: excessive approved-directory count must fail before copying");

    const auto delimited = tree.root /
#if defined(_WIN32)
        "bad;path";
#else
        "bad:path";
#endif
    std::filesystem::create_directory(delimited);
    auto ambiguous_path = tree.configuration();
    ambiguous_path.trusted_executable_directories = {delimited};
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                ambiguous_path).has_value(),
           "RQ-CF-AGENT-012: platform PATH-delimiter ambiguity must fail at trusted configuration");

#if defined(_WIN32)
    auto missing_system_root = tree.configuration();
    missing_system_root.trusted_windows_system_root.clear();
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                missing_system_root).has_value(),
           "RQ-CF-AGENT-012: Windows construction requires an explicit trusted system root");

    // Win32 silently strips a trailing '.' or ' ' from a path component, so
    // e.g. "sessions." would otherwise name the same object as the admitted
    // "sessions" -- an alias the strict-spelling check must reject before it
    // is ever used to bind trust.
    auto trailing_dot_storage = tree.configuration();
    trailing_dot_storage.trusted_session_storage_root =
        std::filesystem::path(tree.session_storage.wstring() + L".");
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                trailing_dot_storage).has_value(),
           "RQ-CF-AGENT-012: a trailing-dot session storage root must fail");

    auto trailing_space_storage = tree.configuration();
    trailing_space_storage.trusted_session_storage_root =
        std::filesystem::path(tree.session_storage.wstring() + L" ");
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                trailing_space_storage).has_value(),
           "RQ-CF-AGENT-012: a trailing-space session storage root must fail");

    auto trailing_dot_executable_directory = tree.configuration();
    trailing_dot_executable_directory.trusted_executable_directories = {
        std::filesystem::path(tree.approved_one.wstring() + L".")};
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                trailing_dot_executable_directory).has_value(),
           "RQ-CF-AGENT-012: a trailing-dot approved executable directory must fail");

    auto trailing_space_executable_directory = tree.configuration();
    trailing_space_executable_directory.trusted_executable_directories = {
        std::filesystem::path(tree.approved_one.wstring() + L" ")};
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                trailing_space_executable_directory).has_value(),
           "RQ-CF-AGENT-012: a trailing-space approved executable directory must fail");

    auto trailing_dot_system_root = tree.configuration();
    trailing_dot_system_root.trusted_windows_system_root =
        std::filesystem::path(tree.windows_system_root.wstring() + L".");
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                trailing_dot_system_root).has_value(),
           "RQ-CF-AGENT-012: a trailing-dot Windows system root must fail");

    auto trailing_space_system_root = tree.configuration();
    trailing_space_system_root.trusted_windows_system_root =
        std::filesystem::path(tree.windows_system_root.wstring() + L" ");
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                trailing_space_system_root).has_value(),
           "RQ-CF-AGENT-012: a trailing-space Windows system root must fail");
#else
    auto unexpected_system_root = tree.configuration();
    unexpected_system_root.trusted_windows_system_root = tree.windows_system_root;
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                unexpected_system_root).has_value(),
           "RQ-CF-AGENT-012: POSIX construction rejects unused Windows configuration");
#endif

    WorkspaceAgentSessionController no_boundary(tree.workspace);
    const auto no_boundary_start = no_boundary.start(activation_request(), audit_sink());
    expect_content_free_denial(
        no_boundary.preflight_process_environment_request(
            invocation_request(no_boundary_start.session.generation)),
        "workspace_agent.process_environment_boundary_unavailable",
        "RQ-CF-AGENT-012: a controller without trusted environment configuration must fail without reflection");
    expect_serialization_content_free_denial(
        no_boundary.preflight_serialized_process_environment_request(
            invocation_request(no_boundary_start.session.generation)),
        "workspace_agent.process_environment_boundary_unavailable",
        "RQ-CF-AGENT-013: serialization denial must not reflect an invocation or environment");
    expect_invocation_serialization_content_free_denial(
        no_boundary.preflight_serialized_process_invocation_request(
            invocation_request(no_boundary_start.session.generation)),
        "workspace_agent.process_environment_boundary_unavailable",
        "RQ-CF-AGENT-015: argument serialization denial must not reflect an invocation, environment, path, or argument");

    WorkspaceAgentSessionController missing_layout(
        tree.workspace, tree.configuration());
    const auto missing_layout_start = missing_layout.start(
        activation_request(), audit_sink());
    std::filesystem::remove_all(tree.session_storage / "session-1" / "cache");
    expect_content_free_denial(
        missing_layout.preflight_process_environment_request(
            invocation_request(missing_layout_start.session.generation)),
        "workspace_agent.environment_session_layout_unavailable",
        "RQ-CF-AGENT-012: incomplete session-owned layout must fail without reflecting paths");

    TempTree insecure_root_tree(false);
    std::filesystem::remove(insecure_root_tree.session_storage);
    std::filesystem::create_directory(insecure_root_tree.session_storage);
#if !defined(_WIN32)
    std::filesystem::permissions(
        insecure_root_tree.session_storage,
        std::filesystem::perms::owner_all |
            std::filesystem::perms::group_read |
            std::filesystem::perms::group_exec,
        std::filesystem::perm_options::replace);
#endif
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                insecure_root_tree.configuration()).has_value(),
           "RQ-CF-AGENT-014: an inherited or broadened trusted storage root must be rejected");

#if !defined(_WIN32)
    TempTree broadened_after_capture;
    WorkspaceAgentSessionController broadened_controller(
        broadened_after_capture.workspace,
        broadened_after_capture.configuration());
    const auto broadened_start = broadened_controller.start(
        activation_request(), audit_sink());
    std::filesystem::permissions(
        broadened_after_capture.session_storage,
        std::filesystem::perms::owner_all |
            std::filesystem::perms::group_read |
            std::filesystem::perms::group_exec,
        std::filesystem::perm_options::replace);
    expect_content_free_denial(
        broadened_controller.preflight_process_environment_request(
            invocation_request(broadened_start.session.generation)),
        "workspace_agent.environment_storage_root_identity_changed",
        "RQ-CF-AGENT-014: storage-root access broadening after capture must fail construction");
#endif
}

void test_physical_identity_and_session_binding() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace, tree.configuration());
    const auto start = controller.start(activation_request(), audit_sink());

    const auto approved_replacement = tree.root / "approved-replacement";
    std::filesystem::create_directory(approved_replacement);
    std::filesystem::remove(tree.approved_two);
    std::filesystem::rename(approved_replacement, tree.approved_two);
    expect_content_free_denial(
        controller.preflight_process_environment_request(
            invocation_request(start.session.generation)),
        "workspace_agent.environment_path_identity_changed",
        "RQ-CF-AGENT-012: replacement of an approved executable directory must fail closed");

    TempTree stale_tree;
    WorkspaceAgentSessionController stale(
        stale_tree.workspace, stale_tree.configuration());
    const auto stale_start = stale.start(activation_request(), audit_sink());
    expect_content_free_denial(
        stale.preflight_process_environment_request(
            invocation_request(stale_start.session.generation + 1U)),
        "workspace_agent.tool_stale_session",
        "RQ-CF-AGENT-012: stale session generations must fail before environment construction");

    TempTree storage_tree;
    WorkspaceAgentSessionController replaced_storage(
        storage_tree.workspace, storage_tree.configuration());
    const auto storage_start = replaced_storage.start(
        activation_request(), audit_sink());
    const auto storage_replacement = storage_tree.root / "sessions-replacement";
    std::filesystem::create_directory(storage_replacement);
    std::filesystem::remove_all(storage_tree.session_storage);
    std::filesystem::rename(storage_replacement, storage_tree.session_storage);
    expect_content_free_denial(
        replaced_storage.preflight_process_environment_request(
            invocation_request(storage_start.session.generation)),
        "workspace_agent.environment_storage_root_identity_changed",
        "RQ-CF-AGENT-012: replacement of the trusted session-storage root must fail closed");

#if defined(_WIN32)
    TempTree system_tree;
    WorkspaceAgentSessionController replaced_system(
        system_tree.workspace, system_tree.configuration());
    const auto system_start = replaced_system.start(
        activation_request(), audit_sink());
    const auto system_replacement = system_tree.root / "windows-root-replacement";
    std::filesystem::create_directory(system_replacement);
    std::filesystem::remove(system_tree.windows_system_root);
    std::filesystem::rename(system_replacement, system_tree.windows_system_root);
    expect_content_free_denial(
        replaced_system.preflight_process_environment_request(
            invocation_request(system_start.session.generation)),
        "workspace_agent.environment_system_root_identity_changed",
        "RQ-CF-AGENT-012: replacement of the trusted Windows system root must fail closed");
#endif

#if !defined(_WIN32)
    TempTree indirect_tree;
    WorkspaceAgentSessionController indirect(
        indirect_tree.workspace, indirect_tree.configuration());
    const auto indirect_start = indirect.start(activation_request(), audit_sink());
    const auto config = indirect_tree.session_storage / "session-1" / "config";
    std::filesystem::remove(config);
    std::filesystem::create_directory_symlink(indirect_tree.outside, config);
    expect_content_free_denial(
        indirect.preflight_process_environment_request(
            invocation_request(indirect_start.session.generation)),
        "workspace_agent.environment_session_layout_unavailable",
        "RQ-CF-AGENT-012: indirect session layout components must fail closed");
#endif
}

void test_later_session_layout_is_not_root_replacement() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace, tree.configuration());
    const auto first = controller.start(activation_request(), audit_sink());
    expect(first.activated && first.session.generation == 1U,
           "RQ-CF-AGENT-012: first fixture generation must activate");
    expect(controller.stop(audit_sink()).revoked,
           "RQ-CF-AGENT-012: first fixture generation must stop");

    const auto second = controller.start(activation_request(), audit_sink());
    expect(second.activated && second.session.generation == 2U,
           "RQ-CF-AGENT-012: later fixture generation must activate");
    const auto result = controller.preflight_process_environment_request(
        invocation_request(second.session.generation));
    expect(result.allowed &&
               result.diagnostic_code ==
                   "workspace_agent.process_environment_request_allowed" &&
               find_entry(result.environment_entries, "HOME") != nullptr,
           "RQ-CF-AGENT-016: session start must prepare a later generation without treating it as storage-root replacement");
}

void test_session_start_prepares_layout_before_authority() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace, tree.configuration());
    const auto started = controller.start(activation_request(), audit_sink());
    bool complete_private_layout = copperfin::platform::verify_private_directory(
        tree.session_storage / "session-1").ok;
    for (const std::string_view leaf :
         {"home", "temp", "config", "cache", "data"}) {
        complete_private_layout = complete_private_layout &&
            copperfin::platform::verify_private_directory(
                tree.session_storage / "session-1" / leaf).ok;
    }
    expect(started.activated && started.audit_committed &&
               started.session.generation == 1U &&
               complete_private_layout,
           "RQ-CF-AGENT-016: configured process-capable start must prepare its exact private generation before authority");

    TempTree preexisting;
    preexisting.create_session_layout(1U);
    WorkspaceAgentSessionController refuses_adoption(
        preexisting.workspace, preexisting.configuration());
    AuditCapture refused_audit;
    const auto refused = refuses_adoption.start(
        activation_request(), audit_sink(refused_audit));
    expect(!refused.activated && refused.audit_committed &&
               refused.diagnostic_code ==
                   "workspace_agent.environment_session_layout_exists" &&
               !refused.policy_decision.allowed &&
               !refuses_adoption.snapshot().active &&
               refused_audit.events.size() == 1U &&
               refused_audit.events.front().outcome == "denied" &&
               refused_audit.events.front().diagnostic_code ==
                   refused.diagnostic_code,
           "RQ-CF-AGENT-016: session start must audit denial and grant no authority when its generation already exists");

    TempTree invalid;
    auto invalid_configuration = invalid.configuration();
    invalid_configuration.schema_version = 2U;
    WorkspaceAgentSessionController invalid_boundary(
        invalid.workspace, invalid_configuration);
    const auto invalid_start = invalid_boundary.start(
        activation_request(), audit_sink());
    expect(!invalid_start.activated && invalid_start.audit_committed &&
               invalid_start.diagnostic_code ==
                   "workspace_agent.session_environment_boundary_unavailable" &&
               !std::filesystem::exists(invalid.session_storage / "session-1"),
           "RQ-CF-AGENT-016: supplied but invalid trusted environment configuration must fail start without creating authority or layout");

    TempTree policy_denied;
    WorkspaceAgentSessionController denied_controller(
        policy_denied.workspace, policy_denied.configuration());
    auto disabled = activation_request();
    disabled.feature_enabled = false;
    const auto denied = denied_controller.start(disabled, audit_sink());
    expect(!denied.activated && denied.audit_committed &&
               !std::filesystem::exists(
                   policy_denied.session_storage / "session-1"),
           "RQ-CF-AGENT-016: policy denial must occur without preparing an unused generation layout");

    TempTree advisory;
    WorkspaceAgentSessionController advisory_controller(
        advisory.workspace, advisory.configuration());
    auto advisory_request = activation_request();
    advisory_request.requested_mode = WorkspaceAgentAccessMode::advisory;
    const auto advisory_start = advisory_controller.start(
        advisory_request, audit_sink());
    expect(advisory_start.activated &&
               !advisory_start.session.capabilities.run_local_processes &&
               !std::filesystem::exists(
                   advisory.session_storage / "session-1"),
           "RQ-CF-AGENT-016: a non-process-capable session must not prepare an unused generation layout");
    expect(advisory_controller.stop(audit_sink()).revoked,
           "RQ-CF-AGENT-016: advisory fixture must revoke cleanly");
    const auto process_after_advisory = advisory_controller.start(
        activation_request(), audit_sink());
    expect(process_after_advisory.activated &&
               process_after_advisory.session.generation == 2U &&
               std::filesystem::exists(
                   advisory.session_storage / "session-2") &&
               !std::filesystem::exists(
                   advisory.session_storage / "session-1"),
           "RQ-CF-AGENT-016: a later process-capable session must prepare only its own fresh generation");

    TempTree audit_failed;
    WorkspaceAgentSessionController audit_failed_controller(
        audit_failed.workspace, audit_failed.configuration());
    const auto uncommitted = audit_failed_controller.start(
        activation_request(), {});
    expect(!uncommitted.activated && !uncommitted.audit_committed &&
               uncommitted.diagnostic_code ==
                   "workspace_agent.session_audit_commit_failed" &&
               std::filesystem::exists(
                   audit_failed.session_storage / "session-1") &&
               !audit_failed_controller.snapshot().active,
           "RQ-CF-AGENT-016: audit failure must withhold authority and leave the prepared generation untouched for later audit-backed cleanup");
    const auto recovered = audit_failed_controller.start(
        activation_request(), audit_sink());
    expect(recovered.activated && recovered.session.generation == 2U &&
               std::filesystem::exists(
                   audit_failed.session_storage / "session-2"),
           "RQ-CF-AGENT-016: a later start must use a fresh generation rather than adopt an orphaned prepared layout");
}

void test_controller_retains_and_audits_explicit_layout_cleanup() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace, tree.configuration());
    AuditCapture audit;
    const auto started = controller.start(activation_request(), audit_sink(audit));
    expect(started.activated &&
               std::filesystem::exists(tree.session_storage / "session-1"),
           "RQ-CF-AGENT-021: configured start must retain cleanup authority for its prepared generation");

    const auto active_denial =
        controller.cleanup_pending_session_layout(audit_sink(audit));
    expect(!active_denial.attempted && !active_denial.cleaned &&
               active_denial.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_active_session" &&
               audit.events.size() == 1U,
           "RQ-CF-AGENT-021: cleanup must not begin while any session authority remains active");

    const auto stopped = controller.stop(audit_sink(audit));
    const auto cleaned =
        controller.cleanup_pending_session_layout(audit_sink(audit));
    expect(stopped.revoked && cleaned.attempted && cleaned.cleaned &&
               cleaned.intent_audit_committed &&
               cleaned.outcome_audit_committed &&
               cleaned.session_generation == started.session.generation &&
               cleaned.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleaned" &&
               !std::filesystem::exists(tree.session_storage / "session-1") &&
               audit.events.size() == 4U &&
               audit.events[2].kind ==
                   copperfin::security::WorkspaceAgentSessionEventKind::
                       layout_cleanup_intent &&
               audit.events[2].outcome == "pending" &&
               audit.events[3].kind ==
                   copperfin::security::WorkspaceAgentSessionEventKind::
                       layout_cleanup_outcome &&
               audit.events[3].outcome == "cleaned",
           "RQ-CF-AGENT-021: revoked empty layout cleanup must be bracketed by intent and outcome audit records");
    const auto duplicate =
        controller.cleanup_pending_session_layout(audit_sink(audit));
    expect(!duplicate.attempted &&
               duplicate.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_not_pending" &&
               audit.events.size() == 4U,
           "RQ-CF-AGENT-021: a consumed cleanup receipt must not be reusable");

    TempTree unaudited_tree;
    WorkspaceAgentSessionController unaudited(
        unaudited_tree.workspace, unaudited_tree.configuration());
    const auto uncommitted = unaudited.start(activation_request(), {});
    expect(!uncommitted.activated &&
               std::filesystem::exists(
                   unaudited_tree.session_storage / "session-1"),
           "RQ-CF-AGENT-021: failed start audit fixture must leave its prepared layout pending");
    const auto missing_intent = unaudited.cleanup_pending_session_layout({});
    expect(!missing_intent.attempted && !missing_intent.cleaned &&
               !missing_intent.intent_audit_committed &&
               missing_intent.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_intent_audit_failed" &&
               std::filesystem::exists(
                   unaudited_tree.session_storage / "session-1"),
           "RQ-CF-AGENT-021: cleanup must not mutate without a durable intent receipt");
    AuditCapture recovery_audit;
    const auto recovered = unaudited.cleanup_pending_session_layout(
        audit_sink(recovery_audit));
    expect(recovered.cleaned && recovery_audit.events.size() == 2U &&
               !std::filesystem::exists(
                   unaudited_tree.session_storage / "session-1"),
           "RQ-CF-AGENT-021: intent-audit failure must retain the opaque receipt for a later explicit retry");

    TempTree outcome_failure_tree;
    WorkspaceAgentSessionController outcome_failure(
        outcome_failure_tree.workspace, outcome_failure_tree.configuration());
    const auto outcome_start = outcome_failure.start(
        activation_request(), audit_sink());
    expect(outcome_start.activated && outcome_failure.stop(audit_sink()).revoked,
           "RQ-CF-AGENT-021: outcome-audit failure fixture must first revoke authority");
    AuditCapture failing_outcome;
    failing_outcome.fail_on_event = 2U;
    const auto cleaned_without_outcome =
        outcome_failure.cleanup_pending_session_layout(
            audit_sink(failing_outcome));
    const auto consumed_after_outcome_failure =
        outcome_failure.cleanup_pending_session_layout(audit_sink());
    expect(cleaned_without_outcome.attempted &&
               cleaned_without_outcome.cleaned &&
               cleaned_without_outcome.intent_audit_committed &&
               !cleaned_without_outcome.outcome_audit_committed &&
               cleaned_without_outcome.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_outcome_audit_failed" &&
               failing_outcome.events.size() == 2U &&
               !std::filesystem::exists(
                   outcome_failure_tree.session_storage / "session-1") &&
               !consumed_after_outcome_failure.attempted &&
               consumed_after_outcome_failure.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_not_pending",
           "RQ-CF-AGENT-021: successful mutation must consume its receipt even when outcome audit persistence fails");

    TempTree occupied_tree;
    WorkspaceAgentSessionController occupied(
        occupied_tree.workspace, occupied_tree.configuration());
    AuditCapture occupied_audit;
    const auto occupied_start = occupied.start(
        activation_request(), audit_sink(occupied_audit));
    std::ofstream(occupied_tree.session_storage / "session-1" / "data" /
                  "retained.txt") << "retain\n";
    expect(occupied.stop(audit_sink(occupied_audit)).revoked,
           "RQ-CF-AGENT-021: occupied fixture must revoke before cleanup");
    const auto retained = occupied.cleanup_pending_session_layout(
        audit_sink(occupied_audit));
    expect(occupied_start.activated && retained.attempted && !retained.cleaned &&
               retained.intent_audit_committed &&
               retained.outcome_audit_committed &&
               retained.diagnostic_code !=
                   "workspace_agent.environment_session_layout_cleaned" &&
               std::filesystem::exists(
                   occupied_tree.session_storage / "session-1" / "data" /
                   "retained.txt") &&
               occupied_audit.events.back().outcome == "retained",
           "RQ-CF-AGENT-021: occupied content must be preserved and produce an audited retained outcome");

    TempTree bounded_tree;
    WorkspaceAgentSessionController bounded(
        bounded_tree.workspace, bounded_tree.configuration());
    for (std::size_t index = 0U;
         index <
             copperfin::security::
                 workspace_agent_session_max_pending_layout_cleanups;
         ++index) {
        const auto bounded_start = bounded.start(activation_request(), audit_sink());
        expect(bounded_start.activated && bounded.stop(audit_sink()).revoked,
               "RQ-CF-AGENT-021: every generation below the pending-receipt cap must retain its distinct receipt");
    }
    AuditCapture capacity_audit;
    const auto capacity_denied = bounded.start(
        activation_request(), audit_sink(capacity_audit));
    expect(!capacity_denied.activated && capacity_denied.audit_committed &&
               capacity_denied.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_capacity_reached" &&
               capacity_audit.events.size() == 1U &&
               capacity_audit.events.front().outcome == "denied" &&
               !std::filesystem::exists(
                   bounded_tree.session_storage / "session-65"),
           "RQ-CF-AGENT-021: the fixed pending-receipt cap must deny before creating another layout");
}

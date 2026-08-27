void test_default_dispatch_catalogs_refresh_when_locale_changes() {
    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    locale_override.set("en-US");
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto english_editor_dispatch = copperfin::studio::plan_studio_editor_action_dispatch({});
    const auto english_toolbox_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({});
    const auto english_builder_dispatch = copperfin::studio::plan_studio_builder_dispatch({});
    const auto english_designer_dispatch = copperfin::studio::plan_studio_designer_dispatch({});
    const auto english_editor_invocation =
        copperfin::studio::plan_studio_editor_action_invocation_admission({});
    const auto english_toolbox_invocation =
        copperfin::studio::plan_studio_toolbox_invocation_admission({});
    const auto english_builder_invocation =
        copperfin::studio::plan_studio_builder_invocation_admission({});
    const auto english_designer_invocation =
        copperfin::studio::plan_studio_designer_invocation_admission({});
    const auto english_actions = copperfin::studio::studio_editor_action_registry();

    locale_override.set("es-419");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto spanish_editor_dispatch = copperfin::studio::plan_studio_editor_action_dispatch({});
    const auto spanish_toolbox_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({});
    const auto spanish_builder_dispatch = copperfin::studio::plan_studio_builder_dispatch({});
    const auto spanish_designer_dispatch = copperfin::studio::plan_studio_designer_dispatch({});
    const auto spanish_editor_invocation =
        copperfin::studio::plan_studio_editor_action_invocation_admission({});
    const auto spanish_toolbox_invocation =
        copperfin::studio::plan_studio_toolbox_invocation_admission({});
    const auto spanish_builder_invocation =
        copperfin::studio::plan_studio_builder_invocation_admission({});
    const auto spanish_designer_invocation =
        copperfin::studio::plan_studio_designer_invocation_admission({});
    const auto spanish_actions = copperfin::studio::studio_editor_action_registry();

    locale_override.set("qps-ploc");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto pseudo_editor_dispatch = copperfin::studio::plan_studio_editor_action_dispatch({});
    const auto pseudo_toolbox_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({});
    const auto pseudo_builder_dispatch = copperfin::studio::plan_studio_builder_dispatch({});
    const auto pseudo_designer_dispatch = copperfin::studio::plan_studio_designer_dispatch({});
    const auto pseudo_editor_invocation =
        copperfin::studio::plan_studio_editor_action_invocation_admission({});
    const auto pseudo_toolbox_invocation =
        copperfin::studio::plan_studio_toolbox_invocation_admission({});
    const auto pseudo_builder_invocation =
        copperfin::studio::plan_studio_builder_invocation_admission({});
    const auto pseudo_designer_invocation =
        copperfin::studio::plan_studio_designer_invocation_admission({});
    const auto pseudo_actions = copperfin::studio::studio_editor_action_registry();

    constexpr std::string_view editor_dispatch_key =
        "Studio.EditorActionDispatch.Error.ValidatedActionIdRequired";
    constexpr std::string_view toolbox_dispatch_key =
        "Studio.ToolboxDispatch.Error.CommandTokenRequired";
    constexpr std::string_view builder_dispatch_key =
        "Studio.BuilderDispatch.Error.ValidatedBuilderIdRequired";
    constexpr std::string_view designer_dispatch_key =
        "Studio.DesignerDispatch.Error.InvocationAdmissionRequired";
    constexpr std::string_view editor_invocation_key =
        "Studio.EditorActionInvocationAdmission.Error.ValidatedActionIdRequired";
    constexpr std::string_view toolbox_invocation_key =
        "Studio.ToolboxInvocationAdmission.Error.ValidatedItemMetadataRequired";
    constexpr std::string_view builder_invocation_key =
        "Studio.BuilderInvocationAdmission.Error.ValidatedBuilderIdRequired";
    constexpr std::string_view designer_invocation_key =
        "Studio.DesignerInvocationAdmission.Error.ValidatedLaunchSurfaceRequired";
    constexpr std::string_view action_label_key = "Studio.EditorAction.ShowPropertyGrid.Label";
    expect(!english_editor_dispatch.ok && english_editor_dispatch.error == english_catalog.translate(editor_dispatch_key) &&
               !spanish_editor_dispatch.ok && spanish_editor_dispatch.error == spanish_catalog.translate(editor_dispatch_key) &&
               !pseudo_editor_dispatch.ok && pseudo_editor_dispatch.error == pseudo_catalog.translate(editor_dispatch_key),
           "#4367: editor-action dispatch diagnostics should refresh across locales");
    expect(!english_toolbox_dispatch.ok && english_toolbox_dispatch.error == english_catalog.translate(toolbox_dispatch_key) &&
               !spanish_toolbox_dispatch.ok && spanish_toolbox_dispatch.error == spanish_catalog.translate(toolbox_dispatch_key) &&
               !pseudo_toolbox_dispatch.ok && pseudo_toolbox_dispatch.error == pseudo_catalog.translate(toolbox_dispatch_key),
           "#4367: toolbox dispatch diagnostics should refresh across locales");
    expect(!english_builder_dispatch.ok && english_builder_dispatch.error == english_catalog.translate(builder_dispatch_key) &&
               !spanish_builder_dispatch.ok && spanish_builder_dispatch.error == spanish_catalog.translate(builder_dispatch_key) &&
               !pseudo_builder_dispatch.ok && pseudo_builder_dispatch.error == pseudo_catalog.translate(builder_dispatch_key),
           "#4367: builder dispatch diagnostics should refresh across locales");
    expect(!english_designer_dispatch.ok && english_designer_dispatch.error == english_catalog.translate(designer_dispatch_key) &&
               !spanish_designer_dispatch.ok && spanish_designer_dispatch.error == spanish_catalog.translate(designer_dispatch_key) &&
               !pseudo_designer_dispatch.ok && pseudo_designer_dispatch.error == pseudo_catalog.translate(designer_dispatch_key),
           "#4367: designer dispatch diagnostics should refresh across locales");
    expect(!english_editor_invocation.ok && english_editor_invocation.error == english_catalog.translate(editor_invocation_key) &&
               !spanish_editor_invocation.ok && spanish_editor_invocation.error == spanish_catalog.translate(editor_invocation_key) &&
               !pseudo_editor_invocation.ok && pseudo_editor_invocation.error == pseudo_catalog.translate(editor_invocation_key),
           "#4367: editor-action invocation diagnostics should refresh across locales");
    expect(!english_toolbox_invocation.ok && english_toolbox_invocation.error == english_catalog.translate(toolbox_invocation_key) &&
               !spanish_toolbox_invocation.ok && spanish_toolbox_invocation.error == spanish_catalog.translate(toolbox_invocation_key) &&
               !pseudo_toolbox_invocation.ok && pseudo_toolbox_invocation.error == pseudo_catalog.translate(toolbox_invocation_key),
           "#4367: toolbox invocation diagnostics should refresh across locales");
    expect(!english_builder_invocation.ok && english_builder_invocation.error == english_catalog.translate(builder_invocation_key) &&
               !spanish_builder_invocation.ok && spanish_builder_invocation.error == spanish_catalog.translate(builder_invocation_key) &&
               !pseudo_builder_invocation.ok && pseudo_builder_invocation.error == pseudo_catalog.translate(builder_invocation_key),
           "#4367: builder invocation diagnostics should refresh across locales");
    expect(!english_designer_invocation.ok && english_designer_invocation.error == english_catalog.translate(designer_invocation_key) &&
               !spanish_designer_invocation.ok && spanish_designer_invocation.error == spanish_catalog.translate(designer_invocation_key) &&
               !pseudo_designer_invocation.ok && pseudo_designer_invocation.error == pseudo_catalog.translate(designer_invocation_key),
           "#4367: designer invocation diagnostics should refresh across locales");
    expect(!english_actions.empty() && english_actions.front().label == english_catalog.translate(action_label_key) &&
               !spanish_actions.empty() && spanish_actions.front().label == spanish_catalog.translate(action_label_key) &&
               !pseudo_actions.empty() && pseudo_actions.front().label == pseudo_catalog.translate(action_label_key),
           "#4367: the default editor-action registry should refresh across locales without changing IDs");
}

#include "test_toolbox_creation_support.h"

namespace copperfin::toolbox_creation_tests
{
void test_toolbox_creation_maps_descriptors_and_defaults() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);

    const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Customer"},
            {.property_name = "PROPERTIES", .property_value = "ControlSource = \"customer.name\"\r\nLeft = 12\r\n"}
        }
    });

    expect(create_result.ok, "#1017: toolbox descriptor creates should succeed for known toolbox ids");
    expect(create_result.record_index == 2U,
        "#1017: toolbox descriptor creates should append the new object row");
    expect(create_result.object_name == "txt2" &&
            create_result.unique_id == "created-textbox-guid" &&
            create_result.parent_name == "frmMain",
        "#1017: toolbox descriptor creates should report generated identity metadata");

    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U,
        "#1017: toolbox descriptor creates should append exactly one object");
    if (list_result.ok && list_result.objects.size() == 3U) {
        const auto& created_object = list_result.objects[2];
        expect(created_object.object_name == "txt2",
            "#1017: created objects should carry generated object names");
        expect(created_object.unique_id == "created-textbox-guid",
            "#1017: created objects should carry unique ids");
        expect(created_object.parent_name == "frmMain",
            "#1017: created objects should carry parent names");
        expect(created_object.class_name == "TextBox",
            "#1017: created objects should carry descriptor class names");
        expect(created_object.baseclass_name == "TextBox",
            "#1017: created objects should carry descriptor baseclass names");
    }

    const auto caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "CAPTION"
    });
    expect(caption.ok && caption.exists && caption.value == "Customer",
        "#1017: toolbox descriptor creates should propagate caller-provided direct fields");

    const auto control_source = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(control_source.ok && control_source.exists && control_source.value == "\"customer.name\"",
        "#1017: toolbox descriptor creates should propagate caller-provided memo properties");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_respects_explicit_object_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_explicit_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);

    const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "commandbutton",
        .object_name = "cmdRun",
        .unique_id = "command-guid",
        .parent_name = "frmMain",
        .field_values = {}
    });

    expect(create_result.ok && create_result.object_name == "cmdRun",
        "#1017: explicit toolbox object names should take precedence over default name generation");

    const auto class_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "command-guid",
        .property_name = "CLASS"
    });
    expect(class_result.ok && class_result.exists && class_result.value == "CommandButton",
        "#1017: explicit-name creates should still map descriptor class metadata");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_rejects_unknown_toolbox_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_failure_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "missing-toolbox-item",
        .object_name = {},
        .unique_id = "should-not-exist",
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Should Not Exist"}
        }
    });

    expect(!create_result.ok,
        "#1017: unknown toolbox ids should fail cleanly");
    expect(create_result.object_name.empty() &&
            create_result.unique_id.empty() &&
            create_result.parent_name.empty(),
        "#1017: failed toolbox descriptor creates should not report stale identity metadata");
    expect(object_count(table_path) == before_count,
        "#1017: unknown toolbox ids should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_enforces_optional_context_filters() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_context_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);

    const auto label_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "label",
        .object_name = {},
        .unique_id = "report-label-guid",
        .parent_name = "DetailBand",
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Total"}
        }
    });
    expect(label_result.ok && label_result.object_name == "lbl1",
        "#1019: report-compatible toolbox items should create when report context is requested");

    const std::size_t before_rejected_count = object_count(table_path);
    const auto textbox_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "report-textbox-guid",
        .parent_name = "DetailBand",
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Should Not Exist"}
        }
    });
    expect(!textbox_result.ok,
        "#1019: report-incompatible toolbox items should fail when report context is requested");
    expect(textbox_result.object_name.empty() &&
            textbox_result.unique_id.empty() &&
            textbox_result.parent_name.empty(),
        "#1019: rejected context-filtered toolbox creates should not report stale identity metadata");
    expect(object_count(table_path) == before_rejected_count,
        "#1019: rejected context-filtered toolbox creates should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_errors_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");

    expect(english_catalog.translate("Studio.ToolboxCreation.Error.ItemNotFound") ==
               "The requested toolbox item was not found." &&
               english_catalog.translate("Studio.ToolboxCreation.Error.ItemUnavailableForContext") ==
                   "The requested toolbox item is not available in the requested designer context." &&
               english_catalog.translate("Studio.ToolboxCreation.FromDispatch.Error.AdmittedDispatchRequired") ==
                   "A toolbox create-from-dispatch request requires an admitted non-executed toolbox dispatch." &&
               english_catalog.translate("Studio.ToolboxCreation.Dispatch.Error.AdmittedCreateOperationRequired") ==
                   "A toolbox create dispatch request requires an admitted non-dry-run create operation." &&
               english_catalog.translate("Studio.ToolboxCreation.BatchDispatch.Error.DescriptorFieldValuesRequired") ==
                   "A toolbox batch create dispatch request requires descriptor field values." &&
               english_catalog.translate("Studio.ToolboxCreation.SelectionBatchDispatchCatalog.Error.PaletteRequired") ==
                   "A selection-context toolbox object batch creation dispatch catalog request requires a toolbox palette." &&
               pseudo_catalog.translate("Studio.ToolboxCreation.Error.ObjectIdentityExists").starts_with("[!! ") &&
               pseudo_catalog.translate("Studio.ToolboxCreation.BatchFromDispatch.Error.ValidatedItemMetadataRequired")
                   .starts_with("[!! "),
           "#2372: toolbox creation error prose should resolve through localizable catalog keys");
    const std::vector<std::string_view> toolbox_creation_error_keys = {
        "Studio.ToolboxCreation.Error.AssetPathRequired",
        "Studio.ToolboxCreation.Error.ItemNotFound",
        "Studio.ToolboxCreation.Error.ItemUnavailableForAdmittedDispatch",
        "Studio.ToolboxCreation.Error.ItemUnavailableForContext",
        "Studio.ToolboxCreation.Error.ObjectIdentityExists",
        "Studio.ToolboxCreation.Error.UniqueObjectNameUnavailable"};
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.Error.AssetPathRequired") ==
            "No se proporciono una ruta de asset.",
        "#2624: es-419 toolbox creation asset-path error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.Error.ItemUnavailableForContext") ==
            "El elemento solicitado de la caja de herramientas no esta disponible en el contexto de disenador solicitado.",
        "#2624: es-419 toolbox creation context-unavailable error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.Error.ItemNotFound") ==
            "O item solicitado da caixa de ferramentas nao foi encontrado.",
        "#2624: pt-BR toolbox creation item-not-found error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.Error.UniqueObjectNameUnavailable") ==
            "Nao foi possivel gerar um nome de objeto unico para o item solicitado da caixa de ferramentas.",
        "#2624: pt-BR toolbox creation unique-object-name error should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.ToolboxCreation.Error.ItemUnavailableForAdmittedDispatch") ==
            copperfin::localization::pseudo_localize(
                "The requested toolbox item is not available in the admitted toolbox dispatch."),
        "#2624: qps-ploc toolbox creation admitted-dispatch availability error should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", toolbox_creation_error_keys) == 0U,
        "#2624: es-419 should define every remaining Studio.ToolboxCreation.Error localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", toolbox_creation_error_keys) == 0U,
        "#2624: pt-BR should define every remaining Studio.ToolboxCreation.Error localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", toolbox_creation_error_keys) == 0U,
        "#2624: qps-ploc should define every remaining Studio.ToolboxCreation.Error localization key");
    const std::vector<std::string_view> toolbox_creation_dispatch_keys = {
        "Studio.ToolboxCreation.Dispatch.Error.AdmittedCreateOperationRequired",
        "Studio.ToolboxCreation.Dispatch.Error.AssetPathRequired",
        "Studio.ToolboxCreation.Dispatch.Error.DescriptorFieldValuesRequired",
        "Studio.ToolboxCreation.Dispatch.Error.PlannedObjectNameRequired",
        "Studio.ToolboxCreation.Dispatch.Error.ValidatedItemMetadataRequired"};
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.Dispatch.Error.AssetPathRequired") ==
            "Una solicitud de dispatch de creacion de caja de herramientas requiere una ruta de asset.",
        "#2626: es-419 toolbox creation dispatch asset-path error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.Dispatch.Error.PlannedObjectNameRequired") ==
            "Una solicitud de dispatch de creacion de caja de herramientas requiere un nombre planificado de objeto.",
        "#2626: es-419 toolbox creation dispatch planned-object-name error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.Dispatch.Error.AdmittedCreateOperationRequired") ==
            "Uma solicitacao de dispatch de criacao da caixa de ferramentas exige uma operacao de criacao admitida que nao seja dry-run.",
        "#2626: pt-BR toolbox creation dispatch admitted-create-operation error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.Dispatch.Error.ValidatedItemMetadataRequired") ==
            "Uma solicitacao de dispatch de criacao da caixa de ferramentas exige metadados validados de itens da caixa de ferramentas.",
        "#2626: pt-BR toolbox creation dispatch validated-item-metadata error should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.ToolboxCreation.Dispatch.Error.DescriptorFieldValuesRequired") ==
            copperfin::localization::pseudo_localize(
                "A toolbox create dispatch request requires descriptor field values."),
        "#2626: qps-ploc toolbox creation dispatch descriptor-field-values error should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", toolbox_creation_dispatch_keys) == 0U,
        "#2626: es-419 should define every remaining Studio.ToolboxCreation.Dispatch localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", toolbox_creation_dispatch_keys) == 0U,
        "#2626: pt-BR should define every remaining Studio.ToolboxCreation.Dispatch localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", toolbox_creation_dispatch_keys) == 0U,
        "#2626: qps-ploc should define every remaining Studio.ToolboxCreation.Dispatch localization key");
    const std::vector<std::string_view> batch_dispatch_keys = {
        "Studio.ToolboxCreation.BatchDispatch.Error.AdmittedCreateOperationRequired",
        "Studio.ToolboxCreation.BatchDispatch.Error.AssetPathRequired",
        "Studio.ToolboxCreation.BatchDispatch.Error.ConsistentPlannedCreatesRequired",
        "Studio.ToolboxCreation.BatchDispatch.Error.DescriptorFieldValuesRequired",
        "Studio.ToolboxCreation.BatchDispatch.Error.PlannedCreatesRequired",
        "Studio.ToolboxCreation.BatchDispatch.Error.PlannedObjectNamesRequired",
        "Studio.ToolboxCreation.BatchDispatch.Error.ValidatedItemMetadataRequired"};
    const std::vector<std::string_view> batch_from_dispatch_keys = {
        "Studio.ToolboxCreation.BatchFromDispatch.Error.AdmittedDispatchRequired",
        "Studio.ToolboxCreation.BatchFromDispatch.Error.AssetPathRequired",
        "Studio.ToolboxCreation.BatchFromDispatch.Error.ConsistentItemMetadataRequired",
        "Studio.ToolboxCreation.BatchFromDispatch.Error.ValidatedItemMetadataRequired"};
    const std::vector<std::string_view> from_dispatch_keys = {
        "Studio.ToolboxCreation.FromDispatch.Error.AdmittedDispatchRequired",
        "Studio.ToolboxCreation.FromDispatch.Error.AssetPathRequired",
        "Studio.ToolboxCreation.FromDispatch.Error.ConsistentItemMetadataRequired",
        "Studio.ToolboxCreation.FromDispatch.Error.ValidatedItemMetadataRequired"};
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.BatchDispatch.Error.AssetPathRequired") ==
            "Una solicitud de dispatch de creacion por lote de caja de herramientas requiere una ruta de asset.",
        "#2619: es-419 toolbox batch-dispatch asset-path error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.BatchDispatch.Error.PlannedObjectNamesRequired") ==
            "Una solicitud de dispatch de creacion por lote de caja de herramientas requiere nombres planificados de objetos.",
        "#2619: es-419 toolbox batch-dispatch planned-object-names error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.BatchDispatch.Error.ConsistentPlannedCreatesRequired") ==
            "Uma solicitacao de dispatch de criacao em lote da caixa de ferramentas exige criacoes planejadas consistentes da caixa de ferramentas.",
        "#2619: pt-BR toolbox batch-dispatch consistent-planned-creates error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.BatchDispatch.Error.ValidatedItemMetadataRequired") ==
            "Uma solicitacao de dispatch de criacao em lote da caixa de ferramentas exige metadados validados de itens da caixa de ferramentas.",
        "#2619: pt-BR toolbox batch-dispatch validated-item-metadata error should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.ToolboxCreation.BatchDispatch.Error.DescriptorFieldValuesRequired") ==
            copperfin::localization::pseudo_localize(
                "A toolbox batch create dispatch request requires descriptor field values."),
        "#2619: qps-ploc toolbox batch-dispatch descriptor-field-values error should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", batch_dispatch_keys) == 0U,
        "#2619: es-419 should define every remaining Studio.ToolboxCreation.BatchDispatch localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", batch_dispatch_keys) == 0U,
        "#2619: pt-BR should define every remaining Studio.ToolboxCreation.BatchDispatch localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", batch_dispatch_keys) == 0U,
        "#2619: qps-ploc should define every remaining Studio.ToolboxCreation.BatchDispatch localization key");
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.BatchFromDispatch.Error.AssetPathRequired") ==
            "Una solicitud de creacion por lote desde dispatch de caja de herramientas requiere una ruta de asset.",
        "#2630: es-419 toolbox batch-from-dispatch asset-path error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.BatchFromDispatch.Error.ConsistentItemMetadataRequired") ==
            "Una solicitud de creacion por lote desde dispatch de caja de herramientas requiere metadatos coherentes de elementos de la caja de herramientas.",
        "#2630: es-419 toolbox batch-from-dispatch consistent-item-metadata error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.BatchFromDispatch.Error.AdmittedDispatchRequired") ==
            "Uma solicitacao de criacao em lote a partir de dispatch da caixa de ferramentas exige um dispatch admitido da caixa de ferramentas que nao tenha sido executado.",
        "#2630: pt-BR toolbox batch-from-dispatch admitted-dispatch error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.BatchFromDispatch.Error.ValidatedItemMetadataRequired") ==
            "Uma solicitacao de criacao em lote a partir de dispatch da caixa de ferramentas exige metadados validados de itens da caixa de ferramentas.",
        "#2630: pt-BR toolbox batch-from-dispatch validated-item-metadata error should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.ToolboxCreation.BatchFromDispatch.Error.ConsistentItemMetadataRequired") ==
            copperfin::localization::pseudo_localize(
                "A toolbox batch create-from-dispatch request requires consistent toolbox item metadata."),
        "#2630: qps-ploc toolbox batch-from-dispatch consistent-item-metadata error should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", batch_from_dispatch_keys) == 0U,
        "#2630: es-419 should define every remaining Studio.ToolboxCreation.BatchFromDispatch localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", batch_from_dispatch_keys) == 0U,
        "#2630: pt-BR should define every remaining Studio.ToolboxCreation.BatchFromDispatch localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", batch_from_dispatch_keys) == 0U,
        "#2630: qps-ploc should define every remaining Studio.ToolboxCreation.BatchFromDispatch localization key");
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.FromDispatch.Error.AssetPathRequired") ==
            "Una solicitud de creacion desde dispatch de caja de herramientas requiere una ruta de asset.",
        "#2631: es-419 toolbox from-dispatch asset-path error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.FromDispatch.Error.ConsistentItemMetadataRequired") ==
            "Una solicitud de creacion desde dispatch de caja de herramientas requiere metadatos coherentes de elementos de la caja de herramientas.",
        "#2631: es-419 toolbox from-dispatch consistent-item-metadata error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.FromDispatch.Error.AdmittedDispatchRequired") ==
            "Uma solicitacao de criacao a partir de dispatch da caixa de ferramentas exige um dispatch admitido da caixa de ferramentas que nao tenha sido executado.",
        "#2631: pt-BR toolbox from-dispatch admitted-dispatch error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.FromDispatch.Error.ValidatedItemMetadataRequired") ==
            "Uma solicitacao de criacao a partir de dispatch da caixa de ferramentas exige metadados validados de itens da caixa de ferramentas.",
        "#2631: pt-BR toolbox from-dispatch validated-item-metadata error should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.ToolboxCreation.FromDispatch.Error.ConsistentItemMetadataRequired") ==
            copperfin::localization::pseudo_localize(
                "A toolbox create-from-dispatch request requires consistent toolbox item metadata."),
        "#2631: qps-ploc toolbox from-dispatch consistent-item-metadata error should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", from_dispatch_keys) == 0U,
        "#2631: es-419 should define every remaining Studio.ToolboxCreation.FromDispatch localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", from_dispatch_keys) == 0U,
        "#2631: pt-BR should define every remaining Studio.ToolboxCreation.FromDispatch localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", from_dispatch_keys) == 0U,
        "#2631: qps-ploc should define every remaining Studio.ToolboxCreation.FromDispatch localization key");
    const std::vector<std::string_view> batch_catalog_selection_keys = {
        "Studio.ToolboxCreation.Batch.Error.NoCreatesProvided",
        "Studio.ToolboxCreation.BatchCatalog.Error.ValidatedItemMetadataRequired",
        "Studio.ToolboxCreation.BatchDispatchCatalog.Error.ValidatedItemMetadataRequired",
        "Studio.ToolboxCreation.Catalog.Error.ValidatedItemMetadataRequired",
        "Studio.ToolboxCreation.SelectionBatchCatalog.Error.PaletteRequired",
        "Studio.ToolboxCreation.SelectionBatchDispatchCatalog.Error.PaletteRequired",
        "Studio.ToolboxCreation.SelectionBatchPlan.Error.PaletteRequired",
        "Studio.ToolboxCreation.SelectionCatalog.Error.PaletteRequired",
        "Studio.ToolboxCreation.SelectionDispatchCatalog.Error.PaletteRequired",
        "Studio.ToolboxCreation.SelectionPlan.Error.PaletteRequired"};
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.Batch.Error.NoCreatesProvided") ==
            "No se proporcionaron creaciones de objetos de caja de herramientas.",
        "#2651: es-419 toolbox batch no-creates error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ToolboxCreation.SelectionCatalog.Error.PaletteRequired") ==
            "Una solicitud de catalogo de creacion de objetos de caja de herramientas con contexto de seleccion requiere una paleta de caja de herramientas.",
        "#2651: es-419 toolbox selection catalog palette error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.BatchCatalog.Error.ValidatedItemMetadataRequired") ==
            "Uma solicitacao de catalogo de criacao em lote da caixa de ferramentas exige metadados validados de itens da caixa de ferramentas.",
        "#2651: pt-BR toolbox batch-catalog validated-item-metadata error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ToolboxCreation.SelectionBatchDispatchCatalog.Error.PaletteRequired") ==
            "Uma solicitacao de catalogo de dispatch de criacao em lote de objetos da caixa de ferramentas com contexto de selecao exige uma paleta da caixa de ferramentas.",
        "#2651: pt-BR toolbox selection batch-dispatch-catalog palette error should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.ToolboxCreation.SelectionPlan.Error.PaletteRequired") ==
            copperfin::localization::pseudo_localize(
                "A selection-context toolbox object creation plan request requires a toolbox palette."),
        "#2651: qps-ploc toolbox selection plan palette error should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", batch_catalog_selection_keys) == 0U,
        "#2651: es-419 should define every remaining Studio.ToolboxCreation batch/catalog/selection localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", batch_catalog_selection_keys) == 0U,
        "#2651: pt-BR should define every remaining Studio.ToolboxCreation batch/catalog/selection localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", batch_catalog_selection_keys) == 0U,
        "#2651: qps-ploc should define every remaining Studio.ToolboxCreation batch/catalog/selection localization key");
}

void test_toolbox_creation_default_catalog_refreshes_when_locale_changes() {
    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    locale_override.set("en-US");
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto english_result = copperfin::studio::plan_visual_object_from_toolbox_item({});
    locale_override.set("es-419");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto spanish_result = copperfin::studio::plan_visual_object_from_toolbox_item({});
    locale_override.set("qps-ploc");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto pseudo_result = copperfin::studio::plan_visual_object_from_toolbox_item({});
    constexpr std::string_view key = "Studio.ToolboxCreation.Error.AssetPathRequired";
    expect(!english_result.ok && english_result.error == english_catalog.translate(key) &&
               !spanish_result.ok && spanish_result.error == spanish_catalog.translate(key) &&
               !pseudo_result.ok && pseudo_result.error == pseudo_catalog.translate(key),
           "#4369: toolbox-creation diagnostics should refresh across locales");
}


}

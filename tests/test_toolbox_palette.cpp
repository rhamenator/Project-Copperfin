// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/studio/toolbox_palette.h"
#include "copperfin/studio/toolbox_dispatch.h"
#include "copperfin/studio/toolbox_invocation_admission.h"
#include "test_environment_support.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
}

bool has_toolbox_item(
    const std::vector<copperfin::studio::StudioToolboxItemDescriptor>& items,
    std::string_view id) {
    for (const auto& item : items) {
        if (item.id == id) {
            return true;
        }
    }
    return false;
}

const copperfin::studio::StudioToolboxItemDescriptor* find_toolbox_item(
    const std::vector<copperfin::studio::StudioToolboxItemDescriptor>& items,
    std::string_view id) {
    for (const auto& item : items) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioToolboxPaletteLaunchCatalogEntry* find_launch_catalog_entry(
    const std::vector<copperfin::studio::StudioToolboxPaletteLaunchCatalogEntry>& entries,
    copperfin::studio::StudioEditorSelectionContext context) {
    for (const auto& entry : entries) {
        if (entry.selection_context == context) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioToolboxDispatchExecutionCatalogEntry* find_execution_catalog_entry(
    const std::vector<copperfin::studio::StudioToolboxDispatchExecutionCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.item.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

bool has_argument_pair(const std::vector<std::string>& arguments, const std::string& key, const std::string& value) {
    for (std::size_t index = 0U; (index + 1U) < arguments.size(); index += 2U) {
        if (arguments[index] == key && arguments[index + 1U] == value) {
            return true;
        }
    }
    return false;
}

}  // namespace

int main() {
    using copperfin::studio::StudioEditorSelectionContext;
    using copperfin::studio::StudioToolboxContext;

    const auto& items = copperfin::studio::studio_toolbox_palette();
    expect(items.size() >= 12U, "#957: toolbox palette should expose common VFP visual controls");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::form)) == "form",
           "#957: form toolbox context token should be stable");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::class_designer)) ==
               "class_designer",
           "#957: class-designer toolbox context token should be stable");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::container)) ==
               "container",
           "#957: container toolbox context token should be stable");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::report)) == "report",
           "#957: report toolbox context token should be stable");

    bool found_container = false;
    bool found_report_item = false;
    bool found_data_control = false;

    for (const auto& item : items) {
        expect(!std::string(item.id).empty(), "#957: each toolbox item should have an id");
        expect(!std::string(item.title).empty(), "#957: each toolbox item should have a title");
        expect(!std::string(item.category).empty(), "#957: each toolbox item should have a category");
        expect(!std::string(item.vfp_class).empty(), "#957: each toolbox item should name the VFP class");
        expect(!std::string(item.base_class).empty(), "#957: each toolbox item should name the VFP base class");
        expect(!std::string(item.default_name_prefix).empty(),
               "#957: each toolbox item should provide a default object-name prefix");
        expect(!item.contexts.empty(), "#957: each toolbox item should name at least one target context");
        expect(!std::string(item.description).empty(), "#957: each toolbox item should describe its creation action");
        if (item.container) {
            found_container = true;
        }
        if (item.category == "Data Controls") {
            found_data_control = true;
        }
        for (const auto context : item.contexts) {
            if (context == StudioToolboxContext::report) {
                found_report_item = true;
            }
        }
    }

    expect(found_container, "#957: toolbox palette should identify container controls");
    expect(found_report_item, "#957: toolbox palette should include report-compatible items");
    expect(found_data_control, "#957: toolbox palette should include data controls");
    expect(has_toolbox_item(items, "label"), "#957: toolbox palette should include Label");
    expect(has_toolbox_item(items, "textbox"), "#957: toolbox palette should include TextBox");
    expect(has_toolbox_item(items, "commandbutton"), "#957: toolbox palette should include CommandButton");
    expect(has_toolbox_item(items, "combobox"), "#957: toolbox palette should include ComboBox");
    expect(has_toolbox_item(items, "grid"), "#957: toolbox palette should include Grid");
    expect(has_toolbox_item(items, "container"), "#957: toolbox palette should include Container");
    expect(has_toolbox_item(items, "pageframe"), "#957: toolbox palette should include PageFrame");

    const auto form_items = copperfin::studio::studio_toolbox_items_for_context(StudioToolboxContext::form);
    expect(has_toolbox_item(form_items, "textbox"), "#957: form context should expose TextBox");
    expect(has_toolbox_item(form_items, "pageframe"), "#957: form context should expose PageFrame");
    expect(has_toolbox_item(form_items, "olecontrol"), "#957: form context should expose OLEControl");

    const auto container_items = copperfin::studio::studio_toolbox_items_for_context(StudioToolboxContext::container);
    expect(has_toolbox_item(container_items, "checkbox"), "#957: container context should expose CheckBox");
    expect(has_toolbox_item(container_items, "grid"), "#957: container context should expose Grid");

    const auto report_items = copperfin::studio::studio_toolbox_items_for_context(StudioToolboxContext::report);
    expect(has_toolbox_item(report_items, "label"), "#957: report context should expose Label");
    expect(has_toolbox_item(report_items, "line"), "#957: report context should expose Line");
    expect(!has_toolbox_item(report_items, "textbox"), "#957: report context should exclude form-only TextBox");
    expect(!has_toolbox_item(report_items, "pageframe"), "#957: report context should exclude form-only PageFrame");

    const auto all_form_query = copperfin::studio::query_studio_toolbox_palette({
        .toolbox_context = StudioToolboxContext::form,
        .search_text = {},
        .category = {}
    });
    expect(all_form_query.ok &&
               all_form_query.toolbox_context == StudioToolboxContext::form &&
               all_form_query.item_count == form_items.size() &&
               all_form_query.items.size() == all_form_query.item_count &&
               all_form_query.dry_run &&
               !all_form_query.mutates_asset &&
               has_toolbox_item(all_form_query.items, "textbox") &&
               has_toolbox_item(all_form_query.items, "pageframe"),
           "#1411: toolbox palette queries should preserve unfiltered context items without mutation");

    const auto graphics_query = copperfin::studio::query_studio_toolbox_palette({
        .toolbox_context = StudioToolboxContext::form,
        .search_text = {},
        .category = "graphics"
    });
    expect(graphics_query.ok &&
               graphics_query.category == "graphics" &&
               has_toolbox_item(graphics_query.items, "image") &&
               has_toolbox_item(graphics_query.items, "line") &&
               has_toolbox_item(graphics_query.items, "shape") &&
               !has_toolbox_item(graphics_query.items, "textbox"),
           "#1411: toolbox palette queries should filter categories case-insensitively");

    const auto text_query = copperfin::studio::query_studio_toolbox_palette({
        .toolbox_context = StudioToolboxContext::form,
        .search_text = "rowsource",
        .category = {}
    });
    expect(text_query.ok &&
               text_query.search_text == "rowsource" &&
               has_toolbox_item(text_query.items, "combobox") &&
               has_toolbox_item(text_query.items, "listbox") &&
               !has_toolbox_item(text_query.items, "pageframe"),
           "#1411: toolbox palette queries should search descriptor text case-insensitively");

    const auto combined_query = copperfin::studio::query_studio_toolbox_palette({
        .toolbox_context = StudioToolboxContext::form,
        .search_text = "command",
        .category = "Standard Controls"
    });
    expect(combined_query.ok &&
               combined_query.item_count == 1U &&
               has_toolbox_item(combined_query.items, "commandbutton"),
           "#1411: toolbox palette queries should combine category and search filters");

    const auto report_textbox_query = copperfin::studio::query_studio_toolbox_palette({
        .toolbox_context = StudioToolboxContext::report,
        .search_text = "textbox",
        .category = {}
    });
    expect(report_textbox_query.ok &&
               report_textbox_query.item_count == 0U &&
               report_textbox_query.items.empty(),
           "#1411: toolbox palette queries should return empty results when context filters remove matches");

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto spanish_toolbox = copperfin::studio::studio_toolbox_palette_for_catalog(spanish_catalog);
    const auto portuguese_toolbox = copperfin::studio::studio_toolbox_palette_for_catalog(portuguese_catalog);
    const auto pseudo_toolbox = copperfin::studio::studio_toolbox_palette_for_catalog(pseudo_catalog);
    const std::vector<std::string_view> text_item_keys = {
        "Studio.Toolbox.Item.Label.Description",
        "Studio.Toolbox.Item.Label.Title",
        "Studio.Toolbox.Item.TextBox.Description",
        "Studio.Toolbox.Item.TextBox.Title",
        "Studio.Toolbox.Item.EditBox.Description",
        "Studio.Toolbox.Item.EditBox.Title"};
    const std::vector<std::string_view> interactive_item_keys = {
        "Studio.Toolbox.Item.CheckBox.Description",
        "Studio.Toolbox.Item.CheckBox.Title",
        "Studio.Toolbox.Item.ComboBox.Description",
        "Studio.Toolbox.Item.ComboBox.Title",
        "Studio.Toolbox.Item.CommandButton.Description",
        "Studio.Toolbox.Item.CommandButton.Title",
        "Studio.Toolbox.Item.ListBox.Description",
        "Studio.Toolbox.Item.ListBox.Title"};
    const std::vector<std::string_view> container_data_item_keys = {
        "Studio.Toolbox.Item.Container.Description",
        "Studio.Toolbox.Item.Container.Title",
        "Studio.Toolbox.Item.Grid.Description",
        "Studio.Toolbox.Item.Grid.Title"};
    const std::vector<std::string_view> graphics_item_keys = {
        "Studio.Toolbox.Item.Image.Description",
        "Studio.Toolbox.Item.Image.Title",
        "Studio.Toolbox.Item.Line.Description",
        "Studio.Toolbox.Item.Line.Title",
        "Studio.Toolbox.Item.Shape.Description",
        "Studio.Toolbox.Item.Shape.Title"};
    const std::vector<std::string_view> frame_interop_item_keys = {
        "Studio.Toolbox.Item.OLEControl.Description",
        "Studio.Toolbox.Item.OLEControl.Title",
        "Studio.Toolbox.Item.PageFrame.Description",
        "Studio.Toolbox.Item.PageFrame.Title"};
    const auto* spanish_label = find_toolbox_item(spanish_toolbox, "label");
    const auto* spanish_checkbox = find_toolbox_item(spanish_toolbox, "checkbox");
    const auto* spanish_container = find_toolbox_item(spanish_toolbox, "container");
    const auto* spanish_image = find_toolbox_item(spanish_toolbox, "image");
    const auto* portuguese_textbox = find_toolbox_item(portuguese_toolbox, "textbox");
    const auto* portuguese_commandbutton = find_toolbox_item(portuguese_toolbox, "commandbutton");
    const auto* portuguese_grid = find_toolbox_item(portuguese_toolbox, "grid");
    const auto* portuguese_shape = find_toolbox_item(portuguese_toolbox, "shape");
    const auto* portuguese_pageframe = find_toolbox_item(portuguese_toolbox, "pageframe");
    const auto* pseudo_editbox = find_toolbox_item(pseudo_toolbox, "editbox");
    const auto* pseudo_combobox = find_toolbox_item(pseudo_toolbox, "combobox");
    const auto* pseudo_container = find_toolbox_item(pseudo_toolbox, "container");
    const auto* pseudo_line = find_toolbox_item(pseudo_toolbox, "line");
    const auto* pseudo_olecontrol = find_toolbox_item(pseudo_toolbox, "olecontrol");
    const auto* spanish_olecontrol = find_toolbox_item(spanish_toolbox, "olecontrol");
    expect(
        spanish_label != nullptr &&
            spanish_label->title == "Etiqueta" &&
            spanish_label->description ==
                "Mostrar texto estatico o titulos de reportes con semantica de Label de VFP." &&
            spanish_label->id == "label" &&
            spanish_label->vfp_class == "Label" &&
            spanish_label->base_class == "Label" &&
            spanish_label->default_name_prefix == "lbl",
        "#2633: es-419 label toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        portuguese_textbox != nullptr &&
            portuguese_textbox->title == "Caixa de texto" &&
            portuguese_textbox->description ==
                "Editar valores de caracteres, numericos, datas e campos vinculados." &&
            portuguese_textbox->id == "textbox" &&
            portuguese_textbox->vfp_class == "TextBox" &&
            portuguese_textbox->base_class == "TextBox" &&
            portuguese_textbox->default_name_prefix == "txt",
        "#2633: pt-BR textbox toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        pseudo_editbox != nullptr &&
            pseudo_editbox->title == copperfin::localization::pseudo_localize("EditBox") &&
            pseudo_editbox->description ==
                copperfin::localization::pseudo_localize("Edit memo and multi-line text values.") &&
            pseudo_editbox->id == "editbox" &&
            pseudo_editbox->vfp_class == "EditBox" &&
            pseudo_editbox->base_class == "EditBox" &&
            pseudo_editbox->default_name_prefix == "edt",
        "#2633: qps-ploc editbox toolbox metadata should resolve through the pseudo-localization transform");
    expect(
        spanish_checkbox != nullptr &&
            spanish_checkbox->title == "Casilla de verificacion" &&
            spanish_checkbox->description ==
                "Editar valores logicos con comportamiento de CheckBox de VFP." &&
            spanish_checkbox->id == "checkbox" &&
            spanish_checkbox->vfp_class == "CheckBox" &&
            spanish_checkbox->base_class == "CheckBox" &&
            spanish_checkbox->default_name_prefix == "chk",
        "#2634: es-419 checkbox toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        portuguese_commandbutton != nullptr &&
            portuguese_commandbutton->title == "Botao de comando" &&
            portuguese_commandbutton->description ==
                "Executar acoes de clique e metodos de comando." &&
            portuguese_commandbutton->id == "commandbutton" &&
            portuguese_commandbutton->vfp_class == "CommandButton" &&
            portuguese_commandbutton->base_class == "CommandButton" &&
            portuguese_commandbutton->default_name_prefix == "cmd",
        "#2634: pt-BR commandbutton toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        pseudo_combobox != nullptr &&
            pseudo_combobox->title == copperfin::localization::pseudo_localize("ComboBox") &&
            pseudo_combobox->description ==
                copperfin::localization::pseudo_localize("Pick or enter values from RowSource-driven lists.") &&
            pseudo_combobox->id == "combobox" &&
            pseudo_combobox->vfp_class == "ComboBox" &&
            pseudo_combobox->base_class == "ComboBox" &&
            pseudo_combobox->default_name_prefix == "cbo",
        "#2634: qps-ploc combobox toolbox metadata should resolve through the pseudo-localization transform");
    expect(
        spanish_container != nullptr &&
            spanish_container->title == "Contenedor" &&
            spanish_container->description ==
                "Agrupar controles anidados bajo la semantica de Container de VFP." &&
            spanish_container->id == "container" &&
            spanish_container->vfp_class == "Container" &&
            spanish_container->base_class == "Container" &&
            spanish_container->default_name_prefix == "cnt" &&
            spanish_container->container,
        "#2635: es-419 container toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        portuguese_grid != nullptr &&
            portuguese_grid->title == "Grade" &&
            portuguese_grid->description ==
                "Exibir e editar linhas de cursores ou tabelas com a semantica de colunas de Grid do VFP." &&
            portuguese_grid->id == "grid" &&
            portuguese_grid->vfp_class == "Grid" &&
            portuguese_grid->base_class == "Grid" &&
            portuguese_grid->default_name_prefix == "grd" &&
            portuguese_grid->container,
        "#2635: pt-BR grid toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        pseudo_container != nullptr &&
            pseudo_container->title == copperfin::localization::pseudo_localize("Container") &&
            pseudo_container->description ==
                copperfin::localization::pseudo_localize("Group nested controls under VFP Container semantics.") &&
            pseudo_container->id == "container" &&
            pseudo_container->vfp_class == "Container" &&
            pseudo_container->base_class == "Container" &&
            pseudo_container->default_name_prefix == "cnt" &&
            pseudo_container->container,
        "#2635: qps-ploc container toolbox metadata should resolve through the pseudo-localization transform");
    expect(
        spanish_image != nullptr &&
            spanish_image->title == "Imagen" &&
            spanish_image->description == "Mostrar mapas de bits y assets de imagen vinculados." &&
            spanish_image->id == "image" &&
            spanish_image->vfp_class == "Image" &&
            spanish_image->base_class == "Image" &&
            spanish_image->default_name_prefix == "img" &&
            !spanish_image->container,
        "#2636: es-419 image toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        portuguese_shape != nullptr &&
            portuguese_shape->title == "Forma" &&
            portuguese_shape->description ==
                "Desenhar retangulos, retangulos arredondados e outras variantes de formas do VFP." &&
            portuguese_shape->id == "shape" &&
            portuguese_shape->vfp_class == "Shape" &&
            portuguese_shape->base_class == "Shape" &&
            portuguese_shape->default_name_prefix == "shp" &&
            !portuguese_shape->container,
        "#2636: pt-BR shape toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        pseudo_line != nullptr &&
            pseudo_line->title == copperfin::localization::pseudo_localize("Line") &&
            pseudo_line->description ==
                copperfin::localization::pseudo_localize("Draw VFP-compatible line shapes.") &&
            pseudo_line->id == "line" &&
            pseudo_line->vfp_class == "Line" &&
            pseudo_line->base_class == "Line" &&
            pseudo_line->default_name_prefix == "lin" &&
            !pseudo_line->container,
        "#2636: qps-ploc line toolbox metadata should resolve through the pseudo-localization transform");
    expect(
        spanish_olecontrol != nullptr &&
            spanish_olecontrol->title == "Control OLE" &&
            spanish_olecontrol->description ==
                "Representar placeholders de controles OLE de VFP para una edicion centrada en la compatibilidad." &&
            spanish_olecontrol->id == "olecontrol" &&
            spanish_olecontrol->vfp_class == "OLEControl" &&
            spanish_olecontrol->base_class == "OLEControl" &&
            spanish_olecontrol->default_name_prefix == "ole" &&
            !spanish_olecontrol->container,
        "#2637: es-419 OLE control toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        portuguese_pageframe != nullptr &&
            portuguese_pageframe->title == "Quadro de paginas" &&
            portuguese_pageframe->description == "Hospedar paginas com abas e controles aninhados." &&
            portuguese_pageframe->id == "pageframe" &&
            portuguese_pageframe->vfp_class == "PageFrame" &&
            portuguese_pageframe->base_class == "PageFrame" &&
            portuguese_pageframe->default_name_prefix == "pgf" &&
            portuguese_pageframe->container,
        "#2637: pt-BR pageframe toolbox metadata should localize through the palette without changing invariant fields");
    expect(
        pseudo_olecontrol != nullptr &&
            pseudo_olecontrol->title == copperfin::localization::pseudo_localize("OLEControl") &&
            pseudo_olecontrol->description ==
                copperfin::localization::pseudo_localize(
                    "Represent VFP OLE control placeholders for compatibility-focused editing.") &&
            pseudo_olecontrol->id == "olecontrol" &&
            pseudo_olecontrol->vfp_class == "OLEControl" &&
            pseudo_olecontrol->base_class == "OLEControl" &&
            pseudo_olecontrol->default_name_prefix == "ole" &&
            !pseudo_olecontrol->container,
        "#2637: qps-ploc OLE control toolbox metadata should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", text_item_keys) == 0U,
        "#2633: es-419 should define every remaining toolbox text-item localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", text_item_keys) == 0U,
        "#2633: pt-BR should define every remaining toolbox text-item localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", text_item_keys) == 0U,
        "#2633: qps-ploc should define every remaining toolbox text-item localization key");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", interactive_item_keys) == 0U,
        "#2634: es-419 should define every remaining toolbox interactive-item localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", interactive_item_keys) == 0U,
        "#2634: pt-BR should define every remaining toolbox interactive-item localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", interactive_item_keys) == 0U,
        "#2634: qps-ploc should define every remaining toolbox interactive-item localization key");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", container_data_item_keys) == 0U,
        "#2635: es-419 should define every remaining toolbox container/data-item localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", container_data_item_keys) == 0U,
        "#2635: pt-BR should define every remaining toolbox container/data-item localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", container_data_item_keys) == 0U,
        "#2635: qps-ploc should define every remaining toolbox container/data-item localization key");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", graphics_item_keys) == 0U,
        "#2636: es-419 should define every remaining toolbox graphics-item localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", graphics_item_keys) == 0U,
        "#2636: pt-BR should define every remaining toolbox graphics-item localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", graphics_item_keys) == 0U,
        "#2636: qps-ploc should define every remaining toolbox graphics-item localization key");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", frame_interop_item_keys) == 0U,
        "#2637: es-419 should define every remaining toolbox frame/interop localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", frame_interop_item_keys) == 0U,
        "#2637: pt-BR should define every remaining toolbox frame/interop localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", frame_interop_item_keys) == 0U,
        "#2637: qps-ploc should define every remaining toolbox frame/interop localization key");

    const auto visual_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid"
    });
    expect(visual_plan.ok,
        "#1209: visual-object selection contexts should plan toolbox palettes");
    expect(visual_plan.plan.selection_context == StudioEditorSelectionContext::visual_object &&
            visual_plan.plan.toolbox_context == StudioToolboxContext::form &&
            visual_plan.plan.asset_path == "forms/customer.scx" &&
            visual_plan.plan.record_index == 1U &&
            visual_plan.plan.object_name == "frmCustomer" &&
            visual_plan.plan.unique_id == "form-guid" &&
            visual_plan.plan.item_count == visual_plan.plan.items.size() &&
            has_toolbox_item(visual_plan.plan.items, "textbox") &&
            has_toolbox_item(visual_plan.plan.items, "pageframe"),
        "#1209: toolbox palette plans should preserve visual selection metadata and form items");

    const auto container_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::container_object,
        .asset_path = "forms/customer.scx",
        .record_index = 2U,
        .object_name = "pgAddress",
        .unique_id = "page-guid"
    });
    expect(container_plan.ok &&
            container_plan.plan.toolbox_context == StudioToolboxContext::container &&
            has_toolbox_item(container_plan.plan.items, "checkbox") &&
            has_toolbox_item(container_plan.plan.items, "grid"),
        "#1209: container selection contexts should plan container-safe toolbox palettes");

    const auto class_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::class_designer,
        .asset_path = "classes/controls.vcx",
        .record_index = 0U,
        .object_name = "txtBase",
        .unique_id = "class-guid"
    });
    expect(class_plan.ok &&
            class_plan.plan.toolbox_context == StudioToolboxContext::class_designer &&
            has_toolbox_item(class_plan.plan.items, "textbox"),
        "#1209: class-designer selection contexts should plan class-safe toolbox palettes");

    const auto report_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::report_expression,
        .asset_path = "reports/orders.frx",
        .record_index = 3U,
        .object_name = "Field1",
        .unique_id = "field-guid"
    });
    expect(report_plan.ok &&
            report_plan.plan.toolbox_context == StudioToolboxContext::report &&
            has_toolbox_item(report_plan.plan.items, "label") &&
            !has_toolbox_item(report_plan.plan.items, "textbox"),
        "#1209: report selection contexts should plan report-safe toolbox palettes");

    const auto label_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::label_expression,
        .asset_path = "labels/mailing.lbx",
        .record_index = 0U,
        .object_name = "Label1",
        .unique_id = "label-guid"
    });
    expect(label_plan.ok &&
            label_plan.plan.toolbox_context == StudioToolboxContext::report &&
            has_toolbox_item(label_plan.plan.items, "label") &&
            !has_toolbox_item(label_plan.plan.items, "pageframe"),
        "#1209: label selection contexts should reuse report-safe toolbox palettes");

    const auto menu_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::menu_item,
        .asset_path = "menus/main.mnx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(!menu_plan.ok,
        "#1209: menu selection contexts should reject toolbox palette launch planning");

    const auto project_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::project_item,
        .asset_path = "apps/customer.pjx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(!project_plan.ok,
        "#1209: project selection contexts should reject toolbox palette launch planning");

    const auto data_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::data_environment,
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = "Dataenvironment",
        .unique_id = "de-guid"
    });
    expect(!data_plan.ok,
        "#1209: data-environment selection contexts should reject toolbox palette launch planning");

    const auto launch_catalog = copperfin::studio::plan_studio_toolbox_palette_launch_catalog({
        .asset_path = "forms/customer.scx",
        .record_index = 4U,
        .object_name = "cmdSave",
        .unique_id = "button-guid"
    });
    const auto* catalog_visual = find_launch_catalog_entry(
        launch_catalog.entries, StudioEditorSelectionContext::visual_object);
    const auto* catalog_container = find_launch_catalog_entry(
        launch_catalog.entries, StudioEditorSelectionContext::container_object);
    const auto* catalog_report = find_launch_catalog_entry(
        launch_catalog.entries, StudioEditorSelectionContext::report_expression);
    const auto* catalog_menu = find_launch_catalog_entry(
        launch_catalog.entries, StudioEditorSelectionContext::menu_item);
    const auto* catalog_project = find_launch_catalog_entry(
        launch_catalog.entries, StudioEditorSelectionContext::project_item);
    const auto* catalog_data = find_launch_catalog_entry(
        launch_catalog.entries, StudioEditorSelectionContext::data_environment);
    expect(launch_catalog.ok &&
            launch_catalog.context_count == 9U &&
            launch_catalog.entries.size() == launch_catalog.context_count &&
            launch_catalog.launch_plan_count == 6U &&
            launch_catalog.error_count == 3U &&
            launch_catalog.dry_run &&
            !launch_catalog.mutates_asset,
        "#1316: toolbox palette launch catalogs should summarize every known selection context");
    expect(catalog_visual != nullptr &&
            catalog_visual->toolbox_available &&
            catalog_visual->item_count == catalog_visual->launch_plan.plan.item_count &&
            catalog_visual->launch_plan.ok &&
            catalog_visual->launch_plan.plan.selection_context == StudioEditorSelectionContext::visual_object &&
            catalog_visual->launch_plan.plan.toolbox_context == StudioToolboxContext::form &&
            catalog_visual->launch_plan.plan.asset_path == "forms/customer.scx" &&
            catalog_visual->launch_plan.plan.record_index == 4U &&
            catalog_visual->launch_plan.plan.object_name == "cmdSave" &&
            catalog_visual->launch_plan.plan.unique_id == "button-guid" &&
            has_toolbox_item(catalog_visual->launch_plan.plan.items, "textbox"),
        "#1316: visual-object toolbox palette launch catalog entries should preserve form metadata");
    expect(catalog_container != nullptr &&
            catalog_container->toolbox_available &&
            catalog_container->launch_plan.ok &&
            catalog_container->launch_plan.plan.toolbox_context == StudioToolboxContext::container &&
            has_toolbox_item(catalog_container->launch_plan.plan.items, "grid"),
        "#1316: container toolbox palette launch catalog entries should preserve container filtering");
    expect(catalog_report != nullptr &&
            catalog_report->toolbox_available &&
            catalog_report->launch_plan.ok &&
            catalog_report->launch_plan.plan.toolbox_context == StudioToolboxContext::report &&
            has_toolbox_item(catalog_report->launch_plan.plan.items, "label") &&
            !has_toolbox_item(catalog_report->launch_plan.plan.items, "textbox"),
        "#1316: report toolbox palette launch catalog entries should preserve report filtering");
    expect(catalog_menu != nullptr &&
            catalog_project != nullptr &&
            catalog_data != nullptr &&
            !catalog_menu->toolbox_available &&
            !catalog_project->toolbox_available &&
            !catalog_data->toolbox_available &&
            catalog_menu->item_count == 0U &&
            catalog_project->item_count == 0U &&
            catalog_data->item_count == 0U &&
            !catalog_menu->launch_plan.ok &&
            !catalog_project->launch_plan.ok &&
            !catalog_data->launch_plan.ok &&
            catalog_menu->error == "The selected Studio context does not expose a toolbox palette." &&
            catalog_project->error == "The selected Studio context does not expose a toolbox palette." &&
            catalog_data->error == "The selected Studio context does not expose a toolbox palette.",
        "#1316: unsupported toolbox palette launch catalog entries should report deterministic errors");

    const auto admitted_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = visual_plan.plan,
        .admit_palette_invocation = true
    });
    expect(admitted_invocation.ok,
        "#1219: toolbox invocation admission should accept validated launch plans");
    expect(admitted_invocation.plan.selection_context == StudioEditorSelectionContext::visual_object &&
            admitted_invocation.plan.toolbox_context == StudioToolboxContext::form &&
            admitted_invocation.plan.command_token == "studio.toolbox.palette.invoke" &&
            admitted_invocation.plan.asset_path == "forms/customer.scx" &&
            admitted_invocation.plan.record_index == 1U &&
            admitted_invocation.plan.object_name == "frmCustomer" &&
            admitted_invocation.plan.unique_id == "form-guid" &&
            admitted_invocation.plan.item_count == visual_plan.plan.item_count &&
            admitted_invocation.plan.items.size() == visual_plan.plan.items.size() &&
            admitted_invocation.plan.palette_invocation_admitted &&
            !admitted_invocation.plan.dry_run &&
            !admitted_invocation.plan.mutates_asset &&
            has_toolbox_item(admitted_invocation.plan.items, "textbox"),
        "#1219: toolbox invocation admission should preserve palette metadata and admitted state");

    const auto toolbox_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = admitted_invocation.plan
    });
    expect(toolbox_dispatch.ok,
        "#1233: toolbox dispatch should accept admitted toolbox palette invocations");
    expect(toolbox_dispatch.plan.selection_context == StudioEditorSelectionContext::visual_object &&
            toolbox_dispatch.plan.toolbox_context == StudioToolboxContext::form &&
            toolbox_dispatch.plan.command_token == "studio.toolbox.palette.invoke" &&
            toolbox_dispatch.plan.asset_path == "forms/customer.scx" &&
            toolbox_dispatch.plan.record_index == 1U &&
            toolbox_dispatch.plan.object_name == "frmCustomer" &&
            toolbox_dispatch.plan.unique_id == "form-guid" &&
            toolbox_dispatch.plan.item_count == visual_plan.plan.item_count &&
            toolbox_dispatch.plan.items.size() == visual_plan.plan.items.size() &&
            toolbox_dispatch.plan.dispatch_admitted &&
            !toolbox_dispatch.plan.dry_run &&
            !toolbox_dispatch.plan.executed &&
            !toolbox_dispatch.plan.mutates_asset &&
            has_toolbox_item(toolbox_dispatch.plan.items, "textbox"),
        "#1233: toolbox dispatch should preserve palette admission metadata without executing");
    expect(has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--command-token", "studio.toolbox.palette.invoke") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--selection-context", "visual_object") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--path", "forms/customer.scx") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--record", "1") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--object-name", "frmCustomer") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--unique-id", "form-guid") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--item-count",
                std::to_string(visual_plan.plan.item_count)),
        "#1233: toolbox dispatch should materialize a deterministic argument contract");

    bool toolbox_executor_called = false;
    const auto executed_toolbox_dispatch = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = toolbox_dispatch.plan,
        .admit_execution = true,
        .executor = [&](const copperfin::studio::StudioToolboxDispatchPlan& plan) {
            toolbox_executor_called = true;
            expect(plan.selection_context == StudioEditorSelectionContext::visual_object &&
                    plan.toolbox_context == StudioToolboxContext::form &&
                    plan.command_token == "studio.toolbox.palette.invoke" &&
                    plan.item_count == visual_plan.plan.item_count &&
                    has_toolbox_item(plan.items, "textbox") &&
                    has_argument_pair(plan.dispatch_arguments, "--toolbox-context", "form") &&
                    has_argument_pair(plan.dispatch_arguments, "--item-count",
                        std::to_string(visual_plan.plan.item_count)),
                "#1322: toolbox dispatch execution should invoke executors with validated dispatch metadata");
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = "toolbox palette launched",
                .error = {},
                .mutates_asset = true
            };
        }
    });
    expect(toolbox_executor_called &&
            executed_toolbox_dispatch.ok &&
            executed_toolbox_dispatch.execution_admitted &&
            executed_toolbox_dispatch.executed &&
            !executed_toolbox_dispatch.dry_run &&
            executed_toolbox_dispatch.mutates_asset &&
            executed_toolbox_dispatch.observation.launched &&
            executed_toolbox_dispatch.observation.exit_code == 0 &&
            executed_toolbox_dispatch.observation.output == "toolbox palette launched" &&
            executed_toolbox_dispatch.dispatch_plan.executed &&
            executed_toolbox_dispatch.dispatch_plan.dispatch_admitted &&
            !executed_toolbox_dispatch.dispatch_plan.dry_run &&
            executed_toolbox_dispatch.dispatch_plan.command_token == "studio.toolbox.palette.invoke" &&
            executed_toolbox_dispatch.dispatch_plan.item_count == visual_plan.plan.item_count &&
            has_toolbox_item(executed_toolbox_dispatch.dispatch_plan.items, "textbox"),
        "#1322: toolbox dispatch execution should preserve dispatch metadata and executed state");

    toolbox_executor_called = false;
    const auto unadmitted_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = toolbox_dispatch.plan,
        .admit_execution = false,
        .executor = [&](const copperfin::studio::StudioToolboxDispatchPlan&) {
            toolbox_executor_called = true;
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .output = {},
                .error = {}
            };
        }
    });
    expect(!toolbox_executor_called &&
            !unadmitted_execution.ok &&
            unadmitted_execution.error ==
                "A toolbox dispatch execution request requires explicit execution admission." &&
            !unadmitted_execution.executed &&
            unadmitted_execution.dry_run,
        "#1322: toolbox dispatch execution should reject unadmitted execution without invoking executors");

    const auto missing_executor_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = toolbox_dispatch.plan,
        .admit_execution = true,
        .executor = {}
    });
    expect(!missing_executor_execution.ok &&
            missing_executor_execution.error == "A toolbox dispatch execution request requires an executor.",
        "#1322: toolbox dispatch execution should reject missing executors");

    auto stale_dispatch_plan = toolbox_dispatch.plan;
    stale_dispatch_plan.executed = true;
    toolbox_executor_called = false;
    const auto stale_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = stale_dispatch_plan,
        .admit_execution = true,
        .executor = [&](const copperfin::studio::StudioToolboxDispatchPlan&) {
            toolbox_executor_called = true;
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .output = {},
                .error = {}
            };
        }
    });
    expect(!toolbox_executor_called &&
            !stale_execution.ok &&
            stale_execution.error == "A toolbox dispatch execution request requires a non-executed dispatch.",
        "#1322: toolbox dispatch execution should reject stale executed dispatches");

    auto missing_arguments_plan = toolbox_dispatch.plan;
    missing_arguments_plan.dispatch_arguments.clear();
    toolbox_executor_called = false;
    const auto missing_arguments_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = missing_arguments_plan,
        .admit_execution = true,
        .executor = [&](const copperfin::studio::StudioToolboxDispatchPlan&) {
            toolbox_executor_called = true;
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .output = {},
                .error = {}
            };
        }
    });
    expect(!toolbox_executor_called &&
            !missing_arguments_execution.ok &&
            missing_arguments_execution.error == "A toolbox dispatch execution request requires dispatch arguments.",
        "#1322: toolbox dispatch execution should reject missing dispatch arguments before launch");

    auto missing_items_execution_plan = toolbox_dispatch.plan;
    missing_items_execution_plan.items.clear();
    missing_items_execution_plan.item_count = 0U;
    toolbox_executor_called = false;
    const auto missing_items_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = missing_items_execution_plan,
        .admit_execution = true,
        .executor = [&](const copperfin::studio::StudioToolboxDispatchPlan&) {
            toolbox_executor_called = true;
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .output = {},
                .error = {}
            };
        }
    });
    expect(!toolbox_executor_called &&
            !missing_items_execution.ok &&
            missing_items_execution.error ==
                "A toolbox dispatch execution request requires validated toolbox item metadata.",
        "#1322: toolbox dispatch execution should reject missing item metadata before launch");

    const auto launch_failure_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = toolbox_dispatch.plan,
        .admit_execution = true,
        .executor = [](const copperfin::studio::StudioToolboxDispatchPlan&) {
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = false,
                .exit_code = 0,
                .output = {},
                .error = "toolbox launcher unavailable",
                .mutates_asset = false
            };
        }
    });
    expect(!launch_failure_execution.ok &&
            launch_failure_execution.error == "toolbox launcher unavailable" &&
            !launch_failure_execution.executed &&
            launch_failure_execution.dry_run &&
            launch_failure_execution.observation.error == "toolbox launcher unavailable",
        "#1322: toolbox dispatch execution should surface launch failures without stale execution metadata");

    const auto non_zero_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = toolbox_dispatch.plan,
        .admit_execution = true,
        .executor = [](const copperfin::studio::StudioToolboxDispatchPlan&) {
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .exit_code = 9,
                .output = {},
                .error = "toolbox failed",
                .mutates_asset = false
            };
        }
    });
    expect(!non_zero_execution.ok &&
            non_zero_execution.error == "toolbox failed" &&
            non_zero_execution.observation.launched &&
            non_zero_execution.observation.exit_code == 9 &&
            !non_zero_execution.executed &&
            non_zero_execution.dry_run,
        "#1322: toolbox dispatch execution should reject non-zero executor exit codes");

    const auto dry_run_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = report_plan.plan,
        .admit_palette_invocation = false
    });
    expect(dry_run_invocation.ok &&
            dry_run_invocation.plan.selection_context == StudioEditorSelectionContext::report_expression &&
            dry_run_invocation.plan.toolbox_context == StudioToolboxContext::report &&
            !dry_run_invocation.plan.palette_invocation_admitted &&
            dry_run_invocation.plan.dry_run &&
            !dry_run_invocation.plan.mutates_asset &&
            has_toolbox_item(dry_run_invocation.plan.items, "label") &&
            !has_toolbox_item(dry_run_invocation.plan.items, "textbox"),
        "#1219: toolbox invocation admission should default to dry-run and preserve filtered report items");

    const auto form_invocation_catalog = copperfin::studio::plan_studio_toolbox_invocation_admission_catalog({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .toolbox_context = StudioToolboxContext::form,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .admit_palette_invocation = true
    });
    expect(form_invocation_catalog.ok &&
            form_invocation_catalog.selection_context == StudioEditorSelectionContext::visual_object &&
            form_invocation_catalog.toolbox_context == StudioToolboxContext::form &&
            form_invocation_catalog.command_token == "studio.toolbox.palette.invoke" &&
            form_invocation_catalog.asset_path == "forms/customer.scx" &&
            form_invocation_catalog.record_index == 1U &&
            form_invocation_catalog.object_name == "frmCustomer" &&
            form_invocation_catalog.unique_id == "form-guid" &&
            form_invocation_catalog.item_count == form_items.size() &&
            form_invocation_catalog.items.size() == form_items.size() &&
            form_invocation_catalog.admission_count == 1U &&
            form_invocation_catalog.error_count == 0U &&
            !form_invocation_catalog.dry_run &&
            !form_invocation_catalog.mutates_asset &&
            form_invocation_catalog.invocation_admission.ok &&
            form_invocation_catalog.invocation_admission.plan.palette_invocation_admitted &&
            has_toolbox_item(form_invocation_catalog.items, "textbox"),
        "#1285: admitted toolbox invocation admission catalogs should preserve form item metadata");

    const auto report_invocation_catalog = copperfin::studio::plan_studio_toolbox_invocation_admission_catalog({
        .selection_context = StudioEditorSelectionContext::report_expression,
        .toolbox_context = StudioToolboxContext::report,
        .asset_path = "reports/orders.frx",
        .record_index = 3U,
        .object_name = "Field1",
        .unique_id = "field-guid",
        .admit_palette_invocation = false
    });
    expect(report_invocation_catalog.ok &&
            report_invocation_catalog.selection_context == StudioEditorSelectionContext::report_expression &&
            report_invocation_catalog.toolbox_context == StudioToolboxContext::report &&
            report_invocation_catalog.item_count == report_items.size() &&
            report_invocation_catalog.admission_count == 1U &&
            report_invocation_catalog.error_count == 0U &&
            report_invocation_catalog.dry_run &&
            !report_invocation_catalog.mutates_asset &&
            report_invocation_catalog.invocation_admission.ok &&
            !report_invocation_catalog.invocation_admission.plan.palette_invocation_admitted &&
            has_toolbox_item(report_invocation_catalog.items, "label") &&
            !has_toolbox_item(report_invocation_catalog.items, "textbox"),
        "#1285: dry-run toolbox invocation admission catalogs should preserve report-safe item metadata");

    const auto empty_invocation_catalog = copperfin::studio::plan_studio_toolbox_invocation_admission_catalog({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .toolbox_context = static_cast<StudioToolboxContext>(999),
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .admit_palette_invocation = true
    });
    expect(!empty_invocation_catalog.ok &&
            empty_invocation_catalog.error ==
                "A toolbox invocation admission catalog request requires validated toolbox item metadata." &&
            empty_invocation_catalog.item_count == 0U &&
            empty_invocation_catalog.admission_count == 0U &&
            empty_invocation_catalog.error_count == 0U &&
            empty_invocation_catalog.dry_run &&
            !empty_invocation_catalog.mutates_asset,
        "#1285: toolbox invocation admission catalogs should reject empty item metadata");

    const auto visual_selection_invocation_catalog =
        copperfin::studio::plan_studio_toolbox_invocation_admission_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid",
            .admit_palette_invocation = true
        });
    expect(visual_selection_invocation_catalog.ok &&
            visual_selection_invocation_catalog.selection_context == StudioEditorSelectionContext::visual_object &&
            visual_selection_invocation_catalog.toolbox_context == StudioToolboxContext::form &&
            visual_selection_invocation_catalog.command_token == "studio.toolbox.palette.invoke" &&
            visual_selection_invocation_catalog.asset_path == "forms/customer.scx" &&
            visual_selection_invocation_catalog.record_index == 1U &&
            visual_selection_invocation_catalog.object_name == "frmCustomer" &&
            visual_selection_invocation_catalog.unique_id == "form-guid" &&
            visual_selection_invocation_catalog.item_count == form_items.size() &&
            visual_selection_invocation_catalog.items.size() == form_items.size() &&
            visual_selection_invocation_catalog.launch_plan.ok &&
            visual_selection_invocation_catalog.launch_plan.plan.toolbox_context == StudioToolboxContext::form &&
            visual_selection_invocation_catalog.invocation_admission.ok &&
            visual_selection_invocation_catalog.invocation_admission.plan.palette_invocation_admitted &&
            visual_selection_invocation_catalog.admission_count == 1U &&
            visual_selection_invocation_catalog.error_count == 0U &&
            !visual_selection_invocation_catalog.dry_run &&
            !visual_selection_invocation_catalog.mutates_asset &&
            has_toolbox_item(visual_selection_invocation_catalog.items, "textbox"),
        "#1288: visual selection toolbox admission catalogs should resolve form toolbox metadata");

    const auto report_selection_invocation_catalog =
        copperfin::studio::plan_studio_toolbox_invocation_admission_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::report_expression,
            .asset_path = "reports/orders.frx",
            .record_index = 3U,
            .object_name = "Field1",
            .unique_id = "field-guid",
            .admit_palette_invocation = false
        });
    expect(report_selection_invocation_catalog.ok &&
            report_selection_invocation_catalog.selection_context == StudioEditorSelectionContext::report_expression &&
            report_selection_invocation_catalog.toolbox_context == StudioToolboxContext::report &&
            report_selection_invocation_catalog.item_count == report_items.size() &&
            report_selection_invocation_catalog.launch_plan.ok &&
            report_selection_invocation_catalog.invocation_admission.ok &&
            !report_selection_invocation_catalog.invocation_admission.plan.palette_invocation_admitted &&
            report_selection_invocation_catalog.admission_count == 1U &&
            report_selection_invocation_catalog.error_count == 0U &&
            report_selection_invocation_catalog.dry_run &&
            !report_selection_invocation_catalog.mutates_asset &&
            has_toolbox_item(report_selection_invocation_catalog.items, "label") &&
            !has_toolbox_item(report_selection_invocation_catalog.items, "textbox"),
        "#1288: report selection toolbox admission catalogs should preserve report-safe dry-run metadata");

    const auto menu_selection_invocation_catalog =
        copperfin::studio::plan_studio_toolbox_invocation_admission_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::menu_item,
            .asset_path = "menus/main.mnx",
            .record_index = 2U,
            .object_name = "File",
            .unique_id = "menu-guid",
            .admit_palette_invocation = true
        });
    expect(!menu_selection_invocation_catalog.ok &&
            menu_selection_invocation_catalog.error ==
                "A selection-context toolbox invocation admission catalog request requires a toolbox palette." &&
            menu_selection_invocation_catalog.selection_context == StudioEditorSelectionContext::menu_item &&
            menu_selection_invocation_catalog.item_count == 0U &&
            menu_selection_invocation_catalog.items.empty() &&
            !menu_selection_invocation_catalog.launch_plan.ok &&
            menu_selection_invocation_catalog.launch_plan.error ==
                "The selected Studio context does not expose a toolbox palette." &&
            !menu_selection_invocation_catalog.invocation_admission.ok &&
            menu_selection_invocation_catalog.admission_count == 0U &&
            menu_selection_invocation_catalog.error_count == 0U &&
            menu_selection_invocation_catalog.dry_run &&
            !menu_selection_invocation_catalog.mutates_asset,
        "#1288: unsupported selection toolbox admission catalogs should reject without mutation");

    const auto dry_run_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = dry_run_invocation.plan
    });
    expect(!dry_run_dispatch.ok &&
            dry_run_dispatch.error == "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1233: toolbox dispatch should reject dry-run admission plans");

    auto missing_command_plan = admitted_invocation.plan;
    missing_command_plan.command_token = {};
    const auto missing_command_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = missing_command_plan
    });
    expect(!missing_command_dispatch.ok &&
            missing_command_dispatch.error == "A toolbox dispatch request requires a command token.",
        "#1233: toolbox dispatch should reject admitted plans without command tokens");

    auto missing_items_plan = admitted_invocation.plan;
    missing_items_plan.items.clear();
    missing_items_plan.item_count = 0U;
    const auto missing_items_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = missing_items_plan
    });
    expect(!missing_items_dispatch.ok &&
            missing_items_dispatch.error == "A toolbox dispatch request requires validated toolbox item metadata.",
        "#1233: toolbox dispatch should reject admitted plans without item metadata");

    auto inconsistent_dispatch_plan = admitted_invocation.plan;
    inconsistent_dispatch_plan.item_count += 1U;
    const auto inconsistent_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = inconsistent_dispatch_plan
    });
    expect(!inconsistent_dispatch.ok &&
            inconsistent_dispatch.error == "A toolbox dispatch request requires consistent toolbox item metadata.",
        "#1233: toolbox dispatch should reject admitted plans with inconsistent item metadata");

    const auto form_dispatch_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = StudioToolboxContext::form,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .admit_palette_invocation = true
    });
    expect(form_dispatch_catalog.ok &&
            form_dispatch_catalog.toolbox_context == StudioToolboxContext::form &&
            form_dispatch_catalog.command_token == "studio.toolbox.palette.invoke" &&
            form_dispatch_catalog.asset_path == "forms/customer.scx" &&
            form_dispatch_catalog.record_index == 1U &&
            form_dispatch_catalog.object_name == "frmCustomer" &&
            form_dispatch_catalog.unique_id == "form-guid" &&
            form_dispatch_catalog.item_count == form_items.size() &&
            form_dispatch_catalog.items.size() == form_items.size() &&
            form_dispatch_catalog.dispatch_count == 1U &&
            form_dispatch_catalog.error_count == 0U &&
            !form_dispatch_catalog.dry_run &&
            !form_dispatch_catalog.mutates_asset &&
            form_dispatch_catalog.invocation_admission.ok &&
            form_dispatch_catalog.dispatch.ok &&
            has_toolbox_item(form_dispatch_catalog.items, "textbox") &&
            has_argument_pair(
                form_dispatch_catalog.dispatch.plan.dispatch_arguments,
                "--toolbox-context",
                "form"),
        "#1235: admitted toolbox dispatch catalogs should preserve form item metadata and dispatch once");
    expect(form_dispatch_catalog.ok &&
            form_invocation_catalog.ok &&
            form_dispatch_catalog.command_token == form_invocation_catalog.command_token &&
            form_dispatch_catalog.item_count == form_invocation_catalog.item_count &&
            form_dispatch_catalog.items.size() == form_invocation_catalog.items.size() &&
            form_dispatch_catalog.invocation_admission.ok &&
            form_dispatch_catalog.invocation_admission.plan.palette_invocation_admitted ==
                form_invocation_catalog.invocation_admission.plan.palette_invocation_admitted &&
            form_dispatch_catalog.invocation_admission.plan.dry_run ==
                form_invocation_catalog.invocation_admission.plan.dry_run &&
            form_dispatch_catalog.invocation_admission.plan.asset_path ==
                form_invocation_catalog.invocation_admission.plan.asset_path,
        "#1287: toolbox dispatch catalogs should preserve shared invocation admission catalog metadata");

    const auto report_dispatch_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = StudioToolboxContext::report,
        .asset_path = "reports/orders.frx",
        .record_index = 3U,
        .object_name = "Field1",
        .unique_id = "field-guid",
        .admit_palette_invocation = true
    });
    expect(report_dispatch_catalog.ok &&
            report_dispatch_catalog.toolbox_context == StudioToolboxContext::report &&
            report_dispatch_catalog.item_count == report_items.size() &&
            report_dispatch_catalog.dispatch_count == 1U &&
            report_dispatch_catalog.error_count == 0U &&
            !report_dispatch_catalog.dry_run &&
            has_toolbox_item(report_dispatch_catalog.items, "label") &&
            !has_toolbox_item(report_dispatch_catalog.items, "textbox") &&
            report_dispatch_catalog.dispatch.ok &&
            report_dispatch_catalog.dispatch.plan.toolbox_context == StudioToolboxContext::report &&
            has_argument_pair(
                report_dispatch_catalog.dispatch.plan.dispatch_arguments,
                "--toolbox-context",
                "report"),
        "#1235: admitted report toolbox dispatch catalogs should preserve report-safe item metadata");

    const auto dry_run_dispatch_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = StudioToolboxContext::form,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .admit_palette_invocation = false
    });
    expect(dry_run_dispatch_catalog.ok &&
            dry_run_dispatch_catalog.item_count == form_items.size() &&
            dry_run_dispatch_catalog.dispatch_count == 0U &&
            dry_run_dispatch_catalog.error_count == 1U &&
            dry_run_dispatch_catalog.dry_run &&
            !dry_run_dispatch_catalog.mutates_asset &&
            dry_run_dispatch_catalog.invocation_admission.ok &&
            !dry_run_dispatch_catalog.invocation_admission.plan.palette_invocation_admitted &&
            !dry_run_dispatch_catalog.dispatch.ok &&
            dry_run_dispatch_catalog.dispatch.error ==
                "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1235: dry-run toolbox dispatch catalogs should report dispatch rejections without mutation");
    expect(dry_run_dispatch_catalog.ok &&
            dry_run_dispatch_catalog.invocation_admission.ok &&
            !dry_run_dispatch_catalog.invocation_admission.plan.palette_invocation_admitted &&
            dry_run_dispatch_catalog.invocation_admission.plan.dry_run &&
            dry_run_dispatch_catalog.invocation_admission.plan.item_count == dry_run_dispatch_catalog.item_count,
        "#1287: dry-run toolbox dispatch catalogs should retain admission catalog dry-run state");

    const auto form_execution_catalog = copperfin::studio::plan_studio_toolbox_dispatch_execution_catalog({
        .toolbox_context = StudioToolboxContext::form,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .admit_palette_invocation = true,
        .admit_execution = true
    });
    expect(form_execution_catalog.ok &&
            form_execution_catalog.toolbox_context == StudioToolboxContext::form &&
            form_execution_catalog.command_token == "studio.toolbox.palette.invoke" &&
            form_execution_catalog.item_count == form_items.size() &&
            form_execution_catalog.items.size() == form_items.size() &&
            form_execution_catalog.entries.size() == form_items.size() &&
            form_execution_catalog.execution_ready_count == form_items.size() &&
            form_execution_catalog.error_count == 0U &&
            !form_execution_catalog.dry_run &&
            !form_execution_catalog.mutates_asset &&
            form_execution_catalog.invocation_admission.ok &&
            form_execution_catalog.dispatch.ok &&
            has_toolbox_item(form_execution_catalog.items, "textbox"),
        "#1330: admitted toolbox dispatch execution catalogs should mark every toolbox item ready without launch");
    const auto* textbox_execution = find_execution_catalog_entry(form_execution_catalog.entries, "textbox");
    expect(textbox_execution != nullptr &&
            textbox_execution->execution_admitted &&
            textbox_execution->execution_ready &&
            textbox_execution->execution_error.empty() &&
            std::string(textbox_execution->item.vfp_class) == "TextBox" &&
            form_execution_catalog.dispatch.plan.dispatch_admitted &&
            !form_execution_catalog.dispatch.plan.executed &&
            has_argument_pair(
                form_execution_catalog.dispatch.plan.dispatch_arguments,
                "--toolbox-context",
                "form"),
        "#1330: toolbox dispatch execution catalog entries should preserve item and dispatch metadata");

    const auto unadmitted_execution_catalog = copperfin::studio::plan_studio_toolbox_dispatch_execution_catalog({
        .toolbox_context = StudioToolboxContext::form,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .admit_palette_invocation = true,
        .admit_execution = false
    });
    const auto* unadmitted_textbox_execution =
        find_execution_catalog_entry(unadmitted_execution_catalog.entries, "textbox");
    expect(unadmitted_execution_catalog.ok &&
            unadmitted_execution_catalog.item_count == form_items.size() &&
            unadmitted_execution_catalog.execution_ready_count == 0U &&
            unadmitted_execution_catalog.error_count == form_items.size() &&
            unadmitted_execution_catalog.dry_run &&
            unadmitted_textbox_execution != nullptr &&
            !unadmitted_textbox_execution->execution_admitted &&
            !unadmitted_textbox_execution->execution_ready &&
            unadmitted_textbox_execution->execution_error ==
                "A toolbox dispatch execution catalog entry requires explicit execution admission.",
        "#1330: toolbox dispatch execution catalogs should require explicit execution admission per item");

    const auto dry_run_execution_catalog = copperfin::studio::plan_studio_toolbox_dispatch_execution_catalog({
        .toolbox_context = StudioToolboxContext::form,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .admit_palette_invocation = false,
        .admit_execution = true
    });
    const auto* dry_run_textbox_execution =
        find_execution_catalog_entry(dry_run_execution_catalog.entries, "textbox");
    expect(dry_run_execution_catalog.ok &&
            dry_run_execution_catalog.item_count == form_items.size() &&
            dry_run_execution_catalog.execution_ready_count == 0U &&
            dry_run_execution_catalog.error_count == form_items.size() &&
            dry_run_execution_catalog.dry_run &&
            dry_run_textbox_execution != nullptr &&
            dry_run_textbox_execution->execution_admitted &&
            !dry_run_textbox_execution->execution_ready &&
            dry_run_textbox_execution->execution_error ==
                "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1330: toolbox dispatch execution catalogs should preserve dispatch readiness failures per item");

    const auto missing_execution_catalog = copperfin::studio::plan_studio_toolbox_dispatch_execution_catalog({
        .toolbox_context = static_cast<StudioToolboxContext>(999),
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .admit_palette_invocation = true,
        .admit_execution = true
    });
    expect(!missing_execution_catalog.ok &&
            missing_execution_catalog.item_count == 0U &&
            missing_execution_catalog.items.empty() &&
            missing_execution_catalog.entries.empty() &&
            missing_execution_catalog.execution_ready_count == 0U &&
            missing_execution_catalog.error_count == 0U &&
            missing_execution_catalog.dry_run &&
            !missing_execution_catalog.mutates_asset,
        "#1330: toolbox dispatch execution catalogs should reject empty toolbox contexts without mutation");

    const auto visual_selection_dispatch_catalog =
        copperfin::studio::plan_studio_toolbox_dispatch_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid",
            .admit_palette_invocation = true
        });
    expect(visual_selection_dispatch_catalog.ok &&
            visual_selection_dispatch_catalog.selection_context == StudioEditorSelectionContext::visual_object &&
            visual_selection_dispatch_catalog.toolbox_context == StudioToolboxContext::form &&
            visual_selection_dispatch_catalog.command_token == "studio.toolbox.palette.invoke" &&
            visual_selection_dispatch_catalog.asset_path == "forms/customer.scx" &&
            visual_selection_dispatch_catalog.record_index == 1U &&
            visual_selection_dispatch_catalog.object_name == "frmCustomer" &&
            visual_selection_dispatch_catalog.unique_id == "form-guid" &&
            visual_selection_dispatch_catalog.item_count == form_items.size() &&
            visual_selection_dispatch_catalog.items.size() == form_items.size() &&
            visual_selection_dispatch_catalog.launch_plan.ok &&
            visual_selection_dispatch_catalog.invocation_admission.ok &&
            visual_selection_dispatch_catalog.dispatch.ok &&
            visual_selection_dispatch_catalog.dispatch_count == 1U &&
            visual_selection_dispatch_catalog.error_count == 0U &&
            !visual_selection_dispatch_catalog.dry_run &&
            !visual_selection_dispatch_catalog.mutates_asset &&
            !visual_selection_dispatch_catalog.dispatch.plan.executed &&
            has_toolbox_item(visual_selection_dispatch_catalog.items, "textbox") &&
            has_argument_pair(
                visual_selection_dispatch_catalog.dispatch.plan.dispatch_arguments,
                "--selection-context",
                "visual_object") &&
            has_argument_pair(
                visual_selection_dispatch_catalog.dispatch.plan.dispatch_arguments,
                "--toolbox-context",
                "form"),
        "#1290: visual selection toolbox dispatch catalogs should resolve form dispatch metadata");

    const auto report_selection_dispatch_catalog =
        copperfin::studio::plan_studio_toolbox_dispatch_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::report_expression,
            .asset_path = "reports/orders.frx",
            .record_index = 3U,
            .object_name = "Field1",
            .unique_id = "field-guid",
            .admit_palette_invocation = true
        });
    expect(report_selection_dispatch_catalog.ok &&
            report_selection_dispatch_catalog.selection_context == StudioEditorSelectionContext::report_expression &&
            report_selection_dispatch_catalog.toolbox_context == StudioToolboxContext::report &&
            report_selection_dispatch_catalog.item_count == report_items.size() &&
            report_selection_dispatch_catalog.dispatch_count == 1U &&
            report_selection_dispatch_catalog.error_count == 0U &&
            !report_selection_dispatch_catalog.dry_run &&
            report_selection_dispatch_catalog.launch_plan.ok &&
            report_selection_dispatch_catalog.invocation_admission.ok &&
            report_selection_dispatch_catalog.dispatch.ok &&
            has_toolbox_item(report_selection_dispatch_catalog.items, "label") &&
            !has_toolbox_item(report_selection_dispatch_catalog.items, "textbox") &&
            has_argument_pair(
                report_selection_dispatch_catalog.dispatch.plan.dispatch_arguments,
                "--selection-context",
                "report_expression") &&
            has_argument_pair(
                report_selection_dispatch_catalog.dispatch.plan.dispatch_arguments,
                "--toolbox-context",
                "report"),
        "#1290: report selection toolbox dispatch catalogs should preserve report-safe dispatch metadata");

    const auto dry_run_selection_dispatch_catalog =
        copperfin::studio::plan_studio_toolbox_dispatch_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid",
            .admit_palette_invocation = false
        });
    expect(dry_run_selection_dispatch_catalog.ok &&
            dry_run_selection_dispatch_catalog.toolbox_context == StudioToolboxContext::form &&
            dry_run_selection_dispatch_catalog.item_count == form_items.size() &&
            dry_run_selection_dispatch_catalog.launch_plan.ok &&
            dry_run_selection_dispatch_catalog.invocation_admission.ok &&
            !dry_run_selection_dispatch_catalog.invocation_admission.plan.palette_invocation_admitted &&
            dry_run_selection_dispatch_catalog.dispatch_count == 0U &&
            dry_run_selection_dispatch_catalog.error_count == 1U &&
            dry_run_selection_dispatch_catalog.dry_run &&
            !dry_run_selection_dispatch_catalog.mutates_asset &&
            !dry_run_selection_dispatch_catalog.dispatch.ok &&
            dry_run_selection_dispatch_catalog.dispatch.error ==
                "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1290: dry-run selection toolbox dispatch catalogs should reject dispatch without mutation");

    const auto unsupported_selection_dispatch_catalog =
        copperfin::studio::plan_studio_toolbox_dispatch_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::menu_item,
            .asset_path = "menus/main.mnx",
            .record_index = 2U,
            .object_name = "File",
            .unique_id = "menu-guid",
            .admit_palette_invocation = true
        });
    expect(!unsupported_selection_dispatch_catalog.ok &&
            unsupported_selection_dispatch_catalog.error ==
                "A selection-context toolbox dispatch catalog request requires a toolbox palette." &&
            unsupported_selection_dispatch_catalog.selection_context == StudioEditorSelectionContext::menu_item &&
            unsupported_selection_dispatch_catalog.item_count == 0U &&
            unsupported_selection_dispatch_catalog.items.empty() &&
            !unsupported_selection_dispatch_catalog.launch_plan.ok &&
            unsupported_selection_dispatch_catalog.launch_plan.error ==
                "The selected Studio context does not expose a toolbox palette." &&
            !unsupported_selection_dispatch_catalog.invocation_admission.ok &&
            !unsupported_selection_dispatch_catalog.dispatch.ok &&
            unsupported_selection_dispatch_catalog.dispatch_count == 0U &&
            unsupported_selection_dispatch_catalog.error_count == 0U &&
            unsupported_selection_dispatch_catalog.dry_run &&
            !unsupported_selection_dispatch_catalog.mutates_asset,
        "#1290: unsupported selection toolbox dispatch catalogs should reject without mutation");

    const auto missing_items_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = {},
        .admit_palette_invocation = true
    });
    expect(!missing_items_invocation.ok &&
            missing_items_invocation.error ==
                "A toolbox invocation admission request requires validated toolbox item metadata.",
        "#1219: toolbox invocation admission should reject missing item metadata");

    auto inconsistent_plan = visual_plan.plan;
    inconsistent_plan.item_count += 1U;
    const auto inconsistent_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = inconsistent_plan,
        .admit_palette_invocation = true
    });
    expect(!inconsistent_invocation.ok &&
            inconsistent_invocation.error ==
                "A toolbox invocation admission request requires consistent toolbox item metadata.",
        "#1219: toolbox invocation admission should reject inconsistent item metadata");

    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    locale_override.set("en-US");
    const auto english_items = copperfin::studio::studio_toolbox_palette();
    const auto english_error = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::menu_item,
        .asset_path = {},
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}});
    locale_override.set("es-419");
    const auto spanish_items = copperfin::studio::studio_toolbox_palette();
    const auto spanish_error = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::menu_item,
        .asset_path = {},
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}});
    locale_override.set("qps-ploc");
    const auto pseudo_items = copperfin::studio::studio_toolbox_palette();
    const auto pseudo_error = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::menu_item,
        .asset_path = {},
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}});

    const auto refresh_catalog_root = copperfin::localization::resolve_catalog_root();
    const auto refresh_english_catalog = copperfin::localization::load_catalogs(refresh_catalog_root, "en-US");
    const auto refresh_spanish_catalog = copperfin::localization::load_catalogs(refresh_catalog_root, "es-419");
    const auto refresh_pseudo_catalog = copperfin::localization::load_catalogs(refresh_catalog_root, "qps-ploc");
    const auto* english_label = find_toolbox_item(english_items, "label");
    const auto* refresh_spanish_label = find_toolbox_item(spanish_items, "label");
    const auto* refresh_pseudo_label = find_toolbox_item(pseudo_items, "label");
    expect(english_label != nullptr &&
               english_label->title == refresh_english_catalog.translate("Studio.Toolbox.Item.Label.Title"),
           "#4363: default toolbox display text should begin in en-US");
    expect(refresh_spanish_label != nullptr &&
               refresh_spanish_label->title == refresh_spanish_catalog.translate("Studio.Toolbox.Item.Label.Title"),
           "#4363: default toolbox display text should refresh to es-419");
    expect(refresh_pseudo_label != nullptr &&
               refresh_pseudo_label->title == refresh_pseudo_catalog.translate("Studio.Toolbox.Item.Label.Title"),
           "#4363: default toolbox display text should refresh to qps-ploc");
    expect(!english_error.ok &&
               english_error.error == refresh_english_catalog.translate("Studio.ToolboxPalette.Error.ContextUnavailable") &&
               !spanish_error.ok &&
               spanish_error.error == refresh_spanish_catalog.translate("Studio.ToolboxPalette.Error.ContextUnavailable") &&
               !pseudo_error.ok &&
               pseudo_error.error == refresh_pseudo_catalog.translate("Studio.ToolboxPalette.Error.ContextUnavailable"),
           "#4363: default toolbox diagnostics should refresh with the active locale");
    expect(english_label != nullptr && refresh_spanish_label != nullptr && refresh_pseudo_label != nullptr &&
               english_label->id == refresh_spanish_label->id && refresh_spanish_label->id == refresh_pseudo_label->id &&
               english_label->vfp_class == refresh_spanish_label->vfp_class &&
               refresh_spanish_label->vfp_class == refresh_pseudo_label->vfp_class,
           "#4363: locale refresh should preserve toolbox item identity and VFP class");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

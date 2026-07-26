// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.
// Included inside PrgRuntimeSession::Impl by prg_engine_session.inl.

        void seed_native_visual_properties(RuntimeOleObjectState &runtime_object)
        {
            const std::string normalized_base_class =
                normalize_identifier(trim_copy(runtime_object.base_class_name));
            const bool is_header_or_column_object =
                normalized_base_class == "column" || normalized_base_class == "header";

            if (is_native_visual_runtime_object(runtime_object) &&
                !is_native_olecontrol_host_object(runtime_object))
            {
                if (!runtime_object.properties.contains("left"))
                {
                    runtime_object.properties["left"] = make_number_value(0.0);
                }
                if (!runtime_object.properties.contains("top"))
                {
                    runtime_object.properties["top"] = make_number_value(0.0);
                }
                if (!runtime_object.properties.contains("width"))
                {
                    runtime_object.properties["width"] = make_number_value(0.0);
                }
                if (!runtime_object.properties.contains("height"))
                {
                    runtime_object.properties["height"] = make_number_value(0.0);
                }
            }

            if (is_native_visual_runtime_object(runtime_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("visible"))
            {
                runtime_object.properties["visible"] = make_boolean_value(true);
            }

            if ((is_native_visual_runtime_object(runtime_object) ||
                 normalized_base_class == "column") &&
                !runtime_object.properties.contains("enabled"))
            {
                runtime_object.properties["enabled"] = make_boolean_value(true);
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("mousepointer"))
            {
                runtime_object.properties["mousepointer"] = make_number_value(0.0);
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("fontname"))
            {
                // Headless contract: keep font identity deterministic without requiring
                // platform font enumeration or rendering support.
                runtime_object.properties["fontname"] = make_string_value("Arial");
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("fontcharset"))
            {
                // Headless contract: value 1 selects the documented default setting.
                runtime_object.properties["fontcharset"] = make_number_value(1.0);
            }

            if ((normalized_base_class == "checkbox" ||
                 normalized_base_class == "combobox" ||
                 normalized_base_class == "commandbutton" ||
                 normalized_base_class == "editbox" ||
                 normalized_base_class == "form" ||
                 normalized_base_class == "grid" ||
                 normalized_base_class == "label" ||
                 normalized_base_class == "listbox" ||
                 normalized_base_class == "optionbutton" ||
                 normalized_base_class == "pageframe" ||
                 normalized_base_class == "spinner" ||
                 normalized_base_class == "textbox") &&
                !runtime_object.properties.contains("righttoleft"))
            {
                // VFP9 documents true as the default, independent of host text direction.
                runtime_object.properties["righttoleft"] = make_boolean_value(true);
            }

            if ((normalized_base_class == "checkbox" ||
                 normalized_base_class == "commandbutton" ||
                 normalized_base_class == "label" ||
                 normalized_base_class == "optionbutton" ||
                 normalized_base_class == "header") &&
                !runtime_object.properties.contains("wordwrap"))
            {
                // VFP9 documents false as the default; text measurement is host-owned.
                runtime_object.properties["wordwrap"] = make_boolean_value(false);
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicfontname"))
            {
                // Headless contract: preserve the raw dynamic-font expression without evaluating it.
                runtime_object.properties["dynamicfontname"] = make_string_value("");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicfontbold"))
            {
                // VFP9 documents DynamicFontBold as the true-default exception in this family.
                // Preserve the raw logical expression without evaluating it.
                runtime_object.properties["dynamicfontbold"] = make_string_value(".T.");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicfontitalic"))
            {
                // VFP9 documents false as the default; preserve the raw logical expression.
                runtime_object.properties["dynamicfontitalic"] = make_string_value(".F.");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicfontstrikethru"))
            {
                // VFP9 documents false as the default; preserve the raw logical expression.
                runtime_object.properties["dynamicfontstrikethru"] = make_string_value(".F.");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicfontunderline"))
            {
                // VFP9 documents false as the default; preserve the raw logical expression.
                runtime_object.properties["dynamicfontunderline"] = make_string_value(".F.");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicfontsize"))
            {
                // Headless contract: preserve the raw dynamic-font-size expression without evaluating it.
                runtime_object.properties["dynamicfontsize"] = make_string_value("");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicfontshadow"))
            {
                // Headless contract: preserve the raw dynamic-font-shadow expression without evaluating it.
                runtime_object.properties["dynamicfontshadow"] = make_string_value("");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicfontoutline"))
            {
                // Headless contract: preserve the raw dynamic-font-outline expression without evaluating it.
                runtime_object.properties["dynamicfontoutline"] = make_string_value("");
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("fontsize"))
            {
                // Headless contract: preserve fractional point sizes without requiring
                // platform font enumeration or rendering support.
                runtime_object.properties["fontsize"] = make_number_value(10.0);
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("fontbold"))
            {
                runtime_object.properties["fontbold"] = make_boolean_value(false);
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("fontitalic"))
            {
                runtime_object.properties["fontitalic"] = make_boolean_value(false);
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("fontunderline"))
            {
                runtime_object.properties["fontunderline"] = make_boolean_value(false);
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("fontstrikethru"))
            {
                runtime_object.properties["fontstrikethru"] = make_boolean_value(false);
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("fontoutline"))
            {
                runtime_object.properties["fontoutline"] = make_boolean_value(false);
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("fontshadow"))
            {
                runtime_object.properties["fontshadow"] = make_boolean_value(false);
            }

            if (is_native_visual_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("controltiptext"))
            {
                runtime_object.properties["controltiptext"] = make_string_value("");
            }

            if (is_header_or_column_object &&
                !runtime_object.properties.contains("tooltiptext"))
            {
                runtime_object.properties["tooltiptext"] = make_string_value("");
            }

            if (is_header_or_column_object &&
                !runtime_object.properties.contains("statusbartext"))
            {
                runtime_object.properties["statusbartext"] = make_string_value("");
            }

            if (is_header_or_column_object &&
                !runtime_object.properties.contains("mouseicon"))
            {
                runtime_object.properties["mouseicon"] = make_string_value("");
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_object) &&
                !runtime_object.properties.contains("tag"))
            {
                runtime_object.properties["tag"] = make_string_value("");
            }

            if (is_native_visual_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("helpcontextid"))
            {
                runtime_object.properties["helpcontextid"] = make_number_value(0.0);
            }

            if (is_native_visual_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("whatsthishelpid"))
            {
                runtime_object.properties["whatsthishelpid"] = make_number_value(0.0);
            }

            if (normalized_base_class == "commandbutton" &&
                !runtime_object.properties.contains("default"))
            {
                runtime_object.properties["default"] = make_boolean_value(false);
            }

            if (normalized_base_class == "commandbutton" &&
                !runtime_object.properties.contains("cancel"))
            {
                runtime_object.properties["cancel"] = make_boolean_value(false);
            }

            if (normalized_base_class == "commandbutton" &&
                !runtime_object.properties.contains("style"))
            {
                runtime_object.properties["style"] = make_number_value(0.0);
            }

            if (normalized_base_class == "commandbutton")
            {
                if (!runtime_object.properties.contains("picturemargin"))
                {
                    runtime_object.properties["picturemargin"] = make_number_value(0.0);
                }
                if (!runtime_object.properties.contains("pictureposition"))
                {
                    runtime_object.properties["pictureposition"] = make_number_value(13.0);
                }
                if (!runtime_object.properties.contains("picturespacing"))
                {
                    runtime_object.properties["picturespacing"] = make_number_value(0.0);
                }
            }

            if ((normalized_base_class == "checkbox" ||
                 normalized_base_class == "commandbutton" ||
                 normalized_base_class == "commandgroup" ||
                 normalized_base_class == "form" ||
                 normalized_base_class == "header" ||
                 normalized_base_class == "label" ||
                 normalized_base_class == "optionbutton" ||
                 normalized_base_class == "optiongroup" ||
                 normalized_base_class == "page" ||
                 normalized_base_class == "pageframe") &&
                !runtime_object.properties.contains("caption"))
            {
                runtime_object.properties["caption"] = make_string_value("");
            }

            if (is_native_visual_picture_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("picture"))
            {
                // Headless contract: preserve the VFP string property without loading image bytes.
                runtime_object.properties["picture"] = make_string_value("");
            }

            if (is_native_visual_drag_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("dragmode"))
            {
                runtime_object.properties["dragmode"] = make_number_value(0.0);
            }

            if (is_native_visual_runtime_object(runtime_object) &&
                normalized_base_class != "form" &&
                normalized_base_class != "toolbar" &&
                !runtime_object.properties.contains("anchor"))
            {
                // Headless contract: preserve the documented default without
                // claiming host resize/reflow behavior.
                runtime_object.properties["anchor"] = make_number_value(0.0);
            }

            if (is_native_visual_drag_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("dragicon"))
            {
                // Headless contract: preserve the VFP path property without loading icon bytes.
                runtime_object.properties["dragicon"] = make_string_value("");
            }

            if (is_native_visual_button_state_picture_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("downpicture"))
            {
                // Headless contract: preserve the VFP path property without loading image bytes.
                runtime_object.properties["downpicture"] = make_string_value("");
            }

            if (is_native_visual_button_state_picture_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("disabledpicture"))
            {
                // Headless contract: preserve the VFP path property without loading image bytes.
                runtime_object.properties["disabledpicture"] = make_string_value("");
            }

            if (is_native_visual_autosize_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("autosize"))
            {
                // Headless contract: expose the boolean without measuring or resizing.
                runtime_object.properties["autosize"] = make_boolean_value(false);
            }

            if (is_native_visual_drawmode_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("drawmode"))
            {
                // Headless contract: expose the drawing mode without compositing pixels.
                runtime_object.properties["drawmode"] = make_number_value(13.0);
            }

            if (is_native_visual_backstyle_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("backstyle"))
            {
                // Headless contract: preserve the transparency mode without compositing pixels.
                runtime_object.properties["backstyle"] = make_number_value(1.0);
            }

            if (is_native_visual_specialeffect_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("specialeffect"))
            {
                // Container has a distinct VFP9 default; other covered controls default to 3D.
                runtime_object.properties["specialeffect"] = make_number_value(
                    normalized_base_class == "container" ? 1.0 : 0.0);
            }

            const bool is_header_or_column_color_object =
                normalized_base_class == "column" || normalized_base_class == "header";

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_color_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("backcolor"))
            {
                // Headless contract: seed an explicit deterministic visual background
                // until per-class VFP defaults are modeled with stronger evidence.
                runtime_object.properties["backcolor"] = make_int64_value(16777215);
            }

            if ((is_native_visual_runtime_object(runtime_object) || is_header_or_column_color_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("forecolor"))
            {
                // Headless contract: seed an explicit deterministic visual foreground
                // until per-class VFP defaults are modeled with stronger evidence.
                runtime_object.properties["forecolor"] = make_int64_value(0);
            }

            if ((normalized_base_class == "combobox" || normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("disableditembackcolor"))
            {
                runtime_object.properties["disableditembackcolor"] = make_int64_value(12632256);
            }

            if ((normalized_base_class == "combobox" || normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("disableditemforecolor"))
            {
                runtime_object.properties["disableditemforecolor"] = make_int64_value(8421504);
            }

            if ((normalized_base_class == "combobox" || normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("itembackcolor"))
            {
                runtime_object.properties["itembackcolor"] = make_int64_value(16777215);
            }

            if ((normalized_base_class == "combobox" || normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("itemforecolor"))
            {
                runtime_object.properties["itemforecolor"] = make_int64_value(0);
            }

            if ((normalized_base_class == "combobox" || normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("selecteditembackcolor"))
            {
                runtime_object.properties["selecteditembackcolor"] = make_int64_value(8421504);
            }

            if ((normalized_base_class == "combobox" || normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("selecteditemforecolor"))
            {
                runtime_object.properties["selecteditemforecolor"] = make_int64_value(16777215);
            }

            if ((normalized_base_class == "column" ||
                 normalized_base_class == "editbox" ||
                 normalized_base_class == "header" ||
                 normalized_base_class == "label" ||
                 normalized_base_class == "textbox") &&
                !runtime_object.properties.contains("alignment"))
            {
                const double default_alignment =
                    normalized_base_class == "column" || normalized_base_class == "header" ? 3.0 : 0.0;
                runtime_object.properties["alignment"] = make_number_value(default_alignment);
            }

            if (normalized_base_class == "editbox" &&
                !runtime_object.properties.contains("scrollbars"))
            {
                runtime_object.properties["scrollbars"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "column") &&
                !runtime_object.properties.contains("inputmask"))
            {
                runtime_object.properties["inputmask"] = make_string_value("");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicinputmask"))
            {
                // Headless contract: preserve the raw dynamic-mask expression without evaluating it.
                runtime_object.properties["dynamicinputmask"] = make_string_value("");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicbackcolor"))
            {
                // Headless contract: preserve the raw color expression without evaluating it.
                runtime_object.properties["dynamicbackcolor"] = make_string_value("");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicforecolor"))
            {
                // Headless contract: preserve the raw color expression without evaluating it.
                runtime_object.properties["dynamicforecolor"] = make_string_value("");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamicalignment"))
            {
                // Headless contract: preserve the raw dynamic-alignment expression without evaluating it.
                runtime_object.properties["dynamicalignment"] = make_string_value("");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("sparse"))
            {
                runtime_object.properties["sparse"] = make_boolean_value(true);
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "column") &&
                !runtime_object.properties.contains("format"))
            {
                runtime_object.properties["format"] = make_string_value("");
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("passwordchar"))
            {
                runtime_object.properties["passwordchar"] = make_string_value("");
            }

            if ((normalized_base_class == "editbox" || normalized_base_class == "textbox") &&
                !runtime_object.properties.contains("maxlength"))
            {
                runtime_object.properties["maxlength"] = make_number_value(0.0);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("specialeffect"))
            {
                runtime_object.properties["specialeffect"] = make_number_value(0.0);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("borderstyle"))
            {
                runtime_object.properties["borderstyle"] = make_number_value(1.0);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("hideselection"))
            {
                runtime_object.properties["hideselection"] = make_boolean_value(true);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("autocomplete"))
            {
                runtime_object.properties["autocomplete"] = make_number_value(0.0);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("enablehyperlinks"))
            {
                runtime_object.properties["enablehyperlinks"] = make_boolean_value(false);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("tooltiptext"))
            {
                runtime_object.properties["tooltiptext"] = make_string_value("");
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("margin"))
            {
                runtime_object.properties["margin"] = make_number_value(0.0);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("mouseicon"))
            {
                runtime_object.properties["mouseicon"] = make_string_value("");
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("disabledbackcolor"))
            {
                runtime_object.properties["disabledbackcolor"] = make_int64_value(12632256LL);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("disabledforecolor"))
            {
                runtime_object.properties["disabledforecolor"] = make_int64_value(8421504LL);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("statusbartext"))
            {
                runtime_object.properties["statusbartext"] = make_string_value("");
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("strictdateentry"))
            {
                runtime_object.properties["strictdateentry"] = make_number_value(1.0);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("themes"))
            {
                runtime_object.properties["themes"] = make_boolean_value(true);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("selectedbackcolor"))
            {
                runtime_object.properties["selectedbackcolor"] = make_int64_value(8421504LL);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("selectedforecolor"))
            {
                runtime_object.properties["selectedforecolor"] = make_int64_value(16777215LL);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("dateformat"))
            {
                runtime_object.properties["dateformat"] = make_number_value(0.0);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("century"))
            {
                runtime_object.properties["century"] = make_number_value(1.0);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("datemark"))
            {
                runtime_object.properties["datemark"] = make_string_value("");
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("hours"))
            {
                runtime_object.properties["hours"] = make_number_value(0.0);
            }

            if (normalized_base_class == "textbox" &&
                !runtime_object.properties.contains("seconds"))
            {
                runtime_object.properties["seconds"] = make_number_value(2.0);
            }

            if (is_native_tabindex_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("tabindex"))
            {
                runtime_object.properties["tabindex"] =
                    make_number_value(static_cast<double>(next_native_tab_index(runtime_object)));
            }

            if (is_native_tabstop_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("tabstop"))
            {
                runtime_object.properties["tabstop"] = make_boolean_value(true);
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "editbox" ||
                 normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("value"))
            {
                runtime_object.properties["value"] = make_string_value("");
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "editbox") &&
                !runtime_object.properties.contains("selstart"))
            {
                runtime_object.properties["selstart"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "editbox") &&
                !runtime_object.properties.contains("sellength"))
            {
                runtime_object.properties["sellength"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "editbox") &&
                !runtime_object.properties.contains("seltext"))
            {
                runtime_object.properties["seltext"] = make_string_value("");
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "editbox") &&
                !runtime_object.properties.contains("text"))
            {
                runtime_object.properties["text"] = make_string_value("");
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox" ||
                 normalized_base_class == "column" ||
                 normalized_base_class == "editbox" ||
                 normalized_base_class == "checkbox" ||
                 normalized_base_class == "spinner") &&
                !runtime_object.properties.contains("controlsource"))
            {
                runtime_object.properties["controlsource"] = make_string_value("");
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "editbox" ||
                 normalized_base_class == "column") &&
                !runtime_object.properties.contains("selectonentry"))
            {
                runtime_object.properties["selectonentry"] =
                    make_boolean_value(normalized_base_class == "column");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("resizable"))
            {
                runtime_object.properties["resizable"] = make_boolean_value(false);
            }

            if ((normalized_base_class == "form" ||
                 normalized_base_class == "column") &&
                !runtime_object.properties.contains("movable"))
            {
                // Headless contract: preserve the documented default without user-drag behavior.
                runtime_object.properties["movable"] = make_boolean_value(true);
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("currentcontrol"))
            {
                runtime_object.properties["currentcontrol"] = make_string_value("Text1");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamiccurrentcontrol"))
            {
                runtime_object.properties["dynamiccurrentcontrol"] = make_string_value("Text1");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("columnorder"))
            {
                runtime_object.properties["columnorder"] =
                    make_number_value(static_cast<double>(next_native_grid_column_order(runtime_object)));
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("bound"))
            {
                runtime_object.properties["bound"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("columncount"))
            {
                runtime_object.properties["columncount"] = make_number_value(-1.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("rowheight"))
            {
                runtime_object.properties["rowheight"] = make_number_value(-1.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("headerheight"))
            {
                runtime_object.properties["headerheight"] = make_number_value(20.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("allowheadersizing"))
            {
                runtime_object.properties["allowheadersizing"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("allowrowsizing"))
            {
                runtime_object.properties["allowrowsizing"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("allowautocolumnfit"))
            {
                runtime_object.properties["allowautocolumnfit"] = make_boolean_value(false);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("gridlinecolor"))
            {
                runtime_object.properties["gridlinecolor"] = make_number_value(0.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("gridlinewidth"))
            {
                runtime_object.properties["gridlinewidth"] = make_number_value(1.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("highlightstyle"))
            {
                runtime_object.properties["highlightstyle"] = make_number_value(0.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("highlightrowlinewidth"))
            {
                runtime_object.properties["highlightrowlinewidth"] = make_number_value(1.0);
            }

            if (normalized_base_class == "pageframe" &&
                !runtime_object.properties.contains("pagecount"))
            {
                runtime_object.properties["pagecount"] = make_number_value(0.0);
            }

            if (normalized_base_class == "pageframe" &&
                !runtime_object.properties.contains("activepage"))
            {
                runtime_object.properties["activepage"] = make_number_value(0.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("recordsource"))
            {
                runtime_object.properties["recordsource"] = make_string_value("");
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("linkmaster"))
            {
                // An unlinked headless Grid has no parent alias.
                runtime_object.properties["linkmaster"] = make_string_value("");
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("childorder"))
            {
                // An unconfigured headless Grid has no child index tag.
                runtime_object.properties["childorder"] = make_string_value("");
            }

            if ((normalized_base_class == "grid" ||
                 normalized_base_class == "relation") &&
                !runtime_object.properties.contains("relationalexpr"))
            {
                // Headless relationship objects preserve the raw expression only.
                runtime_object.properties["relationalexpr"] = make_string_value("");
            }

            if (normalized_base_class == "relation" &&
                !runtime_object.properties.contains("onetomany"))
            {
                // VFP9 defaults a relation to ordinary parent/child movement.
                runtime_object.properties["onetomany"] = make_boolean_value(false);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("allowcellselection"))
            {
                runtime_object.properties["allowcellselection"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("panellink"))
            {
                // VFP9 defaults PanelLink to true for a Grid.
                runtime_object.properties["panellink"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("allowaddnew"))
            {
                runtime_object.properties["allowaddnew"] = make_boolean_value(false);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("gridlines"))
            {
                runtime_object.properties["gridlines"] = make_number_value(3.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("highlight"))
            {
                runtime_object.properties["highlight"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("highlightrow"))
            {
                runtime_object.properties["highlightrow"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("deletemark"))
            {
                runtime_object.properties["deletemark"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("splitbar"))
            {
                runtime_object.properties["splitbar"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("view"))
            {
                // VFP9 documents Browse as the default view for an unsplit Grid.
                runtime_object.properties["view"] = make_number_value(0.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("leftcolumn"))
            {
                runtime_object.properties["leftcolumn"] = make_number_value(1.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("activecolumn"))
            {
                // Headless contract: a grid without hosted focus reports no active column.
                runtime_object.properties["activecolumn"] = make_number_value(0.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("activerow"))
            {
                // Headless contract: a grid without hosted focus reports no active row.
                runtime_object.properties["activerow"] = make_number_value(0.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("relativecolumn"))
            {
                // Headless contract: no visible relative column exists without hosted focus.
                runtime_object.properties["relativecolumn"] = make_number_value(0.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("relativerow"))
            {
                // Headless contract: no visible relative row exists without hosted focus.
                runtime_object.properties["relativerow"] = make_number_value(0.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("recordmark"))
            {
                runtime_object.properties["recordmark"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("recordsourcetype"))
            {
                runtime_object.properties["recordsourcetype"] = make_number_value(1.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("rowsource"))
            {
                runtime_object.properties["rowsource"] = make_string_value("");
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("rowsourcetype"))
            {
                runtime_object.properties["rowsourcetype"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("firstelement"))
            {
                runtime_object.properties["firstelement"] = make_number_value(1.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("numberofelements"))
            {
                runtime_object.properties["numberofelements"] = make_number_value(0.0);
            }

            if (normalized_base_class == "combobox" &&
                !runtime_object.properties.contains("displaycount"))
            {
                runtime_object.properties["displaycount"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("nulldisplay"))
            {
                runtime_object.properties["nulldisplay"] = make_string_value(".NULL.");
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("columnlines"))
            {
                runtime_object.properties["columnlines"] = make_boolean_value(true);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("itemtips"))
            {
                runtime_object.properties["itemtips"] = make_boolean_value(false);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("incrementalsearch"))
            {
                runtime_object.properties["incrementalsearch"] = make_boolean_value(true);
            }

            if ((normalized_base_class == "listbox" ||
                 normalized_base_class == "editbox" ||
                 normalized_base_class == "textbox") &&
                !runtime_object.properties.contains("integralheight"))
            {
                runtime_object.properties["integralheight"] = make_boolean_value(false);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("listindex"))
            {
                runtime_object.properties["listindex"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("displayvalue"))
            {
                runtime_object.properties["displayvalue"] = make_string_value("");
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("listcount"))
            {
                runtime_object.properties["listcount"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("sorted"))
            {
                runtime_object.properties["sorted"] = make_boolean_value(false);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("boundto"))
            {
                runtime_object.properties["boundto"] = make_boolean_value(false);
            }

            if (normalized_base_class == "listbox" &&
                !runtime_object.properties.contains("multiselect"))
            {
                runtime_object.properties["multiselect"] = make_boolean_value(false);
            }

            if (normalized_base_class == "listbox" &&
                !runtime_object.properties.contains("moverbars"))
            {
                runtime_object.properties["moverbars"] = make_boolean_value(false);
            }

            if (normalized_base_class == "listbox" &&
                !runtime_object.properties.contains("autohidescrollbar"))
            {
                runtime_object.properties["autohidescrollbar"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("newindex"))
            {
                runtime_object.properties["newindex"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("newitemid"))
            {
                runtime_object.properties["newitemid"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("listitemid"))
            {
                runtime_object.properties["listitemid"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("topitemid"))
            {
                runtime_object.properties["topitemid"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("topindex"))
            {
                runtime_object.properties["topindex"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("boundcolumn"))
            {
                runtime_object.properties["boundcolumn"] = make_number_value(1.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("columncount"))
            {
                runtime_object.properties["columncount"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("columnwidths"))
            {
                runtime_object.properties["columnwidths"] = make_string_value("");
            }

            if (normalized_base_class == "combobox" &&
                !runtime_object.properties.contains("style"))
            {
                runtime_object.properties["style"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "combobox" ||
                 normalized_base_class == "editbox" ||
                 normalized_base_class == "grid" ||
                 normalized_base_class == "column" ||
                 normalized_base_class == "checkbox" ||
                 normalized_base_class == "spinner") &&
                !runtime_object.properties.contains("readonly"))
            {
                runtime_object.properties["readonly"] = make_boolean_value(false);
            }

            normalize_native_combobox_readonly_invariant(runtime_object);
            normalize_native_list_control_sorted_invariant(runtime_object);
            normalize_native_list_control_columnwidths_invariant(runtime_object);
            normalize_native_listbox_multiselect_invariant(runtime_object);
            normalize_native_pageframe_activepage_invariant(runtime_object);

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("showwindow"))
            {
                runtime_object.properties["showwindow"] = make_number_value(0.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("windowtype"))
            {
                runtime_object.properties["windowtype"] = make_number_value(0.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("windowstate"))
            {
                runtime_object.properties["windowstate"] = make_number_value(0.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("scalemode"))
            {
                // Headless contract: preserve the coordinate mode without converting geometry.
                runtime_object.properties["scalemode"] = make_number_value(0.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("drawstyle"))
            {
                // Headless contract: preserve the line style without rasterizing graphics.
                runtime_object.properties["drawstyle"] = make_number_value(0.0);
            }

            if (is_native_visual_fillstyle_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("fillstyle"))
            {
                // Headless contract: preserve the fill pattern without rasterizing graphics.
                runtime_object.properties["fillstyle"] = make_number_value(1.0);
            }

            if (is_native_visual_fillcolor_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("fillcolor"))
            {
                // Headless contract: preserve the integer fill color without rasterizing graphics.
                runtime_object.properties["fillcolor"] = make_int64_value(0);
            }

            if (is_native_visual_borderwidth_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("borderwidth"))
            {
                // Headless contract: preserve border width without drawing or relayout.
                runtime_object.properties["borderwidth"] = make_number_value(1.0);
            }

            if (is_native_visual_bordercolor_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("bordercolor"))
            {
                // Headless contract: preserve the integer color without system-color lookup.
                runtime_object.properties["bordercolor"] = make_int64_value(0);
            }

            if (is_native_visual_borderstyle_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("borderstyle"))
            {
                const bool is_borderless_default = normalized_base_class == "image" ||
                                                   normalized_base_class == "label";
                runtime_object.properties["borderstyle"] = make_number_value(
                    is_borderless_default ? 0.0 : 1.0);
            }

            if (is_native_form_drawwidth_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("drawwidth"))
            {
                // Headless contract: preserve line width without rasterizing graphics.
                runtime_object.properties["drawwidth"] = make_number_value(1.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("borderstyle"))
            {
                runtime_object.properties["borderstyle"] = make_number_value(3.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("titlebar"))
            {
                runtime_object.properties["titlebar"] = make_number_value(1.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("desktop"))
            {
                runtime_object.properties["desktop"] = make_boolean_value(false);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("scrollbars"))
            {
                runtime_object.properties["scrollbars"] = make_number_value(0.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("alwaysontop"))
            {
                runtime_object.properties["alwaysontop"] = make_boolean_value(false);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("keypreview"))
            {
                // Headless contract: preserve the logical setting without claiming keyboard dispatch.
                runtime_object.properties["keypreview"] = make_boolean_value(false);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("mdiform"))
            {
                // Headless contract: preserve the logical setting without claiming MDI window layout.
                runtime_object.properties["mdiform"] = make_boolean_value(false);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("lockscreen"))
            {
                runtime_object.properties["lockscreen"] = make_boolean_value(false);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("controlbox"))
            {
                runtime_object.properties["controlbox"] = make_boolean_value(true);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("closable"))
            {
                runtime_object.properties["closable"] = make_boolean_value(true);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("minbutton"))
            {
                runtime_object.properties["minbutton"] = make_boolean_value(true);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("maxbutton"))
            {
                runtime_object.properties["maxbutton"] = make_boolean_value(true);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("autocenter"))
            {
                runtime_object.properties["autocenter"] = make_boolean_value(false);
            }
        }

        void seed_native_olecontrol_timeout_policy_properties(RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_olecontrol_host_object(runtime_object))
            {
                return;
            }

            if (!runtime_object.properties.contains("olerequestpendingtimeout"))
            {
                runtime_object.properties["olerequestpendingtimeout"] = make_int64_value(5000);
            }
            if (!runtime_object.properties.contains("oleserverbusytimeout"))
            {
                // VFP help documents the busy-timeout surface but not an explicit
                // default, so keep the native OleControl lane deterministic with a
                // representative 5-second retry window.
                runtime_object.properties["oleserverbusytimeout"] = make_int64_value(5000);
            }
            if (!runtime_object.properties.contains("oleserverbusyraiseerror"))
            {
                runtime_object.properties["oleserverbusyraiseerror"] = make_boolean_value(false);
            }
        }

        RuntimeOleObjectState *ensure_native_column_header_surface(RuntimeOleObjectState &column)
        {
            if (!is_native_column_runtime_object(column))
            {
                return nullptr;
            }

            const auto existing_header = column.properties.find("header");
            if (existing_header != column.properties.end())
            {
                const auto nested = resolve_ole_object(existing_header->second);
                return nested.has_value() ? *nested : nullptr;
            }

            const int handle = next_ole_handle++;
            RuntimeOleObjectState header{
                .handle = handle,
                .prog_id = "Header",
                .source = {},
                .last_action = "column.header",
                .action_count = 1};
            header.properties["parent"] =
                make_string_value("object:" + column.prog_id + "#" + std::to_string(column.handle));
            header.base_class_name = "Header";
            header.class_hierarchy = {"HEADER", "OBJECT"};
            assign_native_runtime_object_name(header, "Header");
            assign_native_window_metadata(header);
            seed_native_visual_properties(header);
            append_builtin_native_olecontrol_methods(header);
            header.default_properties = header.properties;
            header.default_properties.erase("parent");

            auto [header_it, _] = ole_objects.emplace(handle, std::move(header));
            column.properties["header"] =
                make_string_value("object:" + header_it->second.prog_id + "#" +
                                  std::to_string(header_it->second.handle));
            return &header_it->second;
        }

        void seed_native_olecontrol_verb_inspection_properties(RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_olecontrol_host_object(runtime_object))
            {
                return;
            }

            runtime_object.properties["objectverbscount"] = make_int64_value(2);
        }

        std::string representative_olecontrol_application_name(const std::string &oleclass) const
        {
            const std::string normalized = normalize_identifier(trim_copy(oleclass));
            if (normalized.rfind("excel.", 0U) == 0U)
            {
                return "Microsoft Excel";
            }
            if (normalized.rfind("word.", 0U) == 0U)
            {
                return "Microsoft Word";
            }
            return trim_copy(oleclass).empty() ? std::string("OLE Application") : trim_copy(oleclass);
        }

        std::string representative_olecontrol_application_progid(const std::string &oleclass) const
        {
            const std::string trimmed = trim_copy(oleclass);
            const std::size_t separator = trimmed.find('.');
            if (separator == std::string::npos || separator == 0U)
            {
                return trimmed.empty() ? std::string("Application") : trimmed + ".Application";
            }
            return trimmed.substr(0U, separator) + ".Application";
        }

        RuntimeOleObjectState *ensure_native_olecontrol_application_conflict_surface(
            RuntimeOleObjectState &runtime_object,
            RuntimeOleObjectState &object_surface)
        {
            if (!is_native_olecontrol_host_object(runtime_object))
            {
                return nullptr;
            }

            const auto existing_host_application = runtime_object.properties.find("application");
            if (existing_host_application != runtime_object.properties.end())
            {
                const auto nested = resolve_ole_object(existing_host_application->second);
                if (nested.has_value())
                {
                    return *nested;
                }
            }

            const auto oleclass = runtime_object.properties.find("oleclass");
            const std::string automation_prog_id =
                oleclass == runtime_object.properties.end() ? std::string{} : trim_copy(value_as_string(oleclass->second));

            const int host_application_handle = next_ole_handle++;
            RuntimeOleObjectState host_application_surface{
                .handle = host_application_handle,
                .prog_id = "Microsoft Visual FoxPro",
                .source = {},
                .last_action = "application",
                .action_count = 1};
            host_application_surface.properties["name"] = make_string_value("Microsoft Visual FoxPro");
            auto [host_application_it, _] = ole_objects.emplace(host_application_handle, std::move(host_application_surface));
            runtime_object.properties["application"] =
                make_string_value("object:" + host_application_it->second.prog_id + "#" +
                                  std::to_string(host_application_it->second.handle));

            if (!object_surface.properties.contains("application"))
            {
                const int object_application_handle = next_ole_handle++;
                RuntimeOleObjectState object_application_surface{
                    .handle = object_application_handle,
                    .prog_id = representative_olecontrol_application_progid(automation_prog_id),
                    .source = {},
                    .last_action = "application",
                    .action_count = 1};
                object_application_surface.properties["name"] =
                    make_string_value(representative_olecontrol_application_name(automation_prog_id));
                auto [object_application_it, __] = ole_objects.emplace(object_application_handle, std::move(object_application_surface));
                object_surface.properties["application"] =
                    make_string_value("object:" + object_application_it->second.prog_id + "#" +
                                      std::to_string(object_application_it->second.handle));
            }

            return &host_application_it->second;
        }

        RuntimeOleObjectState *ensure_native_olecontrol_object_surface(RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_olecontrol_host_object(runtime_object))
            {
                return nullptr;
            }

            const auto existing_object = runtime_object.properties.find("object");
            if (existing_object != runtime_object.properties.end())
            {
                const auto nested = resolve_ole_object(existing_object->second);
                if (nested.has_value())
                {
                    return *nested;
                }
            }

            const auto oleclass = runtime_object.properties.find("oleclass");
            if (oleclass == runtime_object.properties.end())
            {
                return nullptr;
            }

            const std::string automation_prog_id = trim_copy(value_as_string(oleclass->second));
            if (automation_prog_id.empty())
            {
                return nullptr;
            }

            const int handle = next_ole_handle++;
            RuntimeOleObjectState object_surface{
                .handle = handle,
                .prog_id = automation_prog_id,
                .source = {},
                .last_action = "object",
                .action_count = 1};
            object_surface.properties["left"] =
                make_string_value("ole:" + automation_prog_id + ".left");
            object_surface.properties["visible"] =
                make_string_value("ole:" + automation_prog_id + ".visible");
            object_surface.methods.push_back("compose");
            auto [object_it, _] = ole_objects.emplace(handle, std::move(object_surface));
            runtime_object.properties["object"] =
                make_string_value("object:" + object_it->second.prog_id + "#" + std::to_string(object_it->second.handle));
            (void)ensure_native_olecontrol_application_conflict_surface(runtime_object, object_it->second);
            return &object_it->second;
        }

        RuntimeOleObjectState *sync_native_owned_children_collection(RuntimeOleObjectState &runtime_object)
        {
            std::vector<std::pair<std::string, PrgValue>> child_members;
            std::vector<std::pair<std::string, PrgValue>> column_members;
            std::vector<std::pair<std::string, PrgValue>> page_members;
            child_members.reserve(runtime_object.properties.size());
            for (const auto &[property_name, property_value] : runtime_object.properties)
            {
                if (property_name == "parent" ||
                    property_name == "objects" ||
                    property_name == "controls" ||
                    property_name == "columns" ||
                    property_name == "pages")
                {
                    continue;
                }
                if (property_name == "object" && is_native_olecontrol_host_object(runtime_object))
                {
                    continue;
                }

                const auto child_object = resolve_ole_object(property_value);
                if (!child_object.has_value() || (*child_object)->hidden_runtime_surface)
                {
                    continue;
                }

                const auto child_parent = native_object_parent_reference(**child_object);
                int parent_handle = 0;
                std::string parent_prog_id;
                if (!child_parent.has_value() ||
                    !parse_object_handle_reference(*child_parent, parent_handle, parent_prog_id) ||
                    parent_handle != runtime_object.handle)
                {
                    continue;
                }

                child_members.emplace_back(
                    property_name,
                    make_string_value("object:" + (*child_object)->prog_id + "#" + std::to_string((*child_object)->handle)));
                if (is_native_grid_runtime_object(runtime_object) &&
                    is_native_column_runtime_object(**child_object))
                {
                    column_members.emplace_back(
                        property_name,
                        make_string_value("object:" + (*child_object)->prog_id + "#" + std::to_string((*child_object)->handle)));
                }
            }

            RuntimeOleObjectState *objects_collection = nullptr;
            const auto sync_collection_surface = [&](const std::string &property_name,
                                                    const std::vector<std::pair<std::string, PrgValue>> &members,
                                                    bool ensure_surface) -> RuntimeOleObjectState *
            {
                RuntimeOleObjectState *collection_object = nullptr;
                const auto collection_property = runtime_object.properties.find(property_name);
                if (collection_property != runtime_object.properties.end())
                {
                    const auto nested = resolve_ole_object(collection_property->second);
                    if (nested.has_value() &&
                        is_native_collection_object(**nested) &&
                        (*nested)->hidden_runtime_surface &&
                        (*nested)->read_only_collection_surface)
                    {
                        collection_object = *nested;
                    }
                }

                if (collection_object == nullptr && members.empty() && !ensure_surface)
                {
                    return nullptr;
                }

                if (collection_object == nullptr)
                {
                    const int handle = next_ole_handle++;
                    RuntimeOleObjectState collection_state{
                        .handle = handle,
                        .prog_id = "Collection",
                        .source = {},
                        .last_action = property_name,
                        .action_count = 1,
                        .hidden_runtime_surface = true,
                        .read_only_collection_surface = true};
                    collection_state.base_class_name = "Collection";
                    collection_state.class_hierarchy = {"COLLECTION", "OBJECT"};
                    collection_state.properties["parent"] =
                        make_string_value("object:" + runtime_object.prog_id + "#" + std::to_string(runtime_object.handle));
                    auto [collection_it, _] = ole_objects.emplace(handle, std::move(collection_state));
                    runtime_object.properties[property_name] =
                        make_string_value("object:" + collection_it->second.prog_id + "#" + std::to_string(collection_it->second.handle));
                    collection_object = &collection_it->second;
                }

                collection_object->properties["parent"] =
                    make_string_value("object:" + runtime_object.prog_id + "#" + std::to_string(runtime_object.handle));
                collection_object->collection_items.clear();
                collection_object->collection_item_keys.clear();
                collection_object->collection_items.reserve(members.size());
                collection_object->collection_item_keys.reserve(members.size());
                for (const auto &[child_name, child_reference] : members)
                {
                    collection_object->collection_items.push_back(child_reference);
                    collection_object->collection_item_keys.push_back(child_name);
                }
                (void)read_native_collection_member(*collection_object, "count");
                return collection_object;
            };

            if (!column_members.empty())
            {
                std::sort(
                    column_members.begin(),
                    column_members.end(),
                    [&](const auto &left, const auto &right)
                    {
                        const auto left_child = resolve_ole_object(left.second);
                        const auto right_child = resolve_ole_object(right.second);
                        const int left_order =
                            left_child.has_value() &&
                                    (*left_child)->properties.contains("columnorder")
                                ? normalize_native_column_order_value(
                                      (*left_child)->properties["columnorder"],
                                      1)
                                : 1;
                        const int right_order =
                            right_child.has_value() &&
                                    (*right_child)->properties.contains("columnorder")
                                ? normalize_native_column_order_value(
                                      (*right_child)->properties["columnorder"],
                                      1)
                                : 1;
                        if (left_order != right_order)
                        {
                            return left_order < right_order;
                        }
                        return left.first < right.first;
                    });
            }
            if (!child_members.empty())
            {
                std::sort(
                    child_members.begin(),
                    child_members.end(),
                    [&](const auto &left, const auto &right)
                    {
                        const auto left_child = resolve_ole_object(left.second);
                        const auto right_child = resolve_ole_object(right.second);
                        const int left_handle = left_child.has_value() ? (*left_child)->handle : 0;
                        const int right_handle = right_child.has_value() ? (*right_child)->handle : 0;
                        if (left_handle != right_handle)
                        {
                            return left_handle < right_handle;
                        }
                        return left.first < right.first;
                    });
            }
            if (is_native_pageframe_runtime_object(runtime_object))
            {
                for (const NativePageFramePageMember &page_member :
                     collect_native_pageframe_page_members(runtime_object))
                {
                    page_members.emplace_back(page_member.property_name, page_member.child_reference);
                }
            }

            objects_collection = sync_collection_surface("objects", child_members, false);
            RuntimeOleObjectState *controls_collection =
                sync_collection_surface("controls", child_members, false);
            RuntimeOleObjectState *columns_collection =
                is_native_grid_runtime_object(runtime_object)
                    ? sync_collection_surface("columns", column_members, true)
                    : nullptr;
            RuntimeOleObjectState *pages_collection =
                is_native_pageframe_runtime_object(runtime_object)
                    ? sync_collection_surface("pages", page_members, true)
                    : nullptr;
            if (controls_collection != nullptr)
            {
                runtime_object.properties["controlcount"] =
                    make_number_value(static_cast<double>(controls_collection->collection_items.size()));
            }
            else
            {
                runtime_object.properties.erase("controlcount");
            }
            if (columns_collection != nullptr &&
                runtime_object.properties.contains("columncount") &&
                normalize_native_grid_columncount_value(runtime_object.properties["columncount"]) >= 0)
            {
                runtime_object.properties["columncount"] =
                    make_number_value(static_cast<double>(columns_collection->collection_items.size()));
            }
            if (pages_collection != nullptr)
            {
                runtime_object.properties["pagecount"] =
                    make_number_value(static_cast<double>(pages_collection->collection_items.size()));
                normalize_native_pageframe_activepage_invariant(runtime_object);
            }
            return objects_collection;
        }

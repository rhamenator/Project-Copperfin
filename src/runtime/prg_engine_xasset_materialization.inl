    std::optional<std::string> PrgRuntimeSession::Impl::materialize_xasset_bootstrap(
        const std::string &asset_path,
        bool include_read_events)
    {
        std::filesystem::path snapshot_root;
        const auto snapshot_path = materialize_verified_xasset_snapshot(
            copperfin::platform::path_from_utf8_string(asset_path),
            snapshot_root);
        if (!snapshot_path.has_value())
        {
            return std::nullopt;
        }

        studio::StudioOpenRequest request;
        request.path = copperfin::platform::path_to_utf8_string(*snapshot_path);
        request.read_only = true;
        request.load_full_table = true;
        const auto open_result = studio::open_document(request);
        std::error_code ignored;
        if (!snapshot_root.empty())
        {
            std::filesystem::remove_all(snapshot_root, ignored);
        }
        if (!open_result.ok)
        {
            last_error_message = open_result.error;
            return std::nullopt;
        }

        const XAssetExecutableModel model = build_xasset_executable_model(open_result.document);
        if (!model.ok || !model.runnable_startup)
        {
            last_error_message = model.error.empty()
                                     ? runtime_text(
                                           "Runtime.Prg.Session.Error.NoRunnableStartupMethodsFoundInAsset",
                                           {{"path", asset_path}})
                                     : model.error;
            return std::nullopt;
        }

        const std::filesystem::path asset_file = copperfin::platform::path_from_utf8_string(asset_path);
        const std::filesystem::path bootstrap_path = make_prg_engine_xasset_bootstrap_path(
            runtime_temp_directory,
            asset_file,
            runtime_instance_id);

        const std::string bootstrap_source =
            build_xasset_bootstrap_source(model, include_read_events, asset_path, true);
        struct ScopedXAssetBootstrapFileCleanup
        {
            std::filesystem::path path;
            bool preserve = false;

            ~ScopedXAssetBootstrapFileCleanup()
            {
                if (preserve)
                {
                    return;
                }
                std::error_code ignored;
                std::filesystem::remove(path, ignored);
            }
        } bootstrap_file_cleanup{bootstrap_path};

        std::ofstream output(bootstrap_path, std::ios::binary | std::ios::trunc);
        output << bootstrap_source;
        if (prg_xasset_bootstrap_write_failure_requested(bootstrap_path))
        {
            output.setstate(std::ios::badbit);
        }
        output.close();
        if (!output.good())
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.XAssetBootstrapMaterializeFailed",
                {{"path", asset_path}});
            return std::nullopt;
        }

        options.source_text_overrides[copperfin::runtime::normalize_path(
            copperfin::platform::path_to_utf8_string(bootstrap_path))] = bootstrap_source;
        owned_xasset_bootstrap_paths.push_back(bootstrap_path);
        bootstrap_file_cleanup.preserve = true;

        return copperfin::platform::path_to_utf8_string(bootstrap_path);
    }

    std::optional<std::string> PrgRuntimeSession::Impl::materialize_vcx_class_source(
        const Frame &frame,
        const std::string &class_name,
        const std::string &library_path,
        std::string &resolved_library_path)
    {
        const std::string trimmed_class_name = trim_copy(class_name);
        const std::string trimmed_library_path = trim_copy(library_path);
        if (!is_bare_identifier_text(trimmed_class_name) || trimmed_library_path.empty())
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxClassNotFound",
                {{"className", trimmed_class_name}, {"classLibraryPath", trimmed_library_path}});
            return std::nullopt;
        }

        const std::string resolved_path =
            resolve_native_prg_program_path(trimmed_library_path, frame.file_path);
        const std::filesystem::path library_file =
            copperfin::platform::path_from_utf8_string(resolved_path);
        std::error_code filesystem_error;
        const auto verified_library = find_verified_file_byte_override(library_file);
        const bool has_verified_library =
            verified_library != options.verified_file_byte_overrides.end() &&
            !verified_library->second.empty();
        if (!std::filesystem::is_regular_file(library_file, filesystem_error) &&
            !(options.require_verified_file_byte_overrides && has_verified_library))
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path},
                 {"errorMessage", filesystem_error
                                      ? filesystem_error.message()
                                      : runtime_text("Runtime.Prg.Core.Detail.FileNotFound")}});
            return std::nullopt;
        }

        std::filesystem::path snapshot_root;
        const auto open_library_path = materialize_verified_xasset_snapshot(
            library_file,
            snapshot_root);
        if (!open_library_path.has_value())
        {
            const std::string verified_error = last_error_message;
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path},
                 {"errorMessage", verified_error}});
            return std::nullopt;
        }

        const auto open_result = studio::open_document({
            .path = copperfin::platform::path_to_utf8_string(*open_library_path),
            .read_only = true,
            .load_full_table = true
        });
        if (!snapshot_root.empty())
        {
            std::error_code snapshot_error;
            std::filesystem::remove_all(snapshot_root, snapshot_error);
        }
        if (!open_result.ok)
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path}, {"errorMessage", open_result.error}});
            return std::nullopt;
        }

        const XAssetExecutableModel model = build_xasset_executable_model(open_result.document);
        if (!model.ok)
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path}, {"errorMessage", model.error}});
            return std::nullopt;
        }

        const auto objects = studio::build_object_snapshot(open_result.document);
        const std::string normalized_requested_class = normalize_identifier(trimmed_class_name);
        const studio::StudioObjectSnapshot *root_object = nullptr;
        for (const auto &object : objects)
        {
            if (!object.parent_name.empty())
            {
                continue;
            }
            const std::string normalized_object_name = normalize_identifier(trim_copy(object.object_name));
            const std::string normalized_class_field = normalize_identifier(trim_copy(object.class_name));
            if (normalized_object_name == normalized_requested_class ||
                normalized_class_field == normalized_requested_class ||
                normalize_identifier(trim_copy(object.object_path)) == normalized_requested_class)
            {
                root_object = &object;
                break;
            }
        }
        if (root_object == nullptr)
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxClassNotFound",
                {{"className", trimmed_class_name}, {"classLibraryPath", trimmed_library_path}});
            return std::nullopt;
        }

        const std::string base_class_name = trim_copy(root_object->baseclass_name).empty()
            ? "Custom"
            : trim_copy(root_object->baseclass_name);
        if (!is_bare_identifier_text(base_class_name))
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path},
                 {"errorMessage", runtime_text("Runtime.Prg.Core.Detail.InvalidRootBaseClass")}});
            return std::nullopt;
        }

        std::ostringstream source;
        const std::filesystem::path include_root = options.require_verified_file_byte_overrides
            ? library_file.parent_path()
            : open_library_path->parent_path();
        const std::filesystem::path companion_header_candidate =
            include_root / library_file.stem();
        auto companion_header = companion_header_candidate;
        companion_header.replace_extension(".h");
        std::optional<std::filesystem::path> resolved_companion_header;
        bool companion_header_ambiguous = false;
        if (options.require_verified_file_byte_overrides)
        {
            const auto requested_header = companion_header.lexically_normal();
            const std::string requested_parent = lowercase_copy(normalize_path(
                copperfin::platform::path_to_utf8_string(requested_header.parent_path())));
            const std::string requested_filename = lowercase_copy(
                copperfin::platform::path_to_utf8_string(requested_header.filename()));
            for (const auto &[candidate_name, bytes] : options.verified_file_byte_overrides)
            {
                if (bytes.empty())
                {
                    continue;
                }
                const auto candidate_path = copperfin::platform::path_from_utf8_string(candidate_name).lexically_normal();
                if (lowercase_copy(normalize_path(
                        copperfin::platform::path_to_utf8_string(candidate_path.parent_path()))) != requested_parent ||
                    lowercase_copy(copperfin::platform::path_to_utf8_string(candidate_path.filename())) != requested_filename ||
                    lowercase_copy(copperfin::platform::path_to_utf8_string(candidate_path.extension())) != ".h")
                {
                    continue;
                }
                if (resolved_companion_header.has_value())
                {
                    companion_header_ambiguous = true;
                    resolved_companion_header.reset();
                    break;
                }
                resolved_companion_header = candidate_path;
            }
        }
        else
        {
            const auto companion_header_resolution = copperfin::vfp::resolve_unique_casefold_path(
                companion_header);
            companion_header_ambiguous = companion_header_resolution.ambiguous;
            if (companion_header_resolution.path.has_value())
            {
                resolved_companion_header = companion_header_resolution.path;
            }
        }

        source << "* Copperfin generated VCX class bridge\n";
        if (!companion_header_ambiguous && resolved_companion_header.has_value())
        {
            source << "#include \""
                   << copperfin::platform::path_to_utf8_string(
                          resolved_companion_header->lexically_normal())
                   << "\"\n";
        }
        source << "DEFINE CLASS " << trimmed_class_name << " AS " << base_class_name << "\n";
        for (const auto &property : root_object->properties)
        {
            if (!property.derived_from_property_blob ||
                !is_bare_identifier_text(property.name) ||
                trim_copy(property.value).empty() ||
                property.value.find('\n') != std::string::npos ||
                property.value.find('\r') != std::string::npos)
            {
                continue;
            }
            source << "    " << property.name << " = " << property.value << "\n";
        }

        const std::string root_path = normalize_identifier(trim_copy(root_object->object_path));
        std::set<std::string> emitted_methods;
        for (const auto &method : model.methods)
        {
            if (normalize_identifier(trim_copy(method.object_path)) != root_path ||
                !is_bare_identifier_text(method.method_name) ||
                trim_copy(method.source_text).empty() ||
                !emitted_methods.insert(normalize_identifier(method.method_name)).second)
            {
                continue;
            }
            source << "    PROCEDURE " << method.method_name << "\n"
                   << method.source_text;
            if (method.source_text.back() != '\n')
            {
                source << '\n';
            }
            source << "    ENDPROC\n";
        }
        source << "ENDDEFINE\n";

        std::error_code ignored;
        std::filesystem::create_directories(runtime_temp_directory, ignored);
        const std::string cache_key = resolved_path + ":" + normalized_requested_class;
        const std::filesystem::path generated_path =
            runtime_temp_directory /
            ("vcx_class_" + std::to_string(std::hash<std::string>{}(cache_key)) + ".prg");
        const std::string generated_source = source.str();
        std::ofstream output(generated_path, std::ios::binary | std::ios::trunc);
        output << generated_source;
        output.close();
        if (!output.good())
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path},
                 {"errorMessage", runtime_text("Runtime.Prg.Core.Detail.GeneratedClassSourceWriteFailed")}});
            return std::nullopt;
        }

        const std::string generated_path_text =
            copperfin::platform::path_to_utf8_string(generated_path);
        options.source_text_overrides[normalize_path(generated_path_text)] = generated_source;
        resolved_library_path = resolved_path;
        return generated_path_text;
    }

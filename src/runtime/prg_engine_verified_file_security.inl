// prg_engine_verified_file_security.inl
// Verified immutable file materialization helpers. Included inside Impl.

        std::optional<std::filesystem::path> resolve_verified_file_byte_override_path(
            const std::filesystem::path &file_path,
            bool &ambiguous,
            bool exact_paths_on_posix = false) const
        {
            ambiguous = false;
#if !defined(_WIN32)
            const bool use_exact_path = exact_paths_on_posix;
#else
            const bool use_exact_path = false;
#endif
            const std::string normalized_path = copperfin::platform::path_to_utf8_string(
                file_path.lexically_normal());
            const auto exact = options.verified_file_byte_overrides.find(normalized_path);
            if (exact != options.verified_file_byte_overrides.end())
            {
                if (!exact->second.empty())
                {
                    return copperfin::platform::path_from_utf8_string(exact->first);
                }
                if (use_exact_path)
                {
                    return std::nullopt;
                }
            }

            if (use_exact_path)
            {
                return std::nullopt;
            }

            std::optional<std::filesystem::path> resolved_path;
            for (const auto &[candidate_name, bytes] : options.verified_file_byte_overrides)
            {
                if (bytes.empty() || !paths_equal_insensitive(candidate_name, normalized_path))
                {
                    continue;
                }
                if (resolved_path.has_value())
                {
                    ambiguous = true;
                    return std::nullopt;
                }
                resolved_path = copperfin::platform::path_from_utf8_string(candidate_name);
            }
            return resolved_path;
        }

        std::optional<std::filesystem::path> materialize_verified_file_snapshot(
            const std::filesystem::path &file_path,
            std::filesystem::path &snapshot_root,
            const std::string &diagnostic_key,
            bool require_memo_sidecar,
            bool exact_paths_on_posix = false)
        {
            snapshot_root.clear();
            if (!options.require_verified_file_byte_overrides)
            {
                return file_path;
            }

            bool primary_ambiguous = false;
            const auto resolved_primary_path = resolve_verified_file_byte_override_path(
                file_path,
                primary_ambiguous,
                exact_paths_on_posix);
            const auto primary = resolved_primary_path.has_value()
                ? find_verified_file_byte_override(*resolved_primary_path)
                : options.verified_file_byte_overrides.end();
            if (primary_ambiguous ||
                primary == options.verified_file_byte_overrides.end() || primary->second.empty())
            {
                last_error_message = runtime_text(
                    diagnostic_key,
                    {{"path", copperfin::platform::path_to_utf8_string(file_path)}});
                return std::nullopt;
            }

            const auto sidecar_resolution = copperfin::vfp::resolve_vfp_memo_sidecar_path(file_path);
            const bool exact_sidecar_path =
#if !defined(_WIN32)
                exact_paths_on_posix;
#else
                false;
#endif
            if (sidecar_resolution.ambiguous && !exact_sidecar_path)
            {
                last_error_message = runtime_text(
                    diagnostic_key,
                    {{"path", copperfin::platform::path_to_utf8_string(sidecar_resolution.requested_path)}});
                return std::nullopt;
            }

            std::optional<std::pair<std::filesystem::path, std::string>> sidecar;
            const std::filesystem::path sidecar_candidate = exact_sidecar_path
                ? sidecar_resolution.requested_path
                : sidecar_resolution.path.value_or(sidecar_resolution.requested_path);
            if (sidecar_resolution.path.has_value() ||
                (options.require_verified_file_byte_overrides && !sidecar_candidate.empty()))
            {
                bool sidecar_ambiguous = false;
                const auto resolved_sidecar_path = resolve_verified_file_byte_override_path(
                    sidecar_candidate,
                    sidecar_ambiguous,
                    exact_paths_on_posix);
                const auto verified_sidecar = resolved_sidecar_path.has_value()
                    ? find_verified_file_byte_override(*resolved_sidecar_path)
                    : options.verified_file_byte_overrides.end();
                if (verified_sidecar == options.verified_file_byte_overrides.end() ||
                    verified_sidecar->second.empty() || sidecar_ambiguous)
                {
                    if (require_memo_sidecar)
                    {
                        last_error_message = runtime_text(
                            diagnostic_key,
                            {{"path", copperfin::platform::path_to_utf8_string(sidecar_candidate)}});
                        return std::nullopt;
                    }
                }
                else
                {
                    sidecar = std::make_pair(sidecar_candidate, verified_sidecar->second);
                }
            }

            snapshot_root = runtime_temp_directory /
                ("verified_file_" + std::to_string(runtime_instance_id) + "_" +
                 std::to_string(static_cast<unsigned long long>(executed_statement_count)));
            std::error_code filesystem_error;
            if (!std::filesystem::create_directories(snapshot_root, filesystem_error) || filesystem_error)
            {
                last_error_message = runtime_text(
                    diagnostic_key,
                    {{"path", copperfin::platform::path_to_utf8_string(file_path)}});
                snapshot_root.clear();
                return std::nullopt;
            }

            const auto write_snapshot = [](const std::filesystem::path &path, const std::string &bytes)
            {
                std::ofstream output(path, std::ios::binary | std::ios::trunc);
                output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
                output.close();
                return output.good();
            };
            const std::filesystem::path snapshot_path = snapshot_root / file_path.filename();
            if (!write_snapshot(snapshot_path, primary->second) ||
                (sidecar.has_value() &&
                 !write_snapshot(snapshot_root / sidecar->first.filename(), sidecar->second)))
            {
                std::filesystem::remove_all(snapshot_root, filesystem_error);
                snapshot_root.clear();
                last_error_message = runtime_text(
                    diagnostic_key,
                    {{"path", copperfin::platform::path_to_utf8_string(file_path)}});
                return std::nullopt;
            }
            return snapshot_path;
        }

        bool verified_database_index_path_matches(
            const std::filesystem::path &table_path,
            const std::filesystem::path &candidate_path,
            const std::string &extension) const
        {
            std::filesystem::path expected_path = table_path;
            expected_path.replace_extension(extension);
#if defined(_WIN32)
            return paths_equal_insensitive(
                copperfin::platform::path_to_utf8_string(candidate_path.lexically_normal()),
                copperfin::platform::path_to_utf8_string(expected_path.lexically_normal()));
#else
            return candidate_path.lexically_normal() == expected_path.lexically_normal();
#endif
        }

        std::optional<std::filesystem::path> materialize_verified_table_snapshot(
            const std::filesystem::path &table_path,
            std::filesystem::path &snapshot_root)
        {
            const auto snapshot_path = materialize_verified_file_snapshot(
                table_path,
                snapshot_root,
                "Runtime.Prg.Database.Error.VerifiedBytesUnavailable",
                false,
                true);
            if (!snapshot_path.has_value() || !options.require_verified_file_byte_overrides)
            {
                return snapshot_path;
            }

            for (const auto &[candidate_name, bytes] : options.verified_file_byte_overrides)
            {
                const auto candidate_path = copperfin::platform::path_from_utf8_string(candidate_name);
                const std::string extension = lowercase_copy(
                    copperfin::platform::path_to_utf8_string(candidate_path.extension()));
                if (extension != ".cdx" && extension != ".idx" &&
                    extension != ".ndx" && extension != ".mdx")
                {
                    continue;
                }
                if (!verified_database_index_path_matches(table_path, candidate_path, extension) || bytes.empty())
                {
                    continue;
                }

                std::ofstream output(snapshot_root / candidate_path.filename(), std::ios::binary | std::ios::trunc);
                output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
                output.close();
                if (!output.good())
                {
                    std::error_code filesystem_error;
                    std::filesystem::remove_all(snapshot_root, filesystem_error);
                    snapshot_root.clear();
                    last_error_message = runtime_text(
                        "Runtime.Prg.Database.Error.VerifiedBytesUnavailable",
                        {{"path", copperfin::platform::path_to_utf8_string(candidate_path)}});
                    return std::nullopt;
                }
            }
            return snapshot_path;
        }

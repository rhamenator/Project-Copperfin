// prg_engine_xasset_security.inl
// Verified dynamic xAsset materialization helpers. Included inside Impl.

        std::optional<std::filesystem::path> materialize_verified_xasset_snapshot(
            const std::filesystem::path &asset_path,
            std::filesystem::path &snapshot_root)
        {
            snapshot_root.clear();
            if (!options.require_verified_file_byte_overrides)
            {
                return asset_path;
            }

            const auto primary = find_verified_file_byte_override(asset_path);
            if (primary == options.verified_file_byte_overrides.end() || primary->second.empty())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Core.Error.VerifiedBytesUnavailable",
                    {{"path", copperfin::platform::path_to_utf8_string(asset_path)}});
                return std::nullopt;
            }

            const auto sidecar_resolution = copperfin::vfp::resolve_vfp_memo_sidecar_path(asset_path);
            if (sidecar_resolution.ambiguous)
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Core.Error.VerifiedBytesUnavailable",
                    {{"path", copperfin::platform::path_to_utf8_string(sidecar_resolution.requested_path)}});
                return std::nullopt;
            }

            std::optional<std::pair<std::filesystem::path, std::string>> sidecar;
            if (!sidecar_resolution.requested_path.empty())
            {
                const auto verified_sidecar = find_verified_file_byte_override(
                    sidecar_resolution.path.value_or(sidecar_resolution.requested_path));
                if (verified_sidecar == options.verified_file_byte_overrides.end() ||
                    verified_sidecar->second.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Core.Error.VerifiedBytesUnavailable",
                        {{"path", copperfin::platform::path_to_utf8_string(
                                      sidecar_resolution.path.value_or(sidecar_resolution.requested_path))}});
                    return std::nullopt;
                }
                sidecar = std::make_pair(
                    sidecar_resolution.path.value_or(sidecar_resolution.requested_path),
                    verified_sidecar->second);
            }

            snapshot_root = runtime_temp_directory /
                ("xasset_" + std::to_string(runtime_instance_id) + "_" +
                 std::to_string(static_cast<unsigned long long>(executed_statement_count)));
            std::error_code filesystem_error;
            if (!std::filesystem::create_directories(snapshot_root, filesystem_error) || filesystem_error)
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Core.Error.VerifiedBytesUnavailable",
                    {{"path", copperfin::platform::path_to_utf8_string(asset_path)}});
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
            const std::filesystem::path snapshot_path = snapshot_root / asset_path.filename();
            if (!write_snapshot(snapshot_path, primary->second))
            {
                std::filesystem::remove_all(snapshot_root, filesystem_error);
                snapshot_root.clear();
                last_error_message = runtime_text(
                    "Runtime.Prg.Core.Error.VerifiedBytesUnavailable",
                    {{"path", copperfin::platform::path_to_utf8_string(asset_path)}});
                return std::nullopt;
            }
            if (sidecar.has_value() &&
                !write_snapshot(snapshot_root / sidecar->first.filename(), sidecar->second))
            {
                std::filesystem::remove_all(snapshot_root, filesystem_error);
                snapshot_root.clear();
                last_error_message = runtime_text(
                    "Runtime.Prg.Core.Error.VerifiedBytesUnavailable",
                    {{"path", copperfin::platform::path_to_utf8_string(sidecar->first)}});
                return std::nullopt;
            }
            return snapshot_path;
        }


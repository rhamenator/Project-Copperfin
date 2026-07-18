// prg_engine_xasset_security.inl
// Verified dynamic xAsset materialization helpers. Included inside Impl.

        std::optional<std::filesystem::path> materialize_verified_xasset_snapshot(
            const std::filesystem::path &asset_path,
            std::filesystem::path &snapshot_root)
        {
            return materialize_verified_file_snapshot(
                asset_path,
                snapshot_root,
                "Runtime.Prg.Core.Error.VerifiedBytesUnavailable",
                true);
        }

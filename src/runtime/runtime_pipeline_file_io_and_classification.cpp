#include "runtime_pipeline_support.h"

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

BuildOutputKind parse_build_output_kind(const std::string& value) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    if (normalized == "dll") {
        return BuildOutputKind::dll;
    }
    if (normalized == "app") {
        return BuildOutputKind::app;
    }
    if (normalized == "fll") {
        return BuildOutputKind::fll;
    }
    if (normalized == "fxp") {
        return BuildOutputKind::fxp;
    }
    if (normalized == "ocx") {
        return BuildOutputKind::ocx;
    }
    if (normalized == "executable") {
        return BuildOutputKind::executable;
    }
    return BuildOutputKind::unknown;
}

std::string dotnet_parity_tier_name(copperfin::platform::DotNetParityTier tier) {
    switch (tier) {
        case copperfin::platform::DotNetParityTier::exact:
            return "exact";
        case copperfin::platform::DotNetParityTier::adapted:
            return "adapted";
        case copperfin::platform::DotNetParityTier::intentionally_not_supported:
            return "intentionally_not_supported";
        default:
            return "unknown";
    }
}

bool write_text_file(const std::filesystem::path& path, const std::string& contents, std::string& error) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        error = runtime_text("Runtime.Package.Error.CreateFileFailed", {{"path", path.string()}});
        return false;
    }

    output << contents;
    if (!output.good()) {
        error = runtime_text("Runtime.Package.Error.WriteFileFailed", {{"path", path.string()}});
        return false;
    }

    return true;
}

std::string read_text_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream stream;
    stream << input.rdbuf();
    return stream.str();
}

std::string read_binary_file(const std::filesystem::path& path, std::string& error) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        error = runtime_text("Runtime.Package.Error.OpenFileFailed", {{"path", path.string()}});
        return {};
    }

    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string hex_encode_bytes(const std::string& bytes) {
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (const unsigned char byte : bytes) {
        stream << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return stream.str();
}

bool append_runtime_artifact_digest(
    std::vector<RuntimeArtifactDigest>& digests,
    const std::string& path,
    std::string& error) {
    if (trim_copy(path).empty() || !std::filesystem::exists(path)) {
        return true;
    }

    const auto digest = security::sha256_hex_for_file(path);
    if (!digest.ok) {
        error = digest.error;
        return false;
    }

    const auto existing = std::find_if(digests.begin(), digests.end(), [&](const RuntimeArtifactDigest& entry) {
        return entry.path == path;
    });
    if (existing != digests.end()) {
        existing->sha256 = digest.hex_digest;
        return true;
    }

    digests.push_back({
        .path = path,
        .sha256 = digest.hex_digest
    });
    return true;
}

bool is_library_output_kind(const BuildOutputKind output_kind) {
    return output_kind == BuildOutputKind::dll ||
        output_kind == BuildOutputKind::fll ||
        output_kind == BuildOutputKind::ocx;
}

bool is_native_host_output_kind(const BuildOutputKind output_kind) {
    return output_kind == BuildOutputKind::executable ||
        output_kind == BuildOutputKind::unknown;
}

std::string resolve_output_file_name(const studio::StudioProjectWorkspace& workspace, const std::string& project_title) {
    const std::filesystem::path configured_output(workspace.build_plan.output_path);
    const std::string file_name = configured_output.filename().string();
    if (!trim_copy(file_name).empty()) {
        return file_name;
    }
    return sanitize_file_name(project_title) + ".exe";
}

BuildOutputKind infer_build_output_kind_from_output_path(const std::string& output_path) {
    const std::string extension = lowercase_copy(trim_copy(std::filesystem::path(output_path).extension().string()));
    if (extension == ".dll") {
        return BuildOutputKind::dll;
    }
    if (extension == ".app") {
        return BuildOutputKind::app;
    }
    if (extension == ".fll") {
        return BuildOutputKind::fll;
    }
    if (extension == ".fxp") {
        return BuildOutputKind::fxp;
    }
    if (extension == ".ocx") {
        return BuildOutputKind::ocx;
    }
    if (extension == ".exe") {
        return BuildOutputKind::executable;
    }
    return BuildOutputKind::unknown;
}

bool copy_file_if_exists(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    std::string& error) {
    if (!std::filesystem::exists(source)) {
        error = runtime_text("Runtime.Package.Error.SourceFileMissing", {{"path", source.string()}});
        return false;
    }

    std::error_code directory_error;
    std::filesystem::create_directories(destination.parent_path(), directory_error);
    if (directory_error) {
        error = runtime_text("Runtime.Package.Error.CreateDirectoryFailed", {{"path", destination.parent_path().string()}});
        return false;
    }

    std::error_code copy_error;
    std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
        error = runtime_text("Runtime.Package.Error.CopyFileFailed", {{"path", destination.string()}});
        return false;
    }

    return true;
}

bool validate_runtime_host_source_path(
    const RuntimePackagePlan& plan,
    const std::string& runtime_host_source_path,
    std::string& error) {
    if (runtime_host_source_path.empty()) {
        error = runtime_text("Runtime.Package.Error.RuntimeHostSourcePathEmpty");
        return false;
    }

    std::filesystem::path source(runtime_host_source_path);
    std::error_code canonical_error;
    source = std::filesystem::weakly_canonical(source, canonical_error);
    if (canonical_error) {
        error = runtime_text("Runtime.Package.Error.RuntimeHostSourcePathResolveFailed");
        return false;
    }

    if (!std::filesystem::exists(source) || !std::filesystem::is_regular_file(source)) {
        error = runtime_text("Runtime.Package.Error.RuntimeHostSourcePathNotRegularFile");
        return false;
    }

    if (plan.security_enabled) {
#ifdef _WIN32
        const std::string expected_file_name = "copperfin_runtime_host.exe";
#else
        const std::string expected_file_name = "copperfin_runtime_host";
#endif
        if (!source.is_absolute()) {
            error = runtime_text("Runtime.Package.Error.SecurityRequiresAbsoluteRuntimeHostPath");
            return false;
        }

        if (lowercase_copy(source.filename().string()) != expected_file_name) {
            error = runtime_text("Runtime.Package.Error.SecurityRequiresCanonicalRuntimeHostName");
            return false;
        }
    }

    return true;
}

std::string resolve_project_item_source(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectEntry& entry) {
    const std::filesystem::path base_dir = std::filesystem::path(document.path).parent_path();

    if (!entry.relative_path.empty()) {
        const std::filesystem::path from_relative = base_dir / entry.relative_path;
        if (std::filesystem::exists(from_relative)) {
            return from_relative.lexically_normal().string();
        }
    }

    if (entry.name.empty()) {
        return {};
    }

    const std::filesystem::path raw(entry.name);
    if (raw.is_absolute()) {
        if (std::filesystem::exists(raw)) {
            return raw.lexically_normal().string();
        }

        const std::filesystem::path from_filename = base_dir / raw.filename();
        return from_filename.lexically_normal().string();
    }

    return (base_dir / raw).lexically_normal().string();
}

std::string relative_asset_path(const studio::StudioProjectEntry& entry) {
    const std::string path = !entry.relative_path.empty() ? entry.relative_path : entry.name;
    if (!path.empty()) {
        return path;
    }
    return "record_" + std::to_string(entry.record_index) + ".asset";
}

std::string resolve_working_directory(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace) {
    const std::filesystem::path document_dir = std::filesystem::path(document.path).parent_path();
    if (!workspace.home_directory.empty()) {
        const std::filesystem::path home_directory(workspace.home_directory);
        if (std::filesystem::exists(home_directory)) {
            return home_directory.lexically_normal().string();
        }
    }
    return document_dir.lexically_normal().string();
}

std::string resolve_security_role(bool security_enabled) {
    if (!security_enabled) {
        return {};
    }

    std::string role;
#ifdef _WIN32
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, "COPPERFIN_SECURITY_ROLE") == 0 && raw != nullptr) {
        role = raw;
        std::free(raw);
    }
#else
    if (const char* raw = std::getenv("COPPERFIN_SECURITY_ROLE"); raw != nullptr) {
        role = raw;
    }
#endif

    role = trim_copy(role);
    if (!role.empty()) {
        return role;
    }

    return "developer";
}

bool is_extension_payload_path(const std::filesystem::path& path) {
    const std::string extension = lowercase_copy(trim_copy(path.extension().string()));
    return extension == ".dll" || extension == ".exe" || extension == ".vsix";
}

bool is_recognized_security_role(
    const security::NativeSecurityProfile& profile,
    const std::string& role_id) {
    return std::find_if(profile.roles.begin(), profile.roles.end(), [&](const security::NativeRole& role) {
               return role.id == role_id;
           }) != profile.roles.end();
}

bool is_prg_path(const std::string& value) {
    return lowercase_copy(trim_copy(std::filesystem::path(value).extension().string())) == ".prg";
}

bool is_xasset_path(const std::string& value) {
    const std::string extension = trim_copy(std::filesystem::path(value).extension().string());
    return extension == ".scx" ||
        extension == ".vcx" ||
        extension == ".frx" ||
        extension == ".lbx" ||
        extension == ".mnx";
}

bool should_stage_asset(const RuntimePackageAsset& asset) {
    return asset.exists && (!asset.excluded || asset.required_for_runtime);
}

std::vector<std::filesystem::path> infer_companion_source_paths(const std::filesystem::path& source) {
    std::vector<std::filesystem::path> companions;
    const std::string extension = trim_copy(lowercase_copy(source.extension().string()));
    const auto same_stem = [&](const char* companion_extension) {
        auto path = source;
        path.replace_extension(companion_extension);
        return path;
    };

    if (extension == ".pjx") {
        companions.push_back(same_stem(".pjt"));
    } else if (extension == ".scx") {
        companions.push_back(same_stem(".sct"));
    } else if (extension == ".vcx") {
        companions.push_back(same_stem(".vct"));
    } else if (extension == ".frx") {
        companions.push_back(same_stem(".frt"));
    } else if (extension == ".lbx") {
        companions.push_back(same_stem(".lbt"));
    } else if (extension == ".mnx") {
        companions.push_back(same_stem(".mnt"));
    } else if (extension == ".dbf") {
        companions.push_back(same_stem(".fpt"));
        companions.push_back(same_stem(".cdx"));
        companions.push_back(same_stem(".idx"));
        companions.push_back(same_stem(".ndx"));
        companions.push_back(same_stem(".mdx"));
    } else if (extension == ".dbc") {
        companions.push_back(same_stem(".dct"));
        companions.push_back(same_stem(".dcx"));
    }

    return companions;
}

void copy_companion_files_if_present(
    const RuntimePackageAsset& asset,
    std::vector<std::string>& warnings) {
    const std::filesystem::path source(asset.source_path);
    const std::filesystem::path staged(asset.staged_path);
    for (const auto& companion_source : infer_companion_source_paths(source)) {
        if (!std::filesystem::exists(companion_source)) {
            continue;
        }

        auto companion_destination = staged;
        companion_destination.replace_extension(companion_source.extension().string());
        std::string error;
        if (!copy_file_if_exists(companion_source, companion_destination, error)) {
            warnings.push_back(error);
        }
    }
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime

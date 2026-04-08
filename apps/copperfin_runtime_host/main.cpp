#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

std::string trim_copy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }
    return value;
}

std::string unescape_manifest_value(std::string value) {
    std::string result;
    result.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] == '\\' && (index + 1U) < value.size()) {
            const char next = value[index + 1U];
            if (next == 'n') {
                result.push_back('\n');
                ++index;
                continue;
            }
            if (next == 'r') {
                result.push_back('\r');
                ++index;
                continue;
            }
            if (next == '\\') {
                result.push_back('\\');
                ++index;
                continue;
            }
        }
        result.push_back(value[index]);
    }
    return result;
}

using ManifestMap = std::multimap<std::string, std::string>;

ManifestMap load_manifest(const std::string& path) {
    ManifestMap values;
    std::ifstream input(path, std::ios::binary);
    std::string line;
    while (std::getline(input, line)) {
        const auto delimiter = line.find('=');
        if (delimiter == std::string::npos) {
            continue;
        }

        const std::string key = trim_copy(line.substr(0U, delimiter));
        const std::string value = unescape_manifest_value(line.substr(delimiter + 1U));
        values.emplace(key, value);
    }
    return values;
}

std::string first_value(const ManifestMap& values, const std::string& key) {
    const auto found = values.find(key);
    return found == values.end() ? std::string{} : found->second;
}

std::vector<std::string> all_values(const ManifestMap& values, const std::string& key) {
    std::vector<std::string> result;
    const auto [begin, end] = values.equal_range(key);
    for (auto iterator = begin; iterator != end; ++iterator) {
        result.push_back(iterator->second);
    }
    return result;
}

void print_usage() {
    std::cout << "Usage: copperfin_runtime_host --manifest <path> [--debug]\n";
}

}  // namespace

int main(int argc, char** argv) {
    std::string manifest_path;
    bool debug_mode = false;

    for (int index = 1; index < argc; ++index) {
        const std::string arg = argv[index];
        if (arg == "--manifest" && (index + 1) < argc) {
            manifest_path = argv[++index];
        } else if (arg == "--debug") {
            debug_mode = true;
        } else {
            std::cout << "status: error\n";
            std::cout << "error: Unknown argument: " << arg << "\n";
            print_usage();
            return 2;
        }
    }

    if (manifest_path.empty()) {
        print_usage();
        return 2;
    }

    if (!std::filesystem::exists(manifest_path)) {
        std::cout << "status: error\n";
        std::cout << "error: Manifest file not found.\n";
        return 3;
    }

    const auto manifest = load_manifest(manifest_path);
    if (manifest.empty()) {
        std::cout << "status: error\n";
        std::cout << "error: Manifest is empty or invalid.\n";
        return 4;
    }

    const auto assets = all_values(manifest, "asset");
    const auto warnings = all_values(manifest, "warning");

    std::cout << "status: ok\n";
    std::cout << "runtime.mode: compatibility-launcher\n";
    std::cout << "project.title: " << first_value(manifest, "project_title") << "\n";
    std::cout << "startup.item: " << first_value(manifest, "startup_item") << "\n";
    std::cout << "startup.source: " << first_value(manifest, "startup_source") << "\n";
    std::cout << "working.directory: " << first_value(manifest, "working_directory") << "\n";
    std::cout << "security.enabled: " << first_value(manifest, "security_enabled") << "\n";
    std::cout << "security.mode: " << first_value(manifest, "security_mode") << "\n";
    std::cout << "dotnet.story: " << first_value(manifest, "dotnet_story") << "\n";
    std::cout << "asset.count: " << assets.size() << "\n";
    std::cout << "warning.count: " << warnings.size() << "\n";

    if (debug_mode) {
        std::cout << "debug.session: prepared\n";
        std::cout << "debug.breakpoint_support: false\n";
        std::cout << "debug.step_support: false\n";
        std::cout << "debug.note: Runtime execution is currently a compatibility launcher; full xBase stepping is not implemented yet.\n";
    } else {
        std::cout << "launch.note: Runtime package validated and launch context prepared.\n";
        std::cout << "launch.note: Full xBase execution is the next runtime milestone behind this host.\n";
    }

    return 0;
}

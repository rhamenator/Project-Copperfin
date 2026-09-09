// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/access_container.h"
#include "copperfin/platform/path.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>

namespace copperfin::vfp {

namespace {

// Minimum bytes needed to read the generation byte at offset 0x14.
constexpr std::size_t kHeaderProbeSize = 0x15U;

constexpr std::array<std::uint8_t, 4U> kLeadingBytes = {0x00U, 0x01U, 0x00U, 0x00U};
constexpr std::string_view kJetSignature = "Standard Jet DB";
constexpr std::string_view kAceSignature = "Standard ACE DB";
constexpr std::size_t kSignatureOffset = 4U;
constexpr std::size_t kGenerationByteOffset = 0x14U;

localization::LocalizedCatalog access_container_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        localization::LocalizedCatalog catalog;
    };

    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        localization::load_catalogs(
            localization::resolve_catalog_root(),
            localization::default_locale)};
    const std::filesystem::path locale_root = localization::resolve_catalog_root();
    const std::string locale = localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

std::string access_container_text(std::string_view key) {
    return access_container_catalog().translate(key);
}

bool matches_signature(const std::vector<std::uint8_t>& bytes, std::string_view signature) {
    if (bytes.size() < kSignatureOffset + signature.size()) {
        return false;
    }
    return std::equal(
        signature.begin(),
        signature.end(),
        bytes.begin() + static_cast<std::ptrdiff_t>(kSignatureOffset));
}

AccessContainerGeneration classify_generation(std::uint8_t generation_byte) {
    if (generation_byte == 0x00U) {
        return AccessContainerGeneration::jet3;
    }
    if (generation_byte == 0x01U) {
        return AccessContainerGeneration::jet4;
    }
    return AccessContainerGeneration::later;
}

}  // namespace

bool AccessContainerHeader::looks_like_access_container() const {
    return family != AccessContainerFamily::unknown;
}

const char* access_container_family_name(AccessContainerFamily family) {
    switch (family) {
        case AccessContainerFamily::unknown:
            return "unknown";
        case AccessContainerFamily::jet:
            return "jet";
        case AccessContainerFamily::ace:
            return "ace";
    }
    return "unknown";
}

const char* access_container_generation_name(AccessContainerGeneration generation) {
    switch (generation) {
        case AccessContainerGeneration::unknown:
            return "unknown";
        case AccessContainerGeneration::jet3:
            return "jet3";
        case AccessContainerGeneration::jet4:
            return "jet4";
        case AccessContainerGeneration::later:
            return "later";
    }
    return "unknown";
}

AccessContainerParseResult parse_access_container_header(const std::vector<std::uint8_t>& bytes) {
    if (bytes.size() < kHeaderProbeSize) {
        return {
            .ok = false,
            .header = {},
            .error = access_container_text("Vfp.AccessContainer.Error.HeaderTooSmall")
        };
    }

    if (!std::equal(kLeadingBytes.begin(), kLeadingBytes.end(), bytes.begin())) {
        return {
            .ok = false,
            .header = {},
            .error = access_container_text("Vfp.AccessContainer.Error.SignatureMismatch")
        };
    }

    AccessContainerHeader header;
    if (matches_signature(bytes, kJetSignature)) {
        header.family = AccessContainerFamily::jet;
    } else if (matches_signature(bytes, kAceSignature)) {
        header.family = AccessContainerFamily::ace;
    } else {
        return {
            .ok = false,
            .header = {},
            .error = access_container_text("Vfp.AccessContainer.Error.SignatureMismatch")
        };
    }

    header.generation_byte = bytes[kGenerationByteOffset];
    header.generation = classify_generation(header.generation_byte);

    return {.ok = true, .header = header, .error = {}};
}

AccessContainerParseResult parse_access_container_header_from_file(const std::string& path) {
    std::ifstream input(copperfin::platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        return {
            .ok = false,
            .header = {},
            .error = access_container_text("Vfp.AccessContainer.Error.OpenFileFailed")
        };
    }

    std::vector<std::uint8_t> bytes(kHeaderProbeSize, 0U);
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

    if (input.gcount() < static_cast<std::streamsize>(bytes.size())) {
        return {
            .ok = false,
            .header = {},
            .error = access_container_text("Vfp.AccessContainer.Error.ReadHeaderFailed")
        };
    }

    return parse_access_container_header(bytes);
}

}  // namespace copperfin::vfp

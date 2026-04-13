#include "copperfin/security/sha256.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#endif

#include <array>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>

namespace copperfin::security {

namespace {

std::string to_hex(const std::uint8_t* bytes, std::size_t size) {
    static constexpr char kHex[] = "0123456789abcdef";
    std::string result;
    result.reserve(size * 2U);
    for (std::size_t index = 0; index < size; ++index) {
        const auto value = bytes[index];
        result.push_back(kHex[(value >> 4U) & 0x0FU]);
        result.push_back(kHex[value & 0x0FU]);
    }
    return result;
}

#ifdef _WIN32
Sha256Result hash_bytes(const std::uint8_t* bytes, std::size_t size) {
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    std::vector<std::uint8_t> object_buffer;
    std::array<std::uint8_t, 32U> digest{};

    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) {
        return {.ok = false, .error = "BCryptOpenAlgorithmProvider failed."};
    }

    DWORD object_size = 0;
    DWORD bytes_written = 0;
    if (BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&object_size), sizeof(object_size), &bytes_written, 0) != 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return {.ok = false, .error = "BCryptGetProperty(BCRYPT_OBJECT_LENGTH) failed."};
    }

    object_buffer.resize(object_size);

    if (BCryptCreateHash(algorithm, &hash, object_buffer.data(), object_size, nullptr, 0, 0) != 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return {.ok = false, .error = "BCryptCreateHash failed."};
    }

    if (BCryptHashData(hash, const_cast<PUCHAR>(bytes), static_cast<ULONG>(size), 0) != 0) {
        BCryptDestroyHash(hash);
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return {.ok = false, .error = "BCryptHashData failed."};
    }

    if (BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0) != 0) {
        BCryptDestroyHash(hash);
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return {.ok = false, .error = "BCryptFinishHash failed."};
    }

    BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);

    return {.ok = true, .hex_digest = to_hex(digest.data(), digest.size())};
}
#endif

}  // namespace

Sha256Result sha256_hex_for_text(const std::string& text) {
#ifdef _WIN32
    return hash_bytes(reinterpret_cast<const std::uint8_t*>(text.data()), text.size());
#else
    return {.ok = false, .error = "SHA-256 is only implemented for Windows in this build."};
#endif
}

Sha256Result sha256_hex_for_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {.ok = false, .error = "Unable to open file for SHA-256: " + path};
    }

    std::ostringstream stream;
    stream << input.rdbuf();
    const std::string contents = stream.str();
    return sha256_hex_for_text(contents);
}

}  // namespace copperfin::security

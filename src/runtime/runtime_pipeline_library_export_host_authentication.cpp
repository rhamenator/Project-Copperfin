// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

void append_native_wrapper_host_authentication_source(std::ostringstream& stream) {
    stream << R"CF(
static std::uint32_t copperfin_runtime_bridge_rotr(
    const std::uint32_t value,
    const unsigned int bits) {
    return (value >> bits) | (value << (32U - bits));
}

static std::string copperfin_runtime_bridge_sha256_bytes(
    const std::vector<std::uint8_t>& input) {
    static constexpr std::array<std::uint32_t, 64U> round_constants{
        0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
        0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
        0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
        0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
        0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
        0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
        0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
        0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
        0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
        0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
        0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
        0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
        0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
        0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
        0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
        0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U};
    std::vector<std::uint8_t> bytes = input;
    const std::uint64_t bit_length = static_cast<std::uint64_t>(bytes.size()) * 8ULL;
    bytes.push_back(0x80U);
    while ((bytes.size() % 64U) != 56U) {
        bytes.push_back(0U);
    }
    for (int shift = 56; shift >= 0; shift -= 8) {
        bytes.push_back(static_cast<std::uint8_t>((bit_length >> shift) & 0xffU));
    }

    std::array<std::uint32_t, 8U> state{
        0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
        0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U};
    for (std::size_t offset = 0; offset < bytes.size(); offset += 64U) {
        std::array<std::uint32_t, 64U> words{};
        for (std::size_t index = 0; index < 16U; ++index) {
            const std::size_t base = offset + index * 4U;
            words[index] = (static_cast<std::uint32_t>(bytes[base]) << 24U) |
                (static_cast<std::uint32_t>(bytes[base + 1U]) << 16U) |
                (static_cast<std::uint32_t>(bytes[base + 2U]) << 8U) |
                static_cast<std::uint32_t>(bytes[base + 3U]);
        }
        for (std::size_t index = 16U; index < words.size(); ++index) {
            const auto first = words[index - 15U];
            const auto second = words[index - 2U];
            const auto small_sigma_one = copperfin_runtime_bridge_rotr(second, 17U) ^
                copperfin_runtime_bridge_rotr(second, 19U) ^ (second >> 10U);
            const auto small_sigma_zero = copperfin_runtime_bridge_rotr(first, 7U) ^
                copperfin_runtime_bridge_rotr(first, 18U) ^ (first >> 3U);
            words[index] = words[index - 16U] + small_sigma_zero +
                words[index - 7U] + small_sigma_one;
        }

        std::uint32_t a = state[0];
        std::uint32_t b = state[1];
        std::uint32_t c = state[2];
        std::uint32_t d = state[3];
        std::uint32_t e = state[4];
        std::uint32_t f = state[5];
        std::uint32_t g = state[6];
        std::uint32_t h = state[7];
        for (std::size_t index = 0; index < words.size(); ++index) {
            const auto big_sigma_one = copperfin_runtime_bridge_rotr(e, 6U) ^
                copperfin_runtime_bridge_rotr(e, 11U) ^ copperfin_runtime_bridge_rotr(e, 25U);
            const auto choose = (e & f) ^ ((~e) & g);
            const auto temporary_one = h + big_sigma_one + choose +
                round_constants[index] + words[index];
            const auto big_sigma_zero = copperfin_runtime_bridge_rotr(a, 2U) ^
                copperfin_runtime_bridge_rotr(a, 13U) ^ copperfin_runtime_bridge_rotr(a, 22U);
            const auto majority = (a & b) ^ (a & c) ^ (b & c);
            const auto temporary_two = big_sigma_zero + majority;
            h = g;
            g = f;
            f = e;
            e = d + temporary_one;
            d = c;
            c = b;
            b = a;
            a = temporary_one + temporary_two;
        }
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }

    static constexpr char hex[] = "0123456789abcdef";
    std::string digest;
    digest.reserve(64U);
    for (const auto word : state) {
        for (int shift = 28; shift >= 0; shift -= 4) {
            digest.push_back(hex[(word >> shift) & 0x0fU]);
        }
    }
    return digest;
}

static std::string copperfin_runtime_bridge_manifest_value(
    const std::filesystem::path& manifest_path,
    const std::string& key) {
    std::ifstream input(manifest_path, std::ios::binary);
    if (!input) {
        return {};
    }
    const std::string prefix = key + "=";
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind(prefix, 0U) == 0U) {
            return line.substr(prefix.size());
        }
    }
    return {};
}

static bool copperfin_runtime_bridge_digest_matches(
    const std::filesystem::path& manifest_path,
    const std::vector<std::uint8_t>& bytes) {
    const auto expected = copperfin_runtime_bridge_manifest_value(
        manifest_path,
        "runtime_host_sha256");
    return !expected.empty() &&
        expected == copperfin_runtime_bridge_sha256_bytes(bytes);
}

#if defined(_WIN32)
static bool copperfin_runtime_bridge_read_verified_host(
    const std::filesystem::path& host_path,
    const std::filesystem::path& manifest_path,
    HANDLE& verified_host) {
    verified_host = INVALID_HANDLE_VALUE;
    const auto wide_path = host_path.wstring();
    const HANDLE handle = CreateFileW(
        wide_path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    const DWORD attributes = GetFileAttributesW(wide_path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) != 0U) {
        CloseHandle(handle);
        return false;
    }
    std::vector<std::uint8_t> bytes;
    std::array<std::uint8_t, 8192U> buffer{};
    for (;;) {
        DWORD read = 0U;
        if (!ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &read, nullptr)) {
            CloseHandle(handle);
            return false;
        }
        bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + read);
        if (read == 0U) {
            break;
        }
    }
    LARGE_INTEGER origin{};
    if (!SetFilePointerEx(handle, origin, nullptr, FILE_BEGIN) ||
        !copperfin_runtime_bridge_digest_matches(manifest_path, bytes)) {
        CloseHandle(handle);
        return false;
    }
    verified_host = handle;
    return true;
}
#else
static bool copperfin_runtime_bridge_read_verified_host(
    const std::filesystem::path& host_path,
    const std::filesystem::path& manifest_path,
    int& verified_host) {
    verified_host = -1;
    std::error_code status_error;
    const auto status = std::filesystem::symlink_status(host_path, status_error);
    if (status_error || status.type() != std::filesystem::file_type::regular) {
        return false;
    }
    const int descriptor = open(host_path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (descriptor < 0) {
        return false;
    }
    struct stat before{};
    if (fstat(descriptor, &before) != 0 || !S_ISREG(before.st_mode) || before.st_nlink != 1) {
        close(descriptor);
        return false;
    }
    std::vector<std::uint8_t> bytes;
    std::array<std::uint8_t, 8192U> buffer{};
    for (;;) {
        const ssize_t read = ::read(descriptor, buffer.data(), buffer.size());
        if (read < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(descriptor);
            return false;
        }
        if (read == 0) {
            break;
        }
        bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + read);
    }
    struct stat after{};
    if (fstat(descriptor, &after) != 0 ||
        before.st_dev != after.st_dev || before.st_ino != after.st_ino ||
        before.st_size != after.st_size ||
        !copperfin_runtime_bridge_digest_matches(manifest_path, bytes)) {
        close(descriptor);
        return false;
    }
    verified_host = descriptor;
    return true;
}
#endif

)CF";
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime

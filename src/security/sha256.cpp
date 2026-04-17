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

#ifndef _WIN32
std::uint32_t rotate_right(std::uint32_t value, unsigned int bits) {
    return (value >> bits) | (value << (32U - bits));
}

std::uint32_t read_be_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
        (static_cast<std::uint32_t>(bytes[offset + 1U]) << 16U) |
        (static_cast<std::uint32_t>(bytes[offset + 2U]) << 8U) |
        static_cast<std::uint32_t>(bytes[offset + 3U]);
}

void write_be_u32(std::array<std::uint8_t, 32U>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value & 0xFFU);
}

Sha256Result hash_bytes(const std::uint8_t* bytes, std::size_t size) {
    static constexpr std::array<std::uint32_t, 64U> kRoundConstants{
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
        0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
    };

    std::vector<std::uint8_t> message(bytes, bytes + size);
    const std::uint64_t bit_length = static_cast<std::uint64_t>(size) * 8ULL;
    message.push_back(0x80U);
    while ((message.size() % 64U) != 56U) {
        message.push_back(0U);
    }
    for (int shift = 56; shift >= 0; shift -= 8) {
        message.push_back(static_cast<std::uint8_t>((bit_length >> shift) & 0xFFU));
    }

    std::array<std::uint32_t, 8U> hash{
        0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
        0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U
    };

    for (std::size_t offset = 0U; offset < message.size(); offset += 64U) {
        std::array<std::uint32_t, 64U> schedule{};
        for (std::size_t index = 0U; index < 16U; ++index) {
            schedule[index] = read_be_u32(message, offset + index * 4U);
        }
        for (std::size_t index = 16U; index < 64U; ++index) {
            const std::uint32_t s0 = rotate_right(schedule[index - 15U], 7U) ^
                rotate_right(schedule[index - 15U], 18U) ^
                (schedule[index - 15U] >> 3U);
            const std::uint32_t s1 = rotate_right(schedule[index - 2U], 17U) ^
                rotate_right(schedule[index - 2U], 19U) ^
                (schedule[index - 2U] >> 10U);
            schedule[index] = schedule[index - 16U] + s0 + schedule[index - 7U] + s1;
        }

        std::uint32_t a = hash[0];
        std::uint32_t b = hash[1];
        std::uint32_t c = hash[2];
        std::uint32_t d = hash[3];
        std::uint32_t e = hash[4];
        std::uint32_t f = hash[5];
        std::uint32_t g = hash[6];
        std::uint32_t h = hash[7];

        for (std::size_t index = 0U; index < 64U; ++index) {
            const std::uint32_t s1 = rotate_right(e, 6U) ^ rotate_right(e, 11U) ^ rotate_right(e, 25U);
            const std::uint32_t ch = (e & f) ^ ((~e) & g);
            const std::uint32_t temp1 = h + s1 + ch + kRoundConstants[index] + schedule[index];
            const std::uint32_t s0 = rotate_right(a, 2U) ^ rotate_right(a, 13U) ^ rotate_right(a, 22U);
            const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t temp2 = s0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
        hash[5] += f;
        hash[6] += g;
        hash[7] += h;
    }

    std::array<std::uint8_t, 32U> digest{};
    for (std::size_t index = 0U; index < hash.size(); ++index) {
        write_be_u32(digest, index * 4U, hash[index]);
    }

    return {.ok = true, .hex_digest = to_hex(digest.data(), digest.size())};
}
#endif

}  // namespace

Sha256Result sha256_hex_for_text(const std::string& text) {
    return hash_bytes(reinterpret_cast<const std::uint8_t*>(text.data()), text.size());
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

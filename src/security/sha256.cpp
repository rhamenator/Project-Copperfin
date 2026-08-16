// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/sha256.h"

#include "sha256_native.h"

#include "copperfin/platform/path.h"
#include "localized_text.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#endif

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <new>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace copperfin::security {

namespace {

constexpr std::size_t kSha256BlockSize = 64U;
constexpr std::size_t kFileReadChunkSize = 64U * 1024U;

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

Sha256Result file_read_failure(const std::string& path) {
    return {
        .ok = false,
        .hex_digest = {},
        .error = security_text(
            "Security.Sha256.Error.OpenFileFailed",
            {{"path", path}})};
}

#ifdef _WIN32

struct WindowsSha256Context {
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    std::vector<std::uint8_t> object_buffer;

    ~WindowsSha256Context() {
        if (hash != nullptr) {
            BCryptDestroyHash(hash);
        }
        if (algorithm != nullptr) {
            BCryptCloseAlgorithmProvider(algorithm, 0);
        }
    }

    WindowsSha256Context() = default;
    WindowsSha256Context(const WindowsSha256Context&) = delete;
    WindowsSha256Context& operator=(const WindowsSha256Context&) = delete;
};

Sha256Result initialize_hash(WindowsSha256Context& context) {
    if (BCryptOpenAlgorithmProvider(
            &context.algorithm,
            BCRYPT_SHA256_ALGORITHM,
            nullptr,
            0) != 0) {
        return {
            .ok = false,
            .hex_digest = {},
            .error = security_text(
                "Security.Sha256.Error.BCryptOpenAlgorithmProviderFailed")};
    }

    DWORD object_size = 0;
    DWORD bytes_written = 0;
    if (BCryptGetProperty(
            context.algorithm,
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&object_size),
            sizeof(object_size),
            &bytes_written,
            0) != 0) {
        return {
            .ok = false,
            .hex_digest = {},
            .error = security_text(
                "Security.Sha256.Error.BCryptGetPropertyObjectLengthFailed")};
    }

    context.object_buffer.resize(object_size);
    if (BCryptCreateHash(
            context.algorithm,
            &context.hash,
            context.object_buffer.data(),
            object_size,
            nullptr,
            0,
            0) != 0) {
        return {
            .ok = false,
            .hex_digest = {},
            .error = security_text(
                "Security.Sha256.Error.BCryptCreateHashFailed")};
    }
    return {.ok = true, .hex_digest = {}, .error = {}};
}

bool update_hash(
    WindowsSha256Context& context,
    const std::uint8_t* bytes,
    std::size_t size) {
    while (size > 0U) {
        const ULONG chunk_size = static_cast<ULONG>(std::min<std::size_t>(
            size,
            std::numeric_limits<ULONG>::max()));
        if (BCryptHashData(
                context.hash,
                const_cast<PUCHAR>(bytes),
                chunk_size,
                0) != 0) {
            return false;
        }
        bytes += chunk_size;
        size -= chunk_size;
    }
    return true;
}

Sha256Result finish_hash(WindowsSha256Context& context) {
    std::array<std::uint8_t, 32U> digest{};
    if (BCryptFinishHash(
            context.hash,
            digest.data(),
            static_cast<ULONG>(digest.size()),
            0) != 0) {
        return {
            .ok = false,
            .hex_digest = {},
            .error = security_text(
                "Security.Sha256.Error.BCryptFinishHashFailed")};
    }
    return {
        .ok = true,
        .hex_digest = to_hex(digest.data(), digest.size()),
        .error = {}};
}

Sha256Result hash_bytes(const std::uint8_t* bytes, std::size_t size) {
    WindowsSha256Context context;
    const auto initialized = initialize_hash(context);
    if (!initialized.ok) {
        return initialized;
    }
    if (!update_hash(context, bytes, size)) {
        return {
            .ok = false,
            .hex_digest = {},
            .error = security_text(
                "Security.Sha256.Error.BCryptHashDataFailed")};
    }
    return finish_hash(context);
}

Sha256Result hash_file_stream(std::ifstream& input, const std::string& path) {
    WindowsSha256Context context;
    const auto initialized = initialize_hash(context);
    if (!initialized.ok) {
        return initialized;
    }

    std::array<std::uint8_t, kFileReadChunkSize> buffer{};
    for (;;) {
        input.read(
            reinterpret_cast<char*>(buffer.data()),
            static_cast<std::streamsize>(buffer.size()));
        const auto count = input.gcount();
        if (count > 0 && !update_hash(
                context,
                buffer.data(),
                static_cast<std::size_t>(count))) {
            return {
                .ok = false,
                .hex_digest = {},
                .error = security_text(
                    "Security.Sha256.Error.BCryptHashDataFailed")};
        }
        if (input.eof()) {
            break;
        }
        if (!input) {
            return file_read_failure(path);
        }
    }
    return finish_hash(context);
}

NativeFileSha256SnapshotResult snapshot_native_file(
    const std::intptr_t native_handle,
    const std::uint64_t maximum_bytes,
    const bool retain_snapshot) {
    const HANDLE handle = reinterpret_cast<HANDLE>(native_handle);
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
        return {.digest = file_read_failure({}), .bytes = {}};
    }
    LARGE_INTEGER beginning{};
    if (::SetFilePointerEx(handle, beginning, nullptr, FILE_BEGIN) == 0) {
        return {.digest = file_read_failure({}), .bytes = {}};
    }

    WindowsSha256Context context;
    const auto initialized = initialize_hash(context);
    if (!initialized.ok) {
        return {.digest = initialized, .bytes = {}};
    }
    std::array<std::uint8_t, kFileReadChunkSize> buffer{};
    std::vector<std::uint8_t> bytes;
    std::uint64_t total = 0U;
    for (;;) {
        DWORD count = 0U;
        if (::ReadFile(
                handle,
                buffer.data(),
                static_cast<DWORD>(buffer.size()),
                &count,
                nullptr) == 0) {
            return {.digest = file_read_failure({}), .bytes = {}};
        }
        if (count == 0U) {
            break;
        }
        if (total > maximum_bytes ||
            static_cast<std::uint64_t>(count) > maximum_bytes - total ||
            !update_hash(context, buffer.data(), count)) {
            return {.digest = file_read_failure({}), .bytes = {}};
        }
        if (retain_snapshot) {
            try {
                bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + count);
            } catch (const std::bad_alloc&) {
                return {.digest = file_read_failure({}), .bytes = {}};
            }
        }
        total += count;
    }
    if (::SetFilePointerEx(handle, beginning, nullptr, FILE_BEGIN) == 0) {
        return {.digest = file_read_failure({}), .bytes = {}};
    }
    return {.digest = finish_hash(context), .bytes = std::move(bytes)};
}

#else

std::uint32_t rotate_right(std::uint32_t value, unsigned int bits) {
    return (value >> bits) | (value << (32U - bits));
}

std::uint32_t read_be_u32(const std::uint8_t* bytes) {
    return (static_cast<std::uint32_t>(bytes[0]) << 24U) |
        (static_cast<std::uint32_t>(bytes[1]) << 16U) |
        (static_cast<std::uint32_t>(bytes[2]) << 8U) |
        static_cast<std::uint32_t>(bytes[3]);
}

void write_be_u32(
    std::array<std::uint8_t, 32U>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value & 0xFFU);
}

struct PortableSha256Context {
    std::array<std::uint32_t, 8U> state{
        0x6a09e667U,
        0xbb67ae85U,
        0x3c6ef372U,
        0xa54ff53aU,
        0x510e527fU,
        0x9b05688cU,
        0x1f83d9abU,
        0x5be0cd19U};
    std::array<std::uint8_t, kSha256BlockSize> buffered{};
    std::size_t buffered_size = 0U;
    std::uint64_t total_bytes = 0U;
};

void process_block(PortableSha256Context& context, const std::uint8_t* block) {
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
        0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U};

    std::array<std::uint32_t, 64U> schedule{};
    for (std::size_t index = 0U; index < 16U; ++index) {
        schedule[index] = read_be_u32(block + index * 4U);
    }
    for (std::size_t index = 16U; index < 64U; ++index) {
        const std::uint32_t s0 = rotate_right(schedule[index - 15U], 7U) ^
            rotate_right(schedule[index - 15U], 18U) ^
            (schedule[index - 15U] >> 3U);
        const std::uint32_t s1 = rotate_right(schedule[index - 2U], 17U) ^
            rotate_right(schedule[index - 2U], 19U) ^
            (schedule[index - 2U] >> 10U);
        schedule[index] = schedule[index - 16U] + s0 +
            schedule[index - 7U] + s1;
    }

    std::uint32_t a = context.state[0];
    std::uint32_t b = context.state[1];
    std::uint32_t c = context.state[2];
    std::uint32_t d = context.state[3];
    std::uint32_t e = context.state[4];
    std::uint32_t f = context.state[5];
    std::uint32_t g = context.state[6];
    std::uint32_t h = context.state[7];

    for (std::size_t index = 0U; index < 64U; ++index) {
        const std::uint32_t s1 = rotate_right(e, 6U) ^
            rotate_right(e, 11U) ^ rotate_right(e, 25U);
        const std::uint32_t ch = (e & f) ^ ((~e) & g);
        const std::uint32_t temp1 = h + s1 + ch +
            kRoundConstants[index] + schedule[index];
        const std::uint32_t s0 = rotate_right(a, 2U) ^
            rotate_right(a, 13U) ^ rotate_right(a, 22U);
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

    context.state[0] += a;
    context.state[1] += b;
    context.state[2] += c;
    context.state[3] += d;
    context.state[4] += e;
    context.state[5] += f;
    context.state[6] += g;
    context.state[7] += h;
}

void update_hash(
    PortableSha256Context& context,
    const std::uint8_t* bytes,
    std::size_t size) {
    context.total_bytes += static_cast<std::uint64_t>(size);

    if (context.buffered_size != 0U) {
        const std::size_t copied = std::min(
            size,
            kSha256BlockSize - context.buffered_size);
        std::copy_n(
            bytes,
            copied,
            context.buffered.begin() +
                static_cast<std::ptrdiff_t>(context.buffered_size));
        context.buffered_size += copied;
        bytes += copied;
        size -= copied;
        if (context.buffered_size == kSha256BlockSize) {
            process_block(context, context.buffered.data());
            context.buffered_size = 0U;
        }
    }

    while (size >= kSha256BlockSize) {
        process_block(context, bytes);
        bytes += kSha256BlockSize;
        size -= kSha256BlockSize;
    }
    if (size != 0U) {
        std::copy_n(bytes, size, context.buffered.begin());
        context.buffered_size = size;
    }
}

Sha256Result finish_hash(PortableSha256Context& context) {
    const std::uint64_t bit_length = context.total_bytes * 8ULL;
    context.buffered[context.buffered_size++] = 0x80U;
    if (context.buffered_size > 56U) {
        std::fill(
            context.buffered.begin() +
                static_cast<std::ptrdiff_t>(context.buffered_size),
            context.buffered.end(),
            0U);
        process_block(context, context.buffered.data());
        context.buffered_size = 0U;
    }
    std::fill(
        context.buffered.begin() +
            static_cast<std::ptrdiff_t>(context.buffered_size),
        context.buffered.begin() + 56,
        0U);
    for (std::size_t index = 0U; index < 8U; ++index) {
        const unsigned int shift = static_cast<unsigned int>((7U - index) * 8U);
        context.buffered[56U + index] =
            static_cast<std::uint8_t>((bit_length >> shift) & 0xFFU);
    }
    process_block(context, context.buffered.data());

    std::array<std::uint8_t, 32U> digest{};
    for (std::size_t index = 0U; index < context.state.size(); ++index) {
        write_be_u32(digest, index * 4U, context.state[index]);
    }
    return {
        .ok = true,
        .hex_digest = to_hex(digest.data(), digest.size()),
        .error = {}};
}

Sha256Result hash_bytes(const std::uint8_t* bytes, std::size_t size) {
    PortableSha256Context context;
    update_hash(context, bytes, size);
    return finish_hash(context);
}

Sha256Result hash_file_stream(std::ifstream& input, const std::string& path) {
    PortableSha256Context context;
    std::array<std::uint8_t, kFileReadChunkSize> buffer{};
    for (;;) {
        input.read(
            reinterpret_cast<char*>(buffer.data()),
            static_cast<std::streamsize>(buffer.size()));
        const auto count = input.gcount();
        if (count > 0) {
            update_hash(
                context,
                buffer.data(),
                static_cast<std::size_t>(count));
        }
        if (input.eof()) {
            break;
        }
        if (!input) {
            return file_read_failure(path);
        }
    }
    return finish_hash(context);
}

NativeFileSha256SnapshotResult snapshot_native_file(
    const std::intptr_t native_handle,
    const std::uint64_t maximum_bytes,
    const bool retain_snapshot) {
    const int handle = static_cast<int>(native_handle);
    if (handle < 0) {
        return {.digest = file_read_failure({}), .bytes = {}};
    }
    PortableSha256Context context;
    std::array<std::uint8_t, kFileReadChunkSize> buffer{};
    std::vector<std::uint8_t> bytes;
    std::uint64_t total = 0U;
    std::uint64_t offset = 0U;
    for (;;) {
        if (offset > static_cast<std::uint64_t>(
                std::numeric_limits<off_t>::max())) {
            return {.digest = file_read_failure({}), .bytes = {}};
        }
        const ssize_t count = ::pread(
            handle,
            buffer.data(),
            buffer.size(),
            static_cast<off_t>(offset));
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            return {.digest = file_read_failure({}), .bytes = {}};
        }
        if (count == 0) {
            break;
        }
        const auto count_u64 = static_cast<std::uint64_t>(count);
        if (total > maximum_bytes || count_u64 > maximum_bytes - total ||
            offset > static_cast<std::uint64_t>(
                std::numeric_limits<off_t>::max()) - count_u64) {
            return {.digest = file_read_failure({}), .bytes = {}};
        }
        update_hash(context, buffer.data(), static_cast<std::size_t>(count));
        if (retain_snapshot) {
            try {
                bytes.insert(
                    bytes.end(), buffer.begin(), buffer.begin() + count);
            } catch (const std::bad_alloc&) {
                return {.digest = file_read_failure({}), .bytes = {}};
            }
        }
        total += count_u64;
        offset += count_u64;
    }
    return {.digest = finish_hash(context), .bytes = std::move(bytes)};
}

#endif

}  // namespace

Sha256Result sha256_hex_for_text(const std::string& text) {
    return hash_bytes(
        reinterpret_cast<const std::uint8_t*>(text.data()),
        text.size());
}

Sha256Result sha256_hex_for_file(const std::string& path) {
    std::ifstream input(
        copperfin::platform::path_from_utf8_string(path),
        std::ios::binary);
    if (!input) {
        return file_read_failure(path);
    }
    return hash_file_stream(input, path);
}

Sha256Result sha256_hex_for_native_file(
    const std::intptr_t native_handle,
    const std::uint64_t maximum_bytes) {
    return snapshot_native_file(native_handle, maximum_bytes, false).digest;
}

NativeFileSha256SnapshotResult sha256_snapshot_for_native_file(
    const std::intptr_t native_handle,
    const std::uint64_t maximum_bytes) {
    return snapshot_native_file(native_handle, maximum_bytes, true);
}

Sha256Result sha256_hex_for_native_bytes(
    const std::vector<std::uint8_t>& bytes) {
#if defined(_WIN32)
    WindowsSha256Context context;
    const auto initialized = initialize_hash(context);
    if (!initialized.ok) {
        return initialized;
    }
    if (!bytes.empty() && !update_hash(context, bytes.data(), bytes.size())) {
        return file_read_failure({});
    }
    return finish_hash(context);
#else
    PortableSha256Context context;
    if (!bytes.empty()) {
        update_hash(context, bytes.data(), bytes.size());
    }
    return finish_hash(context);
#endif
}

}  // namespace copperfin::security

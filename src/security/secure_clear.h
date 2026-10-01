// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace copperfin::security {

// Best-effort wipe of a buffer that held sensitive data, written so the compiler cannot prove the
// stores dead and drop them before the buffer is freed or reused. A plain memset() immediately
// before a free() is exactly what optimizers remove; calling memset through a volatile function
// pointer is opaque to them, and the trailing signal fence stops reordering across the wipe.
//
// This clears one buffer. It does not chase copies made elsewhere (caller strings, allocator
// free lists, swap, core dumps), so it is defense in depth for the buffers Copperfin itself
// retains, not a guarantee that a value is unrecoverable from process memory.
inline void secure_clear(void* data, const std::size_t size) noexcept {
    if (data == nullptr || size == 0U) {
        return;
    }
    using MemsetFunction = void* (*)(void*, int, std::size_t);
    static volatile MemsetFunction volatile_memset = &std::memset;
    (void)volatile_memset(data, 0, size);
#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("" : : "r"(data) : "memory");
#endif
}

inline void secure_clear(std::vector<std::uint8_t>& bytes) noexcept {
    secure_clear(bytes.data(), bytes.size());
}

inline void secure_clear(std::string& text) noexcept {
    secure_clear(text.data(), text.size());
}

// Wipes the listed buffers when the scope exits, including on an early return or an exception.
class SecureClearGuard final {
public:
    SecureClearGuard() = default;
    SecureClearGuard(const SecureClearGuard&) = delete;
    SecureClearGuard& operator=(const SecureClearGuard&) = delete;

    void add(std::vector<std::uint8_t>& buffer) { buffers_.push_back(&buffer); }
    void set_active(const bool active) noexcept { active_ = active; }

    ~SecureClearGuard() {
        if (!active_) {
            return;
        }
        for (std::vector<std::uint8_t>* buffer : buffers_) {
            secure_clear(*buffer);
        }
    }

private:
    std::vector<std::vector<std::uint8_t>*> buffers_;
    bool active_ = true;
};

}  // namespace copperfin::security

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

struct CdxHeader {
    std::array<std::uint16_t, 8> raw_words{};
    std::uint16_t root_node_offset = 0;
    std::uint16_t next_free_node_offset = 0;
    std::uint16_t key_length_hint = 0;
    std::uint16_t key_pool_length_hint = 0;
    std::uint16_t page_size = 512;
    std::uint64_t file_size = 0;

    [[nodiscard]] bool looks_like_cdx() const;
};

struct CdxParseResult {
    bool ok = false;
    CdxHeader header{};
    std::string error;
};

CdxParseResult parse_cdx_header(const std::vector<std::uint8_t>& bytes, std::uint64_t file_size);
CdxParseResult parse_cdx_header_from_file(const std::string& path);

}  // namespace copperfin::vfp

#include "copperfin/vfp/cdx_header.h"

#include <filesystem>
#include <fstream>

namespace copperfin::vfp {

namespace {

std::uint16_t read_le_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1]) << 8U);
}

}  // namespace

bool CdxHeader::looks_like_cdx() const {
    if (file_size < page_size || page_size == 0U) {
        return false;
    }
    if ((file_size % page_size) != 0U) {
        return false;
    }
    if (root_node_offset == 0U || (root_node_offset % page_size) != 0U) {
        return false;
    }
    return root_node_offset < file_size;
}

CdxParseResult parse_cdx_header(const std::vector<std::uint8_t>& bytes, std::uint64_t file_size) {
    if (bytes.size() < 16U) {
        return {.ok = false, .error = "File is smaller than the minimum CDX header probe size (16 bytes)."};
    }

    CdxHeader header;
    header.file_size = file_size;

    for (std::size_t index = 0; index < header.raw_words.size(); ++index) {
        header.raw_words[index] = read_le_u16(bytes, index * 2U);
    }

    header.root_node_offset = header.raw_words[0];
    header.next_free_node_offset = header.raw_words[1];
    header.key_length_hint = header.raw_words[6];
    header.key_pool_length_hint = header.raw_words[7];

    if (!header.looks_like_cdx()) {
        return {.ok = false, .header = header, .error = "Header values do not look like a CDX-family index file."};
    }

    return {.ok = true, .header = header};
}

CdxParseResult parse_cdx_header_from_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {.ok = false, .error = "Unable to open file."};
    }

    const std::uint64_t file_size = static_cast<std::uint64_t>(std::filesystem::file_size(path));
    std::vector<std::uint8_t> bytes(16U, 0U);
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

    if (input.gcount() < static_cast<std::streamsize>(bytes.size())) {
        return {.ok = false, .error = "Unable to read a complete 16-byte CDX header probe."};
    }

    return parse_cdx_header(bytes, file_size);
}

}  // namespace copperfin::vfp

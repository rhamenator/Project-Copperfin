// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
//
// Coverage-guided libFuzzer harness for Copperfin's own DBF header parser
// (copperfin::vfp::parse_dbf_header). This complements the bounded,
// deterministic sweeps in tests/test_dbf_header_robustness.cpp with an
// open-ended, mutation-based search over the same input space, seeded from
// the synthetic corpus in tests/fuzz/corpus/dbf_header/. It targets no
// third-party system, downloads no external corpus, and makes no network
// access. This is optional defense-in-depth testing, not a v1 or RC release
// gate, and it does not claim exhaustive format or security coverage -- see
// docs/65-dbf-header-parser-fuzzing.md.
//
// Unlike test_dbf_header_robustness.cpp, this harness is deliberately
// open-ended: libFuzzer's mutation engine and corpus scheduling -- not a
// fixed set of compile-time constants -- decide what input is tried next.
// The CTest-invoked smoke run bounds this with a fixed wall-clock budget and
// a fixed -seed so CI runs are reproducible for a given libFuzzer version and
// corpus; longer, exploratory runs are a manual/local activity (see the doc).

#include "copperfin/vfp/dbf_header.h"

#include <cstddef>
#include <cstdint>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::vector<std::uint8_t> bytes(data, data + size);
    const copperfin::vfp::DbfParseResult result = copperfin::vfp::parse_dbf_header(bytes);
    if (!result.ok) {
        return 0;
    }

    // Every derived accessor reachable from a successful parse must remain
    // safe to call for any header the parser accepted -- the same surface
    // test_dbf_header_robustness.cpp exercises deterministically for its
    // fixed case set.
    (void)result.header.version_description();
    (void)result.header.format_family();
    (void)copperfin::vfp::dbf_format_family_name(result.header.format_family());
    (void)result.header.has_database_container();
    (void)result.header.has_production_index();
    (void)result.header.has_structural_cdx();
    (void)result.header.has_memo_file();
    (void)result.header.last_update_iso8601();
    (void)copperfin::vfp::dbf_code_page_from_mark(result.header.code_page_mark);
    return 0;
}

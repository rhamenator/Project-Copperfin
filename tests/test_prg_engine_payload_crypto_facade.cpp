// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_prg_payload_crypto_facade() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_payload_crypto_facade";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "payload_crypto.prg";
    write_text(
        main_path,
        "cPayload = 'Copperfin'\n"
        "cDigest = CFSHA256(cPayload)\n"
        "cEncoded = CFBASE64ENCODE(cPayload)\n"
        "cDecoded = CFBASE64DECODE(cEncoded)\n"
        "cBinary = CHR(0) + CHR(255) + CHR(16)\n"
        "cBinaryDigest = CFSHA256(cBinary)\n"
        "cBinaryEncoded = CFBASE64ENCODE(cBinary)\n"
        "cBinaryDecoded = CFBASE64DECODE(cBinaryEncoded)\n"
        "cHmacKey = CHR(0) + CHR(255)\n"
        "cHmacData = CHR(16) + 'Copperfin'\n"
        "cHmac = CFHMACSHA256(cHmacKey, cHmacData)\n"
        "lHmacMatch = CFHMACVERIFY(cHmacKey, cHmacData, cHmac)\n"
        "lHmacMismatch = CFHMACVERIFY(cHmacKey, cHmacData + 'x', cHmac)\n"
        "nInvalidHmac = CFHMACVERIFY(cHmacKey, cHmacData, UPPER(cHmac), 37)\n"
        "nNonstrHmac = CFHMACSHA256(42, cHmacData, 73)\n"
        "nBinaryLength = LEN(cBinaryDecoded)\n"
        "nFirst = ASC(SUBSTR(cBinaryDecoded, 1, 1))\n"
        "nSecond = ASC(SUBSTR(cBinaryDecoded, 2, 1))\n"
        "nThird = ASC(SUBSTR(cBinaryDecoded, 3, 1))\n"
        "cInvalid = CFBASE64DECODE('Zh==', 'fallback')\n"
        "nTypedInvalid = CFBASE64DECODE('Z g==', 42)\n"
        "nTypedNonstr = CFSHA256(42, 84)\n"
        "cMissingHash = CFSHA256()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("PRG payload-crypto facade should complete: ") + state.message);

    const auto formatted = [&](const std::string& name) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        return found == state.globals.end()
            ? std::string{}
            : copperfin::runtime::format_value(found->second);
    };
    expect(formatted("cdigest") ==
               "4dcf03a7861d3d240432088e1ab20a9e6e4728de967fabef8d76d76c3513e474",
           "CFSHA256 should return invariant lowercase SHA-256 hex");
    expect(formatted("cencoded") == "Q29wcGVyZmlu",
           "CFBASE64ENCODE should emit canonical RFC 4648 text");
    expect(formatted("cdecoded") == "Copperfin",
           "CFBASE64DECODE should restore the exact payload");
    expect(formatted("cbinaryencoded") == "AP8Q",
           "PRG Base64 should preserve binary bytes");
    expect(formatted("cbinarydigest") ==
               "2da45f2cd1f9c8e69a67abf7a6b26c282533d0a7686787a9533265418680d4d2",
           "PRG SHA-256 should hash every embedded zero and high byte");
    expect(formatted("chmac") ==
               "b430e659ea210279fe3993dbed0634f76edb1e35ff5a8f13ecd6ddcf63ea55f8",
           "PRG HMAC-SHA256 should preserve embedded key and payload bytes");
    const auto hmac_match = state.globals.find("lhmacmatch");
    expect(hmac_match != state.globals.end() &&
               hmac_match->second.kind == copperfin::runtime::PrgValueKind::boolean &&
               hmac_match->second.boolean_value,
           "CFHMACVERIFY should accept an exact authenticated payload");
    const auto hmac_mismatch = state.globals.find("lhmacmismatch");
    expect(hmac_mismatch != state.globals.end() &&
               hmac_mismatch->second.kind == copperfin::runtime::PrgValueKind::boolean &&
               !hmac_mismatch->second.boolean_value,
           "CFHMACVERIFY should return false for a valid ordinary mismatch");
    const auto invalid_hmac = state.globals.find("ninvalidhmac");
    expect(invalid_hmac != state.globals.end() &&
               invalid_hmac->second.kind == copperfin::runtime::PrgValueKind::number &&
               invalid_hmac->second.number_value == 37.0,
           "malformed HMAC text should preserve the typed failure fallback");
    const auto nonstr_hmac = state.globals.find("nnonstrhmac");
    expect(nonstr_hmac != state.globals.end() &&
               nonstr_hmac->second.kind == copperfin::runtime::PrgValueKind::number &&
               nonstr_hmac->second.number_value == 73.0,
           "HMAC generation should reject non-character keys without stringification");
    expect(formatted("nbinarylength") == "3" && formatted("nfirst") == "0" &&
               formatted("nsecond") == "255" && formatted("nthird") == "16",
           "decoded PRG byte strings should retain zero and high bytes");
    expect(formatted("cinvalid") == "fallback",
           "noncanonical Base64 should return the caller fallback");

    const auto typed_invalid = state.globals.find("ntypedinvalid");
    expect(typed_invalid != state.globals.end() &&
               typed_invalid->second.kind == copperfin::runtime::PrgValueKind::number &&
               typed_invalid->second.number_value == 42.0,
           "Base64 decode failure should preserve fallback type and value");
    const auto typed_nonstr = state.globals.find("ntypednonstr");
    expect(typed_nonstr != state.globals.end() &&
               typed_nonstr->second.kind == copperfin::runtime::PrgValueKind::number &&
               typed_nonstr->second.number_value == 84.0,
           "payload helpers should reject non-character input without stringification");
    expect(formatted("cmissinghash").empty(),
           "missing hash input should fail closed to empty text");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_prg_payload_crypto_facade();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}

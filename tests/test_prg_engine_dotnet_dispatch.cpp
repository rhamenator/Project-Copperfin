// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "managed_pe_image.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <windows.h>
#include <oleauto.h>

namespace
{
    using copperfin::test_support::expect;
    using copperfin::test_support::make_runtime_session_options;
    using copperfin::test_support::ScopedEnvironmentValue;
    using copperfin::test_support::write_text;

    constexpr std::size_t repeated_exception_count = 64U;

    class ScopedCurrentPath final
    {
    public:
        explicit ScopedCurrentPath(const std::filesystem::path &path)
            : original_(std::filesystem::current_path())
        {
            std::filesystem::current_path(path);
        }

        ~ScopedCurrentPath()
        {
            std::error_code ignored;
            std::filesystem::current_path(original_, ignored);
        }

        ScopedCurrentPath(const ScopedCurrentPath &) = delete;
        ScopedCurrentPath &operator=(const ScopedCurrentPath &) = delete;

    private:
        std::filesystem::path original_;
    };

    std::string fixture_path()
    {
        return std::filesystem::path(COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH).generic_string();
    }

    std::filesystem::path dependency_path()
    {
        return COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH;
    }

    std::vector<char> read_binary(const std::filesystem::path &path)
    {
        std::ifstream input(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }

    void write_binary(const std::filesystem::path &path, const std::vector<char> &bytes)
    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }

    bool clear_clr_directory(std::vector<char> &bytes)
    {
        if (bytes.size() < sizeof(IMAGE_DOS_HEADER))
        {
            return false;
        }
        IMAGE_DOS_HEADER dos_header{};
        std::memcpy(&dos_header, bytes.data(), sizeof(dos_header));
        if (dos_header.e_magic != IMAGE_DOS_SIGNATURE || dos_header.e_lfanew < 0)
        {
            return false;
        }
        const std::size_t nt_offset = static_cast<std::size_t>(dos_header.e_lfanew);
        const std::size_t optional_offset =
            nt_offset + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
        if (optional_offset + sizeof(WORD) > bytes.size())
        {
            return false;
        }
        WORD magic = 0U;
        std::memcpy(&magic, bytes.data() + optional_offset, sizeof(magic));
        std::size_t directory_offset = 0U;
        if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            directory_offset = optional_offset + offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory) +
                               IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR * sizeof(IMAGE_DATA_DIRECTORY);
        }
        else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            directory_offset = optional_offset + offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory) +
                               IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR * sizeof(IMAGE_DATA_DIRECTORY);
        }
        else
        {
            return false;
        }
        if (directory_offset + sizeof(IMAGE_DATA_DIRECTORY) > bytes.size())
        {
            return false;
        }
        std::fill_n(bytes.begin() + static_cast<std::ptrdiff_t>(directory_offset),
                    sizeof(IMAGE_DATA_DIRECTORY),
                    '\0');
        return true;
    }

    void test_managed_pe_classification_contract()
    {
        using copperfin::runtime::PortableExecutableKind;
        using copperfin::runtime::inspect_portable_executable;
        namespace fs = std::filesystem;

        const fs::path pe32_path = COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH;
        const fs::path pe64_path = COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH;
        expect(inspect_portable_executable(pe32_path) == PortableExecutableKind::managed,
               "#3947: PE32 CLR fixture should classify as managed on either host architecture");
        expect(inspect_portable_executable(pe64_path) == PortableExecutableKind::managed,
               "#3947: PE32+ CLR fixture should classify as managed on either host architecture");
        expect(inspect_portable_executable(COPPERFIN_DECLARED_DLL_FIXTURE_PATH) ==
                   PortableExecutableKind::native,
               "#3947: repository native DLL fixture should classify as native");
#if defined(COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH)
        expect(inspect_portable_executable(COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH) ==
                   PortableExecutableKind::managed,
               "#3947: mixed-mode fixture should expose its CLR directory");
#endif

        const fs::path temp_root = fs::temp_directory_path() / "copperfin_managed_pe_inspection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        for (const auto &[source_path, output_name] :
             std::array<std::pair<fs::path, std::string>, 2U>{
                 std::pair<fs::path, std::string>{pe32_path, "synthetic-native-pe32.dll"},
                 std::pair<fs::path, std::string>{pe64_path, "synthetic-native-pe64.dll"},
             })
        {
            std::vector<char> bytes = read_binary(source_path);
            expect(clear_clr_directory(bytes),
                   "#3947: synthetic native fixture should expose a writable CLR directory entry");
            const fs::path native_path = temp_root / output_name;
            write_binary(native_path, bytes);
            expect(inspect_portable_executable(native_path) == PortableExecutableKind::native,
                   "#3947: PE32/PE32+ image with an empty CLR directory should classify as native");
        }

        for (const auto &[source_path, output_stem] :
             std::array<std::pair<fs::path, std::string>, 2U>{
                 std::pair<fs::path, std::string>{pe32_path, "pe32"},
                 std::pair<fs::path, std::string>{pe64_path, "pe64"},
             })
        {
            std::vector<char> undersized = read_binary(source_path);
            IMAGE_DOS_HEADER dos_header{};
            std::memcpy(&dos_header, undersized.data(), sizeof(dos_header));
            const std::size_t file_header_offset =
                static_cast<std::size_t>(dos_header.e_lfanew) + sizeof(DWORD);
            IMAGE_FILE_HEADER file_header{};
            std::memcpy(
                &file_header,
                undersized.data() + file_header_offset,
                sizeof(file_header));
            file_header.SizeOfOptionalHeader = sizeof(WORD);
            std::memcpy(
                undersized.data() + file_header_offset,
                &file_header,
                sizeof(file_header));
            const fs::path undersized_path = temp_root / (output_stem + "-undersized-optional.dll");
            write_binary(undersized_path, undersized);
            expect(inspect_portable_executable(undersized_path) == PortableExecutableKind::invalid,
                   "#3947: readable PE image with an undersized optional header should fail closed");

            std::vector<char> missing_directory = read_binary(source_path);
            const std::size_t optional_offset = file_header_offset + sizeof(IMAGE_FILE_HEADER);
            WORD optional_magic = 0U;
            std::memcpy(
                &optional_magic,
                missing_directory.data() + optional_offset,
                sizeof(optional_magic));
            const std::size_t directory_count_offset =
                optional_offset +
                (optional_magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC
                     ? offsetof(IMAGE_OPTIONAL_HEADER32, NumberOfRvaAndSizes)
                     : offsetof(IMAGE_OPTIONAL_HEADER64, NumberOfRvaAndSizes));
            const DWORD directory_count = IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR;
            std::memcpy(
                missing_directory.data() + directory_count_offset,
                &directory_count,
                sizeof(directory_count));
            const fs::path missing_directory_path =
                temp_root / (output_stem + "-missing-clr-directory.dll");
            write_binary(missing_directory_path, missing_directory);
            expect(inspect_portable_executable(missing_directory_path) ==
                       PortableExecutableKind::invalid,
                   "#3947: PE image without a declared CLR directory slot should fail closed");
        }

        std::vector<char> truncated = read_binary(pe32_path);
        truncated.resize((std::min)(truncated.size(), sizeof(IMAGE_DOS_HEADER) / 2U));
        const fs::path truncated_path = temp_root / "truncated.dll";
        write_binary(truncated_path, truncated);
        expect(inspect_portable_executable(truncated_path) == PortableExecutableKind::invalid,
               "#3947: truncated PE headers should fail closed");

        std::vector<char> invalid_offset = read_binary(pe64_path);
        const LONG invalid_lfanew = (std::numeric_limits<LONG>::max)();
        std::memcpy(
            invalid_offset.data() + offsetof(IMAGE_DOS_HEADER, e_lfanew),
            &invalid_lfanew,
            sizeof(invalid_lfanew));
        const fs::path invalid_offset_path = temp_root / "invalid-offset.dll";
        write_binary(invalid_offset_path, invalid_offset);
        expect(inspect_portable_executable(invalid_offset_path) == PortableExecutableKind::invalid,
               "#3947: out-of-file e_lfanew should fail closed");

        fs::remove_all(temp_root, ignored);
    }

    void test_mixed_mode_native_export_precedence()
    {
#if defined(COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH)
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_mixed_mode_declare";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path program_path = temp_root / "mixed_mode_native.prg";
        write_text(
            program_path,
            "DECLARE INTEGER CopperfinMixedModeNativeValue IN '" +
                fs::path(COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH).generic_string() + "'\n"
            "nResult = CopperfinMixedModeNativeValue()\n"
            "RETURN\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed, "#3947: mixed-mode native export should remain callable: " + state.message);
        const auto result = state.globals.find("nresult");
        expect(result != state.globals.end() &&
                   copperfin::runtime::format_value(result->second) == "3947",
               "#3947: mixed-mode native export should win before managed reflection fallback");
        fs::remove_all(temp_root, ignored);
#endif
    }

    void test_managed_declare_success_contract()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_success";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path program_path = temp_root / "managed_success.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ReturnFortyTwo IN '" +
                fixture_path() + "' AS ManagedSuccess\n"
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.Add IN '" +
                fixture_path() + "' AS ManagedAdd INTEGER left, INTEGER right\n"
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.WidenInt64 IN '" +
                fixture_path() + "' AS ManagedLong INTEGER value\n"
            "DECLARE INTEGER64 Copperfin.ManagedDeclareFixture.Methods.ReturnInt64BeyondDouble IN '" +
                fixture_path() + "' AS ManagedExactInt64\n"
            "DECLARE INTEGER64 Copperfin.ManagedDeclareFixture.Methods.PreserveInt64 IN '" +
                fixture_path() + "' AS ManagedPreserveInt64 INTEGER64 value\n"
            "DECLARE INTEGER64 Copperfin.ManagedDeclareFixture.Methods.ReturnUInt64BeyondDouble IN '" +
                fixture_path() + "' AS ManagedExactUInt64\n"
            "DECLARE DOUBLE Copperfin.ManagedDeclareFixture.Methods.WidenDouble IN '" +
                fixture_path() + "' AS ManagedDouble INTEGER value\n"
            "DECLARE SINGLE Copperfin.ManagedDeclareFixture.Methods.WidenSingle IN '" +
                fixture_path() + "' AS ManagedSingle SINGLE value\n"
            "DECLARE STRING Copperfin.ManagedDeclareFixture.Methods.Echo IN '" +
                fixture_path() + "' AS ManagedEcho STRING value\n"
            "nResult = ManagedSuccess()\n"
            "nSum = ManagedAdd(19, 23)\n"
            "nLong = ManagedLong(41)\n"
            "nExactInt64 = ManagedExactInt64()\n"
            "nEchoInt64 = ManagedPreserveInt64(nExactInt64)\n"
            "nExactUInt64 = ManagedExactUInt64()\n"
            "nExactPlusOne = nExactInt64 + 1\n"
            "nExactTwice = nExactInt64 * 2\n"
            "nUnsignedPlusOne = nExactUInt64 + 1\n"
            "nUnsignedMinusOne = nExactUInt64 - 1\n"
            "lUnsignedOrdered = nExactUInt64 > nUnsignedMinusOne\n"
            "lUnsignedEqual = (nExactUInt64 = nUnsignedMinusOne)\n"
            "nDouble = ManagedDouble(42)\n"
            "nSingle = ManagedSingle(42)\n"
            "cEcho = ManagedEcho('Copperfin')\n"
            "FOR nCall = 1 TO 128\n"
            "  nRepeated = ManagedSuccess()\n"
            "ENDFOR\n"
            "RETURN\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed, "#3943: successful managed DECLARE should complete: " + state.message);
        const auto result = state.globals.find("nresult");
        expect(result != state.globals.end(), "#3943: successful managed DECLARE should assign its result");
        if (result != state.globals.end())
        {
            expect(copperfin::runtime::format_value(result->second) == "42",
                   "#3943: successful managed dispatch should preserve its return value");
        }
        const auto sum = state.globals.find("nsum");
        expect(sum != state.globals.end(), "#3945: managed DECLARE should assign a typed argument result");
        if (sum != state.globals.end())
        {
            expect(copperfin::runtime::format_value(sum->second) == "42",
                   "#3945: managed dispatch should preserve INTEGER argument and return marshalling");
        }
        const auto widened_long = state.globals.find("nlong");
        expect(widened_long != state.globals.end() &&
                   copperfin::runtime::format_value(widened_long->second) == "42",
               "#3945: CLR binder should widen INTEGER arguments to System.Int64");
        const auto exact_int64 = state.globals.find("nexactint64");
        expect(exact_int64 != state.globals.end() &&
                   exact_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
                   copperfin::runtime::format_value(exact_int64->second) == "9007199254740993",
               "#3934: managed VT_I8 returns should remain exact beyond binary64 precision");
        const auto echo_int64 = state.globals.find("nechoint64");
        expect(echo_int64 != state.globals.end() &&
                   echo_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
                   copperfin::runtime::format_value(echo_int64->second) == "9007199254740993",
               "#3934: managed VT_I8 arguments should remain exact beyond binary64 precision");
        const auto exact_uint64 = state.globals.find("nexactuint64");
        expect(exact_uint64 != state.globals.end() &&
                   exact_uint64->second.kind == copperfin::runtime::PrgValueKind::uint64 &&
                   copperfin::runtime::format_value(exact_uint64->second) == "18014398509481985",
               "#3934: managed VT_UI8 returns should retain unsigned width and precision");
        // #6035: arithmetic and ordering on exact 64-bit results must not round trip through double
        // (18014398509481985 and 18014398509481984 are the same double).
        const auto expect_exact_int64 = [&](const char *name, const char *expected, const char *what) {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end() && found->second.kind == copperfin::runtime::PrgValueKind::int64 &&
                       copperfin::runtime::format_value(found->second) == expected,
                   std::string("#6035: ") + what + " should stay an exact int64 (" + expected + ")");
        };
        expect_exact_int64("nexactplusone", "9007199254740994", "exact + 1");
        expect_exact_int64("nexacttwice", "18014398509481986", "exact * 2");
        expect_exact_int64("nunsignedplusone", "18014398509481986", "unsigned + 1");
        expect_exact_int64("nunsignedminusone", "18014398509481984", "unsigned - 1");
        const auto unsigned_ordered = state.globals.find("lunsignedordered");
        expect(unsigned_ordered != state.globals.end() &&
                   unsigned_ordered->second.kind == copperfin::runtime::PrgValueKind::boolean &&
                   unsigned_ordered->second.boolean_value,
               "#6035: 18014398509481985 must order above 18014398509481984");
        const auto unsigned_equal = state.globals.find("lunsignedequal");
        expect(unsigned_equal != state.globals.end() &&
                   unsigned_equal->second.kind == copperfin::runtime::PrgValueKind::boolean &&
                   !unsigned_equal->second.boolean_value,
               "#6035: 18014398509481985 must not equal 18014398509481984");
        const auto widened_double = state.globals.find("ndouble");
        expect(widened_double != state.globals.end() &&
                   copperfin::runtime::format_value(widened_double->second) == "42.5",
               "#3945: CLR binder should widen INTEGER arguments to System.Double");
        const auto widened_single = state.globals.find("nsingle");
        expect(widened_single != state.globals.end() &&
                   copperfin::runtime::format_value(widened_single->second) == "42.25",
               "#3945: managed dispatch should preserve SINGLE argument and return marshalling");
        const auto echoed = state.globals.find("cecho");
        expect(echoed != state.globals.end() &&
                   copperfin::runtime::format_value(echoed->second) == "Copperfin",
               "#3945: managed dispatch should preserve STRING argument and return marshalling");
        const auto repeated = state.globals.find("nrepeated");
        expect(repeated != state.globals.end() &&
                   copperfin::runtime::format_value(repeated->second) == "42",
               "#3945: repeated managed success should remain stack-frugal and stable");
        expect(
            std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
            {
                return event.category == "runtime.declare_dll" &&
                       event.detail.find("ManagedSuccess") != std::string::npos;
            }),
            "#3943: managed declarations should preserve the invariant runtime.declare_dll event");

        fs::remove_all(temp_root, ignored);
    }

    void test_managed_declare_integer_argument_boundaries()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_integer_boundaries";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const std::string fixture = fixture_path();
        const fs::path program_path = temp_root / "managed_integer_boundaries.prg";
        write_text(
            program_path,
            "PUBLIC nCase, nHugeError, nPositiveInfinityError, nNegativeInfinityError, nInt64OverflowError, nNaNError, nUInt64Error\n"
            "nCase = 0\n"
            "nHugeError = 0\n"
            "nPositiveInfinityError = 0\n"
            "nNegativeInfinityError = 0\n"
            "nInt64OverflowError = 0\n"
            "nNaNError = 0\n"
            "nUInt64Error = 0\n"
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ResetIntegerBoundaryEntryCount IN '" + fixture + "' AS BoundaryReset\n"
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.IntegerBoundaryEntryCount IN '" + fixture + "' AS BoundaryCount\n"
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.EchoInt32Boundary IN '" + fixture + "' AS Echo32 INTEGER value\n"
            "DECLARE INTEGER64 Copperfin.ManagedDeclareFixture.Methods.EchoInt64Boundary IN '" + fixture + "' AS Echo64 INTEGER64 value\n"
            "DECLARE INTEGER64 Copperfin.ManagedDeclareFixture.Methods.ReturnInt64Maximum IN '" + fixture + "' AS Int64Maximum\n"
            "DECLARE INTEGER64 Copperfin.ManagedDeclareFixture.Methods.ReturnUInt64Maximum IN '" + fixture + "' AS UInt64Maximum\n"
            "DECLARE INTEGER64 Copperfin.ManagedDeclareFixture.Methods.ReturnUInt64AtSignedMaximum IN '" + fixture + "' AS UInt64AtSignedMaximum\n"
            "DECLARE DOUBLE Copperfin.ManagedDeclareFixture.Methods.ReturnNaN IN '" + fixture + "' AS ReturnNaN\n"
            "nReset = BoundaryReset()\n"
            "nInt32Max = Echo32(2147483647)\n"
            "nInt32Min = Echo32(2147483648)\n"
            "nUint32Max = Echo32(4294967295)\n"
            "nTwoTo32 = Echo32(4294967296)\n"
            "nBelowInt32Min = Echo32(-2147483649)\n"
            "nInt64MaxSource = Int64Maximum()\n"
            "nInt64Max = Echo64(nInt64MaxSource)\n"
            "nInt64Min = Echo64(-9223372036854775808)\n"
            "nUInt64Max = UInt64Maximum()\n"
            "nUInt64ToInt32 = Echo32(nUInt64Max)\n"
            "nUInt64SignedMaxSource = UInt64AtSignedMaximum()\n"
            "nUInt64SignedMax = Echo64(nUInt64SignedMaxSource)\n"
            "nValidEntries = BoundaryCount()\n"
            "ON ERROR DO HandleBoundaryError\n"
            "nCase = 1\n"
            "nUnexpectedHuge = Echo32(1E300)\n"
            "nCase = 2\n"
            "nUnexpectedPositiveInfinity = Echo32(EXP(1000))\n"
            "nCase = 3\n"
            "nUnexpectedNegativeInfinity = Echo32(-EXP(1000))\n"
            "nCase = 4\n"
            "nUnexpectedInt64Overflow = Echo64(9223372036854775808)\n"
            "nCase = 5\n"
            "nNaN = ReturnNaN()\n"
            "nUnexpectedNaN = Echo32(nNaN)\n"
            "nCase = 6\n"
            "nUnexpectedUInt64 = Echo64(nUInt64Max)\n"
            "ON ERROR\n"
            "SET NUMERICBEHAVIOR TO VFP9\n"
            "nVfp9Huge = Echo32(1E300)\n"
            "SET NUMERICBEHAVIOR TO COPPERFIN\n"
            "nFinalEntries = BoundaryCount()\n"
            "RETURN\n"
            "PROCEDURE HandleBoundaryError\n"
            "DO CASE\n"
            "CASE nCase = 1\n"
            "  nHugeError = ERROR()\n"
            "CASE nCase = 2\n"
            "  nPositiveInfinityError = ERROR()\n"
            "CASE nCase = 3\n"
            "  nNegativeInfinityError = ERROR()\n"
            "CASE nCase = 4\n"
            "  nInt64OverflowError = ERROR()\n"
            "CASE nCase = 5\n"
            "  nNaNError = ERROR()\n"
            "CASE nCase = 6\n"
            "  nUInt64Error = ERROR()\n"
            "ENDCASE\n"
            "RETURN\n"
            "ENDPROC\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#6050: managed DECLARE boundary script should complete: " + state.message);

        const auto expect_value = [&](const char* name, const char* expected, const char* label) {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end() && copperfin::runtime::format_value(found->second) == expected,
                   std::string("#6050: ") + label);
        };
        expect_value("nint32max", "2147483647", "managed INTEGER keeps INT32_MAX");
        expect_value("nint32min", "-2147483648", "managed INTEGER maps 2^31 to INT32_MIN");
        expect_value("nuint32max", "-1", "managed INTEGER preserves UINT32_MAX low bits");
        expect_value("ntwoto32", "0", "managed INTEGER discards bits above bit 31");
        expect_value("nbelowint32min", "2147483647", "managed INTEGER wraps below INT32_MIN");
        expect_value("nint64max", "9223372036854775807", "managed INTEGER64 preserves exact INT64_MAX");
        expect_value("nint64min", "-9223372036854775808", "managed INTEGER64 preserves INT64_MIN");
        expect_value("nuint64toint32", "-1", "managed INTEGER accepts exact UINT64_MAX low bits");
        expect_value("nuint64signedmax", "9223372036854775807", "managed INTEGER64 accepts unsigned INT64_MAX");
        expect_value("nvalidentries", "9", "all valid managed boundary calls enter the fixture");
        expect_value("nhugeerror", "11", "managed INTEGER rejects a huge finite value");
        expect_value("npositiveinfinityerror", "11", "managed INTEGER rejects positive infinity");
        expect_value("nnegativeinfinityerror", "11", "managed INTEGER rejects negative infinity");
        expect_value("nint64overflowerror", "11", "managed INTEGER64 rejects numeric 2^63");
        expect_value("nnanerror", "11", "managed INTEGER rejects an interoperable NaN");
        expect_value("nuint64error", "11", "managed INTEGER64 rejects exact UINT64_MAX");
        expect_value("nvfp9huge", "0", "managed INTEGER maps a huge finite VFP9 value to zero");
        expect_value("nfinalentries", "10", "only the VFP9 huge finite value reaches reflection invocation");

        fs::remove_all(temp_root, ignored);
    }

    void test_managed_declare_requires_qualified_type()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_unqualified";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path program_path = temp_root / "managed_unqualified.prg";
        write_text(
            program_path,
            "DECLARE INTEGER ReturnFortyTwo IN '" + fixture_path() + "' AS ManagedUnqualified\n"
            "nIgnored = ManagedUnqualified()\n"
            "cFailureMessage = MESSAGE()\n"
            "RETURN\n");

        const ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed,
               "#3945: unqualified managed member should retain the recoverable expression contract");
        const auto failure_message = state.globals.find("cfailuremessage");
        expect(failure_message != state.globals.end(),
               "#3945: unqualified managed member should retain MESSAGE() diagnostic state");
        if (failure_message != state.globals.end())
        {
            const auto catalog = copperfin::localization::load_catalogs(
                copperfin::localization::resolve_catalog_root(),
                "qps-ploc");
            const std::string expected = catalog.translate(
                "Runtime.Prg.Dll.Error.DotNetTypeNotFound",
                {{"typeName", ""}});
            expect(copperfin::runtime::format_value(failure_message->second) == expected,
                   "#3945: unqualified managed member should preserve type-lookup diagnostic identity");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_managed_declare_explicit_relative_path()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_relative";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root / "managed");
        const fs::path relative_fixture = temp_root / "managed" / "Copperfin.ManagedDeclareRelative.dll";
        fs::copy_file(
            COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH,
            relative_fixture,
            fs::copy_options::overwrite_existing,
            ignored);
        expect(!ignored && fs::exists(relative_fixture),
               "#3945: managed fixture should copy beneath the PRG working directory");
        const fs::path relative_dependency = temp_root / "managed" / dependency_path().filename();
        ignored.clear();
        fs::copy_file(
            dependency_path(),
            relative_dependency,
            fs::copy_options::overwrite_existing,
            ignored);
        expect(!ignored && fs::exists(relative_dependency),
               "#3945: managed dependency should copy beside the path-loaded fixture");

        const fs::path program_path = temp_root / "managed_relative.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ReturnDependencyValue IN "
            "'managed/Copperfin.ManagedDeclareRelative.dll' AS ManagedRelative\n"
            "nResult = ManagedRelative()\n"
            "RETURN\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed, "#3945: explicit-relative managed DECLARE should complete");
        const auto result = state.globals.find("nresult");
        expect(result != state.globals.end() &&
                   copperfin::runtime::format_value(result->second) == "3945",
               "#3945: explicit-relative Assembly.LoadFrom should resolve sibling dependencies");
        expect(
            std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
            {
                return event.category == "runtime.declare_dll" &&
                       event.detail.find("managed/Copperfin.ManagedDeclareRelative.dll") != std::string::npos;
            }),
            "#3945: explicit-relative managed declaration should preserve its source-facing event path");

        fs::remove_all(temp_root, ignored);
    }

    void test_repeated_managed_exception_cleanup()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_throw";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path program_path = temp_root / "managed_throw.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ThrowAlways IN '" +
                fixture_path() + "' AS ManagedThrow\n"
            "FOR nCall = 1 TO " + std::to_string(repeated_exception_count) + "\n"
            "  nIgnored = ManagedThrow()\n"
            "ENDFOR\n"
            "cFailureMessage = MESSAGE()\n"
            "RETURN\n");

        const ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed,
               "#3943: repeatedly-throwing managed DECLARE fixture should retain the existing recoverable contract");
        const auto failure_message = state.globals.find("cfailuremessage");
        expect(failure_message != state.globals.end(),
               "#3943: managed failure should retain the localized MESSAGE() contract");
        if (failure_message != state.globals.end())
        {
            const std::string text = copperfin::runtime::format_value(failure_message->second);
            const auto catalog = copperfin::localization::load_catalogs(
                copperfin::localization::resolve_catalog_root(),
                "qps-ploc");
            const std::string expected = catalog.translate(
                "Runtime.Prg.Dll.Error.DotNetMethodInvokeFailed",
                {{"hresult", std::to_string(static_cast<long>(DISP_E_EXCEPTION))}});
            expect(text == expected,
                   "#3943: managed failure should preserve the localized diagnostic and HRESULT value");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_managed_declare_parentless_search_path()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_parentless";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        fs::create_directories(temp_root / "moved-cwd");
        const std::string parentless_name = "Copperfin.ManagedDeclareFixture.parentless.dll";
        fs::copy_file(
            COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH,
            temp_root / parentless_name,
            fs::copy_options::overwrite_existing,
            ignored);
        expect(!ignored, "#3947: parentless managed fixture should copy into the process search directory");

        const fs::path program_path = temp_root / "managed_parentless.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ReturnFortyTwo IN '" +
                parentless_name + "' AS ManagedParentless\n"
            "nResult = ManagedParentless()\n"
            "RETURN\n");

        {
            const ScopedCurrentPath current_path(temp_root);
            auto session = copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
            session.add_breakpoint({.file_path = program_path.string(), .line = 2U});
            const auto declared_state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(declared_state.paused && declared_state.location.line == 2U,
                   "#3947: parentless declaration should complete before invocation breakpoint");

            fs::current_path(temp_root / "moved-cwd");
            const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

            expect(state.completed,
                   "#3947: parentless managed DECLARE should retain its loader-resolved path after CWD changes: " +
                       state.message);
            const auto result = state.globals.find("nresult");
            expect(result != state.globals.end() &&
                       copperfin::runtime::format_value(result->second) == "42",
                   "#3947: parentless managed DECLARE should retain invocation behavior");
            expect(
                std::any_of(state.events.begin(), state.events.end(), [&](const auto &event)
                {
                    return event.category == "runtime.declare_dll" &&
                           event.detail.find(parentless_name) != std::string::npos;
                }),
                "#3947: parentless managed declaration should preserve its source designator event");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_managed_load_failure_localization()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_missing";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root / "managed");

        const fs::path missing_fixture = temp_root / "managed" / "removed-managed.dll";
        fs::copy_file(
            COPPERFIN_MANAGED_DECLARE_MISSING_PATH,
            missing_fixture,
            fs::copy_options::overwrite_existing,
            ignored);
        expect(!ignored && fs::exists(missing_fixture),
               "#3945: distinct managed fixture should exist for declaration classification");

        const fs::path program_path = temp_root / "managed_missing.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ReturnFortyTwo IN "
            "'managed/removed-managed.dll' AS ManagedMissing\n"
            "nIgnored = ManagedMissing()\n"
            "nErrorRows = AERROR(aFailure)\n"
            "nFaultLine = aFailure[1,5]\n"
            "cFaultStatement = aFailure[1,7]\n"
            "cFailureMessage = MESSAGE()\n"
            "RETURN\n");

        const ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        session.add_breakpoint({.file_path = program_path.string(), .line = 2U});
        const auto declared_state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(declared_state.paused && declared_state.location.line == 2U,
               "#3945: valid managed image should classify before the load-failure probe");

        ignored.clear();
        fs::remove(missing_fixture, ignored);
        expect(!ignored && !fs::exists(missing_fixture),
               "#3945: managed image should be removable before Assembly.LoadFrom");
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed,
               "#3945: missing managed load should retain the recoverable expression contract");
        const auto ignored_result = state.globals.find("nignored");
        expect(ignored_result != state.globals.end() &&
                   copperfin::runtime::format_value(ignored_result->second).empty(),
               "#3945: failed Assembly.LoadFrom should retain the empty expression result");
        const auto failure_message = state.globals.find("cfailuremessage");
        expect(failure_message != state.globals.end(),
               "#3945: missing Assembly.LoadFrom should retain MESSAGE() diagnostic state");
        if (failure_message != state.globals.end())
        {
            const auto catalog = copperfin::localization::load_catalogs(
                copperfin::localization::resolve_catalog_root(),
                "qps-ploc");
            const std::string expected = catalog.translate(
                "Runtime.Prg.Dll.Error.DotNetAssemblyLoadFailed",
                {
                    {"hresult", std::to_string(static_cast<long>(DISP_E_EXCEPTION))},
                    {"path", missing_fixture.string()},
                });
            const std::string actual =
                copperfin::runtime::format_value(failure_message->second);
            expect(actual == expected,
                   "#3945: missing load should preserve localized path and HRESULT placeholders; "
                   "expected=[" + expected + "] actual=[" + actual + "]");
        }
        const auto error_rows = state.globals.find("nerrorrows");
        const auto fault_line = state.globals.find("nfaultline");
        const auto fault_statement = state.globals.find("cfaultstatement");
        expect(error_rows != state.globals.end() &&
                   copperfin::runtime::format_value(error_rows->second) == "1",
               "#3945: managed load failure should populate one AERROR row");
        expect(fault_line != state.globals.end() &&
                   copperfin::runtime::format_value(fault_line->second) == "2",
               "#3945: managed load failure should preserve the call-site source line");
        expect(fault_statement != state.globals.end() &&
                   copperfin::runtime::format_value(fault_statement->second) ==
                       "nIgnored = ManagedMissing()",
               "#3945: managed load failure should preserve the call-site statement");

        fs::remove_all(temp_root, ignored);
    }
}

int main()
{
    test_managed_pe_classification_contract();
    test_mixed_mode_native_export_precedence();
    test_managed_declare_requires_qualified_type();
    test_managed_declare_explicit_relative_path();
    test_managed_declare_success_contract();
    test_managed_declare_integer_argument_boundaries();
    test_managed_declare_parentless_search_path();
    test_repeated_managed_exception_cleanup();
    test_managed_load_failure_localization();

    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

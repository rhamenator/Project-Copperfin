// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/localization/localization.h"
#include "../src/runtime/prg_compatibility_error.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "../src/runtime/prg_engine_path_functions.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
#include <system_error>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace
{

    using namespace copperfin::test_support;

    void test_portable_path_expression_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_functions_paths";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "path_functions.prg";
        write_text(
            main_path,
            "cWinPath = 'E:\\Project-Copperfin\\src\\runtime\\prg_engine.cpp'\n"
            "cUncPath = '\\\\server\\share\\reports\\invoice.frx'\n"
            "cPosixPath = '/home/rich/dev/Project-Copperfin/src/runtime/prg_engine.cpp'\n"
            "cDrive = JUSTDRIVE(cWinPath)\n"
            "cUncDrive = JUSTDRIVE(cUncPath)\n"
            "cUncRootBackslashDrive = JUSTDRIVE('\\\\server\\share')\n"
            "cUncRootBackslashTrailingDrive = JUSTDRIVE('\\\\server\\share\\')\n"
            "cUncRootSlashDrive = JUSTDRIVE('//server/share')\n"
            "cUncRootSlashTrailingDrive = JUSTDRIVE('//server/share/')\n"
            R"(cMalformedUncBackslashEmptyServer = JUSTDRIVE('\\\share'))" "\n"
            "cMalformedUncSlashEmptyServer = JUSTDRIVE('///share')\n"
            "cExtendedUncDrive = JUSTDRIVE('\\\\?\\UNC\\server\\share\\reports\\invoice.frx')\n"
            "cExtendedUncRootDrive = JUSTDRIVE('\\\\?\\UNC\\server\\share')\n"
            "cLowerExtendedUncDrive = JUSTDRIVE('\\\\?\\unc\\server\\share\\reports\\invoice.frx')\n"
            "cExtendedDrive = JUSTDRIVE('\\\\?\\E:\\reports\\invoice.frx')\n"
            "cDeviceDrive = JUSTDRIVE('\\\\.\\E:\\reports\\invoice.frx')\n"
            "cMalformedExtendedUncMissingShare = JUSTDRIVE('\\\\?\\UNC\\server')\n"
            "cMalformedExtendedUncEmptyServer = JUSTDRIVE('\\\\?\\UNC\\\\share')\n"
            "cGlobalRootDrive = JUSTDRIVE('\\\\?\\GLOBALROOT\\Device\\HarddiskVolume1')\n"
            "cDevicePipeDrive = JUSTDRIVE('\\\\.\\pipe\\copperfin')\n"
            "cPosixDrive = JUSTDRIVE(cPosixPath)\n"
            "cRelativeDrive = JUSTDRIVE('forms\\main.prg')\n"
            "cEmptyDrive = JUSTDRIVE('')\n"
            "cWinDir = JUSTPATH(cWinPath)\n"
            "cWinName = JUSTFNAME(cWinPath)\n"
            "cWinStem = JUSTSTEM(cWinPath)\n"
            "cWinExt = JUSTEXT(cWinPath)\n"
            "cPosixDir = JUSTPATH(cPosixPath)\n"
            "cPosixName = JUSTFNAME(cPosixPath)\n"
            "cPosixStem = JUSTSTEM(cPosixPath)\n"
            "cPosixExt = JUSTEXT(cPosixPath)\n"
            "cWinFullPath = FULLPATH(cWinPath)\n"
            "cUncFullPath = FULLPATH(cUncPath)\n"
            "cWinDotFullPath = FULLPATH('E:\\Project-Copperfin\\src\\.\\runtime\\..\\main.prg')\n"
            "cWinMixedDotFullPath = FULLPATH('E:/Project-Copperfin\\src/../main.prg')\n"
            "cWinRootBoundaryFullPath = FULLPATH('E:\\..\\..\\main.prg')\n"
            "cUncDotFullPath = FULLPATH('\\\\server\\share\\reports\\.\\drafts\\..\\invoice.frx')\n"
            "cUncMixedDotFullPath = FULLPATH('//server/share\\reports/../invoice.frx')\n"
            "cUncRootBoundaryFullPath = FULLPATH('\\\\server\\share\\..\\..\\invoice.frx')\n"
            "cExtendedUncDotFullPath = FULLPATH('\\\\?\\UNC\\server\\share\\reports\\..\\invoice.frx')\n"
            "cExtendedDriveDotFullPath = FULLPATH('\\\\?\\E:\\reports\\..\\invoice.frx')\n"
            "cDevicePipeFullPath = FULLPATH('\\\\.\\pipe\\alpha\\..\\beta')\n"
            "cGlobalRootFullPath = FULLPATH('\\\\?\\GLOBALROOT\\Device\\HarddiskVolumeShadowCopy1\\..\\file')\n"
            "cPosixFullPath = FULLPATH(cPosixPath)\n"
            "cPosixDotFullPath = FULLPATH('/opt/copperfin/../shared/main.prg')\n"
            "cPosixMixedFullPath = FULLPATH('/opt\\copperfin\\main.prg')\n"
            "cRelativeWinFullPath = FULLPATH('forms\\main.prg')\n"
            "cRelativePosixFullPath = FULLPATH('forms/report.prg')\n"
            "cForcedExt = FORCEEXT(cWinPath, 'h')\n"
            "cForcedExtWithDot = FORCEEXT(cWinPath, '.hpp')\n"
            "cDefaultExtAdded = DEFAULTEXT('D:\\generated\\report', 'frx')\n"
            "cDefaultExtKept = DEFAULTEXT('D:\\generated\\report.frx', 'bak')\n"
            "cForcedPath = FORCEPATH(cWinPath, 'D:\\generated')\n"
            "cForcedPosixPath = FORCEPATH(cPosixPath, '/tmp/generated')\n"
            "cForcedUncPath = FORCEPATH('foo.txt', '\\\\server\\share\\generated')\n"
            "cForcedMixedDrivePath = FORCEPATH('foo.txt', 'D:/generated\\subdir')\n"
            "cForcedMixedUncPath = FORCEPATH('foo.txt', '//server/share\\generated')\n"
            "cForcedMixedPosixPath = FORCEPATH('foo.txt', '/tmp\\generated/subdir')\n"
            "cForcedMixedRelativeBackslashPath = FORCEPATH('foo.txt', 'generated\\subdir/leaf')\n"
            "cForcedMixedRelativeSlashPath = FORCEPATH('foo.txt', 'generated/subdir\\leaf')\n"
            "cForcedRelativeBackslashPath = FORCEPATH('foo.txt', 'generated\\subdir')\n"
            "cForcedRelativeSlashPath = FORCEPATH('foo.txt', 'generated/subdir')\n"
            "cForcedRelativeNoSeparatorPath = FORCEPATH('foo.txt', 'generated')\n"
            "cForcedTrailingDrivePath = FORCEPATH('foo.txt', 'D:/generated/')\n"
            "cForcedTrailingPosixPath = FORCEPATH('foo.txt', '/tmp\\generated\\')\n"
            "cForcedDriveRelativePath = FORCEPATH('foo.txt', 'D:generated/subdir')\n"
            "cForcedDriveRootPath = FORCEPATH('foo.txt', 'D:')\n"
            "cForcedPosixRootPath = FORCEPATH('foo.txt', '/')\n"
            "cForcedWindowsRootPath = FORCEPATH('foo.txt', '\\')\n"
            "cForcedNamespaceDrivePath = FORCEPATH('foo.txt', '\\\\?\\E:/generated')\n"
            "cForcedNamespaceUncPath = FORCEPATH('foo.txt', '\\\\?\\UNC\\server/share\\generated')\n"
            "cForcedRepeatedSeparatorPath = FORCEPATH('foo.txt', 'D:\\generated\\\\subdir')\n"
            "cForcedEmptyDirectoryPath = FORCEPATH('folder\\foo.txt', '')\n"
            "cForcedEmptySourcePath = FORCEPATH('', 'D:/generated')\n"
            "cForcedBothEmptyPath = FORCEPATH('', '')\n"
            "cCurrentDir = CURDIR()\n"
            // #5910: run ADDBS() through the real PRG expression
            // parser/dispatch, not just a direct evaluate_path_function()
            // call, so a wiring or packaging regression cannot pass
            // coverage that only exercises the underlying C++ helper.
            "cAddbsEmpty = ADDBS('')\n"
            "cAddbsPlain = ADDBS('abc')\n"
            "cAddbsTrailingSlash = ADDBS('abc/')\n"
            "cAddbsTrailingBackslash = ADDBS('abc\\')\n"
            "cAddbsDriveLetter = ADDBS('C:')\n"
            "cAddbsDriveRoot = ADDBS('C:\\')\n"
            "cAddbsUnc = ADDBS('\\\\server\\share')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "portable path function script should complete");

        const auto drive = state.globals.find("cdrive");
        const auto unc_drive = state.globals.find("cuncdrive");
        const auto unc_root_backslash_drive = state.globals.find("cuncrootbackslashdrive");
        const auto unc_root_backslash_trailing_drive = state.globals.find("cuncrootbackslashtrailingdrive");
        const auto unc_root_slash_drive = state.globals.find("cuncrootslashdrive");
        const auto unc_root_slash_trailing_drive = state.globals.find("cuncrootslashtrailingdrive");
        const auto malformed_unc_backslash_empty_server = state.globals.find("cmalformeduncbackslashemptyserver");
        const auto malformed_unc_slash_empty_server = state.globals.find("cmalformeduncslashemptyserver");
        const auto extended_unc_drive = state.globals.find("cextendeduncdrive");
        const auto extended_unc_root_drive = state.globals.find("cextendeduncrootdrive");
        const auto lower_extended_unc_drive = state.globals.find("clowerextendeduncdrive");
        const auto extended_drive = state.globals.find("cextendeddrive");
        const auto device_drive = state.globals.find("cdevicedrive");
        const auto malformed_extended_unc_missing_share = state.globals.find("cmalformedextendeduncmissingshare");
        const auto malformed_extended_unc_empty_server = state.globals.find("cmalformedextendeduncemptyserver");
        const auto global_root_drive = state.globals.find("cglobalrootdrive");
        const auto device_pipe_drive = state.globals.find("cdevicepipedrive");
        const auto posix_drive = state.globals.find("cposixdrive");
        const auto relative_drive = state.globals.find("crelativedrive");
        const auto empty_drive = state.globals.find("cemptydrive");
        const auto win_dir = state.globals.find("cwindir");
        const auto win_name = state.globals.find("cwinname");
        const auto win_stem = state.globals.find("cwinstem");
        const auto win_ext = state.globals.find("cwinext");
        const auto posix_dir = state.globals.find("cposixdir");
        const auto posix_name = state.globals.find("cposixname");
        const auto posix_stem = state.globals.find("cposixstem");
        const auto posix_ext = state.globals.find("cposixext");
        const auto win_full_path = state.globals.find("cwinfullpath");
        const auto unc_full_path = state.globals.find("cuncfullpath");
        const auto win_dot_full_path = state.globals.find("cwindotfullpath");
        const auto win_mixed_dot_full_path = state.globals.find("cwinmixeddotfullpath");
        const auto win_root_boundary_full_path = state.globals.find("cwinrootboundaryfullpath");
        const auto unc_dot_full_path = state.globals.find("cuncdotfullpath");
        const auto unc_mixed_dot_full_path = state.globals.find("cuncmixeddotfullpath");
        const auto unc_root_boundary_full_path = state.globals.find("cuncrootboundaryfullpath");
        const auto extended_unc_dot_full_path = state.globals.find("cextendeduncdotfullpath");
        const auto extended_drive_dot_full_path = state.globals.find("cextendeddrivedotfullpath");
        const auto device_pipe_full_path = state.globals.find("cdevicepipefullpath");
        const auto global_root_full_path = state.globals.find("cglobalrootfullpath");
        const auto posix_full_path = state.globals.find("cposixfullpath");
        const auto posix_dot_full_path = state.globals.find("cposixdotfullpath");
        const auto posix_mixed_full_path = state.globals.find("cposixmixedfullpath");
        const auto relative_win_full_path = state.globals.find("crelativewinfullpath");
        const auto relative_posix_full_path = state.globals.find("crelativeposixfullpath");
        const auto forced_ext = state.globals.find("cforcedext");
        const auto forced_ext_with_dot = state.globals.find("cforcedextwithdot");
        const auto default_ext_added = state.globals.find("cdefaultextadded");
        const auto default_ext_kept = state.globals.find("cdefaultextkept");
        const auto forced_path = state.globals.find("cforcedpath");
        const auto forced_posix_path = state.globals.find("cforcedposixpath");
        const auto forced_unc_path = state.globals.find("cforceduncpath");
        const auto forced_mixed_drive_path = state.globals.find("cforcedmixeddrivepath");
        const auto forced_mixed_unc_path = state.globals.find("cforcedmixeduncpath");
        const auto forced_mixed_posix_path = state.globals.find("cforcedmixedposixpath");
        const auto forced_mixed_relative_backslash_path = state.globals.find("cforcedmixedrelativebackslashpath");
        const auto forced_mixed_relative_slash_path = state.globals.find("cforcedmixedrelativeslashpath");
        const auto forced_relative_backslash_path = state.globals.find("cforcedrelativebackslashpath");
        const auto forced_relative_slash_path = state.globals.find("cforcedrelativeslashpath");
        const auto forced_relative_no_separator_path = state.globals.find("cforcedrelativenoseparatorpath");
        const auto forced_trailing_drive_path = state.globals.find("cforcedtrailingdrivepath");
        const auto forced_trailing_posix_path = state.globals.find("cforcedtrailingposixpath");
        const auto forced_drive_relative_path = state.globals.find("cforceddriverelativepath");
        const auto forced_drive_root_path = state.globals.find("cforceddriverootpath");
        const auto forced_posix_root_path = state.globals.find("cforcedposixrootpath");
        const auto forced_windows_root_path = state.globals.find("cforcedwindowsrootpath");
        const auto forced_namespace_drive_path = state.globals.find("cforcednamespacedrivepath");
        const auto forced_namespace_unc_path = state.globals.find("cforcednamespaceuncpath");
        const auto forced_repeated_separator_path = state.globals.find("cforcedrepeatedseparatorpath");
        const auto forced_empty_directory_path = state.globals.find("cforcedemptydirectorypath");
        const auto forced_empty_source_path = state.globals.find("cforcedemptysourcepath");
        const auto forced_both_empty_path = state.globals.find("cforcedbothemptypath");
        const auto current_dir = state.globals.find("ccurrentdir");

        expect(drive != state.globals.end(), "JUSTDRIVE result should be captured");
        expect(unc_drive != state.globals.end(), "UNC JUSTDRIVE result should be captured");
        expect(unc_root_backslash_drive != state.globals.end(), "backslash UNC root JUSTDRIVE result should be captured");
        expect(unc_root_backslash_trailing_drive != state.globals.end(), "trailing backslash UNC root JUSTDRIVE result should be captured");
        expect(unc_root_slash_drive != state.globals.end(), "slash UNC root JUSTDRIVE result should be captured");
        expect(unc_root_slash_trailing_drive != state.globals.end(), "trailing slash UNC root JUSTDRIVE result should be captured");
        expect(malformed_unc_backslash_empty_server != state.globals.end(), "empty-server backslash UNC JUSTDRIVE result should be captured");
        expect(malformed_unc_slash_empty_server != state.globals.end(), "empty-server slash UNC JUSTDRIVE result should be captured");
        expect(extended_unc_drive != state.globals.end(), "extended UNC JUSTDRIVE result should be captured");
        expect(extended_unc_root_drive != state.globals.end(), "extended UNC root JUSTDRIVE result should be captured");
        expect(lower_extended_unc_drive != state.globals.end(), "lowercase extended UNC JUSTDRIVE result should be captured");
        expect(extended_drive != state.globals.end(), "extended drive JUSTDRIVE result should be captured");
        expect(device_drive != state.globals.end(), "device drive JUSTDRIVE result should be captured");
        expect(malformed_extended_unc_missing_share != state.globals.end(), "missing-share extended UNC JUSTDRIVE result should be captured");
        expect(malformed_extended_unc_empty_server != state.globals.end(), "empty-server extended UNC JUSTDRIVE result should be captured");
        expect(global_root_drive != state.globals.end(), "GLOBALROOT JUSTDRIVE result should be captured");
        expect(device_pipe_drive != state.globals.end(), "device pipe JUSTDRIVE result should be captured");
        expect(posix_drive != state.globals.end(), "POSIX JUSTDRIVE result should be captured");
        expect(relative_drive != state.globals.end(), "relative JUSTDRIVE result should be captured");
        expect(empty_drive != state.globals.end(), "empty JUSTDRIVE result should be captured");
        expect(win_dir != state.globals.end(), "Windows JUSTPATH result should be captured");
        expect(win_name != state.globals.end(), "Windows JUSTFNAME result should be captured");
        expect(win_stem != state.globals.end(), "Windows JUSTSTEM result should be captured");
        expect(win_ext != state.globals.end(), "Windows JUSTEXT result should be captured");
        expect(posix_dir != state.globals.end(), "POSIX JUSTPATH result should be captured");
        expect(posix_name != state.globals.end(), "POSIX JUSTFNAME result should be captured");
        expect(posix_stem != state.globals.end(), "POSIX JUSTSTEM result should be captured");
        expect(posix_ext != state.globals.end(), "POSIX JUSTEXT result should be captured");
        expect(win_full_path != state.globals.end(), "Windows FULLPATH result should be captured");
        expect(unc_full_path != state.globals.end(), "UNC FULLPATH result should be captured");
        expect(win_dot_full_path != state.globals.end(), "Windows dot-segment FULLPATH result should be captured");
        expect(win_mixed_dot_full_path != state.globals.end(), "Windows mixed-separator FULLPATH result should be captured");
        expect(win_root_boundary_full_path != state.globals.end(), "Windows root-boundary FULLPATH result should be captured");
        expect(unc_dot_full_path != state.globals.end(), "UNC dot-segment FULLPATH result should be captured");
        expect(unc_mixed_dot_full_path != state.globals.end(), "UNC mixed-separator FULLPATH result should be captured");
        expect(unc_root_boundary_full_path != state.globals.end(), "UNC root-boundary FULLPATH result should be captured");
        expect(extended_unc_dot_full_path != state.globals.end(), "extended UNC FULLPATH result should be captured");
        expect(extended_drive_dot_full_path != state.globals.end(), "extended drive FULLPATH result should be captured");
        expect(device_pipe_full_path != state.globals.end(), "device pipe FULLPATH result should be captured");
        expect(global_root_full_path != state.globals.end(), "GLOBALROOT FULLPATH result should be captured");
        expect(posix_full_path != state.globals.end(), "POSIX FULLPATH result should be captured");
        expect(posix_dot_full_path != state.globals.end(), "POSIX dot-segment FULLPATH result should be captured");
        expect(posix_mixed_full_path != state.globals.end(), "POSIX mixed-separator FULLPATH result should be captured");
        expect(relative_win_full_path != state.globals.end(), "relative Windows-style FULLPATH result should be captured");
        expect(relative_posix_full_path != state.globals.end(), "relative POSIX FULLPATH result should be captured");
        expect(forced_ext != state.globals.end(), "FORCEEXT result should be captured");
        expect(forced_ext_with_dot != state.globals.end(), "FORCEEXT dotted-extension result should be captured");
        expect(default_ext_added != state.globals.end(), "DEFAULTEXT add result should be captured");
        expect(default_ext_kept != state.globals.end(), "DEFAULTEXT keep result should be captured");
        expect(forced_path != state.globals.end(), "FORCEPATH Windows result should be captured");
        expect(forced_posix_path != state.globals.end(), "FORCEPATH POSIX result should be captured");
        expect(forced_unc_path != state.globals.end(), "FORCEPATH UNC result should be captured");
        expect(forced_mixed_drive_path != state.globals.end(), "mixed drive FORCEPATH result should be captured");
        expect(forced_mixed_unc_path != state.globals.end(), "mixed UNC FORCEPATH result should be captured");
        expect(forced_mixed_posix_path != state.globals.end(), "mixed POSIX FORCEPATH result should be captured");
        expect(forced_mixed_relative_backslash_path != state.globals.end(), "backslash-first mixed relative FORCEPATH result should be captured");
        expect(forced_mixed_relative_slash_path != state.globals.end(), "slash-first mixed relative FORCEPATH result should be captured");
        expect(forced_relative_backslash_path != state.globals.end(), "clean backslash-relative FORCEPATH result should be captured");
        expect(forced_relative_slash_path != state.globals.end(), "clean slash-relative FORCEPATH result should be captured");
        expect(forced_relative_no_separator_path != state.globals.end(), "separator-free relative FORCEPATH result should be captured");
        expect(forced_trailing_drive_path != state.globals.end(), "trailing drive FORCEPATH result should be captured");
        expect(forced_trailing_posix_path != state.globals.end(), "trailing POSIX FORCEPATH result should be captured");
        expect(forced_drive_relative_path != state.globals.end(), "drive-relative FORCEPATH result should be captured");
        expect(forced_drive_root_path != state.globals.end(), "drive-root FORCEPATH result should be captured");
        expect(forced_posix_root_path != state.globals.end(), "POSIX-root FORCEPATH result should be captured");
        expect(forced_windows_root_path != state.globals.end(), "Windows-root FORCEPATH result should be captured");
        expect(forced_namespace_drive_path != state.globals.end(), "namespace-drive FORCEPATH result should be captured");
        expect(forced_namespace_unc_path != state.globals.end(), "namespace-UNC FORCEPATH result should be captured");
        expect(forced_repeated_separator_path != state.globals.end(), "repeated-separator FORCEPATH result should be captured");
        expect(forced_empty_directory_path != state.globals.end(), "empty-directory FORCEPATH result should be captured");
        expect(forced_empty_source_path != state.globals.end(), "empty-source FORCEPATH result should be captured");
        expect(forced_both_empty_path != state.globals.end(), "fully empty FORCEPATH result should be captured");
        expect(current_dir != state.globals.end(), "CURDIR result should be captured");

        if (drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(drive->second) == "E:", "JUSTDRIVE should parse drive-letter roots on every host");
        }
        if (unc_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_drive->second) == "\\\\server\\share",
                   "#3964: JUSTDRIVE should preserve ordinary UNC server/share roots");
        }
        if (unc_root_backslash_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_root_backslash_drive->second) == "\\\\server\\share",
                   "#4050: JUSTDRIVE should preserve an ordinary backslash UNC share root");
        }
        if (unc_root_backslash_trailing_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_root_backslash_trailing_drive->second) == "\\\\server\\share",
                   "#4050: JUSTDRIVE should remove a trailing separator from a backslash UNC share root");
        }
        if (unc_root_slash_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_root_slash_drive->second) == "//server/share",
                   "#4050: JUSTDRIVE should preserve an ordinary slash UNC share root");
        }
        if (unc_root_slash_trailing_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_root_slash_trailing_drive->second) == "//server/share",
                   "#4050: JUSTDRIVE should remove a trailing separator from a slash UNC share root");
        }
        if (malformed_unc_backslash_empty_server != state.globals.end())
        {
            expect(copperfin::runtime::format_value(malformed_unc_backslash_empty_server->second).empty(),
                   "#4050: JUSTDRIVE should reject an ordinary backslash UNC root with an empty server");
        }
        if (malformed_unc_slash_empty_server != state.globals.end())
        {
            expect(copperfin::runtime::format_value(malformed_unc_slash_empty_server->second).empty(),
                   "#4050: JUSTDRIVE should reject an ordinary slash UNC root with an empty server");
        }
        if (extended_unc_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(extended_unc_drive->second) == "\\\\?\\UNC\\server\\share",
                   "#3964: JUSTDRIVE should preserve complete extended UNC server/share roots");
        }
        if (extended_unc_root_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(extended_unc_root_drive->second) == "\\\\?\\UNC\\server\\share",
                   "#3964: JUSTDRIVE should accept an extended UNC root without a trailing path");
        }
        if (lower_extended_unc_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(lower_extended_unc_drive->second) == "\\\\?\\unc\\server\\share",
                   "#3964: JUSTDRIVE should recognize the extended UNC marker case-insensitively");
        }
        if (extended_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(extended_drive->second) == "\\\\?\\E:",
                   "#3964: JUSTDRIVE should preserve existing extended drive behavior");
        }
        if (device_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(device_drive->second) == "\\\\.\\E:",
                   "#3964: JUSTDRIVE should preserve explicit device drive behavior");
        }
        if (malformed_extended_unc_missing_share != state.globals.end())
        {
            expect(copperfin::runtime::format_value(malformed_extended_unc_missing_share->second).empty(),
                   "#3964: JUSTDRIVE should reject an extended UNC path without a share");
        }
        if (malformed_extended_unc_empty_server != state.globals.end())
        {
            expect(copperfin::runtime::format_value(malformed_extended_unc_empty_server->second).empty(),
                   "#3964: JUSTDRIVE should reject an extended UNC path without a server");
        }
        if (global_root_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(global_root_drive->second).empty(),
                   "#3964: JUSTDRIVE should not reinterpret GLOBALROOT as a drive root");
        }
        if (device_pipe_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(device_pipe_drive->second).empty(),
                   "#3964: JUSTDRIVE should not reinterpret a device pipe as a drive root");
        }
        if (posix_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_drive->second).empty(),
                   "#3964: JUSTDRIVE should keep POSIX paths drive-less");
        }
        if (relative_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(relative_drive->second).empty(),
                   "#3964: JUSTDRIVE should keep relative paths drive-less");
        }
        if (empty_drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(empty_drive->second).empty(),
                   "#3964: JUSTDRIVE should keep empty paths drive-less");
        }
        if (win_dir != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_dir->second) == "E:\\Project-Copperfin\\src\\runtime",
                   "JUSTPATH should parse Windows-style backslash paths on every host");
        }
        if (win_name != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_name->second) == "prg_engine.cpp",
                   "JUSTFNAME should parse Windows-style file names on every host");
        }
        if (win_stem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_stem->second) == "prg_engine",
                   "JUSTSTEM should parse Windows-style stems on every host");
        }
        if (win_ext != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_ext->second) == "cpp",
                   "JUSTEXT should parse Windows-style extensions on every host");
        }
        if (posix_dir != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_dir->second) == "/home/rich/dev/Project-Copperfin/src/runtime",
                   "JUSTPATH should continue parsing POSIX-style paths");
        }
        if (posix_name != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_name->second) == "prg_engine.cpp",
                   "JUSTFNAME should continue parsing POSIX-style file names");
        }
        if (posix_stem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_stem->second) == "prg_engine",
                   "JUSTSTEM should continue parsing POSIX-style stems");
        }
        if (posix_ext != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_ext->second) == "cpp",
                   "JUSTEXT should continue parsing POSIX-style extensions");
        }
        if (win_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_full_path->second) == "E:\\Project-Copperfin\\src\\runtime\\prg_engine.cpp",
                   "FULLPATH should preserve Windows drive-rooted absolute paths on every host");
        }
        if (unc_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_full_path->second) == "\\\\server\\share\\reports\\invoice.frx",
                   "FULLPATH should preserve UNC absolute paths on every host");
        }
        if (win_dot_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_dot_full_path->second) == "E:\\Project-Copperfin\\src\\main.prg",
                   "#3962: FULLPATH should normalize Windows drive-absolute dot segments on every host");
        }
        if (win_mixed_dot_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_mixed_dot_full_path->second) == "E:\\Project-Copperfin\\main.prg",
                   "#3962: FULLPATH should normalize mixed Windows separators and parent segments");
        }
        if (win_root_boundary_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_root_boundary_full_path->second) == "E:\\main.prg",
                   "#3962: FULLPATH should not reduce a drive-absolute path above its root");
        }
        if (unc_dot_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_dot_full_path->second) == "\\\\server\\share\\reports\\invoice.frx",
                   "#3962: FULLPATH should normalize UNC dot segments while preserving the share root");
        }
        if (unc_mixed_dot_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_mixed_dot_full_path->second) == "\\\\server\\share\\invoice.frx",
                   "#3962: FULLPATH should normalize mixed UNC separators to Windows separators");
        }
        if (unc_root_boundary_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_root_boundary_full_path->second) == "\\\\server\\share\\invoice.frx",
                   "#3962: FULLPATH should not reduce a UNC path above its share root");
        }
        if (extended_unc_dot_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(extended_unc_dot_full_path->second) == "\\\\?\\UNC\\server\\share\\invoice.frx",
                   "#3962: FULLPATH should lock the share root of extended UNC paths");
        }
        if (extended_drive_dot_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(extended_drive_dot_full_path->second) == "\\\\?\\E:\\invoice.frx",
                   "#3962: FULLPATH should normalize extended drive paths without host-root interpretation");
        }
        if (device_pipe_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(device_pipe_full_path->second) == "\\\\.\\pipe\\alpha\\..\\beta",
                   "#3962: FULLPATH should preserve non-filesystem device namespaces byte-for-byte");
        }
        if (global_root_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(global_root_full_path->second) ==
                       "\\\\?\\GLOBALROOT\\Device\\HarddiskVolumeShadowCopy1\\..\\file",
                   "#3962: FULLPATH should not reinterpret GLOBALROOT namespace paths as UNC shares");
        }
        if (posix_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_full_path->second) == "/home/rich/dev/Project-Copperfin/src/runtime/prg_engine.cpp",
                   "FULLPATH should preserve POSIX absolute paths");
        }
        if (posix_dot_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_dot_full_path->second) == "/opt/shared/main.prg",
                   "FULLPATH should normalize POSIX absolute dot segments on every host");
        }
        if (posix_mixed_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_mixed_full_path->second) == "/opt/copperfin/main.prg",
                   "FULLPATH should normalize VFP backslashes inside POSIX absolute paths on every host");
        }
        if (relative_win_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(relative_win_full_path->second) == (temp_root / "forms" / "main.prg").string(),
                   "FULLPATH should treat backslashes as separators for relative VFP paths");
        }
        if (relative_posix_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(relative_posix_full_path->second) == (temp_root / "forms" / "report.prg").string(),
                   "FULLPATH should continue resolving relative POSIX-style paths against CURDIR");
        }
        if (forced_ext != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_ext->second) == "E:\\Project-Copperfin\\src\\runtime\\prg_engine.h",
                   "FORCEEXT should replace an extension on Windows-style paths");
        }
        if (forced_ext_with_dot != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_ext_with_dot->second) == "E:\\Project-Copperfin\\src\\runtime\\prg_engine.hpp",
                   "FORCEEXT should accept a leading dot in the requested extension");
        }
        if (default_ext_added != state.globals.end())
        {
            expect(copperfin::runtime::format_value(default_ext_added->second) == "D:\\generated\\report.frx",
                   "DEFAULTEXT should append an extension only when one is missing");
        }
        if (default_ext_kept != state.globals.end())
        {
            expect(copperfin::runtime::format_value(default_ext_kept->second) == "D:\\generated\\report.frx",
                   "DEFAULTEXT should preserve an existing extension");
        }
        if (forced_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_path->second) == "D:\\generated\\prg_engine.cpp",
                   "FORCEPATH should replace a Windows-style directory on every host");
        }
        if (forced_posix_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_posix_path->second) == "/tmp/generated/prg_engine.cpp",
                   "FORCEPATH should replace a POSIX-style directory");
        }
        if (forced_unc_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_unc_path->second) == "\\\\server\\share\\generated\\foo.txt",
                   "#3965: FORCEPATH should preserve a clean UNC directory style");
        }
        if (forced_mixed_drive_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_mixed_drive_path->second) == "D:\\generated\\subdir\\foo.txt",
                   "#3965: FORCEPATH should normalize mixed drive paths to Windows separators");
        }
        if (forced_mixed_unc_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_mixed_unc_path->second) == "\\\\server\\share\\generated\\foo.txt",
                   "#3965: FORCEPATH should normalize mixed UNC paths to Windows separators");
        }
        if (forced_mixed_posix_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_mixed_posix_path->second) == "/tmp/generated/subdir/foo.txt",
                   "#3965: FORCEPATH should normalize mixed POSIX paths to slash separators");
        }
        if (forced_mixed_relative_backslash_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_mixed_relative_backslash_path->second) ==
                       "generated\\subdir\\leaf\\foo.txt",
                   "#3965: FORCEPATH should use the first relative separator when it is a backslash");
        }
        if (forced_mixed_relative_slash_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_mixed_relative_slash_path->second) ==
                       "generated/subdir/leaf/foo.txt",
                   "#3965: FORCEPATH should use the first relative separator when it is a slash");
        }
        if (forced_relative_backslash_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_relative_backslash_path->second) ==
                       "generated\\subdir\\foo.txt",
                   "#3965: FORCEPATH should preserve clean backslash-relative paths");
        }
        if (forced_relative_slash_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_relative_slash_path->second) ==
                       "generated/subdir/foo.txt",
                   "#3965: FORCEPATH should preserve clean slash-relative paths");
        }
        if (forced_relative_no_separator_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_relative_no_separator_path->second) == "generated/foo.txt",
                   "#3965: FORCEPATH should preserve the existing separator-free relative join");
        }
        if (forced_trailing_drive_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_trailing_drive_path->second) == "D:\\generated\\foo.txt",
                   "#3965: FORCEPATH should normalize an existing trailing drive separator without duplication");
        }
        if (forced_trailing_posix_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_trailing_posix_path->second) == "/tmp/generated/foo.txt",
                   "#3965: FORCEPATH should normalize an existing trailing POSIX separator without duplication");
        }
        if (forced_drive_relative_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_drive_relative_path->second) ==
                       "D:generated\\subdir\\foo.txt",
                   "#3965: FORCEPATH should preserve drive-relative semantics while normalizing separators");
        }
        if (forced_drive_root_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_drive_root_path->second) == "D:\\foo.txt",
                   "#3965: FORCEPATH should join a drive root with one Windows separator");
        }
        if (forced_posix_root_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_posix_root_path->second) == "/foo.txt",
                   "#3965: FORCEPATH should join a POSIX root without duplicating its separator");
        }
        if (forced_windows_root_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_windows_root_path->second) == "\\foo.txt",
                   "#3965: FORCEPATH should join a Windows current-drive root without duplicating its separator");
        }
        if (forced_namespace_drive_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_namespace_drive_path->second) ==
                       "\\\\?\\E:\\generated\\foo.txt",
                   "#3965: FORCEPATH should normalize namespace drive directories as Windows paths");
        }
        if (forced_namespace_unc_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_namespace_unc_path->second) ==
                       "\\\\?\\UNC\\server\\share\\generated\\foo.txt",
                   "#3965: FORCEPATH should normalize namespace UNC directories as Windows paths");
        }
        if (forced_repeated_separator_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_repeated_separator_path->second) ==
                       "D:\\generated\\\\subdir\\foo.txt",
                   "#3965: FORCEPATH should normalize style without collapsing repeated separators");
        }
        if (forced_empty_directory_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_empty_directory_path->second) == "foo.txt",
                   "#3965: FORCEPATH should preserve filename-only behavior for an empty directory");
        }
        if (forced_empty_source_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_empty_source_path->second) == "D:\\generated\\",
                   "#3965: FORCEPATH should preserve the normalized directory for an empty source path");
        }
        if (forced_both_empty_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_both_empty_path->second).empty(),
                   "#3965: FORCEPATH should preserve the fully empty input result");
        }
        if (current_dir != state.globals.end())
        {
            expect(copperfin::runtime::format_value(current_dir->second) == temp_root.string(),
                   "CURDIR should expose the runtime working directory");
        }

        // #5910: ADDBS() vectors run through the real PRG parser/dispatch
        // path (see the script above), not just a direct C++ call, per
        // #5910's own acceptance criteria that direct expressions,
        // generated artifacts, and runtime package execution must agree.
        // Confirmed against actual VFP9 output (retained differential
        // evidence: /home/rich/temp/vfp9-probes/addbs-contract-36.{prg,out}).
        const auto addbs_empty = state.globals.find("caddbsempty");
        const auto addbs_plain = state.globals.find("caddbsplain");
        const auto addbs_trailing_slash = state.globals.find("caddbstrailingslash");
        const auto addbs_trailing_backslash = state.globals.find("caddbstrailingbackslash");
        const auto addbs_drive_letter = state.globals.find("caddbsdriveletter");
        const auto addbs_drive_root = state.globals.find("caddbsdriveroot");
        const auto addbs_unc = state.globals.find("caddbsunc");
        if (addbs_empty != state.globals.end())
        {
            expect(copperfin::runtime::format_value(addbs_empty->second).empty(),
                   "#5910: ADDBS('') should remain empty through the PRG parser/dispatch path");
        }
        if (addbs_plain != state.globals.end())
        {
            expect(copperfin::runtime::format_value(addbs_plain->second) == "abc\\",
                   "#5910: ADDBS('abc') should append a backslash through the PRG parser/dispatch path");
        }
        if (addbs_trailing_slash != state.globals.end())
        {
            expect(copperfin::runtime::format_value(addbs_trailing_slash->second) == "abc/\\",
                   "#5910: ADDBS('abc/') should append a backslash -- a trailing forward slash is not a "
                   "terminating separator in real VFP9, through the PRG parser/dispatch path");
        }
        if (addbs_trailing_backslash != state.globals.end())
        {
            expect(copperfin::runtime::format_value(addbs_trailing_backslash->second) == "abc\\",
                   "#5910: ADDBS('abc\\\\') should leave an existing trailing backslash unchanged through the "
                   "PRG parser/dispatch path");
        }
        if (addbs_drive_letter != state.globals.end())
        {
            expect(copperfin::runtime::format_value(addbs_drive_letter->second) == "C:\\",
                   "#5910: ADDBS('C:') should append a backslash through the PRG parser/dispatch path");
        }
        if (addbs_drive_root != state.globals.end())
        {
            expect(copperfin::runtime::format_value(addbs_drive_root->second) == "C:\\",
                   "#5910: ADDBS('C:\\\\') should leave an existing drive-root backslash unchanged through the "
                   "PRG parser/dispatch path");
        }
        if (addbs_unc != state.globals.end())
        {
            expect(copperfin::runtime::format_value(addbs_unc->second) == "\\\\server\\share\\",
                   "#5910: ADDBS() on a UNC path should append a backslash through the PRG parser/dispatch path");
        }

        fs::remove_all(temp_root, ignored);
    }

#if !defined(_WIN32)
    void test_fullpath_handles_unavailable_current_directory()
    {
        namespace fs = std::filesystem;
        const fs::path original_directory = fs::current_path();
        const fs::path temporary_directory = fs::temp_directory_path() / "copperfin_fullpath_unavailable_cwd";
        std::error_code ignored;
        fs::remove_all(temporary_directory, ignored);
        fs::create_directories(temporary_directory);
        fs::current_path(temporary_directory);
        fs::remove_all(temporary_directory, ignored);

        copperfin::runtime::PrgValue argument;
        argument.kind = copperfin::runtime::PrgValueKind::string;
        argument.string_value = "forms/main.prg";

        bool threw = false;
        std::optional<copperfin::runtime::PrgValue> result;
        try
        {
            result = copperfin::runtime::evaluate_path_function("fullpath", {argument}, {});
        }
        catch (...)
        {
            threw = true;
        }

        std::error_code restore_error;
        fs::current_path(original_directory, restore_error);
        expect(!restore_error, "FULLPATH unavailable-CWD regression should restore the original directory");
        expect(!threw, "FULLPATH should not throw when absolute-path resolution cannot read the current directory");
        expect(result.has_value(), "FULLPATH should return a controlled value when absolute-path resolution fails");
        if (result.has_value())
        {
            expect(result->kind == copperfin::runtime::PrgValueKind::string,
                   "FULLPATH unavailable-CWD fallback should remain a string");
            expect(result->string_value == "forms/main.prg",
                   "FULLPATH unavailable-CWD fallback should preserve the normalized relative path");
        }
    }
#endif

    void test_addbs_matches_vfp9_trailing_separator_contract()
    {
        // #5910: real VFP9 SP2's ADDBS() adds a backslash unless the
        // nonempty input already ends in a backslash -- a trailing
        // forward slash does NOT satisfy that contract. Confirmed
        // against actual VFP9 output (retained differential evidence:
        // /home/rich/temp/vfp9-probes/addbs-contract-36.{prg,out}):
        // ADDBS('')='', ADDBS('abc')='abc\', ADDBS('abc/')='abc/\',
        // ADDBS('C:')='C:\', ADDBS('C:\')='C:\' (unchanged).
        const auto addbs = [](const std::string& input) {
            copperfin::runtime::PrgValue argument;
            argument.kind = copperfin::runtime::PrgValueKind::string;
            argument.string_value = input;
            const auto result = copperfin::runtime::evaluate_path_function("addbs", {argument}, {});
            return result.has_value() ? result->string_value : std::string{"<no result>"};
        };

        expect(addbs("") == "", "ADDBS('') should remain empty");
        expect(addbs("abc") == "abc\\", "ADDBS('abc') should append a backslash");
        expect(addbs("abc/") == "abc/\\",
               "ADDBS('abc/') should append a backslash -- a trailing forward slash is not a "
               "terminating separator in real VFP9");
        expect(addbs("abc\\") == "abc\\", "ADDBS('abc\\\\') should leave an existing trailing backslash unchanged");
        expect(addbs("C:") == "C:\\", "ADDBS('C:') should append a backslash to a bare drive letter");
        expect(addbs("C:\\") == "C:\\", "ADDBS('C:\\\\') should leave an existing drive-root backslash unchanged");
        expect(addbs("\\\\server\\share") == "\\\\server\\share\\",
               "ADDBS() on a UNC path should append a backslash when missing");
        expect(addbs("\\\\server\\share/") == "\\\\server\\share/\\",
               "ADDBS() on a UNC path ending in a forward slash should still append a backslash");
    }

    // Installed VFP9 SP2 (Windows VM COM probes 2026-09-30, retained under
    // /home/rich/temp/vfp9-probes/path-funcs-6580/): a drive-relative path with no separator
    // drops its drive designator from the file name (#6581), and a path longer than 259 bytes
    // raises error 202 "Invalid path or file name." for the JUST*, FORCEEXT, DEFAULTEXT and
    // FORCEPATH functions but not ADDBS (#6580, #6595).
    void test_drive_relative_names_and_the_259_byte_path_limit()
    {
        const auto call = [](const std::string& function, const std::string& first, const std::string& second = {}) {
            std::vector<copperfin::runtime::PrgValue> arguments;
            copperfin::runtime::PrgValue value;
            value.kind = copperfin::runtime::PrgValueKind::string;
            value.string_value = first;
            arguments.push_back(value);
            if (!second.empty()) {
                value.string_value = second;
                arguments.push_back(value);
            }
            const auto result = copperfin::runtime::evaluate_path_function(function, arguments, {});
            return result.has_value() ? result->string_value : std::string{"<no result>"};
        };

        // Drive-relative: JUSTFNAME/JUSTSTEM no longer leak the drive; controls are unchanged.
        expect(call("justfname", "C:foo.txt") == "foo.txt", "#6581: JUSTFNAME('C:foo.txt')");
        expect(call("juststem", "C:foo.txt") == "foo", "#6581: JUSTSTEM('C:foo.txt')");
        expect(call("justfname", "c:foo.txt") == "foo.txt", "#6581: JUSTFNAME with a lowercase drive");
        expect(call("juststem", "c:foo.txt") == "foo", "#6581: JUSTSTEM with a lowercase drive");
        expect(call("justfname", "C:foo") == "foo", "#6581: JUSTFNAME without an extension");
        expect(call("juststem", "C:foo") == "foo", "#6581: JUSTSTEM without an extension");
        expect(call("justfname", "C:folder\\foo.txt") == "foo.txt", "#6581: JUSTFNAME control with a folder");
        expect(call("juststem", "C:folder\\foo.txt") == "foo", "#6581: JUSTSTEM control with a folder");
        expect(call("justdrive", "C:foo.txt") == "C:", "#6581: JUSTDRIVE control");
        expect(call("justpath", "C:foo.txt").empty(), "#6581: JUSTPATH control");
        expect(call("justext", "C:foo.txt") == "txt", "#6581: JUSTEXT control");
        expect(call("justext", "C:.profile") == "profile",
               "#6581: drive stripping stays local to JUSTFNAME/JUSTSTEM/FORCEPATH (leading dots are #5916)");
        expect(call("justfname", "C:").empty(), "#6581: JUSTFNAME('C:') is empty");
        expect(call("justfname", "foo.txt") == "foo.txt", "#6581: a plain name is unchanged");
        expect(call("forcepath", "C:foo.txt", "D:\\x") == "D:\\x\\foo.txt", "#6581: FORCEPATH drops the drive of a drive-relative name");
        expect(call("forceext", "C:foo.txt", "bak") == "C:foo.bak", "#6581: FORCEEXT keeps the drive designator");

        // 259 bytes is accepted; 260 is error 202, counted in bytes.
        const auto path_of_length = [](std::size_t length) {
            const std::string tail = "xxxxxxxx.txt";
            std::string path = "C:\\" + std::string(length - 3U - 1U - 1U - tail.size() - 40U, 'a') + "\\" + std::string(40U, 'b') + "\\" + tail;
            return path;
        };
        const std::string at_limit = path_of_length(259U);
        const std::string over_limit = path_of_length(260U);
        expect(at_limit.size() == 259U && over_limit.size() == 260U, "#6580: fixture lengths");
        // The error carries VFP number 202 directly; its prose is localized, so it is not compared.
        const auto throws_invalid_path = [&](const std::string& function, const std::string& path, const std::string& second = {}) {
            try {
                (void)call(function, path, second);
            } catch (const copperfin::runtime::PrgCompatibilityError& error) {
                return error.error_code() == 202;
            }
            return false;
        };
        for (const std::string function : {"justext", "justpath", "justfname", "juststem", "justdrive"}) {
            expect(call(function, at_limit) != "<no result>" && !throws_invalid_path(function, at_limit),
                   "#6580/#6595: " + function + " accepts a 259-byte path");
            expect(throws_invalid_path(function, over_limit), "#6580/#6595: " + function + " rejects a 260-byte path");
        }
        expect(call("justext", at_limit) == "txt", "#6580: JUSTEXT result at 259 bytes");
        expect(call("justpath", at_limit).size() == 246U, "#6595: JUSTPATH result at 259 bytes");
        for (const std::string function : {"forceext", "defaultext", "forcepath"}) {
            const std::string second = function == "forcepath" ? "D:\\x" : "bak";
            expect(!throws_invalid_path(function, at_limit, second), "#6580: " + function + " accepts a 259-byte path");
            expect(throws_invalid_path(function, over_limit, second), "#6580: " + function + " rejects a 260-byte path");
        }
        expect(call("addbs", over_limit).size() == 261U, "#6580: ADDBS has no path-length limit in VFP9");

        // Bytes, not characters: 130 two-byte UTF-8 characters are 260 bytes.
        std::string multibyte = "C:\\";
        for (int index = 0; index < 120; ++index) {
            multibyte += "\xC3\xA9";
        }
        multibyte += ".txt";  // 3 + 240 + 4 = 247 bytes: accepted
        expect(!throws_invalid_path("justext", multibyte), "#6580: a 247-byte multibyte path is accepted");
        for (int index = 0; index < 7; ++index) {
            multibyte += "\xC3\xA9";
        }
        multibyte += "xxx";  // 264 bytes but only 137 characters (127 two-byte + 10 ASCII)
        expect(throws_invalid_path("justext", multibyte), "#6580: the limit counts bytes, not characters");
    }

    // The 259-byte violation is a catchable runtime error with VFP error number 202.
    void test_path_limit_error_is_catchable_with_error_202()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_path_limit_error_6580";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const fs::path script_path = temp_root / "limit.prg";
        write_text(
            script_path,
            "cPath = 'C:\\' + REPLICATE('a',120) + '\\' + REPLICATE('b',123) + '\\' + 'xxxxxxxx.txt'\n"
            "nLen = LEN(cPath)\n"
            "nCode = 0\n"
            "TRY\n"
            "cExt = JUSTEXT(cPath)\n"
            "CATCH TO oErr\n"
            "nCode = oErr.ErrorNo\n"
            "cMessage = oErr.Message\n"
            "ENDTRY\n"
            "lAfter = .T.\n"
            "RETURN\n");
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), temp_root.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#6580: the script should finish after catching the error: " + state.message);
        const auto len = state.globals.find("nlen");
        const auto code = state.globals.find("ncode");
        expect(len != state.globals.end() && copperfin::runtime::format_value(len->second) == "260",
               "#6580: the fixture path is 260 bytes");
        expect(code != state.globals.end() && copperfin::runtime::format_value(code->second) == "202",
               "#6580: JUSTEXT over 259 bytes should raise error 202");
        const auto catalog = copperfin::localization::load_catalogs(
            copperfin::localization::resolve_catalog_root(),
            copperfin::localization::select_locale());
        const auto message = state.globals.find("cmessage");
        expect(message != state.globals.end() &&
                   copperfin::runtime::format_value(message->second) ==
                       catalog.translate("Runtime.Prg.Expression.Error.InvalidPathOrFileName"),
               "#6580: the error text should be the active locale's message");
        fs::remove_all(temp_root, ignored);
    }

    // #5916: leading-dot, trailing-dot, dot-only, double-dot and multiple-dot file names.
    // Every expected value below is the output of installed VFP9 09.00.0000.7423 (Windows VM COM,
    // retained at /home/rich/temp/vfp9-probes/path-funcs-6580/vfp9-result-dots.txt): a cross
    // product of 21 names and 6 extension arguments, plus JUSTSTEM/JUSTEXT/JUSTFNAME/JUSTPATH and
    // FORCEPATH per name.
    void test_dot_edge_file_names_match_vfp9()
    {
        const auto call = [](const std::string& function, const std::string& first, const std::string& second = {},
                             bool has_second = false) {
            std::vector<copperfin::runtime::PrgValue> arguments;
            copperfin::runtime::PrgValue value;
            value.kind = copperfin::runtime::PrgValueKind::string;
            value.string_value = first;
            arguments.push_back(value);
            if (has_second) {
                value.string_value = second;
                arguments.push_back(value);
            }
            const auto result = copperfin::runtime::evaluate_path_function(function, arguments, {});
            return result.has_value() ? result->string_value : std::string{"<no result>"};
        };

        struct NameRow { const char* name; const char* stem; const char* ext; const char* fname; const char* path; };
        const std::vector<NameRow> names = {
        {".profile", "", "profile", ".profile", ""},
        {"a.", "a", "", "a.", ""},
        {"a", "a", "", "a", ""},
        {".", "", "", ".", ""},
        {"..", ".", "", "..", ""},
        {"...", "..", "", "...", ""},
        {"a.b.c", "a.b", "c", "a.b.c", ""},
        {".a.b", ".a", "b", ".a.b", ""},
        {"a..b", "a.", "b", "a..b", ""},
        {"a.b.", "a.b", "", "a.b.", ""},
        {"..a", ".", "a", "..a", ""},
        {"dir\\.profile", "", "profile", ".profile", "dir"},
        {"dir/.profile", "", "profile", ".profile", "dir"},
        {"dir\\a.", "a", "", "a.", "dir"},
        {"C:.profile", "", "profile", ".profile", ""},
        {"C:a.", "a", "", "a.", ""},
        {"C:folder\\.profile", "", "profile", ".profile", "C:folder"},
        {"x\\.", "", "", ".", "x"},
        {"x\\..", ".", "", "..", "x"},
        {".a.", ".a", "", ".a.", ""},
        {"a.b.c.", "a.b.c", "", "a.b.c.", ""}
        };
        for (const NameRow& row : names) {
            const std::string label = std::string("#5916 ") + row.name + ": ";
            expect(call("juststem", row.name) == row.stem, label + "JUSTSTEM, got [" + call("juststem", row.name) + "]");
            expect(call("justext", row.name) == row.ext, label + "JUSTEXT, got [" + call("justext", row.name) + "]");
            expect(call("justfname", row.name) == row.fname, label + "JUSTFNAME, got [" + call("justfname", row.name) + "]");
            expect(call("justpath", row.name) == row.path, label + "JUSTPATH, got [" + call("justpath", row.name) + "]");
        }

        struct ExtRow { const char* name; const char* extension; const char* forced; const char* defaulted; };
        const std::vector<ExtRow> extension_rows = {
        {".profile", "", "", ".profile"},
        {".profile", ".bak", "", ".profile"},
        {".profile", "bak", "", ".profile"},
        {".profile", "b.k", "", ".profile"},
        {".profile", ".", "", ".profile"},
        {".profile", "bak.", "", ".profile"},
        {"a.", "", "a", "a."},
        {"a.", ".bak", "a.bak", "a."},
        {"a.", "bak", "a.bak", "a."},
        {"a.", "b.k", "a.b.k", "a."},
        {"a.", ".", "a.", "a."},
        {"a.", "bak.", "a.bak.", "a."},
        {"a", "", "a", "a"},
        {"a", ".bak", "a.bak", "a.bak"},
        {"a", "bak", "a.bak", "a.bak"},
        {"a", "b.k", "a.b.k", "a.b.k"},
        {"a", ".", "a.", "a."},
        {"a", "bak.", "a.bak.", "a.bak."},
        {".", "", "", "."},
        {".", ".bak", "", "."},
        {".", "bak", "", "."},
        {".", "b.k", "", "."},
        {".", ".", "", "."},
        {".", "bak.", "", "."},
        {"..", "", ".", ".."},
        {"..", ".bak", "..bak", ".."},
        {"..", "bak", "..bak", ".."},
        {"..", "b.k", "..b.k", ".."},
        {"..", ".", "..", ".."},
        {"..", "bak.", "..bak.", ".."},
        {"...", "", "..", "..."},
        {"...", ".bak", "...bak", "..."},
        {"...", "bak", "...bak", "..."},
        {"...", "b.k", "...b.k", "..."},
        {"...", ".", "...", "..."},
        {"...", "bak.", "...bak.", "..."},
        {"a.b.c", "", "a.b", "a.b.c"},
        {"a.b.c", ".bak", "a.b.bak", "a.b.c"},
        {"a.b.c", "bak", "a.b.bak", "a.b.c"},
        {"a.b.c", "b.k", "a.b.b.k", "a.b.c"},
        {"a.b.c", ".", "a.b.", "a.b.c"},
        {"a.b.c", "bak.", "a.b.bak.", "a.b.c"},
        {".a.b", "", ".a", ".a.b"},
        {".a.b", ".bak", ".a.bak", ".a.b"},
        {".a.b", "bak", ".a.bak", ".a.b"},
        {".a.b", "b.k", ".a.b.k", ".a.b"},
        {".a.b", ".", ".a.", ".a.b"},
        {".a.b", "bak.", ".a.bak.", ".a.b"},
        {"a..b", "", "a.", "a..b"},
        {"a..b", ".bak", "a..bak", "a..b"},
        {"a..b", "bak", "a..bak", "a..b"},
        {"a..b", "b.k", "a..b.k", "a..b"},
        {"a..b", ".", "a..", "a..b"},
        {"a..b", "bak.", "a..bak.", "a..b"},
        {"a.b.", "", "a.b", "a.b."},
        {"a.b.", ".bak", "a.b.bak", "a.b."},
        {"a.b.", "bak", "a.b.bak", "a.b."},
        {"a.b.", "b.k", "a.b.b.k", "a.b."},
        {"a.b.", ".", "a.b.", "a.b."},
        {"a.b.", "bak.", "a.b.bak.", "a.b."},
        {"..a", "", ".", "..a"},
        {"..a", ".bak", "..bak", "..a"},
        {"..a", "bak", "..bak", "..a"},
        {"..a", "b.k", "..b.k", "..a"},
        {"..a", ".", "..", "..a"},
        {"..a", "bak.", "..bak.", "..a"},
        {"dir\\.profile", "", "", "dir\\.profile"},
        {"dir\\.profile", ".bak", "", "dir\\.profile"},
        {"dir\\.profile", "bak", "", "dir\\.profile"},
        {"dir\\.profile", "b.k", "", "dir\\.profile"},
        {"dir\\.profile", ".", "", "dir\\.profile"},
        {"dir\\.profile", "bak.", "", "dir\\.profile"},
        {"dir/.profile", "", "", "dir/.profile"},
        {"dir/.profile", ".bak", "", "dir/.profile"},
        {"dir/.profile", "bak", "", "dir/.profile"},
        {"dir/.profile", "b.k", "", "dir/.profile"},
        {"dir/.profile", ".", "", "dir/.profile"},
        {"dir/.profile", "bak.", "", "dir/.profile"},
        {"dir\\a.", "", "dir\\a", "dir\\a."},
        {"dir\\a.", ".bak", "dir\\a.bak", "dir\\a."},
        {"dir\\a.", "bak", "dir\\a.bak", "dir\\a."},
        {"dir\\a.", "b.k", "dir\\a.b.k", "dir\\a."},
        {"dir\\a.", ".", "dir\\a.", "dir\\a."},
        {"dir\\a.", "bak.", "dir\\a.bak.", "dir\\a."},
        {"C:.profile", "", "", "C:.profile"},
        {"C:.profile", ".bak", "", "C:.profile"},
        {"C:.profile", "bak", "", "C:.profile"},
        {"C:.profile", "b.k", "", "C:.profile"},
        {"C:.profile", ".", "", "C:.profile"},
        {"C:.profile", "bak.", "", "C:.profile"},
        {"C:a.", "", "C:a", "C:a."},
        {"C:a.", ".bak", "C:a.bak", "C:a."},
        {"C:a.", "bak", "C:a.bak", "C:a."},
        {"C:a.", "b.k", "C:a.b.k", "C:a."},
        {"C:a.", ".", "C:a.", "C:a."},
        {"C:a.", "bak.", "C:a.bak.", "C:a."},
        {"C:folder\\.profile", "", "", "C:folder\\.profile"},
        {"C:folder\\.profile", ".bak", "", "C:folder\\.profile"},
        {"C:folder\\.profile", "bak", "", "C:folder\\.profile"},
        {"C:folder\\.profile", "b.k", "", "C:folder\\.profile"},
        {"C:folder\\.profile", ".", "", "C:folder\\.profile"},
        {"C:folder\\.profile", "bak.", "", "C:folder\\.profile"},
        {"x\\.", "", "", "x\\."},
        {"x\\.", ".bak", "", "x\\."},
        {"x\\.", "bak", "", "x\\."},
        {"x\\.", "b.k", "", "x\\."},
        {"x\\.", ".", "", "x\\."},
        {"x\\.", "bak.", "", "x\\."},
        {"x\\..", "", "x\\.", "x\\.."},
        {"x\\..", ".bak", "x\\..bak", "x\\.."},
        {"x\\..", "bak", "x\\..bak", "x\\.."},
        {"x\\..", "b.k", "x\\..b.k", "x\\.."},
        {"x\\..", ".", "x\\..", "x\\.."},
        {"x\\..", "bak.", "x\\..bak.", "x\\.."},
        {".a.", "", ".a", ".a."},
        {".a.", ".bak", ".a.bak", ".a."},
        {".a.", "bak", ".a.bak", ".a."},
        {".a.", "b.k", ".a.b.k", ".a."},
        {".a.", ".", ".a.", ".a."},
        {".a.", "bak.", ".a.bak.", ".a."},
        {"a.b.c.", "", "a.b.c", "a.b.c."},
        {"a.b.c.", ".bak", "a.b.c.bak", "a.b.c."},
        {"a.b.c.", "bak", "a.b.c.bak", "a.b.c."},
        {"a.b.c.", "b.k", "a.b.c.b.k", "a.b.c."},
        {"a.b.c.", ".", "a.b.c.", "a.b.c."},
        {"a.b.c.", "bak.", "a.b.c.bak.", "a.b.c."}
        };
        for (const ExtRow& row : extension_rows) {
            const std::string label = std::string("#5916 ") + row.name + " + '" + row.extension + "': ";
            expect(call("forceext", row.name, row.extension, true) == row.forced,
                   label + "FORCEEXT, got [" + call("forceext", row.name, row.extension, true) + "]");
            expect(call("defaultext", row.name, row.extension, true) == row.defaulted,
                   label + "DEFAULTEXT, got [" + call("defaultext", row.name, row.extension, true) + "]");
        }

        struct ForcePathRow { const char* name; const char* expected; };
        const std::vector<ForcePathRow> force_path_rows = {
        {".profile", "D:\\x\\.profile"},
        {"a.", "D:\\x\\a."},
        {"a", "D:\\x\\a"},
        {".", "D:\\x\\."},
        {"..", "D:\\x\\.."},
        {"...", "D:\\x\\..."},
        {"a.b.c", "D:\\x\\a.b.c"},
        {".a.b", "D:\\x\\.a.b"},
        {"a..b", "D:\\x\\a..b"},
        {"a.b.", "D:\\x\\a.b."},
        {"..a", "D:\\x\\..a"},
        {"dir\\.profile", "D:\\x\\.profile"},
        {"dir/.profile", "D:\\x\\.profile"},
        {"dir\\a.", "D:\\x\\a."},
        {"C:.profile", "D:\\x\\.profile"},
        {"C:a.", "D:\\x\\a."},
        {"C:folder\\.profile", "D:\\x\\.profile"},
        {"x\\.", "D:\\x\\."},
        {"x\\..", "D:\\x\\.."},
        {".a.", "D:\\x\\.a."},
        {"a.b.c.", "D:\\x\\a.b.c."}
        };
        for (const ForcePathRow& row : force_path_rows) {
            expect(call("forcepath", row.name, "D:\\x", true) == row.expected,
                   std::string("#5916 FORCEPATH(") + row.name + ") keeps dot-edge names intact");
        }
    }

    // Audit result for #5916: the shared portable_path_stem() helper is also used to match database
    // designators, where an empty stem for a hidden-style name would let unrelated databases match
    // each other. The PRG functions therefore use their own VFP-exact splitting and the designator
    // helper is unchanged.
    void test_shared_stem_helper_used_for_database_designators_is_unchanged()
    {
        expect(copperfin::runtime::portable_path_stem(".profile") == ".profile",
               "#5916: the shared stem helper keeps a leading-dot name intact for designator matching");
        expect(copperfin::runtime::portable_path_stem("sales.dbc") == "sales", "#5916: ordinary stems are unchanged");
        expect(copperfin::runtime::portable_path_stem("C:\\data\\sales.dbc") == "sales", "#5916: directory stems are unchanged");
    }

} // namespace

int main()
{
    test_portable_path_expression_functions();
#if !defined(_WIN32)
    test_fullpath_handles_unavailable_current_directory();
#endif
    test_addbs_matches_vfp9_trailing_separator_contract();
    test_drive_relative_names_and_the_259_byte_path_limit();
    test_path_limit_error_is_catchable_with_error_202();
    test_dot_edge_file_names_match_vfp9();
    test_shared_stem_helper_used_for_database_designators_is_unchanged();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_path_functions.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
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


} // namespace

int main()
{
    test_portable_path_expression_functions();
#if !defined(_WIN32)
    test_fullpath_handles_unavailable_current_directory();
#endif

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

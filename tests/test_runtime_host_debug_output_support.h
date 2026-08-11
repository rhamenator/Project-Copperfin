// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/security/audit_stream.h"
#include "copperfin/security/sha256.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#include <unistd.h>
#else
#include <process.h>
#endif

inline int failures = 0;

namespace {

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::ScopedEnvironmentPath;

[[maybe_unused]] std::filesystem::path runtime_host_audit_temp_root(const char* stem) {
    static std::atomic_uint sequence{0U};
#if defined(_WIN32)
    const unsigned long process_id = static_cast<unsigned long>(::_getpid());
#else
    const unsigned long process_id = static_cast<unsigned long>(::getpid());
#endif
    const unsigned int run_id = sequence.fetch_add(1U, std::memory_order_relaxed);
    return std::filesystem::temp_directory_path() /
        (std::string(stem) + "_" + std::to_string(process_id) + "_" + std::to_string(run_id));
}

[[maybe_unused]] void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

[[maybe_unused]] void write_text(const std::filesystem::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
}

[[maybe_unused]] std::string quote_manifest_value(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        if (ch == '\\') {
            escaped += "\\\\";
        } else if (ch == '\n') {
            escaped += "\\n";
        } else if (ch == '\r') {
            escaped += "\\r";
        } else {
            escaped.push_back(ch);
        }
    }
    return escaped;
}

[[maybe_unused]] void write_synthetic_database_index(const std::filesystem::path& path) {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    const auto write_le_u16 = [&](std::size_t offset, std::uint16_t value) {
        bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
        bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    };
    const auto write_le_u32 = [&](std::size_t offset, std::uint32_t value) {
        bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
        bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
        bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
        bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    };
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(1026U, 1U);
    write_le_u32(1028U, 4U * 512U);
    write_le_u16(4U * 512U, 0x0003U);
    write_le_u16((4U * 512U) + 2U, 1U);
    const std::string tag_name = "NAME";
    std::copy(tag_name.begin(), tag_name.end(), bytes.begin() + static_cast<std::ptrdiff_t>((3U * 512U) - 10U));
    const std::string expression = "UPPER(NAME)";
    std::copy(expression.begin(), expression.end(), bytes.begin() + static_cast<std::ptrdiff_t>((4U * 512U) + 24U));
    write_text(path, std::string(bytes.begin(), bytes.end()));
}

[[maybe_unused]] std::filesystem::path deployed_runtime_host_path(
    const std::filesystem::path& deployed_root,
    const std::string& runtime_host_path) {
    const std::filesystem::path runtime_host_file_name =
        std::filesystem::path(runtime_host_path).filename();
    if (!runtime_host_file_name.empty()) {
        return deployed_root / runtime_host_file_name;
    }
#if defined(_WIN32)
    return deployed_root / "copperfin_runtime_host.exe";
#else
    return deployed_root / "copperfin_runtime_host";
#endif
}

[[maybe_unused]] void write_runtime_host_usage_catalogs(const std::filesystem::path& locale_root) {
    const std::filesystem::path english_root = locale_root / "en-US";
    const std::filesystem::path spanish_root = locale_root / "es-419";
    const std::filesystem::path portuguese_root = locale_root / "pt-BR";
    const std::filesystem::path pseudo_root = locale_root / "qps-ploc";
    std::filesystem::create_directories(english_root);
    std::filesystem::create_directories(spanish_root);
    std::filesystem::create_directories(portuguese_root);
    std::filesystem::create_directories(pseudo_root);
    write_text(
        english_root / "strings.json",
        "{\n"
        "  \"RuntimeHost.Bridge.Error.CreateResponseDirectoryFailed\": \"Unable to create bridge response directory.\",\n"
        "  \"RuntimeHost.Bridge.Error.PrgStartupRequired\": \"Bridge invocation currently requires a PRG startup source.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestArtifactNotFound\": \"Bridge request artifact not found.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestDescriptorMismatch\": \"Bridge request descriptor mismatch.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestMediaTypeMismatch\": \"Bridge request media type mismatch.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestParameterCountMismatch\": \"Bridge request parameter count mismatch.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestParameterNameMismatch\": \"Bridge request parameter name mismatch.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestSchemaVersionMismatch\": \"Bridge request schema version mismatch.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequiredArguments\": \"Bridge invocation requires request/response path, media type, and schema version arguments.\",\n"
        "  \"RuntimeHost.Bridge.Error.SourceArtifactNotFound\": \"Bridge routine source artifact not found.\",\n"
        "  \"RuntimeHost.Bridge.Error.SourceMissingFromPackage\": \"Bridge routine source is missing from the package: {fileName}\",\n"
        "  \"RuntimeHost.Bridge.Error.UnsupportedRoutineExportName\": \"Bridge routine export name is not a supported PRG identifier.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteResponseArtifactFailed\": \"Unable to write bridge response artifact.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteRoutineBootstrapFailed\": \"Unable to write bridge routine bootstrap.\",\n"
        "  \"RuntimeHost.Debug.Error.DispatchXAssetActionFailed\": \"Unable to dispatch xAsset action: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.InvalidBreakpointCommand\": \"Invalid breakpoint command: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.InvalidCommand\": \"Invalid debug command: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.MaterializeXAssetBootstrapFailed\": \"Unable to materialize xAsset bootstrap.\",\n"
        "  \"RuntimeHost.Debug.Error.NoRunnableStartupMethodsFound\": \"No runnable startup methods were found in asset.\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownBreakpoint\": \"Unknown breakpoint: {path}:{line}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownBreakpointForXAssetAction\": \"Unknown breakpoint for xAsset action: {action}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownOrNonBreakpointableXAssetAction\": \"Unknown or non-breakpointable xAsset action: {action}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownXAssetAction\": \"Unknown xAsset action: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.WatchRequiresPausedState\": \"Watch evaluation requires an active paused state.\",\n"
        "  \"RuntimeHost.Debug.Error.XAssetActionBreakpointsRequireBootstrapMode\": \"xAsset action breakpoints require xasset-bootstrap mode.\",\n"
        "  \"RuntimeHost.Prefix.Error\": \"error: \",\n"
        "  \"RuntimeHost.Prefix.Warning\": \"warning: \",\n"
        "  \"Studio.DocumentOpen.Error.SidecarPrimaryMissing\": \"The primary document for sidecar '{path}' was not found.\",\n"
        "  \"RuntimeHost.Launch.Note.CompatibilityLauncher\": \"Startup asset is not a PRG file and could not be materialized for xAsset bootstrap. This launch is falling back to compatibility-launcher mode.\",\n"
        "  \"RuntimeHost.Error.BridgeFederationModeConflict\": \"Bridge invocation mode cannot be combined with federation query mode.\",\n"
        "  \"RuntimeHost.Error.FederationRequiredOptions\": \"{federationBackendOption} and {federationQueryOption} are both required in federation mode.\",\n"
        "  \"RuntimeHost.Error.FederationExecutionRequiresSqlite\": \"Read-only federation execution currently requires the sqlite backend.\",\n"
        "  \"RuntimeHost.Error.FederationExecutionTargetRequired\": \"Read-only SQLite federation execution requires --federation-target to name an existing database file.\",\n"
        "  \"RuntimeHost.Error.FederationExecutionFailed\": \"Read-only SQLite federation execution failed: {errorCode}\",\n"
        "  \"RuntimeHost.Error.AssetEntryMalformed\": \"asset entry is malformed in manifest.\",\n"
        "  \"RuntimeHost.Error.DataAssetMalformed\": \"data_asset entry is malformed in manifest.\",\n"
        "  \"RuntimeHost.Error.DataPayloadMalformed\": \"data_payload entry is malformed in manifest.\",\n"
        "  \"RuntimeHost.Error.DataPolicyMalformed\": \"data_policy is missing or unsupported in manifest.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMalformed\": \"extension_payload entry is malformed in manifest.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMissingFromPackage\": \"Extension payload is missing from the package: {fileName}\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadSha256Mismatch\": \"Extension payload hash mismatch: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagePathPhysicalContainmentFailed\": \"Package path failed physical containment validation: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagedAssetMissing\": \"Packaged asset is missing from the package: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagedAssetDigestMissing\": \"Packaged asset is missing a verified digest: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagedAssetSha256Mismatch\": \"Packaged asset hash mismatch: {fileName}\",\n"
        "  \"RuntimeHost.Error.StartupAssetDigestMissing\": \"Security-enabled startup is missing a verified package digest: {fileName}\",\n"
        "  \"RuntimeHost.Error.ManifestEmptyOrInvalid\": \"Manifest is empty or invalid.\",\n"
        "  \"RuntimeHost.Error.ManifestMissingRuntimeHostSha256\": \"Security-enabled manifest is missing runtime_host_sha256.\",\n"
        "  \"RuntimeHost.Error.ManifestNotFound\": \"Manifest file not found.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionContractAmbiguous\": \"Manifest must contain exactly one of manifest_version or debug_manifest_version.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionMissing\": \"Manifest is missing manifest_version.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionUnsupported\": \"Unsupported manifest_version: {version}. Supported versions: {supportedVersions}.\",\n"
        "  \"RuntimeHost.Error.MaterializeVerifiedStartupSnapshotFailed\": \"Unable to materialize the verified startup snapshot: {fileName}\",\n"
        "  \"RuntimeHost.Prompt.QuitConfirm\": \"Do you want to quit this application? [{yesToken}/{defaultNoToken}]: \",\n"
        "  \"RuntimeHost.Error.RuntimeHostSha256Mismatch\": \"Runtime host hash does not match manifest digest.\",\n"
        "  \"RuntimeHost.Error.SecurityPolicyDenied\": \"Security policy denied {permission} for role '{role}'.\",\n"
        "  \"RuntimeHost.Error.TrueFalseValueRequired\": \"The {option} value must be true or false.\",\n"
        "  \"RuntimeHost.Error.VerifiedSourceUnavailable\": \"Verified package source is unavailable: {fileName}\",\n"
        "  \"RuntimeHost.Error.UnknownArgument\": \"Unknown argument: {argument}\",\n"
        "  \"RuntimeHost.Error.UnknownFederationBackend\": \"Unknown federation backend: {backend}\",\n"
        "  \"RuntimeHost.Error.UnhandledFault\": \"Runtime host fault was contained: {detail}\",\n"
        "  \"Platform.FederationExecution.Error.AiPlannerNotImplemented\": \"Planner is not yet implemented for {planMode} AI policy. Deterministic translation failed: {translationError}\",\n"
        "  \"Platform.QueryTranslator.Error.SelectFromOnly\": \"Only first-pass SELECT...FROM SQL translation is supported.\",\n"
        "  \"Runtime.Prg.Session.Error.NoRunnableStartupMethodsFoundInAsset\": \"No runnable startup methods were found in asset: {path}\",\n"
        "  \"Runtime.Prg.Session.Message.BreakpointHit\": \"Breakpoint hit.\",\n"
        "  \"Runtime.Prg.Session.Message.ExecutionCompleted\": \"Execution completed.\",\n"
        "  \"Runtime.Prg.Session.Message.StepCompleted\": \"Step completed.\",\n"
        "  \"Runtime.Prg.Session.Message.StepOutCompleted\": \"Step-out completed.\",\n"
        "  \"Runtime.Prg.Session.Message.StepOverCompleted\": \"Step-over completed.\",\n"
        "  \"Runtime.Prg.Session.Message.StoppedOnEntry\": \"Stopped on entry.\",\n"
        "  \"Runtime.Prg.Session.Message.WaitingInReadEvents\": \"The runtime is waiting in READ EVENTS.\",\n"
        "  \"Runtime.Prg.Watch.Error.EmptyExpression\": \"Watch expression is empty.\",\n"
        "  \"Runtime.Prg.Watch.Error.Failed\": \"Watch evaluation failed.\",\n"
        "  \"Runtime.Prg.Watch.Error.OutOfMemory\": \"Watch evaluation ran out of memory.\",\n"
        "  \"Runtime.Prg.Watch.Error.RequiresPausedFrame\": \"Watch evaluation requires a paused runtime frame.\",\n"
        "  \"RuntimeHost.Usage.Federation\": \"   or: {commandName} {federationBackendOption} {federationBackendValue} {federationQueryOption} {federationQueryValue} [{federationTargetOption} {federationTargetValue}]\",\n"
        "  \"RuntimeHost.Usage.FederationExecution\": \"       [{federationReadOnlyExecuteOption} {booleanValue}]\",\n"
        "  \"RuntimeHost.Usage.FederationPlanning\": \"       [{planningEnableOption} {booleanValue}] [{planningRequireOption} {booleanValue}] [{planningAuditOption} {booleanValue}]\",\n"
        "  \"RuntimeHost.Usage.Manifest\": \"Usage: {commandName} {manifestOption} {manifestValue} [{debugOption}] [{debugStopOnEntryOption}] [{breakpointOption} {breakpointValue}] [{debugCommandOption} {debugCommandValue}]\"\n"
        "}\n");
    write_text(
        spanish_root / "strings.json",
        "{\n"
        "  \"RuntimeHost.Bridge.Error.CreateResponseDirectoryFailed\": \"No se pudo crear el directorio de respuesta bridge.\",\n"
        "  \"RuntimeHost.Bridge.Error.PrgStartupRequired\": \"La invocacion bridge actualmente requiere un origen de inicio PRG.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestArtifactNotFound\": \"No se encontro el artefacto de solicitud bridge.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestDescriptorMismatch\": \"El descriptor de la solicitud bridge no coincide.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestMediaTypeMismatch\": \"El tipo de medio de la solicitud bridge no coincide.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestParameterCountMismatch\": \"La cantidad de parametros de la solicitud bridge no coincide.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestParameterNameMismatch\": \"El nombre de parametro de la solicitud bridge no coincide.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestSchemaVersionMismatch\": \"La version de esquema de la solicitud bridge no coincide.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequiredArguments\": \"La invocacion bridge requiere argumentos de ruta de solicitud/respuesta, tipo de medio y version de esquema.\",\n"
        "  \"RuntimeHost.Bridge.Error.SourceArtifactNotFound\": \"No se encontro el artefacto fuente de la rutina bridge.\",\n"
        "  \"RuntimeHost.Bridge.Error.SourceMissingFromPackage\": \"Falta la fuente de la rutina bridge en el paquete: {fileName}\",\n"
        "  \"RuntimeHost.Bridge.Error.UnsupportedRoutineExportName\": \"El nombre de exportacion de la rutina bridge no es un identificador PRG compatible.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteResponseArtifactFailed\": \"No se pudo escribir el artefacto de respuesta bridge.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteRoutineBootstrapFailed\": \"No se pudo escribir el bootstrap de la rutina bridge.\",\n"
        "  \"RuntimeHost.Debug.Error.DispatchXAssetActionFailed\": \"No se pudo despachar la accion xAsset: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.InvalidBreakpointCommand\": \"Comando de breakpoint invalido: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.InvalidCommand\": \"Comando de depuracion invalido: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.MaterializeXAssetBootstrapFailed\": \"No se pudo materializar el bootstrap xAsset.\",\n"
        "  \"RuntimeHost.Debug.Error.NoRunnableStartupMethodsFound\": \"No se encontraron metodos de inicio ejecutables en el asset.\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownBreakpoint\": \"Breakpoint desconocido: {path}:{line}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownBreakpointForXAssetAction\": \"Breakpoint desconocido para la accion xAsset: {action}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownOrNonBreakpointableXAssetAction\": \"Accion xAsset desconocida o no breakpointable: {action}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownXAssetAction\": \"Accion xAsset desconocida: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.WatchRequiresPausedState\": \"La evaluacion de watch requiere un estado pausado activo.\",\n"
        "  \"RuntimeHost.Debug.Error.XAssetActionBreakpointsRequireBootstrapMode\": \"Los breakpoints de accion xAsset requieren el modo xasset-bootstrap.\",\n"
        "  \"RuntimeHost.Prefix.Error\": \"error: \",\n"
        "  \"RuntimeHost.Prefix.Warning\": \"advertencia: \",\n"
        "  \"Studio.DocumentOpen.Error.SidecarPrimaryMissing\": \"No se encontro el documento principal para el archivo asociado '{path}'.\",\n"
        "  \"RuntimeHost.Launch.Note.CompatibilityLauncher\": \"El asset de inicio no es un archivo PRG y no pudo materializarse para xAsset bootstrap. Este inicio esta recurriendo al modo compatibility-launcher.\",\n"
        "  \"RuntimeHost.Error.BridgeFederationModeConflict\": \"El modo de invocacion bridge no puede combinarse con el modo de consulta de federacion.\",\n"
        "  \"RuntimeHost.Error.AssetEntryMalformed\": \"La entrada asset del manifiesto esta mal formada.\",\n"
        "  \"RuntimeHost.Error.DataAssetMalformed\": \"La entrada data_asset del manifiesto esta mal formada.\",\n"
        "  \"RuntimeHost.Error.DataPayloadMalformed\": \"La entrada data_payload del manifiesto esta mal formada.\",\n"
        "  \"RuntimeHost.Error.DataPolicyMalformed\": \"Falta data_policy en el manifiesto o su valor no es compatible.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMalformed\": \"La entrada extension_payload del manifiesto esta mal formada.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMissingFromPackage\": \"Falta el payload de extension en el paquete: {fileName}\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadSha256Mismatch\": \"El hash del payload de extension no coincide: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagePathPhysicalContainmentFailed\": \"La ruta del paquete no supero la validacion de contencion fisica: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagedAssetMissing\": \"Falta el asset empaquetado en el paquete: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagedAssetDigestMissing\": \"Al asset empaquetado le falta un digest verificado: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagedAssetSha256Mismatch\": \"El hash del asset empaquetado no coincide: {fileName}\",\n"
        "  \"RuntimeHost.Error.StartupAssetDigestMissing\": \"Al inicio con seguridad habilitada le falta un digest de paquete verificado: {fileName}\",\n"
        "  \"RuntimeHost.Error.FederationRequiredOptions\": \"{federationBackendOption} y {federationQueryOption} son obligatorios en el modo de federacion.\",\n"
        "  \"RuntimeHost.Error.FederationExecutionRequiresSqlite\": \"La ejecucion de federacion de solo lectura actualmente requiere el backend sqlite.\",\n"
        "  \"RuntimeHost.Error.FederationExecutionTargetRequired\": \"La ejecucion de federacion SQLite de solo lectura requiere que --federation-target nombre un archivo de base de datos existente.\",\n"
        "  \"RuntimeHost.Error.FederationExecutionFailed\": \"Fallo la ejecucion de federacion SQLite de solo lectura: {errorCode}\",\n"
        "  \"RuntimeHost.Error.ManifestEmptyOrInvalid\": \"El manifiesto esta vacio o no es valido.\",\n"
        "  \"RuntimeHost.Error.ManifestMissingRuntimeHostSha256\": \"Al manifiesto con seguridad habilitada le falta runtime_host_sha256.\",\n"
        "  \"RuntimeHost.Error.ManifestNotFound\": \"No se encontro el archivo de manifiesto.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionContractAmbiguous\": \"El manifiesto debe contener exactamente uno de manifest_version o debug_manifest_version.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionMissing\": \"Al manifiesto le falta manifest_version.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionUnsupported\": \"manifest_version no es compatible: {version}. Las versiones compatibles son: {supportedVersions}.\",\n"
        "  \"RuntimeHost.Error.MaterializeVerifiedStartupSnapshotFailed\": \"No se pudo materializar la instantanea de inicio verificada: {fileName}\",\n"
        "  \"RuntimeHost.Prompt.QuitConfirm\": \"Desea salir de esta aplicacion? [{yesToken}/{defaultNoToken}]: \",\n"
        "  \"RuntimeHost.Error.RuntimeHostSha256Mismatch\": \"El hash del runtime host no coincide con el digest del manifiesto.\",\n"
        "  \"RuntimeHost.Error.SecurityPolicyDenied\": \"La politica de seguridad denego {permission} para el rol '{role}'.\",\n"
        "  \"RuntimeHost.Error.TrueFalseValueRequired\": \"El valor de {option} debe ser true o false.\",\n"
        "  \"RuntimeHost.Error.VerifiedSourceUnavailable\": \"La fuente verificada del paquete no esta disponible: {fileName}\",\n"
        "  \"RuntimeHost.Error.UnknownArgument\": \"Argumento desconocido: {argument}\",\n"
        "  \"RuntimeHost.Error.UnknownFederationBackend\": \"Backend de federacion desconocido: {backend}\",\n"
        "  \"RuntimeHost.Error.UnhandledFault\": \"Se contuvo una falla del host de runtime: {detail}\",\n"
        "  \"Platform.FederationExecution.Error.AiPlannerNotImplemented\": \"El planner aun no esta implementado para la politica de IA {planMode}. La traduccion deterministica fallo: {translationError}\",\n"
        "  \"Platform.QueryTranslator.Error.SelectFromOnly\": \"Solo se admite la traduccion SQL deterministica de primera pasada de SELECT...FROM.\",\n"
        "  \"Runtime.Prg.Session.Error.NoRunnableStartupMethodsFoundInAsset\": \"No se encontraron metodos de inicio ejecutables en el asset: {path}\",\n"
        "  \"Runtime.Prg.Session.Message.BreakpointHit\": \"Se alcanzo un breakpoint.\",\n"
        "  \"Runtime.Prg.Session.Message.ExecutionCompleted\": \"La ejecucion se completo.\",\n"
        "  \"Runtime.Prg.Session.Message.StepCompleted\": \"El paso se completo.\",\n"
        "  \"Runtime.Prg.Session.Message.StepOutCompleted\": \"El paso de salida se completo.\",\n"
        "  \"Runtime.Prg.Session.Message.StepOverCompleted\": \"El paso sobre se completo.\",\n"
        "  \"Runtime.Prg.Session.Message.StoppedOnEntry\": \"Se detuvo en la entrada.\",\n"
        "  \"Runtime.Prg.Session.Message.WaitingInReadEvents\": \"El runtime esta esperando en READ EVENTS.\",\n"
        "  \"Runtime.Prg.Watch.Error.EmptyExpression\": \"La expresion de watch esta vacia.\",\n"
        "  \"Runtime.Prg.Watch.Error.Failed\": \"La evaluacion de watch fallo.\",\n"
        "  \"Runtime.Prg.Watch.Error.OutOfMemory\": \"La evaluacion de watch se quedo sin memoria.\",\n"
        "  \"Runtime.Prg.Watch.Error.RequiresPausedFrame\": \"La evaluacion de watch requiere un frame de runtime pausado.\",\n"
        "  \"RuntimeHost.Usage.Federation\": \"   o: {commandName} {federationBackendOption} {federationBackendValue} {federationQueryOption} {federationQueryValue} [{federationTargetOption} {federationTargetValue}]\",\n"
        "  \"RuntimeHost.Usage.FederationExecution\": \"       [{federationReadOnlyExecuteOption} {booleanValue}]\",\n"
        "  \"RuntimeHost.Usage.FederationPlanning\": \"       [{planningEnableOption} {booleanValue}] [{planningRequireOption} {booleanValue}] [{planningAuditOption} {booleanValue}]\",\n"
        "  \"RuntimeHost.Usage.Manifest\": \"Uso: {commandName} {manifestOption} {manifestValue} [{debugOption}] [{debugStopOnEntryOption}] [{breakpointOption} {breakpointValue}] [{debugCommandOption} {debugCommandValue}]\"\n"
        "}\n");
    write_text(
        portuguese_root / "strings.json",
        "{\n"
        "  \"RuntimeHost.Launch.Note.CompatibilityLauncher\": \"O asset de inicializacao nao e um arquivo PRG e nao pode ser materializado para xAsset bootstrap. Esta inicializacao esta recorrendo ao modo compatibility-launcher.\",\n"
        "  \"RuntimeHost.Bridge.Error.CreateResponseDirectoryFailed\": \"Nao foi possivel criar o diretorio de resposta bridge.\",\n"
        "  \"RuntimeHost.Error.BridgeFederationModeConflict\": \"O modo de invocacao bridge nao pode ser combinado com o modo de consulta de federacao.\",\n"
        "  \"RuntimeHost.Error.AssetEntryMalformed\": \"A entrada asset do manifesto esta malformada.\",\n"
        "  \"RuntimeHost.Error.DataAssetMalformed\": \"A entrada data_asset do manifesto esta malformada.\",\n"
        "  \"RuntimeHost.Error.DataPayloadMalformed\": \"A entrada data_payload do manifesto esta malformada.\",\n"
        "  \"RuntimeHost.Error.DataPolicyMalformed\": \"data_policy esta ausente ou nao e compativel no manifesto.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMalformed\": \"A entrada extension_payload do manifesto esta malformada.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMissingFromPackage\": \"O payload de extensao esta ausente do pacote: {fileName}\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadSha256Mismatch\": \"O hash do payload de extensao nao corresponde: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagePathPhysicalContainmentFailed\": \"O caminho do pacote falhou na validacao de contencao fisica: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagedAssetMissing\": \"O asset empacotado esta ausente do pacote: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagedAssetDigestMissing\": \"O asset empacotado nao tem um digest verificado: {fileName}\",\n"
        "  \"RuntimeHost.Error.PackagedAssetSha256Mismatch\": \"O hash do asset empacotado nao corresponde: {fileName}\",\n"
        "  \"RuntimeHost.Error.StartupAssetDigestMissing\": \"A inicializacao com seguranca habilitada nao tem um digest de pacote verificado: {fileName}\",\n"
        "  \"RuntimeHost.Bridge.Error.PrgStartupRequired\": \"A invocacao bridge atualmente exige uma origem de inicializacao PRG.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestArtifactNotFound\": \"Artefato de solicitacao bridge nao encontrado.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestDescriptorMismatch\": \"O descritor da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestMediaTypeMismatch\": \"O tipo de midia da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestParameterCountMismatch\": \"A contagem de parametros da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestParameterNameMismatch\": \"O nome do parametro da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestSchemaVersionMismatch\": \"A versao de esquema da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequiredArguments\": \"A invocacao bridge exige argumentos de caminho de solicitacao/resposta, tipo de midia e versao de esquema.\",\n"
        "  \"RuntimeHost.Bridge.Error.SourceArtifactNotFound\": \"Artefato fonte da rotina bridge nao encontrado.\",\n"
        "  \"RuntimeHost.Bridge.Error.SourceMissingFromPackage\": \"A origem da rotina bridge esta ausente do pacote: {fileName}\",\n"
        "  \"RuntimeHost.Bridge.Error.UnsupportedRoutineExportName\": \"O nome de exportacao da rotina bridge nao e um identificador PRG suportado.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteResponseArtifactFailed\": \"Nao foi possivel gravar o artefato de resposta bridge.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteRoutineBootstrapFailed\": \"Nao foi possivel gravar o bootstrap da rotina bridge.\",\n"
        "  \"RuntimeHost.Error.FederationRequiredOptions\": \"{federationBackendOption} e {federationQueryOption} sao obrigatorios no modo de federacao.\",\n"
        "  \"RuntimeHost.Error.FederationExecutionRequiresSqlite\": \"A execucao de federacao somente leitura atualmente requer o backend sqlite.\",\n"
        "  \"RuntimeHost.Error.FederationExecutionTargetRequired\": \"A execucao de federacao SQLite somente leitura requer que --federation-target nomeie um arquivo de banco de dados existente.\",\n"
        "  \"RuntimeHost.Error.FederationExecutionFailed\": \"A execucao de federacao SQLite somente leitura falhou: {errorCode}\",\n"
        "  \"RuntimeHost.Error.ManifestEmptyOrInvalid\": \"O manifesto esta vazio ou e invalido.\",\n"
        "  \"RuntimeHost.Error.ManifestMissingRuntimeHostSha256\": \"Falta runtime_host_sha256 no manifesto com seguranca habilitada.\",\n"
        "  \"RuntimeHost.Error.ManifestNotFound\": \"Arquivo de manifesto nao encontrado.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionContractAmbiguous\": \"O manifesto deve conter exatamente um de manifest_version ou debug_manifest_version.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionMissing\": \"Falta manifest_version no manifesto.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionUnsupported\": \"manifest_version nao e compativel: {version}. As versoes compativeis sao: {supportedVersions}.\",\n"
        "  \"RuntimeHost.Error.MaterializeVerifiedStartupSnapshotFailed\": \"Nao foi possivel materializar o snapshot de inicializacao verificado: {fileName}\",\n"
        "  \"RuntimeHost.Prompt.QuitConfirm\": \"Deseja sair deste aplicativo? [{yesToken}/{defaultNoToken}]: \",\n"
        "  \"RuntimeHost.Error.RuntimeHostSha256Mismatch\": \"O hash do runtime host nao corresponde ao digest do manifesto.\",\n"
        "  \"RuntimeHost.Error.SecurityPolicyDenied\": \"A politica de seguranca negou {permission} para a funcao '{role}'.\",\n"
        "  \"RuntimeHost.Error.TrueFalseValueRequired\": \"O valor de {option} deve ser true ou false.\",\n"
        "  \"RuntimeHost.Error.VerifiedSourceUnavailable\": \"A origem verificada do pacote nao esta disponivel: {fileName}\",\n"
        "  \"RuntimeHost.Error.UnknownArgument\": \"Argumento desconhecido: {argument}\",\n"
        "  \"RuntimeHost.Error.UnknownFederationBackend\": \"Backend de federacao desconhecido: {backend}\",\n"
        "  \"RuntimeHost.Error.UnhandledFault\": \"Uma falha do host de runtime foi contida: {detail}\",\n"
        "  \"Platform.FederationExecution.Error.AiPlannerNotImplemented\": \"O planner ainda nao esta implementado para a politica de IA {planMode}. A traducao deterministica falhou: {translationError}\",\n"
        "  \"Platform.QueryTranslator.Error.SelectFromOnly\": \"Somente a traducao SQL deterministica de primeira passagem de SELECT...FROM e suportada.\",\n"
        "  \"Runtime.Prg.Session.Error.NoRunnableStartupMethodsFoundInAsset\": \"Nenhum metodo de inicializacao executavel foi encontrado no asset: {path}\",\n"
        "  \"Runtime.Prg.Session.Message.BreakpointHit\": \"Um breakpoint foi atingido.\",\n"
        "  \"Runtime.Prg.Session.Message.ExecutionCompleted\": \"A execucao foi concluida.\",\n"
        "  \"Runtime.Prg.Session.Message.StepCompleted\": \"O passo foi concluido.\",\n"
        "  \"Runtime.Prg.Session.Message.StepOutCompleted\": \"O passo de saida foi concluido.\",\n"
        "  \"Runtime.Prg.Session.Message.StepOverCompleted\": \"O passo sobre foi concluido.\",\n"
        "  \"Runtime.Prg.Session.Message.StoppedOnEntry\": \"Parado na entrada.\",\n"
        "  \"Runtime.Prg.Session.Message.WaitingInReadEvents\": \"O runtime esta aguardando em READ EVENTS.\",\n"
        "  \"Runtime.Prg.Watch.Error.EmptyExpression\": \"A expressao de watch esta vazia.\",\n"
        "  \"Runtime.Prg.Watch.Error.Failed\": \"A avaliacao de watch falhou.\",\n"
        "  \"Runtime.Prg.Watch.Error.OutOfMemory\": \"A avaliacao de watch ficou sem memoria.\",\n"
        "  \"Runtime.Prg.Watch.Error.RequiresPausedFrame\": \"A avaliacao de watch exige um frame de runtime pausado.\",\n"
        "  \"RuntimeHost.Usage.Federation\": \"   ou: {commandName} {federationBackendOption} {federationBackendValue} {federationQueryOption} {federationQueryValue} [{federationTargetOption} {federationTargetValue}]\",\n"
        "  \"RuntimeHost.Usage.FederationExecution\": \"       [{federationReadOnlyExecuteOption} {booleanValue}]\",\n"
        "  \"RuntimeHost.Usage.FederationPlanning\": \"       [{planningEnableOption} {booleanValue}] [{planningRequireOption} {booleanValue}] [{planningAuditOption} {booleanValue}]\",\n"
        "  \"RuntimeHost.Usage.Manifest\": \"Uso: {commandName} {manifestOption} {manifestValue} [{debugOption}] [{debugStopOnEntryOption}] [{breakpointOption} {breakpointValue}] [{debugCommandOption} {debugCommandValue}]\",\n"
        "  \"RuntimeHost.Debug.Error.DispatchXAssetActionFailed\": \"Nao foi possivel despachar a acao xAsset: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.InvalidBreakpointCommand\": \"Comando de breakpoint invalido: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.InvalidCommand\": \"Comando de depuracao invalido: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.MaterializeXAssetBootstrapFailed\": \"Nao foi possivel materializar o bootstrap xAsset.\",\n"
        "  \"RuntimeHost.Debug.Error.NoRunnableStartupMethodsFound\": \"Nenhum metodo de inicializacao executavel foi encontrado no asset.\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownBreakpoint\": \"Breakpoint desconhecido: {path}:{line}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownBreakpointForXAssetAction\": \"Breakpoint desconhecido para a acao xAsset: {action}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownOrNonBreakpointableXAssetAction\": \"Acao xAsset desconhecida ou sem suporte a breakpoint: {action}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownXAssetAction\": \"Acao xAsset desconhecida: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.WatchRequiresPausedState\": \"A avaliacao de watch requer um estado pausado ativo.\",\n"
        "  \"RuntimeHost.Debug.Error.XAssetActionBreakpointsRequireBootstrapMode\": \"Breakpoints de acao xAsset exigem o modo xasset-bootstrap.\",\n"
        "  \"RuntimeHost.Prefix.Error\": \"erro: \",\n"
        "  \"RuntimeHost.Prefix.Warning\": \"aviso: \",\n"
        "  \"Studio.DocumentOpen.Error.SidecarPrimaryMissing\": \"O documento principal do arquivo complementar '{path}' nao foi encontrado.\"\n"
        "}\n");
    write_text(
        pseudo_root / "strings.json",
        "{\n"
        "  \"Studio.DocumentOpen.Error.SidecarPrimaryMissing\": \"[!! Ţhë prïmåry døçümëñţ før sïdëçår '{path}' wås ñøţ føüñd. !!]\",\n"
        "  \"Runtime.Prg.Session.Message.StoppedOnEntry\": \"[!! Sţøppëd øñ ëñţry. !!]\"\n"
        "}\n");
}

[[maybe_unused]] std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

[[maybe_unused]] std::string json_escape_string(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const unsigned char ch : value) {
        switch (ch) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (ch < 0x20U) {
                    static constexpr char hex[] = "0123456789abcdef";
                    escaped += "\\u00";
                    escaped.push_back(hex[(ch >> 4U) & 0x0FU]);
                    escaped.push_back(hex[ch & 0x0FU]);
                } else {
                    escaped.push_back(static_cast<char>(ch));
                }
                break;
        }
    }
    return escaped;
}

[[maybe_unused]] void normalize_captured_newlines(std::string& text) {
    std::size_t offset = 0U;
    while ((offset = text.find("\r\n", offset)) != std::string::npos) {
        text.erase(offset, 1U);
    }
}

[[maybe_unused]] std::string output_line_value(const std::string& output, const std::string& prefix) {
    const std::size_t offset = output.find(prefix);
    if (offset == std::string::npos) {
        return {};
    }
    const std::size_t value_start = offset + prefix.size();
    const std::size_t value_end = output.find('\n', value_start);
    return output.substr(
        value_start,
        value_end == std::string::npos ? std::string::npos : value_end - value_start);
}

[[maybe_unused]] std::string breakpoint_entry_path(const std::string& breakpoint_entry) {
    const std::size_t separator = breakpoint_entry.rfind(':');
    if (separator == std::string::npos) {
        return {};
    }
    return breakpoint_entry.substr(0, separator);
}

[[maybe_unused]] std::string quote_command_argument(const std::string& value) {
    std::string quoted = "\"";
    quoted.reserve(value.size() + 2U);
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('"');
    return quoted;
}

[[maybe_unused]] bool create_directory_indirection(
    const std::filesystem::path& target,
    const std::filesystem::path& link) {
#if defined(_WIN32)
    const std::string command =
        "cmd.exe /d /c mklink /J " + quote_command_argument(link.string()) + " " +
        quote_command_argument(target.string()) + " > NUL 2>&1";
    return copperfin::test_support::run_shell_command(command) == 0;
#else
    std::error_code error;
    std::filesystem::create_directory_symlink(target, link, error);
    return !error;
#endif
}

[[maybe_unused]] void remove_directory_indirection(const std::filesystem::path& link) {
#if defined(_WIN32)
    const std::string command =
        "cmd.exe /d /c rmdir " + quote_command_argument(link.string()) + " > NUL 2>&1";
    (void)copperfin::test_support::run_shell_command(command);
#else
    std::error_code ignored;
    std::filesystem::remove(link, ignored);
#endif
}

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
    std::string stderr_text;
};

[[maybe_unused]] ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory,
    const std::optional<std::string>& stdin_text = std::nullopt) {
    namespace fs = std::filesystem;

    const fs::path stdout_path = working_directory / "runtime_host_stdout.log";
    const fs::path stderr_path = working_directory / "runtime_host_stderr.log";
    const fs::path stdin_path = working_directory / "runtime_host_stdin.log";

    std::string command = quote_command_argument(executable_path);
    for (const auto& argument : arguments) {
        command += " ";
        command += quote_command_argument(argument);
    }
    if (stdin_text.has_value()) {
        write_text(stdin_path, *stdin_text);
        command += " < ";
        command += quote_command_argument(stdin_path.string());
    }
    command += " > ";
    command += quote_command_argument(stdout_path.string());
    command += " 2> ";
    command += quote_command_argument(stderr_path.string());

    const fs::path original_directory = fs::current_path();
    fs::current_path(working_directory);
    const int raw_exit_code = copperfin::test_support::run_shell_command(command);
    fs::current_path(original_directory);

    ProcessResult result;
    if (fs::exists(stdout_path)) {
        result.stdout_text = read_text(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_text(stderr_path);
    }
    normalize_captured_newlines(result.stdout_text);
    normalize_captured_newlines(result.stderr_text);

#if defined(_WIN32)
    result.exit_code = raw_exit_code;
#else
    if (raw_exit_code != -1 && WIFEXITED(raw_exit_code)) {
        result.exit_code = WEXITSTATUS(raw_exit_code);
    } else {
        result.exit_code = raw_exit_code;
    }
#endif
    return result;
}

[[maybe_unused]] std::optional<copperfin::runtime::XAssetActionBinding> find_action(
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& action_id) {
    for (const auto& action : model.actions) {
        if (action.action_id == action_id) {
            return action;
        }
    }
    return std::nullopt;
}

[[maybe_unused]] std::size_t find_breakpoint_line_for_routine_statement(
    const std::string& source,
    const std::string& routine_name,
    const std::string& statement_text) {
    std::size_t current_line = 0;
    bool in_target_routine = false;
    std::string line;
    std::size_t line_start = 0;

    while (line_start <= source.size()) {
        const std::size_t line_end = source.find('\n', line_start);
        line = source.substr(
            line_start,
            line_end == std::string::npos ? std::string::npos : line_end - line_start);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        ++current_line;
        if (line == "PROCEDURE " + routine_name) {
            in_target_routine = true;
        } else if (in_target_routine && line == statement_text) {
            return current_line;
        } else if (in_target_routine && line == "ENDPROC") {
            break;
        }

        if (line_end == std::string::npos) {
            break;
        }
        line_start = line_end + 1U;
    }

    return 0U;
}

[[maybe_unused]] std::size_t find_first_breakpoint_line_for_routine(
    const std::string& source,
    const std::string& routine_name) {
    std::size_t current_line = 0;
    std::string line;
    std::size_t line_start = 0;
    bool in_target_routine = false;

    while (line_start <= source.size()) {
        const std::size_t line_end = source.find('\n', line_start);
        line = source.substr(
            line_start,
            line_end == std::string::npos ? std::string::npos : line_end - line_start);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        ++current_line;
        if (line == "PROCEDURE " + routine_name) {
            in_target_routine = true;
        } else if (in_target_routine) {
            if (line == "ENDPROC") {
                break;
            }
            if (!line.empty() && line[0] != '*') {
                return current_line;
            }
        }

        if (line_end == std::string::npos) {
            break;
        }
        line_start = line_end + 1U;
    }

    return 0U;
}

[[maybe_unused]] void write_synthetic_form_asset(const std::filesystem::path& table_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "PLATFORM", .type = 'C', .length = 16U},
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "WINDOWS",
            "Dataenvironment",
            "",
            "dataenvironment",
            "PROCEDURE BeforeOpenTables\r\nSET DELETED ON\r\nENDPROC\r\n"
            "PROCEDURE OpenTables\r\nx = 1\r\nENDPROC\r\n"
            "PROCEDURE CloseTables\r\nCLEAR EVENTS\r\nENDPROC\r\n"
        },
        {
            "WINDOWS",
            "frmDemo",
            "",
            "form",
            "PROCEDURE Load\r\nx = 2\r\nENDPROC\r\n"
            "PROCEDURE Init\r\nx = 3\r\nENDPROC\r\n"
            "PROCEDURE Activate\r\nx = 4\r\nENDPROC\r\n"
            "PROCEDURE Destroy\r\nx = 5\r\nENDPROC\r\n"
        },
        {
            "WINDOWS",
            "pgfMain",
            "frmDemo",
            "pageframe",
            "PROCEDURE Page2.Activate\r\nTHISFORM.Refresh\r\nENDPROC\r\n"
        }
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "synthetic SCX/SCT debugger fixture should be created");
}

[[maybe_unused]] void write_synthetic_menu_asset(const std::filesystem::path& table_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 3U},
        {.name = "NAME", .type = 'M', .length = 4U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"1", "MainMenu"}});
    expect(create_result.ok, "synthetic MNX/MNT runtime fixture should be created");
}

[[maybe_unused]] void write_synthetic_writable_data_asset(const std::filesystem::path& table_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "NOTE", .type = 'M', .length = 4U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"Alice", "seed-value"}});
    expect(create_result.ok, "synthetic writable DBF/FPT runtime fixture should be created");
}

[[maybe_unused]] void write_synthetic_report_asset(const std::filesystem::path& table_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "PLATFORM", .type = 'C', .length = 16U},
        {.name = "OBJTYPE", .type = 'N', .length = 3U},
        {.name = "OBJCODE", .type = 'N', .length = 3U},
        {.name = "EXPR", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"WINDOWS", "1", "0", "ENVIRONMENT = 1"},
        {"WINDOWS", "9", "4", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        records);
    expect(create_result.ok, "synthetic FRX/FRT snapshot fixture should be created");
}


}  // namespace

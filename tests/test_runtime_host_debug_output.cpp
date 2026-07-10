// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/security/audit_stream.h"
#include "copperfin/security/sha256.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace {

using copperfin::test_support::ScopedEnvironmentValue;

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void write_text(const std::filesystem::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
}

void write_runtime_host_usage_catalogs(const std::filesystem::path& locale_root) {
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
        "  \"RuntimeHost.Bridge.Error.UnsupportedRoutineExportName\": \"Bridge routine export name is not a supported PRG identifier.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteResponseArtifactFailed\": \"Unable to write bridge response artifact.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteRoutineBootstrapFailed\": \"Unable to write bridge routine bootstrap.\",\n"
        "  \"RuntimeHost.Debug.Error.DispatchXAssetActionFailed\": \"Unable to dispatch xAsset action: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.InvalidBreakpointCommand\": \"Invalid breakpoint command: {command}\",\n"
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
        "  \"RuntimeHost.Launch.Note.CompatibilityLauncher\": \"Startup asset is not a PRG file and could not be materialized for xAsset bootstrap. This launch is falling back to compatibility-launcher mode.\",\n"
        "  \"RuntimeHost.Error.BridgeFederationModeConflict\": \"Bridge invocation mode cannot be combined with federation query mode.\",\n"
        "  \"RuntimeHost.Error.FederationRequiredOptions\": \"{federationBackendOption} and {federationQueryOption} are both required in federation mode.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMalformed\": \"extension_payload entry is malformed in manifest.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMissingFromPackage\": \"Extension payload is missing from the package: {fileName}\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadSha256Mismatch\": \"Extension payload hash mismatch: {fileName}\",\n"
        "  \"RuntimeHost.Error.ManifestEmptyOrInvalid\": \"Manifest is empty or invalid.\",\n"
        "  \"RuntimeHost.Error.ManifestMissingRuntimeHostSha256\": \"Security-enabled manifest is missing runtime_host_sha256.\",\n"
        "  \"RuntimeHost.Error.ManifestNotFound\": \"Manifest file not found.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionMissing\": \"Manifest is missing manifest_version.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionUnsupported\": \"Unsupported manifest_version: {version}. Supported versions: {supportedVersions}.\",\n"
        "  \"RuntimeHost.Prompt.QuitConfirm\": \"Do you want to quit this application? [{yesToken}/{defaultNoToken}]: \",\n"
        "  \"RuntimeHost.Error.RuntimeHostSha256Mismatch\": \"Runtime host hash does not match manifest digest.\",\n"
        "  \"RuntimeHost.Error.SecurityPolicyDenied\": \"Security policy denied {permission} for role '{role}'.\",\n"
        "  \"RuntimeHost.Error.TrueFalseValueRequired\": \"The {option} value must be true or false.\",\n"
        "  \"RuntimeHost.Error.UnknownArgument\": \"Unknown argument: {argument}\",\n"
        "  \"RuntimeHost.Error.UnknownFederationBackend\": \"Unknown federation backend: {backend}\",\n"
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
        "  \"RuntimeHost.Usage.FederationPlanning\": \"       [{planningEnableOption} {booleanValue}] [{planningRequireOption} {booleanValue}] [{planningAuditOption} {booleanValue}]\",\n"
        "  \"RuntimeHost.Usage.Manifest\": \"Usage: {commandName} {manifestOption} {manifestValue} [{debugOption}] [{breakpointOption} {breakpointValue}] [{debugCommandOption} {debugCommandValue}]\"\n"
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
        "  \"RuntimeHost.Bridge.Error.UnsupportedRoutineExportName\": \"El nombre de exportacion de la rutina bridge no es un identificador PRG compatible.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteResponseArtifactFailed\": \"No se pudo escribir el artefacto de respuesta bridge.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteRoutineBootstrapFailed\": \"No se pudo escribir el bootstrap de la rutina bridge.\",\n"
        "  \"RuntimeHost.Debug.Error.DispatchXAssetActionFailed\": \"No se pudo despachar la accion xAsset: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.InvalidBreakpointCommand\": \"Comando de breakpoint invalido: {command}\",\n"
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
        "  \"RuntimeHost.Launch.Note.CompatibilityLauncher\": \"El asset de inicio no es un archivo PRG y no pudo materializarse para xAsset bootstrap. Este inicio esta recurriendo al modo compatibility-launcher.\",\n"
        "  \"RuntimeHost.Error.BridgeFederationModeConflict\": \"El modo de invocacion bridge no puede combinarse con el modo de consulta de federacion.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMalformed\": \"La entrada extension_payload del manifiesto esta mal formada.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMissingFromPackage\": \"Falta el payload de extension en el paquete: {fileName}\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadSha256Mismatch\": \"El hash del payload de extension no coincide: {fileName}\",\n"
        "  \"RuntimeHost.Error.FederationRequiredOptions\": \"{federationBackendOption} y {federationQueryOption} son obligatorios en el modo de federacion.\",\n"
        "  \"RuntimeHost.Error.ManifestEmptyOrInvalid\": \"El manifiesto esta vacio o no es valido.\",\n"
        "  \"RuntimeHost.Error.ManifestMissingRuntimeHostSha256\": \"Al manifiesto con seguridad habilitada le falta runtime_host_sha256.\",\n"
        "  \"RuntimeHost.Error.ManifestNotFound\": \"No se encontro el archivo de manifiesto.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionMissing\": \"Al manifiesto le falta manifest_version.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionUnsupported\": \"manifest_version no es compatible: {version}. Las versiones compatibles son: {supportedVersions}.\",\n"
        "  \"RuntimeHost.Prompt.QuitConfirm\": \"Desea salir de esta aplicacion? [{yesToken}/{defaultNoToken}]: \",\n"
        "  \"RuntimeHost.Error.RuntimeHostSha256Mismatch\": \"El hash del runtime host no coincide con el digest del manifiesto.\",\n"
        "  \"RuntimeHost.Error.SecurityPolicyDenied\": \"La politica de seguridad denego {permission} para el rol '{role}'.\",\n"
        "  \"RuntimeHost.Error.TrueFalseValueRequired\": \"El valor de {option} debe ser true o false.\",\n"
        "  \"RuntimeHost.Error.UnknownArgument\": \"Argumento desconocido: {argument}\",\n"
        "  \"RuntimeHost.Error.UnknownFederationBackend\": \"Backend de federacion desconocido: {backend}\",\n"
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
        "  \"RuntimeHost.Usage.FederationPlanning\": \"       [{planningEnableOption} {booleanValue}] [{planningRequireOption} {booleanValue}] [{planningAuditOption} {booleanValue}]\",\n"
        "  \"RuntimeHost.Usage.Manifest\": \"Uso: {commandName} {manifestOption} {manifestValue} [{debugOption}] [{breakpointOption} {breakpointValue}] [{debugCommandOption} {debugCommandValue}]\"\n"
        "}\n");
    write_text(
        portuguese_root / "strings.json",
        "{\n"
        "  \"RuntimeHost.Launch.Note.CompatibilityLauncher\": \"O asset de inicializacao nao e um arquivo PRG e nao pode ser materializado para xAsset bootstrap. Esta inicializacao esta recorrendo ao modo compatibility-launcher.\",\n"
        "  \"RuntimeHost.Bridge.Error.CreateResponseDirectoryFailed\": \"Nao foi possivel criar o diretorio de resposta bridge.\",\n"
        "  \"RuntimeHost.Error.BridgeFederationModeConflict\": \"O modo de invocacao bridge nao pode ser combinado com o modo de consulta de federacao.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMalformed\": \"A entrada extension_payload do manifesto esta malformada.\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadMissingFromPackage\": \"O payload de extensao esta ausente do pacote: {fileName}\",\n"
        "  \"RuntimeHost.Error.ExtensionPayloadSha256Mismatch\": \"O hash do payload de extensao nao corresponde: {fileName}\",\n"
        "  \"RuntimeHost.Bridge.Error.PrgStartupRequired\": \"A invocacao bridge atualmente exige uma origem de inicializacao PRG.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestArtifactNotFound\": \"Artefato de solicitacao bridge nao encontrado.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestDescriptorMismatch\": \"O descritor da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestMediaTypeMismatch\": \"O tipo de midia da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestParameterCountMismatch\": \"A contagem de parametros da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestParameterNameMismatch\": \"O nome do parametro da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequestSchemaVersionMismatch\": \"A versao de esquema da solicitacao bridge nao corresponde.\",\n"
        "  \"RuntimeHost.Bridge.Error.RequiredArguments\": \"A invocacao bridge exige argumentos de caminho de solicitacao/resposta, tipo de midia e versao de esquema.\",\n"
        "  \"RuntimeHost.Bridge.Error.SourceArtifactNotFound\": \"Artefato fonte da rotina bridge nao encontrado.\",\n"
        "  \"RuntimeHost.Bridge.Error.UnsupportedRoutineExportName\": \"O nome de exportacao da rotina bridge nao e um identificador PRG suportado.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteResponseArtifactFailed\": \"Nao foi possivel gravar o artefato de resposta bridge.\",\n"
        "  \"RuntimeHost.Bridge.Error.WriteRoutineBootstrapFailed\": \"Nao foi possivel gravar o bootstrap da rotina bridge.\",\n"
        "  \"RuntimeHost.Error.FederationRequiredOptions\": \"{federationBackendOption} e {federationQueryOption} sao obrigatorios no modo de federacao.\",\n"
        "  \"RuntimeHost.Error.ManifestEmptyOrInvalid\": \"O manifesto esta vazio ou e invalido.\",\n"
        "  \"RuntimeHost.Error.ManifestMissingRuntimeHostSha256\": \"Falta runtime_host_sha256 no manifesto com seguranca habilitada.\",\n"
        "  \"RuntimeHost.Error.ManifestNotFound\": \"Arquivo de manifesto nao encontrado.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionMissing\": \"Falta manifest_version no manifesto.\",\n"
        "  \"RuntimeHost.Error.ManifestVersionUnsupported\": \"manifest_version nao e compativel: {version}. As versoes compativeis sao: {supportedVersions}.\",\n"
        "  \"RuntimeHost.Prompt.QuitConfirm\": \"Deseja sair deste aplicativo? [{yesToken}/{defaultNoToken}]: \",\n"
        "  \"RuntimeHost.Error.RuntimeHostSha256Mismatch\": \"O hash do runtime host nao corresponde ao digest do manifesto.\",\n"
        "  \"RuntimeHost.Error.SecurityPolicyDenied\": \"A politica de seguranca negou {permission} para a funcao '{role}'.\",\n"
        "  \"RuntimeHost.Error.TrueFalseValueRequired\": \"O valor de {option} deve ser true ou false.\",\n"
        "  \"RuntimeHost.Error.UnknownArgument\": \"Argumento desconhecido: {argument}\",\n"
        "  \"RuntimeHost.Error.UnknownFederationBackend\": \"Backend de federacao desconhecido: {backend}\",\n"
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
        "  \"RuntimeHost.Usage.FederationPlanning\": \"       [{planningEnableOption} {booleanValue}] [{planningRequireOption} {booleanValue}] [{planningAuditOption} {booleanValue}]\",\n"
        "  \"RuntimeHost.Usage.Manifest\": \"Uso: {commandName} {manifestOption} {manifestValue} [{debugOption}] [{breakpointOption} {breakpointValue}] [{debugCommandOption} {debugCommandValue}]\",\n"
        "  \"RuntimeHost.Debug.Error.DispatchXAssetActionFailed\": \"Nao foi possivel despachar a acao xAsset: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.InvalidBreakpointCommand\": \"Comando de breakpoint invalido: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.MaterializeXAssetBootstrapFailed\": \"Nao foi possivel materializar o bootstrap xAsset.\",\n"
        "  \"RuntimeHost.Debug.Error.NoRunnableStartupMethodsFound\": \"Nenhum metodo de inicializacao executavel foi encontrado no asset.\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownBreakpoint\": \"Breakpoint desconhecido: {path}:{line}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownBreakpointForXAssetAction\": \"Breakpoint desconhecido para a acao xAsset: {action}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownOrNonBreakpointableXAssetAction\": \"Acao xAsset desconhecida ou sem suporte a breakpoint: {action}\",\n"
        "  \"RuntimeHost.Debug.Error.UnknownXAssetAction\": \"Acao xAsset desconhecida: {command}\",\n"
        "  \"RuntimeHost.Debug.Error.WatchRequiresPausedState\": \"A avaliacao de watch requer um estado pausado ativo.\",\n"
        "  \"RuntimeHost.Debug.Error.XAssetActionBreakpointsRequireBootstrapMode\": \"Breakpoints de acao xAsset exigem o modo xasset-bootstrap.\",\n"
        "  \"RuntimeHost.Prefix.Error\": \"erro: \",\n"
        "  \"RuntimeHost.Prefix.Warning\": \"aviso: \"\n"
        "}\n");
    write_text(pseudo_root / "strings.json", "{}\n");
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string output_line_value(const std::string& output, const std::string& prefix) {
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

std::string breakpoint_entry_path(const std::string& breakpoint_entry) {
    const std::size_t separator = breakpoint_entry.rfind(':');
    if (separator == std::string::npos) {
        return {};
    }
    return breakpoint_entry.substr(0, separator);
}

std::string quote_command_argument(const std::string& value) {
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

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
    std::string stderr_text;
};

ProcessResult run_process_capture(
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
    const int raw_exit_code = std::system(command.c_str());
    fs::current_path(original_directory);

    ProcessResult result;
    if (fs::exists(stdout_path)) {
        result.stdout_text = read_text(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_text(stderr_path);
    }

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

std::optional<copperfin::runtime::XAssetActionBinding> find_action(
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& action_id) {
    for (const auto& action : model.actions) {
        if (action.action_id == action_id) {
            return action;
        }
    }
    return std::nullopt;
}

std::size_t find_breakpoint_line_for_routine_statement(
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

std::size_t find_first_breakpoint_line_for_routine(
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

void write_synthetic_form_asset(const std::filesystem::path& table_path) {
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

void test_runtime_host_compatibility_launcher_note_reflects_xasset_fallback(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_compatibility_note_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());

    const fs::path table_path = temp_root / "broken.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(table_path, "not-a-dbf");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=BrokenXAsset\n"
        "startup_item=broken.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {"--manifest", manifest_path.string()},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "compatibility launcher stdout:\n" << process.stdout_text << "\n";
        std::cerr << "compatibility launcher stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "#3736: compatibility-launcher fallback should still exit successfully for non-PRG startup assets");
    expect(process.stdout_text.find("runtime.mode: compatibility-launcher") != std::string::npos,
           "#3736: compatibility-launcher fallback should report compatibility-launcher mode");
    expect(process.stdout_text.find(
               "launch.note: Startup asset is not a PRG file and could not be materialized for xAsset bootstrap. This launch is falling back to compatibility-launcher mode.") != std::string::npos,
           "#3736: compatibility-launcher fallback should describe xAsset bootstrap fallback instead of claiming non-PRG startup is only a later runtime slice");
    expect(process.stdout_text.find("later runtime slice") == std::string::npos,
           "#3736: compatibility-launcher fallback should not emit the stale later-runtime-slice wording");
    expect(process.stdout_text.find("launch.note: ") != std::string::npos,
           "#3736: compatibility-launcher fallback should continue surfacing a second detailed launch note");
    expect(process.stdout_text.find("debug.breakpoint_support: false") != std::string::npos,
           "#3736: compatibility-launcher fallback should keep breakpoint support disabled");
    expect(process.stdout_text.find("debug.step_support: false") != std::string::npos,
           "#3736: compatibility-launcher fallback should keep step-debug support disabled");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_supports_breakpoint_management_commands(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_breakpoint_command_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "nValue = 2\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=BreakpointDemo\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "break:add:2",
            "--debug-command", "break:list",
            "--debug-command", "break:clear",
            "--debug-command", "break:list",
            "--debug-command", "break:add:3",
            "--debug-command", "continue"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "breakpoint command stdout:\n" << process.stdout_text << "\n";
        std::cerr << "breakpoint command stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "runtime host breakpoint-command smoke should exit successfully");
    expect(process.stdout_text.find("debug.command[0]: break:add:2") != std::string::npos,
           "runtime host should report breakpoint add commands");
    expect(process.stdout_text.find("debug.command[1]: break:list") != std::string::npos,
           "runtime host should report breakpoint list commands");
    expect(process.stdout_text.find("debug.command[2]: break:clear") != std::string::npos,
           "runtime host should report breakpoint clear commands");
    expect(process.stdout_text.find("debug.breakpoint.count: 1") != std::string::npos,
           "runtime host should report one active breakpoint after add");
    expect(process.stdout_text.find("debug.breakpoint[0]: " + startup_path.string() + ":2") != std::string::npos,
           "runtime host should list the added breakpoint against the startup source");
    expect(process.stdout_text.find("debug.breakpoint.count: 0") != std::string::npos,
           "runtime host should report an empty breakpoint inventory after clear");
    expect(process.stdout_text.find("debug.command[5]: continue") != std::string::npos,
           "runtime host should continue after breakpoint management commands");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should still pause on the live managed breakpoint");
    expect(process.stdout_text.find("debug.location: " + startup_path.string() + ":3") != std::string::npos,
           "runtime host should break on the breakpoint added after clear");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_supports_single_breakpoint_removal(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_breakpoint_remove_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "nValue = 2\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=BreakpointRemoveDemo\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "break:add:2",
            "--debug-command", "break:add:3",
            "--debug-command", "break:remove:2",
            "--debug-command", "break:list",
            "--debug-command", "continue"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "breakpoint remove stdout:\n" << process.stdout_text << "\n";
        std::cerr << "breakpoint remove stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "runtime host single-breakpoint removal smoke should exit successfully");
    expect(process.stdout_text.find("debug.command[2]: break:remove:2") != std::string::npos,
           "runtime host should report breakpoint remove commands");
    expect(process.stdout_text.find("debug.command[3]: break:list") != std::string::npos,
           "runtime host should report breakpoint list after removal");
    expect(process.stdout_text.find("debug.breakpoint[0]: " + startup_path.string() + ":2") != std::string::npos,
           "runtime host should initially register the first breakpoint before removal");
    expect(process.stdout_text.find("debug.breakpoint[1]: " + startup_path.string() + ":3") != std::string::npos,
           "runtime host should initially register the second breakpoint before removal");
    expect(process.stdout_text.find("debug.breakpoint[0]: " + startup_path.string() + ":3") != std::string::npos,
           "runtime host should retain the unrelated breakpoint after single removal");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should still pause on the remaining breakpoint");
    expect(process.stdout_text.find("debug.location: " + startup_path.string() + ":3") != std::string::npos,
           "runtime host should pause on the surviving breakpoint after removing the earlier line");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_prefers_debug_manifest_for_implicit_debug_launches(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_implicit_debug_manifest_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path deployed_runtime_host = temp_root / fs::path(runtime_host_path).filename();
    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    const fs::path release_startup_path = temp_root / "release_main.prg";
    const fs::path debug_startup_path = temp_root / "debug_main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path debug_manifest_path = temp_root / "app.cfdebug";
    write_text(
        release_startup_path,
        "LOCAL cMode\n"
        "cMode = 'release'\n"
        "RETURN\n");
    write_text(
        debug_startup_path,
        "LOCAL cMode\n"
        "cMode = 'debug'\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=ImplicitReleaseManifest\n"
        "startup_item=release_main.prg\n"
        "startup_source=" + release_startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");
    write_text(
        debug_manifest_path,
        "debug_manifest_version=2\n"
        "project_title=ImplicitDebugManifest\n"
        "startup_item=debug_main.prg\n"
        "startup_source=" + debug_startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto debug_process = run_process_capture(
        deployed_runtime_host.string(),
        {
            "--debug",
            "--debug-command", "break:add:2",
            "--debug-command", "continue"
        },
        temp_root);

    if (debug_process.exit_code != 0) {
        std::cerr << "implicit debug manifest stdout:\n" << debug_process.stdout_text << "\n";
        std::cerr << "implicit debug manifest stderr:\n" << debug_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(debug_process.exit_code == 0,
           "#3669: implicit debug launches should succeed when app.cfdebug is present");
    expect(debug_process.stdout_text.find("debug.location: " + debug_startup_path.string() + ":2") != std::string::npos,
           "#3669: implicit debug launches should prefer app.cfdebug over app.cfmanifest");

    write_text(debug_manifest_path, "manifest_version=1\nproject_title=BrokenDebugManifest\n");

    const auto non_debug_process = run_process_capture(
        deployed_runtime_host.string(),
        {},
        temp_root);

    if (non_debug_process.exit_code != 0) {
        std::cerr << "implicit non-debug manifest stdout:\n" << non_debug_process.stdout_text << "\n";
        std::cerr << "implicit non-debug manifest stderr:\n" << non_debug_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(non_debug_process.exit_code == 0,
           "#3669: implicit non-debug launches should continue to use app.cfmanifest");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_reports_xasset_pause_identity(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_debug_output_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_synthetic_form_asset(table_path);

    copperfin::studio::StudioOpenRequest request{};
    request.path = table_path.string();
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    expect(open_result.ok, "runtime-host debugger fixture should reopen as a full SCX document");
    if (!open_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "runtime-host debugger fixture should yield an xAsset executable model");
    const auto page_activate = find_action(model, "frmdemo.pgfmain.page2.activate");
    expect(page_activate.has_value(), "synthetic form fixture should expose the nested page action");
    if (!model.ok || !page_activate.has_value()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    const std::size_t breakpoint_line = find_breakpoint_line_for_routine_statement(
        bootstrap,
        page_activate->routine_name,
        "THISFORM.Refresh");
    expect(breakpoint_line != 0U, "synthetic xAsset bootstrap should contain a breakpointable nested page statement");
    if (breakpoint_line == 0U) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoForm\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--breakpoint", std::to_string(breakpoint_line),
            "--debug-command", "continue",
            "--debug-command", "select:frmdemo.pgfmain.page2.activate"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "runtime host stdout:\n" << process.stdout_text << "\n";
        std::cerr << "runtime host stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "runtime host xAsset debugger smoke should exit successfully");
    expect(process.stdout_text.find("runtime.mode: xasset-bootstrap") != std::string::npos,
           "runtime host should report xasset-bootstrap mode");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should pause on the nested xAsset breakpoint");
    expect(process.stdout_text.find("debug.command[1]: select:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report the dispatched xAsset debug command");
    expect(process.stdout_text.find("debug.xasset.action_id: " + page_activate->action_id) != std::string::npos,
           "runtime host pause output should report the originating xAsset action id");
    expect(process.stdout_text.find("debug.xasset.record_index: " + std::to_string(page_activate->record_index)) != std::string::npos,
           "runtime host pause output should report the originating xAsset record index");
    expect(process.stdout_text.find("debug.xasset.kind: " + page_activate->kind) != std::string::npos,
           "runtime host pause output should report the xAsset action kind");
    expect(process.stdout_text.find("debug.xasset.title: " + page_activate->title) != std::string::npos,
           "runtime host pause output should report the xAsset action title");
    expect(process.stdout_text.find("debug.frame[0]: " + page_activate->routine_name + "@") != std::string::npos,
           "runtime host pause stack should still identify the generated action routine");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_supports_xasset_action_breakpoint_commands(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_xasset_breakpoint_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_synthetic_form_asset(table_path);

    copperfin::studio::StudioOpenRequest request{};
    request.path = table_path.string();
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    expect(open_result.ok, "xAsset action-breakpoint fixture should reopen as a full SCX document");
    if (!open_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "xAsset action-breakpoint fixture should yield an executable model");
    const auto page_activate = find_action(model, "frmdemo.pgfmain.page2.activate");
    const auto root_activate = find_action(model, "frmdemo.activate");
    expect(page_activate.has_value(), "xAsset action-breakpoint fixture should expose the nested page action");
    expect(root_activate.has_value(), "xAsset action-breakpoint fixture should expose the root form activate action");
    if (!model.ok || !page_activate.has_value() || !root_activate.has_value()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    const std::size_t first_breakpoint_line =
        find_first_breakpoint_line_for_routine(bootstrap, page_activate->routine_name);
    expect(first_breakpoint_line != 0U, "xAsset action-breakpoint fixture should resolve the first executable line");
    if (first_breakpoint_line == 0U) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoFormBreakpoint\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto add_process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "continue",
            "--debug-command", "break:add-action:frmdemo.pgfmain.page2.activate",
            "--debug-command", "break:list",
            "--debug-command", "select:frmdemo.pgfmain.page2.activate"
        },
        temp_root);

    if (add_process.exit_code != 0) {
        std::cerr << "xasset add-action stdout:\n" << add_process.stdout_text << "\n";
        std::cerr << "xasset add-action stderr:\n" << add_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(add_process.exit_code == 0, "runtime host xAsset add-action breakpoint smoke should exit successfully");
    expect(add_process.stdout_text.find("debug.command[1]: break:add-action:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report xAsset add-action commands");
    expect(add_process.stdout_text.find("debug.breakpoint.count: 1") != std::string::npos,
           "runtime host should report one active xAsset action breakpoint");
    expect(add_process.stdout_text.find("_copperfin_host_bootstrap_") != std::string::npos &&
               add_process.stdout_text.find(".prg:" + std::to_string(first_breakpoint_line)) != std::string::npos,
           "runtime host should list the resolved bootstrap breakpoint for the xAsset action");
    expect(add_process.stdout_text.find("debug.breakpoint[0].xasset.action_id: " + page_activate->action_id) != std::string::npos,
           "runtime host should surface xAsset action ids in breakpoint inventory");
    expect(add_process.stdout_text.find("debug.breakpoint[0].xasset.title: " + page_activate->title) != std::string::npos,
           "runtime host should surface xAsset action titles in breakpoint inventory");
    expect(add_process.stdout_text.find("debug.command[3]: select:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report the dispatched xAsset action after add-action");
    expect(add_process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should pause on the xAsset action breakpoint added by action id");

    const auto remove_process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "continue",
            "--debug-command", "break:add-action:frmdemo.activate",
            "--debug-command", "break:remove-action:frmdemo.activate",
            "--debug-command", "break:list",
            "--debug-command", "select:frmdemo.activate"
        },
        temp_root);

    if (remove_process.exit_code != 0) {
        std::cerr << "xasset remove-action stdout:\n" << remove_process.stdout_text << "\n";
        std::cerr << "xasset remove-action stderr:\n" << remove_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(remove_process.exit_code == 0, "runtime host xAsset remove-action breakpoint smoke should exit successfully");
    expect(remove_process.stdout_text.find("debug.command[2]: break:remove-action:frmdemo.activate") != std::string::npos,
           "runtime host should report xAsset remove-action commands");
    expect(remove_process.stdout_text.find("debug.breakpoint.count: 0") != std::string::npos,
           "runtime host should report an empty inventory after removing the xAsset action breakpoint");
    expect(remove_process.stdout_text.find("debug.command[4]: select:frmdemo.activate") != std::string::npos,
           "runtime host should still dispatch the xAsset action after remove-action");
    expect(remove_process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
           "runtime host should return to the event loop instead of breaking after removing the xAsset action breakpoint");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_surfaces_xasset_breakpoint_metadata_in_pause_output(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_xasset_pause_breakpoint_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_synthetic_form_asset(table_path);

    copperfin::studio::StudioOpenRequest request{};
    request.path = table_path.string();
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    expect(open_result.ok, "xAsset pause-breakpoint fixture should reopen as a full SCX document");
    if (!open_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "xAsset pause-breakpoint fixture should yield an executable model");
    const auto page_activate = find_action(model, "frmdemo.pgfmain.page2.activate");
    expect(page_activate.has_value(), "xAsset pause-breakpoint fixture should expose the nested page action");
    if (!model.ok || !page_activate.has_value()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    const std::size_t first_breakpoint_line =
        find_first_breakpoint_line_for_routine(bootstrap, page_activate->routine_name);
    expect(first_breakpoint_line != 0U, "xAsset pause-breakpoint fixture should resolve the first executable line");
    if (first_breakpoint_line == 0U) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoFormPauseBreakpoint\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "continue",
            "--debug-command", "break:add-action:frmdemo.pgfmain.page2.activate",
            "--debug-command", "select:frmdemo.pgfmain.page2.activate"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "xasset pause-breakpoint stdout:\n" << process.stdout_text << "\n";
        std::cerr << "xasset pause-breakpoint stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "runtime host xAsset pause-breakpoint smoke should exit successfully");
    expect(process.stdout_text.find("break:list") == std::string::npos,
           "pause-breakpoint smoke should not rely on explicit breakpoint inventory commands");
    expect(process.stdout_text.find("debug.command[1]: break:add-action:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report xAsset add-action commands in pause-breakpoint smoke");
    expect(process.stdout_text.find("debug.command[2]: select:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report the dispatched xAsset action in pause-breakpoint smoke");
    expect(process.stdout_text.find("debug.breakpoint.count: 1") != std::string::npos,
           "runtime host pause output should report one active xAsset action breakpoint");
    expect(process.stdout_text.find("_copperfin_host_bootstrap_") != std::string::npos &&
               process.stdout_text.find(".prg:" + std::to_string(first_breakpoint_line)) != std::string::npos,
           "runtime host pause output should still report the resolved bootstrap breakpoint");
    expect(process.stdout_text.find("debug.breakpoint[0].xasset.action_id: " + page_activate->action_id) != std::string::npos,
           "runtime host pause output should surface xAsset action ids for active breakpoints");
    expect(process.stdout_text.find("debug.breakpoint[0].xasset.title: " + page_activate->title) != std::string::npos,
           "runtime host pause output should surface xAsset action titles for active breakpoints");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should pause on the xAsset action breakpoint in pause-breakpoint smoke");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_removes_xasset_bootstrap_after_execution(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_xasset_bootstrap_cleanup_tests";
    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    write_synthetic_form_asset(table_path);

    copperfin::studio::StudioOpenRequest request{};
    request.path = table_path.string();
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    expect(open_result.ok, "xAsset bootstrap cleanup fixture should reopen as a full SCX document");
    if (!open_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "xAsset bootstrap cleanup fixture should yield an executable xAsset model");
    const auto page_activate = find_action(model, "frmdemo.pgfmain.page2.activate");
    expect(page_activate.has_value(), "xAsset bootstrap cleanup fixture should expose the nested page action");
    if (!model.ok || !page_activate.has_value()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoFormCleanup\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto run_debug_inventory = [&](const std::string& label) {
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add-action:frmdemo.pgfmain.page2.activate",
                "--debug-command", "break:list"
            },
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << label << " stdout:\n" << process.stdout_text << "\n";
            std::cerr << label << " stderr:\n" << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0, "runtime host xAsset bootstrap cleanup smoke should exit successfully");
        const std::string breakpoint_entry = output_line_value(process.stdout_text, "debug.breakpoint[0]: ");
        expect(!breakpoint_entry.empty(), "runtime host should report the generated xAsset bootstrap breakpoint entry");
        const std::string bootstrap_path = breakpoint_entry_path(breakpoint_entry);
        expect(!bootstrap_path.empty(), "runtime host should report a parseable xAsset bootstrap breakpoint path");
        if (!bootstrap_path.empty()) {
            expect(bootstrap_path.find("demo_copperfin_host_bootstrap_") != std::string::npos,
                   "runtime host should use a per-run xAsset bootstrap file name");
            expect(!fs::exists(bootstrap_path),
                   "runtime host should remove the xAsset bootstrap file after execution completes");
        }
        return breakpoint_entry;
    };

    const std::string first_breakpoint_entry = run_debug_inventory("xasset-bootstrap-cleanup first run");
    const std::string second_breakpoint_entry = run_debug_inventory("xasset-bootstrap-cleanup second run");
    if (!first_breakpoint_entry.empty() && !second_breakpoint_entry.empty()) {
        expect(first_breakpoint_entry != second_breakpoint_entry,
               "runtime host should not reuse one deterministic xAsset bootstrap path across repeated launches");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_extension_payload_basename_fallback(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_payload_path_fidelity";
    const fs::path builder_root = temp_root / "builder" / "DemoApp";
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path content_root = deployed_root / "content";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(builder_root / "content" / "plugins");
    fs::create_directories(content_root);

    const fs::path deployed_runtime_host = deployed_root / "copperfin_runtime_host.exe";
    const fs::path startup_path = content_root / "main.prg";
    const fs::path root_helper_path = deployed_root / "helper.dll";
    const fs::path manifest_path = deployed_root / "app.cfmanifest";
    const fs::path locale_root = temp_root / "locales";

    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    write_runtime_host_usage_catalogs(locale_root);
    write_text(startup_path, "RETURN\n");
    write_text(root_helper_path, "plugin-payload");

    const auto runtime_host_hash = copperfin::security::sha256_hex_for_file(deployed_runtime_host.string());
    const auto helper_hash = copperfin::security::sha256_hex_for_file(root_helper_path.string());
    expect(runtime_host_hash.ok, "payload-path fidelity fixture should hash the deployed runtime host");
    expect(helper_hash.ok, "payload-path fidelity fixture should hash the decoy root helper payload");
    if (!runtime_host_hash.ok || !helper_hash.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=PayloadPathFidelity\n"
        "project_path=" + (builder_root / "demo.pjx").string() + "\n"
        "package_root=" + builder_root.string() + "\n"
        "content_root=" + (builder_root / "content").string() + "\n"
        "working_directory=" + (builder_root / "content").string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + (builder_root / "content" / "main.prg").string() + "\n"
        "configuration=debug\n"
        "security_enabled=true\n"
        "security_role=developer\n"
        "security_mode=native\n"
        "audit_log_path=" + (builder_root / "security_audit.log").string() + "\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "extension_payload=" + deployed_runtime_host.string() + "|" + runtime_host_hash.hex_digest + "\n"
        "extension_payload=" + (builder_root / "content" / "plugins" / "helper.dll").string() + "|" + helper_hash.hex_digest + "\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            temp_root);

        if (process.exit_code == 0) {
            std::cerr << "payload-path fidelity stdout:\n" << process.stdout_text << "\n";
            std::cerr << "payload-path fidelity stderr:\n" << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 8,
               "runtime host should reject extension payloads that only match by basename outside their recorded package path");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "runtime host payload-path fidelity failures should preserve machine-readable status");
        expect(process.stdout_text.find("error: Extension payload is missing from the package: helper.dll") !=
                   std::string::npos,
               "runtime host should report the missing recorded payload path instead of accepting a same-named root payload");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            temp_root);

        expect(process.exit_code == 8,
               "#2588: pt-BR payload-path fidelity failures should keep the manifest verification exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2588: pt-BR payload-path fidelity failures should preserve machine-readable status");
        expect(process.stdout_text.find("erro: O payload de extensao esta ausente do pacote: helper.dll") !=
                   std::string::npos,
               "#2588: pt-BR payload-path fidelity failures should localize the missing payload error while preserving the file name");
        expect(process.stdout_text.find("Extension payload is missing from the package: helper.dll") ==
                   std::string::npos,
               "#2588: pt-BR payload-path fidelity failures should not fall back to the raw English error");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_manifest_verification_errors_localize_without_changing_contracts(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_manifest_error_localization";
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path content_root = deployed_root / "content";
    const fs::path locale_root = temp_root / "locales";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);

    const fs::path deployed_runtime_host = deployed_root / "copperfin_runtime_host.exe";
    const fs::path startup_path = content_root / "main.prg";
    const fs::path manifest_path = deployed_root / "app.cfmanifest";

    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    write_runtime_host_usage_catalogs(locale_root);
    write_text(startup_path, "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=ManifestErrorLocalization\n"
        "project_path=" + (temp_root / "demo.pjx").string() + "\n"
        "package_root=" + deployed_root.string() + "\n"
        "content_root=" + content_root.string() + "\n"
        "working_directory=" + content_root.string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "configuration=debug\n"
        "security_enabled=true\n"
        "security_role=developer\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            temp_root);

        expect(process.exit_code == 8,
               "#2588: es-419 manifest verification failures should keep the manifest verification exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2588: es-419 manifest verification failures should preserve machine-readable status");
        expect(process.stdout_text.find(
                   "error: Al manifiesto con seguridad habilitada le falta runtime_host_sha256.") !=
                   std::string::npos,
               "#2588: es-419 manifest verification failures should localize the missing runtime_host_sha256 error");
        expect(process.stdout_text.find("Security-enabled manifest is missing runtime_host_sha256.") ==
                   std::string::npos,
               "#2588: es-419 manifest verification failures should not fall back to the raw English error");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_security_denial_audit_details_localize_without_changing_audit_contracts(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_security_denial_audit_localization";
    const fs::path locale_root = temp_root / "locales";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    write_runtime_host_usage_catalogs(locale_root);

    {
        const fs::path case_root = temp_root / "project_open_denied";
        const fs::path content_root = case_root / "content";
        const fs::path manifest_path = case_root / "project_open_denied.cfmanifest";
        const fs::path startup_path = content_root / "project_open_denied.prg";
        const fs::path audit_log_path = case_root / "security_audit.log";
        fs::create_directories(content_root);
        write_text(startup_path, "RETURN\n");
        const std::string manifest_text =
            std::string("manifest_version=1\n"
            "project_title=ProjectOpenDeniedLocalization\n"
            "package_root=") + case_root.string() + "\n"
            "content_root=" + content_root.string() + "\n"
            "working_directory=" + content_root.string() + "\n"
            "startup_item=project_open_denied.prg\n"
            "startup_source=" + startup_path.string() + "\n"
            "security_enabled=true\n"
            "security_role=guest\n"
            "security_mode=native\n"
            "audit_log_path=security_audit.log\n"
            "dotnet_story=none\n";
        write_text(manifest_path, manifest_text);

        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root);

        expect(process.exit_code == 7,
               "#2592: es-419 project.open denials should keep the security denial exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2592: es-419 project.open denials should preserve machine-readable status");
        expect(process.stdout_text.find(
                   "error: La politica de seguridad denego project.open para el rol 'guest'.") !=
                   std::string::npos,
               "#2592: es-419 project.open denials should localize console prose while preserving invariant ids");

        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(audit_log_path.string());
        expect(audit_chain.ok && audit_chain.entries == 1U,
               "#2592: es-419 project.open denials should preserve the immutable audit chain format");
        const std::string audit_text = read_text(audit_log_path);
        expect(audit_text.find("|policy.denied|") != std::string::npos,
               "#2592: es-419 project.open denials should preserve the policy.denied audit event name");
        expect(audit_text.find("La politica de seguridad denego project.open para el rol 'guest'.") !=
                   std::string::npos,
               "#2592: es-419 project.open denials should localize audit detail prose");
        expect(audit_text.find("role missing permission") == std::string::npos,
               "#2592: es-419 project.open denials should not leave raw English audit detail wrappers");
    }

    {
        const fs::path deployed_root = temp_root / "runtime_admin_denied";
        const fs::path content_root = deployed_root / "content";
        const fs::path deployed_runtime_host = deployed_root / "copperfin_runtime_host.exe";
        const fs::path manifest_path = deployed_root / "app.cfmanifest";
        const fs::path startup_path = content_root / "main.prg";
        const fs::path audit_log_path = deployed_root / "security_audit.log";
        fs::create_directories(content_root);
        fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
        fs::permissions(
            deployed_runtime_host,
            fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
            fs::perm_options::add,
            ignored);
#endif

        const auto runtime_host_hash = copperfin::security::sha256_hex_for_file(deployed_runtime_host.string());
        expect(runtime_host_hash.ok, "#2592: runtime-admin denial fixture should hash the deployed runtime host");
        if (!runtime_host_hash.ok) {
            fs::remove_all(temp_root, ignored);
            return;
        }

        write_text(startup_path, "RETURN\n");
        const std::string manifest_text =
            std::string("manifest_version=1\n"
            "project_title=RuntimeAdminDeniedLocalization\n"
            "project_path=") + (deployed_root / "demo.pjx").string() + "\n"
            "package_root=" + deployed_root.string() + "\n"
            "content_root=" + content_root.string() + "\n"
            "working_directory=" + content_root.string() + "\n"
            "startup_item=main.prg\n"
            "startup_source=" + startup_path.string() + "\n"
            "configuration=debug\n"
            "security_enabled=true\n"
            "security_role=developer\n"
            "security_mode=native\n"
            "audit_log_path=" + audit_log_path.string() + "\n"
            "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
            "dotnet_story=none\n";
        write_text(manifest_path, manifest_text);

        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue"
            },
            temp_root);

        expect(process.exit_code == 9,
               "#2592: pt-BR runtime.admin denials should keep the debug-command security denial exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2592: pt-BR runtime.admin denials should preserve machine-readable status");
        expect(process.stdout_text.find(
                   "erro: A politica de seguranca negou runtime.admin para a funcao 'developer'.") !=
                   std::string::npos,
               "#2592: pt-BR runtime.admin denials should localize console prose while preserving invariant ids");

        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(audit_log_path.string());
        expect(audit_chain.ok && audit_chain.entries >= 1U,
               "#2592: pt-BR runtime.admin denials should preserve the immutable audit chain format");
        const std::string audit_text = read_text(audit_log_path);
        expect(audit_text.find("|policy.denied|") != std::string::npos,
               "#2592: pt-BR runtime.admin denials should preserve the policy.denied audit event name");
        expect(audit_text.find("A politica de seguranca negou runtime.admin para a funcao 'developer'.") !=
                   std::string::npos,
               "#2592: pt-BR runtime.admin denials should localize audit detail prose");
        expect(audit_text.find("role missing permission") == std::string::npos,
               "#2592: pt-BR runtime.admin denials should not leave raw English audit detail wrappers");
    }

    {
        const fs::path case_root = temp_root / "manifest_hash_denied";
        const fs::path content_root = case_root / "content";
        const fs::path manifest_path = case_root / "manifest_hash_denied.cfmanifest";
        const fs::path startup_path = content_root / "manifest_hash_denied.prg";
        const fs::path audit_log_path = case_root / "security_audit.log";
        fs::create_directories(content_root);
        write_text(startup_path, "RETURN\n");
        const std::string manifest_text =
            std::string("manifest_version=1\n"
            "project_title=ManifestHashDeniedLocalization\n"
            "package_root=") + case_root.string() + "\n"
            "content_root=" + content_root.string() + "\n"
            "working_directory=" + content_root.string() + "\n"
            "startup_item=manifest_hash_denied.prg\n"
            "startup_source=" + startup_path.string() + "\n"
            "security_enabled=true\n"
            "security_role=developer\n"
            "security_mode=native\n"
            "audit_log_path=security_audit.log\n"
            "dotnet_story=none\n";
        write_text(manifest_path, manifest_text);

        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root);

        expect(process.exit_code == 8,
               "#2592: es-419 manifest-hash denials should keep the manifest verification exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2592: es-419 manifest-hash denials should preserve machine-readable status");
        expect(process.stdout_text.find(
                   "error: Al manifiesto con seguridad habilitada le falta runtime_host_sha256.") !=
                   std::string::npos,
               "#2592: es-419 manifest-hash denials should localize console verification prose");

        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(audit_log_path.string());
        expect(audit_chain.ok && audit_chain.entries == 1U,
               "#2592: es-419 manifest-hash denials should preserve the immutable audit chain format");
        const std::string audit_text = read_text(audit_log_path);
        expect(audit_text.find("|policy.denied|") != std::string::npos,
               "#2592: es-419 manifest-hash denials should preserve the policy.denied audit event name");
        expect(audit_text.find("Al manifiesto con seguridad habilitada le falta runtime_host_sha256.") !=
                   std::string::npos,
               "#2592: es-419 manifest-hash denials should localize audit detail prose");
        expect(audit_text.find("hash verification failed") == std::string::npos,
               "#2592: es-419 manifest-hash denials should not leave the raw English hash-verification wrapper");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_validates_manifest_versions_without_changing_error_contracts(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_manifest_version_contracts";
    const fs::path locale_root = temp_root / "locales";
    const fs::path startup_path = temp_root / "main.prg";
    const fs::path supported_manifest_path = temp_root / "supported_v2.cfmanifest";
    const fs::path missing_manifest_path = temp_root / "missing_version.cfmanifest";
    const fs::path unsupported_manifest_path = temp_root / "unsupported_version.cfmanifest";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_runtime_host_usage_catalogs(locale_root);
    write_text(startup_path, "RETURN\n");

    const std::string base_manifest =
        "project_title=ManifestVersionContract\n"
        "project_path=" + (temp_root / "demo.pjx").string() + "\n"
        "package_root=" + temp_root.string() + "\n"
        "content_root=" + temp_root.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "configuration=debug\n"
        "security_enabled=false\n"
        "security_role=developer\n"
        "security_mode=off\n"
        "dotnet_story=none\n";
    write_text(supported_manifest_path, "manifest_version=2\n" + base_manifest);
    write_text(missing_manifest_path, base_manifest);
    write_text(unsupported_manifest_path, "manifest_version=99\n" + base_manifest);

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", supported_manifest_path.string()},
            temp_root);
        expect(process.exit_code == 0,
               "runtime host should accept supported manifest_version=2 package contracts");
        expect(process.stdout_text.find("status: ok") != std::string::npos,
               "runtime host should preserve machine-readable success status for supported manifest versions");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", missing_manifest_path.string()},
            temp_root);
        expect(process.exit_code == 4,
               "runtime host should reject manifests that omit manifest_version");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "runtime host missing-manifest-version failures should preserve machine-readable status");
        expect(process.stdout_text.find("error: Manifest is missing manifest_version.") != std::string::npos,
               "runtime host should report a localized missing-manifest-version error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", unsupported_manifest_path.string()},
            temp_root);
        expect(process.exit_code == 4,
               "runtime host should reject unsupported manifest versions");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "runtime host unsupported-manifest-version failures should preserve machine-readable status");
        expect(process.stdout_text.find("error: manifest_version no es compatible: 99. Las versiones compatibles son: 1, 2.") != std::string::npos,
               "runtime host should localize unsupported-manifest-version errors while preserving the rejected value");
        expect(process.stdout_text.find("Unsupported manifest_version: 99. Supported versions: 1, 2.") == std::string::npos,
               "runtime host unsupported-manifest-version localization should not fall back to raw English prose");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_ai_federation_planning_without_ai_permission(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_federation_ai_permission_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--federation-backend", "oracle",
            "--federation-query", "DELETE FROM customer",
            "--federation-planning-enable", "true"
        },
        temp_root);

    if (process.exit_code == 0) {
        std::cerr << "federation-ai-permission stdout:\n" << process.stdout_text << "\n";
        std::cerr << "federation-ai-permission stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 7,
           "runtime host should deny AI-assisted federation planning when the effective role lacks ai.mcp");
    expect(process.stdout_text.find("runtime.mode: federation-query-plan") != std::string::npos,
           "runtime host should keep the federation runtime mode visible on AI permission denials");
    expect(process.stdout_text.find("error: Security policy denied ai.mcp for role 'developer'.") != std::string::npos,
           "runtime host should report the missing ai.mcp permission for the default developer role");

    {
        ScopedEnvironmentValue allow_ai_role("COPPERFIN_SECURITY_ROLE", "runtime-operator");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto allowed_process = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "oracle",
                "--federation-query", "DELETE FROM customer",
                "--federation-planning-enable", "true"
            },
            temp_root);

        expect(allowed_process.exit_code == 6,
               "#2593: es-419 runtime host should advance past AI permission gating for runtime-operator and reach planner fallback");
        expect(allowed_process.stdout_text.find(
                   "error: El planner aun no esta implementado para la politica de IA optional. La traduccion deterministica fallo: "
                   "Solo se admite la traduccion SQL deterministica de primera pasada de SELECT...FROM.") !=
                   std::string::npos,
               "#2594: es-419 runtime host should localize both the planner-fallback wrapper and translator payload once AI permission is granted");
        expect(allowed_process.stdout_text.find("runtime.mode: federation-query-plan") != std::string::npos,
               "#2593: es-419 runtime host should preserve the federation runtime mode during planner fallback");
        expect(allowed_process.stdout_text.find("Platform.QueryTranslator.Error.SelectFromOnly") == std::string::npos,
               "#2594: es-419 runtime host should not leak the unresolved translator diagnostic key");
        expect(allowed_process.stdout_text.find("Planner is not yet implemented for optional AI policy.") == std::string::npos,
               "#2593: es-419 runtime host should not fall back to raw English planner-fallback prose");
    }

    {
        ScopedEnvironmentValue allow_ai_role("COPPERFIN_SECURITY_ROLE", "runtime-operator");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const std::string pseudo_translation_error = copperfin::localization::pseudo_localize(
            "Only first-pass SELECT...FROM SQL translation is supported.");
        const auto allowed_process = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "oracle",
                "--federation-query", "DELETE FROM customer",
                "--federation-planning-enable", "true"
            },
            temp_root);

        expect(allowed_process.exit_code == 6,
               "#2593: qps-ploc runtime host should keep the planner-fallback exit code after AI permission is granted");
        expect(allowed_process.stdout_text.find("runtime.mode: federation-query-plan") != std::string::npos,
               "#2593: qps-ploc runtime host should preserve the federation runtime mode during planner fallback");
        expect(allowed_process.stdout_text.find("[!! ërrør:  !!][!! ") != std::string::npos,
               "#2593: qps-ploc runtime host should pseudo-localize the planner-fallback prose");
        expect(allowed_process.stdout_text.find("Platform.QueryTranslator.Error.SelectFromOnly") ==
                   std::string::npos,
               "#2594: qps-ploc runtime host should not leak the unresolved translator diagnostic key");
        expect(allowed_process.stdout_text.find("Only first-pass SELECT...FROM SQL translation is supported.") ==
                   std::string::npos,
               "#2594: qps-ploc runtime host should pseudo-localize the deterministic translator payload prose");
        expect(allowed_process.stdout_text.find(pseudo_translation_error) != std::string::npos,
               "#2594: qps-ploc runtime host should surface the pseudo-localized deterministic translator payload");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_writes_bridge_response_artifact(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_response_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeResponse\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "RETURN 42\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 7,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "7",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-response stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-response stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should accept wrapper-emitted bridge descriptor and response arguments");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should report bridge invocation mode");
    expect(process.stdout_text.find("bridge.library_export: AddNumbers") != std::string::npos,
           "runtime host should preserve bridge export metadata in diagnostics");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should report the PRG return value in bridge diagnostics");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host bridge mode should invoke exported routines through a bootstrap");
    expect(fs::exists(response_path),
           "runtime host should write the requested bridge response artifact");

    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"status\": \"ok\"") != std::string::npos,
           "runtime host bridge response should include ok status");
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "runtime host bridge response should include the evaluated PRG return value");
    expect(response_document.find("\"response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\"") != std::string::npos,
           "runtime host bridge response should echo the expected response media type");
    expect(response_document.find("\"schema_version\": \"v1\"") != std::string::npos,
           "runtime host bridge response should echo the requested schema version");
    expect(response_document.find("\"diagnostics\": \"bridge_response_written\"") != std::string::npos,
           "runtime host bridge response should include diagnostics");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_invokes_zero_argument_bridge_export(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_zero_arg_export_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeZeroArgExport\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"GetAnswer\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-zero-arg-export stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-zero-arg-export stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should invoke zero-argument bridge exports through a bootstrap PRG");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should report bridge invocation mode for zero-argument exports");
    expect(process.stdout_text.find("bridge.library_export: GetAnswer") != std::string::npos,
           "runtime host should preserve zero-argument export metadata in diagnostics");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should report routine bootstrap execution for zero-argument exports");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should report the zero-argument export return value in diagnostics");
    expect(fs::exists(response_path),
           "runtime host should write the bridge response for zero-argument exports");

    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"status\": \"ok\"") != std::string::npos,
           "zero-argument bridge export response should include ok status");
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "zero-argument bridge export response should include the exported routine return value");
    expect(response_document.find("\"schema_version\": \"v1\"") != std::string::npos,
           "zero-argument bridge export response should echo the requested schema version");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_removes_bridge_routine_bootstrap_after_execution(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_bootstrap_cleanup_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeBootstrapCleanup\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"GetAnswer\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-bootstrap-cleanup stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-bootstrap-cleanup stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should complete bridge routine invocation before bootstrap cleanup assertion");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should report routine bootstrap execution for cleanup coverage");
    const std::string execution_source = output_line_value(process.stdout_text, "bridge.execution_source: ");
    expect(!execution_source.empty(),
           "runtime host should report the materialized bootstrap execution source");
    if (!execution_source.empty()) {
        expect(execution_source.find("copperfin_bridge_GetAnswer_") != std::string::npos,
               "runtime host should report the generated bridge bootstrap path");
        expect(!fs::exists(execution_source),
               "runtime host should remove the generated bridge bootstrap after execution");
    }
    expect(fs::exists(response_path),
           "runtime host should still write the bridge response after bootstrap cleanup");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_unescapes_bridge_descriptor_string_fields(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_escaped_descriptor_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports\\escaped.prg";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeEscapedDescriptor\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");

    std::string escaped_source_path = source_path.string();
    std::size_t slash_offset = 0;
    while ((slash_offset = escaped_source_path.find('\\', slash_offset)) != std::string::npos) {
        escaped_source_path.replace(slash_offset, 1U, "\\\\");
        slash_offset += 2U;
    }
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"GetAnswer\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + escaped_source_path + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-escaped-descriptor stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-escaped-descriptor stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should decode escaped descriptor strings before bridge validation");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should reach routine bootstrap execution after escaped descriptor validation");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should execute the escaped-path descriptor source");
    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "escaped descriptor bridge response should include the exported routine return value");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_passes_bridge_request_parameters_to_export(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_parameter_export_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeParameterExport\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"dll-int\"},\n"
        "    {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"dll-int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-parameter-export stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-parameter-export stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should pass bridge request parameter values to exported routines");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should report routine bootstrap execution for parameterized exports");
    expect(process.stdout_text.find("bridge.parameter_count: 2") != std::string::npos,
           "runtime host should preserve the parameter count in diagnostics");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should report the parameterized export return value in diagnostics");
    expect(fs::exists(response_path),
           "runtime host should write the bridge response for parameterized exports");

    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"status\": \"ok\"") != std::string::npos,
           "parameterized bridge export response should include ok status");
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "parameterized bridge export response should include the exported routine return value");
    expect(response_document.find("\"response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\"") != std::string::npos,
           "parameterized bridge export response should echo the expected response media type");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_parameter_count_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_parameter_mismatch_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeParameterMismatch\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight|tnExtra\",\n"
        "  \"parameter_count\": 3,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"dll-int\"},\n"
        "    {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"dll-int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight|tnExtra",
            "--parameter-count", "3",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-parameter-mismatch stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-parameter-mismatch stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge parameter count mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on parameter count mismatches");
    expect(process.stdout_text.find("error: Bridge request parameter count mismatch.") != std::string::npos,
           "runtime host should report bridge parameter count mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when bridge parameter counts mismatch");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_nested_bridge_parameter_array_for_nonzero_arity(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_nested_parameter_array_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeNestedParameters\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n"
        "RETURN 7\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameter_shadow\": {\n"
        "    \"parameters\": [\n"
        "      {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"int\"},\n"
        "      {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"int\"}\n"
        "    ]\n"
        "  }\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-nested-parameter-array stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-nested-parameter-array stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject nested bridge parameter arrays for nonzero arity");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on nested parameter-array errors");
    expect(process.stdout_text.find("error: Bridge request parameter count mismatch.") != std::string::npos,
           "runtime host should report a parameter count mismatch when no top-level parameter payload exists");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when nonzero bridge parameters are nested");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_nested_bridge_parameter_values_for_nonzero_arity(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_nested_parameter_value_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeNestedParameterValues\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n"
        "RETURN 7\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value_shadow\": {\"value\": \"40\"}, \"surface\": \"int\"},\n"
        "    {\"name\": \"tnRight\", \"value_shadow\": {\"value\": \"2\"}, \"surface\": \"int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-nested-parameter-values stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-nested-parameter-values stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject nested bridge parameter values for nonzero arity");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on nested parameter-value errors");
    expect(process.stdout_text.find("error: Bridge request parameter count mismatch.") != std::string::npos,
           "runtime host should report a parameter count mismatch when parameter values are nested");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when nonzero bridge parameter values are nested");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_parameter_name_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_parameter_name_mismatch_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeParameterNameMismatch\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnRight\", \"value\": \"40\", \"surface\": \"dll-int\"},\n"
        "    {\"name\": \"tnLeft\", \"value\": \"2\", \"surface\": \"dll-int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-parameter-name-mismatch stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-parameter-name-mismatch stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge parameter name mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on parameter name mismatches");
    expect(process.stdout_text.find("error: Bridge request parameter name mismatch.") != std::string::npos,
           "runtime host should report bridge parameter name mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when bridge parameter names mismatch");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_request_contract_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_request_contract_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeRequestContract\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        "{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.bad-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "left,right",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-request-contract stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-request-contract stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge request media-type mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on request contract errors");
    expect(process.stdout_text.find("error: Bridge request media type mismatch.") != std::string::npos,
           "runtime host should report the request media-type mismatch");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when the request contract mismatches");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_nested_bridge_descriptor_fields(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_nested_descriptor_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeNestedDescriptor\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"descriptor_shadow\": {\n"
        "    \"export_name\": \"AddNumbers\",\n"
        "    \"routine_kind\": \"procedure\",\n"
        "    \"source_path\": \"") + source_path.string() + "\",\n"
        "    \"parameter_count\": 0,\n"
        "    \"schema_version\": \"v1\",\n"
        "    \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\"\n"
        "  },\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-nested-descriptor stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-nested-descriptor stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject nested bridge descriptor fields before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on nested descriptor errors");
    expect(process.stdout_text.find("error: Bridge request media type mismatch.") != std::string::npos,
           "runtime host should not accept nested request-media fields as top-level contract fields");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when bridge descriptor fields are nested");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_descriptor_identity_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_descriptor_contract_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeDescriptorContract\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"WrongExport\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"left,right\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "left,right",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-descriptor-contract stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-descriptor-contract stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge descriptor identity mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on descriptor contract errors");
    expect(process.stdout_text.find("error: Bridge request descriptor mismatch.") != std::string::npos,
           "runtime host should report descriptor identity mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when descriptor identity mismatches");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_descriptor_metadata_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_descriptor_metadata_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeDescriptorMetadata\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + source_path.string() + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"PARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"int\"},\n"
        "    {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-descriptor-metadata stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-descriptor-metadata stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge descriptor metadata mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on descriptor metadata errors");
    expect(process.stdout_text.find("error: Bridge request descriptor mismatch.") != std::string::npos,
           "runtime host should report descriptor metadata mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when descriptor metadata mismatches");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_usage_text_localizes_without_changing_cli_tokens(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_usage_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(runtime_host_path, {}, temp_root);
        expect(process.exit_code == 2,
               "#2349: runtime host without manifest should keep the usage exit code");
        expect(process.stdout_text.find("Usage: copperfin_runtime_host --manifest <path> [--debug]") != std::string::npos,
               "#2349: runtime host en-US usage should remain stable");
        expect(process.stdout_text.find("--federation-backend <sqlite|postgresql|sqlserver|oracle>") != std::string::npos,
               "#2349: runtime host en-US usage should preserve federation CLI tokens");

        const auto invalid_federation_bool = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "sqlite",
                "--federation-query", "SELECT * FROM customer",
                "--federation-planning-enable", "maybe"
            },
            temp_root);
        expect(invalid_federation_bool.exit_code == 2,
               "#3791: runtime host should reject invalid federation planning booleans");
        expect(invalid_federation_bool.stdout_text.find("status: error") != std::string::npos,
               "#3791: invalid federation planning booleans should preserve machine-readable status");
        expect(invalid_federation_bool.stdout_text.find(
                   "error: The --federation-planning-enable value must be true or false.") != std::string::npos,
               "#3791: invalid federation planning booleans should localize the en-US parse error");
        expect(invalid_federation_bool.stdout_text.find("--federation-planning-enable") != std::string::npos &&
                   invalid_federation_bool.stdout_text.find("<true|false>") != std::string::npos,
               "#3791: invalid federation planning booleans should preserve invariant CLI tokens in usage output");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(runtime_host_path, {}, temp_root);
        expect(process.exit_code == 2,
               "#2585: es-419 runtime host usage should keep the usage exit code");
        expect(process.stdout_text.find("Uso: copperfin_runtime_host --manifest <path> [--debug]") != std::string::npos,
               "#2585: es-419 runtime host usage should localize manifest usage prose");
        expect(process.stdout_text.find("   o: copperfin_runtime_host") != std::string::npos &&
                   process.stdout_text.find("--federation-backend") != std::string::npos &&
                   process.stdout_text.find("--federation-query") != std::string::npos,
               "#2585: es-419 runtime host usage should localize alternate usage prose while preserving CLI tokens");
        expect(process.stdout_text.find("Usage: copperfin_runtime_host --manifest <path> [--debug]") == std::string::npos,
               "#2585: es-419 runtime host usage should not fall back to raw English prose");

        const auto slash_locale_process = run_process_capture(runtime_host_path, {"/locale", "es-419"}, temp_root);
        expect(slash_locale_process.exit_code == 2,
               "#3752: /locale should keep the normal usage exit code when no manifest is available");
        expect(slash_locale_process.stdout_text.find("Uso: copperfin_runtime_host --manifest <path> [--debug]") != std::string::npos,
               "#3752: /locale should select the same localized catalog as --locale");
        expect(slash_locale_process.stdout_text.find("Usage: copperfin_runtime_host --manifest <path> [--debug]") == std::string::npos,
               "#3752: /locale should not fall back to raw English prose");

        const auto invalid_federation_bool = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "sqlite",
                "--federation-query", "SELECT * FROM customer",
                "--federation-planning-require", "quizas"
            },
            temp_root);
        expect(invalid_federation_bool.exit_code == 2,
               "#3791: es-419 invalid federation planning booleans should keep the usage exit code");
        expect(invalid_federation_bool.stdout_text.find(
                   "error: El valor de --federation-planning-require debe ser true o false.") != std::string::npos,
               "#3791: es-419 invalid federation planning booleans should localize parse errors while preserving option tokens");
        expect(invalid_federation_bool.stdout_text.find(
                   "error: The --federation-planning-require value must be true or false.") == std::string::npos,
               "#3791: es-419 invalid federation planning booleans should not fall back to raw English prose");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(runtime_host_path, {}, temp_root);
        expect(process.exit_code == 2,
               "#2349: pseudo-localized runtime host usage should keep the usage exit code");
        expect(process.stdout_text.find("[!! ") != std::string::npos,
               "#2349: pseudo-localized runtime host usage should decorate prose");
        expect(process.stdout_text.find("copperfin_runtime_host") != std::string::npos &&
                   process.stdout_text.find("--manifest") != std::string::npos &&
                   process.stdout_text.find("--debug-command") != std::string::npos &&
                   process.stdout_text.find("<continue|step|next|out|watch:<expr>|select:<action-id>|invoke:<action-id>|break:add:<file:line>|break:remove:<file:line>|break:add-action:<action-id>|break:remove-action:<action-id>|break:clear|break:list>") != std::string::npos,
               "#2349: pseudo-localized runtime host usage should preserve CLI and debug-command tokens");

        const auto slash_debug = run_process_capture(runtime_host_path, {"/debug"}, temp_root);
        expect(slash_debug.exit_code == 2,
               "#3752: /debug should keep the normal usage exit code when no manifest is available");
        expect(slash_debug.stdout_text.find("status: error") == std::string::npos,
               "#3752: /debug should be accepted as a host alias instead of surfacing an unknown-argument contract");
        expect(slash_debug.stdout_text.find("[!! ") != std::string::npos,
               "#3752: /debug acceptance should still honor the selected pseudo-localized catalog");
        expect(slash_debug.stdout_text.find("--debug-command") != std::string::npos,
               "#3752: /debug acceptance should preserve ordinary usage/debug token output");

        const auto unknown_argument = run_process_capture(runtime_host_path, {"--unknown-option"}, temp_root);
        expect(unknown_argument.exit_code == 2,
               "#2351: pseudo-localized runtime host unknown arguments should keep the usage exit code");
        expect(unknown_argument.stdout_text.find("status: error") != std::string::npos,
               "#2351: pseudo-localized runtime host errors should preserve machine-readable status");
        expect(unknown_argument.stdout_text.find("[!! ") != std::string::npos,
               "#2351: pseudo-localized runtime host unknown arguments should decorate prose");
        expect(unknown_argument.stdout_text.find("--unknown-option") != std::string::npos,
               "#2351: pseudo-localized runtime host unknown arguments should preserve CLI tokens");

        const auto missing_federation_argument = run_process_capture(
            runtime_host_path,
            {"--federation-backend", "sqlite"},
            temp_root);
        expect(missing_federation_argument.exit_code == 2,
               "#2351: pseudo-localized federation validation should keep the usage exit code");
        expect(missing_federation_argument.stdout_text.find("status: error") != std::string::npos,
               "#2351: pseudo-localized federation validation should preserve machine-readable status");
        expect(missing_federation_argument.stdout_text.find("[!! ") != std::string::npos,
               "#2351: pseudo-localized federation validation should decorate prose");
        expect(missing_federation_argument.stdout_text.find("--federation-backend") != std::string::npos &&
                   missing_federation_argument.stdout_text.find("--federation-query") != std::string::npos,
               "#2351: pseudo-localized federation validation should preserve CLI tokens");

        const auto invalid_federation_bool = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "sqlite",
                "--federation-query", "SELECT * FROM customer",
                "--federation-planning-audit", "maybe"
            },
            temp_root);
        expect(invalid_federation_bool.exit_code == 2,
               "#3791: pseudo-localized invalid federation planning booleans should keep the usage exit code");
        expect(invalid_federation_bool.stdout_text.find("status: error") != std::string::npos,
               "#3791: pseudo-localized invalid federation planning booleans should preserve machine-readable status");
        expect(invalid_federation_bool.stdout_text.find("[!! ") != std::string::npos,
               "#3791: pseudo-localized invalid federation planning booleans should decorate prose");
        expect(invalid_federation_bool.stdout_text.find("--federation-planning-audit") != std::string::npos &&
                   invalid_federation_bool.stdout_text.find("true") != std::string::npos &&
                   invalid_federation_bool.stdout_text.find("false") != std::string::npos,
               "#3791: pseudo-localized invalid federation planning booleans should preserve invariant boolean tokens");

        const fs::path bridge_manifest_path = temp_root / "bridge.cfmanifest";
        const fs::path bridge_source_path = temp_root / "bridge.prg";
        const fs::path bridge_response_path = temp_root / "bridge.response.json";
        write_text(
            bridge_manifest_path,
            std::string("manifest_version=1\n"
            "project_title=BridgeLocalization\n"
            "startup_item=bridge.prg\n"
            "startup_source=") + bridge_source_path.string() + "\n"
            "security_enabled=false\n"
            "dotnet_story=none\n");
        write_text(bridge_source_path, "RETURN 1\n");

        const auto bridge_error = run_process_capture(
            runtime_host_path,
            {
                "--manifest", bridge_manifest_path.string(),
                "--request-path", (temp_root / "missing.request.json").string(),
                "--response-path", bridge_response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        expect(bridge_error.exit_code == 6,
               "#2352: pseudo-localized bridge errors should keep the bridge validation exit code");
        expect(bridge_error.stdout_text.find("status: error") != std::string::npos,
               "#2352: pseudo-localized bridge errors should preserve machine-readable status");
        expect(bridge_error.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
               "#2352: pseudo-localized bridge errors should preserve bridge runtime mode");
        expect(bridge_error.stdout_text.find("[!! ") != std::string::npos,
               "#2352: pseudo-localized bridge errors should decorate prose");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");

        const fs::path bridge_manifest_path = temp_root / "bridge_es.cfmanifest";
        const fs::path bridge_source_path = temp_root / "bridge_es.prg";
        const fs::path bridge_response_path = temp_root / "bridge_es.response.json";
        write_text(
            bridge_manifest_path,
            std::string("manifest_version=1\n"
            "project_title=BridgeLocalizationSpanish\n"
            "startup_item=bridge_es.prg\n"
            "startup_source=") + bridge_source_path.string() + "\n"
            "security_enabled=false\n"
            "dotnet_story=none\n");
        write_text(bridge_source_path, "RETURN 1\n");

        const auto bridge_error = run_process_capture(
            runtime_host_path,
            {
                "--manifest", bridge_manifest_path.string(),
                "--request-path", (temp_root / "missing.request.es.json").string(),
                "--response-path", bridge_response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        expect(bridge_error.exit_code == 6,
               "#2587: es-419 bridge request-artifact errors should keep the bridge validation exit code");
        expect(bridge_error.stdout_text.find("status: error") != std::string::npos,
               "#2587: es-419 bridge request-artifact errors should preserve machine-readable status");
        expect(bridge_error.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
               "#2587: es-419 bridge request-artifact errors should preserve bridge runtime mode");
        expect(bridge_error.stdout_text.find("error: No se encontro el artefacto de solicitud bridge.") != std::string::npos,
               "#2587: es-419 bridge request-artifact errors should localize prose");
        expect(bridge_error.stdout_text.find("error: Bridge request artifact not found.") == std::string::npos,
               "#2587: es-419 bridge request-artifact errors should not fall back to raw English prose");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");

        const auto unknown_argument = run_process_capture(runtime_host_path, {"--unknown-option"}, temp_root);
        expect(unknown_argument.exit_code == 2,
               "#2585: pt-BR runtime host unknown arguments should keep the usage exit code");
        expect(unknown_argument.stdout_text.find("status: error") != std::string::npos,
               "#2585: pt-BR runtime host unknown arguments should preserve machine-readable status");
        expect(unknown_argument.stdout_text.find("erro: Argumento desconhecido: --unknown-option") != std::string::npos,
               "#2585: pt-BR runtime host unknown arguments should localize prefixed error prose");
        expect(unknown_argument.stdout_text.find("error: Unknown argument: --unknown-option") == std::string::npos,
               "#2585: pt-BR runtime host unknown arguments should not fall back to raw English prose");

        const auto missing_federation_argument = run_process_capture(
            runtime_host_path,
            {"--federation-backend", "sqlite"},
            temp_root);
        expect(missing_federation_argument.exit_code == 2,
               "#2585: pt-BR federation validation should keep the usage exit code");
        expect(missing_federation_argument.stdout_text.find("erro: --federation-backend e --federation-query sao obrigatorios no modo de federacao.") != std::string::npos,
               "#2585: pt-BR federation validation should localize required-option prose");
        expect(missing_federation_argument.stdout_text.find("--federation-backend") != std::string::npos &&
                   missing_federation_argument.stdout_text.find("--federation-query") != std::string::npos,
               "#2585: pt-BR federation validation should preserve CLI tokens");
        expect(missing_federation_argument.stdout_text.find("error: --federation-backend and --federation-query are both required in federation mode.") == std::string::npos,
               "#2585: pt-BR federation validation should not fall back to raw English prose");

        const fs::path bridge_manifest_path = temp_root / "bridge_pt.cfmanifest";
        const fs::path bridge_source_path = temp_root / "bridge_pt.prg";
        const fs::path bridge_response_path = temp_root / "bridge_pt.response.json";
        write_text(
            bridge_manifest_path,
            std::string("manifest_version=1\n"
            "project_title=BridgeLocalizationPortuguese\n"
            "startup_item=bridge_pt.prg\n"
            "startup_source=") + bridge_source_path.string() + "\n"
            "security_enabled=false\n"
            "dotnet_story=none\n");
        write_text(bridge_source_path, "RETURN 1\n");

        const auto bridge_error = run_process_capture(
            runtime_host_path,
            {
                "--manifest", bridge_manifest_path.string(),
                "--request-path", (temp_root / "missing.request.pt.json").string(),
                "--response-path", bridge_response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        expect(bridge_error.exit_code == 6,
               "#2587: pt-BR bridge request-artifact errors should keep the bridge validation exit code");
        expect(bridge_error.stdout_text.find("status: error") != std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should preserve machine-readable status");
        expect(bridge_error.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should preserve bridge runtime mode");
        expect(bridge_error.stdout_text.find("erro: Artefato de solicitacao bridge nao encontrado.") != std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should localize prose");
        expect(bridge_error.stdout_text.find("error: Bridge request artifact not found.") == std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should not fall back to raw English prose");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_debug_errors_localize_without_changing_command_tokens(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_debug_error_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DebugErrorLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add:not-a-breakpoint"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2391: en-US invalid breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2391: en-US invalid breakpoint diagnostics should preserve machine-readable status");
        expect(
            process.stdout_text.find("error: Invalid breakpoint command: break:add:not-a-breakpoint") !=
                std::string::npos,
            "#2391: en-US invalid breakpoint diagnostics should remain stable");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:remove:2"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2586: es-419 unknown breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2586: es-419 unknown breakpoint diagnostics should preserve machine-readable status");
        expect(
            process.stdout_text.find("Breakpoint desconocido: " + startup_path.string() + ":2") != std::string::npos,
            "#2586: es-419 unknown breakpoint diagnostics should localize the error body while preserving path and line");
        expect(process.stdout_text.find("Unknown breakpoint: " + startup_path.string() + ":2") == std::string::npos,
               "#2586: es-419 unknown breakpoint diagnostics should not fall back to the raw English error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add:not-a-breakpoint"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2566: pt-BR invalid breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should preserve machine-readable status");
        expect(process.stdout_text.find("erro: ") != std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should localize the error prefix");
        expect(process.stdout_text.find("Comando de breakpoint invalido: break:add:not-a-breakpoint") != std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should localize the error body");
        expect(process.stdout_text.find("error: Invalid breakpoint command") == std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should not fall back to the raw English prefixed error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "watch:nValue"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2586: pt-BR watch diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2586: pt-BR watch diagnostics should preserve machine-readable status");
        expect(process.stdout_text.find("erro: ") != std::string::npos,
               "#2586: pt-BR watch diagnostics should localize the error prefix");
        expect(process.stdout_text.find("A avaliacao de watch requer um estado pausado ativo.") != std::string::npos,
               "#2586: pt-BR watch diagnostics should localize the paused-state error");
        expect(process.stdout_text.find("Watch evaluation requires an active paused state.") == std::string::npos,
               "#2586: pt-BR watch diagnostics should not fall back to the raw English error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add:not-a-breakpoint"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2391: pseudo-localized invalid breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2391: pseudo-localized invalid breakpoint diagnostics should preserve machine-readable status");
        const std::string pseudo_error_prefix =
            copperfin::localization::load_catalogs(locale_root, "qps-ploc").translate("RuntimeHost.Prefix.Error");
        expect(process.stdout_text.find(pseudo_error_prefix) != std::string::npos,
               "#2566: pseudo-localized invalid breakpoint diagnostics should route the error prefix through qps-ploc");
        expect(process.stdout_text.find("[!! ") != std::string::npos,
               "#2391: pseudo-localized invalid breakpoint diagnostics should decorate prose");
        expect(process.stdout_text.find("break:add:not-a-breakpoint") != std::string::npos,
               "#2391: pseudo-localized invalid breakpoint diagnostics should preserve debug command tokens");
        expect(process.stdout_text.find("error: Invalid breakpoint command: break:add:not-a-breakpoint") == std::string::npos,
               "#2566: pseudo-localized invalid breakpoint diagnostics should not fall back to the raw English prefixed error");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_pause_messages_localize_without_changing_pause_reasons(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_pause_message_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=PauseMessageLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        write_text(startup_path, "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: en-US completed pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("status: ok") != std::string::npos,
               "#2589: en-US completed pause messages should preserve machine-readable ok status");
        expect(process.stdout_text.find("debug.reason: completed") != std::string::npos,
               "#2589: en-US completed pause messages should preserve the completed pause reason");
        expect(process.stdout_text.find("debug.message: Execution completed.") != std::string::npos,
               "#2589: en-US completed pause messages should remain stable");
    }

    {
        write_text(
            startup_path,
            "LOCAL nValue\n"
            "nValue = 1\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "step"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: es-419 step pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: step") != std::string::npos,
               "#2589: es-419 step pause messages should preserve the step pause reason");
        expect(process.stdout_text.find("debug.message: El paso se completo.") != std::string::npos,
               "#2589: es-419 step pause messages should localize the step-completed prose");
        expect(process.stdout_text.find("debug.message: Step completed.") == std::string::npos,
               "#2589: es-419 step pause messages should not fall back to the raw English step-completed prose");
    }

    {
        write_text(
            startup_path,
            "READ EVENTS\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: pt-BR READ EVENTS pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
               "#2589: pt-BR READ EVENTS pause messages should preserve the event-loop pause reason");
        expect(process.stdout_text.find("debug.message: O runtime esta aguardando em READ EVENTS.") !=
                   std::string::npos,
               "#2589: pt-BR READ EVENTS pause messages should localize prose while preserving the READ EVENTS token");
        expect(process.stdout_text.find("The runtime is waiting in READ EVENTS.") == std::string::npos,
               "#2589: pt-BR READ EVENTS pause messages should not fall back to the raw English prose");
    }

    {
        write_text(
            startup_path,
            "LOCAL nValue\n"
            "nValue = 1\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--breakpoint", startup_path.string() + ":2",
                "--debug-command", "continue"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: qps-ploc breakpoint pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "#2589: qps-ploc breakpoint pause messages should preserve the breakpoint pause reason");
        expect(process.stdout_text.find("debug.message: [!! ") != std::string::npos,
               "#2589: qps-ploc breakpoint pause messages should pseudo-localize the debug message");
        expect(process.stdout_text.find("Breakpoint hit.") == std::string::npos,
               "#2589: qps-ploc breakpoint pause messages should not fall back to the raw English breakpoint prose");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_watch_errors_localize_without_changing_watch_fields(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_watch_error_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=WatchErrorLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--breakpoint", startup_path.string() + ":2",
                "--debug-command", "continue",
                "--debug-command", "watch:"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2590: pt-BR watch errors should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "#2590: pt-BR watch errors should preserve the breakpoint pause reason");
        expect(process.stdout_text.find("debug.watch.expression: ") != std::string::npos,
               "#2590: pt-BR watch errors should preserve the debug.watch.expression field");
        expect(process.stdout_text.find("debug.watch.ok: false") != std::string::npos,
               "#2590: pt-BR watch errors should preserve the debug.watch.ok field");
        expect(process.stdout_text.find("debug.watch.error: A expressao de watch esta vazia.") !=
                   std::string::npos,
               "#2590: pt-BR watch errors should localize the watch error prose");
        expect(process.stdout_text.find("debug.watch.error: Watch expression is empty.") ==
                   std::string::npos,
               "#2590: pt-BR watch errors should not fall back to the raw English watch error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--breakpoint", startup_path.string() + ":2",
                "--debug-command", "continue",
                "--debug-command", "watch:"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2590: qps-ploc watch errors should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "#2590: qps-ploc watch errors should preserve the breakpoint pause reason");
        expect(process.stdout_text.find("debug.watch.ok: false") != std::string::npos,
               "#2590: qps-ploc watch errors should preserve the debug.watch.ok field");
        expect(process.stdout_text.find("debug.watch.error: [!! ") != std::string::npos,
               "#2590: qps-ploc watch errors should pseudo-localize the watch error prose");
        expect(process.stdout_text.find("debug.watch.error: Watch expression is empty.") ==
                   std::string::npos,
               "#2590: qps-ploc watch errors should not fall back to the raw English watch error");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_quit_prompt_localizes_without_changing_confirmation_tokens(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_quit_prompt_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "quit_prompt.prg";
    const fs::path manifest_path = temp_root / "quit_prompt.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "QUIT\n"
        "nValue = 1\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=QuitPromptLocalization\n"
        "startup_item=quit_prompt.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("Do you want to quit this application? [y/N]: ") != std::string::npos,
               "#2591: runtime-host quit prompt should preserve the en-US confirmation prompt");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: es-419 runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("Desea salir de esta aplicacion? [y/N]: ") != std::string::npos,
               "#2591: es-419 runtime-host quit prompt should localize the prompt prose");
        expect(process.stderr_text.find("Do you want to quit this application?") == std::string::npos,
               "#2591: es-419 runtime-host quit prompt should not fall back to raw English prose");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: pt-BR runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("Deseja sair deste aplicativo? [y/N]: ") != std::string::npos,
               "#2591: pt-BR runtime-host quit prompt should localize the prompt prose");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: qps-ploc runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("[!! ") != std::string::npos,
               "#2591: qps-ploc runtime-host quit prompt should pseudo-localize the prompt prose");
        expect(process.stderr_text.find("[y/N]: ") != std::string::npos,
               "#2591: qps-ploc runtime-host quit prompt should preserve confirmation tokens");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "FAIL: runtime host executable path argument is required\n";
        return 1;
    }

    test_runtime_host_supports_breakpoint_management_commands(argv[1]);
    test_runtime_host_supports_single_breakpoint_removal(argv[1]);
    test_runtime_host_prefers_debug_manifest_for_implicit_debug_launches(argv[1]);
    test_runtime_host_compatibility_launcher_note_reflects_xasset_fallback(argv[1]);
    test_runtime_host_reports_xasset_pause_identity(argv[1]);
    test_runtime_host_supports_xasset_action_breakpoint_commands(argv[1]);
    test_runtime_host_surfaces_xasset_breakpoint_metadata_in_pause_output(argv[1]);
    test_runtime_host_removes_xasset_bootstrap_after_execution(argv[1]);
    test_runtime_host_rejects_extension_payload_basename_fallback(argv[1]);
    test_runtime_host_validates_manifest_versions_without_changing_error_contracts(argv[1]);
    test_runtime_host_manifest_verification_errors_localize_without_changing_contracts(argv[1]);
    test_runtime_host_security_denial_audit_details_localize_without_changing_audit_contracts(argv[1]);
    test_runtime_host_rejects_ai_federation_planning_without_ai_permission(argv[1]);
    test_runtime_host_writes_bridge_response_artifact(argv[1]);
    test_runtime_host_invokes_zero_argument_bridge_export(argv[1]);
    test_runtime_host_removes_bridge_routine_bootstrap_after_execution(argv[1]);
    test_runtime_host_unescapes_bridge_descriptor_string_fields(argv[1]);
    test_runtime_host_passes_bridge_request_parameters_to_export(argv[1]);
    test_runtime_host_rejects_bridge_parameter_count_mismatch(argv[1]);
    test_runtime_host_rejects_nested_bridge_parameter_array_for_nonzero_arity(argv[1]);
    test_runtime_host_rejects_nested_bridge_parameter_values_for_nonzero_arity(argv[1]);
    test_runtime_host_rejects_bridge_parameter_name_mismatch(argv[1]);
    test_runtime_host_rejects_bridge_request_contract_mismatch(argv[1]);
    test_runtime_host_rejects_nested_bridge_descriptor_fields(argv[1]);
    test_runtime_host_rejects_bridge_descriptor_identity_mismatch(argv[1]);
    test_runtime_host_rejects_bridge_descriptor_metadata_mismatch(argv[1]);
    test_runtime_host_usage_text_localizes_without_changing_cli_tokens(argv[1]);
    test_runtime_host_debug_errors_localize_without_changing_command_tokens(argv[1]);
    test_runtime_host_pause_messages_localize_without_changing_pause_reasons(argv[1]);
    test_runtime_host_watch_errors_localize_without_changing_watch_fields(argv[1]);
    test_runtime_host_quit_prompt_localizes_without_changing_confirmation_tokens(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }

    std::cout << "All runtime host debug-output tests passed\n";
    return 0;
}

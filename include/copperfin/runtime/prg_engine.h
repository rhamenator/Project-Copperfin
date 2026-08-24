// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/rushmore_planning.h"

#include <functional>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::runtime {

enum class PrgValueKind {
    empty,
    boolean,
    number,
    string,
    int64,
    uint64,
    currency
};

enum class PrgStringFlavor {
    none,
    date,
    datetime
};

struct PrgValue {
    PrgValueKind kind = PrgValueKind::empty;
    bool is_null = false;
    bool boolean_value = false;
    double number_value = 0.0;
    std::string string_value;
    PrgStringFlavor string_flavor = PrgStringFlavor::none;
    std::int64_t int64_value = 0;
    std::uint64_t uint64_value = 0;
    // VFP Currency is a signed 64-bit integer scaled by 10,000.
    std::int64_t currency_value = 0;
};

struct SourceLocation {
    std::string file_path;
    std::size_t line = 0;
};

struct RuntimeBreakpoint {
    std::string file_path;
    std::size_t line = 0;
};

enum class DebugResumeAction {
    continue_run,
    step_into,
    step_over,
    step_out
};

enum class DebugPauseReason {
    none,
    entry,
    breakpoint,
    step,
    event_loop,
    completed,
    error
};

struct RuntimeStackFrame {
    std::string file_path;
    std::string routine_name;
    std::size_t line = 0;
    std::map<std::string, PrgValue> locals;
};

struct RuntimeEvent {
    std::string category;
    std::string detail;
    SourceLocation location{};
};

enum class DiagnosticSeverity {
    info,
    warning,
    error
};

struct PrgStaticDiagnostic {
    std::string code;
    DiagnosticSeverity severity = DiagnosticSeverity::warning;
    std::string message;
    SourceLocation location{};
};

struct RuntimeWorkAreaState {
    int selected = 1;
    int data_session = 1;
    std::map<int, std::string> aliases;
};

struct RuntimeCursorState {
    int work_area = 0;
    std::string alias;
    std::string source_path;
    std::string source_kind;
    std::string filter_expression;
    bool remote = false;
    std::size_t record_count = 0;
    std::size_t recno = 0;
    bool bof = true;
    bool eof = true;
};

struct RuntimeDatabaseState {
    std::string path;
    std::string name;
    bool exclusive = false;
    bool read_only = false;
    bool current = false;
};

struct RuntimeSqlConnectionState {
    int handle = 0;
    std::string target{};
    std::string provider{};
    std::string last_cursor_alias{};
    std::size_t last_result_count = 0;
    std::string prepared_command{};
    std::string last_sql_action{};
    bool transaction_dirty = false;
    bool cancel_requested = false;
    std::map<std::string, std::string> properties{};
};

enum class NativeMemberVisibility {
    public_member,
    protected_member,
    hidden_member
};

struct RuntimeOleObjectState {
    int handle = 0;
    std::string prog_id{};
    std::string source{};
    std::string activation_computer_name{};
    std::string requested_interface_id{};
    std::string last_action{};
    int action_count = 0;
    bool hidden_runtime_surface = false;
    bool read_only_collection_surface = false;
    std::optional<std::intptr_t> native_hwnd{};
    std::map<std::string, PrgValue> properties{};
    std::map<std::string, PrgValue> default_properties{};
    std::optional<PrgValueKind> controlsource_value_kind_hint{};
    // Records an explicit true-to-false BoundTo transition so the selected
    // Value can return to one-based index semantics without changing the
    // default behavior of character-valued unbound lists.
    bool boundto_index_value_mode = false;
    std::vector<PrgValue> collection_items{};
    std::vector<std::string> collection_item_keys{};
    std::vector<std::vector<PrgValue>> list_rows{};
    std::vector<PrgValue> list_item_data{};
    std::vector<bool> list_selected{};
    std::vector<std::string> class_hierarchy{};
    std::map<std::string, NativeMemberVisibility> member_visibility{};
    std::map<std::string, std::string> member_visibility_owner{};
    std::string base_class_name{};
    std::string class_library{};
    std::vector<std::string> methods{};
    std::vector<std::string> events{};
};

// Host-owned admission metadata for a locally connected COM event source. The
// runtime never discovers, activates, or infers this identity from a PRG
// object; a host must explicitly supply it for a source it owns.
struct RuntimeComEventSourceAdmission {
    std::string source_identity{};
    std::string handler_interface_id{};
    std::vector<std::string> required_handler_methods{};
};

struct RuntimePauseState {
    bool paused = false;
    bool completed = false;
    bool waiting_for_events = false;
    DebugPauseReason reason = DebugPauseReason::none;
    SourceLocation location{};
    std::string statement_text;
    std::string message;
    std::vector<RuntimeStackFrame> call_stack;
    std::map<std::string, PrgValue> globals;
    std::optional<PrgValue> last_return_value;
    std::vector<RuntimeEvent> events;
    RuntimeWorkAreaState work_area{};
    std::vector<RuntimeCursorState> cursors;
    std::vector<RuntimeDatabaseState> databases;
    std::vector<RuntimeSqlConnectionState> sql_connections;
    std::vector<RuntimeOleObjectState> ole_objects;
    std::size_t executed_statement_count = 0;
};

struct RuntimeWatchResult {
    std::string expression;
    bool ok = false;
    PrgValue value{};
    std::string message;
};

enum class RuntimeKeyboardCompatibility {
    windows,
    dos,
};

enum class RuntimePolyglotDispatchStatus {
    success,
    invalid_request,
    native_failed,
    candidate_failed,
    cancelled,
    parity_failed,
};

enum class RuntimePolyglotDispatchAuthority {
    none,
    native,
    candidate,
};

enum class RuntimePolyglotDispatchSelection {
    none,
    native,
    shadow,
    candidate,
};

struct RuntimePolyglotDispatchRequest {
    std::string capability_id;
    std::string arguments_json;
    std::uint8_t selection_sample = 0U;
    // The host may pass this non-owning, read-only probe into a bounded bridge
    // operation. It never exposes mutable PRG runtime state.
    std::function<bool()> cancellation_requested;
};

struct RuntimePolyglotDispatchResult {
    RuntimePolyglotDispatchStatus status =
        RuntimePolyglotDispatchStatus::invalid_request;
    std::string error_code;
    RuntimePolyglotDispatchAuthority authority =
        RuntimePolyglotDispatchAuthority::none;
    RuntimePolyglotDispatchSelection selection =
        RuntimePolyglotDispatchSelection::none;
    std::uint32_t native_invocation_count = 0U;
    std::uint32_t candidate_invocation_count = 0U;
    bool native_fallback_executed = false;
    // One complete bounded JSON value. Empty means JSON null.
    std::string payload_json;
};

struct RuntimeSessionOptions {
    std::string startup_path;
    // Optional host-selected catalog. Null keeps the legacy environment lookup.
    std::shared_ptr<const localization::LocalizedCatalog> localization_catalog;
    // Parses trusted startup bytes under startup_path's logical source identity.
    std::optional<std::string> startup_source_text;
    std::map<std::string, std::string> source_text_overrides;
    bool require_source_text_overrides = false;
    // Immutable non-source package bytes admitted by the runtime host.
    std::map<std::string, std::string> verified_file_byte_overrides;
    bool require_verified_file_byte_overrides = false;
    std::map<std::string, std::string> source_path_display_aliases;
    std::string working_directory;
    bool stop_on_entry = false;
    std::size_t max_call_depth = 1024;
    std::size_t max_executed_statements = 500000;
    std::size_t max_loop_iterations = 200000;
    std::string temp_directory;
    std::size_t scheduler_yield_statement_interval = 4096;
    std::size_t scheduler_yield_sleep_ms = 1;
    // CODEPAGE is a CONFIG.FPW startup setting in VFP9, not a live SET command.
    // 0 is reserved for an explicitly invalid CONFIG.FPW CODEPAGE so consumers
    // such as ISLEADBYTE() fail closed instead of inheriting host DBCS state.
    std::optional<int> configured_code_page;
    // Windows is the only modeled OptionGroup keyboard contract. DOS remains
    // explicit and unsupported until its distinct navigation semantics land.
    RuntimeKeyboardCompatibility keyboard_compatibility = RuntimeKeyboardCompatibility::windows;
    RushmorePlanningOptions rushmore_planning{};
    // Optional host-owned bridge for CFPOLYGLOTDISPATCH(). The runtime invokes
    // it synchronously on the current PRG task with immutable request data.
    // Null fails closed; the callback receives no runtime/session reference.
    std::function<RuntimePolyglotDispatchResult(
        const RuntimePolyglotDispatchRequest&)> polyglot_dispatch_callback;
    // Optional host-owned printer enumeration seam. Null uses the platform
    // implementation; tests and embedded hosts may supply deterministic names.
    std::function<std::vector<std::string>()> printer_enumeration_callback;
    // Optional host-owned EVENTHANDLER admission seam. A null callback, empty
    // identity, interface identity, or incomplete handler-method contract fails closed. The
    // callback must not perform activation, registry probing, or network I/O.
    std::function<std::optional<RuntimeComEventSourceAdmission>(
        const RuntimeOleObjectState&)> com_event_source_admission_callback;
    // Called when QUIT executes. Return true to allow quit; false to cancel it
    // (e.g. show a dialog asking the user to confirm). Null means always quit.
    std::function<bool()> quit_confirm_callback;
};

class PrgRuntimeSession {
public:
    static PrgRuntimeSession create(const RuntimeSessionOptions& options);

    PrgRuntimeSession(PrgRuntimeSession&&) noexcept;
    PrgRuntimeSession& operator=(PrgRuntimeSession&&) noexcept;
    ~PrgRuntimeSession();

    void add_breakpoint(const RuntimeBreakpoint& breakpoint);
    [[nodiscard]] bool remove_breakpoint(const RuntimeBreakpoint& breakpoint);
    void clear_breakpoints();
    [[nodiscard]] std::vector<RuntimeBreakpoint> list_breakpoints() const;
    [[nodiscard]] bool dispatch_event_handler(const std::string& routine_name);
    // Delivers one host-normalized VFP key label to an active READ EVENTS
    // session. This is a headless runtime seam; native keyboard capture and
    // form-local KeyPress routing remain host/UI responsibilities.
    [[nodiscard]] bool dispatch_key_label(const std::string& key_label);
    // Delivers an Escape request to an active READ EVENTS session.  ON ESCAPE
    // has priority over an ON KEY LABEL ESC assignment when SET ESCAPE is ON.
    [[nodiscard]] bool dispatch_escape();
    [[nodiscard]] bool dispatch_popup_bar_selection(
        const std::string& popup_name,
        std::int64_t bar_number);
    [[nodiscard]] std::optional<std::intptr_t> dispatch_windows_message(
        std::intptr_t hwnd,
        std::uint32_t message,
        std::intptr_t wparam = 0,
        std::intptr_t lparam = 0);
    [[nodiscard]] bool can_undo_command() const;
    [[nodiscard]] std::string command_undo_label() const;
    [[nodiscard]] RuntimeWatchResult evaluate_watch_expression(const std::string& expression);
    void request_cancel();
    // Thread-safe host request.  The runtime consumes it at its next safe PRG
    // statement boundary; it never interrupts native code mid-statement.
    void request_escape();

    [[nodiscard]] RuntimePauseState run(DebugResumeAction action);
    [[nodiscard]] const RuntimePauseState& state() const noexcept;

private:
    struct Impl;

    explicit PrgRuntimeSession(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> impl_;
};

[[nodiscard]] const char* debug_pause_reason_name(DebugPauseReason reason);
[[nodiscard]] std::string format_value(const PrgValue& value);
[[nodiscard]] std::vector<PrgStaticDiagnostic> analyze_prg_file(const std::string& path);

}  // namespace copperfin::runtime

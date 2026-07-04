// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/runtime/index_seek_optimizer.h"
#include "copperfin/platform/environment.h"
#include "localized_text.h"
#include "prg_engine_command_helpers.h"
#include "prg_engine_helpers.h"
#include "prg_engine_internal.h"
#include "prg_engine_file_io_functions.h"
#include "prg_engine_runtime_config.h"
#include "prg_engine_runtime_surface_functions.h"
#include "prg_engine_table_structure_helpers.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/report_layout.h"
#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <atomic>
#include <array>
#include <cctype>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <future>
#include <map>
#include <iterator>
#include <mutex>
#include <new>
#include <optional>
#include <set>
#include <sstream>
#include <system_error>
#include <thread>
#include <chrono>
#include <deque>
#include <utility>

#if defined(_WIN32)
#include <process.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winver.h>
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")
#pragma comment(lib, "version.lib")
#include <comdef.h>

// Minimal COM interface declarations for .NET CLR v4 hosting.
// We only need _AppDomain::Load_2, _Type::GetMethod_2, _MethodInfo::Invoke_3.
// Rather than importing the full mscorlib.tlb (which collides with SDK headers),
// we declare only what we need via IDispatch-based late binding.
// The actual CLR invocation uses IDispatch::Invoke for safety and compatibility.
#include <oaidl.h>
#else
#include <unistd.h>
#endif

namespace copperfin::runtime
{

    namespace
    {
        constexpr std::intptr_t kCopperfinVfpMainWindowHwnd = 1000;
        constexpr std::intptr_t kCopperfinScreenClientHwnd = 1001;
        constexpr std::intptr_t kCopperfinSyntheticWindowHwndBase = 100000;
        constexpr std::intptr_t kCopperfinScreenWhandle = 900001;
        constexpr std::intptr_t kCopperfinVfpWhandle = 900002;

        PrgValue canonicalize_native_olecontrol_doverb_argument(const PrgValue &verb)
        {
            if (verb.kind == PrgValueKind::string)
            {
                const std::string normalized_verb = normalize_identifier(trim_copy(value_as_string(verb)));
                if (normalized_verb == "edit")
                {
                    return make_number_value(-1.0);
                }
                if (normalized_verb == "open")
                {
                    return make_number_value(-2.0);
                }
            }

            return verb;
        }

        std::optional<PrgValue> read_native_olecontrol_objectverb_by_index(
            const RuntimeOleObjectState &runtime_object,
            const std::vector<PrgValue> &arguments)
        {
            const bool is_olecontrol =
                normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
                normalize_identifier(runtime_object.prog_id) == "olecontrol";
            if (!is_olecontrol)
            {
                return std::nullopt;
            }

            if (arguments.empty())
            {
                return make_empty_value();
            }

            const long long index = std::llround(value_as_number(arguments.front()));
            if (index == 0)
            {
                return make_string_value("edit");
            }
            if (index == 1)
            {
                return make_string_value("open");
            }

            return make_empty_value();
        }

        bool runtime_object_member_matches(
            const std::vector<std::string> &members,
            const std::string &normalized_member_name)
        {
            return std::any_of(members.begin(), members.end(), [&](const std::string &member_name)
            {
                return normalize_identifier(member_name) == normalized_member_name;
            });
        }

        bool runtime_object_method_ends_with_suffix(
            const std::string &method_name,
            const std::string &suffix,
            std::string *stem = nullptr)
        {
            const std::string normalized_method = normalize_identifier(method_name);
            if (normalized_method.size() <= suffix.size() ||
                normalized_method.compare(normalized_method.size() - suffix.size(), suffix.size(), suffix) != 0)
            {
                return false;
            }

            if (normalized_method[normalized_method.size() - suffix.size() - 1U] != '_')
            {
                return false;
            }

            if (stem != nullptr)
            {
                *stem = normalized_method.substr(0U, normalized_method.size() - suffix.size() - 1U);
            }
            return true;
        }

        bool runtime_object_has_accessor_property(
            const RuntimeOleObjectState &runtime_object,
            const std::string &normalized_property_name)
        {
            return std::any_of(runtime_object.methods.begin(), runtime_object.methods.end(), [&](const std::string &method_name)
            {
                std::string stem;
                return runtime_object_method_ends_with_suffix(method_name, "access", &stem) &&
                       stem == normalized_property_name;
            });
        }

        bool runtime_object_has_assigner_property(
            const RuntimeOleObjectState &runtime_object,
            const std::string &normalized_property_name)
        {
            return std::any_of(runtime_object.methods.begin(), runtime_object.methods.end(), [&](const std::string &method_name)
            {
                std::string stem;
                return runtime_object_method_ends_with_suffix(method_name, "assign", &stem) &&
                       stem == normalized_property_name;
            });
        }

        bool is_native_visual_runtime_object(const RuntimeOleObjectState &runtime_object)
        {
            if (runtime_object.class_hierarchy.empty())
            {
                return false;
            }

            const std::string normalized_base_class =
                normalize_identifier(trim_copy(runtime_object.base_class_name));
            return normalized_base_class == "checkbox" ||
                   normalized_base_class == "combobox" ||
                   normalized_base_class == "commandbutton" ||
                   normalized_base_class == "commandgroup" ||
                   normalized_base_class == "container" ||
                   normalized_base_class == "editbox" ||
                   normalized_base_class == "form" ||
                   normalized_base_class == "grid" ||
                   normalized_base_class == "image" ||
                   normalized_base_class == "label" ||
                   normalized_base_class == "line" ||
                   normalized_base_class == "listbox" ||
                   normalized_base_class == "olecontrol" ||
                   normalized_base_class == "optionbutton" ||
                   normalized_base_class == "optiongroup" ||
                   normalized_base_class == "page" ||
                   normalized_base_class == "pageframe" ||
                   normalized_base_class == "separator" ||
                   normalized_base_class == "shape" ||
                   normalized_base_class == "spinner" ||
                   normalized_base_class == "textbox" ||
                   normalized_base_class == "toolbar";
        }

        bool is_native_focusable_runtime_object(const RuntimeOleObjectState &runtime_object)
        {
            if (runtime_object.class_hierarchy.empty())
            {
                return false;
            }

            const std::string normalized_base_class =
                normalize_identifier(trim_copy(runtime_object.base_class_name));
            return normalized_base_class == "checkbox" ||
                   normalized_base_class == "combobox" ||
                   normalized_base_class == "commandbutton" ||
                   normalized_base_class == "editbox" ||
                   normalized_base_class == "form" ||
                   normalized_base_class == "grid" ||
                   normalized_base_class == "listbox" ||
                   normalized_base_class == "olecontrol" ||
                   normalized_base_class == "optionbutton" ||
                   normalized_base_class == "page" ||
                   normalized_base_class == "spinner" ||
                   normalized_base_class == "textbox";
        }

        bool is_builtin_native_noarg_method_name(
            const RuntimeOleObjectState &runtime_object,
            const std::string &normalized_member_name)
        {
            if (normalized_member_name == "release" &&
                !runtime_object.source.empty())
            {
                return true;
            }

            if (normalized_member_name == "refresh")
            {
                return !runtime_object.class_hierarchy.empty();
            }

            if ((normalized_member_name == "show" || normalized_member_name == "hide") &&
                is_native_visual_runtime_object(runtime_object))
            {
                return true;
            }

            if (normalized_member_name == "setfocus" &&
                is_native_focusable_runtime_object(runtime_object))
            {
                return true;
            }

            return false;
        }

        void ensure_fault_context_defaults(
            const Statement *statement,
            SourceLocation &last_fault_location,
            std::string &last_fault_statement)
        {
            if (statement != nullptr)
            {
                if (last_fault_location.file_path.empty())
                {
                    last_fault_location = statement->location;
                }
                if (last_fault_statement.empty())
                {
                    last_fault_statement = statement->text;
                }
            }
        }

        struct LoopState
        {
            std::size_t for_statement_index = 0;
            std::size_t endfor_statement_index = 0;
            std::string variable_name{};
            double end_value = 0.0;
            double step_value = 1.0;
            std::size_t iteration_count = 0;
            // FOR EACH support
            bool is_for_each = false;
            std::vector<PrgValue> each_values{}; // snapshot of collection at entry
            std::size_t each_index = 0;
        };

        struct ScanState
        {
            std::size_t scan_statement_index = 0;
            std::size_t endscan_statement_index = 0;
            int work_area = 0;
            std::string for_expression;
            std::string while_expression;
            std::size_t iteration_count = 0;
        };

        struct WhileState
        {
            std::size_t do_while_statement_index = 0;
            std::size_t enddo_statement_index = 0;
            std::size_t iteration_count = 0;
        };

        struct CaseState
        {
            std::size_t do_case_statement_index = 0;
            std::size_t endcase_statement_index = 0;
            bool matched = false;
        };

        struct WithState
        {
            PrgValue target;
            std::string binding_name;
        };

        struct TryState
        {
            std::size_t try_statement_index = 0;
            std::vector<std::size_t> catch_statement_indices;
            std::optional<std::size_t> finally_statement_index;
            std::size_t endtry_statement_index = 0;
            bool handling_error = false;
            bool entered_catch = false;
            bool entered_finally = false;
            bool propagate_after_finally = false;
        };

        struct Frame
        {
            std::string file_path;
            std::string routine_name;
            const Routine *routine = nullptr;
            std::size_t pc = 0;
            std::map<std::string, PrgValue> locals;
            std::vector<PrgValue> call_arguments;
            std::vector<std::optional<std::string>> call_argument_references;
            std::map<std::string, std::string> parameter_reference_bindings;
            std::set<std::string> local_names;
            std::map<std::string, std::optional<PrgValue>> private_saved_values;
            std::vector<LoopState> loops;
            std::vector<ScanState> scans;
            std::vector<WhileState> whiles;
            std::vector<CaseState> cases;
            std::vector<WithState> withs;
            std::vector<TryState> tries;
            std::string native_method_class_name;
            std::string native_method_name;
            bool requested_nodefault = false;
            bool evaluate_conditional_else = false;
        };

        struct ExecutionOutcome
        {
            bool ok = true;
            bool waiting_for_events = false;
            bool frame_returned = false;
            std::string message;
        };

        struct CursorState
        {
            struct OrderState
            {
                std::string name;
                std::string expression;
                std::string for_expression;
                std::string index_path;
                std::string normalization_hint;
                std::string collation_hint;
                std::string key_domain_hint;
                bool descending = false;
            };

            struct FieldRule
            {
                bool nullable = true;
                bool has_default = false;
                std::string default_expression;
            };

            int work_area = 0;
            std::string alias;
            std::string source_path;
            std::string dbf_identity;
            std::string source_kind;
            bool remote = false;
            bool exclusive = false;
            std::size_t field_count = 0;
            std::size_t record_count = 0;
            std::size_t record_length = 0;
            std::size_t recno = 0;
            bool found = false;
            bool bof = true;
            bool eof = true;
            std::vector<OrderState> orders;
            std::string active_order_name;
            std::string active_order_expression;
            std::string active_order_for_expression;
            std::string active_order_path;
            std::string active_order_normalization_hint;
            std::string active_order_collation_hint;
            std::string active_order_key_domain_hint;
            bool active_order_descending = false;
            std::string filter_expression;
            std::vector<vfp::DbfRecord> remote_records;
            std::vector<vfp::DbfFieldDescriptor> remote_fields;
            std::map<std::string, FieldRule> field_rules;
        };

        struct IndexedCandidate
        {
            std::string key;
            std::size_t recno = 0;
        };

        struct CursorPositionSnapshot
        {
            std::size_t recno = 0;
            bool found = false;
            bool bof = true;
            bool eof = true;
            std::string active_order_name;
            std::string active_order_expression;
            std::string active_order_for_expression;
            std::string active_order_path;
            std::string active_order_normalization_hint;
            std::string active_order_collation_hint;
            std::string active_order_key_domain_hint;
            bool active_order_descending = false;
        };

        struct RegisteredApiFunction
        {
            int handle = 0;
            std::string variant;
            std::string function_name;
            std::string argument_types;
            std::string return_type;
            std::string dll_name;
        };

        struct DeclaredDllFunction
        {
            std::string alias;         // Name used in PRG code (may equal function_name)
            std::string function_name; // Actual export name in DLL
            std::string dll_path;      // Path to DLL/FLL/.NET assembly
            std::string return_type;   // e.g. "INTEGER", "STRING", "DOUBLE", etc.
            std::string param_types;   // Comma-separated param types
            bool is_dotnet = false;
#if defined(_WIN32)
            HMODULE hmodule = nullptr;
            FARPROC proc_address = nullptr;
#endif
            // .NET-specific (assembly!Namespace.Type.Method)
            std::string dotnet_type_name;
            std::string dotnet_method_name;
        };

        struct DataSessionState
        {
            int selected_work_area = 1;
            int next_work_area = 1;
            std::map<int, std::string> aliases;
            std::map<int, CursorState> cursors;
            std::set<int> table_locks;
            std::map<int, std::set<std::size_t>> record_locks;
            std::vector<std::string> key_stack;
            std::vector<std::string> menu_stack;
            std::vector<std::string> popup_stack;
        };

        struct RuntimeArray
        {
            std::size_t rows = 0;
            std::size_t columns = 1;
            std::vector<PrgValue> values;
        };

        struct TransactionJournalFileEntry
        {
            std::string original_path;
            std::string backup_path;
            bool existed_at_start = false;
        };

        struct TransactionJournalState
        {
            int level = 0;
            std::filesystem::path root_path;
            std::filesystem::path journal_path;
            std::string command_label;
            std::map<std::string, TransactionJournalFileEntry> tracked_files;
        };

        struct AsyncTaskState
        {
            int handle = 0;
            std::string routine_name;
            std::string source_path;
            std::shared_ptr<std::atomic<bool>> cancel_requested;
            std::shared_future<RuntimePauseState> future;
            bool finished = false;
            RuntimePauseState result{};
        };

        struct RuntimeConcurrencyState
        {
            std::mutex mutex;
            std::map<int, std::map<int, std::shared_ptr<AsyncTaskState>>> async_tasks_by_session;
            std::map<int, int> next_async_task_handle_by_session;
            std::map<std::string, std::shared_ptr<std::recursive_mutex>> critical_sections;
            std::map<std::string, std::string> table_lock_owner_by_resource;
            std::map<std::string, std::map<std::size_t, std::string>> record_lock_owner_by_resource;
        };

        struct CurrentNativeEventContext
        {
            int source_handle = 0;
            std::string event_name;
            int event_type = 0;
        };

        struct CurrentWindowMessageContext
        {
            int window_handle = 0;
            int message = 0;
            std::intptr_t wparam = 0;
            std::intptr_t lparam = 0;
        };

#include "prg_engine_free_functions.inl"
    } // namespace

    struct PrgRuntimeSession::Impl
    {
        explicit Impl(RuntimeSessionOptions session_options)
            : options(std::move(session_options))
        {
            static std::atomic<std::uint64_t> runtime_instance_counter{1ULL};
            max_call_depth = std::max<std::size_t>(1U, options.max_call_depth);
            max_executed_statements = std::max<std::size_t>(1U, options.max_executed_statements);
            max_loop_iterations = std::max<std::size_t>(1U, options.max_loop_iterations);
            scheduler_yield_statement_interval = std::max<std::size_t>(1U, options.scheduler_yield_statement_interval);
            scheduler_yield_sleep_ms = options.scheduler_yield_sleep_ms;
            task_cancel_requested = std::make_shared<std::atomic<bool>>(false);
            runtime_instance_id = runtime_instance_counter.fetch_add(1ULL, std::memory_order_relaxed);
            concurrency_state = std::make_shared<RuntimeConcurrencyState>();
            runtime_temp_directory = choose_runtime_temp_directory(options);
        }

        struct AErrorCompatibilitySnapshot
        {
            std::string sql_detail;
            std::string sql_state;
            int sql_native_code = 0;
            bool has_sql_native_code = false;
            std::string sql_context;
            std::string sql_payload;
            std::string ole_detail;
            std::string ole_app;
            std::string ole_source;
            std::string ole_action;
            int ole_native_code = 0;
            bool has_ole_native_code = false;
            std::optional<PrgValue> thrown_user_value;
            std::optional<int> explicit_error_code;
            std::optional<PrgValue> active_exception_reference;
            bool preserve_fault_context = false;
        };

        struct FaultMetadataSnapshot
        {
            std::string message;
            SourceLocation location{};
            std::string statement;
            int code = 0;
            int work_area = 0;
            int data_session = 1;
            std::string procedure;
            AErrorCompatibilitySnapshot compatibility;
            std::optional<DataSessionState> session_state_snapshot;
        };

        struct NativeEventBinding
        {
            int source_handle = 0;
            std::string event_name;
            bool target_is_routine = false;
            std::string target_program_path;
            int target_handle = 0;
            std::string delegate_name;
            int flags = 0;
            std::size_t ordinal = 0;
        };

        struct WindowMessageBinding
        {
            int window_handle = 0;
            int message = 0;
            int target_handle = 0;
            std::string delegate_name;
            std::size_t ordinal = 0;
        };

        RuntimeSessionOptions options;
        std::map<std::string, Program> programs;
        std::deque<Frame> stack;
        std::map<std::string, PrgValue> globals;
        std::optional<PrgValue> last_return_value;
        bool last_popped_frame_requested_nodefault = false;
        std::map<std::string, RuntimeArray> arrays;
        std::set<std::string> public_names;
        std::vector<RuntimeBreakpoint> breakpoints;
        std::optional<SourceLocation> resume_skip_breakpoint_location;
        std::vector<RuntimeEvent> events;
        RuntimePauseState last_state{};
        std::string startup_default_directory;
        std::string last_error_message;
        SourceLocation last_fault_location{};
        std::string last_fault_statement;
        int last_error_code = 0;
        int last_error_work_area = 0;
        std::string last_error_procedure;
        AErrorCompatibilitySnapshot last_error_compatibility;
        std::vector<FaultMetadataSnapshot> error_metadata_stack;
        std::string error_handler;
        std::string shutdown_handler;
        std::map<int, std::map<std::string, std::string>> set_state_by_session;
        int current_data_session = 1;
        std::map<int, int> next_sql_handle_by_session;
        std::map<int, int> next_api_handle_by_session;
        std::map<int, int> transaction_level_by_session;
        std::map<int, TransactionJournalState> transaction_journal_by_session;
        std::map<int, TransactionJournalState> command_undo_journal_by_session;
        std::map<int, std::vector<TransactionJournalState>> command_undo_stack_by_session;
        std::shared_ptr<RuntimeConcurrencyState> concurrency_state;
        int next_ole_handle = 1;
        std::map<int, DataSessionState> data_sessions;
        std::map<int, std::string> default_directory_by_session;
        std::map<int, std::size_t> memowidth_by_session;
        std::map<int, std::map<int, RuntimeSqlConnectionState>> sql_connections_by_session;
        std::map<int, RuntimeOleObjectState> ole_objects;
        std::optional<int> representative_active_form_handle;
        std::optional<int> representative_application_forms_collection_handle;
        std::string representative_application_caption = "Microsoft Visual FoxPro";
        int representative_application_window_state = 0;
        std::vector<NativeEventBinding> native_event_bindings;
        std::set<std::string> active_native_event_keys;
        std::vector<CurrentNativeEventContext> active_native_event_contexts;
        std::vector<WindowMessageBinding> window_message_bindings;
        std::vector<CurrentWindowMessageContext> active_window_message_contexts;
        std::size_t next_native_event_binding_ordinal = 1U;
        std::set<std::string> loaded_libraries;
        std::map<int, std::map<int, RegisteredApiFunction>> registered_api_functions_by_session;
        std::map<std::string, DeclaredDllFunction> declared_dll_functions; // keyed by normalized alias
        bool entry_pause_pending = false;
        bool waiting_for_events = false;
        bool handling_error = false;
        bool handling_shutdown = false;
        std::optional<std::size_t> error_handler_return_depth;
        std::optional<std::size_t> shutdown_handler_return_depth;
        bool quit_pending_after_shutdown = false;
        SourceLocation pending_quit_location{};
        // Saved fault position for RETRY / RESUME
        std::string fault_frame_file_path;
        std::string fault_frame_routine_name;
        std::size_t fault_statement_index = 0U;
        bool fault_pc_valid = false;
        std::optional<std::size_t> event_dispatch_return_depth;
        bool restore_event_loop_after_dispatch = false;
        std::size_t executed_statement_count = 0;
        std::size_t max_call_depth = 1024;
        std::size_t max_executed_statements = 500000;
        std::size_t max_loop_iterations = 200000;
        std::filesystem::path runtime_temp_directory;
        std::uint64_t runtime_instance_id = 0;
        std::size_t scheduler_yield_statement_interval = 4096;
        std::size_t scheduler_yield_sleep_ms = 1;
        std::shared_ptr<std::atomic<bool>> task_cancel_requested;
        std::vector<std::string> critical_section_stack;
        std::map<std::string, std::size_t> critical_section_depth_by_name;
        std::map<std::string, std::shared_ptr<std::recursive_mutex>> critical_section_mutexes_by_name;
        std::size_t native_class_instantiation_depth = 0;

        // Index seek optimizer - pattern cache
        std::map<std::string, IndexExpressionPattern> index_pattern_cache;  // Cache analyzed patterns by expression text

#include "prg_engine_session.inl"
#include "prg_engine_cursor.inl"
#include "prg_engine_records.inl"
#include "prg_engine_index_seek.inl"
#include "prg_engine_aggregate.inl"

#include "prg_engine_dll.inl"
#include "prg_engine_sql.inl"
        PrgValue evaluate_expression(const std::string &expression, const Frame &frame);
        PrgValue evaluate_expression(const std::string &expression, const Frame &frame, const CursorState *preferred_cursor);
        std::optional<std::string> materialize_xasset_bootstrap(const std::string &asset_path, bool include_read_events);

#include "prg_engine_variables.inl"
#include "prg_engine_arrays.inl"
#define COPPERFIN_PRG_ENGINE_IMPL_CONTEXT
#include "prg_engine_flow.inl"
#undef COPPERFIN_PRG_ENGINE_IMPL_CONTEXT
        bool dispatch_event_handler(const std::string &routine_name);
        void assign_native_window_metadata(RuntimeOleObjectState &runtime_object);
        [[nodiscard]] std::optional<std::intptr_t> hwnd_from_whandle(std::intptr_t whandle) const;
        [[nodiscard]] std::optional<std::intptr_t> whandle_from_hwnd(std::intptr_t hwnd) const;
        RuntimeOleObjectState *representative_active_form_object();
        const RuntimeOleObjectState *representative_active_form_object() const;
        void note_representative_active_form(const RuntimeOleObjectState &runtime_object);
        std::vector<int> representative_application_window_handles() const;
        std::size_t representative_application_form_count() const;
        RuntimeOleObjectState *ensure_representative_application_forms_collection_object();
        bool consume_last_popped_frame_requested_nodefault();
        std::optional<std::intptr_t> dispatch_windows_message(
            std::intptr_t hwnd,
            std::uint32_t message,
            std::intptr_t wparam,
            std::intptr_t lparam);
        bool dispatch_error_handler();
        std::optional<PrgValue> invoke_expression_user_routine(
            const Frame &source_frame,
            const std::string &identifier,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        PrgValue bind_native_event(
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        PrgValue raise_native_event(
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        PrgValue unbind_native_events(
            const std::vector<PrgValue> &arguments);
        PrgValue inspect_native_events(
            const std::vector<PrgValue> &arguments,
            const std::vector<std::string> &raw_arguments);
        std::optional<PrgValue> read_native_property_if_present(
            RuntimeOleObjectState &runtime_object,
            const std::string &property_name,
            const Frame &source_frame);
        bool write_native_property_if_present(
            RuntimeOleObjectState &runtime_object,
            const std::string &property_name,
            const PrgValue &assigned_value,
            const Frame &source_frame);
        std::optional<PrgValue> invoke_native_object_method_body_if_present(
            RuntimeOleObjectState &runtime_object,
            const std::string &identifier,
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        std::optional<PrgValue> invoke_native_object_method_if_present(
            RuntimeOleObjectState &runtime_object,
            const std::string &identifier,
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        std::optional<PrgValue> invoke_native_event_delegate(
            const NativeEventBinding &binding,
            const CurrentNativeEventContext &event_context,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        std::optional<PrgValue> invoke_window_message_delegate(
            const WindowMessageBinding &binding,
            const CurrentWindowMessageContext &message_context,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        PrgValue release_native_object(
            RuntimeOleObjectState &runtime_object,
            const std::string &effective_member_path);
        std::optional<PrgValue> invoke_expression_base_method(
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        PrgValue run_expression_invoked_routine_until_return(std::size_t return_depth);
        RuntimeWatchResult evaluate_watch_expression(const std::string &expression);
        ExecutionOutcome execute_current_statement();
        RuntimePauseState run(DebugResumeAction action);
    };

#include "prg_engine_expression.inl"

    PrgValue PrgRuntimeSession::Impl::evaluate_expression(const std::string &expression, const Frame &frame)
    {
        return evaluate_expression(expression, frame, resolve_cursor_target({}));
    }

    PrgValue PrgRuntimeSession::Impl::evaluate_expression(
        const std::string &expression,
        const Frame &frame,
        const CursorState *preferred_cursor)
    {
        const std::string effective_expression = apply_with_context(expression, frame);
        ExpressionParser parser(
            effective_expression,
            frame,
            globals,
            current_default_directory(),
            current_error_message(),
            current_error_code(),
            current_error_procedure(),
            current_fault_location().line,
            error_handler,
            shutdown_handler,
            is_set_enabled("exact"),
            current_selected_work_area(),
            [this]()
            {
                return next_available_work_area();
            },
            [this](const std::string &designator)
            {
                if (designator.empty())
                {
                    return current_selected_work_area();
                }
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? 0 : cursor->work_area;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? std::string{} : cursor->alias;
            },
            [this](const std::string &designator)
            {
                return resolve_cursor_target(designator) != nullptr;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? std::string{} : cursor->dbf_identity;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? 0U : cursor->field_count;
            },
            [this](std::size_t index, const std::string &designator)
            {
                return cursor_field_name(designator, index);
            },
            [this](const std::string &field_name, std::size_t index, const std::string &designator)
            {
                return cursor_field_size(designator, field_name, index);
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? 0U : cursor->record_count;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? 0U : cursor->record_length;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? 0U : cursor->recno;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? false : cursor->found;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? true : cursor->eof;
            },
            [this](const std::string &designator)
            {
                const CursorState *cursor = resolve_cursor_target(designator);
                return cursor == nullptr ? true : cursor->bof;
            },
            [this](const std::string &function, const std::vector<std::string> &raw_arguments, const std::vector<PrgValue> &arguments)
            {
                return runtime_lock_function(function, raw_arguments, arguments);
            },
            [this, preferred_cursor](const std::string &identifier)
            {
                const CursorState *current_cursor = preferred_cursor == nullptr ? resolve_cursor_target({}) : preferred_cursor;
                return resolve_field_value(identifier, current_cursor);
            },
            [this](const std::string &name)
            {
                return has_array(name);
            },
            [this](const std::string &name, int dimension)
            {
                return array_length(name, dimension);
            },
            [this](const std::string &name, std::size_t row, std::size_t column)
            {
                return array_value(name, row, column);
            },
            [this](const std::string &function, const std::vector<std::string> &raw_arguments, const std::vector<PrgValue> &arguments)
            {
                return mutate_array_function(function, raw_arguments, arguments);
            },
            [this](const std::string &name)
            {
                return populate_error_array(name);
            },
            [this, &frame](const std::string &function_name, const std::vector<std::string> &raw_arguments)
            {
                return aggregate_function_value(function_name, raw_arguments, frame);
            },
            [this](const std::string &designator, bool include_path)
            {
                return order_function_value(designator, include_path);
            },
            [this](const std::string &index_file_name, std::size_t tag_number, const std::string &designator)
            {
                return tag_function_value(index_file_name, tag_number, designator);
            },
            [this](const std::string &search_key, bool move_pointer, const std::string &designator, const std::string &order_designator)
            {
                CursorState *cursor = resolve_cursor_target(designator);
                if (cursor == nullptr)
                {
                    return false;
                }
                const SeekFunctionOrderDesignator parsed_order = parse_seek_function_order_designator(order_designator);
                return execute_seek(
                    *cursor,
                    search_key,
                    move_pointer,
                    false,
                    parsed_order.order_designator,
                    parsed_order.descending_override);
            },
            [this](const std::string &search_key, bool move_pointer, const std::string &designator, const std::string &order_designator)
            {
                CursorState *cursor = resolve_cursor_target(designator);
                if (cursor == nullptr)
                {
                    return false;
                }
                const SeekFunctionOrderDesignator parsed_order = parse_seek_function_order_designator(order_designator);
                return execute_seek(
                    *cursor,
                    search_key,
                    move_pointer,
                    true,
                    parsed_order.order_designator,
                    parsed_order.descending_override);
            },
            [this]()
            {
                return std::string("FOXTOOLS:9.0");
            },
            [this]()
            {
                return static_cast<int>(kCopperfinScreenClientHwnd);
            },
            [this](const std::string &variant,
                   const std::string &function_name,
                   const std::string &argument_types,
                   const std::string &return_type,
                   const std::string &dll_name)
            {
                return register_api_function(variant, function_name, argument_types, return_type, dll_name);
            },
            [this](int handle, const std::vector<PrgValue> &arguments)
            {
                return call_registered_api_function(handle, arguments);
            },
            [this](const std::string &target, const std::string &provider)
            {
                return sql_connect(target, provider);
            },
            [this](int handle, const std::string &command, const std::string &cursor_alias)
            {
                return sql_exec(handle, command, cursor_alias);
            },
            [this](int handle)
            {
                return sql_disconnect(handle);
            },
            [this](int handle)
            {
                return sql_row_count(handle);
            },
            [this](int handle, const std::string &command)
            {
                return sql_prepare(handle, command);
            },
            [this](int handle)
            {
                return sql_cancel(handle);
            },
            [this](int handle)
            {
                return sql_commit(handle);
            },
            [this](int handle)
            {
                return sql_rollback(handle);
            },
            [this](int handle, const std::string &cursor_alias)
            {
                return sql_databases(handle, cursor_alias);
            },
            [this](int handle, const std::string &table_name, const std::string &cursor_alias)
            {
                return sql_primary_keys(handle, table_name, cursor_alias);
            },
            [this](int handle, const std::string &table_name, const std::string &cursor_alias)
            {
                return sql_foreign_keys(handle, table_name, cursor_alias);
            },
            [this](int handle, const std::string &table_types, const std::string &cursor_alias)
            {
                return sql_tables(handle, table_types, cursor_alias);
            },
            [this](int handle, const std::string &table_name, const std::string &format, const std::string &cursor_alias)
            {
                return sql_columns(handle, table_name, format, cursor_alias);
            },
            [this](int handle, const std::string &property_name)
            {
                return sql_get_prop(handle, property_name);
            },
            [this](int handle, const std::string &property_name, const PrgValue &value)
            {
                return sql_set_prop(handle, property_name, value);
            },
            [this, &frame](
                const std::string &prog_id,
                const std::string &source,
                const std::vector<PrgValue> &constructor_arguments,
                const std::vector<std::optional<std::string>> &constructor_argument_references)
            {
                return register_ole_object(frame, prog_id, source, constructor_arguments, constructor_argument_references);
            },
            [this, &frame](
                const std::string &base_name,
                const std::string &member_path,
                const std::vector<PrgValue> &arguments,
                const std::vector<std::optional<std::string>> &argument_references)
            {
                const auto active_form_member_tail = [](const std::string &path) -> std::optional<std::string>
                {
                    const std::size_t separator = path.find('.');
                    const std::string first_segment = trim_copy(
                        separator == std::string::npos
                            ? path
                            : path.substr(0U, separator));
                    if (normalize_identifier(first_segment) != "activeform")
                    {
                        return std::nullopt;
                    }
                    if (separator == std::string::npos)
                    {
                        return std::string{};
                    }
                    return path.substr(separator + 1U);
                };
                const auto application_forms_member_tail = [](const std::string &path) -> std::optional<std::string>
                {
                    const std::size_t separator = path.find('.');
                    const std::string first_segment = trim_copy(
                        separator == std::string::npos
                            ? path
                            : path.substr(0U, separator));
                    if (normalize_identifier(first_segment) != "forms")
                    {
                        return std::nullopt;
                    }
                    if (separator == std::string::npos)
                    {
                        return std::string{};
                    }
                    return path.substr(separator + 1U);
                };
                const Statement *statement = current_statement();
                const std::string action_text = statement == nullptr
                    ? base_name + "." + member_path + "()"
                    : statement->text;
                const auto raise_ole_fault = [&](const std::string &detail,
                                                 const std::string &source,
                                                 const std::string &message) -> PrgValue
                {
                    record_ole_aerror_context(detail,
                                              "Copperfin OLE",
                                              source,
                                              action_text,
                                              1429);
                    throw std::runtime_error(message);
                };
                ResolvedRuntimeObjectMemberPath resolved_path;
                const std::string normalized_base_name = normalize_identifier(base_name);
                if (normalized_base_name == "_vfp" || normalized_base_name == "_screen")
                {
                    if (const auto forms_tail = application_forms_member_tail(member_path);
                        forms_tail.has_value())
                    {
                        RuntimeOleObjectState *forms_collection =
                            ensure_representative_application_forms_collection_object();
                        if (forms_collection != nullptr)
                        {
                            resolved_path = forms_tail->empty()
                                ? ResolvedRuntimeObjectMemberPath{
                                      .runtime_object = forms_collection,
                                      .remaining_member_path = {}}
                                : resolve_runtime_object_member_path(
                                      forms_collection,
                                      *forms_tail);
                        }
                    }
                    if (const auto active_form_tail = active_form_member_tail(member_path);
                        resolved_path.runtime_object == nullptr &&
                            active_form_tail.has_value() &&
                            !active_form_tail->empty())
                    {
                        resolved_path = resolve_runtime_object_member_path(
                            representative_active_form_object(),
                            *active_form_tail);
                    }
                }
                if (resolved_path.runtime_object == nullptr)
                {
                    resolved_path = resolve_runtime_object_member_path(frame, base_name, member_path);
                }
                if (resolved_path.runtime_object == nullptr)
                {
                    return raise_ole_fault(base_name + "." + member_path + "()",
                                           base_name,
                                           runtime_text(
                                               "Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation",
                                               {{"targetIdentifier", base_name + "." + member_path}}));
                }

                RuntimeOleObjectState *runtime_object = resolved_path.runtime_object;
                const auto make_runtime_object_reference = [](const RuntimeOleObjectState &object_state) -> PrgValue
                {
                    return make_string_value("object:" + object_state.prog_id + "#" + std::to_string(object_state.handle));
                };
                const std::string effective_member_path =
                    resolved_path.remaining_member_path.empty()
                        ? member_path
                        : resolved_path.remaining_member_path;
                const std::string leaf = normalize_identifier(
                    effective_member_path.substr(
                        effective_member_path.rfind('.') == std::string::npos
                            ? 0U
                            : effective_member_path.rfind('.') + 1U));
                if (leaf == "addobject" && !runtime_object->source.empty() && arguments.size() >= 2U)
                {
                    const std::string child_name_text = trim_copy(value_as_string(arguments[0]));
                    const std::string child_name = normalize_identifier(child_name_text);
                    const std::string child_class = trim_copy(value_as_string(arguments[1]));
                    if (child_name.empty() || child_class.empty())
                    {
                        return make_boolean_value(false);
                    }

                    const std::string child_library =
                        arguments.size() >= 3U ? trim_copy(value_as_string(arguments[2])) : std::string{};
                    const bool explicit_native_prg_library =
                        lowercase_copy(std::filesystem::path(child_library).extension().string()) == ".prg";
                    const std::string implicit_child_program_path =
                        frame.native_method_class_name.empty()
                            ? runtime_object->source
                            : frame.file_path;
                    const std::size_t constructor_start_index = explicit_native_prg_library ? 3U : 2U;
                    std::vector<PrgValue> child_constructor_arguments;
                    std::vector<std::optional<std::string>> child_argument_references;
                    child_constructor_arguments.reserve(arguments.size() > constructor_start_index ? arguments.size() - constructor_start_index : 0U);
                    child_argument_references.reserve(argument_references.size() > constructor_start_index ? argument_references.size() - constructor_start_index : 0U);
                    for (std::size_t index = constructor_start_index; index < arguments.size(); ++index)
                    {
                        child_constructor_arguments.push_back(arguments[index]);
                        child_argument_references.push_back(
                            index < argument_references.size()
                                ? argument_references[index]
                                : std::optional<std::string>{});
                    }

                    const std::string primary_child_program_path =
                        explicit_native_prg_library
                            ? resolve_native_prg_program_path(child_library, implicit_child_program_path)
                            : implicit_child_program_path;
                    RuntimeOleObjectState *child_object = instantiate_native_class_object(
                        frame,
                        child_class,
                        primary_child_program_path,
                        "addobject",
                        child_constructor_arguments,
                        child_argument_references,
                        make_runtime_object_reference(*runtime_object));
                    if (child_object == nullptr && !explicit_native_prg_library)
                    {
                        const std::string owner_program_path = normalize_path(runtime_object->source);
                        if (!owner_program_path.empty() &&
                            owner_program_path != normalize_path(primary_child_program_path))
                        {
                            child_object = instantiate_native_class_object(
                                frame,
                                child_class,
                                owner_program_path,
                                "addobject",
                                child_constructor_arguments,
                                child_argument_references,
                                make_runtime_object_reference(*runtime_object));
                        }
                    }
                    if (child_object == nullptr)
                    {
                        return make_boolean_value(false);
                    }

                    assign_native_runtime_object_name(*child_object, child_name_text);
                    runtime_object->properties[child_name] = make_runtime_object_reference(*child_object);
                    if (child_object->properties.contains("columnorder"))
                    {
                        (void)write_native_columnorder_property(
                            *child_object,
                            child_object->properties["columnorder"]);
                    }
                    if (is_native_column_runtime_object(*runtime_object) &&
                        native_column_bound_value(*runtime_object))
                    {
                        sync_native_column_child_controlsources(*runtime_object);
                    }
                    (void)sync_native_owned_children_collection(*runtime_object);
                    runtime_object->last_action = effective_member_path + "(" + child_name + "," + child_class + ")";
                    ++runtime_object->action_count;
                    events.push_back({.category = "prg.object.addobject",
                                      .detail = runtime_object->prog_id + "." + child_name + ":" + child_class,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    return make_boolean_value(true);
                }
                if (leaf == "removeobject" && !runtime_object->source.empty() && !arguments.empty())
                {
                    const std::string child_name = normalize_identifier(trim_copy(value_as_string(arguments[0])));
                    if (child_name.empty())
                    {
                        return make_boolean_value(false);
                    }

                    const auto child_property = runtime_object->properties.find(child_name);
                    if (child_property == runtime_object->properties.end())
                    {
                        return make_boolean_value(false);
                    }

                    const auto child_object = resolve_ole_object(child_property->second);
                    if (!child_object.has_value())
                    {
                        return make_boolean_value(false);
                    }
                    if ((*child_object)->hidden_runtime_surface)
                    {
                        return make_boolean_value(false);
                    }

                    const auto child_parent = native_object_parent_reference(**child_object);
                    int parent_handle = 0;
                    std::string parent_prog_id;
                    if (!child_parent.has_value() ||
                        !parse_object_handle_reference(*child_parent, parent_handle, parent_prog_id) ||
                        parent_handle != runtime_object->handle)
                    {
                        return make_boolean_value(false);
                    }

                    (*child_object)->properties.erase("parent");
                    runtime_object->properties.erase(child_name);
                    (void)sync_native_owned_children_collection(*runtime_object);
                    runtime_object->last_action = effective_member_path + "(" + child_name + ")";
                    ++runtime_object->action_count;
                    events.push_back({.category = "prg.object.removeobject",
                                      .detail = runtime_object->prog_id + "." + child_name,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    return make_boolean_value(true);
                }
                if (leaf == "setall" && !runtime_object->source.empty())
                {
                    return apply_native_setall(
                        *runtime_object,
                        frame,
                        effective_member_path,
                        arguments);
                }
                if (leaf == "release" && !runtime_object->source.empty())
                {
                    last_popped_frame_requested_nodefault = false;
                    if (auto native_result = invoke_native_object_method_if_present(
                            *runtime_object,
                            leaf,
                            frame,
                            arguments,
                            argument_references);
                        native_result.has_value())
                    {
                        if (consume_last_popped_frame_requested_nodefault())
                        {
                            return *native_result;
                        }
                    }
                    return release_native_object(*runtime_object, effective_member_path);
                }
                if (auto native_result = invoke_native_object_method_if_present(
                        *runtime_object,
                        leaf,
                        frame,
                        arguments,
                        argument_references);
                            native_result.has_value())
                {
                    return *native_result;
                }
                if (leaf == "move" &&
                    is_native_visual_runtime_object(*runtime_object))
                {
                    if (arguments.empty())
                    {
                        return make_empty_value();
                    }

                    const bool left_written = write_native_property_if_present(
                        *runtime_object,
                        "left",
                        make_number_value(value_as_number(arguments[0])),
                        frame);
                    if (!left_written)
                    {
                        return make_empty_value();
                    }

                    if (arguments.size() >= 2U)
                    {
                        (void)write_native_property_if_present(
                            *runtime_object,
                            "top",
                            make_number_value(value_as_number(arguments[1])),
                            frame);
                    }
                    if (arguments.size() >= 3U)
                    {
                        (void)write_native_property_if_present(
                            *runtime_object,
                            "width",
                            make_number_value(value_as_number(arguments[2])),
                            frame);
                    }
                    if (arguments.size() >= 4U)
                    {
                        (void)write_native_property_if_present(
                            *runtime_object,
                            "height",
                            make_number_value(value_as_number(arguments[3])),
                            frame);
                    }

                    runtime_object->last_action = effective_member_path + "()";
                    ++runtime_object->action_count;
                    events.push_back({.category = "prg.object.move",
                                      .detail = runtime_object->prog_id + "." + effective_member_path,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    return make_empty_value();
                }
                if (leaf == "refresh" && !runtime_object->class_hierarchy.empty())
                {
                    runtime_object->last_action = effective_member_path + "()";
                    ++runtime_object->action_count;
                    events.push_back({.category = "prg.object.refresh",
                                      .detail = runtime_object->prog_id + "." + effective_member_path,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    return make_empty_value();
                }
                if ((leaf == "show" || leaf == "hide") &&
                    is_native_visual_runtime_object(*runtime_object))
                {
                    const bool visible = leaf == "show";
                    (void)write_native_property_if_present(
                        *runtime_object,
                        "visible",
                        make_boolean_value(visible),
                        frame);
                    if (visible)
                    {
                        note_representative_active_form(*runtime_object);
                    }
                    runtime_object->last_action = effective_member_path + "()";
                    ++runtime_object->action_count;
                    events.push_back({.category = visible ? "prg.object.show" : "prg.object.hide",
                                      .detail = runtime_object->prog_id + "." + effective_member_path,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    return make_empty_value();
                }
                if (leaf == "setfocus" &&
                    is_native_focusable_runtime_object(*runtime_object))
                {
                    const PrgValue runtime_object_reference =
                        make_string_value("object:" + runtime_object->prog_id + "#" + std::to_string(runtime_object->handle));
                    note_representative_active_form(*runtime_object);
                    if (const auto owner_form_reference = native_object_owner_form_reference(*runtime_object);
                        owner_form_reference.has_value())
                    {
                        if (auto owner_form = resolve_ole_object(*owner_form_reference);
                            owner_form.has_value())
                        {
                            (void)write_native_property_if_present(
                                **owner_form,
                                "activecontrol",
                                runtime_object_reference,
                                frame);
                        }
                    }
                    else if (normalize_identifier(trim_copy(runtime_object->base_class_name)) == "form")
                    {
                        (void)write_native_property_if_present(
                            *runtime_object,
                            "activecontrol",
                            runtime_object_reference,
                            frame);
                    }
                    runtime_object->last_action = effective_member_path + "()";
                    ++runtime_object->action_count;
                    events.push_back({.category = "prg.object.setfocus",
                                      .detail = runtime_object->prog_id + "." + effective_member_path,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    return make_empty_value();
                }
                if (leaf == "resettodefault" && !runtime_object->class_hierarchy.empty())
                {
                    if (arguments.empty())
                    {
                        return make_boolean_value(false);
                    }

                    const std::string property_name = trim_copy(value_as_string(arguments.front()));
                    const std::string normalized_property_name = normalize_identifier(property_name);
                    if (normalized_property_name.empty())
                    {
                        return make_boolean_value(false);
                    }

                    const auto default_value = runtime_object->default_properties.find(normalized_property_name);
                    if (default_value == runtime_object->default_properties.end())
                    {
                        return make_boolean_value(false);
                    }

                    const bool restored = write_native_property_if_present(
                        *runtime_object,
                        property_name,
                        default_value->second,
                        frame);
                    if (!restored)
                    {
                        return make_boolean_value(false);
                    }

                    runtime_object->last_action = effective_member_path + "(" + property_name + ")";
                    ++runtime_object->action_count;
                    events.push_back({.category = "prg.object.resettodefault",
                                      .detail = runtime_object->prog_id + "." + normalized_property_name,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    return make_boolean_value(true);
                }
                if (is_native_olecontrol_host_object(*runtime_object) && leaf == "doverb")
                {
                    RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(*runtime_object);
                    if (object_surface == nullptr)
                    {
                        return make_boolean_value(false);
                    }

                    const PrgValue verb = arguments.empty()
                                              ? make_number_value(0.0)
                                              : canonicalize_native_olecontrol_doverb_argument(arguments.front());
                    runtime_object->last_action = effective_member_path + "(" + format_value(verb) + ")";
                    ++runtime_object->action_count;
                    events.push_back({.category = "ole.invoke",
                                      .detail = runtime_object->prog_id + "." + effective_member_path + ":" + format_value(verb),
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    object_surface->last_action = "activate:" + format_value(verb);
                    ++object_surface->action_count;
                    return make_boolean_value(true);
                }
                if (is_native_olecontrol_host_object(*runtime_object) && leaf == "objectverbs")
                {
                    return read_native_olecontrol_objectverb_by_index(*runtime_object, arguments).value_or(make_empty_value());
                }
                if (is_native_olecontrol_host_object(*runtime_object))
                {
                    RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(*runtime_object);
                    if (object_surface != nullptr)
                    {
                        if (auto nested_native_result = invoke_native_object_method_if_present(
                                *object_surface,
                                leaf,
                                frame,
                                arguments,
                                argument_references);
                            nested_native_result.has_value())
                        {
                            return *nested_native_result;
                        }
                        runtime_object = object_surface;
                    }
                }

                runtime_object->last_action = effective_member_path + "()";
                ++runtime_object->action_count;
                events.push_back({.category = "ole.invoke",
                                  .detail = runtime_object->prog_id + "." + effective_member_path,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                if (auto collection_result = invoke_native_collection_method(*runtime_object, leaf, arguments);
                    collection_result.has_value())
                {
                    return *collection_result;
                }
                if (normalize_identifier(runtime_object->prog_id) == "scripting.dictionary")
                {
                    auto update_dictionary_count = [&]()
                    {
                        std::size_t entry_count = 0U;
                        for (const auto &[property_name, property_value] : runtime_object->properties)
                        {
                            if (property_name != "count" && property_name != "comparemode")
                            {
                                ++entry_count;
                            }
                        }
                        runtime_object->properties["count"] = make_number_value(static_cast<double>(entry_count));
                    };
                    const auto key_for_argument = [&](std::size_t index) -> std::string
                    {
                        return index < arguments.size()
                            ? normalize_identifier(trim_copy(value_as_string(arguments[index])))
                            : std::string{};
                    };

                    if (leaf == "add" && arguments.size() >= 2U)
                    {
                        const std::string key = key_for_argument(0U);
                        if (!key.empty())
                        {
                            runtime_object->properties[key] = arguments[1];
                            update_dictionary_count();
                        }
                        return make_boolean_value(true);
                    }
                    if (leaf == "exists" && !arguments.empty())
                    {
                        const std::string key = key_for_argument(0U);
                        return make_boolean_value(!key.empty() && runtime_object->properties.contains(key));
                    }
                    if (leaf == "item" && !arguments.empty())
                    {
                        const std::string key = key_for_argument(0U);
                        const auto found = runtime_object->properties.find(key);
                        return found == runtime_object->properties.end() ? make_empty_value() : found->second;
                    }
                    if (leaf == "remove" && !arguments.empty())
                    {
                        const std::string key = key_for_argument(0U);
                        if (!key.empty())
                        {
                            runtime_object->properties.erase(key);
                            update_dictionary_count();
                        }
                        return make_boolean_value(true);
                    }
                    if (leaf == "removeall")
                    {
                        const auto comparemode = runtime_object->properties.find("comparemode");
                        const PrgValue comparemode_value = comparemode == runtime_object->properties.end()
                            ? make_number_value(0.0)
                            : comparemode->second;
                        runtime_object->properties.clear();
                        runtime_object->properties["comparemode"] = comparemode_value;
                        runtime_object->properties["count"] = make_number_value(0.0);
                        return make_boolean_value(true);
                    }

                    return raise_ole_fault(runtime_object->prog_id + "." + effective_member_path + "()",
                                           base_name,
                                           runtime_text(
                                               "Runtime.Prg.Core.Error.OleMemberNotFoundForMethodInvocation",
                                               {{"memberIdentifier", runtime_object->prog_id + "." + effective_member_path}}));
                }

                if (leaf == "add" || leaf == "create" || leaf == "open" || leaf == "item")
                {
                    return make_string_value("object:" + runtime_object->prog_id + "." + effective_member_path + "#" + std::to_string(runtime_object->handle));
                }
                if (arguments.empty())
                {
                    return make_string_value("ole:" + runtime_object->prog_id + "." + effective_member_path);
                }
                return arguments.front();
            },
            [this, &frame](const std::string &property_path)
            {
                const auto active_form_member_tail = [](const std::string &path) -> std::optional<std::string>
                {
                    const std::size_t separator = path.find('.');
                    if (separator == std::string::npos)
                    {
                        return std::nullopt;
                    }

                    const std::string base_name = trim_copy(path.substr(0U, separator));
                    const std::string normalized_base_name = normalize_identifier(base_name);
                    if (normalized_base_name != "_vfp" && normalized_base_name != "_screen")
                    {
                        return std::nullopt;
                    }

                    const std::string member_path = path.substr(separator + 1U);
                    const std::size_t member_separator = member_path.find('.');
                    const std::string first_segment = trim_copy(
                        member_separator == std::string::npos
                            ? member_path
                            : member_path.substr(0U, member_separator));
                    if (normalize_identifier(first_segment) != "activeform")
                    {
                        return std::nullopt;
                    }
                    if (member_separator == std::string::npos)
                    {
                        return std::string{};
                    }
                    return member_path.substr(member_separator + 1U);
                };
                const auto application_forms_member_tail = [](const std::string &path) -> std::optional<std::string>
                {
                    const std::size_t separator = path.find('.');
                    if (separator == std::string::npos)
                    {
                        return std::nullopt;
                    }

                    const std::string base_name = trim_copy(path.substr(0U, separator));
                    const std::string normalized_base_name = normalize_identifier(base_name);
                    if (normalized_base_name != "_vfp" && normalized_base_name != "_screen")
                    {
                        return std::nullopt;
                    }

                    const std::string member_path = path.substr(separator + 1U);
                    const std::size_t member_separator = member_path.find('.');
                    const std::string first_segment = trim_copy(
                        member_separator == std::string::npos
                            ? member_path
                            : member_path.substr(0U, member_separator));
                    if (normalize_identifier(first_segment) != "forms")
                    {
                        return std::nullopt;
                    }
                    if (member_separator == std::string::npos)
                    {
                        return std::string{};
                    }
                    return member_path.substr(member_separator + 1U);
                };
                const Statement *statement = current_statement();
                const std::string action_text = statement == nullptr ? property_path : statement->text;
                const std::string normalized_property_path = normalize_identifier(property_path);
                if (normalized_property_path == "_screen.hwnd")
                {
                    return make_int64_value(static_cast<std::int64_t>(kCopperfinScreenClientHwnd));
                }
                if (normalized_property_path == "_vfp.hwnd")
                {
                    return make_int64_value(static_cast<std::int64_t>(kCopperfinVfpMainWindowHwnd));
                }
                if (normalized_property_path == "_screen.activeform" ||
                    normalized_property_path == "_vfp.activeform")
                {
                    if (const RuntimeOleObjectState *runtime_object = representative_active_form_object();
                        runtime_object != nullptr)
                    {
                        return make_string_value(
                            "object:" + runtime_object->prog_id + "#" + std::to_string(runtime_object->handle));
                    }
                    return make_empty_value();
                }
                if (normalized_property_path == "_screen.caption" ||
                    normalized_property_path == "_vfp.caption")
                {
                    return make_string_value(representative_application_caption);
                }
                if (normalized_property_path == "_screen.windowstate" ||
                    normalized_property_path == "_vfp.windowstate")
                {
                    return make_int64_value(static_cast<std::int64_t>(representative_application_window_state));
                }
                if (normalized_property_path == "_screen.formcount" ||
                    normalized_property_path == "_vfp.formcount")
                {
                    return make_int64_value(
                        static_cast<std::int64_t>(representative_application_form_count()));
                }
                const auto raise_ole_fault = [&](const std::string &detail,
                                                 const std::string &source,
                                                 const std::string &message) -> PrgValue
                {
                    record_ole_aerror_context(detail,
                                              "Copperfin OLE",
                                              source,
                                              action_text,
                                              1429);
                    throw std::runtime_error(message);
                };
                if (const auto active_form_tail = active_form_member_tail(property_path);
                    active_form_tail.has_value() && !active_form_tail->empty())
                {
                    RuntimeOleObjectState *runtime_object = representative_active_form_object();
                    if (runtime_object == nullptr)
                    {
                        return make_empty_value();
                    }

                    const auto resolved_path =
                        resolve_runtime_object_member_path(runtime_object, *active_form_tail);
                    if (resolved_path.runtime_object == nullptr)
                    {
                        return raise_ole_fault(property_path,
                                               property_path.substr(0U, property_path.find('.')),
                                               runtime_text(
                                                   "Runtime.Prg.Core.Error.OleObjectNotFoundForPropertyRead",
                                                   {{"propertyPath", property_path}}));
                    }

                    runtime_object = resolved_path.runtime_object;
                    const std::string effective_property_path =
                        resolved_path.remaining_member_path.empty()
                            ? *active_form_tail
                            : resolved_path.remaining_member_path;
                    runtime_object->last_action = effective_property_path;
                    ++runtime_object->action_count;
                    events.push_back({.category = "ole.get",
                                      .detail = runtime_object->prog_id + "." + effective_property_path,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    const std::string property_name = normalize_identifier(effective_property_path);
                    if (auto property_result = read_native_property_if_present(*runtime_object, property_name, frame);
                        property_result.has_value())
                    {
                        return *property_result;
                    }
                    if (!runtime_object->class_hierarchy.empty() &&
                        property_name == "hwnd" &&
                        !runtime_object->native_hwnd.has_value())
                    {
                        return raise_ole_fault(runtime_object->prog_id + "." + effective_property_path,
                                               property_path.substr(0U, property_path.find('.')),
                                               runtime_text(
                                                   "Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead",
                                                   {{"memberIdentifier", runtime_object->prog_id + "." + effective_property_path}}));
                    }
                    if (normalize_identifier(runtime_object->prog_id) == "scripting.dictionary")
                    {
                        return raise_ole_fault(runtime_object->prog_id + "." + effective_property_path,
                                               property_path.substr(0U, property_path.find('.')),
                                               runtime_text(
                                                   "Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead",
                                                   {{"memberIdentifier", runtime_object->prog_id + "." + effective_property_path}}));
                    }
                    return make_string_value("ole:" + runtime_object->prog_id + "." + effective_property_path);
                }
                if (const auto forms_tail = application_forms_member_tail(property_path);
                    forms_tail.has_value())
                {
                    RuntimeOleObjectState *runtime_object =
                        ensure_representative_application_forms_collection_object();
                    if (runtime_object == nullptr)
                    {
                        return make_empty_value();
                    }

                    if (forms_tail->empty())
                    {
                        return make_string_value(
                            "object:" + runtime_object->prog_id + "#" + std::to_string(runtime_object->handle));
                    }

                    const auto resolved_path =
                        resolve_runtime_object_member_path(runtime_object, *forms_tail);
                    if (resolved_path.runtime_object == nullptr)
                    {
                        return raise_ole_fault(property_path,
                                               property_path.substr(0U, property_path.find('.')),
                                               runtime_text(
                                                   "Runtime.Prg.Core.Error.OleObjectNotFoundForPropertyRead",
                                                   {{"propertyPath", property_path}}));
                    }

                    runtime_object = resolved_path.runtime_object;
                    const std::string effective_property_path =
                        resolved_path.remaining_member_path.empty()
                            ? *forms_tail
                            : resolved_path.remaining_member_path;
                    runtime_object->last_action = effective_property_path;
                    ++runtime_object->action_count;
                    events.push_back({.category = "ole.get",
                                      .detail = runtime_object->prog_id + "." + effective_property_path,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});

                    if (auto property_result = read_native_property_if_present(
                            *runtime_object,
                            effective_property_path,
                            frame);
                        property_result.has_value())
                    {
                        return *property_result;
                    }

                    return make_string_value("ole:" + runtime_object->prog_id + "." + effective_property_path);
                }
                const auto separator = property_path.find('.');
                if (separator == std::string::npos)
                {
                    return make_empty_value();
                }

                const std::string object_name = property_path.substr(0U, separator);
                const std::string member_path = property_path.substr(separator + 1U);
                const auto resolved_path = resolve_runtime_object_member_path(frame, object_name, member_path);
                if (resolved_path.runtime_object == nullptr)
                {
                    return raise_ole_fault(property_path,
                                           object_name,
                                           runtime_text(
                                               "Runtime.Prg.Core.Error.OleObjectNotFoundForPropertyRead",
                                               {{"propertyPath", property_path}}));
                }

                RuntimeOleObjectState *runtime_object = resolved_path.runtime_object;
                const std::string effective_property_path =
                    resolved_path.remaining_member_path.empty()
                        ? member_path
                        : resolved_path.remaining_member_path;
                runtime_object->last_action = effective_property_path;
                ++runtime_object->action_count;
                events.push_back({.category = "ole.get",
                                  .detail = runtime_object->prog_id + "." + effective_property_path,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                const std::string property_name = normalize_identifier(effective_property_path);
                if (auto property_result = read_native_property_if_present(*runtime_object, property_name, frame);
                    property_result.has_value())
                {
                    return *property_result;
                }
                if (!runtime_object->class_hierarchy.empty() &&
                    property_name == "hwnd" &&
                    !runtime_object->native_hwnd.has_value())
                {
                    return raise_ole_fault(runtime_object->prog_id + "." + effective_property_path,
                                           object_name,
                                           runtime_text(
                                               "Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead",
                                               {{"memberIdentifier", runtime_object->prog_id + "." + effective_property_path}}));
                }
                if (normalize_identifier(runtime_object->prog_id) == "scripting.dictionary")
                {
                    return raise_ole_fault(runtime_object->prog_id + "." + effective_property_path,
                                           object_name,
                                           runtime_text(
                                               "Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead",
                                               {{"memberIdentifier", runtime_object->prog_id + "." + effective_property_path}}));
                }
                return make_string_value("ole:" + runtime_object->prog_id + "." + effective_property_path);
            },
            [this, &frame, preferred_cursor](const std::string &nested_expression)
            {
                return evaluate_expression(nested_expression, frame, preferred_cursor);
            },
            [this](const std::string &option_name)
            {
                const std::string normalized_name = normalize_identifier(option_name);
                if (normalized_name == "default")
                {
                    return current_default_directory();
                }
                if (normalized_name == "memowidth")
                {
                    const auto found_memowidth = memowidth_by_session.find(current_data_session);
                    const std::size_t memo_width = found_memowidth != memowidth_by_session.end() ? found_memowidth->second : 50U;
                    return std::to_string(memo_width);
                }
                if (normalized_name == "txnlevel")
                {
                    return std::to_string(current_transaction_level());
                }
                if (normalized_name == "fields")
                {
                    if (!is_set_enabled("fields_enabled"))
                    {
                        return std::string("OFF");
                    }
                    const auto found_fields = current_set_state().find("fields");
                    return found_fields == current_set_state().end() || trim_copy(found_fields->second).empty()
                               ? std::string("ON")
                               : found_fields->second;
                }

                const auto found = current_set_state().find(normalized_name);
                if (found == current_set_state().end())
                {
                    if (normalized_name == "century")
                    {
                        return std::string("ON");
                    }
                    if (normalized_name == "date")
                    {
                        return std::string("MDY");
                    }
                    if (normalized_name == "mark")
                    {
                        return std::string("/");
                    }
                    if (normalized_name == "hours")
                    {
                        return std::string("24");
                    }
                    if (normalized_name == "seconds")
                    {
                        return std::string("ON");
                    }
                    if (normalized_name == "exclusive")
                    {
                        return std::string("ON");
                    }
                    if (normalized_name == "fdow" || normalized_name == "fweek")
                    {
                        return std::string("1");
                    }
                    if (normalized_name == "reprocess")
                    {
                        return std::string("AUTOMATIC");
                    }
                    if (normalized_name == "path")
                    {
                        return std::string{};
                    }
                    if (normalized_name == "decimals")
                    {
                        return std::string("2");
                    }
                    if (normalized_name == "collate")
                    {
                        return std::string("MACHINE");
                    }
                    if (normalized_name == "point")
                    {
                        return std::string(".");
                    }
                    if (normalized_name == "separator")
                    {
                        return std::string(",");
                    }
                    if (normalized_name == "currency")
                    {
                        return std::string("$");
                    }
                    return std::string("OFF");
                }

                if (normalized_name == "path" ||
                    normalized_name == "date" ||
                    normalized_name == "mark" ||
                    normalized_name == "hours" ||
                    normalized_name == "fdow" ||
                    normalized_name == "fweek" ||
                    normalized_name == "reprocess" ||
                    normalized_name == "decimals" ||
                    normalized_name == "collate" ||
                    normalized_name == "point" ||
                    normalized_name == "separator" ||
                    normalized_name == "currency")
                {
                    return found->second;
                }

                const std::string normalized_value = normalize_identifier(found->second);
                if (normalized_value.empty() || normalized_value == "on" || normalized_value == "true" || normalized_value == "1" ||
                    normalized_value == ".t." || normalized_value == "yes" || normalized_value == "y")
                {
                    return std::string("ON");
                }
                if (normalized_value == "off" || normalized_value == "false" || normalized_value == "0" ||
                    normalized_value == ".f." || normalized_value == "no" || normalized_value == "n")
                {
                    return std::string("OFF");
                }
                return found->second;
            },
            [this](const std::string &designator) -> std::optional<RuntimeSurfaceCursorSnapshot>
            {
                CursorState *cursor = resolve_cursor_target(designator);
                if (cursor == nullptr)
                {
                    return std::nullopt;
                }

                RuntimeSurfaceCursorSnapshot snapshot;
                snapshot.alias = cursor->alias;
                if (!cursor->remote && !cursor->source_path.empty())
                {
                    const auto header_result = vfp::parse_dbf_header_from_file(cursor->source_path);
                    if (header_result.ok)
                    {
                        snapshot.code_page = vfp::dbf_code_page_from_mark(header_result.header.code_page_mark);
                    }
                }

                const std::vector<vfp::DbfFieldDescriptor> descriptors = cursor_field_descriptors(*cursor);
                snapshot.fields.reserve(descriptors.size());
                for (const auto &descriptor : descriptors)
                {
                    snapshot.fields.push_back(RuntimeSurfaceCursorField{
                        .name = descriptor.name,
                        .type = descriptor.type,
                        .width = static_cast<std::size_t>(descriptor.length),
                        .decimals = static_cast<std::size_t>(descriptor.decimal_count)});
                }

                if (cursor->remote)
                {
                    snapshot.rows.reserve(cursor->remote_records.size());
                    for (const auto &record : cursor->remote_records)
                    {
                        RuntimeSurfaceCursorRow row;
                        row.values.reserve(snapshot.fields.size());
                        for (const auto &field : snapshot.fields)
                        {
                            row.values.push_back(record_field_value(record, field.name).value_or(std::string{}));
                        }
                        snapshot.rows.push_back(std::move(row));
                    }
                    return snapshot;
                }

                if (cursor->source_path.empty())
                {
                    return std::nullopt;
                }

                const auto parse_result =
                    vfp::parse_dbf_table_from_file(cursor->source_path, std::max<std::size_t>(cursor->record_count, 1U));
                if (!parse_result.ok)
                {
                    return std::nullopt;
                }

                snapshot.rows.reserve(parse_result.table.records.size());
                for (const auto &record : parse_result.table.records)
                {
                    RuntimeSurfaceCursorRow row;
                    row.values.reserve(snapshot.fields.size());
                    for (const auto &field : snapshot.fields)
                    {
                        row.values.push_back(record_field_value(record, field.name).value_or(std::string{}));
                    }
                    snapshot.rows.push_back(std::move(row));
                }
                return snapshot;
            },
            [this](const RuntimeSurfaceCursorSnapshot &snapshot, const std::string &destination_alias) -> std::optional<std::size_t>
            {
                std::string alias = normalize_identifier(trim_copy(destination_alias));
                if (alias.empty())
                {
                    return std::nullopt;
                }

                CursorState *existing = resolve_cursor_target(alias);
                if (existing != nullptr)
                {
                    if (existing->remote || existing->source_path.empty())
                    {
                        return std::nullopt;
                    }

                    const std::vector<vfp::DbfFieldDescriptor> existing_fields = cursor_field_descriptors(*existing);
                    if (existing_fields.size() != snapshot.fields.size())
                    {
                        return std::nullopt;
                    }
                    for (std::size_t index = 0U; index < existing_fields.size(); ++index)
                    {
                        if (collapse_identifier(existing_fields[index].name) != collapse_identifier(snapshot.fields[index].name))
                        {
                            return std::nullopt;
                        }
                    }

                    if (!ensure_transaction_backup_for_table(existing->source_path))
                    {
                        return std::nullopt;
                    }
                    const auto truncate_result = vfp::truncate_dbf_table_file(existing->source_path, 0U);
                    if (!truncate_result.ok)
                    {
                        return std::nullopt;
                    }

                    existing->record_count = 0U;
                    move_cursor_to(*existing, 0);
                    existing->found = false;

                    for (const auto &row : snapshot.rows)
                    {
                        const auto append_result = vfp::append_blank_record_to_file(existing->source_path);
                        if (!append_result.ok)
                        {
                            return std::nullopt;
                        }
                        existing->record_count = append_result.record_count;
                        move_cursor_to(*existing, static_cast<long long>(append_result.record_count));

                        for (std::size_t index = 0U; index < snapshot.fields.size() && index < row.values.size(); ++index)
                        {
                            const auto replace_result = vfp::replace_record_field_value(
                                existing->source_path,
                                append_result.record_count - 1U,
                                snapshot.fields[index].name,
                                row.values[index]);
                            if (!replace_result.ok)
                            {
                                return std::nullopt;
                            }
                            existing->record_count = replace_result.record_count;
                        }
                    }

                    if (existing->record_count == 0U)
                    {
                        move_cursor_to(*existing, 0);
                    }
                    else
                    {
                        move_cursor_to(*existing, 1);
                    }
                    return snapshot.rows.size();
                }

                std::vector<vfp::DbfFieldDescriptor> descriptors;
                descriptors.reserve(snapshot.fields.size());
                for (std::size_t index = 0U; index < snapshot.fields.size(); ++index)
                {
                    const RuntimeSurfaceCursorField &field = snapshot.fields[index];
                    const std::string fallback_name = "F" + std::to_string(index + 1U);
                    std::string field_name = trim_copy(field.name);
                    if (field_name.empty())
                    {
                        field_name = fallback_name;
                    }
                    char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.type)));
                    if (field_type != 'N' && field_type != 'L' && field_type != 'D' && field_type != 'C')
                    {
                        field_type = 'C';
                    }
                    const std::size_t default_width = field_type == 'N' ? 18U : (field_type == 'L' ? 1U : 64U);
                    const std::size_t bounded_width = std::max<std::size_t>(1U, std::min<std::size_t>(field.width, 254U));
                    const std::size_t width = field.width == 0U ? default_width : bounded_width;
                    const std::size_t decimals = field_type == 'N'
                                                     ? std::min<std::size_t>(std::min<std::size_t>(field.decimals, 15U), width > 0U ? width - 1U : 0U)
                                                     : 0U;
                    descriptors.push_back(vfp::DbfFieldDescriptor{
                        .name = field_name,
                        .type = field_type,
                        .length = static_cast<std::uint8_t>(width),
                        .decimal_count = static_cast<std::uint8_t>(decimals)});
                }
                if (descriptors.empty())
                {
                    return std::nullopt;
                }

                std::vector<std::vector<std::string>> rows;
                rows.reserve(snapshot.rows.size());
                for (const auto &row : snapshot.rows)
                {
                    std::vector<std::string> values(descriptors.size(), std::string{});
                    for (std::size_t index = 0U; index < descriptors.size() && index < row.values.size(); ++index)
                    {
                        values[index] = row.values[index];
                    }
                    rows.push_back(std::move(values));
                }

                std::error_code ignored;
                const std::filesystem::path cursor_root = runtime_temp_directory / "cursors";
                std::filesystem::create_directories(cursor_root, ignored);
                std::filesystem::path table_path;
                for (std::size_t attempt = 0U;; ++attempt)
                {
                    const std::string suffix = attempt == 0U ? std::string{} : "_" + std::to_string(attempt + 1U);
                    table_path = cursor_root / (alias + "_xml_ds" + std::to_string(current_data_session) + suffix + ".dbf");
                    if (!std::filesystem::exists(table_path, ignored))
                    {
                        break;
                    }
                }

                const auto create_result = vfp::create_dbf_table_file(table_path.string(), descriptors, rows);
                if (!create_result.ok)
                {
                    return std::nullopt;
                }

                if (!open_table_cursor(table_path.string(), alias, {}, true, false, 0, {}, 0U))
                {
                    return std::nullopt;
                }
                return snapshot.rows.size();
            },
            [this](const std::string &category, const std::string &detail)
            {
                events.push_back({.category = category,
                                  .detail = detail,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            },
            [this](const PrgValue &value) -> RuntimeOleObjectState *
            {
                auto object = resolve_ole_object(value);
                return object.has_value() ? *object : nullptr;
            },
            [this, &frame](const std::string &identifier) -> RuntimeOleObjectState *
            {
                const std::string normalized_identifier = normalize_identifier(identifier);
                if (normalized_identifier == "_screen.forms" ||
                    normalized_identifier == "_vfp.forms")
                {
                    return ensure_representative_application_forms_collection_object();
                }

                const auto separator = identifier.find('.');
                if (separator == std::string::npos)
                {
                    const PrgValue value = lookup_variable(frame, identifier);
                    auto object = resolve_ole_object(value);
                    return object.has_value() ? *object : nullptr;
                }

                const std::string object_name = identifier.substr(0U, separator);
                const std::string member_path = identifier.substr(separator + 1U);
                const auto resolved_path = resolve_runtime_object_member_path(frame, object_name, member_path);
                if (resolved_path.runtime_object == nullptr)
                {
                    return nullptr;
                }

                const std::string effective_member_path =
                    resolved_path.remaining_member_path.empty()
                        ? member_path
                        : resolved_path.remaining_member_path;
                if (effective_member_path.find('.') != std::string::npos)
                {
                    return nullptr;
                }

                const auto property = resolved_path.runtime_object->properties.find(
                    normalize_identifier(effective_member_path));
                if (property == resolved_path.runtime_object->properties.end())
                {
                    return nullptr;
                }

                auto object = resolve_ole_object(property->second);
                return object.has_value() ? *object : nullptr;
            },
            [this, &frame](const PrgValue &value, const std::string &member_name) -> std::optional<PrgValue>
            {
                auto object = resolve_ole_object(value);
                if (!object.has_value())
                {
                    return std::nullopt;
                }

                RuntimeOleObjectState *target_object = *object;
                std::vector<std::string> segments;
                std::size_t start = 0U;
                while (start <= member_name.size())
                {
                    const std::size_t separator = member_name.find('.', start);
                    std::string segment = separator == std::string::npos
                                              ? member_name.substr(start)
                                              : member_name.substr(start, separator - start);
                    segment = trim_copy(segment);
                    if (!segment.empty())
                    {
                        segments.push_back(segment);
                    }
                    if (separator == std::string::npos)
                    {
                        break;
                    }
                    start = separator + 1U;
                }
                if (segments.empty())
                {
                    return std::nullopt;
                }

                for (std::size_t index = 0U; index + 1U < segments.size(); ++index)
                {
                    const auto property = target_object->properties.find(normalize_identifier(segments[index]));
                    if (property == target_object->properties.end())
                    {
                        return std::nullopt;
                    }
                    const auto nested_object = resolve_ole_object(property->second);
                    if (!nested_object.has_value())
                    {
                        return std::nullopt;
                    }
                    target_object = *nested_object;
                }

                const std::string &leaf_member_name = segments.back();
                const std::string normalized_member_name = normalize_identifier(leaf_member_name);
                if (!normalized_member_name.empty() &&
                    !target_object->class_hierarchy.empty() &&
                    normalized_member_name == "hwnd" &&
                    !target_object->native_hwnd.has_value())
                {
                    const Statement *statement = current_statement();
                    const std::string action_text = statement == nullptr ? member_name : statement->text;
                    const std::string detail = target_object->prog_id + "." + leaf_member_name;
                    record_ole_aerror_context(detail,
                                              "Copperfin OLE",
                                              target_object->prog_id,
                                              action_text,
                                              1429);
                    throw std::runtime_error(
                        runtime_text(
                            "Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead",
                            {{"memberIdentifier", detail}}));
                }
                return read_native_property_if_present(*target_object, leaf_member_name, frame);
            },
            [this, &frame](const PrgValue &value, const std::string &member_name, const PrgValue &assigned_value) -> bool
            {
                auto object = resolve_ole_object(value);
                if (!object.has_value())
                {
                    return false;
                }
                return write_native_property_if_present(**object, member_name, assigned_value, frame);
            },
            [this](std::int64_t hwnd) -> std::optional<std::int64_t>
            {
                if (auto whandle = whandle_from_hwnd(static_cast<std::intptr_t>(hwnd)); whandle.has_value())
                {
                    return static_cast<std::int64_t>(*whandle);
                }
                return std::nullopt;
            },
            [this](std::int64_t whandle) -> std::optional<std::int64_t>
            {
                if (auto hwnd = hwnd_from_whandle(static_cast<std::intptr_t>(whandle)); hwnd.has_value())
                {
                    return static_cast<std::int64_t>(*hwnd);
                }
                return std::nullopt;
            },
            [this](const std::string &name, std::vector<PrgValue> values)
            {
                assign_array(name, std::move(values));
            },
            [this, &frame](
                const std::vector<PrgValue> &arguments,
                const std::vector<std::optional<std::string>> &argument_references) -> PrgValue
            {
                return bind_native_event(frame, arguments, argument_references);
            },
            [this, &frame](
                const std::vector<PrgValue> &arguments,
                const std::vector<std::optional<std::string>> &argument_references) -> PrgValue
            {
                return raise_native_event(frame, arguments, argument_references);
            },
            [this](const std::vector<PrgValue> &arguments) -> PrgValue
            {
                return unbind_native_events(arguments);
            },
            [this](
                const std::vector<PrgValue> &arguments,
                const std::vector<std::string> &raw_arguments) -> PrgValue
            {
                return inspect_native_events(arguments, raw_arguments);
            },
            [this]()
            {
                const auto found = memowidth_by_session.find(current_data_session);
                return found != memowidth_by_session.end() ? found->second : 50U;
            },
            [this, &frame](
                const std::vector<PrgValue> &arguments,
                const std::vector<std::optional<std::string>> &argument_references) -> std::optional<PrgValue>
            {
                return invoke_expression_base_method(frame, arguments, argument_references);
            },
            [this, &frame](
                const std::string &identifier,
                const std::vector<PrgValue> &arguments,
                const std::vector<std::optional<std::string>> &argument_references) -> std::optional<PrgValue>
            {
                return invoke_expression_user_routine(frame, identifier, arguments, argument_references);
            },
            [this](
                const std::string &fn_key,
                const std::vector<PrgValue> &fn_args,
                const std::vector<std::optional<std::string>> &fn_argument_references) -> PrgValue
            {
                return invoke_declared_dll_function(fn_key, fn_args, fn_argument_references);
            });
        return parser.parse();
    }

    void PrgRuntimeSession::Impl::assign_native_window_metadata(RuntimeOleObjectState &runtime_object)
    {
        if (runtime_object.native_hwnd.has_value())
        {
            return;
        }

        const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
        const bool supports_runtime_window_handle =
            normalized_base_class == "form" ||
            normalized_base_class == "olecontrol" ||
            normalized_base_class == "toolbar";
        if (!supports_runtime_window_handle)
        {
            return;
        }

        runtime_object.native_hwnd = kCopperfinSyntheticWindowHwndBase + runtime_object.handle;
    }

    std::optional<std::intptr_t> PrgRuntimeSession::Impl::hwnd_from_whandle(std::intptr_t whandle) const
    {
        if (whandle == kCopperfinScreenWhandle)
        {
            return kCopperfinScreenClientHwnd;
        }
        if (whandle == kCopperfinVfpWhandle)
        {
            return kCopperfinVfpMainWindowHwnd;
        }
        const auto found = ole_objects.find(static_cast<int>(whandle));
        if (found == ole_objects.end() || !found->second.native_hwnd.has_value())
        {
            return std::nullopt;
        }
        return found->second.native_hwnd;
    }

    std::optional<std::intptr_t> PrgRuntimeSession::Impl::whandle_from_hwnd(std::intptr_t hwnd) const
    {
        if (hwnd == kCopperfinScreenClientHwnd)
        {
            return kCopperfinScreenWhandle;
        }
        if (hwnd == kCopperfinVfpMainWindowHwnd)
        {
            return kCopperfinVfpWhandle;
        }
        for (const auto &[handle, runtime_object] : ole_objects)
        {
            if (runtime_object.native_hwnd.has_value() && *runtime_object.native_hwnd == hwnd)
            {
                return static_cast<std::intptr_t>(handle);
            }
        }
        return std::nullopt;
    }

    RuntimeOleObjectState *PrgRuntimeSession::Impl::representative_active_form_object()
    {
        if (!representative_active_form_handle.has_value())
        {
            return nullptr;
        }

        const auto found = ole_objects.find(*representative_active_form_handle);
        if (found == ole_objects.end() ||
            normalize_identifier(trim_copy(found->second.base_class_name)) != "form")
        {
            return nullptr;
        }

        return &found->second;
    }

    const RuntimeOleObjectState *PrgRuntimeSession::Impl::representative_active_form_object() const
    {
        if (!representative_active_form_handle.has_value())
        {
            return nullptr;
        }

        const auto found = ole_objects.find(*representative_active_form_handle);
        if (found == ole_objects.end() ||
            normalize_identifier(trim_copy(found->second.base_class_name)) != "form")
        {
            return nullptr;
        }

        return &found->second;
    }

    void PrgRuntimeSession::Impl::note_representative_active_form(const RuntimeOleObjectState &runtime_object)
    {
        if (normalize_identifier(trim_copy(runtime_object.base_class_name)) == "form")
        {
            representative_active_form_handle = runtime_object.handle;
            return;
        }

        if (const auto owner_form_reference = native_object_owner_form_reference(runtime_object);
            owner_form_reference.has_value())
        {
            int owner_form_handle = 0;
            std::string owner_form_prog_id;
            if (parse_object_handle_reference(*owner_form_reference, owner_form_handle, owner_form_prog_id))
            {
                representative_active_form_handle = owner_form_handle;
            }
        }
    }

    std::vector<int> PrgRuntimeSession::Impl::representative_application_window_handles() const
    {
        std::vector<int> handles;
        handles.reserve(ole_objects.size());

        const auto is_application_window = [](const RuntimeOleObjectState &runtime_object)
        {
            if (runtime_object.hidden_runtime_surface)
            {
                return false;
            }

            const std::string normalized_base_class_name =
                normalize_identifier(trim_copy(runtime_object.base_class_name));
            return normalized_base_class_name == "form" ||
                   normalized_base_class_name == "toolbar";
        };

        if (representative_active_form_handle.has_value())
        {
            const auto active_found = ole_objects.find(*representative_active_form_handle);
            if (active_found != ole_objects.end() &&
                is_application_window(active_found->second) &&
                normalize_identifier(trim_copy(active_found->second.base_class_name)) == "form")
            {
                handles.push_back(active_found->second.handle);
            }
        }

        for (auto it = ole_objects.rbegin(); it != ole_objects.rend(); ++it)
        {
            if (!is_application_window(it->second))
            {
                continue;
            }
            if (!handles.empty() && handles.front() == it->second.handle)
            {
                continue;
            }
            handles.push_back(it->second.handle);
        }

        return handles;
    }

    std::size_t PrgRuntimeSession::Impl::representative_application_form_count() const
    {
        return representative_application_window_handles().size();
    }

    RuntimeOleObjectState *PrgRuntimeSession::Impl::ensure_representative_application_forms_collection_object()
    {
        RuntimeOleObjectState *collection_object = nullptr;
        if (representative_application_forms_collection_handle.has_value())
        {
            const auto found = ole_objects.find(*representative_application_forms_collection_handle);
            if (found != ole_objects.end() &&
                is_native_collection_object(found->second) &&
                found->second.hidden_runtime_surface &&
                found->second.read_only_collection_surface)
            {
                collection_object = &found->second;
            }
        }

        if (collection_object == nullptr)
        {
            const int handle = next_ole_handle++;
            RuntimeOleObjectState collection_state{
                .handle = handle,
                .prog_id = "Collection",
                .source = {},
                .last_action = "Forms",
                .action_count = 1,
                .hidden_runtime_surface = true,
                .read_only_collection_surface = true};
            collection_state.base_class_name = "Collection";
            collection_state.class_hierarchy = {"COLLECTION", "OBJECT"};
            auto [collection_it, _] = ole_objects.emplace(handle, std::move(collection_state));
            representative_application_forms_collection_handle = handle;
            collection_object = &collection_it->second;
        }

        collection_object->collection_items.clear();
        collection_object->collection_item_keys.clear();

        for (const int handle : representative_application_window_handles())
        {
            const auto found = ole_objects.find(handle);
            if (found == ole_objects.end())
            {
                continue;
            }

            collection_object->collection_items.push_back(
                make_string_value("object:" + found->second.prog_id + "#" + std::to_string(found->second.handle)));
            const auto name = found->second.properties.find("name");
            collection_object->collection_item_keys.push_back(
                name != found->second.properties.end()
                    ? value_as_string(name->second)
                    : found->second.prog_id);
        }

        (void)read_native_collection_member(*collection_object, "count");
        return collection_object;
    }

    bool PrgRuntimeSession::Impl::consume_last_popped_frame_requested_nodefault()
    {
        const bool requested = last_popped_frame_requested_nodefault;
        last_popped_frame_requested_nodefault = false;
        return requested;
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_expression_user_routine(
        const Frame &source_frame,
        const std::string &identifier,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        Program &program = load_program(source_frame.file_path);
        if (!source_frame.native_method_class_name.empty())
        {
            const auto this_found = source_frame.locals.find("this");
            if (this_found != source_frame.locals.end())
            {
                auto runtime_object = resolve_ole_object(this_found->second);
                if (runtime_object.has_value())
                {
                    if (auto native_result = invoke_native_object_method_if_present(
                            **runtime_object,
                            identifier,
                            source_frame,
                            arguments,
                            argument_references);
                        native_result.has_value())
                    {
                        return native_result;
                    }
                }
            }
        }

        const auto found = program.routines.find(normalize_identifier(identifier));
        if (found == program.routines.end())
        {
            return std::nullopt;
        }

        if (!can_push_frame())
        {
            throw std::runtime_error(call_depth_limit_message());
        }

        const std::size_t return_depth = stack.size();
        push_routine_frame(program.path, found->second, arguments, argument_references);
        return run_expression_invoked_routine_until_return(return_depth);
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_native_object_method_body_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &identifier,
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        if (runtime_object.source.empty())
        {
            return std::nullopt;
        }

        bool use_source_frame_method_context = false;
        if (!source_frame.native_method_class_name.empty())
        {
            const auto this_found = source_frame.locals.find("this");
            if (this_found != source_frame.locals.end())
            {
                if (auto current_this_object = resolve_ole_object(this_found->second);
                    current_this_object.has_value() &&
                    (*current_this_object)->handle == runtime_object.handle)
                {
                    use_source_frame_method_context = true;
                }
            }
        }

        Program &program = load_program(
            use_source_frame_method_context
                ? source_frame.file_path
                : runtime_object.source);
        std::string native_method_name;
        std::string native_defining_class_name;
        const auto native_method =
            find_native_class_method_lookup(
                program,
                use_source_frame_method_context
                    ? source_frame.native_method_class_name
                    : runtime_object.prog_id,
                identifier,
                true,
                native_method_name,
                &native_defining_class_name);
        if (!native_method.has_value())
        {
            return std::nullopt;
        }

        if (!can_push_frame())
        {
            throw std::runtime_error(call_depth_limit_message());
        }

        runtime_object.last_action = identifier + "()";
        ++runtime_object.action_count;
        events.push_back({.category = "prg.object.invoke",
                          .detail = native_method_name,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});

        const std::size_t return_depth = stack.size();
        const PrgValue this_reference =
            make_string_value("object:" + runtime_object.prog_id + "#" + std::to_string(runtime_object.handle));
        push_method_frame(native_method->program->path,
                          native_method_name,
                          *native_method->routine,
                          this_reference,
                          native_defining_class_name,
                          normalize_identifier(identifier),
                          native_object_parent_reference(runtime_object),
                          native_object_owner_form_reference(runtime_object),
                          native_object_owner_formset_reference(runtime_object),
                          std::vector<PrgValue>(arguments.begin(), arguments.end()),
                          std::vector<std::optional<std::string>>(
                              argument_references.begin(),
                              argument_references.end()));
        return run_expression_invoked_routine_until_return(return_depth);
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_native_event_delegate(
        const NativeEventBinding &binding,
        const CurrentNativeEventContext &event_context,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        struct CurrentEventGuard
        {
            std::vector<CurrentNativeEventContext> &stack;

            ~CurrentEventGuard()
            {
                if (!stack.empty())
                {
                    stack.pop_back();
                }
            }
        };

        active_native_event_contexts.push_back(event_context);
        CurrentEventGuard guard{active_native_event_contexts};

        if (binding.target_is_routine)
        {
            Program &program = load_program(binding.target_program_path);
            const auto found = program.routines.find(normalize_identifier(binding.delegate_name));
            if (found == program.routines.end())
            {
                return std::nullopt;
            }
            if (!can_push_frame())
            {
                throw std::runtime_error(call_depth_limit_message());
            }

            events.push_back({.category = "prg.event.delegate",
                              .detail = binding.event_name + " -> " + found->second.name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            const std::size_t return_depth = stack.size();
            push_routine_frame(program.path, found->second, arguments, argument_references);
            return run_expression_invoked_routine_until_return(return_depth);
        }

        const auto target_found = ole_objects.find(binding.target_handle);
        if (target_found == ole_objects.end())
        {
            return std::nullopt;
        }

        RuntimeOleObjectState &target_object = target_found->second;
        std::string method_program_path;
        std::string method_name;
        if (const Routine *method = find_native_object_method(
                target_object,
                binding.delegate_name,
                method_program_path,
                method_name);
            method != nullptr)
        {
            if (!can_push_frame())
            {
                throw std::runtime_error(call_depth_limit_message());
            }

            target_object.last_action = "bindevent:" + binding.delegate_name;
            ++target_object.action_count;
            events.push_back({.category = "prg.event.delegate",
                              .detail = binding.event_name + " -> " + method_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            const std::size_t return_depth = stack.size();
            const PrgValue this_reference =
                make_string_value("object:" + target_object.prog_id + "#" + std::to_string(target_object.handle));
            push_method_frame(method_program_path,
                              method_name,
                              *method,
                              this_reference,
                              method_name.substr(0U, method_name.rfind('.')),
                              normalize_identifier(binding.delegate_name),
                              native_object_parent_reference(target_object),
                              native_object_owner_form_reference(target_object),
                              native_object_owner_formset_reference(target_object),
                              arguments,
                              argument_references);
            return run_expression_invoked_routine_until_return(return_depth);
        }

        return std::nullopt;
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_window_message_delegate(
        const WindowMessageBinding &binding,
        const CurrentWindowMessageContext &message_context,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        struct CurrentMessageGuard
        {
            std::vector<CurrentWindowMessageContext> &stack;

            ~CurrentMessageGuard()
            {
                if (!stack.empty())
                {
                    stack.pop_back();
                }
            }
        };

        active_window_message_contexts.push_back(message_context);
        CurrentMessageGuard guard{active_window_message_contexts};

        const auto target_found = ole_objects.find(binding.target_handle);
        if (target_found == ole_objects.end())
        {
            return std::nullopt;
        }

        RuntimeOleObjectState &target_object = target_found->second;
        std::string method_program_path;
        std::string method_name;
        if (const Routine *method = find_native_object_method(
                target_object,
                binding.delegate_name,
                method_program_path,
                method_name);
            method != nullptr)
        {
            if (!can_push_frame())
            {
                throw std::runtime_error(call_depth_limit_message());
            }

            target_object.last_action = "bindevent:" + binding.delegate_name;
            ++target_object.action_count;
            events.push_back({.category = "prg.event.delegate",
                              .detail = std::to_string(message_context.window_handle) + ":" +
                                            std::to_string(message_context.message) + " -> " + method_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            const std::size_t return_depth = stack.size();
            const PrgValue this_reference =
                make_string_value("object:" + target_object.prog_id + "#" + std::to_string(target_object.handle));
            push_method_frame(method_program_path,
                              method_name,
                              *method,
                              this_reference,
                              method_name.substr(0U, method_name.rfind('.')),
                              normalize_identifier(binding.delegate_name),
                              native_object_parent_reference(target_object),
                              native_object_owner_form_reference(target_object),
                              native_object_owner_formset_reference(target_object),
                              arguments,
                              argument_references);
            return run_expression_invoked_routine_until_return(return_depth);
        }

        return std::nullopt;
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_native_object_method_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &identifier,
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        const std::string normalized_identifier = normalize_identifier(identifier);
        std::vector<NativeEventBinding> bindings;
        bindings.reserve(native_event_bindings.size());
        for (const NativeEventBinding &binding : native_event_bindings)
        {
            if (binding.source_handle == runtime_object.handle &&
                binding.event_name == normalized_identifier &&
                (binding.flags & 2) == 0)
            {
                bindings.push_back(binding);
            }
        }

        auto invoke_delegates_for_phase = [&](bool after_source_method)
        {
            for (const NativeEventBinding &binding : bindings)
            {
                const bool binding_after_source_method = (binding.flags & 1) != 0;
                if (binding_after_source_method == after_source_method)
                {
                    (void)invoke_native_event_delegate(
                        binding,
                        {.source_handle = runtime_object.handle,
                         .event_name = normalized_identifier,
                         .event_type = 2},
                        arguments,
                        argument_references);
                }
            }
        };

        const std::string active_event_key =
            std::to_string(runtime_object.handle) + ":" + normalized_identifier;
        const bool already_active =
            active_native_event_keys.find(active_event_key) != active_native_event_keys.end();

        if (!bindings.empty() && !already_active)
        {
            active_native_event_keys.insert(active_event_key);
            invoke_delegates_for_phase(false);
            auto result = invoke_native_object_method_body_if_present(
                runtime_object,
                normalized_identifier,
                source_frame,
                arguments,
                argument_references);
            invoke_delegates_for_phase(true);
            active_native_event_keys.erase(active_event_key);
            return result;
        }

        return invoke_native_object_method_body_if_present(
            runtime_object,
            normalized_identifier,
            source_frame,
            arguments,
            argument_references);
    }

    PrgValue PrgRuntimeSession::Impl::bind_native_event(
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        (void)argument_references;
        if (arguments.size() < 3U)
        {
            return make_number_value(0.0);
        }

        int object_handle_probe = 0;
        std::string object_prog_id_probe;
        const bool source_is_object =
            parse_object_handle_reference(arguments[0], object_handle_probe, object_prog_id_probe);
        const bool looks_like_window_message_binding =
            arguments.size() >= 4U &&
            !source_is_object &&
            arguments[1].kind != PrgValueKind::string;
        if (looks_like_window_message_binding)
        {
            const int window_handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
            const int message = static_cast<int>(std::llround(value_as_number(arguments[1])));
            if (message == 0)
            {
                return make_number_value(0.0);
            }

            auto target_object = resolve_ole_object(arguments[2]);
            const std::string delegate_name = trim_copy(value_as_string(arguments[3]));
            if (!target_object.has_value() || (*target_object)->source.empty() || delegate_name.empty())
            {
                return make_number_value(0.0);
            }

            std::string target_program_path;
            std::string target_method_name;
            if (const Routine *target_method = find_native_object_method(
                    **target_object,
                    delegate_name,
                    target_program_path,
                    target_method_name);
                target_method == nullptr)
            {
                return make_number_value(0.0);
            }

            WindowMessageBinding binding;
            binding.window_handle = window_handle;
            binding.message = message;
            binding.target_handle = (*target_object)->handle;
            binding.delegate_name = delegate_name;
            binding.ordinal = next_native_event_binding_ordinal++;

            const auto duplicate = std::find_if(
                window_message_bindings.begin(),
                window_message_bindings.end(),
                [&](const WindowMessageBinding &existing)
                {
                    return existing.window_handle == binding.window_handle &&
                           existing.message == binding.message &&
                           existing.target_handle == binding.target_handle &&
                           normalize_identifier(existing.delegate_name) == normalize_identifier(binding.delegate_name);
                });

            if (duplicate == window_message_bindings.end())
            {
                window_message_bindings.push_back(binding);
            }

            const auto binding_count = static_cast<double>(std::count_if(
                window_message_bindings.begin(),
                window_message_bindings.end(),
                [&](const WindowMessageBinding &existing)
                {
                    return existing.window_handle == binding.window_handle &&
                           existing.message == binding.message;
                }));

            events.push_back({.category = "prg.event.bind",
                              .detail = std::to_string(window_handle) + ":" + std::to_string(message) +
                                            " -> " + binding.delegate_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_number_value(binding_count);
        }

        auto source_object = resolve_ole_object(arguments[0]);
        if (!source_object.has_value() || (*source_object)->source.empty())
        {
            return make_number_value(0.0);
        }

        const std::string event_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (event_name.empty())
        {
            return make_number_value(0.0);
        }

        NativeEventBinding binding;
        binding.source_handle = (*source_object)->handle;
        binding.event_name = event_name;
        binding.ordinal = next_native_event_binding_ordinal++;

        auto target_object = arguments.size() >= 4U ? resolve_ole_object(arguments[2]) : std::optional<RuntimeOleObjectState *>{};
        if (arguments.size() >= 4U && target_object.has_value())
        {
            const std::string delegate_name = trim_copy(value_as_string(arguments[3]));
            if (!target_object.has_value() || (*target_object)->source.empty() || delegate_name.empty())
            {
                return make_number_value(0.0);
            }

            std::string target_program_path;
            std::string target_method_name;
            if (const Routine *target_method = find_native_object_method(
                    **target_object,
                    delegate_name,
                    target_program_path,
                    target_method_name);
                target_method == nullptr)
            {
                return make_number_value(0.0);
            }

            binding.target_is_routine = false;
            binding.target_handle = (*target_object)->handle;
            binding.delegate_name = delegate_name;
            binding.flags = arguments.size() >= 5U
                                ? static_cast<int>(std::llround(value_as_number(arguments[4]))) & 3
                                : 0;
        }
        else
        {
            const std::string routine_name = trim_copy(value_as_string(arguments[2]));
            if (routine_name.empty())
            {
                return make_number_value(0.0);
            }

            Program &program = load_program(source_frame.file_path);
            const auto found = program.routines.find(normalize_identifier(routine_name));
            if (found == program.routines.end())
            {
                return make_number_value(0.0);
            }

            binding.target_is_routine = true;
            binding.target_program_path = program.path;
            binding.delegate_name = found->second.name;
            binding.flags = arguments.size() >= 4U
                                ? static_cast<int>(std::llround(value_as_number(arguments[3]))) & 3
                                : 0;
        }

        const auto duplicate = std::find_if(
            native_event_bindings.begin(),
            native_event_bindings.end(),
            [&](const NativeEventBinding &existing)
            {
                return existing.source_handle == binding.source_handle &&
                       existing.event_name == binding.event_name &&
                       existing.target_is_routine == binding.target_is_routine &&
                       existing.target_program_path == binding.target_program_path &&
                       existing.target_handle == binding.target_handle &&
                       normalize_identifier(existing.delegate_name) == normalize_identifier(binding.delegate_name);
            });

        if (duplicate == native_event_bindings.end())
        {
            native_event_bindings.push_back(binding);
        }
        else
        {
            duplicate->flags = binding.flags;
        }

        const auto binding_count = static_cast<double>(std::count_if(
            native_event_bindings.begin(),
            native_event_bindings.end(),
            [&](const NativeEventBinding &existing)
            {
                return existing.source_handle == binding.source_handle &&
                       existing.event_name == binding.event_name;
            }));

        events.push_back({.category = "prg.event.bind",
                          .detail = (*source_object)->prog_id + "." + event_name + " -> " + binding.delegate_name,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        return make_number_value(binding_count);
    }

    PrgValue PrgRuntimeSession::Impl::raise_native_event(
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        if (arguments.size() < 2U)
        {
            return make_boolean_value(false);
        }

        auto source_object = resolve_ole_object(arguments[0]);
        if (!source_object.has_value() || (*source_object)->source.empty())
        {
            return make_boolean_value(false);
        }

        const std::string event_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (event_name.empty())
        {
            return make_boolean_value(false);
        }

        std::vector<PrgValue> event_arguments;
        std::vector<std::optional<std::string>> event_argument_references;
        if (arguments.size() > 2U)
        {
            event_arguments.assign(arguments.begin() + 2U, arguments.end());
        }
        if (argument_references.size() > 2U)
        {
            event_argument_references.assign(argument_references.begin() + 2U, argument_references.end());
        }

        std::vector<NativeEventBinding> bindings;
        bindings.reserve(native_event_bindings.size());
        for (const NativeEventBinding &binding : native_event_bindings)
        {
            if (binding.source_handle == (*source_object)->handle &&
                binding.event_name == event_name)
            {
                bindings.push_back(binding);
            }
        }

        const std::string active_event_key =
            std::to_string((*source_object)->handle) + ":" + event_name;
        if (active_native_event_keys.find(active_event_key) != active_native_event_keys.end())
        {
            return make_boolean_value(true);
        }

        active_native_event_keys.insert(active_event_key);
        const auto invoke_delegates_for_phase = [&](bool after_source_method)
        {
            for (const NativeEventBinding &binding : bindings)
            {
                const bool binding_after_source_method = (binding.flags & 1) != 0;
                if (binding_after_source_method == after_source_method)
                {
                    (void)invoke_native_event_delegate(
                        binding,
                        {.source_handle = (*source_object)->handle,
                         .event_name = event_name,
                         .event_type = 1},
                        event_arguments,
                        event_argument_references);
                }
            }
        };

        invoke_delegates_for_phase(false);
        (void)invoke_native_object_method_body_if_present(
            **source_object,
            event_name,
            source_frame,
            event_arguments,
            event_argument_references);
        invoke_delegates_for_phase(true);
        active_native_event_keys.erase(active_event_key);

        events.push_back({.category = "prg.event.raise",
                          .detail = (*source_object)->prog_id + "." + event_name,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        return make_boolean_value(true);
    }

    PrgValue PrgRuntimeSession::Impl::unbind_native_events(
        const std::vector<PrgValue> &arguments)
    {
        if (arguments.empty())
        {
            return make_number_value(0.0);
        }

        int object_handle_probe = 0;
        std::string object_prog_id_probe;
        if (!parse_object_handle_reference(arguments[0], object_handle_probe, object_prog_id_probe))
        {
            const int window_handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
            const bool has_message_filter = arguments.size() >= 2U;
            const int message = has_message_filter
                                    ? static_cast<int>(std::llround(value_as_number(arguments[1])))
                                    : 0;
            const std::size_t before_count = window_message_bindings.size();
            const auto erase_from = std::remove_if(
                window_message_bindings.begin(),
                window_message_bindings.end(),
                [&](const WindowMessageBinding &binding)
                {
                    return binding.window_handle == window_handle &&
                           (!has_message_filter || binding.message == message);
                });
            window_message_bindings.erase(erase_from, window_message_bindings.end());

            const std::size_t removed_count = before_count - window_message_bindings.size();
            if (removed_count != 0U)
            {
                events.push_back({.category = "prg.event.unbind",
                                  .detail = std::to_string(window_handle) + ":" +
                                                (has_message_filter ? std::to_string(message) : std::string("*")),
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            }
            return make_number_value(static_cast<double>(removed_count));
        }

        const std::size_t before_count = native_event_bindings.size();
        const auto erase_from = std::remove_if(
            native_event_bindings.begin(),
            native_event_bindings.end(),
            [&](const NativeEventBinding &binding)
            {
                if (arguments.size() == 1U)
                {
                    int object_handle = 0;
                    std::string object_prog_id;
                    return parse_object_handle_reference(arguments[0], object_handle, object_prog_id) &&
                           (binding.source_handle == object_handle ||
                            (!binding.target_is_routine && binding.target_handle == object_handle));
                }

                int source_handle = 0;
                std::string source_prog_id;
                if (!parse_object_handle_reference(arguments[0], source_handle, source_prog_id) ||
                    binding.source_handle != source_handle)
                {
                    return false;
                }

                const std::string event_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
                if (event_name.empty() || binding.event_name != event_name)
                {
                    return false;
                }

                if (arguments.size() == 3U && binding.target_is_routine)
                {
                    return normalize_identifier(binding.delegate_name) ==
                           normalize_identifier(trim_copy(value_as_string(arguments[2])));
                }

                if (arguments.size() >= 4U && !binding.target_is_routine)
                {
                    int target_handle = 0;
                    std::string target_prog_id;
                    return parse_object_handle_reference(arguments[2], target_handle, target_prog_id) &&
                           binding.target_handle == target_handle &&
                           normalize_identifier(binding.delegate_name) ==
                               normalize_identifier(trim_copy(value_as_string(arguments[3])));
                }

                return arguments.size() == 2U;
            });
        native_event_bindings.erase(erase_from, native_event_bindings.end());

        const std::size_t removed_count = before_count - native_event_bindings.size();
        if (removed_count != 0U)
        {
            events.push_back({.category = "prg.event.unbind",
                              .detail = std::to_string(removed_count),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        }
        return make_number_value(static_cast<double>(removed_count));
    }

    PrgValue PrgRuntimeSession::Impl::inspect_native_events(
        const std::vector<PrgValue> &arguments,
        const std::vector<std::string> &raw_arguments)
    {
        if (raw_arguments.empty() || arguments.size() < 2U)
        {
            return make_number_value(0.0);
        }

        const auto resolve_array_argument_name = [&](std::size_t index)
        {
            std::string candidate = index < raw_arguments.size() ? trim_copy(raw_arguments[index]) : std::string{};
            if (!is_bare_identifier_text(candidate) &&
                index < arguments.size() &&
                arguments[index].kind == PrgValueKind::string)
            {
                const std::string evaluated_name = trim_copy(value_as_string(arguments[index]));
                if (is_bare_identifier_text(evaluated_name))
                {
                    candidate = evaluated_name;
                }
            }
            if (is_bare_identifier_text(candidate) && !stack.empty())
            {
                Frame &frame = stack.back();
                constexpr std::size_t max_array_name_depth = 16U;
                std::vector<std::string> visited_identifiers;
                visited_identifiers.reserve(8U);
                for (std::size_t depth = 0U; depth < max_array_name_depth; ++depth)
                {
                    const std::string normalized = normalize_memory_variable_identifier(candidate);
                    if (std::find(visited_identifiers.begin(), visited_identifiers.end(), normalized) != visited_identifiers.end())
                    {
                        break;
                    }
                    visited_identifiers.push_back(normalized);

                    const PrgValue indirect_value = lookup_variable(frame, candidate);
                    if (indirect_value.kind != PrgValueKind::string)
                    {
                        break;
                    }

                    const std::string next = trim_copy(value_as_string(indirect_value));
                    if (next.empty() || next == candidate || !is_bare_identifier_text(next))
                    {
                        break;
                    }

                    candidate = next;
                }
            }
            return candidate;
        };

        const std::string array_name = resolve_array_argument_name(0U);
        if (array_name.empty())
        {
            return make_number_value(0.0);
        }

        const auto second_argument_is_zero_probe = [&]() -> bool
        {
            if (arguments.size() < 2U)
            {
                return false;
            }
            int object_handle = 0;
            std::string object_prog_id;
            if (parse_object_handle_reference(arguments[1], object_handle, object_prog_id))
            {
                return false;
            }

            switch (arguments[1].kind)
            {
            case PrgValueKind::number:
                return std::abs(arguments[1].number_value) < 0.000001;
            case PrgValueKind::int64:
                return arguments[1].int64_value == 0;
            case PrgValueKind::uint64:
                return arguments[1].uint64_value == 0U;
            case PrgValueKind::boolean:
                return !arguments[1].boolean_value;
            case PrgValueKind::string:
                return trim_copy(arguments[1].string_value) == "0";
            case PrgValueKind::empty:
                return false;
            }
            return false;
        };

        if (second_argument_is_zero_probe())
        {
            if (active_native_event_contexts.empty())
            {
                if (active_window_message_contexts.empty())
                {
                    return make_number_value(0.0);
                }

                const CurrentWindowMessageContext &message_context = active_window_message_contexts.back();
                assign_array(
                    array_name,
                    {make_int64_value(static_cast<std::int64_t>(message_context.window_handle)),
                     make_string_value(std::to_string(message_context.message)),
                     make_number_value(0.0)},
                    1U);
                return make_number_value(3.0);
            }

            const CurrentNativeEventContext &event_context = active_native_event_contexts.back();
            const auto source_found = ole_objects.find(event_context.source_handle);
            if (source_found == ole_objects.end())
            {
                return make_number_value(0.0);
            }

            assign_array(
                array_name,
                {make_string_value("object:" + source_found->second.prog_id + "#" + std::to_string(source_found->second.handle)),
                 make_string_value(event_context.event_name),
                 make_number_value(static_cast<double>(event_context.event_type))},
                1U);
            return make_number_value(3.0);
        }

        const auto second_argument_is_one_probe = [&]() -> bool
        {
            if (arguments.size() < 2U)
            {
                return false;
            }
            int object_handle = 0;
            std::string object_prog_id;
            if (parse_object_handle_reference(arguments[1], object_handle, object_prog_id))
            {
                return false;
            }

            switch (arguments[1].kind)
            {
            case PrgValueKind::number:
                return std::abs(arguments[1].number_value - 1.0) < 0.000001;
            case PrgValueKind::int64:
                return arguments[1].int64_value == 1;
            case PrgValueKind::uint64:
                return arguments[1].uint64_value == 1U;
            case PrgValueKind::boolean:
                return arguments[1].boolean_value;
            case PrgValueKind::string:
                return trim_copy(arguments[1].string_value) == "1";
            case PrgValueKind::empty:
                return false;
            }
            return false;
        };

        if (second_argument_is_one_probe())
        {
            std::vector<PrgValue> values;
            values.reserve(window_message_bindings.size() * 4U);
            for (const WindowMessageBinding &binding : window_message_bindings)
            {
                const auto target = ole_objects.find(binding.target_handle);
                values.push_back(make_int64_value(static_cast<std::int64_t>(binding.window_handle)));
                values.push_back(make_int64_value(static_cast<std::int64_t>(binding.message)));
                values.push_back(
                    target == ole_objects.end()
                        ? make_empty_value()
                        : make_string_value("object:" + target->second.prog_id + "#" + std::to_string(target->second.handle)));
                values.push_back(make_string_value(binding.delegate_name));
            }

            if (values.empty())
            {
                return make_number_value(0.0);
            }

            assign_array(array_name, std::move(values), 4U);
            return make_number_value(static_cast<double>(array_length(array_name, 1)));
        }

        int object_handle = 0;
        std::string object_prog_id;
        if (!parse_object_handle_reference(arguments[1], object_handle, object_prog_id))
        {
            return make_number_value(0.0);
        }

        std::vector<PrgValue> values;
        values.reserve(native_event_bindings.size() * 5U);
        for (const NativeEventBinding &binding : native_event_bindings)
        {
            const bool object_is_source = binding.source_handle == object_handle;
            const bool object_is_handler = !binding.target_is_routine && binding.target_handle == object_handle;
            if (!object_is_source && !object_is_handler)
            {
                continue;
            }

            values.push_back(make_boolean_value(object_is_handler));
            if (object_is_handler)
            {
                const auto source = ole_objects.find(binding.source_handle);
                values.push_back(
                    source == ole_objects.end()
                        ? make_empty_value()
                        : make_string_value("object:" + source->second.prog_id + "#" + std::to_string(source->second.handle)));
            }
            else if (binding.target_is_routine)
            {
                values.push_back(make_empty_value());
            }
            else
            {
                const auto target = ole_objects.find(binding.target_handle);
                values.push_back(
                    target == ole_objects.end()
                        ? make_empty_value()
                        : make_string_value("object:" + target->second.prog_id + "#" + std::to_string(target->second.handle)));
            }
            values.push_back(make_string_value(binding.event_name));
            values.push_back(make_string_value(binding.delegate_name));
            values.push_back(make_number_value(static_cast<double>(binding.flags)));
        }

        if (values.empty())
        {
            return make_number_value(0.0);
        }

        assign_array(array_name, std::move(values), 5U);
        return make_number_value(static_cast<double>(array_length(array_name, 1)));
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::read_native_property_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &property_name,
        const Frame &source_frame)
    {
        const std::string normalized_property_name = normalize_identifier(property_name);
        if (normalized_property_name.empty())
        {
            return std::nullopt;
        }

        const auto perform_property_read = [&]() -> std::optional<PrgValue>
        {
            if (auto access_result = invoke_native_object_method_body_if_present(
                    runtime_object,
                    normalized_property_name + "_access",
                    source_frame,
                    {},
                    {});
                access_result.has_value())
            {
                return *access_result;
            }
            if (auto metadata_value = read_native_identity_metadata(runtime_object, normalized_property_name);
                metadata_value.has_value())
            {
                return *metadata_value;
            }
            if (auto collection_value = read_native_collection_member(runtime_object, normalized_property_name);
                collection_value.has_value())
            {
                return *collection_value;
            }
            if (is_native_identity_member_name(runtime_object, normalized_property_name))
            {
                return make_empty_value();
            }
            const auto property = runtime_object.properties.find(normalized_property_name);
            if (property != runtime_object.properties.end())
            {
                return property->second;
            }
            if (is_native_olecontrol_host_object(runtime_object))
            {
                RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(runtime_object);
                if (object_surface != nullptr && object_surface->handle != runtime_object.handle)
                {
                    return read_native_property_if_present(*object_surface, property_name, source_frame);
                }
            }
            return std::nullopt;
        };

        std::vector<NativeEventBinding> bindings;
        bindings.reserve(native_event_bindings.size());
        for (const NativeEventBinding &binding : native_event_bindings)
        {
            if (binding.source_handle == runtime_object.handle &&
                binding.event_name == normalized_property_name &&
                (binding.flags & 2) == 0)
            {
                bindings.push_back(binding);
            }
        }

        const std::string active_event_key =
            std::to_string(runtime_object.handle) + ":" + normalized_property_name;
        const bool already_active =
            active_native_event_keys.find(active_event_key) != active_native_event_keys.end();

        if (!bindings.empty() && !already_active)
        {
            active_native_event_keys.insert(active_event_key);
            const auto invoke_delegates_for_phase = [&](bool after_source_member)
            {
                for (const NativeEventBinding &binding : bindings)
                {
                    const bool binding_after_source_member = (binding.flags & 1) != 0;
                    if (binding_after_source_member == after_source_member)
                    {
                        (void)invoke_native_event_delegate(
                            binding,
                            {.source_handle = runtime_object.handle,
                             .event_name = normalized_property_name,
                             .event_type = 2},
                            {},
                            {});
                    }
                }
            };

            invoke_delegates_for_phase(false);
            auto result = perform_property_read();
            invoke_delegates_for_phase(true);
            active_native_event_keys.erase(active_event_key);
            return result;
        }

        return perform_property_read();
    }

    bool PrgRuntimeSession::Impl::write_native_property_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &property_name,
        const PrgValue &assigned_value,
        const Frame &source_frame)
    {
        const std::string normalized_property_name = normalize_identifier(property_name);
        if (normalized_property_name.empty())
        {
            return false;
        }

        const auto perform_property_write = [&]() -> bool
        {
            if (invoke_native_object_method_body_if_present(
                    runtime_object,
                    normalized_property_name + "_assign",
                    source_frame,
                    {assigned_value},
                    {}).has_value())
            {
                return true;
            }
            if (is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains(normalized_property_name))
            {
                RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(runtime_object);
                if (object_surface != nullptr && object_surface->handle != runtime_object.handle &&
                    write_native_property_if_present(*object_surface, property_name, assigned_value, source_frame))
                {
                    return true;
                }
            }
            if (!is_native_identity_member_name(runtime_object, normalized_property_name) &&
                !is_native_controlcount_member_name(runtime_object, normalized_property_name) &&
                !is_native_child_collection_member_name(runtime_object, normalized_property_name) &&
                !is_native_name_member_name(runtime_object, normalized_property_name) &&
                !is_native_splitbar_member_name(runtime_object, normalized_property_name) &&
                !is_native_leftcolumn_member_name(runtime_object, normalized_property_name) &&
                !is_native_form_desktop_member_name(runtime_object, normalized_property_name) &&
                !is_native_form_scrollbars_member_name(runtime_object, normalized_property_name) &&
                !is_native_olecontrol_creation_time_member_name(runtime_object, normalized_property_name) &&
                !is_native_olecontrol_object_member_name(runtime_object, normalized_property_name) &&
                !is_native_olecontrol_inspection_member_name(runtime_object, normalized_property_name) &&
                !is_native_child_parent_member_name(runtime_object, normalized_property_name) &&
                !is_native_collection_readonly_member_name(runtime_object, normalized_property_name))
            {
                if (is_native_controlsource_member_name(runtime_object, normalized_property_name) &&
                    !is_native_column_runtime_object(runtime_object) &&
                    native_child_controlsource_write_blocked_by_parent_column(runtime_object))
                {
                    return false;
                }
                if (is_native_column_bound_member_name(runtime_object, normalized_property_name))
                {
                    return write_native_column_bound_property(runtime_object, assigned_value);
                }
                if (is_native_columnorder_member_name(runtime_object, normalized_property_name))
                {
                    return write_native_columnorder_property(runtime_object, assigned_value);
                }
                if (is_native_columncount_member_name(runtime_object, normalized_property_name) &&
                    is_native_grid_runtime_object(runtime_object))
                {
                    return write_native_grid_columncount_property(
                        runtime_object,
                        assigned_value,
                        source_frame);
                }
                if (is_native_controlsource_member_name(runtime_object, normalized_property_name) &&
                    is_native_column_runtime_object(runtime_object))
                {
                    return write_native_column_controlsource_property(runtime_object, assigned_value);
                }
                if (normalized_property_name == "readonly" &&
                    native_combobox_readonly_assignment_blocked(runtime_object, assigned_value))
                {
                    return false;
                }
                runtime_object.properties[normalized_property_name] = assigned_value;
                if (normalized_property_name == "style" ||
                    normalized_property_name == "readonly")
                {
                    normalize_native_combobox_readonly_invariant(runtime_object);
                }
                return true;
            }
            return false;
        };

        std::vector<NativeEventBinding> bindings;
        bindings.reserve(native_event_bindings.size());
        for (const NativeEventBinding &binding : native_event_bindings)
        {
            if (binding.source_handle == runtime_object.handle &&
                binding.event_name == normalized_property_name &&
                (binding.flags & 2) == 0)
            {
                bindings.push_back(binding);
            }
        }

        const std::string active_event_key =
            std::to_string(runtime_object.handle) + ":" + normalized_property_name;
        const bool already_active =
            active_native_event_keys.find(active_event_key) != active_native_event_keys.end();

        if (!bindings.empty() && !already_active)
        {
            active_native_event_keys.insert(active_event_key);
            const auto invoke_delegates_for_phase = [&](bool after_source_member)
            {
                for (const NativeEventBinding &binding : bindings)
                {
                    const bool binding_after_source_member = (binding.flags & 1) != 0;
                    if (binding_after_source_member == after_source_member)
                    {
                        (void)invoke_native_event_delegate(
                            binding,
                            {.source_handle = runtime_object.handle,
                             .event_name = normalized_property_name,
                             .event_type = 2},
                            {assigned_value},
                            {});
                    }
                }
            };

            invoke_delegates_for_phase(false);
            const bool result = perform_property_write();
            invoke_delegates_for_phase(true);
            active_native_event_keys.erase(active_event_key);
            return result;
        }

        return perform_property_write();
    }

    PrgValue PrgRuntimeSession::Impl::release_native_object(
        RuntimeOleObjectState &runtime_object,
        const std::string &effective_member_path)
    {
        struct PendingRelease
        {
            int handle = 0;
            bool children_queued = false;
        };

        std::vector<int> release_order;
        std::vector<PendingRelease> pending;
        std::set<int> scheduled_handles;
        pending.push_back({.handle = runtime_object.handle, .children_queued = false});
        scheduled_handles.insert(runtime_object.handle);

        while (!pending.empty())
        {
            const PendingRelease current = pending.back();
            pending.pop_back();

            const auto found = ole_objects.find(current.handle);
            if (found == ole_objects.end())
            {
                continue;
            }

            if (!current.children_queued)
            {
                pending.push_back({.handle = current.handle, .children_queued = true});
                std::vector<int> child_handles = collect_native_owned_child_handles(found->second);
                for (auto it = child_handles.rbegin(); it != child_handles.rend(); ++it)
                {
                    if (scheduled_handles.insert(*it).second)
                    {
                        pending.push_back({.handle = *it, .children_queued = false});
                    }
                }
                continue;
            }

            release_order.push_back(current.handle);
        }

        for (const int handle : release_order)
        {
            auto found = ole_objects.find(handle);
            if (found == ole_objects.end())
            {
                continue;
            }

            RuntimeOleObjectState &object_state = found->second;
            std::string destroy_program_path;
            std::string destroy_method_name;
            if (const Routine *destroy_method = find_native_object_method(
                    object_state,
                    "destroy",
                    destroy_program_path,
                    destroy_method_name);
                destroy_method != nullptr)
            {
                if (!can_push_frame())
                {
                    throw std::runtime_error(call_depth_limit_message());
                }

                events.push_back({.category = "prg.object.destroy",
                                  .detail = destroy_method_name,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                const std::size_t return_depth = stack.size();
                const PrgValue this_reference =
                    make_string_value("object:" + object_state.prog_id + "#" + std::to_string(object_state.handle));
                push_method_frame(destroy_program_path,
                                  destroy_method_name,
                                  *destroy_method,
                                  this_reference,
                                  destroy_method_name.substr(0U, destroy_method_name.rfind('.')),
                                  "destroy",
                                  native_object_parent_reference(object_state),
                                  native_object_owner_form_reference(object_state),
                                  native_object_owner_formset_reference(object_state),
                                  {},
                                  {});
                (void)run_expression_invoked_routine_until_return(return_depth);

                found = ole_objects.find(handle);
                if (found == ole_objects.end())
                {
                    continue;
                }
            }

            RuntimeOleObjectState &released_object = found->second;
            const auto parent_reference = native_object_parent_reference(released_object);
            if (parent_reference.has_value())
            {
                int parent_handle = 0;
                std::string parent_prog_id;
                if (parse_object_handle_reference(*parent_reference, parent_handle, parent_prog_id))
                {
                    const auto parent_found = ole_objects.find(parent_handle);
                    if (parent_found != ole_objects.end())
                    {
                        const std::string released_reference =
                            value_as_string(make_string_value("object:" + released_object.prog_id + "#" + std::to_string(released_object.handle)));
                        auto &parent_properties = parent_found->second.properties;
                        for (auto property_it = parent_properties.begin(); property_it != parent_properties.end();)
                        {
                            if (value_as_string(property_it->second) == released_reference)
                            {
                                property_it = parent_properties.erase(property_it);
                            }
                            else
                            {
                                ++property_it;
                            }
                        }
                        (void)sync_native_owned_children_collection(parent_found->second);
                    }
                }
            }

            released_object.properties.erase("parent");
            released_object.last_action = effective_member_path + "()";
            ++released_object.action_count;
            events.push_back({.category = "prg.object.release",
                              .detail = released_object.prog_id,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        }

        for (const int handle : release_order)
        {
            native_event_bindings.erase(
                std::remove_if(
                    native_event_bindings.begin(),
                    native_event_bindings.end(),
                    [handle](const NativeEventBinding &binding)
                    {
                        return binding.source_handle == handle ||
                               (!binding.target_is_routine && binding.target_handle == handle);
                    }),
                native_event_bindings.end());
            ole_objects.erase(handle);
        }

        return make_boolean_value(true);
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_expression_base_method(
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        if (source_frame.native_method_class_name.empty() || source_frame.native_method_name.empty())
        {
            return std::nullopt;
        }

        const auto this_found = source_frame.locals.find("this");
        if (this_found == source_frame.locals.end())
        {
            return std::nullopt;
        }

        auto runtime_object = resolve_ole_object(this_found->second);
        if (!runtime_object.has_value())
        {
            return std::nullopt;
        }

        Program &program = load_program(source_frame.file_path);
        std::string base_method_name;
        std::string base_defining_class_name;
        const auto base_method =
            find_native_class_method_lookup(
                program,
                source_frame.native_method_class_name,
                source_frame.native_method_name,
                false,
                base_method_name,
                &base_defining_class_name);
        if (!base_method.has_value())
        {
            return std::nullopt;
        }

        if (!can_push_frame())
        {
            throw std::runtime_error(call_depth_limit_message());
        }

        RuntimeOleObjectState *object_state = *runtime_object;
        object_state->last_action = "dodefault:" + source_frame.native_method_name;
        ++object_state->action_count;
        events.push_back({.category = "prg.object.baseinvoke",
                          .detail = base_method_name,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});

        const std::size_t return_depth = stack.size();
        const auto &effective_arguments =
            arguments.empty() ? source_frame.call_arguments : arguments;
        const auto &effective_argument_references =
            arguments.empty() ? source_frame.call_argument_references : argument_references;
        push_method_frame(base_method->program->path,
                          base_method_name,
                          *base_method->routine,
                          this_found->second,
                          base_defining_class_name,
                          source_frame.native_method_name,
                          [&source_frame]() -> std::optional<PrgValue>
                          {
                              const auto parent_found = source_frame.locals.find("parent");
                              return parent_found == source_frame.locals.end()
                                  ? std::nullopt
                                  : std::optional<PrgValue>(parent_found->second);
                          }(),
                          [&source_frame]() -> std::optional<PrgValue>
                          {
                              const auto form_found = source_frame.locals.find("thisform");
                              return form_found == source_frame.locals.end()
                                  ? std::nullopt
                                  : std::optional<PrgValue>(form_found->second);
                          }(),
                          [&source_frame]() -> std::optional<PrgValue>
                          {
                              const auto formset_found = source_frame.locals.find("thisformset");
                              return formset_found == source_frame.locals.end()
                                  ? std::nullopt
                                  : std::optional<PrgValue>(formset_found->second);
                          }(),
                          std::vector<PrgValue>(effective_arguments.begin(), effective_arguments.end()),
                          std::vector<std::optional<std::string>>(
                              effective_argument_references.begin(),
                              effective_argument_references.end()));
        return run_expression_invoked_routine_until_return(return_depth);
    }

    PrgValue PrgRuntimeSession::Impl::run_expression_invoked_routine_until_return(std::size_t return_depth)
    {
        while (true)
        {
            while (stack.size() > return_depth &&
                   (stack.back().routine == nullptr || stack.back().pc >= stack.back().routine->statements.size()))
            {
                pop_frame();
            }

            if (stack.size() < return_depth)
            {
                waiting_for_events = false;
                throw std::runtime_error(
                    runtime_text("Runtime.Prg.Expression.Error.UserRoutineAbortedExecution"));
            }

            if (stack.size() == return_depth)
            {
                return last_return_value.value_or(make_empty_value());
            }

            const Statement *next = current_statement();
            if (next == nullptr)
            {
                pop_frame();
                continue;
            }

            if (executed_statement_count >= max_executed_statements)
            {
                last_error_message = step_budget_limit_message();
                last_fault_location = next->location;
                last_fault_statement = next->text;
                events.push_back({.category = "runtime.error",
                                  .detail = last_error_message,
                                  .location = next->location});
                throw std::runtime_error(last_error_message);
            }

            const ExecutionOutcome outcome = execute_current_statement();
            if (!outcome.ok)
            {
                bool handled_by_try = false;
                if (!stack.empty())
                {
                    capture_last_error_context(stack.back(), *next);
                    handled_by_try = dispatch_try_handler(stack.back(), *next);
                }
                if (!handled_by_try)
                {
                    const std::size_t depth_at_fault = stack.size();
                    for (std::size_t index = 1U; index < depth_at_fault && !handled_by_try; ++index)
                    {
                        Frame &parent = stack[depth_at_fault - 1U - index];
                        const bool has_open_try = std::any_of(
                            parent.tries.begin(),
                            parent.tries.end(),
                            [](const TryState &state)
                            {
                                return !state.handling_error;
                            });
                        if (has_open_try)
                        {
                            while (stack.size() > depth_at_fault - index)
                            {
                                pop_frame();
                            }
                            if (!stack.empty())
                            {
                                handled_by_try = dispatch_try_handler(stack.back(), *next);
                            }
                            break;
                        }
                    }
                }
                if (handled_by_try)
                {
                    continue;
                }
                if (dispatch_error_handler())
                {
                    continue;
                }
                throw std::runtime_error(outcome.message);
            }

            if (outcome.waiting_for_events)
            {
                waiting_for_events = false;
                throw std::runtime_error(
                    runtime_text("Runtime.Prg.Expression.Error.UserRoutineEnteredEventLoop"));
            }
        }
    }

    RuntimeWatchResult PrgRuntimeSession::Impl::evaluate_watch_expression(const std::string &expression)
    {
        RuntimeWatchResult result;
        result.expression = trim_copy(expression);
        if (result.expression.empty())
        {
            result.message = runtime_text("Runtime.Prg.Watch.Error.EmptyExpression");
            return result;
        }
        if (stack.empty())
        {
            result.message = runtime_text("Runtime.Prg.Watch.Error.RequiresPausedFrame");
            return result;
        }

        try
        {
            result.value = evaluate_expression(result.expression, stack.back(), resolve_cursor_target({}));
            result.ok = true;
        }
        catch (const std::bad_alloc &)
        {
            result.message = runtime_text("Runtime.Prg.Watch.Error.OutOfMemory");
        }
        catch (const std::exception &ex)
        {
            result.message = ex.what();
        }
        catch (...)
        {
            result.message = runtime_text("Runtime.Prg.Watch.Error.Failed");
        }

        return result;
    }

    std::optional<std::string> PrgRuntimeSession::Impl::materialize_xasset_bootstrap(
        const std::string &asset_path,
        bool include_read_events)
    {
        studio::StudioOpenRequest request;
        request.path = asset_path;
        request.read_only = true;
        request.load_full_table = true;
        const auto open_result = studio::open_document(request);
        if (!open_result.ok)
        {
            last_error_message = open_result.error;
            return std::nullopt;
        }

        const XAssetExecutableModel model = build_xasset_executable_model(open_result.document);
        if (!model.ok || !model.runnable_startup)
        {
            last_error_message = model.error.empty()
                                     ? runtime_text(
                                           "Runtime.Prg.Session.Error.NoRunnableStartupMethodsFoundInAsset",
                                           {{"path", asset_path}})
                                     : model.error;
            return std::nullopt;
        }

        const std::filesystem::path asset_file(asset_path);
        const std::filesystem::path bootstrap_path =
            runtime_temp_directory /
            (asset_file.stem().string() + "_copperfin_bootstrap.prg");

        std::ofstream output(bootstrap_path, std::ios::binary);
        output << build_xasset_bootstrap_source(model, include_read_events);
        output.close();
        if (!output.good())
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.XAssetBootstrapMaterializeFailed",
                {{"path", asset_path}});
            return std::nullopt;
        }

        return bootstrap_path.string();
    }

#include "prg_engine_dispatch.inl"
    bool PrgRuntimeSession::Impl::dispatch_event_handler(const std::string &routine_name)
    {
        if (!waiting_for_events || stack.empty())
        {
            return false;
        }

        const std::string normalized_target = normalize_identifier(routine_name);
        for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator)
        {
            Program &program = load_program(iterator->file_path);
            const auto found = program.routines.find(normalized_target);
            if (found == program.routines.end())
            {
                continue;
            }

            waiting_for_events = false;
            event_dispatch_return_depth = stack.size();
            restore_event_loop_after_dispatch = true;
            if (!can_push_frame())
            {
                waiting_for_events = true;
                restore_event_loop_after_dispatch = false;
                event_dispatch_return_depth.reset();
                last_error_message = call_depth_limit_message();
                events.push_back({.category = "runtime.error",
                                  .detail = last_error_message,
                                  .location = {}});
                return false;
            }
            push_routine_frame(program.path, found->second);
            events.push_back({.category = "runtime.dispatch",
                              .detail = found->second.name,
                              .location = {}});
            return true;
        }

        return false;
    }

    std::optional<std::intptr_t> PrgRuntimeSession::Impl::dispatch_windows_message(
        std::intptr_t hwnd,
        std::uint32_t message,
        std::intptr_t wparam,
        std::intptr_t lparam)
    {
        if (!waiting_for_events || stack.empty())
        {
            return std::nullopt;
        }

        const int effective_hwnd = static_cast<int>(hwnd);
        const int effective_message = static_cast<int>(message);

        std::vector<WindowMessageBinding> bindings;
        bindings.reserve(window_message_bindings.size());
        for (const WindowMessageBinding &binding : window_message_bindings)
        {
            if (binding.window_handle == effective_hwnd &&
                binding.message == effective_message)
            {
                bindings.push_back(binding);
            }
        }
        if (bindings.empty())
        {
            for (const WindowMessageBinding &binding : window_message_bindings)
            {
                if (binding.window_handle == 0 &&
                    binding.message == effective_message)
                {
                    bindings.push_back(binding);
                }
            }
        }
        if (bindings.empty())
        {
            return std::nullopt;
        }

        const bool was_waiting_for_events = waiting_for_events;
        const bool previous_restore_event_loop_after_dispatch = restore_event_loop_after_dispatch;
        waiting_for_events = false;
        restore_event_loop_after_dispatch = true;
        struct EventLoopRestoreGuard
        {
            PrgRuntimeSession::Impl &runtime;
            bool should_restore = false;
            bool previous_restore_flag = false;

            ~EventLoopRestoreGuard()
            {
                if (!should_restore)
                {
                    return;
                }
                runtime.waiting_for_events =
                    !runtime.stack.empty() && runtime.restore_event_loop_after_dispatch;
                runtime.restore_event_loop_after_dispatch = previous_restore_flag;
            }
        } restore_guard{*this, was_waiting_for_events, previous_restore_event_loop_after_dispatch};

        std::vector<PrgValue> arguments{
            make_int64_value(static_cast<std::int64_t>(hwnd)),
            make_int64_value(static_cast<std::int64_t>(message)),
            make_int64_value(static_cast<std::int64_t>(wparam)),
            make_int64_value(static_cast<std::int64_t>(lparam))};
        const std::vector<std::optional<std::string>> argument_references(arguments.size(), std::nullopt);

        auto result_to_intptr = [](const PrgValue &value) -> std::intptr_t
        {
            switch (value.kind)
            {
            case PrgValueKind::boolean:
                return value.boolean_value ? 1 : 0;
            case PrgValueKind::number:
                return static_cast<std::intptr_t>(std::llround(value.number_value));
            case PrgValueKind::string:
            {
                const std::string trimmed = trim_copy(value.string_value);
                if (trimmed.empty())
                {
                    return 0;
                }
                try
                {
                    return static_cast<std::intptr_t>(std::stoll(trimmed));
                }
                catch (...)
                {
                    return 0;
                }
            }
            case PrgValueKind::int64:
                return static_cast<std::intptr_t>(value.int64_value);
            case PrgValueKind::uint64:
                return static_cast<std::intptr_t>(value.uint64_value);
            case PrgValueKind::empty:
                return 0;
            }
            return 0;
        };

        std::optional<std::intptr_t> last_result;
        for (const WindowMessageBinding &binding : bindings)
        {
            const auto target_found = ole_objects.find(binding.target_handle);
            if (target_found == ole_objects.end())
            {
                continue;
            }

            if (auto result = invoke_window_message_delegate(
                    binding,
                    {.window_handle = effective_hwnd,
                     .message = effective_message,
                     .wparam = wparam,
                     .lparam = lparam},
                    arguments,
                    argument_references);
                result.has_value())
            {
                last_result = result_to_intptr(*result);
            }
        }

        window_message_bindings.erase(
            std::remove_if(
                window_message_bindings.begin(),
                window_message_bindings.end(),
                [&](const WindowMessageBinding &binding)
                {
                    return ole_objects.find(binding.target_handle) == ole_objects.end();
                }),
            window_message_bindings.end());

        if (last_result.has_value())
        {
            events.push_back({.category = "runtime.dispatch",
                              .detail = "winmsg:" + std::to_string(effective_hwnd) + ":" + std::to_string(effective_message),
                              .location = {}});
        }
        return last_result;
    }

    bool PrgRuntimeSession::Impl::dispatch_error_handler()
    {
        if (handling_error)
        {
            return false;
        }
        if (stack.empty())
        {
            return false;
        }

        std::string handler = trim_copy(error_handler);
        if (handler.empty())
        {
            return false;
        }

        if (!starts_with_insensitive(handler, "DO "))
        {
            return false;
        }
        handler = trim_copy(handler.substr(3U));
        if (handler.empty())
        {
            return false;
        }

        std::string handler_arguments_clause;
        const std::size_t with_position = find_keyword_top_level(handler, "WITH");
        if (with_position != std::string::npos)
        {
            handler_arguments_clause = trim_copy(handler.substr(with_position + 4U));
            handler = trim_copy(handler.substr(0U, with_position));
        }
        if (handler.empty())
        {
            return false;
        }

        const Frame &error_frame = stack.back();
        std::vector<PrgValue> handler_arguments;
        if (!handler_arguments_clause.empty())
        {
            for (const std::string &raw_argument : split_csv_like(handler_arguments_clause))
            {
                const std::string argument_expression = trim_copy(raw_argument);
                if (!argument_expression.empty())
                {
                    handler_arguments.push_back(evaluate_expression(argument_expression, error_frame));
                }
            }
        }

        for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator)
        {
            Program &program = load_program(iterator->file_path);
            const auto found = program.routines.find(normalize_identifier(handler));
            if (found == program.routines.end())
            {
                continue;
            }
            if (!can_push_frame())
            {
                return false;
            }

            handling_error = true;
            error_metadata_stack.push_back(snapshot_current_error_metadata());
            // Save fault position for RETRY / RESUME
            fault_frame_file_path = error_frame.file_path;
            fault_frame_routine_name = error_frame.routine_name;
            fault_statement_index = error_frame.pc > 0U ? error_frame.pc - 1U : 0U;
            fault_pc_valid = true;
            error_handler_return_depth = stack.size();
            push_routine_frame(program.path, found->second, handler_arguments);
            events.push_back({.category = "runtime.error_handler",
                              .detail = handler_arguments.empty()
                                            ? found->second.name
                                            : found->second.name + " WITH " + std::to_string(handler_arguments.size()) + " argument(s)",
                              .location = {}});
            return true;
        }

        return false;
    }

    RuntimePauseState PrgRuntimeSession::Impl::run(DebugResumeAction action)
    {
        const auto finalize_pause_state = [this](DebugPauseReason reason, std::string message)
        {
            if (reason == DebugPauseReason::completed || reason == DebugPauseReason::error)
            {
                release_all_critical_sections();
            }
            return build_pause_state(reason, std::move(message));
        };

        if (entry_pause_pending)
        {
            entry_pause_pending = false;
            return finalize_pause_state(
                DebugPauseReason::entry,
                runtime_text("Runtime.Prg.Session.Message.StoppedOnEntry"));
        }

        if (waiting_for_events)
        {
            return finalize_pause_state(
                DebugPauseReason::event_loop,
                runtime_text("Runtime.Prg.Session.Message.WaitingInReadEvents"));
        }

        const std::size_t base_depth = stack.size();

        try
        {
            while (true)
            {
                if (task_cancel_requested != nullptr && task_cancel_requested->load(std::memory_order_relaxed))
                {
                    ensure_fault_context_defaults(current_statement(), last_fault_location, last_fault_statement);
                    last_error_message = runtime_text("Runtime.Prg.Core.Error.AsyncTaskCancelled");
                    events.push_back({.category = "runtime.task.cancelled",
                                      .detail = "cancelled",
                                      .location = last_fault_location});
                    rollback_active_transaction_journal();
                    rollback_active_command_undo_journal();
                    return finalize_pause_state(DebugPauseReason::error, last_error_message);
                }
                while (!stack.empty() &&
                       (stack.back().routine == nullptr || stack.back().pc >= stack.back().routine->statements.size()))
                {
                    pop_frame();
                }
                if (event_dispatch_return_depth.has_value() && stack.size() <= *event_dispatch_return_depth)
                {
                    event_dispatch_return_depth.reset();
                    if (restore_event_loop_after_dispatch)
                    {
                        restore_event_loop_after_dispatch = false;
                        waiting_for_events = true;
                        return finalize_pause_state(
                            DebugPauseReason::event_loop,
                            runtime_text("Runtime.Prg.Session.Message.WaitingInReadEvents"));
                    }
                    restore_event_loop_after_dispatch = false;
                }
                if (error_handler_return_depth.has_value() && stack.size() <= *error_handler_return_depth)
                {
                    error_handler_return_depth.reset();
                    handling_error = false;
                    if (!error_metadata_stack.empty())
                    {
                        current_data_session = std::max(1, error_metadata_stack.back().data_session);
                        if (error_metadata_stack.back().session_state_snapshot.has_value())
                        {
                            current_session_state() = *error_metadata_stack.back().session_state_snapshot;
                        }
                        error_metadata_stack.pop_back();
                    }
                }
                if (shutdown_handler_return_depth.has_value() && stack.size() <= *shutdown_handler_return_depth)
                {
                    shutdown_handler_return_depth.reset();
                    handling_shutdown = false;
                    if (quit_pending_after_shutdown)
                    {
                        perform_quit(pending_quit_location);
                        continue;
                    }
                }
                if (stack.empty())
                {
                    return finalize_pause_state(
                        DebugPauseReason::completed,
                        runtime_text("Runtime.Prg.Session.Message.ExecutionCompleted"));
                }

                const Statement *next = current_statement();
                if (next == nullptr)
                {
                    pop_frame();
                    continue;
                }

                if (executed_statement_count >= max_executed_statements)
                {
                    last_error_message = step_budget_limit_message();
                    last_fault_location = next->location;
                    last_fault_statement = next->text;
                    events.push_back({.category = "runtime.error",
                                      .detail = last_error_message,
                                      .location = next->location});
                    return finalize_pause_state(DebugPauseReason::error, last_error_message);
                }

                if (breakpoint_matches(next->location))
                {
                    if (resume_skip_breakpoint_location.has_value() &&
                        normalize_path(resume_skip_breakpoint_location->file_path) == normalize_path(next->location.file_path) &&
                        resume_skip_breakpoint_location->line == next->location.line)
                    {
                        resume_skip_breakpoint_location.reset();
                    }
                    else
                    {
                        resume_skip_breakpoint_location = next->location;
                        return finalize_pause_state(
                            DebugPauseReason::breakpoint,
                            runtime_text("Runtime.Prg.Session.Message.BreakpointHit"));
                    }
                }
                else
                {
                    resume_skip_breakpoint_location.reset();
                }

                const ExecutionOutcome outcome = execute_current_statement();
                if (!outcome.ok)
                {
                    resume_skip_breakpoint_location.reset();
                    bool handled_by_try = false;
                    if (!stack.empty())
                    {
                        capture_last_error_context(stack.back(), *next);
                        handled_by_try = dispatch_try_handler(stack.back(), *next);
                    }
                    if (!handled_by_try)
                    {
                        // Walk parent frames: TRY/CATCH in a caller should catch
                        // faults that propagate through nested DO calls.
                        const std::size_t depth_at_fault = stack.size();
                        for (std::size_t i = 1U; i < depth_at_fault && !handled_by_try; ++i)
                        {
                            Frame &parent = stack[depth_at_fault - 1U - i];
                            const bool has_open_try = std::any_of(
                                parent.tries.begin(), parent.tries.end(),
                                [](const TryState &t) { return !t.handling_error; });
                            if (has_open_try)
                            {
                                // Pop intermediate frames back to this parent.
                                while (stack.size() > depth_at_fault - i)
                                {
                                    pop_frame();
                                }
                                if (!stack.empty())
                                {
                                    handled_by_try = dispatch_try_handler(stack.back(), *next);
                                }
                                break;
                            }
                        }
                    }
                    if (handled_by_try)
                    {
                        continue;
                    }
                    if (dispatch_error_handler())
                    {
                        continue;
                    }
                    return finalize_pause_state(DebugPauseReason::error, outcome.message);
                }
                if (outcome.waiting_for_events)
                {
                    return finalize_pause_state(
                        DebugPauseReason::event_loop,
                        runtime_text("Runtime.Prg.Session.Message.WaitingInReadEvents"));
                }

                if (stack.empty())
                {
                    return finalize_pause_state(
                        DebugPauseReason::completed,
                        runtime_text("Runtime.Prg.Session.Message.ExecutionCompleted"));
                }

                switch (action)
                {
                case DebugResumeAction::continue_run:
                    if (scheduler_yield_statement_interval != 0U &&
                        (executed_statement_count % scheduler_yield_statement_interval) == 0U)
                    {
                        if (scheduler_yield_sleep_ms == 0U)
                        {
                            std::this_thread::yield();
                        }
                        else
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(scheduler_yield_sleep_ms));
                        }
                    }
                    break;
                case DebugResumeAction::step_into:
                    return finalize_pause_state(
                        DebugPauseReason::step,
                        runtime_text("Runtime.Prg.Session.Message.StepCompleted"));
                case DebugResumeAction::step_over:
                    if (stack.size() <= base_depth)
                    {
                        return finalize_pause_state(
                            DebugPauseReason::step,
                            runtime_text("Runtime.Prg.Session.Message.StepOverCompleted"));
                    }
                    break;
                case DebugResumeAction::step_out:
                    if (stack.size() < base_depth)
                    {
                        return finalize_pause_state(
                            DebugPauseReason::step,
                            runtime_text("Runtime.Prg.Session.Message.StepOutCompleted"));
                    }
                    break;
                }
            }
        }
        catch (const std::bad_alloc &)
        {
            ensure_fault_context_defaults(current_statement(), last_fault_location, last_fault_statement);
            last_error_compatibility = {};
            last_error_message = runtime_text("Runtime.Prg.Core.Error.ResourceOutOfMemory");
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            if (!stack.empty())
            {
                fault_frame_file_path = stack.back().file_path;
                fault_frame_routine_name = stack.back().routine_name;
                fault_statement_index = stack.back().pc > 0U ? stack.back().pc - 1U : 0U;
                fault_pc_valid = true;
            }
            error_metadata_stack.push_back(snapshot_current_error_metadata());
            return finalize_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (const std::filesystem::filesystem_error &error)
        {
            ensure_fault_context_defaults(current_statement(), last_fault_location, last_fault_statement);
            last_error_compatibility = {};
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.ResourceFilesystemFailure",
                {{"detail", error.what()}});
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            if (!stack.empty())
            {
                fault_frame_file_path = stack.back().file_path;
                fault_frame_routine_name = stack.back().routine_name;
                fault_statement_index = stack.back().pc > 0U ? stack.back().pc - 1U : 0U;
                fault_pc_valid = true;
            }
            error_metadata_stack.push_back(snapshot_current_error_metadata());
            return finalize_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (const std::system_error &error)
        {
            ensure_fault_context_defaults(current_statement(), last_fault_location, last_fault_statement);
            last_error_compatibility = {};
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.ResourceSystemError",
                {{"detail", error.what()}});
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            if (!stack.empty())
            {
                fault_frame_file_path = stack.back().file_path;
                fault_frame_routine_name = stack.back().routine_name;
                fault_statement_index = stack.back().pc > 0U ? stack.back().pc - 1U : 0U;
                fault_pc_valid = true;
            }
            error_metadata_stack.push_back(snapshot_current_error_metadata());
            return finalize_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (const std::exception &error)
        {
            ensure_fault_context_defaults(current_statement(), last_fault_location, last_fault_statement);
            last_error_compatibility = {};
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.RuntimeFault",
                {{"detail", error.what()}});
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            if (!stack.empty())
            {
                fault_frame_file_path = stack.back().file_path;
                fault_frame_routine_name = stack.back().routine_name;
                fault_statement_index = stack.back().pc > 0U ? stack.back().pc - 1U : 0U;
                fault_pc_valid = true;
            }
            error_metadata_stack.push_back(snapshot_current_error_metadata());
            return finalize_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (...)
        {
            ensure_fault_context_defaults(current_statement(), last_fault_location, last_fault_statement);
            last_error_compatibility = {};
            last_error_message = runtime_text("Runtime.Prg.Core.Error.UnknownRuntimeFault");
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            if (!stack.empty())
            {
                fault_frame_file_path = stack.back().file_path;
                fault_frame_routine_name = stack.back().routine_name;
                fault_statement_index = stack.back().pc > 0U ? stack.back().pc - 1U : 0U;
                fault_pc_valid = true;
            }
            error_metadata_stack.push_back(snapshot_current_error_metadata());
            return finalize_pause_state(DebugPauseReason::error, last_error_message);
        }
    }

    PrgRuntimeSession PrgRuntimeSession::create(const RuntimeSessionOptions &options)
    {
        RuntimeSessionOptions effective = options;
        effective.startup_path = normalize_path(effective.startup_path);
        effective.working_directory = effective.working_directory.empty()
                                          ? std::filesystem::path(effective.startup_path).parent_path().string()
                                          : normalize_path(effective.working_directory);

        if (const auto config = load_runtime_config_near(
                std::filesystem::path(effective.startup_path),
                std::filesystem::path(effective.working_directory)))
        {
            apply_runtime_config_defaults(effective, *config);
        }

        auto impl = std::make_unique<Impl>(effective);
        impl->startup_default_directory = effective.working_directory;
        impl->default_directory_by_session.emplace(1, impl->startup_default_directory);
        impl->data_sessions.try_emplace(1);
        impl->events.push_back({.category = "runtime.config",
                                .detail = "temp=" + impl->runtime_temp_directory.string() +
                                          ";max_call_depth=" + std::to_string(impl->max_call_depth) +
                                          ";max_executed_statements=" + std::to_string(impl->max_executed_statements) +
                                          ";max_loop_iterations=" + std::to_string(impl->max_loop_iterations),
                                .location = {}});
        impl->replay_pending_transaction_journals();
        impl->push_main_frame(effective.startup_path);
        impl->entry_pause_pending = effective.stop_on_entry;
        return PrgRuntimeSession(std::move(impl));
    }

    void PrgRuntimeSession::add_breakpoint(const RuntimeBreakpoint &breakpoint)
    {
        const RuntimeBreakpoint normalized{
            .file_path = normalize_path(breakpoint.file_path),
            .line = breakpoint.line
        };
        const auto existing = std::find_if(
            impl_->breakpoints.begin(),
            impl_->breakpoints.end(),
            [&](const RuntimeBreakpoint& candidate) {
                return candidate.file_path == normalized.file_path &&
                    candidate.line == normalized.line;
            });
        if (existing == impl_->breakpoints.end())
        {
            impl_->breakpoints.push_back(normalized);
        }
    }

    bool PrgRuntimeSession::remove_breakpoint(const RuntimeBreakpoint &breakpoint)
    {
        const RuntimeBreakpoint normalized{
            .file_path = normalize_path(breakpoint.file_path),
            .line = breakpoint.line
        };
        const auto existing = std::find_if(
            impl_->breakpoints.begin(),
            impl_->breakpoints.end(),
            [&](const RuntimeBreakpoint& candidate) {
                return candidate.file_path == normalized.file_path &&
                    candidate.line == normalized.line;
            });
        if (existing == impl_->breakpoints.end())
        {
            return false;
        }
        impl_->breakpoints.erase(existing);
        return true;
    }

    void PrgRuntimeSession::clear_breakpoints()
    {
        impl_->breakpoints.clear();
    }

    std::vector<RuntimeBreakpoint> PrgRuntimeSession::list_breakpoints() const
    {
        return impl_->breakpoints;
    }

    bool PrgRuntimeSession::dispatch_event_handler(const std::string &routine_name)
    {
        return impl_->dispatch_event_handler(routine_name);
    }

    std::optional<std::intptr_t> PrgRuntimeSession::dispatch_windows_message(
        std::intptr_t hwnd,
        std::uint32_t message,
        std::intptr_t wparam,
        std::intptr_t lparam)
    {
        return impl_->dispatch_windows_message(hwnd, message, wparam, lparam);
    }

    bool PrgRuntimeSession::can_undo_command() const
    {
        return impl_->can_undo_command();
    }

    std::string PrgRuntimeSession::command_undo_label() const
    {
        return impl_->command_undo_label();
    }

    RuntimeWatchResult PrgRuntimeSession::evaluate_watch_expression(const std::string &expression)
    {
        return impl_->evaluate_watch_expression(expression);
    }

    RuntimePauseState PrgRuntimeSession::run(DebugResumeAction action)
    {
        return impl_->run(action);
    }

    const RuntimePauseState &PrgRuntimeSession::state() const noexcept
    {
        return impl_->last_state;
    }

    PrgRuntimeSession::PrgRuntimeSession(std::unique_ptr<Impl> impl)
        : impl_(std::move(impl))
    {
    }

    PrgRuntimeSession::PrgRuntimeSession(PrgRuntimeSession &&) noexcept = default;

    PrgRuntimeSession &PrgRuntimeSession::operator=(PrgRuntimeSession &&) noexcept = default;

    PrgRuntimeSession::~PrgRuntimeSession() = default;

    const char *debug_pause_reason_name(DebugPauseReason reason)
    {
        switch (reason)
        {
        case DebugPauseReason::none:
            return "none";
        case DebugPauseReason::entry:
            return "entry";
        case DebugPauseReason::breakpoint:
            return "breakpoint";
        case DebugPauseReason::step:
            return "step";
        case DebugPauseReason::event_loop:
            return "event_loop";
        case DebugPauseReason::completed:
            return "completed";
        case DebugPauseReason::error:
            return "error";
        }
        return "none";
    }

    std::string format_value(const PrgValue &value)
    {
        return value_as_string(value);
    }

} // namespace copperfin::runtime

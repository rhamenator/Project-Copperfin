#include "copperfin/runtime/prg_engine.h"
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
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <new>
#include <optional>
#include <set>
#include <sstream>
#include <system_error>
#include <thread>
#include <chrono>
#include <utility>

#if defined(_WIN32)
#include <process.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")
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

        struct LoopState
        {
            std::size_t for_statement_index = 0;
            std::size_t endfor_statement_index = 0;
            std::string variable_name;
            double end_value = 0.0;
            double step_value = 1.0;
            std::size_t iteration_count = 0;
            // FOR EACH support
            bool is_for_each = false;
            std::vector<PrgValue> each_values; // snapshot of collection at entry
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
            std::optional<std::size_t> catch_statement_index;
            std::optional<std::size_t> finally_statement_index;
            std::size_t endtry_statement_index = 0;
            std::string catch_variable;
            bool handling_error = false;
            bool entered_catch = false;
            bool entered_finally = false;
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
            std::map<std::string, TransactionJournalFileEntry> tracked_files;
        };

#include "prg_engine_free_functions.inl"
    } // namespace

    struct PrgRuntimeSession::Impl
    {
        explicit Impl(RuntimeSessionOptions session_options)
            : options(std::move(session_options))
        {
            max_call_depth = std::max<std::size_t>(1U, options.max_call_depth);
            max_executed_statements = std::max<std::size_t>(1U, options.max_executed_statements);
            max_loop_iterations = std::max<std::size_t>(1U, options.max_loop_iterations);
            scheduler_yield_statement_interval = std::max<std::size_t>(1U, options.scheduler_yield_statement_interval);
            scheduler_yield_sleep_ms = options.scheduler_yield_sleep_ms;
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
        };

        RuntimeSessionOptions options;
        std::map<std::string, Program> programs;
        std::vector<Frame> stack;
        std::map<std::string, PrgValue> globals;
        std::map<std::string, RuntimeArray> arrays;
        std::vector<RuntimeBreakpoint> breakpoints;
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
        std::string error_handler;
        std::string shutdown_handler;
        std::map<int, std::map<std::string, std::string>> set_state_by_session;
        int current_data_session = 1;
        std::map<int, int> next_sql_handle_by_session;
        std::map<int, int> next_api_handle_by_session;
        std::map<int, int> transaction_level_by_session;
        std::map<int, TransactionJournalState> transaction_journal_by_session;
        int next_ole_handle = 1;
        std::map<int, DataSessionState> data_sessions;
        std::map<int, std::string> default_directory_by_session;
        std::map<int, std::size_t> memowidth_by_session;
        std::map<int, std::map<int, RuntimeSqlConnectionState>> sql_connections_by_session;
        std::map<int, RuntimeOleObjectState> ole_objects;
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
        std::size_t scheduler_yield_statement_interval = 4096;
        std::size_t scheduler_yield_sleep_ms = 1;

#include "prg_engine_session.inl"
#include "prg_engine_cursor.inl"
#include "prg_engine_records.inl"
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
        bool dispatch_error_handler();
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
            last_error_message,
            last_error_code,
            last_error_procedure,
            last_fault_location.line,
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
            []()
            {
                return 1001;
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
            [this](int handle, const std::string &property_name)
            {
                return sql_get_prop(handle, property_name);
            },
            [this](int handle, const std::string &property_name, const PrgValue &value)
            {
                return sql_set_prop(handle, property_name, value);
            },
            [this](const std::string &prog_id, const std::string &source)
            {
                return register_ole_object(prog_id, source);
            },
            [this, &frame](const std::string &base_name, const std::string &member_path, const std::vector<PrgValue> &arguments)
            {
                const PrgValue object_value = lookup_variable(frame, base_name);
                auto object = resolve_ole_object(object_value);
                if (!object.has_value())
                {
                    return make_empty_value();
                }

                RuntimeOleObjectState *runtime_object = *object;
                runtime_object->last_action = member_path + "()";
                ++runtime_object->action_count;
                events.push_back({.category = "ole.invoke",
                                  .detail = runtime_object->prog_id + "." + member_path,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});

                const std::string leaf = member_path.substr(member_path.rfind('.') == std::string::npos ? 0U : member_path.rfind('.') + 1U);
                if (leaf == "add" || leaf == "create" || leaf == "open" || leaf == "item")
                {
                    return make_string_value("object:" + runtime_object->prog_id + "." + member_path + "#" + std::to_string(runtime_object->handle));
                }
                if (arguments.empty())
                {
                    return make_string_value("ole:" + runtime_object->prog_id + "." + member_path);
                }
                return arguments.front();
            },
            [this, &frame](const std::string &property_path)
            {
                const auto separator = property_path.find('.');
                if (separator == std::string::npos)
                {
                    return make_empty_value();
                }

                const PrgValue object_value = lookup_variable(frame, property_path.substr(0U, separator));
                auto object = resolve_ole_object(object_value);
                if (!object.has_value())
                {
                    return make_empty_value();
                }

                RuntimeOleObjectState *runtime_object = *object;
                runtime_object->last_action = property_path.substr(separator + 1U);
                ++runtime_object->action_count;
                events.push_back({.category = "ole.get",
                                  .detail = runtime_object->prog_id + "." + property_path.substr(separator + 1U),
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                return make_string_value("ole:" + runtime_object->prog_id + "." + property_path.substr(separator + 1U));
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
                        return std::string("0");
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
            [this](const std::string &name, std::vector<PrgValue> values)
            {
                assign_array(name, std::move(values));
            },
            [this]()
            {
                const auto found = memowidth_by_session.find(current_data_session);
                return found != memowidth_by_session.end() ? found->second : 50U;
            },
            [this](const std::string &fn_key, const std::vector<PrgValue> &fn_args) -> PrgValue
            {
                return invoke_declared_dll_function(fn_key, fn_args);
            });
        return parser.parse();
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
                                     ? "No runnable startup methods were found in asset: " + asset_path
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
            last_error_message = "Unable to materialize xAsset bootstrap for: " + asset_path;
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
        if (entry_pause_pending)
        {
            entry_pause_pending = false;
            return build_pause_state(DebugPauseReason::entry, "Stopped on entry.");
        }

        if (waiting_for_events)
        {
            return build_pause_state(DebugPauseReason::event_loop, "The runtime is waiting in READ EVENTS.");
        }

        const std::size_t base_depth = stack.size();
        bool first_statement = true;

        try
        {
            while (true)
            {
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
                        return build_pause_state(DebugPauseReason::event_loop, "The runtime is waiting in READ EVENTS.");
                    }
                    restore_event_loop_after_dispatch = false;
                }
                if (error_handler_return_depth.has_value() && stack.size() <= *error_handler_return_depth)
                {
                    error_handler_return_depth.reset();
                    handling_error = false;
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
                    return build_pause_state(DebugPauseReason::completed, "Execution completed.");
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
                    return build_pause_state(DebugPauseReason::error, last_error_message);
                }

                if (!first_statement && breakpoint_matches(next->location))
                {
                    return build_pause_state(DebugPauseReason::breakpoint, "Breakpoint hit.");
                }

                const ExecutionOutcome outcome = execute_current_statement();
                if (!outcome.ok)
                {
                    if (!stack.empty())
                    {
                        capture_last_error_context(stack.back(), *next);
                        if (dispatch_try_handler(stack.back(), *next))
                        {
                            continue;
                        }
                    }
                    if (dispatch_error_handler())
                    {
                        continue;
                    }
                    return build_pause_state(DebugPauseReason::error, outcome.message);
                }
                if (outcome.waiting_for_events)
                {
                    return build_pause_state(DebugPauseReason::event_loop, "The runtime is waiting in READ EVENTS.");
                }

                if (stack.empty())
                {
                    return build_pause_state(DebugPauseReason::completed, "Execution completed.");
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
                    return build_pause_state(DebugPauseReason::step, "Step completed.");
                case DebugResumeAction::step_over:
                    if (stack.size() <= base_depth)
                    {
                        return build_pause_state(DebugPauseReason::step, "Step-over completed.");
                    }
                    break;
                case DebugResumeAction::step_out:
                    if (stack.size() < base_depth)
                    {
                        return build_pause_state(DebugPauseReason::step, "Step-out completed.");
                    }
                    break;
                }

                first_statement = false;
            }
        }
        catch (const std::bad_alloc &)
        {
            last_error_message = "Runtime resource fault: out of memory. Execution paused safely.";
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (const std::filesystem::filesystem_error &error)
        {
            last_error_message = std::string("Runtime resource fault: filesystem failure: ") + error.what();
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (const std::system_error &error)
        {
            last_error_message = std::string("Runtime resource fault: system error: ") + error.what();
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (const std::exception &error)
        {
            last_error_message = std::string("Runtime fault: ") + error.what();
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
        }
        catch (...)
        {
            last_error_message = "Runtime fault: unknown exception";
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = last_fault_location});
            return build_pause_state(DebugPauseReason::error, last_error_message);
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
        impl_->breakpoints.push_back({.file_path = normalize_path(breakpoint.file_path),
                                      .line = breakpoint.line});
    }

    void PrgRuntimeSession::clear_breakpoints()
    {
        impl_->breakpoints.clear();
    }

    bool PrgRuntimeSession::dispatch_event_handler(const std::string &routine_name)
    {
        return impl_->dispatch_event_handler(routine_name);
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

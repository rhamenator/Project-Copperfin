// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/runtime/index_seek_optimizer.h"
#include "copperfin/platform/environment.h"
#include "copperfin/platform/path.h"
#include "localized_text.h"
#include "prg_engine_command_helpers.h"
#include "prg_engine_helpers.h"
#include "prg_engine_internal.h"
#include "prg_engine_file_io_functions.h"
#include "prg_engine_runtime_config.h"
#include "prg_engine_locale_code_page.h"
#include "prg_engine_runtime_surface_functions.h"
#include "prg_engine_table_structure_helpers.h"
#include "prg_engine_date_time_functions.h"
#include "prg_engine_string_functions.h"
#include "win64_native_call.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/report_layout.h"
#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/index_probe.h"
#include "copperfin/vfp/sidecar_path.h"

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
#include <limits>
#include <mutex>
#include <new>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
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
#include <oleauto.h>
#include "managed_declared_call.h"
#include "managed_pe_image.h"

#else
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
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
        constexpr std::uint32_t kCopperfinWindowCloseMessage = 0x0010U;
        constexpr std::uint32_t kWindowsMouseMoveMessage = 0x0200U;
        constexpr std::uint32_t kWindowsLeftButtonDownMessage = 0x0201U;
        constexpr std::uint32_t kWindowsKeyDownMessage = 0x0100U;
        constexpr std::uint32_t kWindowsLeftButtonUpMessage = 0x0202U;
        constexpr std::uint32_t kWindowsLeftButtonDoubleClickMessage = 0x0203U;
        constexpr std::uint32_t kWindowsRightButtonDownMessage = 0x0204U;
        constexpr std::uint32_t kWindowsRightButtonUpMessage = 0x0205U;
        constexpr std::uint32_t kWindowsMiddleButtonDownMessage = 0x0207U;
        constexpr std::uint32_t kWindowsMiddleButtonUpMessage = 0x0208U;
        constexpr std::intptr_t kWindowsAltContextBit = static_cast<std::intptr_t>(1) << 29;

        class PrgCompatibilityError final : public std::runtime_error
        {
        public:
            PrgCompatibilityError(std::string message, int error_code)
                : std::runtime_error(std::move(message)),
                  error_code_(error_code)
            {
            }

            [[nodiscard]] int error_code() const noexcept
            {
                return error_code_;
            }

        private:
            int error_code_;
        };

        class PrgPropagatedRuntimeError final : public std::runtime_error
        {
        public:
            using std::runtime_error::runtime_error;
        };

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

        std::string serialize_runtime_expression_text(const PrgValue &value)
        {
            switch (value.kind)
            {
            case PrgValueKind::boolean:
                return value.boolean_value ? ".T." : ".F.";
            case PrgValueKind::number:
            case PrgValueKind::int64:
            case PrgValueKind::uint64:
                return value_as_string(value);
            case PrgValueKind::currency:
                return "VAL(\"$" + value_as_string(value) + "\")";
            case PrgValueKind::string:
            {
                std::string quoted = value.string_value;
                std::string escaped;
                escaped.reserve(quoted.size());
                for (const char ch : quoted)
                {
                    if (ch == '"')
                    {
                        escaped += "\"\"";
                    }
                    else
                    {
                        escaped.push_back(ch);
                    }
                }
                return "\"" + escaped + "\"";
            }
            case PrgValueKind::empty:
            default:
                return value_as_string(value);
            }
        }

        std::string serialize_insert_value_expression(const PrgValue &value)
        {
            if (value.is_null)
            {
                return ".NULL.";
            }
            if (value.kind == PrgValueKind::empty)
            {
                return "\"\"";
            }
            if (value.kind == PrgValueKind::number)
            {
                std::ostringstream stream;
                stream << std::setprecision(std::numeric_limits<double>::max_digits10)
                       << value.number_value;
                return stream.str();
            }
            return serialize_runtime_expression_text(value);
        }

        std::string serialize_insert_row_expression_list(const std::vector<PrgValue> &row)
        {
            std::string result;
            for (std::size_t index = 0U; index < row.size(); ++index)
            {
                if (index != 0U)
                {
                    result.push_back(',');
                }
                result += serialize_insert_value_expression(row[index]);
            }
            return result;
        }

        std::string make_native_method_override_key(
            const std::string &program_path,
            const std::string &qualified_method_name)
        {
            return normalize_path(program_path) + ":" + normalize_identifier(qualified_method_name);
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
                   normalized_base_class == "optiongroup" ||
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
            std::size_t case_stack_depth_at_entry = 0;
            std::size_t with_stack_depth_at_entry = 0;
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
            std::size_t case_stack_depth_at_entry = 0;
            std::size_t with_stack_depth_at_entry = 0;
            int work_area = 0;
            std::string for_expression;
            std::string while_expression;
            std::size_t iteration_count = 0;
        };

        struct WhileState
        {
            std::size_t do_while_statement_index = 0;
            std::size_t enddo_statement_index = 0;
            std::size_t case_stack_depth_at_entry = 0;
            std::size_t with_stack_depth_at_entry = 0;
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
            std::size_t with_stack_depth_at_try_entry = 0;
            std::size_t case_stack_depth_at_try_entry = 0;
            std::vector<std::size_t> catch_statement_indices;
            std::optional<std::size_t> finally_statement_index;
            std::size_t endtry_statement_index = 0;
            bool handling_error = false;
            bool entered_catch = false;
            bool entered_finally = false;
            bool propagate_after_finally = false;
            bool return_after_finally = false;
        };

        bool try_state_can_process_fault(const TryState &state)
        {
            if (state.entered_finally)
            {
                return false;
            }
            if (!state.handling_error)
            {
                return true;
            }
            return state.finally_statement_index.has_value();
        }

        struct RuntimeArray
        {
            std::size_t rows = 0;
            std::size_t columns = 1;
            std::vector<PrgValue> values;
        };

        struct ExpressionPrimaryCheckpoint
        {
            std::size_t end = 0U;
            PrgValue value;
        };

        struct ExpressionContinuation
        {
            Statement statement;
            std::map<std::size_t, ExpressionPrimaryCheckpoint> primary_checkpoints;
            std::map<std::pair<std::size_t, std::size_t>, PrgValue> routine_results;
            std::optional<std::pair<std::size_t, std::size_t>> awaiting_routine;
        };

        struct ExpressionSuspended
        {
        };

        struct CommandArgumentContinuation
        {
            Statement statement;
            std::string target;
            std::vector<std::string> argument_expressions;
            std::size_t next_argument_index = 0U;
            std::vector<PrgValue> values;
            std::vector<std::optional<std::string>> references;
        };

        struct CommandTargetContinuation
        {
            Statement statement;
        };

        struct CommandArrayNameContinuation
        {
            Statement statement;
        };

        struct TextMergeContinuation
        {
            Statement statement;
            std::string source_text;
            std::string left_delimiter;
            std::string right_delimiter;
            std::string merged_text;
            std::size_t cursor = 0U;
            std::size_t pending_expression_end = 0U;
            bool pending_expression = false;
        };

        struct ParameterDefaultContinuation
        {
            Statement statement;
            std::size_t next_parameter_index = 0U;
            bool pending_default = false;
        };

        struct UseCommandContinuation
        {
            Statement statement;
            std::optional<PrgValue> target_value;
            bool pending_alias = false;
        };

        struct CopyFileContinuation
        {
            Statement statement;
            std::optional<PrgValue> source_value;
            bool pending_destination = false;
        };

        struct RenameFileContinuation
        {
            Statement statement;
            std::optional<PrgValue> source_value;
            bool pending_destination = false;
        };

        enum class LoopExpressionStage
        {
            for_start,
            for_end,
            for_step,
            do_while_predicate,
            for_each_collection
        };

        struct LoopExpressionContinuation
        {
            Statement statement;
            LoopExpressionStage stage = LoopExpressionStage::for_start;
            double start_value = 0.0;
            double end_value = 0.0;
        };

        enum class ScanExpressionStage
        {
            while_predicate,
            cursor_filter,
            for_predicate
        };

        enum class ScanSearchKind
        {
            enter_scan,
            continue_scan
        };

        struct ScanExpressionContinuation
        {
            Statement statement;
            ScanExpressionStage stage = ScanExpressionStage::while_predicate;
            ScanSearchKind kind = ScanSearchKind::enter_scan;
            int work_area = 0;
            std::size_t candidate_recno = 1U;
            std::size_t scan_statement_index = 0U;
            std::size_t endscan_statement_index = 0U;
            std::size_t iteration_count = 0U;
            bool jump_after_completion = false;
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
            std::map<std::string, std::string> array_reference_bindings;
            std::set<std::string> local_names;
            std::map<std::string, RuntimeArray> local_arrays;
            std::map<std::string, std::optional<PrgValue>> private_saved_values;
            std::map<std::string, std::optional<RuntimeArray>> private_saved_arrays;
            std::vector<LoopState> loops;
            std::vector<ScanState> scans;
            std::vector<WhileState> whiles;
            std::vector<CaseState> cases;
            std::vector<WithState> withs;
            std::vector<TryState> tries;
            bool procedure_context = false;
            std::string native_method_class_name;
            std::string native_method_name;
            bool requested_nodefault = false;
            bool return_pending = false;
            bool expression_routine_return_pending = false;
            std::optional<ExpressionContinuation> expression_continuation;
            std::optional<CommandTargetContinuation> command_target_continuation;
            std::optional<CommandArrayNameContinuation> command_array_name_continuation;
            std::optional<CommandArgumentContinuation> command_argument_continuation;
            std::optional<TextMergeContinuation> text_merge_continuation;
            std::optional<ParameterDefaultContinuation> parameter_default_continuation;
            std::optional<UseCommandContinuation> use_command_continuation;
            std::optional<CopyFileContinuation> copy_file_continuation;
            std::optional<RenameFileContinuation> rename_file_continuation;
            std::optional<LoopExpressionContinuation> loop_expression_continuation;
            std::optional<ScanExpressionContinuation> scan_expression_continuation;
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
            std::string active_locate_for_expression;
            std::string active_locate_while_expression;
            bool locate_active = false;
            std::string filter_expression;
            int buffering_mode = 1;
            std::map<std::size_t, vfp::DbfRecord> buffered_records;
            std::map<std::size_t, vfp::DbfRecord> buffered_original_records;
            std::set<std::size_t> buffered_record_locks;
            std::set<std::size_t> buffered_appended_records;
            std::vector<vfp::DbfRecord> remote_records;
            std::vector<vfp::DbfFieldDescriptor> remote_fields;
            std::vector<vfp::DbfFieldDescriptor> local_fields;
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
            std::string alias;                  // Name used in PRG code (may equal function_name)
            std::string function_name;          // Export name written in the source declaration
            std::string dll_path;               // Source-facing DLL/FLL/.NET assembly designator
            std::string loaded_module_path;     // Actual native module selected by the loader
            std::string resolved_function_name; // Actual native export, including A fallback
            std::string return_type;            // e.g. "INTEGER", "STRING", "DOUBLE", etc.
            std::string param_types;            // Comma-separated param types
            bool is_dotnet = false;
#if defined(_WIN32)
            HMODULE hmodule = nullptr;
            FARPROC proc_address = nullptr;
            bool native_cdecl = false;
#endif
            // .NET-specific (assembly!Namespace.Type.Method)
            std::string dotnet_type_name;
            std::string dotnet_method_name;
        };

        struct DataSessionState
        {
            struct RelationState
            {
                int parent_work_area = 0;
                int child_work_area = 0;
                std::string expression;
                bool skip_one_to_many = false;
            };

            struct PopupActionRoutine
            {
                std::string action_text;
                std::string source_path;
                std::string routine_name;
            };

            int selected_work_area = 1;
            int next_work_area = 1;
            std::map<int, std::string> aliases;
            std::map<int, CursorState> cursors;
            std::vector<RelationState> relations;
            std::set<int> table_locks;
            std::map<int, std::set<std::size_t>> record_locks;
            std::vector<std::string> key_stack;
            std::vector<std::string> menu_stack;
            std::vector<std::string> popup_stack;
            std::set<std::string> defined_menus;
            std::map<std::string, std::map<long long, std::string>> popup_bar_prompts;
            std::map<std::string, std::map<long long, bool>> popup_bar_skip_states;
            std::map<std::string, std::map<long long, bool>> popup_bar_mark_states;
            std::map<std::string, std::map<long long, std::string>> popup_bar_selection_handlers;
            std::map<std::string, std::map<long long, std::string>> popup_bar_selection_actions;
            std::map<std::string, std::map<long long, PopupActionRoutine>> popup_bar_action_routines;
            std::map<std::string, std::map<long long, std::string>> popup_bar_activation_targets;
            std::map<std::string, std::string> popup_selection_handlers;
            std::vector<RuntimeDatabaseState> databases;
            std::string current_database_path;
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

        struct ActiveNativeEventKeyGuard
        {
            std::set<std::string> &active_keys;
            std::string key;
            bool engaged = false;

            ActiveNativeEventKeyGuard(std::set<std::string> &keys, std::string event_key)
                : active_keys(keys),
                  key(std::move(event_key))
            {
                engaged = active_keys.insert(key).second;
            }

            ~ActiveNativeEventKeyGuard()
            {
                if (engaged)
                {
                    active_keys.erase(key);
                }
            }
        };

        struct ActiveNativePropertyAssignmentGuard
        {
            std::set<std::string> &active_keys;
            std::string key;
            bool engaged = false;

            ActiveNativePropertyAssignmentGuard(std::set<std::string> &keys, std::string property_key)
                : active_keys(keys),
                  key(std::move(property_key))
            {
                engaged = active_keys.insert(key).second;
            }

            ~ActiveNativePropertyAssignmentGuard()
            {
                if (engaged)
                {
                    active_keys.erase(key);
                }
            }
        };

#include "prg_engine_free_functions.inl"

        std::filesystem::path make_prg_engine_xasset_bootstrap_path(
            const std::filesystem::path &runtime_temp_directory,
            const std::filesystem::path &asset_file,
            std::uint64_t runtime_instance_id)
        {
            static std::atomic<unsigned long long> bootstrap_nonce_counter{0ULL};
            const auto now_ticks = static_cast<unsigned long long>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count());
            const auto nonce_counter = bootstrap_nonce_counter.fetch_add(1ULL, std::memory_order_relaxed);
            return runtime_temp_directory /
                (copperfin::platform::path_to_utf8_string(asset_file.stem()) +
                 "_copperfin_bootstrap_" +
                 std::to_string(now_ticks) + "_" +
                 std::to_string(static_cast<unsigned long long>(current_process_id())) + "_" +
                 std::to_string(static_cast<unsigned long long>(runtime_instance_id)) + "_" +
                 std::to_string(nonce_counter) + ".prg");
        }

        bool prg_xasset_bootstrap_write_failure_requested(const std::filesystem::path &path)
        {
            const auto marker = copperfin::platform::read_environment_variable(
                "COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS");
            const auto stage = copperfin::platform::read_environment_variable(
                "COPPERFIN_TEST_FAIL_WRITE_STAGE");
            return marker.has_value() && stage.has_value() &&
                !marker->empty() && *stage == "prg-xasset-bootstrap" &&
                copperfin::platform::path_to_utf8_string(path).find(*marker) != std::string::npos;
        }
    } // namespace

    struct PrgRuntimeSession::Impl
    {
        explicit Impl(RuntimeSessionOptions session_options)
            : options(std::move(session_options)),
              localization_scope(options.localization_catalog.get())
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
            if (options.require_verified_file_byte_overrides)
            {
                // The parser consumes source text through this map. Mirror only admitted
                // PRG/header text so strict include resolution cannot fall back to disk.
                for (const auto &[candidate_name, bytes] : options.verified_file_byte_overrides)
                {
                    const std::string extension = lowercase_copy(
                        copperfin::platform::path_to_utf8_string(
                            copperfin::platform::path_from_utf8_string(candidate_name).extension()));
                    if ((extension == ".prg" || extension == ".h") && !bytes.empty())
                    {
                        options.source_text_overrides.try_emplace(normalize_path(candidate_name), bytes);
                    }
                }
            }
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

        void clear_caught_exception_identity(AErrorCompatibilitySnapshot &compatibility)
        {
            compatibility.thrown_user_value.reset();
            compatibility.explicit_error_code.reset();
            compatibility.active_exception_reference.reset();
            compatibility.preserve_fault_context = false;
        }

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

        struct NativeClassIdentity
        {
            std::string program_path;
            std::string class_name;
        };

        RuntimeSessionOptions options;
        RuntimeCatalogScope localization_scope;
        std::map<std::string, Program> programs;
        std::deque<Frame> stack;
        std::map<std::string, PrgValue> globals;
        std::optional<PrgValue> last_return_value;
        bool last_popped_frame_requested_nodefault = false;
        bool last_popped_frame_returned = false;
        std::map<std::string, RuntimeArray> arrays;
        std::map<int, std::map<std::string, RuntimeArray>> native_object_arrays;
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
        std::string udfparms_mode = "VALUE";
        std::size_t expression_evaluation_depth = 0U;
        bool resumable_expression_dispatch_active = false;
        std::size_t resumable_expression_depth = 0U;
        ExpressionContinuation *active_expression_continuation = nullptr;
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
        std::map<int, std::vector<NativeClassIdentity>> native_object_class_lineage_by_handle;
        std::map<int, std::map<std::string, std::string>> native_property_expression_text_by_handle;
        std::map<int, std::map<std::string, std::string>> native_default_property_expression_text_by_handle;
        std::map<std::string, std::string> native_method_source_text_by_key;
        std::optional<int> representative_active_form_handle;
        std::optional<int> representative_application_forms_collection_handle;
        std::optional<int> representative_application_surface_handle;
        std::string representative_application_caption = "Microsoft Visual FoxPro";
        int representative_application_window_state = 0;
        bool representative_application_right_to_left = true;
        bool representative_application_show_tips = false;
        std::vector<NativeEventBinding> native_event_bindings;
        std::set<std::string> active_native_event_keys;
        std::set<std::string> active_native_property_assignments;
        std::vector<CurrentNativeEventContext> active_native_event_contexts;
        std::vector<WindowMessageBinding> window_message_bindings;
        std::vector<CurrentWindowMessageContext> active_window_message_contexts;
        std::size_t next_native_event_binding_ordinal = 1U;
        std::set<std::string> loaded_libraries;
        std::vector<std::string> procedure_program_paths;
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
        std::size_t next_popup_action_id = 0;
        std::vector<std::filesystem::path> owned_xasset_bootstrap_paths;
        std::size_t scheduler_yield_statement_interval = 4096;
        std::size_t scheduler_yield_sleep_ms = 1;
        std::shared_ptr<std::atomic<bool>> task_cancel_requested;
        std::vector<std::string> critical_section_stack;
        std::map<std::string, std::size_t> critical_section_depth_by_name;
        std::map<std::string, std::shared_ptr<std::recursive_mutex>> critical_section_mutexes_by_name;
        std::size_t native_class_instantiation_depth = 0;

        // Index seek optimizer - pattern cache
        std::map<std::string, IndexExpressionPattern> index_pattern_cache;  // Cache analyzed patterns by expression text
        std::vector<std::pair<const CursorState *, const vfp::DbfRecord *>> record_evaluation_overrides;
        bool relation_synchronization_active = false;

#include "prg_engine_session.inl"
#include "prg_engine_verified_file_security.inl"
#include "prg_engine_xasset_security.inl"
#include "prg_engine_cursor.inl"
#include "prg_engine_records.inl"
#include "prg_engine_index_seek.inl"
#include "prg_engine_aggregate.inl"

#include "prg_engine_dll.inl"
#include "prg_engine_sql.inl"
        PrgValue evaluate_expression(const std::string &expression, const Frame &frame);
        PrgValue evaluate_expression(const std::string &expression, const Frame &frame, const CursorState *preferred_cursor);
        std::optional<std::string> materialize_xasset_bootstrap(const std::string &asset_path, bool include_read_events);
        std::optional<std::string> materialize_vcx_class_source(
            const Frame &frame,
            const std::string &class_name,
            const std::string &library_path,
            std::string &resolved_library_path);

#include "prg_engine_variables.inl"
#include "prg_engine_arrays.inl"
#define COPPERFIN_PRG_ENGINE_IMPL_CONTEXT
#include "prg_engine_flow.inl"
#undef COPPERFIN_PRG_ENGINE_IMPL_CONTEXT
        bool dispatch_event_handler(const std::string &routine_name);
        bool dispatch_popup_bar_selection(const std::string &popup_name, std::int64_t bar_number);
        void assign_native_window_metadata(RuntimeOleObjectState &runtime_object);
        [[nodiscard]] std::optional<std::intptr_t> hwnd_from_whandle(std::intptr_t whandle) const;
        [[nodiscard]] std::optional<std::intptr_t> whandle_from_hwnd(std::intptr_t hwnd) const;
        RuntimeOleObjectState *representative_active_form_object();
        const RuntimeOleObjectState *representative_active_form_object() const;
        void note_representative_active_form(const RuntimeOleObjectState &runtime_object);
        std::vector<int> representative_application_window_handles() const;
        std::size_t representative_application_form_count() const;
        RuntimeOleObjectState *representative_application_surface_object();
        RuntimeOleObjectState *ensure_representative_application_forms_collection_object();
        bool consume_last_popped_frame_requested_nodefault();
        bool consume_last_popped_frame_returned();
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
            const std::vector<std::string> &raw_arguments,
            const std::vector<std::optional<std::string>> &argument_references,
            std::size_t invocation_start,
            std::size_t invocation_end);
        std::optional<PrgValue> evaluate_resumable_expression(
            Frame &source_frame,
            const Statement &statement,
            const CursorState *preferred_cursor = nullptr);
        bool requery_native_list_control(
            RuntimeOleObjectState &runtime_object,
            const Frame &frame,
            bool require_query_resolution = false);
        bool materialize_select_query_rows(
            const std::string &query_text,
            const Frame &frame,
            std::vector<std::vector<PrgValue>> &rows);
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
        std::optional<std::string> read_native_property_expression_if_present(
            RuntimeOleObjectState &runtime_object,
            const std::string &property_name);
        std::optional<std::string> read_native_method_source_if_present(
            const RuntimeOleObjectState &runtime_object,
            const std::string &method_name);
        bool write_native_method_source_if_present(
            RuntimeOleObjectState &runtime_object,
            const std::string &method_name,
            const std::string &method_source_text,
            bool create_if_missing = false);
        std::optional<PrgValue> invoke_runtime_object_reference_member(
            const PrgValue &object_reference,
            const std::string &member_path,
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        PrgValue invoke_runtime_object_member(
            RuntimeOleObjectState &runtime_object,
            const std::string &effective_member_path,
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        bool set_native_focus(
            RuntimeOleObjectState &runtime_object,
            const std::string &effective_member_path,
            const Frame &source_frame);
        bool move_native_focus_to_next_tab_stop(
            RuntimeOleObjectState &runtime_object,
            const Frame &source_frame);
        bool move_native_optiongroup_selection(
            RuntimeOleObjectState &runtime_object,
            const Frame &source_frame,
            int direction);
        bool write_native_property_if_present(
            RuntimeOleObjectState &runtime_object,
            const std::string &property_name,
            const PrgValue &assigned_value,
            const Frame &source_frame,
            std::optional<std::string> assigned_expression_text = std::nullopt);
        void invoke_native_list_control_programmatic_change_if_needed(
            RuntimeOleObjectState &runtime_object,
            const Frame &source_frame,
            const std::optional<std::string> &before_signature);
        bool native_member_access_allowed(
            const RuntimeOleObjectState &runtime_object,
            NativeMemberVisibility visibility,
            const std::string &owner_class_name,
            const Frame &source_frame);
        [[noreturn]] void raise_native_member_access_denied(
            const RuntimeOleObjectState &runtime_object,
            const std::string &member_name,
            bool method_member);
        std::optional<PrgValue> invoke_native_object_method_body_if_present(
            RuntimeOleObjectState &runtime_object,
            const std::string &identifier,
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references,
            bool *requested_nodefault = nullptr);
        std::optional<PrgValue> invoke_native_object_method_if_present(
            RuntimeOleObjectState &runtime_object,
            const std::string &identifier,
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references,
            bool *requested_nodefault = nullptr,
            bool *returned_false = nullptr);
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
        void discard_native_object_tree_without_destroy(RuntimeOleObjectState &runtime_object);
        std::optional<PrgValue> invoke_expression_base_method(
            const Frame &source_frame,
            const std::vector<PrgValue> &arguments,
            const std::vector<std::optional<std::string>> &argument_references);
        bool handle_async_runtime_cancellation(
            SourceLocation location,
            std::string statement_text,
            std::string message,
            bool emit_task_cancelled_event = true);
        PrgValue run_expression_invoked_routine_until_return(std::size_t return_depth);
        RuntimeWatchResult evaluate_watch_expression(const std::string &expression);
        ExecutionOutcome execute_current_statement();
        RuntimePauseState run(DebugResumeAction action);

        ~Impl()
        {
            release_declared_dll_functions();
            for (const auto &path : owned_xasset_bootstrap_paths)
            {
                std::error_code ignored;
                std::filesystem::remove(path, ignored);
            }
        }
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
        struct ScopedExpressionDepth
        {
            explicit ScopedExpressionDepth(std::size_t &depth_value)
                : depth(depth_value)
            {
                ++depth;
            }
            ~ScopedExpressionDepth()
            {
                --depth;
            }
            std::size_t &depth;
        } scoped_expression_depth(expression_evaluation_depth);

        const std::string effective_expression = apply_with_context(expression, frame);
        const auto resolve_expression_cursor = [this, preferred_cursor](const std::string &designator)
        {
            if (trim_copy(designator).empty() && preferred_cursor != nullptr)
            {
                return preferred_cursor;
            }
            return static_cast<const CursorState *>(resolve_cursor_target(designator));
        };
        const auto resolve_mutable_expression_cursor = [this, preferred_cursor](const std::string &designator)
        {
            if (trim_copy(designator).empty() && preferred_cursor != nullptr)
            {
                return resolve_cursor_target(std::to_string(preferred_cursor->work_area));
            }
            return resolve_cursor_target(designator);
        };
        const auto expression_cursor_designator = [preferred_cursor](const std::string &designator)
        {
            return trim_copy(designator).empty() && preferred_cursor != nullptr
                       ? preferred_cursor->alias
                       : designator;
        };
        const bool handling_try_error = std::any_of(
            frame.tries.begin(),
            frame.tries.end(),
            [](const TryState &try_state)
            {
                return try_state.handling_error;
            });
        const std::string diagnostic_program_name =
            (handling_error || handling_try_error || !error_metadata_stack.empty())
                ? current_error_procedure()
                : frame.routine_name;
        ExpressionParser parser(
            effective_expression,
            frame,
            globals,
            current_default_directory(),
            current_error_message(),
            current_error_code(),
            current_error_procedure(),
            current_fault_location().line,
            diagnostic_program_name,
            stack.size(),
            [this](const long long level) -> std::optional<RuntimeProgramStackFrame>
            {
                if (level < 0 || stack.empty())
                {
                    return std::nullopt;
                }

                const std::size_t frame_index = level <= 1
                    ? 0U
                    : static_cast<std::size_t>(level - 1);
                if (frame_index >= stack.size())
                {
                    return std::nullopt;
                }

                const Frame &stack_frame = stack[frame_index];
                return RuntimeProgramStackFrame{
                    stack_frame.routine_name,
                    stack_frame.file_path,
                    stack_frame.procedure_context};
            },
            error_handler,
            shutdown_handler,
            is_set_enabled("exact"),
            preferred_cursor == nullptr ? current_selected_work_area() : preferred_cursor->work_area,
            [this]()
            {
                return next_available_work_area();
            },
            [resolve_expression_cursor, preferred_cursor](const std::string &designator)
            {
                if (trim_copy(designator).empty() && preferred_cursor != nullptr)
                {
                    return preferred_cursor->work_area;
                }
                const CursorState *cursor = resolve_expression_cursor(designator);
                return cursor == nullptr ? 0 : cursor->work_area;
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                const CursorState *cursor = resolve_expression_cursor(designator);
                return cursor == nullptr ? std::string{} : cursor->alias;
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                return resolve_expression_cursor(designator) != nullptr;
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                const CursorState *cursor = resolve_expression_cursor(designator);
                return cursor == nullptr ? std::string{} : cursor->dbf_identity;
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                const CursorState *cursor = resolve_expression_cursor(designator);
                return cursor == nullptr ? 0U : cursor->field_count;
            },
            [this, expression_cursor_designator](std::size_t index, const std::string &designator)
            {
                return cursor_field_name(expression_cursor_designator(designator), index);
            },
            [this, expression_cursor_designator](const std::string &field_name, std::size_t index, const std::string &designator)
            {
                return cursor_field_size(expression_cursor_designator(designator), field_name, index);
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                const CursorState *cursor = resolve_expression_cursor(designator);
                return cursor == nullptr ? 0U : cursor->record_count;
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                const CursorState *cursor = resolve_expression_cursor(designator);
                return cursor == nullptr ? 0U : cursor->record_length;
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                const CursorState *cursor = resolve_expression_cursor(designator);
                return cursor == nullptr ? 0U : cursor->recno;
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                const CursorState *cursor = resolve_expression_cursor(designator);
                return cursor == nullptr ? false : cursor->found;
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                const CursorState *cursor = resolve_expression_cursor(designator);
                return cursor == nullptr ? true : cursor->eof;
            },
            [resolve_expression_cursor](const std::string &designator)
            {
                const CursorState *cursor = resolve_expression_cursor(designator);
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
            [this, &frame](const std::string &name)
            {
                return has_array(name, frame);
            },
            [this, &frame](const std::string &name, int dimension)
            {
                return array_length(name, dimension, frame);
            },
            [this, &frame](const std::string &name, std::size_t row, std::size_t column)
            {
                return array_value(name, row, column, frame);
            },
            [this, &frame](const std::string &function, const std::vector<std::string> &raw_arguments, const std::vector<PrgValue> &arguments)
            {
                return mutate_array_function(function, raw_arguments, arguments, frame);
            },
            [this, &frame](const std::string &name)
            {
                return populate_error_array(canonical_array_name(name, frame));
            },
            [this, &frame](const std::string &function_name, const std::vector<std::string> &raw_arguments)
            {
                return aggregate_function_value(function_name, raw_arguments, frame);
            },
            [this, expression_cursor_designator](const std::string &designator, bool include_path)
            {
                return order_function_value(expression_cursor_designator(designator), include_path);
            },
            [this, expression_cursor_designator](const std::string &index_file_name, std::size_t tag_number, const std::string &designator)
            {
                return tag_function_value(index_file_name, tag_number, expression_cursor_designator(designator));
            },
            [this, &frame, resolve_mutable_expression_cursor](const std::string &search_key, bool move_pointer, const std::string &designator, const std::string &order_designator)
            {
                CursorState *cursor = resolve_mutable_expression_cursor(designator);
                if (cursor == nullptr)
                {
                    return false;
                }
                const SeekFunctionOrderDesignator parsed_order = parse_seek_function_order_designator(order_designator);
                return execute_seek(
                    *cursor,
                    search_key,
                    frame,
                    move_pointer,
                    false,
                    parsed_order.order_designator,
                    parsed_order.descending_override);
            },
            [this, &frame, resolve_mutable_expression_cursor](const std::string &search_key, bool move_pointer, const std::string &designator, const std::string &order_designator)
            {
                CursorState *cursor = resolve_mutable_expression_cursor(designator);
                if (cursor == nullptr)
                {
                    return false;
                }
                const SeekFunctionOrderDesignator parsed_order = parse_seek_function_order_designator(order_designator);
                return execute_seek(
                    *cursor,
                    search_key,
                    frame,
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
                std::string object_base_name = base_name;
                std::string object_member_path = member_path;
                if (normalized_base_name == "m")
                {
                    const std::size_t separator = member_path.find('.');
                    if (separator != std::string::npos)
                    {
                        object_base_name = "m." + member_path.substr(0U, separator);
                        object_member_path = member_path.substr(separator + 1U);
                    }
                }
                if (const auto direct_result = invoke_runtime_object_reference_member(
                        make_string_value(object_base_name),
                        object_member_path,
                        frame,
                        arguments,
                        argument_references);
                    direct_result.has_value())
                {
                    return *direct_result;
                }
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
                    resolved_path = resolve_runtime_object_member_path(
                        frame,
                        object_base_name,
                        object_member_path);
                }
                if (resolved_path.runtime_object == nullptr)
                {
                    return raise_ole_fault(base_name + "." + member_path + "()",
                                           base_name,
                                           runtime_text(
                                               "Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation",
                                               {{"targetIdentifier", base_name + "." + member_path}}));
                }

                const std::string effective_member_path =
                    resolved_path.remaining_member_path.empty()
                        ? member_path
                        : resolved_path.remaining_member_path;
                return invoke_runtime_object_member(
                    *resolved_path.runtime_object,
                    effective_member_path,
                    frame,
                    arguments,
                    argument_references);
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
                if (normalized_property_path == "_screen.righttoleft" ||
                    normalized_property_path == "_vfp.righttoleft")
                {
                    return make_boolean_value(representative_application_right_to_left);
                }
                if (normalized_property_path == "_screen.showtips" ||
                    normalized_property_path == "_vfp.showtips")
                {
                    return make_boolean_value(representative_application_show_tips);
                }
                if (normalized_property_path == "_screen.mousepointer" ||
                    normalized_property_path == "_vfp.mousepointer")
                {
                    if (representative_application_surface_handle.has_value())
                    {
                        const auto application_surface = ole_objects.find(
                            *representative_application_surface_handle);
                        if (application_surface != ole_objects.end())
                        {
                            const auto mouse_pointer =
                                application_surface->second.properties.find("mousepointer");
                            if (mouse_pointer != application_surface->second.properties.end())
                            {
                                return mouse_pointer->second;
                            }
                        }
                    }
                    return make_number_value(0.0);
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
                const std::string trimmed_option_name = trim_copy(option_name);
                constexpr std::string_view dbused_prefix = "__dbused__\x1f";
                if (trimmed_option_name.starts_with(dbused_prefix))
                {
                    const std::string designator = trimmed_option_name.substr(dbused_prefix.size());
                    return database_is_open(designator) ? std::string{"1"} : std::string{"0"};
                }
                if (trimmed_option_name == "__textmerge_delimiters__")
                {
                    const auto [left_delimiter, right_delimiter] = current_textmerge_delimiters();
                    return std::to_string(left_delimiter.size()) + ":" + left_delimiter + right_delimiter;
                }
                constexpr std::string_view relation_introspection_prefix = "__relation_introspection__\x1f";
                if (trimmed_option_name.starts_with(relation_introspection_prefix))
                {
                    const std::string payload = trimmed_option_name.substr(relation_introspection_prefix.size());
                    const std::size_t kind_end = payload.find('\x1f');
                    const std::size_t number_end = kind_end == std::string::npos
                        ? std::string::npos
                        : payload.find('\x1f', kind_end + 1U);
                    if (kind_end == std::string::npos || number_end == std::string::npos)
                    {
                        return std::string{};
                    }

                    long long relation_number = 0;
                    try
                    {
                        relation_number = std::stoll(payload.substr(kind_end + 1U, number_end - kind_end - 1U));
                    }
                    catch (...)
                    {
                        return std::string{};
                    }
                    if (relation_number < 1)
                    {
                        return std::string{};
                    }

                    const std::string kind = payload.substr(0U, kind_end);
                    const std::string designator = payload.substr(number_end + 1U);
                    CursorState *parent = designator.empty()
                        ? resolve_cursor_target(std::to_string(current_selected_work_area()))
                        : resolve_cursor_target(designator);
                    if (parent == nullptr)
                    {
                        return std::string{};
                    }

                    long long current_relation_number = 0;
                    for (const auto &relation : current_session_state().relations)
                    {
                        if (relation.parent_work_area != parent->work_area)
                        {
                            continue;
                        }
                        ++current_relation_number;
                        if (current_relation_number != relation_number)
                        {
                            continue;
                        }

                        CursorState *child = find_cursor_by_area(relation.child_work_area);
                        if (child == nullptr)
                        {
                            return std::string{};
                        }
                        return kind == "target" ? child->alias : relation.expression;
                    }
                    return std::string{};
                }

                std::string base_option_name = trimmed_option_name;
                std::string option_variant;
                const std::size_t comma_position = trimmed_option_name.find(',');
                if (comma_position != std::string::npos)
                {
                    base_option_name = trim_copy(trimmed_option_name.substr(0U, comma_position));
                    option_variant = trim_copy(trimmed_option_name.substr(comma_position + 1U));
                }

                std::string normalized_name = normalize_identifier(base_option_name);
                if (normalized_name == "udfp")
                {
                    normalized_name = "udfparms";
                }
                const std::string normalized_variant = normalize_identifier(option_variant);
                if (normalized_name == "textmerge" && normalized_variant == "1")
                {
                    const auto [left_delimiter, right_delimiter] = current_textmerge_delimiters();
                    return left_delimiter + "," + right_delimiter;
                }
                if (normalized_name == "textmerge" && normalized_variant == "3")
                {
                    const auto found_show_mode = current_set_state().find("textmerge_show");
                    return found_show_mode == current_set_state().end() || trim_copy(found_show_mode->second).empty()
                               ? std::string{"SHOW"}
                               : uppercase_copy(found_show_mode->second);
                }
                if (normalized_name == "default")
                {
                    return current_default_directory();
                }
                if (normalized_name == "database")
                {
                    return current_database_path();
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
                if (normalized_name == "datasession")
                {
                    return std::to_string(current_data_session);
                }
                if (normalized_name == "udfparms")
                {
                    return udfparms_mode;
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

                if (normalized_name == "codepage")
                {
                    return options.configured_code_page.has_value()
                               ? std::to_string(*options.configured_code_page)
                               : std::to_string(detail::default_host_code_page());
                }

                if (normalized_name == "relation" || normalized_name == "skip")
                {
                    CursorState *parent = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    if (parent == nullptr)
                    {
                        return std::string{};
                    }

                    std::string result;
                    for (const auto &relation : current_session_state().relations)
                    {
                        if (relation.parent_work_area != parent->work_area ||
                            (normalized_name == "skip" && !relation.skip_one_to_many))
                        {
                            continue;
                        }
                        CursorState *child = find_cursor_by_area(relation.child_work_area);
                        if (child == nullptr)
                        {
                            continue;
                        }
                        if (!result.empty())
                        {
                            result += normalized_name == "relation" ? ", " : ",";
                        }
                        result += normalized_name == "relation"
                            ? relation.expression + " INTO " + child->alias
                            : child->alias;
                    }
                    return result;
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
                    if (normalized_name == "epoch")
                    {
                        return std::string("1950");
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
                    std::filesystem::path snapshot_root;
                    std::string header_path = cursor->source_path;
                    if (options.require_verified_file_byte_overrides)
                    {
                        const auto verified_table_path = materialize_verified_file_snapshot(
                            copperfin::platform::path_from_utf8_string(cursor->source_path),
                            snapshot_root,
                            "Runtime.Prg.Database.Error.VerifiedBytesUnavailable",
                            false,
                            true);
                        if (!verified_table_path.has_value())
                        {
                            return std::nullopt;
                        }
                        header_path = copperfin::platform::path_to_utf8_string(*verified_table_path);
                    }

                    const auto header_result = vfp::parse_dbf_header_from_file(header_path);
                    if (header_result.ok)
                    {
                        snapshot.code_page = vfp::dbf_code_page_from_mark(header_result.header.code_page_mark);
                        snapshot.table_type = static_cast<int>(header_result.header.version);
                    }
                    if (!snapshot_root.empty())
                    {
                        std::error_code snapshot_error;
                        std::filesystem::remove_all(snapshot_root, snapshot_error);
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
                    parse_cursor_table(*cursor, std::max<std::size_t>(cursor->record_count, 1U));
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
                if (std::any_of(snapshot.rows.begin(), snapshot.rows.end(), [&](const RuntimeSurfaceCursorRow &row)
                    {
                        return row.values.size() != snapshot.fields.size();
                    }))
                {
                    return std::nullopt;
                }

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

                const auto create_result = vfp::create_dbf_table_file(
                    copperfin::platform::path_to_utf8_string(table_path), descriptors, rows);
                if (!create_result.ok)
                {
                    return std::nullopt;
                }

                if (options.require_verified_file_byte_overrides)
                {
                    std::ifstream input(table_path, std::ios::binary);
                    std::ostringstream bytes;
                    bytes << input.rdbuf();
                    if (!input.good() && !input.eof())
                    {
                        return std::nullopt;
                    }
                    options.verified_file_byte_overrides[
                        copperfin::platform::path_to_utf8_string(table_path)] = bytes.str();
                }

                if (!open_table_cursor(
                        copperfin::platform::path_to_utf8_string(table_path), alias, {}, true, false, 0, {}, 0U))
                {
                    return std::nullopt;
                }
                return snapshot.rows.size();
            },
            options.require_verified_file_byte_overrides,
            [this](const std::filesystem::path &path) -> std::optional<std::string>
            {
                const auto verified = find_verified_file_byte_override(path);
                if (verified == options.verified_file_byte_overrides.end() || verified->second.empty())
                {
                    return std::nullopt;
                }
                return verified->second;
            },
            [this](const std::string &function, const std::vector<PrgValue> &arguments)
            {
                return cursor_buffering_function(function, arguments);
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
                auto resolved_property = property;
                if (resolved_property == resolved_path.runtime_object->properties.end() &&
                    normalize_identifier(effective_member_path) == "header" &&
                    is_native_column_runtime_object(*resolved_path.runtime_object))
                {
                    (void)ensure_native_column_header_surface(*resolved_path.runtime_object);
                    resolved_property = resolved_path.runtime_object->properties.find("header");
                }
                if (resolved_property == resolved_path.runtime_object->properties.end())
                {
                    return nullptr;
                }

                auto object = resolve_ole_object(resolved_property->second);
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
                    const std::string normalized_segment = normalize_identifier(segments[index]);
                    auto property = target_object->properties.find(normalized_segment);
                    if (property == target_object->properties.end() &&
                        normalized_segment == "header" &&
                        is_native_column_runtime_object(*target_object))
                    {
                        (void)ensure_native_column_header_surface(*target_object);
                        property = target_object->properties.find(normalized_segment);
                    }
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
            [this, &frame](
                const PrgValue &value,
                const std::string &member_name,
                const std::vector<PrgValue> &arguments,
                const std::vector<std::optional<std::string>> &argument_references) -> std::optional<PrgValue>
            {
                return invoke_runtime_object_reference_member(
                    value,
                    member_name,
                    frame,
                    arguments,
                    argument_references);
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
            [this](const std::string &window_name) -> std::optional<bool>
            {
                // WVISIBLE("") denotes the modeled main VFP application window.
                if (trim_copy(window_name).empty())
                {
                    return true;
                }

                const std::string normalized_name = normalize_identifier(trim_copy(window_name));
                if (normalized_name.empty())
                {
                    return std::nullopt;
                }

                for (const auto &[handle, runtime_object] : ole_objects)
                {
                    (void)handle;
                    if (runtime_object.hidden_runtime_surface ||
                        !runtime_object.native_hwnd.has_value())
                    {
                        continue;
                    }

                    const auto name = runtime_object.properties.find("name");
                    const auto visible = runtime_object.properties.find("visible");
                    if (name != runtime_object.properties.end() &&
                        visible != runtime_object.properties.end() &&
                        normalize_identifier(trim_copy(value_as_string(name->second))) == normalized_name)
                    {
                        return value_as_bool(visible->second);
                    }
                }
                return std::nullopt;
            },
            [this](const std::string &name, std::vector<PrgValue> values)
            {
                assign_array(name, std::move(values));
            },
            [this](const std::vector<PrgValue> &arguments) -> std::optional<PrgValue>
            {
                if (arguments.size() < 2U)
                {
                    return std::nullopt;
                }

                const std::string popup_name = normalize_identifier(value_as_string(arguments[0]));
                const long long bar_number = static_cast<long long>(std::llround(value_as_number(arguments[1])));
                if (popup_name.empty() || bar_number < 1LL)
                {
                    return std::nullopt;
                }

                const auto popup = current_session_state().popup_bar_prompts.find(popup_name);
                if (popup == current_session_state().popup_bar_prompts.end())
                {
                    return std::nullopt;
                }
                const auto bar = popup->second.find(bar_number);
                if (bar == popup->second.end())
                {
                    return std::nullopt;
                }

                std::string prompt = bar->second;
                if (prompt.rfind("\\-", 0U) == 0U)
                {
                    prompt.clear();
                }
                else if (prompt.rfind("\\<", 0U) == 0U)
                {
                    prompt.erase(0U, 2U);
                }
                else if (prompt.rfind("\\", 0U) == 0U)
                {
                    prompt.erase(0U, 1U);
                }
                return make_string_value(prompt);
            },
            [this](const std::vector<PrgValue> &arguments) -> std::optional<PrgValue>
            {
                if (arguments.empty())
                {
                    return std::nullopt;
                }

                const std::string popup_name = normalize_identifier(value_as_string(arguments[0]));
                if (popup_name.empty())
                {
                    return std::nullopt;
                }

                const auto popup = current_session_state().popup_bar_prompts.find(popup_name);
                if (popup == current_session_state().popup_bar_prompts.end())
                {
                    return std::nullopt;
                }
                return make_number_value(static_cast<double>(popup->second.size()));
            },
            [this](const std::vector<PrgValue> &arguments) -> std::optional<PrgValue>
            {
                if (arguments.size() < 2U)
                {
                    return std::nullopt;
                }

                const std::string popup_name = normalize_identifier(value_as_string(arguments[0]));
                const long long position = static_cast<long long>(std::llround(value_as_number(arguments[1])));
                if (popup_name.empty() || position < 1LL)
                {
                    return std::nullopt;
                }

                const auto popup = current_session_state().popup_bar_prompts.find(popup_name);
                if (popup == current_session_state().popup_bar_prompts.end() ||
                    position > static_cast<long long>(popup->second.size()))
                {
                    return std::nullopt;
                }

                auto bar = popup->second.begin();
                std::advance(bar, position - 1LL);
                return make_number_value(static_cast<double>(bar->first));
            },
            [this](const std::vector<PrgValue> &arguments) -> std::optional<PrgValue>
            {
                if (arguments.size() < 2U)
                {
                    return std::nullopt;
                }

                const std::string popup_name = normalize_identifier(value_as_string(arguments[0]));
                const long long bar_number = static_cast<long long>(std::llround(value_as_number(arguments[1])));
                if (popup_name.empty() || bar_number < 1LL)
                {
                    return std::nullopt;
                }

                const auto popup = current_session_state().popup_bar_skip_states.find(popup_name);
                if (popup == current_session_state().popup_bar_skip_states.end())
                {
                    return make_boolean_value(false);
                }
                const auto bar = popup->second.find(bar_number);
                return make_boolean_value(bar != popup->second.end() && bar->second);
            },
            [this](const std::vector<PrgValue> &arguments) -> std::optional<PrgValue>
            {
                if (arguments.size() < 2U)
                {
                    return std::nullopt;
                }

                const std::string popup_name = normalize_identifier(value_as_string(arguments[0]));
                const long long bar_number = static_cast<long long>(std::llround(value_as_number(arguments[1])));
                if (popup_name.empty() || bar_number < 1LL)
                {
                    return std::nullopt;
                }

                const auto popup = current_session_state().popup_bar_mark_states.find(popup_name);
                if (popup == current_session_state().popup_bar_mark_states.end())
                {
                    return make_boolean_value(false);
                }
                const auto bar = popup->second.find(bar_number);
                return make_boolean_value(bar != popup->second.end() && bar->second);
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
                const std::vector<std::string> &raw_arguments,
                const std::vector<std::optional<std::string>> &argument_references,
                std::size_t invocation_start,
                std::size_t invocation_end) -> std::optional<PrgValue>
            {
                return invoke_expression_user_routine(
                    frame,
                    identifier,
                    arguments,
                    raw_arguments,
                    argument_references,
                    invocation_start,
                    invocation_end);
            },
            [this](
                const std::string &fn_key,
                const std::vector<PrgValue> &fn_args,
                const std::vector<std::optional<std::string>> &fn_argument_references)
                -> std::optional<PrgValue>
            {
                return invoke_declared_dll_function(fn_key, fn_args, fn_argument_references);
            },
            resumable_expression_dispatch_active &&
                    expression_evaluation_depth == resumable_expression_depth
                ? active_expression_continuation
                : nullptr);
        return parser.parse();
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_runtime_object_reference_member(
        const PrgValue &object_reference,
        const std::string &member_path,
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        auto object = resolve_ole_object(object_reference);
        if (!object.has_value())
        {
            return std::nullopt;
        }

        const auto resolved_path = resolve_runtime_object_member_path(*object, member_path);
        if (resolved_path.runtime_object == nullptr)
        {
            return std::nullopt;
        }

        return invoke_runtime_object_member(
            *resolved_path.runtime_object,
            resolved_path.remaining_member_path.empty()
                ? member_path
                : resolved_path.remaining_member_path,
            source_frame,
            arguments,
            argument_references);
    }

    bool PrgRuntimeSession::Impl::set_native_focus(
        RuntimeOleObjectState &runtime_object,
        const std::string &effective_member_path,
        const Frame &frame)
    {
        if (!is_native_focusable_runtime_object(runtime_object))
        {
            return false;
        }

        const PrgValue runtime_object_reference =
            make_string_value("object:" + runtime_object.prog_id + "#" + std::to_string(runtime_object.handle));
        std::optional<PrgValue> previous_active_control;
        bool focus_changed = true;
        bool suppress_focus_transition = false;
        if (const auto owner_form_reference = native_object_owner_form_reference(runtime_object);
            owner_form_reference.has_value())
        {
            if (auto owner_form = resolve_ole_object(*owner_form_reference);
                owner_form.has_value())
            {
                if (const auto current_active_control =
                        read_native_property_if_present(**owner_form, "activecontrol", frame);
                    current_active_control.has_value())
                {
                    previous_active_control = *current_active_control;
                }
                if (previous_active_control.has_value())
                {
                    if (auto previous_control = resolve_ole_object(*previous_active_control);
                        previous_control.has_value())
                    {
                        focus_changed = (*previous_control)->handle != runtime_object.handle;
                        if (focus_changed)
                        {
                            last_popped_frame_requested_nodefault = false;
                            bool valid_requested_nodefault = false;
                            const auto valid_result =
                                invoke_native_object_method_if_present(
                                    **previous_control,
                                    "valid",
                                    frame,
                                    {},
                                    {},
                                    &valid_requested_nodefault);
                            (void)consume_last_popped_frame_requested_nodefault();
                            const bool validation_rejected =
                                valid_result.has_value() &&
                                valid_result->kind != PrgValueKind::empty &&
                                !value_as_bool(*valid_result);
                            if (validation_rejected || valid_requested_nodefault)
                            {
                                suppress_focus_transition = true;
                            }
                        }
                        if (focus_changed && !suppress_focus_transition)
                        {
                            last_popped_frame_requested_nodefault = false;
                            bool lost_focus_requested_nodefault = false;
                            (void)invoke_native_object_method_if_present(
                                **previous_control,
                                "lostfocus",
                                frame,
                                {},
                                {},
                                &lost_focus_requested_nodefault);
                            (void)consume_last_popped_frame_requested_nodefault();
                            suppress_focus_transition = lost_focus_requested_nodefault;
                        }
                    }
                }
                if (!suppress_focus_transition)
                {
                    (void)write_native_property_if_present(
                        **owner_form,
                        "activecontrol",
                        runtime_object_reference,
                        frame);
                }
            }
        }
        else if (!suppress_focus_transition &&
                 normalize_identifier(trim_copy(runtime_object.base_class_name)) == "form")
        {
            (void)write_native_property_if_present(
                runtime_object,
                "activecontrol",
                runtime_object_reference,
                frame);
        }
        if (!suppress_focus_transition)
        {
            note_representative_active_form(runtime_object);
            if (focus_changed)
            {
                last_popped_frame_requested_nodefault = false;
                bool got_focus_requested_nodefault = false;
                (void)invoke_native_object_method_if_present(
                    runtime_object,
                    "gotfocus",
                    frame,
                    {},
                    {},
                    &got_focus_requested_nodefault);
                (void)consume_last_popped_frame_requested_nodefault();
            }
        }
        runtime_object.last_action = effective_member_path + "()";
        ++runtime_object.action_count;
        events.push_back({.category = "prg.object.setfocus",
                          .detail = runtime_object.prog_id + "." + effective_member_path,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        return true;
    }

    bool PrgRuntimeSession::Impl::move_native_focus_to_next_tab_stop(
        RuntimeOleObjectState &runtime_object,
        const Frame &frame)
    {
        RuntimeOleObjectState *owner_form = nullptr;
        if (const auto owner_form_reference = native_object_owner_form_reference(runtime_object);
            owner_form_reference.has_value())
        {
            if (auto resolved_form = resolve_ole_object(*owner_form_reference);
                resolved_form.has_value())
            {
                owner_form = *resolved_form;
            }
        }
        else if (normalize_identifier(trim_copy(runtime_object.base_class_name)) == "form")
        {
            owner_form = &runtime_object;
        }

        if (owner_form == nullptr)
        {
            return false;
        }

        struct TabStopCandidate
        {
            int handle = 0;
            std::vector<long long> tab_order;
        };
        struct PendingTabObject
        {
            int handle = 0;
            bool ancestor_visible = true;
            bool ancestor_enabled = true;
            std::vector<long long> tab_prefix;
        };
        const auto read_tab_index = [](const RuntimeOleObjectState &object)
        {
            if (const auto property = object.properties.find("tabindex");
                property != object.properties.end())
            {
                try
                {
                    return static_cast<long long>(std::llround(value_as_number(property->second)));
                }
                catch (...)
                {
                    return 0LL;
                }
            }
            return 0LL;
        };
        std::vector<TabStopCandidate> candidates;
        std::vector<PendingTabObject> pending_objects;
        std::set<int> seen_handles;
        for (const int child_handle : collect_native_owned_child_handles(*owner_form))
        {
            pending_objects.push_back({child_handle, true, true, {}});
        }

        while (!pending_objects.empty())
        {
            const PendingTabObject pending = pending_objects.back();
            pending_objects.pop_back();
            if (!seen_handles.insert(pending.handle).second)
            {
                continue;
            }

            const auto child_found = ole_objects.find(pending.handle);
            if (child_found == ole_objects.end())
            {
                continue;
            }

            const auto property_is_true = [&](const std::string &property_name)
            {
                const auto property = child_found->second.properties.find(property_name);
                return property == child_found->second.properties.end() ||
                    value_as_bool(property->second);
            };
            const bool visible = pending.ancestor_visible && property_is_true("visible");
            const bool enabled = pending.ancestor_enabled && property_is_true("enabled");
            const std::string normalized_base_class =
                normalize_identifier(trim_copy(child_found->second.base_class_name));
            if (is_native_focusable_runtime_object(child_found->second) &&
                normalized_base_class != "page" &&
                property_is_true("tabstop") && visible && enabled)
            {
                std::vector<long long> tab_order = pending.tab_prefix;
                tab_order.push_back(read_tab_index(child_found->second));
                candidates.push_back({pending.handle, std::move(tab_order)});
            }

            if (!visible || !enabled)
            {
                continue;
            }

            if (normalized_base_class == "container" || normalized_base_class == "page")
            {
                std::vector<long long> child_tab_prefix = pending.tab_prefix;
                child_tab_prefix.push_back(read_tab_index(child_found->second));
                for (const int nested_handle : collect_native_owned_child_handles(child_found->second))
                {
                    pending_objects.push_back({nested_handle, visible, enabled, child_tab_prefix});
                }
            }
            else if (normalized_base_class == "pageframe")
            {
                long long active_page = 0LL;
                if (const auto property = child_found->second.properties.find("activepage");
                    property != child_found->second.properties.end())
                {
                    try
                    {
                        active_page = std::llround(value_as_number(property->second));
                    }
                    catch (...)
                    {
                        active_page = 0LL;
                    }
                }

                if (active_page <= 0LL)
                {
                    continue;
                }

                const auto page_members = collect_native_pageframe_page_members(child_found->second);
                const auto page_index = static_cast<std::size_t>(active_page - 1LL);
                if (page_index >= page_members.size() || page_members[page_index].child_object == nullptr)
                {
                    continue;
                }

                std::vector<long long> page_tab_prefix = pending.tab_prefix;
                page_tab_prefix.push_back(read_tab_index(child_found->second));
                pending_objects.push_back({
                    page_members[page_index].child_object->handle,
                    visible,
                    enabled,
                    page_tab_prefix});
            }
            else if (normalized_base_class == "commandgroup" && property_is_true("tabstop"))
            {
                const std::vector<long long> group_tab_prefix{
                    pending.tab_prefix.begin(),
                    pending.tab_prefix.end()};
                const long long group_tab_index = read_tab_index(child_found->second);
                std::vector<long long> child_tab_prefix = group_tab_prefix;
                child_tab_prefix.push_back(group_tab_index);
                for (const int nested_handle : collect_native_owned_child_handles(child_found->second))
                {
                    pending_objects.push_back({nested_handle, visible, enabled, child_tab_prefix});
                }
            }
        }

        if (candidates.empty())
        {
            return false;
        }

        std::sort(
            candidates.begin(),
            candidates.end(),
            [](const TabStopCandidate &left, const TabStopCandidate &right)
            {
                if (left.tab_order != right.tab_order)
                {
                    return left.tab_order < right.tab_order;
                }
                return left.handle < right.handle;
            });

        std::size_t next_index = 0U;
        const auto current = std::find_if(
            candidates.begin(),
            candidates.end(),
            [&](const TabStopCandidate &candidate)
            {
                return candidate.handle == runtime_object.handle;
            });
        if (current != candidates.end())
        {
            next_index = static_cast<std::size_t>(std::distance(candidates.begin(), current) + 1U) %
                candidates.size();
        }

        auto next_object = ole_objects.find(candidates[next_index].handle);
        if (next_object == ole_objects.end())
        {
            return false;
        }

        bool requested_nodefault = false;
        if (const auto native_result = invoke_native_object_method_if_present(
                next_object->second,
                "setfocus",
                frame,
                {},
                {},
                &requested_nodefault);
            native_result.has_value())
        {
            (void)consume_last_popped_frame_requested_nodefault();
            return true;
        }

        return set_native_focus(next_object->second, "SetFocus", frame);
    }

    bool PrgRuntimeSession::Impl::move_native_optiongroup_selection(
        RuntimeOleObjectState &runtime_object,
        const Frame &frame,
        const int direction)
    {
        if (normalize_identifier(trim_copy(runtime_object.base_class_name)) != "optiongroup" ||
            options.keyboard_compatibility != RuntimeKeyboardCompatibility::windows ||
            (direction != -1 && direction != 1))
        {
            return false;
        }

        struct OptionCandidate
        {
            int handle = 0;
            long long option_number = 0;
            long long tab_index = 0;
            bool eligible = false;
        };
        std::vector<OptionCandidate> candidates;
        for (const int child_handle : collect_native_owned_child_handles(runtime_object))
        {
            const auto child_found = ole_objects.find(child_handle);
            if (child_found == ole_objects.end() ||
                normalize_identifier(trim_copy(child_found->second.base_class_name)) != "optionbutton")
            {
                continue;
            }

            const auto visible = read_native_property_if_present(child_found->second, "visible", frame);
            const auto enabled = read_native_property_if_present(child_found->second, "enabled", frame);
            const bool eligible = (!visible.has_value() || value_as_bool(*visible)) &&
                (!enabled.has_value() || value_as_bool(*enabled));

            long long tab_index = child_handle;
            if (const auto tab_index_value =
                    read_native_property_if_present(child_found->second, "tabindex", frame);
                tab_index_value.has_value())
            {
                try
                {
                    tab_index = std::llround(value_as_number(*tab_index_value));
                }
                catch (...)
                {
                    tab_index = child_handle;
                }
            }
            candidates.push_back({child_handle, 0, tab_index, eligible});
        }

        if (candidates.empty())
        {
            return false;
        }

        std::sort(
            candidates.begin(),
            candidates.end(),
            [](const OptionCandidate &left, const OptionCandidate &right)
            {
                return left.handle < right.handle;
            });
        for (std::size_t index = 0U; index < candidates.size(); ++index)
        {
            candidates[index].option_number = static_cast<long long>(index + 1U);
        }
        candidates.erase(
            std::remove_if(
                candidates.begin(),
                candidates.end(),
                [](const OptionCandidate &candidate)
                {
                    return !candidate.eligible;
                }),
            candidates.end());
        if (candidates.empty())
        {
            return false;
        }

        std::sort(
            candidates.begin(),
            candidates.end(),
            [](const OptionCandidate &left, const OptionCandidate &right)
            {
                if (left.tab_index != right.tab_index)
                {
                    return left.tab_index < right.tab_index;
                }
                return left.handle < right.handle;
            });

        std::optional<std::size_t> current_index;
        if (const auto group_value = read_native_property_if_present(runtime_object, "value", frame);
            group_value.has_value())
        {
            const long long selected_option = std::llround(value_as_number(*group_value));
            for (std::size_t index = 0U; index < candidates.size(); ++index)
            {
                if (candidates[index].option_number == selected_option)
                {
                    current_index = index;
                    break;
                }
            }
        }
        if (!current_index.has_value())
        {
            for (std::size_t index = 0U; index < candidates.size(); ++index)
            {
                const auto child_found = ole_objects.find(candidates[index].handle);
                if (child_found == ole_objects.end())
                {
                    continue;
                }
                const auto selected = read_native_property_if_present(child_found->second, "value", frame);
                if (selected.has_value() && value_as_bool(*selected))
                {
                    current_index = index;
                    break;
                }
            }
        }

        const std::size_t start_index = current_index.has_value()
            ? *current_index
            : (direction > 0 ? candidates.size() - 1U : 0U);
        const std::size_t next_index = direction > 0
            ? (start_index + 1U) % candidates.size()
            : (start_index + candidates.size() - 1U) % candidates.size();
        const long long selected_option = candidates[next_index].option_number;

        for (const OptionCandidate &candidate : candidates)
        {
            const auto child_found = ole_objects.find(candidate.handle);
            if (child_found == ole_objects.end())
            {
                continue;
            }
            (void)write_native_property_if_present(
                child_found->second,
                "value",
                make_boolean_value(candidate.handle == candidates[next_index].handle),
                frame);
        }
        if (!write_native_property_if_present(
                runtime_object,
                "value",
                make_number_value(static_cast<double>(selected_option)),
                frame))
        {
            return false;
        }

        bool ignored_nodefault = false;
        if (invoke_native_object_method_if_present(
                runtime_object,
                "interactivechange",
                frame,
                {},
                {},
                &ignored_nodefault,
                nullptr)
                .has_value())
        {
            events.push_back({.category = "prg.event.interactivechange",
                              .detail = runtime_object.prog_id,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        }
        runtime_object.last_action = "OptionGroup.Value";
        ++runtime_object.action_count;
        return true;
    }

    PrgValue PrgRuntimeSession::Impl::invoke_runtime_object_member(
        RuntimeOleObjectState &runtime_object,
        const std::string &effective_member_path,
        const Frame &frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        const auto make_runtime_object_reference = [](const RuntimeOleObjectState &object_state) -> PrgValue
        {
            return make_string_value("object:" + object_state.prog_id + "#" + std::to_string(object_state.handle));
        };
        RuntimeOleObjectState *target_object = &runtime_object;
        const std::string leaf = normalize_identifier(
            effective_member_path.substr(
                effective_member_path.rfind('.') == std::string::npos
                    ? 0U
                    : effective_member_path.rfind('.') + 1U));

        if (leaf == "addobject" && !target_object->source.empty() && arguments.size() >= 2U)
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
                lowercase_copy(copperfin::platform::path_to_utf8_string(
                    copperfin::platform::path_from_utf8_string(child_library).extension())) == ".prg";
            const std::string implicit_child_program_path =
                frame.native_method_class_name.empty()
                    ? target_object->source
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
                make_runtime_object_reference(*target_object));
            if (child_object == nullptr && !explicit_native_prg_library)
            {
                const std::string owner_program_path = normalize_path(target_object->source);
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
                        make_runtime_object_reference(*target_object));
                }
            }
            if (child_object == nullptr)
            {
                return make_boolean_value(false);
            }

            assign_native_runtime_object_name(*child_object, child_name_text);
            target_object->properties[child_name] = make_runtime_object_reference(*child_object);
            if (child_object->properties.contains("columnorder"))
            {
                (void)write_native_columnorder_property(
                    *child_object,
                    child_object->properties["columnorder"]);
            }
            if (is_native_column_runtime_object(*target_object) &&
                native_column_bound_value(*target_object))
            {
                sync_native_column_child_controlsources(*target_object);
            }
            (void)sync_native_owned_children_collection(*target_object);
            target_object->last_action = effective_member_path + "(" + child_name + "," + child_class + ")";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.addobject",
                              .detail = target_object->prog_id + "." + child_name + ":" + child_class,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(true);
        }
        if (leaf == "removeobject" && !target_object->source.empty() && !arguments.empty())
        {
            const std::string child_name = normalize_identifier(trim_copy(value_as_string(arguments[0])));
            if (child_name.empty())
            {
                return make_boolean_value(false);
            }

            const auto child_property = target_object->properties.find(child_name);
            if (child_property == target_object->properties.end())
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
                parent_handle != target_object->handle)
            {
                return make_boolean_value(false);
            }

            (*child_object)->properties.erase("parent");
            target_object->properties.erase(child_name);
            (void)sync_native_owned_children_collection(*target_object);
            target_object->last_action = effective_member_path + "(" + child_name + ")";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.removeobject",
                              .detail = target_object->prog_id + "." + child_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(true);
        }
        if (leaf == "setall" && !target_object->source.empty())
        {
            return apply_native_setall(
                *target_object,
                frame,
                effective_member_path,
                arguments);
        }
        if (leaf == "release" && !target_object->source.empty())
        {
            last_popped_frame_requested_nodefault = false;
            if (auto native_result = invoke_native_object_method_if_present(
                    *target_object,
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
            return release_native_object(*target_object, effective_member_path);
        }
        if (auto native_result = invoke_native_object_method_if_present(
                *target_object,
                leaf,
                frame,
                arguments,
                argument_references);
            native_result.has_value())
        {
            return *native_result;
        }
        const bool is_report_listener_object = std::any_of(
            target_object->class_hierarchy.begin(),
            target_object->class_hierarchy.end(),
            [](const std::string &class_name)
            {
                return normalize_identifier(trim_copy(class_name)) == "reportlistener";
            }) ||
            normalize_identifier(trim_copy(target_object->base_class_name)) == "reportlistener";
        if (is_report_listener_object && leaf == "createconfigtable")
        {
            std::string result_code = "invalid-path";
            bool created = false;
            if (!arguments.empty() && !trim_copy(value_as_string(arguments.front())).empty())
            {
                std::filesystem::path requested_path = copperfin::platform::path_from_utf8_string(
                    value_as_string(arguments.front()));
                if (requested_path.extension().empty())
                {
                    requested_path.replace_extension(".dbf");
                }
                if (requested_path.is_relative())
                {
                    requested_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                        requested_path;
                }

                const bool overwrite = arguments.size() >= 2U && value_as_bool(arguments[1]);
                const auto existing_path = copperfin::vfp::resolve_unique_casefold_path(requested_path);
                std::filesystem::path memo_path = requested_path;
                memo_path.replace_extension(".fpt");
                std::filesystem::path index_path = requested_path;
                index_path.replace_extension(".cdx");
                const auto existing_memo_path = copperfin::vfp::resolve_unique_casefold_path(memo_path);
                const auto existing_index_path = copperfin::vfp::resolve_unique_casefold_path(index_path);
                if (existing_path.ambiguous || existing_memo_path.ambiguous || existing_index_path.ambiguous)
                {
                    result_code = "ambiguous-path";
                }
                else if (options.require_verified_file_byte_overrides)
                {
                    result_code = "verified-write-rejected";
                }
                else if ((existing_path.path.has_value() || existing_memo_path.path.has_value() ||
                          existing_index_path.path.has_value()) && !overwrite)
                {
                    result_code = "exists";
                }
                else
                {
                    const std::filesystem::path target_path = existing_path.path.value_or(requested_path);
                    const auto create_result = copperfin::vfp::create_dbf_table_file(
                        copperfin::platform::path_to_utf8_string(target_path),
                        {
                            {.name = "OBJTYPE", .type = 'I', .length = 4U},
                            {.name = "OBJCODE", .type = 'I', .length = 4U},
                            {.name = "OBJNAME", .type = 'V', .length = 60U},
                            {.name = "OBJVALUE", .type = 'V', .length = 60U},
                            {.name = "OBJINFO", .type = 'M', .length = 4U}
                        },
                        {});
                    created = create_result.ok;
                    result_code = created ? "created" : "write-failed";
                    if (created)
                    {
                        target_object->properties["configurationtable"] = make_string_value(
                            copperfin::platform::path_to_utf8_string(target_path.lexically_normal()));
                    }
                }
            }
            target_object->properties["haderror"] = make_boolean_value(!created);
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.reportlistener.createconfigtable",
                              .detail = target_object->prog_id + ":" + result_code,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(created);
        }
        if (is_report_listener_object && leaf == "getconfigtable")
        {
            const std::filesystem::path working_directory =
                copperfin::platform::path_from_utf8_string(current_default_directory());
            const auto resolve_configuration_path = [&](const std::filesystem::path &candidate)
                -> std::optional<std::filesystem::path>
            {
                if (options.require_verified_file_byte_overrides)
                {
                    bool ambiguous = false;
                    return resolve_verified_file_byte_override_path(candidate, ambiguous, true);
                }
                const auto resolution = copperfin::vfp::resolve_unique_casefold_path(candidate);
                if (resolution.ambiguous || !resolution.path.has_value())
                {
                    return std::nullopt;
                }
                return resolution.path;
            };
            std::filesystem::path requested_path;
            if (const auto configured = target_object->properties.find("configurationtable");
                configured != target_object->properties.end() &&
                !trim_copy(value_as_string(configured->second)).empty())
            {
                requested_path = copperfin::platform::path_from_utf8_string(
                    value_as_string(configured->second));
                if (requested_path.is_relative())
                {
                    requested_path = working_directory / requested_path;
                }
            }
            else
            {
                const std::array<std::filesystem::path, 2U> candidates = {
                    working_directory / "OutputConfig.dbf",
                    working_directory / "_ReportOutputConfig.dbf"};
                for (const auto &candidate : candidates)
                {
                    if (const auto resolved = resolve_configuration_path(candidate);
                        resolved.has_value())
                    {
                        requested_path = *resolved;
                        break;
                    }
                }
            }

            std::optional<std::filesystem::path> resolved_path;
            if (!requested_path.empty())
            {
                resolved_path = resolve_configuration_path(requested_path);
            }
            const std::string config_path = resolved_path.has_value()
                ? copperfin::platform::path_to_utf8_string(resolved_path->lexically_normal())
                : std::string{};
            target_object->properties["configurationtable"] = make_string_value(config_path);
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.reportlistener.getconfigtable",
                              .detail = target_object->prog_id + ":" + config_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_string_value(config_path);
        }
        if (is_report_listener_object && leaf == "verifyconfigtable")
        {
            const auto configured = target_object->properties.find("configurationtable");
            const std::string configured_path = configured == target_object->properties.end()
                ? std::string{}
                : trim_copy(value_as_string(configured->second));
            std::string result_code = "missing";
            bool valid = false;
            if (!configured_path.empty())
            {
                const auto requested_path = copperfin::platform::path_from_utf8_string(configured_path);
                std::optional<std::filesystem::path> resolved_path;
                if (options.require_verified_file_byte_overrides)
                {
                    bool ambiguous = false;
                    resolved_path = resolve_verified_file_byte_override_path(requested_path, ambiguous, true);
                    if (ambiguous)
                    {
                        resolved_path.reset();
                    }
                }
                else
                {
                    const auto resolution = copperfin::vfp::resolve_unique_casefold_path(requested_path);
                    if (!resolution.ambiguous && resolution.path.has_value())
                    {
                        resolved_path = resolution.path;
                    }
                }
                if (resolved_path.has_value())
                {
                    std::filesystem::path snapshot_root;
                    const auto table_path = options.require_verified_file_byte_overrides
                        ? materialize_verified_table_snapshot(*resolved_path, snapshot_root)
                        : resolved_path;
                    const auto table = table_path.has_value()
                        ? copperfin::vfp::parse_dbf_table_from_file(
                              copperfin::platform::path_to_utf8_string(*table_path),
                              std::numeric_limits<std::size_t>::max())
                        : copperfin::vfp::DbfTableParseResult{};
                    if (table.ok)
                    {
                        const auto has_exactly_one_field = [&](const std::string &name) {
                            const std::string normalized_name = normalize_identifier(name);
                            return std::count_if(
                                       table.table.fields.begin(),
                                       table.table.fields.end(),
                                       [&](const copperfin::vfp::DbfFieldDescriptor &field)
                                       {
                                           return normalize_identifier(field.name) == normalized_name;
                                       }) == 1;
                        };
                        const auto field_matches = [&](const std::string &name, const std::string &types) {
                            const std::string normalized_name = normalize_identifier(name);
                            const auto field = std::find_if(
                                table.table.fields.begin(),
                                table.table.fields.end(),
                                [&](const copperfin::vfp::DbfFieldDescriptor &candidate)
                                {
                                    return normalize_identifier(candidate.name) == normalized_name;
                                });
                            return field != table.table.fields.end() &&
                                types.find(field->type) != std::string::npos;
                        };
                        valid = has_exactly_one_field("OBJTYPE") &&
                            has_exactly_one_field("OBJCODE") &&
                            has_exactly_one_field("OBJNAME") &&
                            has_exactly_one_field("OBJVALUE") &&
                            has_exactly_one_field("OBJINFO") &&
                            field_matches("OBJTYPE", "IN") &&
                            field_matches("OBJCODE", "IN") &&
                            field_matches("OBJNAME", "CV") &&
                            field_matches("OBJVALUE", "CV") &&
                            field_matches("OBJINFO", "MCV");
                        result_code = valid ? "valid" : "unsupported";
                    }
                    else
                    {
                        result_code = "unsupported";
                    }
                    if (!snapshot_root.empty())
                    {
                        std::error_code snapshot_error;
                        std::filesystem::remove_all(snapshot_root, snapshot_error);
                    }
                }
            }
            target_object->properties["haderror"] = make_boolean_value(!valid);
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.reportlistener.verifyconfigtable",
                              .detail = target_object->prog_id + ":" + result_code,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(valid);
        }
        std::function<bool(const std::vector<PrgValue>&)> before_list_control_move;
        if (leaf == "moveitem")
        {
            before_list_control_move = [&](const std::vector<PrgValue>& event_arguments)
            {
                bool requested_nodefault = false;
                bool returned_false = false;
                (void)invoke_native_object_method_if_present(
                    *target_object,
                    "onmoveitem",
                    frame,
                    event_arguments,
                    std::vector<std::optional<std::string>>(event_arguments.size()),
                    &requested_nodefault,
                    &returned_false);
                (void)consume_last_popped_frame_requested_nodefault();
                return !returned_false;
            };
        }
        const auto before_list_control_signature =
            native_list_control_selection_signature(*target_object);
        if (auto list_control_result = invoke_native_list_control_method(
                *target_object,
                leaf,
                arguments,
                before_list_control_move);
            list_control_result.has_value())
        {
            if (leaf == "additem" || leaf == "addlistitem" || leaf == "clear" ||
                leaf == "removeitem" || leaf == "removelistitem")
            {
                write_native_list_control_controlsource_target(*target_object, frame);
            }
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            if (leaf == "additem" || leaf == "addlistitem" || leaf == "clear" || leaf == "removeitem" ||
                leaf == "removelistitem")
            {
                events.push_back({.category = "prg.object." + leaf,
                                  .detail = target_object->prog_id + "." + effective_member_path,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            }
            invoke_native_list_control_programmatic_change_if_needed(
                *target_object,
                frame,
                before_list_control_signature);
            return *list_control_result;
        }
        if (leaf == "readexpression" &&
            (!target_object->class_hierarchy.empty() || !target_object->source.empty()))
        {
            if (arguments.empty())
            {
                return make_string_value("");
            }

            const auto expression_text =
                read_native_property_expression_if_present(*target_object, value_as_string(arguments.front()));
            target_object->last_action = effective_member_path + "(" + value_as_string(arguments.front()) + ")";
            ++target_object->action_count;
            return make_string_value(expression_text.value_or(std::string{}));
        }
        if (leaf == "readmethod" &&
            (!target_object->class_hierarchy.empty() || !target_object->source.empty()))
        {
            if (arguments.empty())
            {
                return make_string_value("");
            }

            const auto source_text =
                read_native_method_source_if_present(*target_object, value_as_string(arguments.front()));
            target_object->last_action = effective_member_path + "(" + value_as_string(arguments.front()) + ")";
            ++target_object->action_count;
            return make_string_value(source_text.value_or(std::string{}));
        }
        if (leaf == "writeexpression" &&
            (!target_object->class_hierarchy.empty() || !target_object->source.empty()))
        {
            if (arguments.size() < 2U)
            {
                return make_empty_value();
            }

            const std::string property_name = trim_copy(value_as_string(arguments[0]));
            const std::string expression_text = value_as_string(arguments[1]);
            const PrgValue assigned_value = evaluate_expression(expression_text, frame);
            if (!write_native_property_if_present(
                    *target_object,
                    property_name,
                    assigned_value,
                    frame,
                    expression_text))
            {
                return make_empty_value();
            }

            target_object->last_action = effective_member_path + "(" + property_name + ")";
            ++target_object->action_count;
            return make_empty_value();
        }
        if (leaf == "writemethod" &&
            (!target_object->class_hierarchy.empty() || !target_object->source.empty()))
        {
            if (arguments.size() < 2U)
            {
                return make_empty_value();
            }

            const std::string method_name = trim_copy(value_as_string(arguments[0]));
            const std::string method_source_text = value_as_string(arguments[1]);
            const bool create_if_missing =
                arguments.size() >= 3U &&
                value_as_bool(arguments[2]);
            if (!write_native_method_source_if_present(
                    *target_object,
                    method_name,
                    method_source_text,
                    create_if_missing))
            {
                return make_empty_value();
            }

            target_object->last_action = effective_member_path + "(" + method_name + ")";
            ++target_object->action_count;
            return make_empty_value();
        }
        if (leaf == "move" &&
            is_native_visual_runtime_object(*target_object))
        {
            if (arguments.empty())
            {
                return make_empty_value();
            }

            const bool left_written = write_native_property_if_present(
                *target_object,
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
                    *target_object,
                    "top",
                    make_number_value(value_as_number(arguments[1])),
                    frame);
            }
            if (arguments.size() >= 3U)
            {
                (void)write_native_property_if_present(
                    *target_object,
                    "width",
                    make_number_value(value_as_number(arguments[2])),
                    frame);
            }
            if (arguments.size() >= 4U)
            {
                (void)write_native_property_if_present(
                    *target_object,
                    "height",
                    make_number_value(value_as_number(arguments[3])),
                    frame);
            }

            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.move",
                              .detail = target_object->prog_id + "." + effective_member_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            last_popped_frame_requested_nodefault = false;
            bool moved_requested_nodefault = false;
            (void)invoke_native_object_method_if_present(
                *target_object,
                "moved",
                frame,
                {},
                {},
                &moved_requested_nodefault);
            (void)consume_last_popped_frame_requested_nodefault();
            (void)moved_requested_nodefault;
            return make_empty_value();
        }
        if (leaf == "refresh" && !target_object->class_hierarchy.empty())
        {
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.refresh",
                              .detail = target_object->prog_id + "." + effective_member_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            last_popped_frame_requested_nodefault = false;
            bool paint_requested_nodefault = false;
            (void)invoke_native_object_method_if_present(
                *target_object,
                "paint",
                frame,
                {},
                {},
                &paint_requested_nodefault);
            (void)consume_last_popped_frame_requested_nodefault();
            (void)paint_requested_nodefault;
            return make_empty_value();
        }
        if ((leaf == "show" || leaf == "hide") &&
            is_native_visual_runtime_object(*target_object))
        {
            const bool visible = leaf == "show";
            (void)write_native_property_if_present(
                *target_object,
                "visible",
                make_boolean_value(visible),
                frame);
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = visible ? "prg.object.show" : "prg.object.hide",
                              .detail = target_object->prog_id + "." + effective_member_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            last_popped_frame_requested_nodefault = false;
            bool handler_requested_nodefault = false;
            (void)invoke_native_object_method_if_present(
                *target_object,
                visible ? "activate" : "deactivate",
                frame,
                {},
                {},
                &handler_requested_nodefault);
            (void)consume_last_popped_frame_requested_nodefault();
            const bool suppress_default_activation = visible && handler_requested_nodefault;
            if (visible && !suppress_default_activation)
            {
                note_representative_active_form(*target_object);
            }
            return make_empty_value();
        }
        if (leaf == "setfocus" &&
            is_native_focusable_runtime_object(*target_object))
        {
            (void)set_native_focus(*target_object, effective_member_path, frame);
            return make_empty_value();
        }
        if (leaf == "resettodefault" && !target_object->class_hierarchy.empty())
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

            const auto default_value = target_object->default_properties.find(normalized_property_name);
            if (default_value == target_object->default_properties.end())
            {
                return make_boolean_value(false);
            }

            std::optional<std::string> default_expression_text;
            if (const auto default_texts =
                    native_default_property_expression_text_by_handle.find(target_object->handle);
                default_texts != native_default_property_expression_text_by_handle.end())
            {
                if (const auto expression_text = default_texts->second.find(normalized_property_name);
                    expression_text != default_texts->second.end())
                {
                    default_expression_text = expression_text->second;
                }
            }

            const bool restored = write_native_property_if_present(
                *target_object,
                property_name,
                default_value->second,
                frame,
                default_expression_text);
            if (!restored)
            {
                return make_boolean_value(false);
            }

            target_object->last_action = effective_member_path + "(" + property_name + ")";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.resettodefault",
                              .detail = target_object->prog_id + "." + normalized_property_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(true);
        }
        const auto before_requery_signature =
            native_list_control_selection_signature(*target_object);
        if (leaf == "requery" && requery_native_list_control(*target_object, frame))
        {
            if (!write_native_list_control_controlsource_target(*target_object, frame))
            {
                return make_boolean_value(false);
            }
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.requery",
                              .detail = target_object->prog_id + "." + effective_member_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            invoke_native_list_control_programmatic_change_if_needed(
                *target_object,
                frame,
                before_requery_signature);
            return make_empty_value();
        }
        if (is_native_olecontrol_host_object(*target_object) && leaf == "doverb")
        {
            RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(*target_object);
            if (object_surface == nullptr)
            {
                return make_boolean_value(false);
            }

            const PrgValue verb = arguments.empty()
                                      ? make_number_value(0.0)
                                      : canonicalize_native_olecontrol_doverb_argument(arguments.front());
            target_object->last_action = effective_member_path + "(" + format_value(verb) + ")";
            ++target_object->action_count;
            events.push_back({.category = "ole.invoke",
                              .detail = target_object->prog_id + "." + effective_member_path + ":" + format_value(verb),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            object_surface->last_action = "activate:" + format_value(verb);
            ++object_surface->action_count;
            return make_boolean_value(true);
        }
        if (is_native_olecontrol_host_object(*target_object) && leaf == "objectverbs")
        {
            return read_native_olecontrol_objectverb_by_index(*target_object, arguments).value_or(make_empty_value());
        }
        if (is_native_olecontrol_host_object(*target_object))
        {
            RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(*target_object);
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
                target_object = object_surface;
            }
        }

        target_object->last_action = effective_member_path + "()";
        ++target_object->action_count;
        events.push_back({.category = "ole.invoke",
                          .detail = target_object->prog_id + "." + effective_member_path,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        if (auto collection_result = invoke_native_collection_method(*target_object, leaf, arguments);
            collection_result.has_value())
        {
            return *collection_result;
        }
        if (normalize_identifier(target_object->prog_id) == "scripting.dictionary")
        {
            auto update_dictionary_count = [&]()
            {
                std::size_t entry_count = 0U;
                for (const auto &[property_name, property_value] : target_object->properties)
                {
                    if (property_name != "count" && property_name != "comparemode")
                    {
                        ++entry_count;
                    }
                }
                target_object->properties["count"] = make_number_value(static_cast<double>(entry_count));
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
                    target_object->properties[key] = arguments[1];
                    update_dictionary_count();
                }
                return make_boolean_value(true);
            }
            if (leaf == "exists" && !arguments.empty())
            {
                const std::string key = key_for_argument(0U);
                return make_boolean_value(!key.empty() && target_object->properties.contains(key));
            }
            if (leaf == "item" && !arguments.empty())
            {
                const std::string key = key_for_argument(0U);
                const auto found = target_object->properties.find(key);
                return found == target_object->properties.end() ? make_empty_value() : found->second;
            }
            if (leaf == "remove" && !arguments.empty())
            {
                const std::string key = key_for_argument(0U);
                if (!key.empty())
                {
                    target_object->properties.erase(key);
                    update_dictionary_count();
                }
                return make_boolean_value(true);
            }
            if (leaf == "removeall")
            {
                const auto comparemode = target_object->properties.find("comparemode");
                const PrgValue comparemode_value = comparemode == target_object->properties.end()
                    ? make_number_value(0.0)
                    : comparemode->second;
                target_object->properties.clear();
                target_object->properties["comparemode"] = comparemode_value;
                target_object->properties["count"] = make_number_value(0.0);
                return make_boolean_value(true);
            }

            const Statement *statement = current_statement();
            const std::string action_text = statement == nullptr
                ? target_object->prog_id + "." + effective_member_path + "()"
                : statement->text;
            record_ole_aerror_context(target_object->prog_id + "." + effective_member_path + "()",
                                      "Copperfin OLE",
                                      target_object->prog_id,
                                      action_text,
                                      1429);
            throw std::runtime_error(
                runtime_text(
                    "Runtime.Prg.Core.Error.OleMemberNotFoundForMethodInvocation",
                    {{"memberIdentifier", target_object->prog_id + "." + effective_member_path}}));
        }

        if (leaf == "add" || leaf == "create" || leaf == "open" || leaf == "item")
        {
            return make_string_value("object:" + target_object->prog_id + "." + effective_member_path + "#" + std::to_string(target_object->handle));
        }
        if (arguments.empty())
        {
            return make_string_value("ole:" + target_object->prog_id + "." + effective_member_path);
        }
        return arguments.front();
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
            normalized_base_class == "formset" ||
            normalized_base_class == "olecontrol" ||
            normalized_base_class == "toolbar" ||
            normalized_base_class == "listbox" ||
            normalized_base_class == "combobox";
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

    RuntimeOleObjectState *PrgRuntimeSession::Impl::representative_application_surface_object()
    {
        if (!representative_application_surface_handle.has_value())
        {
            return nullptr;
        }
        const auto found = ole_objects.find(*representative_application_surface_handle);
        return found == ole_objects.end() ? nullptr : &found->second;
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

    bool PrgRuntimeSession::Impl::consume_last_popped_frame_returned()
    {
        const bool returned = last_popped_frame_returned;
        last_popped_frame_returned = false;
        return returned;
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_expression_user_routine(
        const Frame &source_frame,
        const std::string &identifier,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::string> &raw_arguments,
        const std::vector<std::optional<std::string>> &argument_references,
        std::size_t invocation_start,
        std::size_t invocation_end)
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

        const auto found = find_unqualified_routine_lookup(program.path, identifier);
        if (!found.has_value())
        {
            return std::nullopt;
        }

        const auto invocation_key = std::make_pair(invocation_start, invocation_end);
        if (resumable_expression_dispatch_active &&
            expression_evaluation_depth == resumable_expression_depth &&
            active_expression_continuation != nullptr)
        {
            const auto completed =
                active_expression_continuation->routine_results.find(invocation_key);
            if (completed != active_expression_continuation->routine_results.end())
            {
                return completed->second;
            }
        }

        if (!can_push_frame())
        {
            throw std::runtime_error(call_depth_limit_message());
        }

        std::vector<std::optional<std::string>> resolved_references = argument_references;
        resolved_references.resize(arguments.size());
        const std::size_t raw_argument_count = std::min(arguments.size(), raw_arguments.size());
        for (std::size_t index = 0U; index < raw_argument_count; ++index)
        {
            const std::string raw_argument = trim_copy(raw_arguments[index]);
            if (raw_argument.size() < 3U || raw_argument.front() != '(' || raw_argument.back() != ')')
            {
                continue;
            }
            const std::string source_name = trim_copy(raw_argument.substr(1U, raw_argument.size() - 2U));
            if (is_memory_variable_reference_text(source_name) && find_array(source_name, source_frame) != nullptr)
            {
                resolved_references[index] = make_array_copy_reference(source_name);
            }
        }
        if (udfparms_mode == "REFERENCE")
        {
            const std::size_t candidate_count = std::min(arguments.size(), raw_arguments.size());
            for (std::size_t index = 0U; index < candidate_count; ++index)
            {
                if (resolved_references[index].has_value())
                {
                    continue;
                }

                const std::string candidate = trim_copy(raw_arguments[index]);
                if (is_memory_variable_reference_text(candidate) &&
                    (find_variable(source_frame, candidate) != nullptr || find_array(candidate, source_frame) != nullptr))
                {
                    resolved_references[index] = candidate;
                }
            }
        }

        const std::size_t return_depth = stack.size();
        push_routine_frame(found->program->path, *found->routine, arguments, resolved_references);
        if (resumable_expression_dispatch_active &&
            expression_evaluation_depth == resumable_expression_depth &&
            active_expression_continuation != nullptr)
        {
            active_expression_continuation->awaiting_routine = invocation_key;
            throw ExpressionSuspended{};
        }
        return run_expression_invoked_routine_until_return(return_depth);
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::evaluate_resumable_expression(
        Frame &source_frame,
        const Statement &statement,
        const CursorState *preferred_cursor)
    {
        const std::string &expression = statement.expression;
        if (!source_frame.expression_continuation.has_value() ||
            source_frame.expression_continuation->statement.kind != statement.kind ||
            source_frame.expression_continuation->statement.location.file_path != statement.location.file_path ||
            source_frame.expression_continuation->statement.location.line != statement.location.line ||
            source_frame.expression_continuation->statement.text != statement.text ||
            source_frame.expression_continuation->statement.expression != statement.expression)
        {
            source_frame.expression_continuation =
                ExpressionContinuation{
                    .statement = statement,
                    .primary_checkpoints = {},
                    .routine_results = {},
                    .awaiting_routine = std::nullopt};
        }

        const bool previous_active = resumable_expression_dispatch_active;
        const std::size_t previous_depth = resumable_expression_depth;
        ExpressionContinuation *const previous_continuation =
            active_expression_continuation;
        resumable_expression_dispatch_active = true;
        resumable_expression_depth = expression_evaluation_depth + 1U;
        active_expression_continuation =
            &*source_frame.expression_continuation;
        try
        {
            PrgValue result = preferred_cursor == nullptr
                ? evaluate_expression(expression, source_frame)
                : evaluate_expression(expression, source_frame, preferred_cursor);
            resumable_expression_dispatch_active = previous_active;
            resumable_expression_depth = previous_depth;
            active_expression_continuation = previous_continuation;
            source_frame.expression_continuation.reset();
            return result;
        }
        catch (const ExpressionSuspended &)
        {
            resumable_expression_dispatch_active = previous_active;
            resumable_expression_depth = previous_depth;
            active_expression_continuation = previous_continuation;
            source_frame.expression_routine_return_pending = true;
            return std::nullopt;
        }
        catch (...)
        {
            resumable_expression_dispatch_active = previous_active;
            resumable_expression_depth = previous_depth;
            active_expression_continuation = previous_continuation;
            source_frame.expression_continuation.reset();
            source_frame.command_target_continuation.reset();
            source_frame.command_array_name_continuation.reset();
            source_frame.parameter_default_continuation.reset();
            source_frame.use_command_continuation.reset();
            source_frame.copy_file_continuation.reset();
            source_frame.rename_file_continuation.reset();
            throw;
        }
    }

    bool PrgRuntimeSession::Impl::requery_native_list_control(
        RuntimeOleObjectState &runtime_object,
        const Frame &frame,
        bool require_query_resolution)
    {
        const std::string normalized_base_class =
            normalize_identifier(trim_copy(runtime_object.base_class_name));
        if (normalized_base_class != "combobox" &&
            normalized_base_class != "listbox")
        {
            return false;
        }

        const auto row_source_type_found = runtime_object.properties.find("rowsourcetype");
        const long long row_source_type =
            row_source_type_found == runtime_object.properties.end()
                ? 0LL
                : std::llround(value_as_number(row_source_type_found->second));
        const auto row_source_found = runtime_object.properties.find("rowsource");
        const std::string row_source =
            row_source_found == runtime_object.properties.end()
                ? std::string{}
                : value_as_string(row_source_found->second);

        const auto resolved_column_count = [&]() -> std::size_t
        {
            std::size_t column_count = 1U;
            if (const auto column_count_found = runtime_object.properties.find("columncount");
                column_count_found != runtime_object.properties.end())
            {
                column_count = std::max<std::size_t>(
                    1U,
                    static_cast<std::size_t>(std::max<double>(
                        1.0,
                        value_as_number(column_count_found->second))));
            }
            return column_count;
        };

        const auto build_rows_from_cursor_fields =
            [&](CursorState &cursor,
                const std::vector<std::string> &field_names,
                std::vector<std::vector<PrgValue>> &rows)
        {
            if (field_names.empty())
            {
                rows.clear();
                return;
            }

            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            rows.clear();
            rows.reserve(cursor.record_count);

            for (std::size_t recno = 1U; recno <= cursor.record_count; ++recno)
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                if (!current_record_matches_visibility(cursor, frame, {}))
                {
                    continue;
                }

                const auto record = current_record(cursor);
                if (!record.has_value())
                {
                    continue;
                }

                std::vector<PrgValue> row_values;
                row_values.reserve(field_names.size());
                for (const std::string &field_name : field_names)
                {
                    const auto raw_field = std::find_if(
                        record->values.begin(),
                        record->values.end(),
                        [&](const vfp::DbfRecordValue &value)
                        {
                            return collapse_identifier(value.field_name) ==
                                   collapse_identifier(field_name);
                        });
                    row_values.push_back(
                        raw_field == record->values.end()
                            ? make_string_value("")
                            : record_value_to_prg_value(*raw_field));
                }
                rows.push_back(std::move(row_values));
            }

            restore_cursor_snapshot(cursor, original);
        };

        struct QueryOrderExpression
        {
            std::string expression;
            std::optional<std::size_t> projection_ordinal;
            bool descending = false;
        };

        struct QueryPlan
        {
            enum class JoinKind
            {
                none,
                inner,
                left
            };

            std::string source_designator;
            std::string source_alias;
            std::string joined_source_designator;
            std::string joined_source_alias;
            std::string join_on_expression;
            JoinKind join_kind = JoinKind::none;
            std::vector<std::string> projection_expressions;
            std::vector<std::string> projection_aliases;
            std::string where_expression;
            std::vector<std::string> group_expressions;
            std::string having_expression;
            std::vector<QueryOrderExpression> order_expressions;
            bool distinct = false;
            std::optional<std::size_t> top_count;
        };

        const auto normalize_query_text = [](std::string text) -> std::string
        {
            for (char &ch : text)
            {
                if (ch == ';' || ch == '\r' || ch == '\n' || ch == '\t')
                {
                    ch = ' ';
                }
            }
            std::string normalized;
            normalized.reserve(text.size());
            bool previous_was_space = false;
            for (const char ch : text)
            {
                if (std::isspace(static_cast<unsigned char>(ch)) != 0)
                {
                    if (!previous_was_space)
                    {
                        normalized.push_back(' ');
                        previous_was_space = true;
                    }
                    continue;
                }
                normalized.push_back(ch);
                previous_was_space = false;
            }
            return trim_copy(normalized);
        };

        const auto find_top_level_keyword =
            [](const std::string &upper_text,
               std::size_t start_pos,
               const std::string &keyword) -> std::size_t
        {
            bool in_single_quote = false;
            bool in_double_quote = false;
            int parentheses_depth = 0;

            for (std::size_t index = start_pos; index < upper_text.size(); ++index)
            {
                const char ch = upper_text[index];
                if (in_single_quote)
                {
                    if (ch == '\'' && (index + 1U) < upper_text.size() && upper_text[index + 1U] == '\'')
                    {
                        ++index;
                        continue;
                    }
                    if (ch == '\'')
                    {
                        in_single_quote = false;
                    }
                    continue;
                }
                if (in_double_quote)
                {
                    if (ch == '"')
                    {
                        in_double_quote = false;
                    }
                    continue;
                }
                if (ch == '\'')
                {
                    in_single_quote = true;
                    continue;
                }
                if (ch == '"')
                {
                    in_double_quote = true;
                    continue;
                }
                if (ch == '(')
                {
                    ++parentheses_depth;
                    continue;
                }
                if (ch == ')')
                {
                    if (parentheses_depth > 0)
                    {
                        --parentheses_depth;
                    }
                    continue;
                }
                if (parentheses_depth != 0)
                {
                    continue;
                }
                if (index + keyword.size() > upper_text.size() ||
                    upper_text.compare(index, keyword.size(), keyword) != 0)
                {
                    continue;
                }

                const auto is_word_char = [](char current) -> bool
                {
                    return std::isalnum(static_cast<unsigned char>(current)) != 0 || current == '_';
                };
                const std::size_t keyword_end = index + keyword.size();
                const bool before_ok = index == 0U || !is_word_char(upper_text[index - 1U]);
                const bool after_ok = keyword_end >= upper_text.size() || !is_word_char(upper_text[keyword_end]);
                if (before_ok && after_ok)
                {
                    return index;
                }
            }

            return std::string::npos;
        };

        const auto strip_projection_alias =
            [&](std::string expression) -> std::string
        {
            expression = trim_copy(expression);
            if (expression.empty() || expression == "*")
            {
                return expression;
            }

            const std::string upper_expression = uppercase_copy(expression);
            const std::size_t as_position =
                find_top_level_keyword(upper_expression, 0U, "AS");
            if (as_position != std::string::npos)
            {
                return trim_copy(expression.substr(0U, as_position));
            }
            return expression;
        };

        const auto strip_trailing_join_modifier =
            [&](std::string designator,
                std::string_view modifier_keyword) -> std::string
        {
            designator = trim_copy(std::move(designator));
            if (designator.size() <= modifier_keyword.size())
            {
                return designator;
            }

            const std::string upper_designator = uppercase_copy(designator);
            const std::size_t modifier_position =
                upper_designator.size() - modifier_keyword.size();
            if (upper_designator.compare(
                    modifier_position,
                    modifier_keyword.size(),
                    modifier_keyword) != 0)
            {
                return designator;
            }

            if (std::isspace(static_cast<unsigned char>(
                    designator[modifier_position - 1U])) == 0)
            {
                return designator;
            }

            return trim_copy(designator.substr(0U, modifier_position));
        };

        const auto parse_source_designator_and_alias =
            [&](std::string designator,
                std::string &source_designator,
                std::string &source_alias) -> bool
        {
            designator = trim_copy(std::move(designator));
            if (designator.empty())
            {
                return false;
            }

            const std::string upper_designator = uppercase_copy(designator);
            const std::size_t as_position =
                find_top_level_keyword(upper_designator, 0U, "AS");
            if (as_position != std::string::npos)
            {
                source_designator = trim_copy(designator.substr(0U, as_position));
                source_alias = trim_copy(designator.substr(as_position + 2U));
            }
            else
            {
                const std::size_t whitespace_position = designator.find_first_of(" \t");
                if (whitespace_position == std::string::npos)
                {
                    source_designator = designator;
                    source_alias.clear();
                }
                else
                {
                    source_designator = trim_copy(designator.substr(0U, whitespace_position));
                    source_alias = trim_copy(designator.substr(whitespace_position + 1U));
                }
            }

            if (source_designator.empty() ||
                source_designator.find_first_of(" ,") != std::string::npos)
            {
                return false;
            }
            if (!source_alias.empty() &&
                source_alias.find_first_of(" ,") != std::string::npos)
            {
                return false;
            }
            return true;
        };

        const auto parse_query_plan =
            [&](const std::string &raw_query_text,
                QueryPlan &plan) -> bool
        {
            const std::string query_text = normalize_query_text(raw_query_text);
            if (query_text.empty())
            {
                return false;
            }

            const std::string upper_query = uppercase_copy(query_text);
            if (upper_query.rfind("SELECT ", 0U) != 0U)
            {
                return false;
            }

            std::size_t projection_start = 6U;
            bool distinct = false;
            std::optional<std::size_t> top_count;
            for (;;)
            {
                if (!distinct && upper_query.compare(projection_start, 9U, " DISTINCT") == 0)
                {
                    projection_start += 9U;
                    distinct = true;
                    continue;
                }

                if (!top_count.has_value() &&
                    upper_query.compare(projection_start, 5U, " TOP ") == 0)
                {
                    const std::size_t count_start = projection_start + 5U;
                    std::size_t count_end = count_start;
                    while (count_end < query_text.size() &&
                           std::isdigit(static_cast<unsigned char>(query_text[count_end])) != 0)
                    {
                        ++count_end;
                    }
                    if (count_end == count_start)
                    {
                        return false;
                    }
                    if (count_end < query_text.size() &&
                        std::isspace(static_cast<unsigned char>(query_text[count_end])) == 0)
                    {
                        return false;
                    }

                    try
                    {
                        const unsigned long long parsed_count = std::stoull(
                            query_text.substr(count_start, count_end - count_start));
                        if (parsed_count > std::numeric_limits<std::size_t>::max())
                        {
                            return false;
                        }
                        top_count = static_cast<std::size_t>(parsed_count);
                    }
                    catch (const std::exception &)
                    {
                        return false;
                    }
                    projection_start = count_end;
                    continue;
                }

                break;
            }
            const std::size_t from_position =
                find_top_level_keyword(upper_query, projection_start, "FROM");
            if (from_position == std::string::npos)
            {
                return false;
            }

            const std::size_t after_from = from_position + 4U;
            const std::size_t where_position =
                find_top_level_keyword(upper_query, after_from, "WHERE");
            const std::size_t order_position =
                find_top_level_keyword(upper_query, after_from, "ORDER BY");
            const std::size_t group_position =
                find_top_level_keyword(upper_query, after_from, "GROUP BY");
            const std::size_t having_position =
                find_top_level_keyword(upper_query, after_from, "HAVING");
            const std::size_t into_position =
                find_top_level_keyword(upper_query, after_from, "INTO CURSOR");

            const auto clause_end = [&](std::size_t start) -> std::size_t
            {
                std::size_t end = query_text.size();
                for (const std::size_t candidate : {
                         where_position,
                         order_position,
                         group_position,
                         having_position,
                         into_position})
                {
                    if (candidate != std::string::npos && candidate > start)
                    {
                        end = std::min(end, candidate);
                    }
                }
                return end;
            };

            const std::string projection_clause =
                trim_copy(query_text.substr(projection_start, from_position - projection_start));
            const std::string from_clause =
                trim_copy(query_text.substr(after_from, clause_end(after_from) - after_from));
            if (projection_clause.empty() || from_clause.empty())
            {
                return false;
            }

            const std::string upper_from_clause = uppercase_copy(from_clause);
            std::string primary_source_designator;
            std::string primary_source_alias;
            std::string joined_source_designator;
            std::string joined_source_alias;
            std::string join_on_expression;
            QueryPlan::JoinKind join_kind = QueryPlan::JoinKind::none;
            bool normalize_right_join = false;
            if (const std::size_t join_position =
                    find_top_level_keyword(upper_from_clause, 0U, "JOIN");
                join_position != std::string::npos)
            {
                const std::size_t after_join = join_position + 4U;
                const std::size_t on_position =
                    find_top_level_keyword(upper_from_clause, after_join, "ON");
                if (on_position == std::string::npos)
                {
                    return false;
                }

                primary_source_designator = trim_copy(from_clause.substr(0U, join_position));
                const std::string without_outer =
                    strip_trailing_join_modifier(primary_source_designator, "OUTER");
                if (without_outer != primary_source_designator)
                {
                    const std::string without_left =
                        strip_trailing_join_modifier(without_outer, "LEFT");
                    if (without_left != without_outer)
                    {
                        primary_source_designator = without_left;
                        join_kind = QueryPlan::JoinKind::left;
                    }
                    else
                    {
                        const std::string without_right =
                            strip_trailing_join_modifier(without_outer, "RIGHT");
                        if (without_right != without_outer)
                        {
                            primary_source_designator = without_right;
                            join_kind = QueryPlan::JoinKind::left;
                            normalize_right_join = true;
                        }
                    }
                }
                if (join_kind == QueryPlan::JoinKind::none)
                {
                    const std::string without_left =
                        strip_trailing_join_modifier(primary_source_designator, "LEFT");
                    if (without_left != primary_source_designator)
                    {
                        primary_source_designator = without_left;
                        join_kind = QueryPlan::JoinKind::left;
                    }
                }
                if (join_kind == QueryPlan::JoinKind::none)
                {
                    const std::string without_right =
                        strip_trailing_join_modifier(primary_source_designator, "RIGHT");
                    if (without_right != primary_source_designator)
                    {
                        primary_source_designator = without_right;
                        join_kind = QueryPlan::JoinKind::left;
                        normalize_right_join = true;
                    }
                }
                if (join_kind == QueryPlan::JoinKind::none)
                {
                    primary_source_designator = strip_trailing_join_modifier(
                        primary_source_designator,
                        "INNER");
                    join_kind = QueryPlan::JoinKind::inner;
                }
                const std::string raw_joined_source_designator = trim_copy(
                    from_clause.substr(after_join, on_position - after_join));
                join_on_expression =
                    trim_copy(from_clause.substr(on_position + 2U));
                if (find_top_level_keyword(
                        uppercase_copy(join_on_expression),
                        0U,
                        "JOIN") != std::string::npos)
                {
                    return false;
                }
                if (!parse_source_designator_and_alias(
                        primary_source_designator,
                        primary_source_designator,
                        primary_source_alias) ||
                    !parse_source_designator_and_alias(
                        raw_joined_source_designator,
                        joined_source_designator,
                        joined_source_alias) ||
                    join_on_expression.empty())
                {
                    return false;
                }

                if (normalize_right_join)
                {
                    std::swap(primary_source_designator, joined_source_designator);
                    std::swap(primary_source_alias, joined_source_alias);
                }
            }
            else
            {
                if (!parse_source_designator_and_alias(
                        from_clause,
                        primary_source_designator,
                        primary_source_alias))
                {
                    return false;
                }
            }

            std::vector<std::string> projection_expressions;
            std::vector<std::string> projection_aliases;
            for (std::string projection : split_csv_like(projection_clause))
            {
                projection = trim_copy(std::move(projection));
                std::string projection_alias;
                const std::size_t as_position =
                    find_top_level_keyword(uppercase_copy(projection), 0U, "AS");
                if (as_position != std::string::npos)
                {
                    projection_alias = trim_copy(projection.substr(as_position + 2U));
                    projection = trim_copy(projection.substr(0U, as_position));
                    if (projection_alias.empty())
                    {
                        return false;
                    }
                }
                else
                {
                    projection = strip_projection_alias(std::move(projection));
                }
                if (projection.empty())
                {
                    return false;
                }
                projection_expressions.push_back(std::move(projection));
                projection_aliases.push_back(std::move(projection_alias));
            }
            if (projection_expressions.empty())
            {
                return false;
            }

            std::vector<std::string> group_expressions;
            if (group_position != std::string::npos)
            {
                const std::size_t group_clause_start = group_position + 8U;
                const std::string group_clause = trim_copy(
                    query_text.substr(
                        group_clause_start,
                        clause_end(group_clause_start) - group_clause_start));
                if (group_clause.empty())
                {
                    return false;
                }
                for (std::string group_expression : split_csv_like(group_clause))
                {
                    group_expression = trim_copy(std::move(group_expression));
                    if (group_expression.empty())
                    {
                        return false;
                    }
                    group_expressions.push_back(std::move(group_expression));
                }
            }

            std::string having_expression;
            if (having_position != std::string::npos)
            {
                const std::size_t having_clause_start = having_position + 6U;
                having_expression = trim_copy(
                    query_text.substr(
                        having_clause_start,
                        clause_end(having_clause_start) - having_clause_start));
                if (having_expression.empty())
                {
                    return false;
                }
            }
            if (!joined_source_designator.empty() &&
                (!group_expressions.empty() || !having_expression.empty()))
            {
                return false;
            }

            std::vector<QueryOrderExpression> order_expressions;
            if (order_position != std::string::npos)
            {
                const std::size_t order_clause_start = order_position + 8U;
                const std::size_t order_clause_end =
                    into_position != std::string::npos && into_position > order_clause_start
                        ? into_position
                        : query_text.size();
                const std::string order_clause =
                    trim_copy(query_text.substr(order_clause_start, order_clause_end - order_clause_start));
                for (std::string order_expression : split_csv_like(order_clause))
                {
                    order_expression = trim_copy(order_expression);
                    if (order_expression.empty())
                    {
                        return false;
                    }

                    bool descending = false;
                    const std::string upper_order_expression = uppercase_copy(order_expression);
                    if (upper_order_expression.size() > 5U &&
                        upper_order_expression.compare(upper_order_expression.size() - 5U, 5U, " DESC") == 0)
                    {
                        descending = true;
                        order_expression = trim_copy(
                            order_expression.substr(0U, order_expression.size() - 5U));
                    }
                    else if (upper_order_expression.size() > 4U &&
                             upper_order_expression.compare(upper_order_expression.size() - 4U, 4U, " ASC") == 0)
                    {
                        order_expression = trim_copy(
                            order_expression.substr(0U, order_expression.size() - 4U));
                    }

                    if (order_expression.empty())
                    {
                        return false;
                    }

                    std::optional<std::size_t> projection_ordinal;
                    const bool all_digits = std::all_of(
                        order_expression.begin(),
                        order_expression.end(),
                        [](char current)
                        {
                            return std::isdigit(static_cast<unsigned char>(current)) != 0;
                        });
                    if (all_digits)
                    {
                        const std::size_t ordinal =
                            static_cast<std::size_t>(std::strtoull(order_expression.c_str(), nullptr, 10));
                        if (ordinal > 0U)
                        {
                            projection_ordinal = ordinal;
                        }
                    }

                    order_expressions.push_back({.expression = std::move(order_expression),
                                                 .projection_ordinal = projection_ordinal,
                                                 .descending = descending});
                }
            }

            std::string where_expression;
            if (where_position != std::string::npos)
            {
                const std::size_t where_clause_start = where_position + 5U;
                where_expression = trim_copy(
                    query_text.substr(where_clause_start,
                                      clause_end(where_clause_start) - where_clause_start));
                if (where_expression.empty())
                {
                    return false;
                }

                const std::string upper_where_expression = uppercase_copy(where_expression);
                const std::size_t in_position =
                    find_top_level_keyword(upper_where_expression, 0U, "IN");
                if (in_position != std::string::npos)
                {
                    std::size_t subquery_start = in_position + 2U;
                    while (subquery_start < where_expression.size() &&
                           std::isspace(static_cast<unsigned char>(where_expression[subquery_start])) != 0)
                    {
                        ++subquery_start;
                    }
                    if (subquery_start < where_expression.size() &&
                        where_expression[subquery_start] == '(')
                    {
                        ++subquery_start;
                        while (subquery_start < where_expression.size() &&
                               std::isspace(static_cast<unsigned char>(where_expression[subquery_start])) != 0)
                        {
                            ++subquery_start;
                        }
                        if (upper_where_expression.compare(subquery_start, 6U, "SELECT") == 0)
                        {
                            return false;
                        }
                    }
                }
            }

            plan.source_designator = std::move(primary_source_designator);
            plan.source_alias = std::move(primary_source_alias);
            plan.joined_source_designator = std::move(joined_source_designator);
            plan.joined_source_alias = std::move(joined_source_alias);
            plan.join_on_expression = std::move(join_on_expression);
            plan.join_kind = join_kind;
            plan.projection_expressions = std::move(projection_expressions);
            plan.projection_aliases = std::move(projection_aliases);
            plan.where_expression = std::move(where_expression);
            plan.group_expressions = std::move(group_expressions);
            plan.having_expression = std::move(having_expression);
            plan.order_expressions = std::move(order_expressions);
            plan.distinct = distinct;
            plan.top_count = top_count;
            return true;
        };

        const auto build_rows_from_query_plan =
            [&](CursorState &cursor,
                CursorState *joined_cursor,
                const QueryPlan &plan,
                std::vector<std::vector<PrgValue>> &rows)
        {
            const auto is_numeric_sort_value = [](const PrgValue &value) -> bool
            {
                return value.kind == PrgValueKind::number ||
                       value.kind == PrgValueKind::int64 ||
                       value.kind == PrgValueKind::uint64 ||
                       value.kind == PrgValueKind::currency;
            };
            const auto date_time_set_callback = [this](const std::string &option_name)
            {
                const auto &set_state = current_set_state();
                const auto found = set_state.find(normalize_identifier(option_name));
                return found == set_state.end() ? std::string{} : found->second;
            };
            const auto row_values_equal =
                [&](const std::vector<PrgValue> &left, const std::vector<PrgValue> &right) -> bool
            {
                const auto prg_values_equal = [&](const PrgValue &left_value, const PrgValue &right_value) -> bool
                {
                    if (left_value.kind == PrgValueKind::string || right_value.kind == PrgValueKind::string)
                    {
                        return trim_copy(value_as_string(left_value)) == trim_copy(value_as_string(right_value));
                    }
                    if (left_value.kind == PrgValueKind::boolean || right_value.kind == PrgValueKind::boolean)
                    {
                        return value_as_bool(left_value) == value_as_bool(right_value);
                    }
                    if ((left_value.kind == PrgValueKind::int64 || left_value.kind == PrgValueKind::uint64) &&
                        (right_value.kind == PrgValueKind::int64 || right_value.kind == PrgValueKind::uint64))
                    {
                        return left_value.kind == PrgValueKind::int64
                                   ? (right_value.kind == PrgValueKind::int64
                                          ? left_value.int64_value == right_value.int64_value
                                          : left_value.int64_value >= 0 &&
                                                static_cast<std::uint64_t>(left_value.int64_value) ==
                                                    right_value.uint64_value)
                                   : (right_value.kind == PrgValueKind::uint64
                                          ? left_value.uint64_value == right_value.uint64_value
                                          : right_value.int64_value >= 0 &&
                                                left_value.uint64_value ==
                                                    static_cast<std::uint64_t>(right_value.int64_value));
                    }
                    return std::abs(value_as_number(left_value) - value_as_number(right_value)) < 0.000001;
                };

                if (left.size() != right.size())
                {
                    return false;
                }

                for (std::size_t index = 0U; index < left.size(); ++index)
                {
                    if (!prg_values_equal(left[index], right[index]))
                    {
                        return false;
                    }
                }
                return true;
            };

            struct MaterializedQueryRow
            {
                std::vector<PrgValue> values;
                std::vector<PrgValue> order_keys;
            };

            struct QueryAggregateProjection
            {
                std::string function;
                std::vector<std::string> arguments;
            };

            const auto parse_query_aggregate_projection =
                [](const std::string &expression) -> std::optional<QueryAggregateProjection>
            {
                const std::string trimmed = trim_copy(expression);
                const std::size_t open_parenthesis = trimmed.find('(');
                if (open_parenthesis == std::string::npos || trimmed.empty() || trimmed.back() != ')')
                {
                    return std::nullopt;
                }

                std::string function = lowercase_copy(trim_copy(trimmed.substr(0U, open_parenthesis)));
                if (function == "cnt")
                {
                    function = "count";
                }
                if (function != "count" && function != "sum" && function != "avg" &&
                    function != "average" && function != "min" && function != "max")
                {
                    return std::nullopt;
                }

                const std::string argument_text = trim_copy(
                    trimmed.substr(open_parenthesis + 1U, trimmed.size() - open_parenthesis - 2U));
                QueryAggregateProjection projection{.function = function, .arguments = {}};
                if (!argument_text.empty())
                {
                    projection.arguments = split_csv_like(argument_text);
                    if (std::any_of(
                            projection.arguments.begin(),
                            projection.arguments.end(),
                            [](const std::string &argument) { return trim_copy(argument).empty(); }))
                    {
                        return std::nullopt;
                    }
                }
                return projection;
            };

            std::vector<std::optional<QueryAggregateProjection>> aggregate_projections;
            aggregate_projections.reserve(plan.projection_expressions.size());
            for (const std::string &projection_expression : plan.projection_expressions)
            {
                aggregate_projections.push_back(
                    parse_query_aggregate_projection(projection_expression));
            }
            const bool aggregate_query =
                joined_cursor == nullptr && plan.group_expressions.empty() &&
                plan.having_expression.empty() && !aggregate_projections.empty() &&
                std::all_of(
                    aggregate_projections.begin(),
                    aggregate_projections.end(),
                    [](const std::optional<QueryAggregateProjection> &projection)
                    {
                        return projection.has_value();
                    });
            const bool grouped_query =
                joined_cursor == nullptr &&
                (!plan.group_expressions.empty() || !plan.having_expression.empty());

            const CursorPositionSnapshot original = capture_cursor_snapshot(cursor);
            const std::optional<CursorPositionSnapshot> joined_original =
                joined_cursor == nullptr
                    ? std::nullopt
                    : std::optional<CursorPositionSnapshot>(capture_cursor_snapshot(*joined_cursor));
            DataSessionState &session = current_session_state();
            const auto source_alias_it = session.aliases.find(cursor.work_area);
            const std::optional<std::string> source_alias_before =
                source_alias_it == session.aliases.end()
                    ? std::nullopt
                    : std::optional<std::string>(source_alias_it->second);
            if (!plan.source_alias.empty())
            {
                session.aliases[cursor.work_area] = plan.source_alias;
            }
            std::optional<std::string> joined_alias_before;
            if (joined_cursor != nullptr)
            {
                const auto joined_alias_it = session.aliases.find(joined_cursor->work_area);
                if (joined_alias_it != session.aliases.end())
                {
                    joined_alias_before = joined_alias_it->second;
                }
                if (!plan.joined_source_alias.empty())
                {
                    session.aliases[joined_cursor->work_area] = plan.joined_source_alias;
                }
            }
            std::vector<MaterializedQueryRow> materialized_rows;
            materialized_rows.reserve(cursor.record_count);
            bool aggregate_materialized = false;

            struct QueryGroup
            {
                std::vector<std::size_t> record_numbers;
                std::vector<PrgValue> key_values;
            };

            const auto query_value_literal = [](const PrgValue &value) -> std::string
            {
                if (value.is_null)
                {
                    return ".NULL.";
                }
                if (value.kind == PrgValueKind::string)
                {
                    std::string escaped = value.string_value;
                    std::string::size_type position = 0U;
                    while ((position = escaped.find('\'', position)) != std::string::npos)
                    {
                        escaped.insert(position, 1U, '\'');
                        position += 2U;
                    }
                    return "'" + escaped + "'";
                }
                if (value.kind == PrgValueKind::boolean)
                {
                    return value.boolean_value ? ".T." : ".F.";
                }
                return format_value(value);
            };

            const auto substitute_query_projection_aliases =
                [&](const std::string &expression,
                    const std::vector<PrgValue> &values) -> std::string
            {
                const auto is_word_char = [](char value)
                {
                    return std::isalnum(static_cast<unsigned char>(value)) != 0 || value == '_';
                };
                std::string substituted;
                substituted.reserve(expression.size());
                for (std::size_t index = 0U; index < expression.size();)
                {
                    if (expression[index] == '\'' || expression[index] == '"')
                    {
                        const char quote = expression[index];
                        substituted.push_back(expression[index++]);
                        while (index < expression.size())
                        {
                            substituted.push_back(expression[index]);
                            if (expression[index] == quote)
                            {
                                if (index + 1U < expression.size() && expression[index + 1U] == quote)
                                {
                                    substituted.push_back(expression[++index]);
                                }
                                else
                                {
                                    ++index;
                                    break;
                                }
                            }
                            ++index;
                        }
                        continue;
                    }

                    if (!is_word_char(expression[index]))
                    {
                        substituted.push_back(expression[index++]);
                        continue;
                    }

                    const std::size_t word_start = index;
                    while (index < expression.size() && is_word_char(expression[index]))
                    {
                        ++index;
                    }
                    const std::string word = expression.substr(word_start, index - word_start);
                    bool replaced = false;
                    for (std::size_t alias_index = 0U;
                         alias_index < plan.projection_aliases.size() &&
                         alias_index < values.size();
                         ++alias_index)
                    {
                        if (!plan.projection_aliases[alias_index].empty() &&
                            uppercase_copy(plan.projection_aliases[alias_index]) == uppercase_copy(word))
                        {
                            substituted += query_value_literal(values[alias_index]);
                            replaced = true;
                            break;
                        }
                    }
                    if (!replaced)
                    {
                        substituted += word;
                    }
                }
                return substituted;
            };

            const auto evaluate_group_aggregate =
                [&](const QueryAggregateProjection &aggregate,
                    const std::vector<std::size_t> &record_numbers) -> PrgValue
            {
                const CursorPositionSnapshot group_original = capture_cursor_snapshot(cursor);
                const std::string value_expression =
                    aggregate.arguments.empty() ? std::string{} : aggregate.arguments.front();
                std::size_t matched_count = 0U;
                double sum = 0.0;
                double min_value = 0.0;
                double max_value = 0.0;
                for (const std::size_t recno : record_numbers)
                {
                    move_cursor_to(cursor, static_cast<long long>(recno));
                    if (aggregate.function == "count")
                    {
                        if (value_expression.empty() || trim_copy(value_expression) == "*")
                        {
                            ++matched_count;
                            continue;
                        }
                        const PrgValue value = evaluate_expression(value_expression, frame, &cursor);
                        if (!value.is_null && value.kind != PrgValueKind::empty &&
                            !(value.kind == PrgValueKind::string && trim_copy(value.string_value).empty()))
                        {
                            ++matched_count;
                        }
                        continue;
                    }

                    if (value_expression.empty())
                    {
                        continue;
                    }
                    const auto numeric_value = try_parse_aggregate_numeric_value(
                        evaluate_expression(value_expression, frame, &cursor));
                    if (!numeric_value.has_value())
                    {
                        continue;
                    }
                    if (matched_count == 0U)
                    {
                        min_value = *numeric_value;
                        max_value = *numeric_value;
                    }
                    else
                    {
                        min_value = std::min(min_value, *numeric_value);
                        max_value = std::max(max_value, *numeric_value);
                    }
                    sum += *numeric_value;
                    ++matched_count;
                }
                restore_cursor_snapshot(cursor, group_original);

                if (aggregate.function == "count")
                {
                    return make_number_value(static_cast<double>(matched_count));
                }
                if (matched_count == 0U)
                {
                    return make_number_value(0.0);
                }
                if (aggregate.function == "sum")
                {
                    return make_number_value(sum);
                }
                if (aggregate.function == "avg" || aggregate.function == "average")
                {
                    return make_number_value(sum / static_cast<double>(matched_count));
                }
                if (aggregate.function == "min")
                {
                    return make_number_value(min_value);
                }
                if (aggregate.function == "max")
                {
                    return make_number_value(max_value);
                }
                return make_number_value(0.0);
            };

            const auto substitute_query_aggregate_expressions =
                [&](const std::string &expression,
                    const std::vector<std::size_t> &record_numbers) -> std::string
            {
                std::string substituted;
                substituted.reserve(expression.size());
                for (std::size_t index = 0U; index < expression.size();)
                {
                    if (expression[index] == '\'' || expression[index] == '"')
                    {
                        const char quote = expression[index];
                        substituted.push_back(expression[index++]);
                        while (index < expression.size())
                        {
                            substituted.push_back(expression[index]);
                            if (expression[index] == quote)
                            {
                                if (index + 1U < expression.size() && expression[index + 1U] == quote)
                                {
                                    substituted.push_back(expression[++index]);
                                }
                                else
                                {
                                    ++index;
                                    break;
                                }
                            }
                            ++index;
                        }
                        continue;
                    }

                    const bool is_word_char =
                        std::isalnum(static_cast<unsigned char>(expression[index])) != 0 ||
                        expression[index] == '_';
                    if (!is_word_char)
                    {
                        substituted.push_back(expression[index++]);
                        continue;
                    }

                    const std::size_t word_start = index;
                    while (index < expression.size() &&
                           (std::isalnum(static_cast<unsigned char>(expression[index])) != 0 ||
                            expression[index] == '_'))
                    {
                        ++index;
                    }
                    std::size_t call_start = index;
                    while (call_start < expression.size() &&
                           std::isspace(static_cast<unsigned char>(expression[call_start])) != 0)
                    {
                        ++call_start;
                    }
                    if (call_start >= expression.size() || expression[call_start] != '(')
                    {
                        substituted.append(expression, word_start, index - word_start);
                        continue;
                    }

                    int parentheses_depth = 0;
                    bool in_single_quote = false;
                    bool in_double_quote = false;
                    std::size_t close_parenthesis = std::string::npos;
                    for (std::size_t scan = call_start; scan < expression.size(); ++scan)
                    {
                        const char current = expression[scan];
                        if (in_single_quote)
                        {
                            if (current == '\'' && scan + 1U < expression.size() &&
                                expression[scan + 1U] == '\'')
                            {
                                ++scan;
                            }
                            else if (current == '\'')
                            {
                                in_single_quote = false;
                            }
                            continue;
                        }
                        if (in_double_quote)
                        {
                            if (current == '"')
                            {
                                in_double_quote = false;
                            }
                            continue;
                        }
                        if (current == '\'')
                        {
                            in_single_quote = true;
                        }
                        else if (current == '"')
                        {
                            in_double_quote = true;
                        }
                        else if (current == '(')
                        {
                            ++parentheses_depth;
                        }
                        else if (current == ')' && --parentheses_depth == 0)
                        {
                            close_parenthesis = scan;
                            break;
                        }
                    }

                    if (close_parenthesis == std::string::npos)
                    {
                        substituted.append(expression, word_start, index - word_start);
                        continue;
                    }

                    const auto aggregate = parse_query_aggregate_projection(
                        expression.substr(word_start, close_parenthesis - word_start + 1U));
                    if (!aggregate.has_value())
                    {
                        substituted.append(expression, word_start, index - word_start);
                        continue;
                    }

                    substituted += query_value_literal(
                        evaluate_group_aggregate(*aggregate, record_numbers));
                    index = close_parenthesis + 1U;
                }
                return substituted;
            };

            if (grouped_query)
            {
                std::vector<QueryGroup> groups;
                for (std::size_t recno = 1U; recno <= cursor.record_count; ++recno)
                {
                    move_cursor_to(cursor, static_cast<long long>(recno));
                    if (!current_record_matches_visibility(cursor, frame, {}) ||
                        (!plan.where_expression.empty() &&
                         !value_as_bool(evaluate_expression(plan.where_expression, frame, &cursor))))
                    {
                        continue;
                    }

                    std::vector<PrgValue> key_values;
                    key_values.reserve(plan.group_expressions.size());
                    for (const std::string &group_expression : plan.group_expressions)
                    {
                        key_values.push_back(evaluate_expression(group_expression, frame, &cursor));
                    }
                    auto group = std::find_if(
                        groups.begin(),
                        groups.end(),
                        [&](const QueryGroup &candidate)
                        {
                            return row_values_equal(candidate.key_values, key_values);
                        });
                    if (group == groups.end())
                    {
                        groups.push_back(QueryGroup{.record_numbers = {recno}, .key_values = std::move(key_values)});
                    }
                    else
                    {
                        group->record_numbers.push_back(recno);
                    }
                }

                for (const QueryGroup &group : groups)
                {
                    if (group.record_numbers.empty())
                    {
                        continue;
                    }
                    move_cursor_to(cursor, static_cast<long long>(group.record_numbers.front()));
                    MaterializedQueryRow query_row;
                    for (std::size_t projection_index = 0U;
                         projection_index < plan.projection_expressions.size();
                         ++projection_index)
                    {
                        const std::string &projection_expression =
                            plan.projection_expressions[projection_index];
                        if (aggregate_projections[projection_index].has_value())
                        {
                            query_row.values.push_back(
                                evaluate_group_aggregate(
                                    *aggregate_projections[projection_index],
                                    group.record_numbers));
                        }
                        else
                        {
                            query_row.values.push_back(
                                evaluate_expression(projection_expression, frame, &cursor));
                        }
                    }

                    if (!plan.having_expression.empty())
                    {
                        const std::string having_expression = substitute_query_projection_aliases(
                            substitute_query_aggregate_expressions(
                                plan.having_expression,
                                group.record_numbers),
                            query_row.values);
                        if (!value_as_bool(evaluate_expression(having_expression, frame, &cursor)))
                        {
                            continue;
                        }
                    }

                    for (const QueryOrderExpression &order_expression : plan.order_expressions)
                    {
                        if (order_expression.projection_ordinal.has_value() &&
                            *order_expression.projection_ordinal <= query_row.values.size())
                        {
                            query_row.order_keys.push_back(
                                query_row.values[*order_expression.projection_ordinal - 1U]);
                        }
                        else
                        {
                            query_row.order_keys.push_back(
                                evaluate_expression(
                                    substitute_query_projection_aliases(
                                        order_expression.expression,
                                        query_row.values),
                                    frame,
                                    &cursor));
                        }
                    }
                    materialized_rows.push_back(std::move(query_row));
                }
            }
            else
            {
            for (std::size_t recno = 1U; recno <= cursor.record_count; ++recno)
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                if (!current_record_matches_visibility(cursor, frame, {}))
                {
                    continue;
                }

                const auto record = current_record(cursor);
                if (!record.has_value() && !aggregate_query)
                {
                    continue;
                }

                const auto materialize_current_row = [&]()
                {
                    if (!aggregate_query && !plan.where_expression.empty() &&
                        !value_as_bool(evaluate_expression(plan.where_expression, frame, &cursor)))
                    {
                        return;
                    }

                    MaterializedQueryRow query_row;
                    for (std::size_t projection_index = 0U;
                         projection_index < plan.projection_expressions.size();
                         ++projection_index)
                    {
                        const std::string &projection_expression =
                            plan.projection_expressions[projection_index];
                        if (aggregate_query)
                        {
                            QueryAggregateProjection aggregate =
                                *aggregate_projections[projection_index];
                            if (aggregate.function == "count" &&
                                (aggregate.arguments.empty() ||
                                 (aggregate.arguments.size() == 1U &&
                                  trim_copy(aggregate.arguments.front()) == "*")))
                            {
                                aggregate.arguments.clear();
                            }
                            if (!plan.where_expression.empty())
                            {
                                aggregate.arguments.push_back(plan.where_expression);
                            }
                            query_row.values.push_back(
                                aggregate_function_value(
                                    aggregate.function,
                                    aggregate.arguments,
                                    frame,
                                    &cursor));
                            continue;
                        }
                        if (projection_expression == "*")
                        {
                            for (const auto &field_value : record->values)
                            {
                                query_row.values.push_back(record_value_to_prg_value(field_value));
                            }
                            if (joined_cursor != nullptr)
                            {
                                const auto joined_record = current_record(*joined_cursor);
                                if (joined_record.has_value())
                                {
                                    for (const auto &field_value : joined_record->values)
                                    {
                                        query_row.values.push_back(record_value_to_prg_value(field_value));
                                    }
                                }
                            }
                            continue;
                        }
                        query_row.values.push_back(
                            evaluate_expression(projection_expression, frame, &cursor));
                    }

                    for (const QueryOrderExpression &order_expression : plan.order_expressions)
                    {
                        if (order_expression.projection_ordinal.has_value() &&
                            *order_expression.projection_ordinal <= query_row.values.size())
                        {
                            query_row.order_keys.push_back(
                                query_row.values[*order_expression.projection_ordinal - 1U]);
                        }
                        else
                        {
                            query_row.order_keys.push_back(
                                evaluate_expression(order_expression.expression, frame, &cursor));
                        }
                    }

                    materialized_rows.push_back(std::move(query_row));
                    aggregate_materialized = aggregate_query;
                };

                if (aggregate_query)
                {
                    materialize_current_row();
                    break;
                }

                if (joined_cursor == nullptr)
                {
                    materialize_current_row();
                    continue;
                }

                bool matched_join = false;
                for (std::size_t joined_recno = 1U;
                     joined_recno <= joined_cursor->record_count;
                     ++joined_recno)
                {
                    move_cursor_to(*joined_cursor, static_cast<long long>(joined_recno));
                    if (!current_record_matches_visibility(*joined_cursor, frame, {}))
                    {
                        continue;
                    }

                    if (!plan.join_on_expression.empty() &&
                        !value_as_bool(evaluate_expression(plan.join_on_expression, frame, &cursor)))
                    {
                        continue;
                    }

                    if (!current_record(*joined_cursor).has_value())
                    {
                        continue;
                    }

                    matched_join = true;
                    materialize_current_row();
                }

                if (plan.join_kind == QueryPlan::JoinKind::left && !matched_join)
                {
                    const std::vector<vfp::DbfFieldDescriptor> joined_fields =
                        cursor_field_descriptors(*joined_cursor);
                    auto previous_remote_records = std::move(joined_cursor->remote_records);
                    auto previous_remote_fields = std::move(joined_cursor->remote_fields);
                    const bool previous_remote = joined_cursor->remote;
                    const std::size_t previous_field_count = joined_cursor->field_count;
                    const std::size_t previous_record_count = joined_cursor->record_count;
                    const std::size_t previous_recno = joined_cursor->recno;
                    const bool previous_found = joined_cursor->found;
                    const bool previous_bof = joined_cursor->bof;
                    const bool previous_eof = joined_cursor->eof;

                    vfp::DbfRecord blank_joined_record;
                    blank_joined_record.record_index = 1U;
                    blank_joined_record.values.reserve(joined_fields.size());
                    for (const auto &field : joined_fields)
                    {
                        blank_joined_record.values.push_back(
                            vfp::DbfRecordValue{
                                .field_name = field.name,
                                .field_type = field.type,
                                .display_value = ""});
                    }

                    joined_cursor->remote = true;
                    joined_cursor->field_count = joined_fields.size();
                    joined_cursor->record_count = 1U;
                    joined_cursor->recno = 1U;
                    joined_cursor->found = false;
                    joined_cursor->bof = false;
                    joined_cursor->eof = false;
                    joined_cursor->remote_fields = joined_fields;
                    joined_cursor->remote_records = {std::move(blank_joined_record)};

                    materialize_current_row();

                    joined_cursor->remote_records = std::move(previous_remote_records);
                    joined_cursor->remote_fields = std::move(previous_remote_fields);
                    joined_cursor->remote = previous_remote;
                    joined_cursor->field_count = previous_field_count;
                    joined_cursor->record_count = previous_record_count;
                    joined_cursor->recno = previous_recno;
                    joined_cursor->found = previous_found;
                    joined_cursor->bof = previous_bof;
                    joined_cursor->eof = previous_eof;
                }
            }
            }

            if (aggregate_query && !aggregate_materialized)
            {
                MaterializedQueryRow query_row;
                for (const std::optional<QueryAggregateProjection> &aggregate_projection : aggregate_projections)
                {
                    QueryAggregateProjection aggregate = *aggregate_projection;
                    if (aggregate.function == "count" &&
                        (aggregate.arguments.empty() ||
                         (aggregate.arguments.size() == 1U &&
                          trim_copy(aggregate.arguments.front()) == "*")))
                    {
                        aggregate.arguments.clear();
                    }
                    if (!plan.where_expression.empty())
                    {
                        aggregate.arguments.push_back(plan.where_expression);
                    }
                    query_row.values.push_back(
                        aggregate_function_value(aggregate.function, aggregate.arguments, frame, &cursor));
                }
                materialized_rows.push_back(std::move(query_row));
            }

            if (plan.distinct)
            {
                std::vector<MaterializedQueryRow> distinct_rows;
                distinct_rows.reserve(materialized_rows.size());
                for (auto &query_row : materialized_rows)
                {
                    const bool already_present = std::any_of(
                        distinct_rows.begin(),
                        distinct_rows.end(),
                        [&](const MaterializedQueryRow &candidate)
                        {
                            return row_values_equal(candidate.values, query_row.values);
                        });
                    if (!already_present)
                    {
                        distinct_rows.push_back(std::move(query_row));
                    }
                }
                materialized_rows = std::move(distinct_rows);
            }

            restore_cursor_snapshot(cursor, original);
            if (joined_cursor != nullptr && joined_original.has_value())
            {
                restore_cursor_snapshot(*joined_cursor, *joined_original);
            }
            if (!plan.source_alias.empty())
            {
                if (source_alias_before.has_value())
                {
                    session.aliases[cursor.work_area] = *source_alias_before;
                }
                else
                {
                    session.aliases.erase(cursor.work_area);
                }
            }
            if (joined_cursor != nullptr && !plan.joined_source_alias.empty())
            {
                if (joined_alias_before.has_value())
                {
                    session.aliases[joined_cursor->work_area] = *joined_alias_before;
                }
                else
                {
                    session.aliases.erase(joined_cursor->work_area);
                }
            }

            std::stable_sort(
                materialized_rows.begin(),
                materialized_rows.end(),
                [&](const MaterializedQueryRow &left, const MaterializedQueryRow &right)
                {
                    const std::size_t comparison_count =
                        std::min(left.order_keys.size(), right.order_keys.size());
                    for (std::size_t index = 0U; index < comparison_count; ++index)
                    {
                        const PrgValue &left_key = left.order_keys[index];
                        const PrgValue &right_key = right.order_keys[index];
                        const bool descending = index < plan.order_expressions.size() &&
                                                plan.order_expressions[index].descending;

                        if (is_numeric_sort_value(left_key) && is_numeric_sort_value(right_key))
                        {
                            const double left_number = value_as_number(left_key);
                            const double right_number = value_as_number(right_key);
                            if (left_number == right_number)
                            {
                                continue;
                            }
                            return descending ? right_number < left_number : left_number < right_number;
                        }

                        if (left_key.string_flavor != PrgStringFlavor::none &&
                            right_key.string_flavor != PrgStringFlavor::none)
                        {
                            const auto comparison = compare_date_time_values(
                                left_key,
                                right_key,
                                date_time_set_callback);
                            if (comparison.has_value())
                            {
                                if (*comparison == 0)
                                {
                                    continue;
                                }
                                return descending ? *comparison > 0 : *comparison < 0;
                            }
                        }

                        const std::string left_text = value_as_string(left_key);
                        const std::string right_text = value_as_string(right_key);
                        if (left_text == right_text)
                        {
                            continue;
                        }
                        return descending ? right_text < left_text : left_text < right_text;
                    }
                    return false;
                });

            if (plan.top_count.has_value() &&
                materialized_rows.size() > *plan.top_count)
            {
                materialized_rows.resize(*plan.top_count);
            }

            rows.clear();
            rows.reserve(materialized_rows.size());
            for (auto &query_row : materialized_rows)
            {
                rows.push_back(std::move(query_row.values));
            }
        };

        const auto load_query_file_text =
            [&](const std::string &query_file,
                const std::string &fallback_path) -> std::optional<std::string>
        {
            std::string resolved_query_path =
                resolve_native_prg_program_path(unquote_string(query_file), fallback_path);
            std::error_code ignored;
            std::filesystem::path query_path =
                copperfin::platform::path_from_utf8_string(resolved_query_path);
            const bool physical_query_exists = std::filesystem::exists(query_path, ignored);
            const bool verified_query_exists =
                options.require_verified_file_byte_overrides &&
                find_verified_file_byte_override(query_path) != options.verified_file_byte_overrides.end();
            if (!physical_query_exists && !verified_query_exists && query_path.extension().empty())
            {
                query_path.replace_extension(".qpr");
                resolved_query_path =
                    resolve_native_prg_program_path(
                        copperfin::platform::path_to_utf8_string(query_path), fallback_path);
                query_path = copperfin::platform::path_from_utf8_string(resolved_query_path);
            }

            if (options.require_verified_file_byte_overrides)
            {
                const auto verified_query = find_verified_file_byte_override(query_path);
                if (verified_query != options.verified_file_byte_overrides.end() &&
                    !verified_query->second.empty())
                {
                    return verified_query->second;
                }
            }

            std::filesystem::path snapshot_root;
            const auto query_snapshot = materialize_verified_file_snapshot(
                query_path,
                snapshot_root,
                "Runtime.Prg.Database.Error.VerifiedBytesUnavailable",
                false);
            if (!query_snapshot.has_value())
            {
                return std::nullopt;
            }

            std::ifstream input(*query_snapshot, std::ios::binary);
            if (!input)
            {
                if (!snapshot_root.empty())
                {
                    std::filesystem::remove_all(snapshot_root, ignored);
                }
                return std::nullopt;
            }

            std::ostringstream buffer;
            buffer << input.rdbuf();
            if (!snapshot_root.empty())
            {
                std::filesystem::remove_all(snapshot_root, ignored);
            }
            return buffer.str();
        };

        std::vector<std::vector<PrgValue>> refreshed_rows;
        switch (row_source_type)
        {
        case 1:
        {
            const std::size_t column_count = resolved_column_count();

            const std::vector<std::string> values = split_csv_like(row_source);
            refreshed_rows.reserve((values.size() + column_count - 1U) / column_count);
            for (std::size_t index = 0U; index < values.size(); index += column_count)
            {
                std::vector<PrgValue> row;
                row.reserve(column_count);
                for (std::size_t column = 0U; column < column_count; ++column)
                {
                    const std::size_t value_index = index + column;
                    row.push_back(
                        value_index < values.size()
                            ? make_string_value(values[value_index])
                            : make_string_value(""));
                }
                refreshed_rows.push_back(std::move(row));
            }
            break;
        }
        case 2:
        {
            CursorState *cursor = resolve_cursor_target(row_source);
            if (cursor == nullptr)
            {
                break;
            }

            const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(*cursor);
            if (fields.empty())
            {
                break;
            }

            const std::size_t column_count =
                std::min<std::size_t>(resolved_column_count(), fields.size());
            std::vector<std::string> projected_fields;
            projected_fields.reserve(column_count);
            for (std::size_t index = 0U; index < column_count; ++index)
            {
                projected_fields.push_back(fields[index].name);
            }
            build_rows_from_cursor_fields(*cursor, projected_fields, refreshed_rows);
            break;
        }
        case 5:
        {
            const RuntimeArray *array = find_array(row_source);
            if (array == nullptr)
            {
                break;
            }

            refreshed_rows.reserve(array->rows);
            for (std::size_t row = 1U; row <= array->rows; ++row)
            {
                std::vector<PrgValue> row_values;
                row_values.reserve(array->columns);
                for (std::size_t column = 1U; column <= array->columns; ++column)
                {
                    row_values.push_back(array_value(row_source, row, column));
                }
                refreshed_rows.push_back(std::move(row_values));
            }
            break;
        }
        case 6:
        {
            std::vector<std::string> requested_fields;
            requested_fields.reserve(4U);
            CursorState *cursor = nullptr;
            std::string carried_designator;
            bool invalid_fields_source = false;
            for (std::string field_spec : split_csv_like(row_source))
            {
                field_spec = trim_copy(field_spec);
                if (field_spec.empty())
                {
                    invalid_fields_source = true;
                    break;
                }

                std::string designator;
                std::string field_name = field_spec;
                if (const std::size_t separator = field_spec.find('.');
                    separator != std::string::npos)
                {
                    designator = trim_copy(field_spec.substr(0U, separator));
                    field_name = trim_copy(field_spec.substr(separator + 1U));
                }

                if (field_name.empty())
                {
                    invalid_fields_source = true;
                    break;
                }

                if (designator.empty() && !carried_designator.empty())
                {
                    designator = carried_designator;
                }

                CursorState *field_cursor = designator.empty()
                    ? resolve_cursor_target({})
                    : resolve_cursor_target(designator);
                if (field_cursor == nullptr)
                {
                    invalid_fields_source = true;
                    break;
                }
                if (cursor == nullptr)
                {
                    cursor = field_cursor;
                }
                else if (cursor != field_cursor)
                {
                    invalid_fields_source = true;
                    break;
                }

                if (!designator.empty())
                {
                    carried_designator = designator;
                }
                requested_fields.push_back(field_name);
            }

            if (invalid_fields_source || cursor == nullptr || requested_fields.empty())
            {
                break;
            }

            build_rows_from_cursor_fields(*cursor, requested_fields, refreshed_rows);
            break;
        }
        case 7:
        {
            if (trim_copy(row_source).empty())
            {
                break;
            }

            namespace fs = std::filesystem;
            fs::path pattern_path = copperfin::platform::path_from_utf8_string(row_source);
            if (pattern_path.is_relative())
            {
                pattern_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                    pattern_path;
            }

            const fs::path directory = pattern_path.has_parent_path()
                ? pattern_path.parent_path()
                : copperfin::platform::path_from_utf8_string(current_default_directory());
            const std::string pattern = copperfin::platform::path_to_utf8_string(pattern_path.filename());
            if (pattern.empty())
            {
                break;
            }

            std::vector<std::string> file_names;
            std::error_code directory_error;
            fs::directory_iterator iterator(directory, directory_error);
            const fs::directory_iterator end;
            while (!directory_error && iterator != end)
            {
                const fs::directory_entry entry = *iterator;
                std::error_code entry_error;
                if (entry.is_regular_file(entry_error) &&
                    wildcard_match_insensitive(
                        pattern,
                        copperfin::platform::path_to_utf8_string(entry.path().filename())))
                {
                    file_names.push_back(
                        copperfin::platform::path_to_utf8_string(entry.path().filename()));
                }
                iterator.increment(directory_error);
            }

            std::sort(
                file_names.begin(),
                file_names.end(),
                [](const std::string &left, const std::string &right)
                {
                    const std::string left_folded = lowercase_copy(left);
                    const std::string right_folded = lowercase_copy(right);
                    return left_folded == right_folded ? left < right : left_folded < right_folded;
                });
            refreshed_rows.reserve(file_names.size());
            for (const std::string &file_name : file_names)
            {
                refreshed_rows.push_back({make_string_value(file_name)});
            }
            break;
        }
        case 8:
        {
            CursorState *cursor = row_source.empty()
                ? resolve_cursor_target({})
                : resolve_cursor_target(row_source);
            if (cursor == nullptr)
            {
                break;
            }

            const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(*cursor);
            refreshed_rows.reserve(fields.size());
            for (const vfp::DbfFieldDescriptor &field : fields)
            {
                refreshed_rows.push_back({make_string_value(field.name)});
            }
            break;
        }
        case 9:
        {
            std::string popup_name_text = trim_copy(row_source);
            if (popup_name_text.find('&') != std::string::npos)
            {
                popup_name_text = value_as_string(evaluate_expression(popup_name_text, frame));
            }
            const std::string popup_name = normalize_identifier(
                unquote_identifier(trim_copy(popup_name_text)));
            const auto popup = current_session_state().popup_bar_prompts.find(popup_name);
            if (popup == current_session_state().popup_bar_prompts.end())
            {
                break;
            }

            refreshed_rows.reserve(popup->second.size());
            for (const auto &[bar_number, prompt] : popup->second)
            {
                (void)bar_number;
                refreshed_rows.push_back({make_string_value(prompt)});
            }
            break;
        }
        case 10:
        {
            const std::vector<std::string> source_parts = split_csv_like(row_source);
            if (source_parts.empty() || trim_copy(source_parts.front()).empty())
            {
                break;
            }

            const PrgValue collection_value = evaluate_expression(trim_copy(source_parts.front()), frame);
            const auto collection = resolve_ole_object(collection_value);
            if (!collection.has_value() || !is_native_collection_object(**collection))
            {
                break;
            }

            std::vector<std::string> property_names;
            property_names.reserve(source_parts.size() > 1U ? source_parts.size() - 1U : 0U);
            for (std::size_t index = 1U; index < source_parts.size(); ++index)
            {
                const std::string property_name = trim_copy(unquote_string(source_parts[index]));
                if (!property_name.empty())
                {
                    property_names.push_back(property_name);
                }
            }

            refreshed_rows.reserve((*collection)->collection_items.size());
            for (const PrgValue &item : (*collection)->collection_items)
            {
                const auto item_object = resolve_ole_object(item);
                if (!item_object.has_value())
                {
                    std::vector<PrgValue> row;
                    row.reserve(property_names.size() + 1U);
                    row.push_back(item);
                    row.resize(property_names.size() + 1U, make_empty_value());
                    refreshed_rows.push_back(std::move(row));
                    continue;
                }

                if (property_names.empty())
                {
                    refreshed_rows.push_back({make_string_value("(Object)")});
                    continue;
                }

                std::vector<PrgValue> row;
                row.reserve(property_names.size());
                for (const std::string &property_name : property_names)
                {
                    row.push_back(
                        read_native_property_if_present(**item_object, property_name, frame)
                            .value_or(make_empty_value()));
                }
                refreshed_rows.push_back(std::move(row));
            }
            break;
        }
        case 3:
        case 4:
        {
            const std::optional<std::string> query_text =
                row_source_type == 3
                    ? std::optional<std::string>(row_source)
                    : load_query_file_text(row_source, frame.file_path);
            if (!query_text.has_value())
            {
                if (options.require_verified_file_byte_overrides || require_query_resolution)
                {
                    return false;
                }
                break;
            }
            QueryPlan plan;
            if (!parse_query_plan(*query_text, plan))
            {
                if (require_query_resolution)
                {
                    return false;
                }
                break;
            }

            CursorState *cursor = resolve_cursor_target(plan.source_designator);
            if (cursor == nullptr)
            {
                cursor = resolve_cursor_target_expression(plan.source_designator, frame);
            }
            if (cursor == nullptr)
            {
                if (require_query_resolution)
                {
                    return false;
                }
                break;
            }

            CursorState *joined_cursor = nullptr;
            if (!plan.joined_source_designator.empty())
            {
                joined_cursor = resolve_cursor_target(plan.joined_source_designator);
                if (joined_cursor == nullptr)
                {
                    joined_cursor = resolve_cursor_target_expression(plan.joined_source_designator, frame);
                }
                if (joined_cursor == nullptr)
                {
                    if (require_query_resolution)
                    {
                        return false;
                    }
                    break;
                }
            }

            build_rows_from_query_plan(*cursor, joined_cursor, plan, refreshed_rows);
            break;
        }
        default:
            return false;
        }

        if (row_source_type == 5) {
            const RuntimeArray *array = find_array(row_source);
            if (array != nullptr && array->columns == 1U) {
                normalize_native_list_control_array_range_invariants(runtime_object);
                const long long first_element = std::llround(value_as_number(
                    runtime_object.properties.at("firstelement")));
                const long long number_of_elements = std::llround(value_as_number(
                    runtime_object.properties.at("numberofelements")));
                const std::size_t start = first_element <= 1LL
                    ? 0U
                    : static_cast<unsigned long long>(first_element - 1LL) >=
                            static_cast<unsigned long long>(refreshed_rows.size())
                        ? refreshed_rows.size()
                        : static_cast<std::size_t>(first_element - 1LL);
                const std::size_t available = refreshed_rows.size() - start;
                const std::size_t count = number_of_elements <= 0LL
                    ? available
                    : static_cast<unsigned long long>(number_of_elements) >=
                            static_cast<unsigned long long>(available)
                        ? available
                        : static_cast<std::size_t>(number_of_elements);
                std::vector<std::vector<PrgValue>> ranged_rows;
                ranged_rows.reserve(count);
                for (std::size_t index = 0U; index < count; ++index) {
                    ranged_rows.push_back(std::move(refreshed_rows[start + index]));
                }
                refreshed_rows = std::move(ranged_rows);
            }
        }

        runtime_object.list_rows = std::move(refreshed_rows);
        runtime_object.collection_items.clear();
        runtime_object.collection_items.reserve(runtime_object.list_rows.size());
        for (const auto &row : runtime_object.list_rows)
        {
            runtime_object.collection_items.push_back(
                row.empty()
                    ? make_string_value("")
                    : row.front());
        }
        runtime_object.collection_item_keys.clear();
        runtime_object.list_selected.clear();
        runtime_object.properties["newindex"] = make_number_value(0.0);
        runtime_object.properties["newitemid"] = make_number_value(0.0);
        sync_native_list_control_count(runtime_object);

        if (const auto list_index_found = runtime_object.properties.find("listindex");
            list_index_found != runtime_object.properties.end())
        {
            long long selected_index = std::llround(value_as_number(list_index_found->second));
            const long long row_count = static_cast<long long>(runtime_object.list_rows.size());
            if (selected_index < 0LL)
            {
                selected_index = 0LL;
            }
            if (selected_index > row_count)
            {
                selected_index = row_count;
            }
            list_index_found->second = make_number_value(static_cast<double>(selected_index));
        }

        if (row_source_type == 1)
        {
            normalize_native_list_control_sorted_invariant(runtime_object);
        }
        sync_native_list_control_displayvalue_from_selection(runtime_object);
        return true;
    }

    bool PrgRuntimeSession::Impl::materialize_select_query_rows(
        const std::string &query_text,
        const Frame &frame,
        std::vector<std::vector<PrgValue>> &rows)
    {
        RuntimeOleObjectState query_surface;
        query_surface.base_class_name = "listbox";
        query_surface.properties["rowsourcetype"] = make_number_value(3.0);
        query_surface.properties["rowsource"] = make_string_value(query_text);
        if (!requery_native_list_control(query_surface, frame, true))
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Dispatch.Error.InsertIntoSelectQueryInvalid");
            return false;
        }

        rows = std::move(query_surface.list_rows);
        return true;
    }

    bool PrgRuntimeSession::Impl::native_member_access_allowed(
        const RuntimeOleObjectState &runtime_object,
        NativeMemberVisibility visibility,
        const std::string &owner_class_name,
        const Frame &source_frame)
    {
        if (visibility == NativeMemberVisibility::public_member)
        {
            return true;
        }

        const std::string current_class_name =
            normalize_identifier(source_frame.native_method_class_name);
        const std::string normalized_owner_class_name =
            normalize_identifier(owner_class_name);
        if (current_class_name.empty() || normalized_owner_class_name.empty())
        {
            return false;
        }
        if (visibility == NativeMemberVisibility::hidden_member)
        {
            return current_class_name == normalized_owner_class_name;
        }

        const std::vector<NativeClassLookup> lineage =
            resolved_native_object_class_lineage(runtime_object);
        std::optional<std::size_t> owner_index;
        std::optional<std::size_t> current_index;
        for (std::size_t index = 0U; index < lineage.size(); ++index)
        {
            const std::string class_name = normalize_identifier(
                lineage[index].class_definition->name);
            if (class_name == normalized_owner_class_name)
            {
                owner_index = index;
            }
            if (class_name == current_class_name)
            {
                current_index = index;
            }
        }
        return owner_index.has_value() && current_index.has_value() &&
               *current_index >= *owner_index;
    }

    [[noreturn]] void PrgRuntimeSession::Impl::raise_native_member_access_denied(
        const RuntimeOleObjectState &runtime_object,
        const std::string &member_name,
        bool method_member)
    {
        const Statement *statement = current_statement();
        const std::string detail = runtime_object.prog_id + "." + member_name +
            (method_member ? "()" : "");
        record_ole_aerror_context(
            detail,
            "Copperfin OLE",
            runtime_object.prog_id,
            statement == nullptr ? detail : statement->text,
            1429);
        throw std::runtime_error(runtime_text(
            "Runtime.Prg.Core.Error.OleMemberAccessDenied",
            {{"memberIdentifier", detail}}));
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_native_object_method_body_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &identifier,
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references,
        bool *requested_nodefault)
    {
        if (runtime_object.source.empty())
        {
            return std::nullopt;
        }

        std::string native_method_name;
        std::string native_defining_class_name;
        const auto native_method =
            find_native_object_class_method_lookup(
                runtime_object,
                identifier,
                {},
                {},
                true,
                native_method_name,
                &native_defining_class_name);
        if (!native_method.has_value())
        {
            return std::nullopt;
        }

        if (!native_member_access_allowed(
                runtime_object,
                native_method->routine->visibility,
                native_defining_class_name,
                source_frame))
        {
            raise_native_member_access_denied(runtime_object, identifier, true);
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
        const PrgValue result = run_expression_invoked_routine_until_return(return_depth);
        if (requested_nodefault != nullptr)
        {
            *requested_nodefault = consume_last_popped_frame_requested_nodefault();
        }
        return result;
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
        const std::vector<std::optional<std::string>> &argument_references,
        bool *requested_nodefault,
        bool *returned_false)
    {
        if (returned_false != nullptr)
        {
            *returned_false = false;
        }
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
                const bool binding_after_source_method = (binding.flags & 1) == 0;
                if (binding_after_source_method == after_source_method)
                {
                    const auto delegate_result = invoke_native_event_delegate(
                        binding,
                        {.source_handle = runtime_object.handle,
                         .event_name = normalized_identifier,
                         .event_type = 2},
                        arguments,
                        argument_references);
                    if (returned_false != nullptr &&
                        delegate_result.has_value() &&
                        delegate_result->kind != PrgValueKind::empty &&
                        !value_as_bool(*delegate_result))
                    {
                        *returned_false = true;
                    }
                }
            }
        };

        const std::string active_event_key =
            std::to_string(runtime_object.handle) + ":" + normalized_identifier;
        const bool already_active =
            active_native_event_keys.find(active_event_key) != active_native_event_keys.end();

        if (!bindings.empty() && !already_active)
        {
            ActiveNativeEventKeyGuard active_event_guard(active_native_event_keys, active_event_key);
            invoke_delegates_for_phase(false);
            auto result = invoke_native_object_method_body_if_present(
                runtime_object,
                normalized_identifier,
                source_frame,
                arguments,
                argument_references,
                requested_nodefault);
            if (returned_false != nullptr &&
                result.has_value() &&
                result->kind != PrgValueKind::empty &&
                !value_as_bool(*result))
            {
                *returned_false = true;
            }
            invoke_delegates_for_phase(true);
            return result;
        }

        auto result = invoke_native_object_method_body_if_present(
            runtime_object,
            normalized_identifier,
            source_frame,
            arguments,
            argument_references,
            requested_nodefault);
        if (returned_false != nullptr &&
            result.has_value() &&
            result->kind != PrgValueKind::empty &&
            !value_as_bool(*result))
        {
            *returned_false = true;
        }
        return result;
    }

    void PrgRuntimeSession::Impl::invoke_native_list_control_programmatic_change_if_needed(
        RuntimeOleObjectState &runtime_object,
        const Frame &source_frame,
        const std::optional<std::string> &before_signature)
    {
        if (!before_signature.has_value()) {
            return;
        }

        const auto after_signature = native_list_control_selection_signature(runtime_object);
        if (!after_signature.has_value() || *after_signature == *before_signature) {
            return;
        }

        bool requested_nodefault = false;
        (void)invoke_native_object_method_if_present(
            runtime_object,
            "programmaticchange",
            source_frame,
            {},
            {},
            &requested_nodefault);
        (void)consume_last_popped_frame_requested_nodefault();
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
        const bool source_is_native_list_control =
            source_object.has_value() &&
            (normalize_identifier(trim_copy((*source_object)->base_class_name)) == "combobox" ||
             normalize_identifier(trim_copy((*source_object)->base_class_name)) == "listbox");
        if (!source_object.has_value() ||
            ((*source_object)->source.empty() && !source_is_native_list_control))
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

            const auto found = find_unqualified_routine_lookup(source_frame.file_path, routine_name);
            if (!found.has_value())
            {
                return make_number_value(0.0);
            }

            binding.target_is_routine = true;
            binding.target_program_path = found->program->path;
            binding.delegate_name = found->routine->name;
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

        ActiveNativeEventKeyGuard active_event_guard(active_native_event_keys, active_event_key);
        if (!active_event_guard.engaged)
        {
            return make_boolean_value(true);
        }
        const auto invoke_delegates_for_phase = [&](bool after_source_method)
        {
            for (const NativeEventBinding &binding : bindings)
            {
                const bool binding_after_source_method = (binding.flags & 1) == 0;
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
            case PrgValueKind::currency:
                return arguments[1].currency_value == 0;
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
            case PrgValueKind::currency:
                return arguments[1].currency_value == 10000;
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

        if (normalized_property_name == "header" &&
            is_native_column_runtime_object(runtime_object))
        {
            (void)ensure_native_column_header_surface(runtime_object);
        }

        if (const auto visibility = runtime_object.member_visibility.find(normalized_property_name);
            visibility != runtime_object.member_visibility.end())
        {
            const auto owner = runtime_object.member_visibility_owner.find(normalized_property_name);
            if (!native_member_access_allowed(
                    runtime_object,
                    visibility->second,
                    owner == runtime_object.member_visibility_owner.end() ? std::string{} : owner->second,
                    source_frame))
            {
                raise_native_member_access_denied(runtime_object, property_name, false);
            }
        }

        auto resolve_list_member_cell = [&]() -> std::optional<NativeListControlCellReference>
        {
            const auto literal_cell =
                parse_native_list_control_list_member_cell(runtime_object, property_name);
            if (literal_cell.has_value())
            {
                return literal_cell;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("list");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const std::size_t comma = selector_text->find(',');
            const std::string row_expression = trim_copy(selector_text->substr(0U, comma));
            const std::string column_expression =
                comma == std::string::npos ? std::string("1") : trim_copy(selector_text->substr(comma + 1U));
            if (row_expression.empty() || column_expression.empty())
            {
                return std::nullopt;
            }

            const PrgValue row_value = evaluate_expression(row_expression, source_frame);
            const PrgValue column_value = evaluate_expression(column_expression, source_frame);
            const long long requested_row = std::llround(value_as_number(row_value));
            const long long requested_column = std::llround(value_as_number(column_value));
            if (requested_row < 1LL || requested_column < 1LL)
            {
                return std::nullopt;
            }
            return NativeListControlCellReference{
                .row_slot = static_cast<std::size_t>(requested_row - 1LL),
                .column_slot = static_cast<std::size_t>(requested_column - 1LL)};
        };

        auto resolve_listitem_member_cell = [&]() -> std::optional<NativeListControlItemCellReference>
        {
            const auto literal_cell =
                parse_native_list_control_listitem_member_cell(runtime_object, property_name);
            if (literal_cell.has_value())
            {
                return literal_cell;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("listitem");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const std::size_t comma = selector_text->find(',');
            const std::string item_id_expression = trim_copy(selector_text->substr(0U, comma));
            const std::string column_expression =
                comma == std::string::npos ? std::string("1") : trim_copy(selector_text->substr(comma + 1U));
            if (item_id_expression.empty() || column_expression.empty())
            {
                return std::nullopt;
            }

            const PrgValue item_id_value = evaluate_expression(item_id_expression, source_frame);
            const PrgValue column_value = evaluate_expression(column_expression, source_frame);
            const long long requested_item_id = std::llround(value_as_number(item_id_value));
            const long long requested_column = std::llround(value_as_number(column_value));
            if (requested_item_id < 1LL || requested_column < 1LL)
            {
                return std::nullopt;
            }
            return NativeListControlItemCellReference{
                .item_id = requested_item_id,
                .column_slot = static_cast<std::size_t>(requested_column - 1LL)};
        };

        auto resolve_indextoitemid_member_slot = [&]() -> std::optional<std::size_t>
        {
            const auto literal_slot =
                parse_native_list_control_indextoitemid_member_slot(runtime_object, property_name);
            if (literal_slot.has_value())
            {
                return literal_slot;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("indextoitemid");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const PrgValue selector_value = evaluate_expression(*selector_text, source_frame);
            const long long requested_index = std::llround(value_as_number(selector_value));
            if (requested_index < 1LL)
            {
                return std::nullopt;
            }
            return static_cast<std::size_t>(requested_index - 1LL);
        };

        auto resolve_itemidtoindex_member_item_id = [&]() -> std::optional<long long>
        {
            const auto literal_item_id =
                parse_native_list_control_itemidtoindex_member_item_id(runtime_object, property_name);
            if (literal_item_id.has_value())
            {
                return literal_item_id;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("itemidtoindex");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const PrgValue selector_value = evaluate_expression(*selector_text, source_frame);
            const long long requested_item_id = std::llround(value_as_number(selector_value));
            if (requested_item_id < 1LL)
            {
                return std::nullopt;
            }
            return requested_item_id;
        };

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
            if (const auto list_cell = resolve_list_member_cell();
                list_cell.has_value())
            {
                return read_native_list_control_cell(
                    runtime_object,
                    list_cell->row_slot,
                    list_cell->column_slot);
            }
            if (const auto item_cell = resolve_listitem_member_cell();
                item_cell.has_value())
            {
                return read_native_list_control_item_cell(
                    runtime_object,
                    item_cell->item_id,
                    item_cell->column_slot);
            }
            if (const auto item_data_slot =
                    parse_native_list_control_itemdata_member_slot(runtime_object, property_name);
                item_data_slot.has_value())
            {
                return read_native_list_control_item_data(runtime_object, *item_data_slot);
            }
            if (const auto item_id_slot = resolve_indextoitemid_member_slot();
                item_id_slot.has_value())
            {
                return read_native_list_control_item_id_for_slot(
                    runtime_object,
                    *item_id_slot);
            }
            if (const auto item_index_id = resolve_itemidtoindex_member_item_id();
                item_index_id.has_value())
            {
                return read_native_list_control_index_for_item_id(
                    runtime_object,
                    *item_index_id);
            }
            if (is_native_listcount_member_name(runtime_object, normalized_property_name))
            {
                sync_native_list_control_count(runtime_object);
            }
            if (is_native_activepage_member_name(runtime_object, normalized_property_name))
            {
                normalize_native_pageframe_activepage_invariant(runtime_object);
            }
            if (is_native_listitemid_member_name(runtime_object, normalized_property_name))
            {
                sync_native_list_control_displayvalue_from_selection(runtime_object);
            }
            if (is_native_textbox_selection_member_name(runtime_object, normalized_property_name))
            {
                normalize_native_textbox_selection_invariant(runtime_object);
            }
            if (is_native_textbox_text_member_name(runtime_object, normalized_property_name))
            {
                normalize_native_textbox_text_invariant(runtime_object);
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
            ActiveNativeEventKeyGuard active_event_guard(active_native_event_keys, active_event_key);
            const auto invoke_delegates_for_phase = [&](bool after_source_member)
            {
                for (const NativeEventBinding &binding : bindings)
                {
                    const bool binding_after_source_member = (binding.flags & 1) == 0;
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
            return result;
        }

        return perform_property_read();
    }

    std::optional<std::string> PrgRuntimeSession::Impl::read_native_property_expression_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &property_name)
    {
        const std::string normalized_property_name = normalize_identifier(property_name);
        if (normalized_property_name.empty())
        {
            return std::nullopt;
        }

        if (const auto object_texts = native_property_expression_text_by_handle.find(runtime_object.handle);
            object_texts != native_property_expression_text_by_handle.end())
        {
            if (const auto expression_text = object_texts->second.find(normalized_property_name);
                expression_text != object_texts->second.end())
            {
                return expression_text->second;
            }
        }

        if (const auto property = runtime_object.properties.find(normalized_property_name);
            property != runtime_object.properties.end())
        {
            return serialize_runtime_expression_text(property->second);
        }

        if (is_native_olecontrol_host_object(runtime_object))
        {
            RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(runtime_object);
            if (object_surface != nullptr && object_surface->handle != runtime_object.handle)
            {
                return read_native_property_expression_if_present(*object_surface, property_name);
            }
        }

        return std::nullopt;
    }

    std::optional<std::string> PrgRuntimeSession::Impl::read_native_method_source_if_present(
        const RuntimeOleObjectState &runtime_object,
        const std::string &method_name)
    {
        const std::string normalized_method_name = normalize_identifier(method_name);
        if (normalized_method_name.empty() || runtime_object.source.empty())
        {
            return std::nullopt;
        }

        std::string method_program_path;
        std::string qualified_method_name;
        const Routine *method =
            find_native_object_method(runtime_object,
                                      normalized_method_name,
                                      method_program_path,
                                      qualified_method_name);
        (void)qualified_method_name;
        if (method == nullptr)
        {
            return std::nullopt;
        }

        if (const auto override_text =
                native_method_source_text_by_key.find(
                    make_native_method_override_key(method_program_path, qualified_method_name));
            override_text != native_method_source_text_by_key.end())
        {
            return override_text->second;
        }

        Program &method_program = load_program(method_program_path);
        if (!method_program.source_lines.empty() &&
            method->declaration_location.line > 0 &&
            method->body_end_line_exclusive > method->declaration_location.line)
        {
            const std::size_t body_start_index = method->declaration_location.line;
            const std::size_t body_end_index =
                std::min(method->body_end_line_exclusive - 1U, method_program.source_lines.size());
            if (body_start_index < body_end_index)
            {
                std::string source_text;
                for (std::size_t line_index = body_start_index; line_index < body_end_index; ++line_index)
                {
                    if (!source_text.empty())
                    {
                        source_text += "\n";
                    }
                    source_text += method_program.source_lines[line_index];
                }
                return source_text;
            }
        }

        std::string source_text;
        for (const Statement &statement : method->statements)
        {
            if (!source_text.empty())
            {
                source_text += "\n";
            }
            source_text += statement.text;
        }
        return source_text;
    }

    bool PrgRuntimeSession::Impl::write_native_method_source_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &method_name,
        const std::string &method_source_text,
        bool create_if_missing)
    {
        const std::string normalized_method_name = normalize_identifier(method_name);
        if (normalized_method_name.empty() || runtime_object.source.empty())
        {
            return false;
        }

        const auto starting_class_lookup =
            [&]() -> std::optional<NativeClassLookup>
            {
                std::vector<NativeClassLookup> lineage =
                    resolved_native_object_class_lineage(runtime_object);
                return lineage.empty()
                    ? std::nullopt
                    : std::optional<NativeClassLookup>(lineage.back());
            }();
        if (!starting_class_lookup.has_value() ||
            starting_class_lookup->class_definition == nullptr)
        {
            return false;
        }

        const auto starting_method =
            starting_class_lookup->class_definition->methods.find(
                normalized_method_name);
        std::string qualified_method_name;
        const auto inherited_method_lookup =
            find_native_object_class_method_lookup(
                runtime_object,
                normalized_method_name,
                {},
                {},
                true,
                qualified_method_name);
        const bool create_on_starting_class =
            create_if_missing &&
            starting_method == starting_class_lookup->class_definition->methods.end();
        const auto &method_lookup =
            create_on_starting_class
                ? std::optional<NativeMethodLookup>{}
                : inherited_method_lookup;
        if (!create_on_starting_class &&
            (!method_lookup.has_value() || method_lookup->routine == nullptr))
        {
            return false;
        }

        const std::string temp_routine_name = "__CopperfinWriteMethodTemp";
        const bool parse_as_function =
            method_lookup.has_value() &&
            method_lookup->routine != nullptr &&
            method_lookup->routine->kind == RoutineKind::function;
        std::error_code ignored;
        std::filesystem::create_directories(runtime_temp_directory, ignored);
        const std::filesystem::path temp_path =
            runtime_temp_directory /
            ("writemethod_" + std::to_string(runtime_instance_id) + "_" +
             std::to_string(static_cast<unsigned long long>(executed_statement_count)) + ".prg");

        {
            std::ofstream output(temp_path, std::ios::binary | std::ios::trunc);
            if (!output)
            {
                return false;
            }

            if (parse_as_function)
            {
                output << "FUNCTION " << temp_routine_name << "\n";
                output << method_source_text << "\n";
                output << "ENDFUNC\n";
            }
            else
            {
                output << "PROCEDURE " << temp_routine_name << "\n";
                output << method_source_text << "\n";
                output << "ENDPROC\n";
            }
        }

        Program parsed_program = parse_program(copperfin::platform::path_to_utf8_string(temp_path));
        std::filesystem::remove(temp_path, ignored);

        const auto parsed_method =
            parsed_program.routines.find(normalize_identifier(temp_routine_name));
        if (parsed_method == parsed_program.routines.end())
        {
            return false;
        }

        Routine updated_routine = parsed_method->second;
        const PrgClassDefinition *target_class_definition = nullptr;
        const Program *target_program = nullptr;
        if (create_on_starting_class)
        {
            updated_routine.name = trim_copy(method_name);
            updated_routine.kind = RoutineKind::procedure;
            updated_routine.declaration_location = {};
            updated_routine.body_end_line_exclusive = 0;
            target_class_definition = starting_class_lookup->class_definition;
            target_program = starting_class_lookup->program;

            std::size_t line_number =
                target_class_definition->declaration_location.line;
            for (Statement &statement : updated_routine.statements)
            {
                statement.location.file_path = target_program->path;
                statement.location.line = line_number;
                ++line_number;
            }
        }
        else
        {
            updated_routine.name = method_lookup->routine->name;
            updated_routine.kind = method_lookup->routine->kind;
            updated_routine.declaration_location =
                method_lookup->routine->declaration_location;
            updated_routine.body_end_line_exclusive =
                updated_routine.declaration_location.line + updated_routine.statements.size() + 1U;
            target_class_definition = method_lookup->class_definition;
            target_program = method_lookup->program;

            std::size_t line_number = updated_routine.declaration_location.line;
            for (Statement &statement : updated_routine.statements)
            {
                statement.location.file_path =
                    updated_routine.declaration_location.file_path;
                statement.location.line = line_number;
                ++line_number;
            }
        }

        if (target_class_definition == nullptr || target_program == nullptr)
        {
            return false;
        }

        auto &mutable_methods =
            const_cast<PrgClassDefinition *>(target_class_definition)->methods;
        mutable_methods[normalized_method_name] = updated_routine;
        const std::string defining_class_name =
            target_class_definition->name.empty()
                ? runtime_object.prog_id
                : target_class_definition->name;
        qualified_method_name = defining_class_name + "." + updated_routine.name;
        native_method_source_text_by_key[make_native_method_override_key(
            target_program->path,
            qualified_method_name)] = method_source_text;
        if (!runtime_object_member_matches(
                runtime_object.methods,
                normalized_method_name))
        {
            runtime_object.methods.push_back(updated_routine.name);
        }
        return true;
    }

    bool PrgRuntimeSession::Impl::write_native_property_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &property_name,
        const PrgValue &assigned_value,
        const Frame &source_frame,
        std::optional<std::string> assigned_expression_text)
    {
        const std::string normalized_property_name = normalize_identifier(property_name);
        if (normalized_property_name.empty())
        {
            return false;
        }

        if (normalized_property_name == "showtips" &&
            normalize_identifier(trim_copy(runtime_object.prog_id)) == "_screen")
        {
            representative_application_show_tips = value_as_bool(assigned_value);
        }

        const std::string property_assignment_key =
            std::to_string(runtime_object.handle) + ":" + normalized_property_name;

        const auto before_list_control_signature =
            native_list_control_selection_signature(runtime_object);

        if (const auto visibility = runtime_object.member_visibility.find(normalized_property_name);
            visibility != runtime_object.member_visibility.end())
        {
            const auto owner = runtime_object.member_visibility_owner.find(normalized_property_name);
            if (!native_member_access_allowed(
                    runtime_object,
                    visibility->second,
                    owner == runtime_object.member_visibility_owner.end() ? std::string{} : owner->second,
                    source_frame))
            {
                raise_native_member_access_denied(runtime_object, property_name, false);
            }
        }

        const auto remember_property_expression = [&]()
        {
            if (normalized_property_name.empty())
            {
                return;
            }

            std::string expression_text = assigned_expression_text.value_or(std::string{});
            expression_text = trim_copy(expression_text);
            if (expression_text.empty())
            {
                expression_text = serialize_runtime_expression_text(assigned_value);
            }
            native_property_expression_text_by_handle[runtime_object.handle][normalized_property_name] =
                std::move(expression_text);
        };

        auto resolve_selected_member_slot = [&]() -> std::optional<std::size_t>
        {
            const auto literal_slot =
                parse_native_list_control_selected_member_slot(runtime_object, property_name);
            if (literal_slot.has_value())
            {
                return literal_slot;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("selected");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const PrgValue selector_value = evaluate_expression(
                *selector_text,
                source_frame);
            const long long requested_index = std::llround(value_as_number(selector_value));
            if (requested_index < 1LL)
            {
                return std::nullopt;
            }
            return static_cast<std::size_t>(requested_index - 1LL);
        };

        auto resolve_selectedid_member_item_id = [&]() -> std::optional<long long>
        {
            const auto literal_item_id =
                parse_native_list_control_selectedid_member_item_id(runtime_object, property_name);
            if (literal_item_id.has_value())
            {
                return literal_item_id;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("selectedid");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const PrgValue selector_value = evaluate_expression(
                *selector_text,
                source_frame);
            const long long requested_item_id = std::llround(value_as_number(selector_value));
            if (requested_item_id < 1LL)
            {
                return std::nullopt;
            }
            return requested_item_id;
        };

        auto resolve_list_member_cell = [&]() -> std::optional<NativeListControlCellReference>
        {
            const auto literal_cell =
                parse_native_list_control_list_member_cell(runtime_object, property_name);
            if (literal_cell.has_value())
            {
                return literal_cell;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("list");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const std::size_t comma = selector_text->find(',');
            const std::string row_expression = trim_copy(selector_text->substr(0U, comma));
            const std::string column_expression =
                comma == std::string::npos ? std::string("1") : trim_copy(selector_text->substr(comma + 1U));
            if (row_expression.empty() || column_expression.empty())
            {
                return std::nullopt;
            }

            const PrgValue row_value = evaluate_expression(row_expression, source_frame);
            const PrgValue column_value = evaluate_expression(column_expression, source_frame);
            const long long requested_row = std::llround(value_as_number(row_value));
            const long long requested_column = std::llround(value_as_number(column_value));
            if (requested_row < 1LL || requested_column < 1LL)
            {
                return std::nullopt;
            }
            return NativeListControlCellReference{
                .row_slot = static_cast<std::size_t>(requested_row - 1LL),
                .column_slot = static_cast<std::size_t>(requested_column - 1LL)};
        };

        auto resolve_listitem_member_cell = [&]() -> std::optional<NativeListControlItemCellReference>
        {
            const auto literal_cell =
                parse_native_list_control_listitem_member_cell(runtime_object, property_name);
            if (literal_cell.has_value())
            {
                return literal_cell;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("listitem");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const std::size_t comma = selector_text->find(',');
            const std::string item_id_expression = trim_copy(selector_text->substr(0U, comma));
            const std::string column_expression =
                comma == std::string::npos ? std::string("1") : trim_copy(selector_text->substr(comma + 1U));
            if (item_id_expression.empty() || column_expression.empty())
            {
                return std::nullopt;
            }

            const PrgValue item_id_value = evaluate_expression(item_id_expression, source_frame);
            const PrgValue column_value = evaluate_expression(column_expression, source_frame);
            const long long requested_item_id = std::llround(value_as_number(item_id_value));
            const long long requested_column = std::llround(value_as_number(column_value));
            if (requested_item_id < 1LL || requested_column < 1LL)
            {
                return std::nullopt;
            }
            return NativeListControlItemCellReference{
                .item_id = requested_item_id,
                .column_slot = static_cast<std::size_t>(requested_column - 1LL)};
        };

        auto resolve_indextoitemid_member_slot = [&]() -> std::optional<std::size_t>
        {
            const auto literal_slot =
                parse_native_list_control_indextoitemid_member_slot(runtime_object, property_name);
            if (literal_slot.has_value())
            {
                return literal_slot;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("indextoitemid");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const PrgValue selector_value = evaluate_expression(*selector_text, source_frame);
            const long long requested_index = std::llround(value_as_number(selector_value));
            if (requested_index < 1LL)
            {
                return std::nullopt;
            }
            return static_cast<std::size_t>(requested_index - 1LL);
        };

        auto resolve_itemidtoindex_member_item_id = [&]() -> std::optional<long long>
        {
            const auto literal_item_id =
                parse_native_list_control_itemidtoindex_member_item_id(runtime_object, property_name);
            if (literal_item_id.has_value())
            {
                return literal_item_id;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("itemidtoindex");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const PrgValue selector_value = evaluate_expression(*selector_text, source_frame);
            const long long requested_item_id = std::llround(value_as_number(selector_value));
            if (requested_item_id < 1LL)
            {
                return std::nullopt;
            }
            return requested_item_id;
        };

        const auto perform_property_write = [&]() -> bool
        {
            if (is_native_textbox_text_member_name(runtime_object, normalized_property_name))
            {
                return false;
            }
            const bool assigner_reentrant =
                active_native_property_assignments.find(property_assignment_key) !=
                active_native_property_assignments.end();
            if (!assigner_reentrant)
            {
                ActiveNativePropertyAssignmentGuard active_property_assignment_guard(
                    active_native_property_assignments,
                    property_assignment_key);
                if (invoke_native_object_method_body_if_present(
                        runtime_object,
                        normalized_property_name + "_assign",
                        source_frame,
                        {assigned_value},
                        {}).has_value())
                {
                    remember_property_expression();
                    return true;
                }
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
                !is_native_listcount_member_name(runtime_object, normalized_property_name) &&
                !is_native_newindex_member_name(runtime_object, normalized_property_name) &&
                !is_native_newitemid_member_name(runtime_object, normalized_property_name) &&
                !is_native_name_member_name(runtime_object, normalized_property_name) &&
                !is_native_splitbar_member_name(runtime_object, normalized_property_name) &&
                !is_native_leftcolumn_member_name(runtime_object, normalized_property_name) &&
                !is_native_grid_childorder_member_name(runtime_object, normalized_property_name) &&
                !is_native_relation_onetomany_member_name(runtime_object, normalized_property_name) &&
                !is_native_grid_activecolumn_member_name(runtime_object, normalized_property_name) &&
                !is_native_grid_activerow_member_name(runtime_object, normalized_property_name) &&
                !is_native_grid_relativecolumn_member_name(runtime_object, normalized_property_name) &&
                !is_native_grid_relativerow_member_name(runtime_object, normalized_property_name) &&
                !is_native_form_desktop_member_name(runtime_object, normalized_property_name) &&
                !is_native_form_show_in_taskbar_member_name(runtime_object, normalized_property_name) &&
                !is_native_form_whats_this_button_member_name(runtime_object, normalized_property_name) &&
                !is_native_form_scrollbars_member_name(runtime_object, normalized_property_name) &&
                !is_native_grid_scrollbars_member_name(runtime_object, normalized_property_name) &&
                !is_native_movable_member_name(runtime_object, normalized_property_name) &&
                !is_native_olecontrol_creation_time_member_name(runtime_object, normalized_property_name) &&
                !is_native_olecontrol_object_member_name(runtime_object, normalized_property_name) &&
                !is_native_olecontrol_inspection_member_name(runtime_object, normalized_property_name) &&
                !is_native_child_parent_member_name(runtime_object, normalized_property_name) &&
                !is_native_integralheight_member_name(runtime_object, normalized_property_name) &&
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
                    const bool wrote =
                        write_native_column_bound_property(runtime_object, assigned_value);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (is_native_textbox_selection_member_name(runtime_object, normalized_property_name))
                {
                    const bool wrote = write_native_textbox_selection_property(
                        runtime_object,
                        normalized_property_name,
                        assigned_value);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (is_native_columnorder_member_name(runtime_object, normalized_property_name))
                {
                    const bool wrote =
                        write_native_columnorder_property(runtime_object, assigned_value);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (is_native_columncount_member_name(runtime_object, normalized_property_name) &&
                    is_native_grid_runtime_object(runtime_object))
                {
                    const bool wrote = write_native_grid_columncount_property(
                        runtime_object,
                        assigned_value,
                        source_frame);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (is_native_pagecount_member_name(runtime_object, normalized_property_name) &&
                    is_native_pageframe_runtime_object(runtime_object))
                {
                    const bool wrote = write_native_pageframe_pagecount_property(
                        runtime_object,
                        assigned_value,
                        source_frame);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (is_native_listitemid_member_name(runtime_object, normalized_property_name))
                {
                    const bool wrote =
                        write_native_list_control_item_id(runtime_object, assigned_value) &&
                        write_native_list_control_controlsource_target(runtime_object, source_frame);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (is_native_topitemid_member_name(runtime_object, normalized_property_name))
                {
                    const bool wrote = write_native_list_control_top_item_id(
                        runtime_object,
                        assigned_value);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (is_native_topindex_member_name(runtime_object, normalized_property_name))
                {
                    const bool wrote = write_native_list_control_top_index(
                        runtime_object,
                        assigned_value);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (is_native_activepage_member_name(runtime_object, normalized_property_name))
                {
                    runtime_object.properties[normalized_property_name] = assigned_value;
                    normalize_native_pageframe_activepage_invariant(runtime_object);
                    remember_property_expression();
                    return true;
                }
                if (normalized_property_name == "value" &&
                    (normalize_identifier(runtime_object.base_class_name) == "combobox" ||
                     normalize_identifier(runtime_object.base_class_name) == "listbox"))
                {
                    const bool wrote =
                        write_native_list_control_value(runtime_object, assigned_value) &&
                        write_native_list_control_controlsource_target(runtime_object, source_frame);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (const auto list_cell = resolve_list_member_cell();
                    list_cell.has_value())
                {
                    return write_native_list_control_cell(
                        runtime_object,
                        list_cell->row_slot,
                        list_cell->column_slot,
                        assigned_value);
                }
                if (const auto item_cell = resolve_listitem_member_cell();
                    item_cell.has_value())
                {
                    return write_native_list_control_item_cell(
                        runtime_object,
                        item_cell->item_id,
                        item_cell->column_slot,
                        assigned_value);
                }
                if (const auto item_data_slot =
                        parse_native_list_control_itemdata_member_slot(runtime_object, property_name);
                    item_data_slot.has_value())
                {
                    return write_native_list_control_item_data(
                        runtime_object,
                        *item_data_slot,
                        assigned_value);
                }
                if (resolve_indextoitemid_member_slot().has_value() ||
                    resolve_itemidtoindex_member_item_id().has_value())
                {
                    return false;
                }
                if (const auto selected_slot = resolve_selected_member_slot();
                    selected_slot.has_value())
                {
                    return write_native_list_control_selected_slot(
                               runtime_object,
                               *selected_slot,
                               assigned_value) &&
                           write_native_list_control_controlsource_target(runtime_object, source_frame);
                }
                if (const auto selected_item_id = resolve_selectedid_member_item_id();
                    selected_item_id.has_value())
                {
                    return write_native_list_control_selected_item_id(
                               runtime_object,
                               *selected_item_id,
                               assigned_value) &&
                           write_native_list_control_controlsource_target(runtime_object, source_frame);
                }
                if (is_native_controlsource_member_name(runtime_object, normalized_property_name) &&
                    is_native_column_runtime_object(runtime_object))
                {
                    const bool wrote =
                        write_native_column_controlsource_property(runtime_object, assigned_value);
                    if (wrote)
                    {
                        remember_property_expression();
                    }
                    return wrote;
                }
                if (normalized_property_name == "readonly" &&
                    native_combobox_readonly_assignment_blocked(runtime_object, assigned_value))
                {
                    return false;
                }
                if (is_native_moverbars_member_name(runtime_object, normalized_property_name) &&
                    !native_listbox_moverbars_row_source_supported(runtime_object))
                {
                    return false;
                }
                const bool was_boundto =
                    normalized_property_name == "boundto" &&
                    native_list_control_boundto_enabled(runtime_object);
                runtime_object.properties[normalized_property_name] = assigned_value;
                if (normalized_property_name == "boundto")
                {
                    update_native_list_control_boundto_index_value_mode(runtime_object, was_boundto);
                }
                if (normalized_property_name == "controlsource")
                {
                    refresh_native_list_control_controlsource_value_kind_hint(
                        runtime_object,
                        [&](const std::string& controlsource_text) -> std::optional<PrgValue>
                        {
                            const PrgValue variable_value =
                                lookup_variable(source_frame, controlsource_text);
                            if (variable_value.kind != PrgValueKind::empty)
                            {
                                return variable_value;
                            }

                            const auto field_value =
                                resolve_field_value(controlsource_text, resolve_cursor_target({}));
                            if (!field_value.has_value() ||
                                field_value->kind == PrgValueKind::empty)
                            {
                                return std::nullopt;
                            }
                            return *field_value;
                        });
                }
                if (normalized_property_name == "style" ||
                    normalized_property_name == "readonly")
                {
                    normalize_native_combobox_readonly_invariant(runtime_object);
                }
                if (normalized_property_name == "multiselect")
                {
                    normalize_native_listbox_multiselect_invariant(runtime_object);
                }
                if (normalized_property_name == "sorted" ||
                    normalized_property_name == "rowsourcetype")
                {
                    normalize_native_list_control_sorted_invariant(runtime_object);
                }
                if (normalized_property_name == "moverbars" ||
                    normalized_property_name == "rowsourcetype")
                {
                    normalize_native_listbox_moverbars_invariant(runtime_object);
                }
                if (normalized_property_name == "autohidescrollbar")
                {
                    normalize_native_listbox_autohidescrollbar_invariant(runtime_object);
                }
                if (normalized_property_name == "mousepointer")
                {
                    const double mouse_pointer = value_as_number(assigned_value);
                    runtime_object.properties[normalized_property_name] = make_number_value(
                        std::isfinite(mouse_pointer) && mouse_pointer >= 0.0
                            ? static_cast<double>(std::llround(mouse_pointer))
                            : 0.0);
                }
                if (normalized_property_name == "fontsize")
                {
                    const double font_size = value_as_number(assigned_value);
                    runtime_object.properties[normalized_property_name] = make_number_value(
                        std::isfinite(font_size) && font_size >= 0.0 ? font_size : 0.0);
                }
                if (normalized_property_name == "fontcharset")
                {
                    normalize_native_visual_fontcharset_invariant(runtime_object);
                }
                if (normalized_property_name == "righttoleft")
                {
                    normalize_native_visual_righttoleft_invariant(runtime_object);
                }
                if (normalized_property_name == "wordwrap")
                {
                    normalize_native_visual_wordwrap_invariant(runtime_object);
                }
                if (normalized_property_name == "fontbold")
                {
                    runtime_object.properties[normalized_property_name] = make_boolean_value(
                        value_as_bool(assigned_value));
                }
                if (normalized_property_name == "fontitalic")
                {
                    runtime_object.properties[normalized_property_name] = make_boolean_value(
                        value_as_bool(assigned_value));
                }
                if (normalized_property_name == "fontunderline")
                {
                    runtime_object.properties[normalized_property_name] = make_boolean_value(
                        value_as_bool(assigned_value));
                }
                if (normalized_property_name == "fontstrikethru")
                {
                    runtime_object.properties[normalized_property_name] = make_boolean_value(
                        value_as_bool(assigned_value));
                }
                if (normalized_property_name == "fontoutline")
                {
                    runtime_object.properties[normalized_property_name] = make_boolean_value(
                        value_as_bool(assigned_value));
                }
                if (normalized_property_name == "fontshadow")
                {
                    runtime_object.properties[normalized_property_name] = make_boolean_value(
                        value_as_bool(assigned_value));
                }
                if (normalized_property_name == "alignment")
                {
                    normalize_native_visual_alignment_invariant(runtime_object);
                }
                if (normalized_property_name == "scalemode")
                {
                    normalize_native_form_scalemode_invariant(runtime_object);
                }
                if (is_native_form_size_limit_member_name(runtime_object, normalized_property_name))
                {
                    normalize_native_form_size_limit_invariant(runtime_object);
                }
                if (normalized_property_name == "drawstyle")
                {
                    normalize_native_form_drawstyle_invariant(runtime_object);
                }
                if (normalized_property_name == "fillstyle")
                {
                    normalize_native_visual_fillstyle_invariant(runtime_object);
                }
                if (normalized_property_name == "fillcolor")
                {
                    normalize_native_visual_fillcolor_invariant(runtime_object);
                }
                if (normalized_property_name == "borderwidth")
                {
                    normalize_native_visual_borderwidth_invariant(runtime_object);
                }
                if (normalized_property_name == "bordercolor")
                {
                    normalize_native_visual_bordercolor_invariant(runtime_object);
                }
                if (normalized_property_name == "borderstyle")
                {
                    normalize_native_visual_borderstyle_invariant(runtime_object);
                }
                if (normalized_property_name == "drawwidth")
                {
                    normalize_native_form_drawwidth_invariant(runtime_object);
                }
                if (normalized_property_name == "drawmode")
                {
                    normalize_native_visual_drawmode_invariant(runtime_object);
                }
                if (normalized_property_name == "backstyle")
                {
                    normalize_native_visual_backstyle_invariant(runtime_object);
                }
                if (normalized_property_name == "rowheight")
                {
                    normalize_native_grid_rowheight_invariant(runtime_object);
                }
                if (normalized_property_name == "headerheight")
                {
                    normalize_native_grid_headerheight_invariant(runtime_object);
                }
                if (normalized_property_name == "allowheadersizing")
                {
                    normalize_native_grid_allowheadersizing_invariant(runtime_object);
                }
                if (normalized_property_name == "allowrowsizing")
                {
                    normalize_native_grid_allowrowsizing_invariant(runtime_object);
                }
                if (normalized_property_name == "allowautocolumnfit")
                {
                    normalize_native_grid_allowautocolumnfit_invariant(runtime_object);
                }
                if (normalized_property_name == "gridlinecolor")
                {
                    normalize_native_grid_gridlinecolor_invariant(runtime_object);
                }
                if (normalized_property_name == "gridlinewidth")
                {
                    normalize_native_grid_gridlinewidth_invariant(runtime_object);
                }
                if (normalized_property_name == "highlightstyle")
                {
                    normalize_native_grid_highlightstyle_invariant(runtime_object);
                }
                if (normalized_property_name == "highlightrowlinewidth")
                {
                    normalize_native_grid_highlightrowlinewidth_invariant(runtime_object);
                }
                if (normalized_property_name == "view")
                {
                    normalize_native_grid_view_invariant(runtime_object);
                }
                if (normalized_property_name == "scrollbars")
                {
                    normalize_native_editbox_scrollbars_invariant(runtime_object);
                }
                if (normalized_property_name == "inputmask")
                {
                    normalize_native_textbox_inputmask_invariant(runtime_object);
                }
                if (normalized_property_name == "dynamicinputmask")
                {
                    normalize_native_textbox_dynamicinputmask_invariant(runtime_object);
                }
                if (normalized_property_name == "format")
                {
                    normalize_native_textbox_format_invariant(runtime_object);
                }
                if (normalized_property_name == "passwordchar")
                {
                    normalize_native_textbox_passwordchar_invariant(runtime_object);
                }
                if (normalized_property_name == "maxlength")
                {
                    normalize_native_textbox_maxlength_invariant(runtime_object);
                }
                if (normalized_property_name == "specialeffect")
                {
                    normalize_native_textbox_specialeffect_invariant(runtime_object);
                }
                if (normalized_property_name == "specialeffect")
                {
                    normalize_native_visual_specialeffect_invariant(runtime_object);
                }
                if (normalized_property_name == "borderstyle")
                {
                    normalize_native_textbox_borderstyle_invariant(runtime_object);
                }
                if (normalized_property_name == "hideselection")
                {
                    normalize_native_textbox_hideselection_invariant(runtime_object);
                }
                if (normalized_property_name == "autocomplete")
                {
                    normalize_native_textbox_autocomplete_invariant(runtime_object);
                }
                if (normalized_property_name == "enablehyperlinks")
                {
                    normalize_native_textbox_enablehyperlinks_invariant(runtime_object);
                }
                if (normalized_property_name == "tooltiptext")
                {
                    normalize_native_textbox_tooltiptext_invariant(runtime_object);
                }
                if (normalized_property_name == "margin")
                {
                    normalize_native_textbox_margin_invariant(runtime_object);
                }
                if (normalized_property_name == "mouseicon")
                {
                    normalize_native_textbox_mouseicon_invariant(runtime_object);
                }
                if (normalized_property_name == "disabledbackcolor")
                {
                    normalize_native_textbox_disabledbackcolor_invariant(runtime_object);
                }
                if (normalized_property_name == "disabledforecolor")
                {
                    normalize_native_textbox_disabledforecolor_invariant(runtime_object);
                }
                if (normalized_property_name == "disableditembackcolor")
                {
                    normalize_native_list_control_disableditembackcolor_invariant(runtime_object);
                }
                if (normalized_property_name == "disableditemforecolor")
                {
                    normalize_native_list_control_disableditemforecolor_invariant(runtime_object);
                }
                if (normalized_property_name == "itembackcolor")
                {
                    normalize_native_list_control_itembackcolor_invariant(runtime_object);
                }
                if (normalized_property_name == "itemforecolor")
                {
                    normalize_native_list_control_itemforecolor_invariant(runtime_object);
                }
                if (normalized_property_name == "selecteditembackcolor")
                {
                    normalize_native_list_control_selecteditembackcolor_invariant(runtime_object);
                }
                if (normalized_property_name == "selecteditemforecolor")
                {
                    normalize_native_list_control_selecteditemforecolor_invariant(runtime_object);
                }
                if (normalized_property_name == "statusbartext")
                {
                    normalize_native_textbox_statusbartext_invariant(runtime_object);
                }
                if (normalized_property_name == "helpcontextid")
                {
                    normalize_native_visual_helpcontextid_invariant(runtime_object);
                }
                if (normalized_property_name == "whatsthishelpid")
                {
                    normalize_native_visual_whatsthishelpid_invariant(runtime_object);
                }
                if (normalized_property_name == "whatsthishelp")
                {
                    normalize_native_form_whatsthishelp_invariant(runtime_object);
                }
                if (normalized_property_name == "default" ||
                    normalized_property_name == "cancel")
                {
                    normalize_native_commandbutton_default_cancel_invariant(runtime_object);
                }
                if (normalized_property_name == "style")
                {
                    normalize_native_commandbutton_style_invariant(runtime_object);
                }
                if (normalized_property_name == "picturemargin" ||
                    normalized_property_name == "pictureposition" ||
                    normalized_property_name == "picturespacing")
                {
                    normalize_native_commandbutton_picture_layout_invariant(runtime_object);
                }
                if (normalized_property_name == "strictdateentry")
                {
                    normalize_native_textbox_strictdateentry_invariant(runtime_object);
                }
                if (normalized_property_name == "themes")
                {
                    normalize_native_textbox_themes_invariant(runtime_object);
                }
                if (normalized_property_name == "selectedbackcolor")
                {
                    normalize_native_textbox_selectedbackcolor_invariant(runtime_object);
                }
                if (normalized_property_name == "selectedforecolor")
                {
                    normalize_native_textbox_selectedforecolor_invariant(runtime_object);
                }
                if (normalized_property_name == "dateformat")
                {
                    normalize_native_textbox_dateformat_invariant(runtime_object);
                }
                if (normalized_property_name == "century")
                {
                    normalize_native_textbox_century_invariant(runtime_object);
                }
                if (normalized_property_name == "datemark")
                {
                    normalize_native_textbox_datemark_invariant(runtime_object);
                }
                if (normalized_property_name == "hours")
                {
                    normalize_native_textbox_hours_invariant(runtime_object);
                }
                if (normalized_property_name == "seconds")
                {
                    normalize_native_textbox_seconds_invariant(runtime_object);
                }
                if (normalized_property_name == "value")
                {
                    normalize_native_textbox_selection_invariant(runtime_object);
                    normalize_native_textbox_text_invariant(runtime_object);
                }
                if (normalized_property_name == "firstelement" ||
                    normalized_property_name == "numberofelements")
                {
                    normalize_native_list_control_array_range_invariants(runtime_object);
                }
                if (normalized_property_name == "displaycount")
                {
                    normalize_native_combobox_displaycount_invariant(runtime_object);
                }
                if (normalized_property_name == "nulldisplay")
                {
                    normalize_native_list_control_nulldisplay_invariant(runtime_object);
                }
                if (normalized_property_name == "columnlines")
                {
                    normalize_native_list_control_columnlines_invariant(runtime_object);
                }
                if (normalized_property_name == "columnwidths")
                {
                    normalize_native_list_control_columnwidths_invariant(runtime_object);
                }
                if (normalized_property_name == "itemtips")
                {
                    normalize_native_list_control_itemtips_invariant(runtime_object);
                }
                if (normalized_property_name == "incrementalsearch")
                {
                    normalize_native_list_control_incrementalsearch_invariant(runtime_object);
                }
                if (normalized_property_name == "selectonentry" &&
                    runtime_object.properties.contains("selectonentry"))
                {
                    runtime_object.properties["selectonentry"] =
                        make_boolean_value(value_as_bool(runtime_object.properties["selectonentry"]));
                }
                if (normalized_property_name == "resizable" &&
                    is_native_resizable_member_name(runtime_object, normalized_property_name) &&
                    runtime_object.properties.contains("resizable"))
                {
                    runtime_object.properties["resizable"] =
                        make_boolean_value(value_as_bool(runtime_object.properties["resizable"]));
                }
                if (normalized_property_name == "anchor" &&
                    is_native_visual_anchor_member_name(runtime_object, normalized_property_name) &&
                    runtime_object.properties.contains("anchor"))
                {
                    normalize_native_visual_anchor_invariant(runtime_object);
                }
                if (normalized_property_name == "righttoleft" &&
                    is_native_visual_righttoleft_member_name(runtime_object, normalized_property_name) &&
                    runtime_object.properties.contains("righttoleft"))
                {
                    normalize_native_visual_righttoleft_invariant(runtime_object);
                }
                if (normalized_property_name == "wordwrap" &&
                    is_native_visual_wordwrap_member_name(runtime_object, normalized_property_name) &&
                    runtime_object.properties.contains("wordwrap"))
                {
                    normalize_native_visual_wordwrap_invariant(runtime_object);
                }
                if (normalized_property_name == "sorted")
                {
                    if (!write_native_list_control_controlsource_target(runtime_object, source_frame))
                    {
                        return false;
                    }
                }
                if (normalized_property_name == "boundcolumn" ||
                    normalized_property_name == "boundto")
                {
                    sync_native_list_control_displayvalue_from_selection(runtime_object);
                    if (!write_native_list_control_controlsource_target(runtime_object, source_frame))
                    {
                        return false;
                    }
                }
                if (normalized_property_name == "listindex")
                {
                    sync_native_list_control_displayvalue_from_selection(runtime_object);
                    if (!write_native_list_control_controlsource_target(runtime_object, source_frame))
                    {
                        return false;
                    }
                }
                remember_property_expression();
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
            ActiveNativeEventKeyGuard active_event_guard(active_native_event_keys, active_event_key);
            const auto invoke_delegates_for_phase = [&](bool after_source_member)
            {
                for (const NativeEventBinding &binding : bindings)
                {
                    const bool binding_after_source_member = (binding.flags & 1) == 0;
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
            if (result) {
                invoke_native_list_control_programmatic_change_if_needed(
                    runtime_object,
                    source_frame,
                    before_list_control_signature);
            }
            return result;
        }

        const bool result = perform_property_write();
        if (result) {
            invoke_native_list_control_programmatic_change_if_needed(
                runtime_object,
                source_frame,
                before_list_control_signature);
        }
        return result;
    }

    void PrgRuntimeSession::Impl::discard_native_object_tree_without_destroy(
        RuntimeOleObjectState &runtime_object)
    {
        std::vector<int> pending_handles = {runtime_object.handle};
        std::set<int> discarded_handles;
        std::set<std::intptr_t> discarded_native_hwnds;
        while (!pending_handles.empty())
        {
            const int handle = pending_handles.back();
            pending_handles.pop_back();
            if (!discarded_handles.insert(handle).second)
            {
                continue;
            }

            const auto found = ole_objects.find(handle);
            if (found == ole_objects.end())
            {
                continue;
            }
            if (found->second.native_hwnd.has_value())
            {
                discarded_native_hwnds.insert(*found->second.native_hwnd);
            }
            for (const int child_handle : collect_native_owned_child_handles(found->second))
            {
                pending_handles.push_back(child_handle);
            }
        }

        native_event_bindings.erase(
            std::remove_if(
                native_event_bindings.begin(),
                native_event_bindings.end(),
                [&discarded_handles](const NativeEventBinding &binding)
                {
                    return discarded_handles.contains(binding.source_handle) ||
                           (!binding.target_is_routine && discarded_handles.contains(binding.target_handle));
                }),
            native_event_bindings.end());
        window_message_bindings.erase(
            std::remove_if(
                window_message_bindings.begin(),
                window_message_bindings.end(),
                [&discarded_handles, &discarded_native_hwnds](const WindowMessageBinding &binding)
                {
                    return discarded_handles.contains(binding.target_handle) ||
                           discarded_native_hwnds.contains(binding.window_handle);
                }),
            window_message_bindings.end());

        for (const int handle : discarded_handles)
        {
            native_property_expression_text_by_handle.erase(handle);
            native_default_property_expression_text_by_handle.erase(handle);
            native_object_arrays.erase(handle);
            native_object_class_lineage_by_handle.erase(handle);
            ole_objects.erase(handle);
        }
        if (representative_active_form_handle.has_value() &&
            discarded_handles.contains(*representative_active_form_handle))
        {
            representative_active_form_handle.reset();
        }
        if (representative_application_forms_collection_handle.has_value() &&
            discarded_handles.contains(*representative_application_forms_collection_handle))
        {
            representative_application_forms_collection_handle.reset();
        }
        else if (representative_application_forms_collection_handle.has_value())
        {
            (void)ensure_representative_application_forms_collection_object();
        }
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

            RuntimeOleObjectState &lifecycle_object = found->second;
            const std::string normalized_base_class =
                normalize_identifier(trim_copy(lifecycle_object.base_class_name));
            if (normalized_base_class == "form" || normalized_base_class == "formset")
            {
                std::string unload_program_path;
                std::string unload_method_name;
                if (const Routine *unload_method = find_native_object_method(
                        lifecycle_object,
                        "unload",
                        unload_program_path,
                        unload_method_name);
                    unload_method != nullptr)
                {
                    if (!can_push_frame())
                    {
                        throw std::runtime_error(call_depth_limit_message());
                    }

                    events.push_back({.category = "prg.object.unload",
                                      .detail = unload_method_name,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    const std::size_t return_depth = stack.size();
                    const PrgValue this_reference =
                        make_string_value("object:" + lifecycle_object.prog_id + "#" + std::to_string(lifecycle_object.handle));
                    push_method_frame(unload_program_path,
                                      unload_method_name,
                                      *unload_method,
                                      this_reference,
                                      unload_method_name.substr(0U, unload_method_name.rfind('.')),
                                      "unload",
                                      native_object_parent_reference(lifecycle_object),
                                      native_object_owner_form_reference(lifecycle_object),
                                      native_object_owner_formset_reference(lifecycle_object),
                                      {},
                                      {});
                    (void)run_expression_invoked_routine_until_return(return_depth);

                    found = ole_objects.find(handle);
                    if (found == ole_objects.end())
                    {
                        continue;
                    }
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
            native_property_expression_text_by_handle.erase(handle);
            native_default_property_expression_text_by_handle.erase(handle);
            native_object_arrays.erase(handle);
            native_object_class_lineage_by_handle.erase(handle);
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

        std::string base_method_name;
        std::string base_defining_class_name;
        const auto base_method =
            find_native_object_class_method_lookup(
                **runtime_object,
                source_frame.native_method_name,
                source_frame.file_path,
                source_frame.native_method_class_name,
                false,
                base_method_name,
                &base_defining_class_name);
        if (!base_method.has_value())
        {
            const bool is_report_listener_object = std::any_of(
                (*runtime_object)->class_hierarchy.begin(),
                (*runtime_object)->class_hierarchy.end(),
                [](const std::string &class_name)
                {
                    return normalize_identifier(trim_copy(class_name)) == "reportlistener";
                }) ||
                normalize_identifier(trim_copy((*runtime_object)->base_class_name)) == "reportlistener";
            if (is_report_listener_object &&
                normalize_identifier(source_frame.native_method_name) == "init")
            {
                (*runtime_object)->last_action = "dodefault:ReportListener.Init";
                ++(*runtime_object)->action_count;
                events.push_back({.category = "prg.object.baseinvoke",
                                  .detail = "ReportListener.Init",
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                return make_boolean_value(true);
            }
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
        bool expression_error_was_handled = false;
        while (true)
        {
            while (stack.size() > return_depth &&
                   !stack.back().expression_routine_return_pending &&
                   (stack.back().routine == nullptr || stack.back().pc >= stack.back().routine->statements.size()))
            {
                pop_frame();
            }

            if (error_handler_return_depth.has_value() && stack.size() <= *error_handler_return_depth)
            {
                error_handler_return_depth.reset();
                handling_error = false;
                expression_error_was_handled = true;
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

            if (stack.size() < return_depth)
            {
                waiting_for_events = false;
                throw std::runtime_error(
                    runtime_text("Runtime.Prg.Expression.Error.UserRoutineAbortedExecution"));
            }

            if (stack.size() == return_depth)
            {
                return expression_error_was_handled ? make_empty_value() : last_return_value.value_or(make_empty_value());
            }

            const Statement *next = current_statement();
            if (next == nullptr)
            {
                pop_frame();
                continue;
            }

            if (!stack.back().expression_routine_return_pending &&
                executed_statement_count >= max_executed_statements)
            {
                last_error_message = step_budget_limit_message();
                last_fault_location = next->location;
                last_fault_statement = next->text;
                events.push_back({.category = "runtime.error",
                                  .detail = last_error_message,
                                  .location = next->location});
                throw PrgPropagatedRuntimeError(last_error_message);
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
                        // Never walk past this call's own return_depth: frames below
                        // it belong to the caller that is suspended while this
                        // expression-invoked routine runs, and must not be touched.
                        if (depth_at_fault - 1U - index < return_depth)
                        {
                            break;
                        }
                        Frame &parent = stack[depth_at_fault - 1U - index];
                        const bool has_open_try = std::any_of(
                            parent.tries.begin(),
                            parent.tries.end(),
                            [](const TryState &state)
                            {
                                return try_state_can_process_fault(state);
                            });
                        if (has_open_try)
                        {
                            while (stack.size() > depth_at_fault - index)
                            {
                                pop_frame();
                            }
                            if (!stack.empty())
                            {
                                stack.back().expression_routine_return_pending = false;
                                stack.back().expression_continuation.reset();
                                stack.back().command_target_continuation.reset();
                                stack.back().command_array_name_continuation.reset();
                                stack.back().command_argument_continuation.reset();
                                stack.back().text_merge_continuation.reset();
                                stack.back().parameter_default_continuation.reset();
                                stack.back().use_command_continuation.reset();
                                stack.back().copy_file_continuation.reset();
                                stack.back().rename_file_continuation.reset();
                                stack.back().loop_expression_continuation.reset();
                                stack.back().scan_expression_continuation.reset();
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
                throw PrgPropagatedRuntimeError(outcome.message);
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

    bool PrgRuntimeSession::Impl::handle_async_runtime_cancellation(
        SourceLocation location,
        std::string statement_text,
        std::string message,
        bool emit_task_cancelled_event)
    {
        if (location.file_path.empty())
        {
            if (const Statement *statement = current_statement(); statement != nullptr)
            {
                location = statement->location;
                if (statement_text.empty())
                {
                    statement_text = statement->text;
                }
            }
        }
        if (last_fault_location.file_path.empty())
        {
            last_fault_location = location;
        }
        if (last_fault_statement.empty())
        {
            last_fault_statement = statement_text;
        }

        last_error_message = std::move(message);
        int &level = current_transaction_level();
        if (level > 0)
        {
            if (!rollback_active_transaction_journal())
            {
                return false;
            }
            level = 0;
            events.push_back({.category = "runtime.transaction.rollback",
                              .detail = "0",
                              .location = location});
        }
        rollback_active_command_undo_journal();
        if (emit_task_cancelled_event)
        {
            events.push_back({.category = "runtime.task.cancelled",
                              .detail = "cancelled",
                              .location = location});
        }
        return true;
    }

    std::optional<std::string> PrgRuntimeSession::Impl::materialize_xasset_bootstrap(
        const std::string &asset_path,
        bool include_read_events)
    {
        std::filesystem::path snapshot_root;
        const auto snapshot_path = materialize_verified_xasset_snapshot(
            copperfin::platform::path_from_utf8_string(asset_path),
            snapshot_root);
        if (!snapshot_path.has_value())
        {
            return std::nullopt;
        }

        studio::StudioOpenRequest request;
        request.path = copperfin::platform::path_to_utf8_string(*snapshot_path);
        request.read_only = true;
        request.load_full_table = true;
        const auto open_result = studio::open_document(request);
        std::error_code ignored;
        if (!snapshot_root.empty())
        {
            std::filesystem::remove_all(snapshot_root, ignored);
        }
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

        const std::filesystem::path asset_file = copperfin::platform::path_from_utf8_string(asset_path);
        const std::filesystem::path bootstrap_path = make_prg_engine_xasset_bootstrap_path(
            runtime_temp_directory,
            asset_file,
            runtime_instance_id);

        const std::string bootstrap_source =
            build_xasset_bootstrap_source(model, include_read_events, asset_path, true);
        struct ScopedXAssetBootstrapFileCleanup
        {
            std::filesystem::path path;
            bool preserve = false;

            ~ScopedXAssetBootstrapFileCleanup()
            {
                if (preserve)
                {
                    return;
                }
                std::error_code ignored;
                std::filesystem::remove(path, ignored);
            }
        } bootstrap_file_cleanup{bootstrap_path};

        std::ofstream output(bootstrap_path, std::ios::binary | std::ios::trunc);
        output << bootstrap_source;
        if (prg_xasset_bootstrap_write_failure_requested(bootstrap_path))
        {
            output.setstate(std::ios::badbit);
        }
        output.close();
        if (!output.good())
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.XAssetBootstrapMaterializeFailed",
                {{"path", asset_path}});
            return std::nullopt;
        }

        options.source_text_overrides[copperfin::runtime::normalize_path(
            copperfin::platform::path_to_utf8_string(bootstrap_path))] = bootstrap_source;
        owned_xasset_bootstrap_paths.push_back(bootstrap_path);
        bootstrap_file_cleanup.preserve = true;

        return copperfin::platform::path_to_utf8_string(bootstrap_path);
    }

    std::optional<std::string> PrgRuntimeSession::Impl::materialize_vcx_class_source(
        const Frame &frame,
        const std::string &class_name,
        const std::string &library_path,
        std::string &resolved_library_path)
    {
        const std::string trimmed_class_name = trim_copy(class_name);
        const std::string trimmed_library_path = trim_copy(library_path);
        if (!is_bare_identifier_text(trimmed_class_name) || trimmed_library_path.empty())
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxClassNotFound",
                {{"className", trimmed_class_name}, {"classLibraryPath", trimmed_library_path}});
            return std::nullopt;
        }

        const std::string resolved_path =
            resolve_native_prg_program_path(trimmed_library_path, frame.file_path);
        const std::filesystem::path library_file =
            copperfin::platform::path_from_utf8_string(resolved_path);
        std::error_code filesystem_error;
        const auto verified_library = find_verified_file_byte_override(library_file);
        const bool has_verified_library =
            verified_library != options.verified_file_byte_overrides.end() &&
            !verified_library->second.empty();
        if (!std::filesystem::is_regular_file(library_file, filesystem_error) &&
            !(options.require_verified_file_byte_overrides && has_verified_library))
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path},
                 {"errorMessage", filesystem_error
                                      ? filesystem_error.message()
                                      : runtime_text("Runtime.Prg.Core.Detail.FileNotFound")}});
            return std::nullopt;
        }

        std::filesystem::path snapshot_root;
        const auto open_library_path = materialize_verified_xasset_snapshot(
            library_file,
            snapshot_root);
        if (!open_library_path.has_value())
        {
            const std::string verified_error = last_error_message;
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path},
                 {"errorMessage", verified_error}});
            return std::nullopt;
        }

        const auto open_result = studio::open_document({
            .path = copperfin::platform::path_to_utf8_string(*open_library_path),
            .read_only = true,
            .load_full_table = true
        });
        if (!snapshot_root.empty())
        {
            std::error_code snapshot_error;
            std::filesystem::remove_all(snapshot_root, snapshot_error);
        }
        if (!open_result.ok)
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path}, {"errorMessage", open_result.error}});
            return std::nullopt;
        }

        const XAssetExecutableModel model = build_xasset_executable_model(open_result.document);
        if (!model.ok)
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path}, {"errorMessage", model.error}});
            return std::nullopt;
        }

        const auto objects = studio::build_object_snapshot(open_result.document);
        const std::string normalized_requested_class = normalize_identifier(trimmed_class_name);
        const studio::StudioObjectSnapshot *root_object = nullptr;
        for (const auto &object : objects)
        {
            if (!object.parent_name.empty())
            {
                continue;
            }
            const std::string normalized_object_name = normalize_identifier(trim_copy(object.object_name));
            const std::string normalized_class_field = normalize_identifier(trim_copy(object.class_name));
            if (normalized_object_name == normalized_requested_class ||
                normalized_class_field == normalized_requested_class ||
                normalize_identifier(trim_copy(object.object_path)) == normalized_requested_class)
            {
                root_object = &object;
                break;
            }
        }
        if (root_object == nullptr)
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxClassNotFound",
                {{"className", trimmed_class_name}, {"classLibraryPath", trimmed_library_path}});
            return std::nullopt;
        }

        const std::string base_class_name = trim_copy(root_object->baseclass_name).empty()
            ? "Custom"
            : trim_copy(root_object->baseclass_name);
        if (!is_bare_identifier_text(base_class_name))
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path},
                 {"errorMessage", runtime_text("Runtime.Prg.Core.Detail.InvalidRootBaseClass")}});
            return std::nullopt;
        }

        std::ostringstream source;
        const std::filesystem::path include_root = options.require_verified_file_byte_overrides
            ? library_file.parent_path()
            : open_library_path->parent_path();
        const std::filesystem::path companion_header_candidate =
            include_root / library_file.stem();
        auto companion_header = companion_header_candidate;
        companion_header.replace_extension(".h");
        std::optional<std::filesystem::path> resolved_companion_header;
        bool companion_header_ambiguous = false;
        if (options.require_verified_file_byte_overrides)
        {
            const auto requested_header = companion_header.lexically_normal();
            const std::string requested_parent = lowercase_copy(normalize_path(
                copperfin::platform::path_to_utf8_string(requested_header.parent_path())));
            const std::string requested_filename = lowercase_copy(
                copperfin::platform::path_to_utf8_string(requested_header.filename()));
            for (const auto &[candidate_name, bytes] : options.verified_file_byte_overrides)
            {
                if (bytes.empty())
                {
                    continue;
                }
                const auto candidate_path = copperfin::platform::path_from_utf8_string(candidate_name).lexically_normal();
                if (lowercase_copy(normalize_path(
                        copperfin::platform::path_to_utf8_string(candidate_path.parent_path()))) != requested_parent ||
                    lowercase_copy(copperfin::platform::path_to_utf8_string(candidate_path.filename())) != requested_filename ||
                    lowercase_copy(copperfin::platform::path_to_utf8_string(candidate_path.extension())) != ".h")
                {
                    continue;
                }
                if (resolved_companion_header.has_value())
                {
                    companion_header_ambiguous = true;
                    resolved_companion_header.reset();
                    break;
                }
                resolved_companion_header = candidate_path;
            }
        }
        else
        {
            const auto companion_header_resolution = copperfin::vfp::resolve_unique_casefold_path(
                companion_header);
            companion_header_ambiguous = companion_header_resolution.ambiguous;
            if (companion_header_resolution.path.has_value())
            {
                resolved_companion_header = companion_header_resolution.path;
            }
        }

        source << "* Copperfin generated VCX class bridge\n";
        if (!companion_header_ambiguous && resolved_companion_header.has_value())
        {
            source << "#include \""
                   << copperfin::platform::path_to_utf8_string(
                          resolved_companion_header->lexically_normal())
                   << "\"\n";
        }
        source << "DEFINE CLASS " << trimmed_class_name << " AS " << base_class_name << "\n";
        for (const auto &property : root_object->properties)
        {
            if (!property.derived_from_property_blob ||
                !is_bare_identifier_text(property.name) ||
                trim_copy(property.value).empty() ||
                property.value.find('\n') != std::string::npos ||
                property.value.find('\r') != std::string::npos)
            {
                continue;
            }
            source << "    " << property.name << " = " << property.value << "\n";
        }

        const std::string root_path = normalize_identifier(trim_copy(root_object->object_path));
        std::set<std::string> emitted_methods;
        for (const auto &method : model.methods)
        {
            if (normalize_identifier(trim_copy(method.object_path)) != root_path ||
                !is_bare_identifier_text(method.method_name) ||
                trim_copy(method.source_text).empty() ||
                !emitted_methods.insert(normalize_identifier(method.method_name)).second)
            {
                continue;
            }
            source << "    PROCEDURE " << method.method_name << "\n"
                   << method.source_text;
            if (method.source_text.back() != '\n')
            {
                source << '\n';
            }
            source << "    ENDPROC\n";
        }
        source << "ENDDEFINE\n";

        std::error_code ignored;
        std::filesystem::create_directories(runtime_temp_directory, ignored);
        const std::string cache_key = resolved_path + ":" + normalized_requested_class;
        const std::filesystem::path generated_path =
            runtime_temp_directory /
            ("vcx_class_" + std::to_string(std::hash<std::string>{}(cache_key)) + ".prg");
        const std::string generated_source = source.str();
        std::ofstream output(generated_path, std::ios::binary | std::ios::trunc);
        output << generated_source;
        output.close();
        if (!output.good())
        {
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.NewObjectVcxOpenFailed",
                {{"classLibraryPath", trimmed_library_path},
                 {"errorMessage", runtime_text("Runtime.Prg.Core.Detail.GeneratedClassSourceWriteFailed")}});
            return std::nullopt;
        }

        const std::string generated_path_text =
            copperfin::platform::path_to_utf8_string(generated_path);
        options.source_text_overrides[normalize_path(generated_path_text)] = generated_source;
        resolved_library_path = resolved_path;
        return generated_path_text;
    }

#include "prg_engine_dispatch.inl"
    bool PrgRuntimeSession::Impl::dispatch_event_handler(const std::string &routine_name)
    {
        if (!waiting_for_events || stack.empty())
        {
            return false;
        }

        const std::string normalized_target = normalize_identifier(routine_name);
        if (const auto found = find_event_handler_routine_lookup(normalized_target); found.has_value())
        {
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
            push_routine_frame(found->program->path, *found->routine);
            events.push_back({.category = "runtime.dispatch",
                              .detail = found->routine->name,
                              .location = {}});
            return true;
        }

        return false;
    }

    bool PrgRuntimeSession::Impl::dispatch_popup_bar_selection(
        const std::string &popup_name,
        std::int64_t bar_number)
    {
        if (!waiting_for_events || stack.empty() || bar_number < 1)
        {
            return false;
        }

        const std::string normalized_popup_name = normalize_identifier(popup_name);
        if (normalized_popup_name.empty())
        {
            return false;
        }

        const auto popup = current_session_state().popup_bar_prompts.find(normalized_popup_name);
        if (popup == current_session_state().popup_bar_prompts.end())
        {
            return false;
        }

        const auto bar = popup->second.find(static_cast<long long>(bar_number));
        if (bar == popup->second.end() || bar->second.rfind("\\-", 0U) == 0U)
        {
            return false;
        }

        const auto skip_popup = current_session_state().popup_bar_skip_states.find(normalized_popup_name);
        if (skip_popup != current_session_state().popup_bar_skip_states.end())
        {
            const auto skip_bar = skip_popup->second.find(static_cast<long long>(bar_number));
            if (skip_bar != skip_popup->second.end() && skip_bar->second)
            {
                return false;
            }
        }

        std::string handler_name;
        std::string action_text;
        const auto handler_popup = current_session_state().popup_bar_selection_handlers.find(
            normalized_popup_name);
        if (handler_popup != current_session_state().popup_bar_selection_handlers.end())
        {
            const auto handler = handler_popup->second.find(static_cast<long long>(bar_number));
            if (handler != handler_popup->second.end())
            {
                if (handler->second.empty() ||
                    !find_event_handler_routine_lookup(handler->second).has_value())
                {
                    return false;
                }
                handler_name = handler->second;
            }
        }

        if (handler_name.empty())
        {
            const auto action_popup = current_session_state().popup_bar_selection_actions.find(
                normalized_popup_name);
            if (action_popup != current_session_state().popup_bar_selection_actions.end())
            {
                const auto action = action_popup->second.find(static_cast<long long>(bar_number));
                if (action != action_popup->second.end())
                {
                    if (action->second.empty() || action->second.find('&') != std::string::npos)
                    {
                        return false;
                    }
                    action_text = action->second;
                }
            }
        }

        if (handler_name.empty() && action_text.empty())
        {
            const auto popup_handler = current_session_state().popup_selection_handlers.find(
                normalized_popup_name);
            if (popup_handler != current_session_state().popup_selection_handlers.end())
            {
                if (popup_handler->second.empty() ||
                    !find_event_handler_routine_lookup(popup_handler->second).has_value())
                {
                    return false;
                }
                handler_name = popup_handler->second;
            }
        }

        if (handler_name.empty() && !action_text.empty())
        {
            if (!can_push_frame())
            {
                waiting_for_events = true;
                last_error_message = call_depth_limit_message();
                events.push_back({.category = "runtime.error",
                                  .detail = last_error_message,
                                  .location = {}});
                return false;
            }

            Program &source_program = load_program(stack.back().file_path);
            const long long normalized_bar_number = static_cast<long long>(bar_number);
            auto &cached_action = current_session_state().popup_bar_action_routines[
                normalized_popup_name][normalized_bar_number];
            const Routine *action_routine = nullptr;
            if (cached_action.action_text == action_text &&
                cached_action.source_path == source_program.path)
            {
                const auto cached_routine = source_program.routines.find(
                    normalize_identifier(cached_action.routine_name));
                if (cached_routine != source_program.routines.end())
                {
                    action_routine = &cached_routine->second;
                }
            }

            if (action_routine == nullptr)
            {
                const std::string action_routine_name =
                    "__copperfin_popup_action_" + std::to_string(runtime_instance_id) + "_" +
                    std::to_string(++next_popup_action_id);
                const Program action_program = parse_program_source(
                    source_program.path,
                    "PROCEDURE " + action_routine_name + "\n" + action_text +
                        "\nRETURN\nENDPROC\n");
                const auto action_found = action_program.routines.find(
                    normalize_identifier(action_routine_name));
                if (action_found == action_program.routines.end())
                {
                    return false;
                }
                auto [inserted_routine, inserted] = source_program.routines.emplace(
                    normalize_identifier(action_routine_name),
                    action_found->second);
                if (!inserted)
                {
                    return false;
                }
                cached_action.action_text = action_text;
                cached_action.source_path = source_program.path;
                cached_action.routine_name = action_routine_name;
                action_routine = &inserted_routine->second;
            }
            waiting_for_events = false;
            event_dispatch_return_depth = stack.size();
            restore_event_loop_after_dispatch = true;
            events.push_back({.category = "runtime.popup.selection",
                              .detail = normalized_popup_name + " bar=" + std::to_string(bar_number),
                              .location = {}});
            push_routine_frame(source_program.path, *action_routine);
            return true;
        }

        if (handler_name.empty())
        {
            const auto activation_popup = current_session_state().popup_bar_activation_targets.find(
                normalized_popup_name);
            if (activation_popup != current_session_state().popup_bar_activation_targets.end())
            {
                const auto activation = activation_popup->second.find(static_cast<long long>(bar_number));
                if (activation != activation_popup->second.end() && !activation->second.empty() &&
                    current_session_state().popup_bar_prompts.find(activation->second) !=
                        current_session_state().popup_bar_prompts.end())
                {
                    waiting_for_events = true;
                    events.push_back({.category = "popup.activate",
                                      .detail = activation->second,
                                      .location = {}});
                    return true;
                }
            }
        }

        if (handler_name.empty())
        {
            return false;
        }

        events.push_back({.category = "runtime.popup.selection",
                          .detail = normalized_popup_name + " bar=" + std::to_string(bar_number),
                          .location = {}});
        return dispatch_event_handler(handler_name);
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

        std::optional<int> window_close_target;
        if (message == kCopperfinWindowCloseMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                const std::string normalized_base_class =
                    normalize_identifier(trim_copy(runtime_object.base_class_name));
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd &&
                    (normalized_base_class == "form" || normalized_base_class == "formset"))
                {
                    window_close_target = handle;
                    break;
                }
            }
        }

        const auto is_optiongroup_arrow_key = [](const std::intptr_t key_code)
        {
            return key_code == 37 || key_code == 38 || key_code == 39 || key_code == 40;
        };
        std::optional<int> keypress_target;
        if (message == kWindowsKeyDownMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd)
                {
                    keypress_target = handle;
                    break;
                }
            }

            if (keypress_target.has_value())
            {
                const auto target_found = ole_objects.find(*keypress_target);
                if (target_found != ole_objects.end() &&
                    (normalize_identifier(trim_copy(target_found->second.base_class_name)) == "form" ||
                     normalize_identifier(trim_copy(target_found->second.base_class_name)) == "formset"))
                {
                    const auto active_control = target_found->second.properties.find("activecontrol");
                    if (active_control != target_found->second.properties.end())
                    {
                        if (const auto resolved_active_control = resolve_ole_object(active_control->second);
                            resolved_active_control.has_value())
                        {
                            const std::string active_base_class = normalize_identifier(
                                trim_copy((*resolved_active_control)->base_class_name));
                            if (wparam == 9 || active_base_class == "commandbutton" ||
                                (is_optiongroup_arrow_key(wparam) &&
                                 (active_base_class == "optiongroup" || active_base_class == "optionbutton")))
                            {
                                keypress_target = (*resolved_active_control)->handle;
                                if (is_optiongroup_arrow_key(wparam) && active_base_class == "optionbutton")
                                {
                                    if (const auto parent_reference =
                                            native_object_parent_reference(**resolved_active_control);
                                        parent_reference.has_value())
                                    {
                                        if (auto parent = resolve_ole_object(*parent_reference);
                                            parent.has_value() &&
                                            normalize_identifier(trim_copy((*parent)->base_class_name)) == "optiongroup")
                                        {
                                            keypress_target = (*parent)->handle;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        std::optional<int> click_target;
        if (message == kWindowsLeftButtonUpMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd)
                {
                    click_target = handle;
                    break;
                }
            }
        }

        std::optional<int> mouse_down_target;
        if (message == kWindowsLeftButtonDownMessage ||
            message == kWindowsRightButtonDownMessage ||
            message == kWindowsMiddleButtonDownMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd)
                {
                    mouse_down_target = handle;
                    break;
                }
            }
        }

        std::optional<int> mouse_move_target;
        if (message == kWindowsMouseMoveMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd)
                {
                    mouse_move_target = handle;
                    break;
                }
            }
        }

        std::optional<int> double_click_target;
        if (message == kWindowsLeftButtonDoubleClickMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd)
                {
                    double_click_target = handle;
                    break;
                }
            }
        }

        std::optional<int> right_click_target;
        if (message == kWindowsRightButtonUpMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd)
                {
                    right_click_target = handle;
                    break;
                }
            }
        }

        std::optional<int> middle_click_target;
        if (message == kWindowsMiddleButtonUpMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd)
                {
                    middle_click_target = handle;
                    break;
                }
            }
        }

        std::optional<int> right_up_target;
        if (message == kWindowsRightButtonUpMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd)
                {
                    right_up_target = handle;
                    break;
                }
            }
        }

        std::optional<int> middle_up_target;
        if (message == kWindowsMiddleButtonUpMessage)
        {
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                if (runtime_object.native_hwnd.has_value() &&
                    *runtime_object.native_hwnd == hwnd)
                {
                    middle_up_target = handle;
                    break;
                }
            }
        }

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
        if (bindings.empty() && !window_close_target.has_value() &&
            !keypress_target.has_value() && !click_target.has_value() &&
            !mouse_down_target.has_value() && !mouse_move_target.has_value() &&
            !double_click_target.has_value() && !right_click_target.has_value() &&
            !right_up_target.has_value() &&
            !middle_click_target.has_value() && !middle_up_target.has_value())
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
            case PrgValueKind::currency:
                return static_cast<std::intptr_t>(std::llround(value_as_number(value)));
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

        const auto mouse_event_arguments = [&]()
        {
            const auto raw_coordinates = static_cast<std::uint64_t>(lparam);
            std::int64_t button = 1;
            if (message == kWindowsMouseMoveMessage)
            {
                button = ((wparam & 0x0001) != 0 ? 1 : 0) |
                         ((wparam & 0x0002) != 0 ? 2 : 0) |
                         ((wparam & 0x0010) != 0 ? 4 : 0);
            }
            else if (message == kWindowsMiddleButtonDownMessage ||
                     message == kWindowsMiddleButtonUpMessage)
            {
                button = 4;
            }
            else if (message == kWindowsRightButtonDownMessage ||
                     message == kWindowsRightButtonUpMessage)
            {
                button = 2;
            }
            const std::int64_t x_coordinate = static_cast<std::int16_t>(
                static_cast<std::uint16_t>(raw_coordinates & 0xffffU));
            const std::int64_t y_coordinate = static_cast<std::int16_t>(
                static_cast<std::uint16_t>((raw_coordinates >> 16U) & 0xffffU));
            const std::int64_t shift_alt_ctrl =
                ((wparam & 0x0004) != 0 ? 1 : 0) |
                ((wparam & 0x0008) != 0 ? 2 : 0);
            return std::vector<PrgValue>{
                make_int64_value(button),
                make_int64_value(shift_alt_ctrl),
                make_int64_value(x_coordinate),
                make_int64_value(y_coordinate)};
        };
        const std::vector<std::optional<std::string>> mouse_argument_references(
            4,
            std::nullopt);

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

        if (mouse_down_target.has_value())
        {
            const auto target_found = ole_objects.find(*mouse_down_target);
            if (target_found != ole_objects.end())
            {
                bool ignored_nodefault = false;
                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "MouseDown",
                        stack.back(),
                        mouse_event_arguments(),
                        mouse_argument_references,
                        &ignored_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.mousedown",
                                      .detail = target_found->second.prog_id,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = 0;
                }
            }
        }

        if (mouse_move_target.has_value())
        {
            const auto target_found = ole_objects.find(*mouse_move_target);
            if (target_found != ole_objects.end())
            {
                bool ignored_nodefault = false;
                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "MouseMove",
                        stack.back(),
                        mouse_event_arguments(),
                        mouse_argument_references,
                        &ignored_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.mousemove",
                                      .detail = target_found->second.prog_id,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = 0;
                }
            }
        }

        if (double_click_target.has_value())
        {
            const auto target_found = ole_objects.find(*double_click_target);
            if (target_found != ole_objects.end())
            {
                bool ignored_nodefault = false;
                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "DblClick",
                        stack.back(),
                        {},
                        {},
                        &ignored_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.dblclick",
                                      .detail = target_found->second.prog_id,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = 0;
                }
            }
        }

        if (right_up_target.has_value())
        {
            const auto target_found = ole_objects.find(*right_up_target);
            if (target_found != ole_objects.end())
            {
                bool ignored_nodefault = false;
                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "MouseUp",
                        stack.back(),
                        mouse_event_arguments(),
                        mouse_argument_references,
                        &ignored_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.mouseup",
                                      .detail = target_found->second.prog_id,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = 0;
                }
            }
        }

        if (right_click_target.has_value())
        {
            const auto target_found = ole_objects.find(*right_click_target);
            if (target_found != ole_objects.end())
            {
                bool ignored_nodefault = false;
                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "RightClick",
                        stack.back(),
                        {},
                        {},
                        &ignored_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.rightclick",
                                      .detail = target_found->second.prog_id,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = 0;
                }
            }
        }

        if (middle_up_target.has_value())
        {
            const auto target_found = ole_objects.find(*middle_up_target);
            if (target_found != ole_objects.end())
            {
                bool ignored_nodefault = false;
                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "MouseUp",
                        stack.back(),
                        mouse_event_arguments(),
                        mouse_argument_references,
                        &ignored_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.mouseup",
                                      .detail = target_found->second.prog_id,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = 0;
                }
            }
        }

        if (middle_click_target.has_value())
        {
            const auto target_found = ole_objects.find(*middle_click_target);
            if (target_found != ole_objects.end())
            {
                bool ignored_nodefault = false;
                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "MiddleClick",
                        stack.back(),
                        {},
                        {},
                        &ignored_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.middleclick",
                                      .detail = target_found->second.prog_id,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = 0;
                }
            }
        }

        if (keypress_target.has_value())
        {
            const auto target_found = ole_objects.find(*keypress_target);
            if (target_found != ole_objects.end())
            {
                bool keypress_suppressed_by_preview = false;
                if (normalize_identifier(trim_copy(target_found->second.base_class_name)) != "form")
                {
                    if (const auto owner_form_reference =
                            native_object_owner_form_reference(target_found->second);
                        owner_form_reference.has_value())
                    {
                        if (auto owner_form = resolve_ole_object(*owner_form_reference);
                            owner_form.has_value())
                        {
                            const auto key_preview = read_native_property_if_present(
                                **owner_form,
                                "keypreview",
                                stack.back());
                            if (key_preview.has_value() && value_as_bool(*key_preview))
                            {
                                bool preview_requested_nodefault = false;
                                if (invoke_native_object_method_if_present(
                                        **owner_form,
                                        "KeyPress",
                                        stack.back(),
                                        {make_int64_value(static_cast<std::int64_t>(wparam)),
                                         make_int64_value((lparam & kWindowsAltContextBit) != 0 ? 4 : 0)},
                                        {std::nullopt, std::nullopt},
                                        &preview_requested_nodefault,
                                        nullptr)
                                        .has_value())
                                {
                                    events.push_back({.category = "prg.event.keypress",
                                                      .detail = (*owner_form)->prog_id +
                                                                " key=" + std::to_string(wparam),
                                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                                    keypress_suppressed_by_preview = preview_requested_nodefault;
                                    if (keypress_suppressed_by_preview)
                                    {
                                        last_result = 1;
                                    }
                                }
                            }
                        }
                    }
                }

                if (keypress_suppressed_by_preview)
                {
                    return last_result;
                }

                const auto before_interactive_change_signature =
                    native_list_control_selection_signature(target_found->second);
                // WM_KEYDOWN carries Alt in bit 29. Shift and Ctrl state are
                // supplied by the platform adapter in a future host-specific
                // bridge because they are not encoded in this message.
                const std::int64_t shift_alt_ctrl =
                    (lparam & kWindowsAltContextBit) != 0 ? 4 : 0;
                bool requested_nodefault = false;
                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "KeyPress",
                        stack.back(),
                        {make_int64_value(static_cast<std::int64_t>(wparam)),
                         make_int64_value(shift_alt_ctrl)},
                        {std::nullopt, std::nullopt},
                        &requested_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.keypress",
                                      .detail = target_found->second.prog_id +
                                                    " key=" + std::to_string(wparam),
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = requested_nodefault ? 1 : 0;

                    const auto after_interactive_change_signature =
                        native_list_control_selection_signature(target_found->second);
                    if (before_interactive_change_signature.has_value() &&
                        after_interactive_change_signature.has_value() &&
                        *before_interactive_change_signature != *after_interactive_change_signature)
                    {
                        bool ignored_nodefault = false;
                        if (invoke_native_object_method_if_present(
                                target_found->second,
                                "InteractiveChange",
                                stack.back(),
                                {},
                                {},
                                &ignored_nodefault,
                                nullptr)
                                .has_value())
                        {
                            events.push_back({.category = "prg.event.interactivechange",
                                              .detail = target_found->second.prog_id,
                                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                        }
                    }
                }

                if (!requested_nodefault && wparam == 9)
                {
                    if (move_native_focus_to_next_tab_stop(target_found->second, stack.back()))
                    {
                        last_result = 0;
                    }
                }

                if (!requested_nodefault && is_optiongroup_arrow_key(wparam))
                {
                    const int direction = wparam == 37 || wparam == 38 ? -1 : 1;
                    if (move_native_optiongroup_selection(target_found->second, stack.back(), direction))
                    {
                        last_result = 0;
                    }
                }

                if (!requested_nodefault && (wparam == 13 || wparam == 27))
                {
                    const auto owner_form_reference = native_object_owner_form_reference(target_found->second);
                    int owner_form_handle = 0;
                    std::string owner_form_prog_id;
                    if (owner_form_reference.has_value() &&
                        parse_object_handle_reference(
                            *owner_form_reference,
                            owner_form_handle,
                            owner_form_prog_id))
                    {
                        const std::string property_name = wparam == 13 ? "default" : "cancel";
                        const bool focused_command_button =
                            wparam == 13 &&
                            normalize_identifier(trim_copy(target_found->second.base_class_name)) == "commandbutton";
                        if (focused_command_button)
                        {
                            const auto focused_enabled = read_native_property_if_present(
                                target_found->second,
                                "enabled",
                                stack.back());
                            if (focused_enabled.has_value() && value_as_bool(*focused_enabled))
                            {
                                bool ignored_nodefault = false;
                                if (invoke_native_object_method_if_present(
                                        target_found->second,
                                        "Click",
                                        stack.back(),
                                        {},
                                        {},
                                        &ignored_nodefault,
                                        nullptr)
                                        .has_value())
                                {
                                    events.push_back({.category = "prg.event.click",
                                                      .detail = target_found->second.prog_id,
                                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                                    last_result = 0;
                                }
                            }
                        }
                        else
                        {
                            for (auto &entry : ole_objects)
                            {
                                auto &candidate = entry.second;
                                if (normalize_identifier(trim_copy(candidate.base_class_name)) != "commandbutton")
                                {
                                    continue;
                                }

                                const auto candidate_owner_reference =
                                    native_object_owner_form_reference(candidate);
                                int candidate_owner_handle = 0;
                                std::string candidate_owner_prog_id;
                                if (!candidate_owner_reference.has_value() ||
                                    !parse_object_handle_reference(
                                        *candidate_owner_reference,
                                        candidate_owner_handle,
                                        candidate_owner_prog_id) ||
                                    candidate_owner_handle != owner_form_handle)
                                {
                                    continue;
                                }

                                const auto enabled = read_native_property_if_present(
                                    candidate,
                                    "enabled",
                                    stack.back());
                                const auto action_property = read_native_property_if_present(
                                    candidate,
                                    property_name,
                                    stack.back());
                                if (!enabled.has_value() || !value_as_bool(*enabled) ||
                                    !action_property.has_value() || !value_as_bool(*action_property))
                                {
                                    continue;
                                }

                                bool ignored_nodefault = false;
                                if (invoke_native_object_method_if_present(
                                        candidate,
                                        "Click",
                                        stack.back(),
                                        {},
                                        {},
                                        &ignored_nodefault,
                                        nullptr)
                                        .has_value())
                                {
                                    events.push_back({.category = "prg.event.click",
                                                      .detail = candidate.prog_id,
                                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                                    last_result = 0;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (click_target.has_value())
        {
            const auto target_found = ole_objects.find(*click_target);
            if (target_found != ole_objects.end())
            {
                bool ignored_nodefault = false;
                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "MouseUp",
                        stack.back(),
                        mouse_event_arguments(),
                        mouse_argument_references,
                        &ignored_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.mouseup",
                                      .detail = target_found->second.prog_id,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = 0;
                }

                if (invoke_native_object_method_if_present(
                        target_found->second,
                        "Click",
                        stack.back(),
                        {},
                        {},
                        &ignored_nodefault,
                        nullptr)
                        .has_value())
                {
                    events.push_back({.category = "prg.event.click",
                                      .detail = target_found->second.prog_id,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    last_result = 0;
                }
            }
        }

        if (window_close_target.has_value())
        {
            auto target_found = ole_objects.find(*window_close_target);
            if (target_found != ole_objects.end())
            {
                const SourceLocation close_location = current_statement() == nullptr
                    ? SourceLocation{}
                    : current_statement()->location;
                if (!dispatch_query_unload_for_objects(
                        collect_native_shutdown_order_for_window(*window_close_target),
                        close_location))
                {
                    events.push_back({.category = "prg.object.window_close_veto",
                                      .detail = target_found->second.prog_id,
                                      .location = close_location});
                    return static_cast<std::intptr_t>(0);
                }

                const std::string target_prog_id = target_found->second.prog_id;
                (void)release_native_object(target_found->second, "WM_CLOSE");
                events.push_back({.category = "prg.object.window_close",
                                  .detail = target_prog_id,
                                  .location = close_location});
                return static_cast<std::intptr_t>(0);
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

        if (const auto found = find_event_handler_routine_lookup(handler); found.has_value())
        {
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
            push_routine_frame(found->program->path, *found->routine, handler_arguments);
            events.push_back({.category = "runtime.error_handler",
                              .detail = handler_arguments.empty()
                                            ? found->routine->name
                                            : found->routine->name + " WITH " + std::to_string(handler_arguments.size()) + " argument(s)",
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
                const Statement *pending_statement = current_statement();
                const bool pending_statement_handles_cancellation =
                    pending_statement != nullptr && pending_statement->kind == StatementKind::sleep_command;
                if (task_cancel_requested != nullptr &&
                    task_cancel_requested->load(std::memory_order_relaxed) &&
                    !pending_statement_handles_cancellation)
                {
                    ensure_fault_context_defaults(pending_statement, last_fault_location, last_fault_statement);
                    if (!handle_async_runtime_cancellation(
                            last_fault_location,
                            last_fault_statement,
                            runtime_text("Runtime.Prg.Core.Error.AsyncTaskCancelled")))
                    {
                        return finalize_pause_state(DebugPauseReason::error, last_error_message);
                    }
                    return finalize_pause_state(DebugPauseReason::error, last_error_message);
                }
                while (!stack.empty() &&
                       !stack.back().expression_routine_return_pending &&
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
                    if (std::any_of(
                            stack.begin(),
                            stack.end(),
                            [](const Frame &frame)
                            {
                                return frame.expression_routine_return_pending;
                            }))
                    {
                        // An ON ERROR handler's RETURN value is not the value of
                        // the expression that raised the fault.
                        last_return_value = make_empty_value();
                    }
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

                if (!stack.back().expression_routine_return_pending &&
                    executed_statement_count >= max_executed_statements)
                {
                    last_error_message = step_budget_limit_message();
                    last_fault_location = next->location;
                    last_fault_statement = next->text;
                    events.push_back({.category = "runtime.error",
                                      .detail = last_error_message,
                                      .location = next->location});
                    return finalize_pause_state(DebugPauseReason::error, last_error_message);
                }

                const bool resuming_expression =
                    stack.back().expression_routine_return_pending;
                if (!resuming_expression && breakpoint_matches(next->location))
                {
                    if (resume_skip_breakpoint_location.has_value() &&
                        paths_equal_for_platform(resume_skip_breakpoint_location->file_path, next->location.file_path) &&
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
                                [](const TryState &state) { return try_state_can_process_fault(state); });
                            if (has_open_try)
                            {
                                // Pop intermediate frames back to this parent.
                                while (stack.size() > depth_at_fault - i)
                                {
                                    pop_frame();
                                }
                                if (!stack.empty())
                                {
                                    stack.back().expression_routine_return_pending = false;
                                    stack.back().expression_continuation.reset();
                                    stack.back().command_target_continuation.reset();
                                    stack.back().command_array_name_continuation.reset();
                                    stack.back().command_argument_continuation.reset();
                                    stack.back().text_merge_continuation.reset();
                                    stack.back().parameter_default_continuation.reset();
                                    stack.back().use_command_continuation.reset();
                                    stack.back().copy_file_continuation.reset();
                                    stack.back().rename_file_continuation.reset();
                                    stack.back().loop_expression_continuation.reset();
                                    stack.back().scan_expression_continuation.reset();
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
                    if (stack.size() <= base_depth &&
                        !stack.back().expression_routine_return_pending)
                    {
                        return finalize_pause_state(
                            DebugPauseReason::step,
                            runtime_text("Runtime.Prg.Session.Message.StepOverCompleted"));
                    }
                    break;
                case DebugResumeAction::step_out:
                    if (stack.size() < base_depth &&
                        !stack.back().expression_routine_return_pending)
                    {
                        return finalize_pause_state(
                            DebugPauseReason::step,
                            runtime_text("Runtime.Prg.Session.Message.StepOutCompleted"));
                    }
                    break;
                }
            }
        }
        catch (const PrgCompatibilityError &error)
        {
            ensure_fault_context_defaults(current_statement(), last_fault_location, last_fault_statement);
            last_error_message = error.what();
            last_error_code = error.error_code();
            last_error_compatibility = {};
            last_error_compatibility.explicit_error_code = last_error_code;
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
                                          ? copperfin::platform::path_to_utf8_string(
                                                copperfin::platform::path_from_utf8_string(effective.startup_path).parent_path())
                                          : normalize_path(effective.working_directory);

        if (const auto config = load_runtime_config_near(
                copperfin::platform::path_from_utf8_string(effective.startup_path),
                copperfin::platform::path_from_utf8_string(effective.working_directory)))
        {
            apply_runtime_config_defaults(effective, *config);
        }

        auto impl = std::make_unique<Impl>(effective);
        impl->startup_default_directory = effective.working_directory;
        impl->default_directory_by_session.emplace(1, impl->startup_default_directory);
        impl->data_sessions.try_emplace(1);
        const int application_surface_handle =
            impl->register_ole_object("_SCREEN", "runtime application surface");
        if (const auto application_surface = impl->ole_objects.find(application_surface_handle);
            application_surface != impl->ole_objects.end())
        {
            application_surface->second.hidden_runtime_surface = true;
            application_surface->second.base_class_name = "Screen";
            application_surface->second.class_hierarchy = {"SCREEN", "OBJECT"};
            application_surface->second.properties["mousepointer"] = make_number_value(0.0);
            application_surface->second.properties["showtips"] = make_boolean_value(false);
            application_surface->second.properties["minwidth"] = make_number_value(-1.0);
            application_surface->second.properties["minheight"] = make_number_value(-1.0);
            application_surface->second.properties["maxwidth"] = make_number_value(-1.0);
            application_surface->second.properties["maxheight"] = make_number_value(-1.0);
            impl->representative_application_surface_handle = application_surface_handle;
            const PrgValue application_surface_reference = make_string_value(
                "object:" + application_surface->second.prog_id + "#" +
                std::to_string(application_surface->second.handle));
            const int projects_handle = impl->next_ole_handle++;
            RuntimeOleObjectState projects_state{
                .handle = projects_handle,
                .prog_id = "Collection",
                .source = {},
                .last_action = "Projects",
                .action_count = 1,
                .hidden_runtime_surface = true,
                .read_only_collection_surface = true};
            projects_state.base_class_name = "Collection";
            projects_state.class_hierarchy = {"COLLECTION", "OBJECT"};
            const auto [projects_it, _] = impl->ole_objects.emplace(
                projects_handle,
                std::move(projects_state));
            application_surface->second.properties["projects"] = make_string_value(
                "object:" + projects_it->second.prog_id + "#" +
                std::to_string(projects_it->second.handle));
            application_surface->second.properties["activeproject"] = make_empty_value();
            impl->globals["_screen"] = application_surface_reference;
            impl->globals["_vfp"] = application_surface_reference;
            impl->globals["application"] = application_surface_reference;
        }
        impl->events.push_back({.category = "runtime.config",
                                .detail = "temp=" + copperfin::platform::path_to_utf8_string(impl->runtime_temp_directory) +
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
                return paths_equal_for_platform(candidate.file_path, normalized.file_path) &&
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
                return paths_equal_for_platform(candidate.file_path, normalized.file_path) &&
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

    bool PrgRuntimeSession::dispatch_popup_bar_selection(
        const std::string &popup_name,
        std::int64_t bar_number)
    {
        return impl_->dispatch_popup_bar_selection(popup_name, bar_number);
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

    void PrgRuntimeSession::request_cancel()
    {
        if (impl_ != nullptr && impl_->task_cancel_requested != nullptr)
        {
            impl_->task_cancel_requested->store(true, std::memory_order_relaxed);
        }
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

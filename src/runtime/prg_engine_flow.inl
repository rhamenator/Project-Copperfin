// prg_engine_flow.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

#if defined(COPPERFIN_PRG_ENGINE_IMPL_CONTEXT)

        void restore_private_declarations(Frame &frame)
        {
            for (const auto &[name, saved] : frame.private_saved_values)
            {
                if (saved.has_value())
                {
                    globals[name] = *saved;
                }
                else
                {
                    globals.erase(name);
                }
            }
        }

        void sync_byref_arguments(Frame &frame)
        {
            if (frame.parameter_reference_bindings.empty())
            {
                return;
            }

            Frame *caller = stack.size() >= 2U ? &stack[stack.size() - 2U] : nullptr;
            for (const auto &[parameter_name, reference_name] : frame.parameter_reference_bindings)
            {
                const auto local = frame.locals.find(parameter_name);
                if (local == frame.locals.end())
                {
                    continue;
                }
                if (caller != nullptr)
                {
                    assign_variable(*caller, reference_name, local->second);
                }
                else
                {
                    globals[normalize_memory_variable_identifier(reference_name)] = local->second;
                }
            }
        }

        void pop_frame()
        {
            if (!stack.empty())
            {
                sync_byref_arguments(stack.back());
                restore_private_declarations(stack.back());
                stack.pop_back();
            }
        }

        bool breakpoint_matches(const SourceLocation &location) const
        {
            const std::string normalized = normalize_path(location.file_path);
            return std::any_of(breakpoints.begin(), breakpoints.end(), [&](const RuntimeBreakpoint &breakpoint)
                               { return normalize_path(breakpoint.file_path) == normalized && breakpoint.line == location.line; });
        }

        std::optional<std::size_t> find_matching_branch(const Frame &frame, std::size_t pc, bool seek_else) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::if_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endif_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
                else if (seek_else && kind == StatementKind::else_statement && depth == 0)
                {
                    return index;
                }
            }
            return std::nullopt;
        }

        std::optional<std::size_t> find_matching_endfor(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::for_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endfor_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
            }
            return std::nullopt;
        }

        std::optional<std::size_t> find_matching_enddo(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::do_while_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::enddo_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
            }
            return std::nullopt;
        }

        std::optional<std::size_t> find_matching_endcase(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::do_case_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endcase_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
            }
            return std::nullopt;
        }

        std::optional<std::size_t> find_matching_endwith(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::with_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endwith_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
            }
            return std::nullopt;
        }

        struct TryClauseTargets
        {
            std::optional<std::size_t> catch_statement_index;
            std::optional<std::size_t> finally_statement_index;
            std::optional<std::size_t> endtry_statement_index;
        };

        TryClauseTargets find_try_clause_targets(const Frame &frame, std::size_t pc) const
        {
            TryClauseTargets targets;
            if (frame.routine == nullptr)
            {
                return targets;
            }

            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::try_statement)
                {
                    ++depth;
                    continue;
                }
                if (kind == StatementKind::endtry_statement)
                {
                    if (depth == 0)
                    {
                        targets.endtry_statement_index = index;
                        return targets;
                    }
                    --depth;
                    continue;
                }
                if (depth != 0)
                {
                    continue;
                }
                if (kind == StatementKind::catch_statement && !targets.catch_statement_index.has_value())
                {
                    targets.catch_statement_index = index;
                }
                else if (kind == StatementKind::finally_statement && !targets.finally_statement_index.has_value())
                {
                    targets.finally_statement_index = index;
                }
            }

            return targets;
        }

        std::string apply_with_context(std::string text, const Frame &frame) const
        {
            if (frame.withs.empty())
            {
                return text;
            }

            const std::string binding_name = frame.withs.back().binding_name;
            if (binding_name.empty())
            {
                return text;
            }

            std::string rewritten;
            rewritten.reserve(text.size() + binding_name.size() * 2U);
            bool in_string = false;
            for (std::size_t index = 0U; index < text.size(); ++index)
            {
                const char ch = text[index];
                if (ch == '\'')
                {
                    in_string = !in_string;
                    rewritten.push_back(ch);
                    continue;
                }
                if (in_string || ch != '.' || (index + 1U) >= text.size())
                {
                    rewritten.push_back(ch);
                    continue;
                }

                const unsigned char next = static_cast<unsigned char>(text[index + 1U]);
                if (std::isalpha(next) == 0 && next != '_')
                {
                    rewritten.push_back(ch);
                    continue;
                }

                char previous_nonspace = '\0';
                for (std::size_t lookback = index; lookback > 0U; --lookback)
                {
                    const char candidate = text[lookback - 1U];
                    if (std::isspace(static_cast<unsigned char>(candidate)) != 0)
                    {
                        continue;
                    }
                    previous_nonspace = candidate;
                    break;
                }

                if (std::isalnum(static_cast<unsigned char>(previous_nonspace)) != 0 ||
                    previous_nonspace == '_' ||
                    previous_nonspace == '.')
                {
                    rewritten.push_back(ch);
                    continue;
                }

                rewritten.append(binding_name);
                rewritten.push_back(ch);
            }

            return rewritten;
        }

        bool dispatch_try_handler(Frame &frame, const Statement &statement)
        {
            for (auto iterator = frame.tries.rbegin(); iterator != frame.tries.rend(); ++iterator)
            {
                TryState &active_try = *iterator;
                if (active_try.handling_error)
                {
                    continue;
                }

                active_try.handling_error = true;
                active_try.entered_catch = false;
                active_try.entered_finally = false;
                if (!active_try.catch_variable.empty())
                {
                    assign_variable(frame, active_try.catch_variable, make_string_value(last_error_message));
                }

                if (active_try.catch_statement_index.has_value())
                {
                    frame.pc = *active_try.catch_statement_index + 1U;
                    active_try.entered_catch = true;
                }
                else if (active_try.finally_statement_index.has_value())
                {
                    frame.pc = *active_try.finally_statement_index + 1U;
                    active_try.entered_finally = true;
                }
                else
                {
                    frame.pc = active_try.endtry_statement_index + 1U;
                    frame.tries.erase(std::next(iterator).base());
                }

                events.push_back({.category = "runtime.try_handler",
                                  .detail = statement.text,
                                  .location = statement.location});
                return true;
            }

            return false;
        }

        std::optional<std::size_t> find_next_case_clause(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::do_case_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endcase_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
                else if (depth == 0 && (kind == StatementKind::case_statement || kind == StatementKind::otherwise_statement))
                {
                    return index;
                }
            }
            return std::nullopt;
        }

        enum class ActiveLoopKind
        {
            for_loop,
            scan_loop,
            while_loop
        };

        struct ActiveLoop
        {
            ActiveLoopKind kind = ActiveLoopKind::for_loop;
            std::size_t start_statement_index = 0;
            std::size_t end_statement_index = 0;
        };

        std::optional<ActiveLoop> find_innermost_active_loop(const Frame &frame) const
        {
            std::optional<ActiveLoop> active;
            const auto consider = [&](ActiveLoop candidate)
            {
                if (!active.has_value() || candidate.start_statement_index > active->start_statement_index)
                {
                    active = candidate;
                }
            };

            if (!frame.loops.empty())
            {
                const LoopState &loop = frame.loops.back();
                consider({.kind = ActiveLoopKind::for_loop,
                          .start_statement_index = loop.for_statement_index,
                          .end_statement_index = loop.endfor_statement_index});
            }
            if (!frame.scans.empty())
            {
                const ScanState &scan = frame.scans.back();
                consider({.kind = ActiveLoopKind::scan_loop,
                          .start_statement_index = scan.scan_statement_index,
                          .end_statement_index = scan.endscan_statement_index});
            }
            if (!frame.whiles.empty())
            {
                const WhileState &loop = frame.whiles.back();
                consider({.kind = ActiveLoopKind::while_loop,
                          .start_statement_index = loop.do_while_statement_index,
                          .end_statement_index = loop.enddo_statement_index});
            }

            return active;
        }

        ExecutionOutcome continue_for_loop(Frame &frame, const Statement &, bool jump_after_completion)
        {
            if (frame.loops.empty())
            {
                return {};
            }

            LoopState &loop = frame.loops.back();
            ++loop.iteration_count;
            if (loop.iteration_count > max_loop_iterations)
            {
                last_error_message = loop_iteration_limit_message();
                return {.ok = false, .message = last_error_message};
            }
            const double next_value = value_as_number(lookup_variable(frame, loop.variable_name)) + loop.step_value;
            assign_variable(frame, loop.variable_name, make_number_value(next_value));
            const bool should_continue = loop.step_value >= 0.0
                                             ? next_value <= loop.end_value
                                             : next_value >= loop.end_value;
            if (should_continue)
            {
                frame.pc = loop.for_statement_index + 1U;
            }
            else
            {
                const std::size_t completion_pc = loop.endfor_statement_index + 1U;
                frame.loops.pop_back();
                if (jump_after_completion)
                {
                    frame.pc = completion_pc;
                }
            }
            return {};
        }

        ExecutionOutcome continue_scan_loop(Frame &frame, const Statement &statement, bool jump_after_completion)
        {
            if (frame.scans.empty())
            {
                return {};
            }

            ScanState &scan = frame.scans.back();
            ++scan.iteration_count;
            if (scan.iteration_count > max_loop_iterations)
            {
                last_error_message = loop_iteration_limit_message();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }
            CursorState *cursor = find_cursor_by_area(scan.work_area);
            if (cursor == nullptr)
            {
                frame.scans.pop_back();
                return {};
            }

            if (!locate_next_matching_record(*cursor, scan.for_expression, scan.while_expression, frame, cursor->recno + 1U))
            {
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            if (cursor->found)
            {
                frame.pc = scan.scan_statement_index + 1U;
            }
            else
            {
                frame.scans.pop_back();
                if (jump_after_completion)
                {
                    frame.pc = scan.endscan_statement_index + 1U;
                }
            }
            return {};
        }

        std::filesystem::path resolve_asset_path(const std::string &raw_path, const char *extension) const
        {
            std::filesystem::path asset_path(unquote_string(take_first_token(raw_path)));
            if (asset_path.extension().empty())
            {
                asset_path += extension;
            }
            if (asset_path.is_relative())
            {
                asset_path = std::filesystem::path(current_default_directory()) / asset_path;
            }
            return asset_path.lexically_normal();
        }

        std::filesystem::path resolve_report_output_path(const std::string &to_clause, const Frame &frame)
        {
            std::string target = trim_copy(to_clause);
            if (target.empty())
            {
                return {};
            }

            if (starts_with_insensitive(target, "FILE "))
            {
                target = trim_copy(target.substr(5U));
            }
            if (target.empty())
            {
                return {};
            }

            std::filesystem::path output_path;
            if (target.size() >= 2U && target.front() == '\'' && target.back() == '\'')
            {
                output_path = std::filesystem::path(unquote_string(target));
            }
            else
            {
                output_path = std::filesystem::path(value_as_string(evaluate_expression(target, frame)));
            }

            if (output_path.is_relative())
            {
                output_path = std::filesystem::path(current_default_directory()) / output_path;
            }
            return output_path.lexically_normal();
        }

        ExecutionOutcome open_report_surface(const Statement &statement, const Frame &frame, const char *extension, const char *category_prefix)
        {
            const std::filesystem::path asset_path = resolve_asset_path(statement.identifier, extension);
            if (!std::filesystem::exists(asset_path))
            {
                last_error_message = std::string("Unable to resolve report asset: ") + asset_path.string();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            studio::StudioOpenRequest request;
            request.path = asset_path.string();
            request.read_only = true;
            request.load_full_table = true;
            const auto open_result = studio::open_document(request);
            if (!open_result.ok)
            {
                last_error_message = open_result.error;
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            const auto layout = studio::build_report_layout(open_result.document);
            const bool preview_mode =
                normalize_identifier(statement.secondary_expression) == "preview" ||
                trim_copy(statement.tertiary_expression).empty();
            if (preview_mode)
            {
                waiting_for_events = true;
                events.push_back({.category = std::string(category_prefix) + ".preview",
                                  .detail = asset_path.string(),
                                  .location = statement.location});
                if (layout.available)
                {
                    events.push_back({.category = std::string(category_prefix) + ".preview.layout",
                                      .detail = std::to_string(layout.sections.size()) + " sections",
                                      .location = statement.location});
                }
                return {.ok = true, .waiting_for_events = true, .frame_returned = false, .message = {}};
            }

            const std::filesystem::path output_path = resolve_report_output_path(statement.tertiary_expression, frame);
            if (output_path.empty())
            {
                last_error_message = "REPORT/LABEL TO clause requires a writable output path";
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            if (!output_path.parent_path().empty())
            {
                std::error_code ignored;
                std::filesystem::create_directories(output_path.parent_path(), ignored);
            }

            std::ofstream output(output_path, std::ios::binary);
            if (!output.good())
            {
                last_error_message = "Unable to open report output path: " + output_path.string();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            output << "Copperfin " << category_prefix << " render\n";
            output << "source=" << asset_path.string() << "\n";
            if (layout.available)
            {
                output << "sections=" << layout.sections.size() << "\n";
            }
            output.close();
            if (!output.good())
            {
                last_error_message = "Unable to write report output path: " + output_path.string();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            events.push_back({.category = std::string(category_prefix) + ".render",
                              .detail = output_path.string(),
                              .location = statement.location});
            return {};
        }

#endif // defined(COPPERFIN_PRG_ENGINE_IMPL_CONTEXT)

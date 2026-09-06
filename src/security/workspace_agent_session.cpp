// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_session.h"

#include "copperfin/security/workspace_agent_session_test_hooks.h"

#include "../platform/bounded_process_private.h"

#include "copperfin/platform/path.h"
#include "copperfin/platform/process_arguments.h"
#include "copperfin/platform/process_environment.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <iomanip>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#elif defined(__linux__)
#include <cerrno>
#include <pthread.h>
#include <sys/random.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#endif

namespace copperfin::security {

struct WorkspaceAgentSessionRevocationLeaseState {
    explicit WorkspaceAgentSessionRevocationLeaseState(
        const std::uint64_t generation_value)
        : generation(generation_value) {}

    std::mutex mutex;
    std::condition_variable released;
    std::uint64_t generation = 0U;
    bool active = true;
    std::size_t outstanding_leases = 0U;
};

class WorkspaceAgentSessionRevocationLease::Impl {
public:
    explicit Impl(
        std::shared_ptr<WorkspaceAgentSessionRevocationLeaseState> state_value)
        : state(std::move(state_value)) {}

    ~Impl() {
        if (!owns_lease || state == nullptr) {
            return;
        }
        std::lock_guard lock(state->mutex);
        --state->outstanding_leases;
        owns_lease = false;
        if (state->outstanding_leases == 0U) {
            state->released.notify_all();
        }
    }

    [[nodiscard]] bool acquire() {
        std::lock_guard lock(state->mutex);
        if (!state->active) {
            return false;
        }
        ++state->outstanding_leases;
        owns_lease = true;
        return true;
    }

    std::shared_ptr<WorkspaceAgentSessionRevocationLeaseState> state;
    bool owns_lease = false;
};

WorkspaceAgentSessionRevocationLease::WorkspaceAgentSessionRevocationLease(
    std::unique_ptr<Impl> impl)
    : impl_(std::move(impl)) {}

WorkspaceAgentSessionRevocationLease::~WorkspaceAgentSessionRevocationLease() = default;
WorkspaceAgentSessionRevocationLease::WorkspaceAgentSessionRevocationLease(
    WorkspaceAgentSessionRevocationLease&&) noexcept = default;
WorkspaceAgentSessionRevocationLease&
WorkspaceAgentSessionRevocationLease::operator=(
    WorkspaceAgentSessionRevocationLease&&) noexcept = default;

bool WorkspaceAgentSessionRevocationLease::valid() const noexcept {
    return impl_ != nullptr && impl_->owns_lease && impl_->state != nullptr;
}

std::uint64_t
WorkspaceAgentSessionRevocationLease::session_generation() const noexcept {
    return valid() ? impl_->state->generation : 0U;
}

class WorkspaceAgentPreparedProcessLaunch::Impl {
public:
    Impl(
        WorkspaceAgentSerializedProcessInvocationPreflightResult plan_value,
        WorkspaceAgentProcessTargetPins pins_value,
        WorkspaceAgentSessionRevocationLease lease_value,
        std::shared_ptr<const std::uint8_t> controller_authority_value) noexcept
        : controller_authority(std::move(controller_authority_value)),
          lease(std::move(lease_value)),
          pins(std::move(pins_value)),
          plan(std::move(plan_value)) {}

    [[nodiscard]] bool valid() const noexcept {
        return plan.allowed && plan.serialized_environment.allowed &&
            plan.serialized_environment.environment_plan.allowed &&
            controller_authority != nullptr && pins.valid() && lease.valid() &&
            lease.session_generation() ==
                plan.serialized_environment.environment_plan.session_generation;
    }

    // Destruction is reverse declaration order: discard the plan, close the
    // retained target objects, and only then release the revocation lease.
    std::shared_ptr<const std::uint8_t> controller_authority;
    WorkspaceAgentSessionRevocationLease lease;
    WorkspaceAgentProcessTargetPins pins;
    WorkspaceAgentSerializedProcessInvocationPreflightResult plan;
};

WorkspaceAgentPreparedProcessLaunch::WorkspaceAgentPreparedProcessLaunch(
    std::unique_ptr<Impl> impl) noexcept
    : impl_(std::move(impl)) {}

WorkspaceAgentPreparedProcessLaunch::WorkspaceAgentPreparedProcessLaunch() =
    default;
WorkspaceAgentPreparedProcessLaunch::~WorkspaceAgentPreparedProcessLaunch() = default;
WorkspaceAgentPreparedProcessLaunch::WorkspaceAgentPreparedProcessLaunch(
    WorkspaceAgentPreparedProcessLaunch&&) noexcept = default;
WorkspaceAgentPreparedProcessLaunch&
WorkspaceAgentPreparedProcessLaunch::operator=(
    WorkspaceAgentPreparedProcessLaunch&&) noexcept = default;

bool WorkspaceAgentPreparedProcessLaunch::valid() const noexcept {
    return impl_ != nullptr && impl_->valid();
}

std::uint64_t
WorkspaceAgentPreparedProcessLaunch::session_generation() const noexcept {
    return valid()
        ? impl_->plan.serialized_environment.environment_plan.session_generation
        : 0U;
}

class WorkspaceAgentMaterializedProcessLaunch::Impl {
public:
    Impl(
        WorkspaceAgentPreparedProcessLaunch candidate_value,
        WorkspaceAgentMaterializedProcessImage image_value) noexcept
        : candidate(std::move(candidate_value)), image(std::move(image_value)) {}

    [[nodiscard]] bool valid() const noexcept {
        return candidate.valid() && image.valid() &&
            candidate.session_generation() == image.session_generation();
    }

    void release_launch_authority() noexcept {
        candidate = WorkspaceAgentPreparedProcessLaunch{};
    }

    // Destruction is reverse declaration order: remove/close the materialized
    // image first, then discard the prepared candidate, whose own ordering
    // closes target pins before releasing the revocation lease.
    WorkspaceAgentPreparedProcessLaunch candidate;
    WorkspaceAgentMaterializedProcessImage image;
};

WorkspaceAgentMaterializedProcessLaunch::WorkspaceAgentMaterializedProcessLaunch(
    std::unique_ptr<Impl> impl) noexcept
    : impl_(std::move(impl)) {}
WorkspaceAgentMaterializedProcessLaunch::WorkspaceAgentMaterializedProcessLaunch() =
    default;
WorkspaceAgentMaterializedProcessLaunch::~WorkspaceAgentMaterializedProcessLaunch() =
    default;
WorkspaceAgentMaterializedProcessLaunch::WorkspaceAgentMaterializedProcessLaunch(
    WorkspaceAgentMaterializedProcessLaunch&&) noexcept = default;
WorkspaceAgentMaterializedProcessLaunch&
WorkspaceAgentMaterializedProcessLaunch::operator=(
    WorkspaceAgentMaterializedProcessLaunch&&) noexcept = default;

bool WorkspaceAgentMaterializedProcessLaunch::valid() const noexcept {
    return impl_ != nullptr && impl_->valid();
}

std::uint64_t
WorkspaceAgentMaterializedProcessLaunch::session_generation() const noexcept {
    return valid() ? impl_->candidate.session_generation() : 0U;
}

static_assert(
    std::is_nothrow_default_constructible_v<
        WorkspaceAgentMaterializedProcessLaunchResult> &&
        std::is_nothrow_move_constructible_v<
            WorkspaceAgentMaterializedProcessLaunchResult>,
    "RQ-CF-AGENT-026: materialization failure fallback must not allocate");
static_assert(
    std::is_nothrow_move_assignable_v<
        copperfin::platform::BoundedProcessResult>,
    "RQ-CF-AGENT-028: a completed private launch must reach outcome audit without allocating while transferring its result");

namespace {

// Resets target to reset_value under mutex on destruction, unless disarmed
// or never armed. This exists so every WorkspaceAgentSessionController
// method that sets transition_ to a non-idle value can guarantee it is
// restored on every exit path -- return or exception -- from a single
// structural point instead of a hand-copied catch(...) block per method.
// start()'s stop()'s original catch-based guard covered only exceptions
// thrown after the guard was set up, not exceptions thrown by copying
// state needed to set it up in the first place (issue #5401); arming this
// guard immediately after the assignment it protects, before any other
// work in the same critical section, closes that gap by construction.
// Templated (rather than naming Transition explicitly) so this anonymous-
// namespace helper needs no access to that private nested enum -- the type
// is deduced from the constructor arguments inside each member function
// that already has that access.
template <typename T>
class ResetOnExit {
public:
    ResetOnExit(std::mutex& mutex, T& target, T reset_value)
        : mutex_(mutex), target_(target), reset_value_(std::move(reset_value)) {}
    ResetOnExit(const ResetOnExit&) = delete;
    ResetOnExit& operator=(const ResetOnExit&) = delete;
    ~ResetOnExit() {
        if (armed_) {
            try {
                std::lock_guard lock(mutex_);
                target_ = reset_value_;
            } catch (...) {
                // A mutex primitive throwing here is an unrecoverable host
                // condition with nothing safe to report it to (this can run
                // while another exception is already unwinding). Swallow
                // rather than let it escape a destructor and std::terminate()
                // the whole process: every session entry point already
                // fails closed (denies further start/stop/cleanup) while
                // target_ is left at its non-idle value, so this degrades to
                // "permanently fail-closed" instead of a crash.
            }
        }
    }

    void arm() noexcept { armed_ = true; }
    void disarm() noexcept { armed_ = false; }

private:
    std::mutex& mutex_;
    T& target_;
    T reset_value_;
    bool armed_ = false;
};

#if defined(COPPERFIN_ENABLE_WORKSPACE_AGENT_SESSION_TEST_HOOKS)
// See workspace_agent_session_test_hooks.h. Relaxed ordering is sufficient:
// this exists only for single-threaded test setup/teardown around a call to
// stop(), never for production synchronization.
std::atomic<void (*)()> stop_test_only_throw_hook{nullptr};
#endif

std::atomic<std::uint64_t> next_workspace_agent_operation_id{1U};

std::uint64_t current_process_execution_identity() noexcept {
#if defined(_WIN32)
    return static_cast<std::uint64_t>(::GetCurrentProcessId());
#else
    const auto process_id = ::getpid();
    return process_id > 0 ? static_cast<std::uint64_t>(process_id) : 0U;
#endif
}

#if !defined(_WIN32)
// RQ-CF-AGENT-028 / issue #5493: a PID-only check can only ever detect a
// fork from the forked-away child's side -- its getpid() genuinely differs
// from the value captured before an application-supplied audit-sink
// callback ran. The parent's own PID never changes across its own fork()
// call, so it is permanently blind to a fork having happened at all unless
// something else observes it. pthread_atfork() lets both continuations
// observe the same fork: its child handler runs in the new child
// immediately after fork() (before fork() returns there), and its parent
// handler runs in the parent immediately after fork() returns there. Each
// continuation starts from the same pre-fork counter value and bumps its
// own copy exactly once, so a caller that snapshots this counter before a
// callback and compares it after sees a change on *either* side of a fork
// that occurred during the callback -- not just the side whose own PID
// changed.
std::atomic<std::uint64_t> workspace_agent_fork_generation{0U};

void workspace_agent_fork_generation_child_handler() noexcept {
    workspace_agent_fork_generation.fetch_add(1U, std::memory_order_relaxed);
}

void workspace_agent_fork_generation_parent_handler() noexcept {
    workspace_agent_fork_generation.fetch_add(1U, std::memory_order_relaxed);
}

std::uint64_t current_workspace_agent_fork_generation() noexcept {
    // A function-local static's initializer runs at most once even under
    // concurrent first calls (C++11 thread-safe initialization), so this
    // registers the atfork handlers exactly once per process regardless of
    // how many threads call this function.
    static const int registered = ::pthread_atfork(
        nullptr, workspace_agent_fork_generation_parent_handler,
        workspace_agent_fork_generation_child_handler);
    (void)registered;
    return workspace_agent_fork_generation.load(std::memory_order_relaxed);
}
#else
std::uint64_t current_workspace_agent_fork_generation() noexcept {
    return 0U;
}
#endif

std::string make_workspace_agent_operation_namespace() noexcept {
    std::array<unsigned char, 16U> bytes{};
#if defined(_WIN32)
    if (::BCryptGenRandom(
            nullptr, bytes.data(), static_cast<ULONG>(bytes.size()),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0) {
        return {};
    }
#elif defined(__linux__)
    std::size_t offset = 0U;
    while (offset < bytes.size()) {
        const ssize_t count =
            ::getrandom(bytes.data() + offset, bytes.size() - offset, 0);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count <= 0) {
            return {};
        }
        offset += static_cast<std::size_t>(count);
    }
#elif defined(__APPLE__)
    ::arc4random_buf(bytes.data(), bytes.size());
#else
    return {};
#endif
    bool any_nonzero = false;
    for (const unsigned char value : bytes) {
        any_nonzero = any_nonzero || value != 0U;
    }
    if (!any_nonzero) {
        return {};
    }
    try {
        static constexpr char hex[] = "0123456789abcdef";
        std::string result(bytes.size() * 2U, '0');
        for (std::size_t index = 0U; index < bytes.size(); ++index) {
            result[index * 2U] = hex[(bytes[index] >> 4U) & 0x0fU];
            result[index * 2U + 1U] = hex[bytes[index] & 0x0fU];
        }
        return result;
    } catch (...) {
        return {};
    }
}

std::uint64_t allocate_workspace_agent_operation_id() noexcept {
    std::uint64_t current =
        next_workspace_agent_operation_id.load(std::memory_order_relaxed);
    while (current != 0U &&
           current != std::numeric_limits<std::uint64_t>::max()) {
        if (next_workspace_agent_operation_id.compare_exchange_weak(
                current, current + 1U, std::memory_order_relaxed,
                std::memory_order_relaxed)) {
            return current;
        }
    }
    return 0U;
}

struct AuditOutcome {
    bool committed = false;
    std::string receipt;
};

enum class ControllerCallbackKind : std::uint32_t {
    audit,
    cancellation
};

class ControllerCallbackScope;
thread_local const ControllerCallbackScope* active_controller_callback_scope =
    nullptr;

class ControllerCallbackScope {
public:
    ControllerCallbackScope(
        const WorkspaceAgentSessionController* controller,
        const ControllerCallbackKind kind) noexcept
        : controller_(controller),
          kind_(kind),
          previous_(active_controller_callback_scope) {
        active_controller_callback_scope = this;
    }
    ~ControllerCallbackScope() {
        active_controller_callback_scope = previous_;
    }
    ControllerCallbackScope(const ControllerCallbackScope&) = delete;
    ControllerCallbackScope& operator=(const ControllerCallbackScope&) = delete;

    [[nodiscard]] const WorkspaceAgentSessionController* controller()
        const noexcept {
        return controller_;
    }
    [[nodiscard]] ControllerCallbackKind kind() const noexcept { return kind_; }
    [[nodiscard]] const ControllerCallbackScope* previous() const noexcept {
        return previous_;
    }

private:
    const WorkspaceAgentSessionController* controller_ = nullptr;
    ControllerCallbackKind kind_ = ControllerCallbackKind::audit;
    const ControllerCallbackScope* previous_ = nullptr;
};

std::optional<ControllerCallbackKind> controller_callback_active_for(
    const WorkspaceAgentSessionController* controller) noexcept {
    if (controller == nullptr) {
        return std::nullopt;
    }
    for (const ControllerCallbackScope* scope = active_controller_callback_scope;
         scope != nullptr; scope = scope->previous()) {
        if (scope->controller() == controller) {
            return scope->kind();
        }
    }
    return std::nullopt;
}

AuditOutcome commit_audit_event(
    const WorkspaceAgentSessionAuditEvent& event,
    const WorkspaceAgentSessionAuditSink& sink,
    const WorkspaceAgentSessionController* controller) {
    WorkspaceAgentSessionAuditCommitResult result;
    if (sink.commit != nullptr) {
        try {
            const ControllerCallbackScope audit_scope(
                controller, ControllerCallbackKind::audit);
            result = sink.commit(event, sink.context);
        } catch (...) {
            result = {};
        }
    }
    if (!result.ok || result.receipt.empty()) {
        return {};
    }
    return {.committed = true, .receipt = std::move(result.receipt)};
}

std::string event_kind_name(WorkspaceAgentSessionEventKind kind) {
    switch (kind) {
        case WorkspaceAgentSessionEventKind::start:
            return "start";
        case WorkspaceAgentSessionEventKind::stop:
            return "stop";
        case WorkspaceAgentSessionEventKind::layout_cleanup_intent:
            return "layout_cleanup_intent";
        case WorkspaceAgentSessionEventKind::layout_cleanup_outcome:
            return "layout_cleanup_outcome";
        case WorkspaceAgentSessionEventKind::process_launch_intent:
            return "process_launch_intent";
        case WorkspaceAgentSessionEventKind::process_launch_outcome:
            return "process_launch_outcome";
        case WorkspaceAgentSessionEventKind::workspace_file_read_intent:
            return "workspace_file_read_intent";
        case WorkspaceAgentSessionEventKind::workspace_file_read_outcome:
            return "workspace_file_read_outcome";
    }
    return "invalid";
}

std::string json_escape(std::string_view value) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    for (const unsigned char character : value) {
        switch (character) {
            case '\"':
                stream << "\\\"";
                break;
            case '\\':
                stream << "\\\\";
                break;
            case '\b':
                stream << "\\b";
                break;
            case '\f':
                stream << "\\f";
                break;
            case '\n':
                stream << "\\n";
                break;
            case '\r':
                stream << "\\r";
                break;
            case '\t':
                stream << "\\t";
                break;
            default:
                if (character < 0x20U || character == 0x7fU) {
                    stream << "\\u"
                           << std::hex << std::setw(4) << std::setfill('0')
                           << static_cast<unsigned int>(character)
                           << std::dec << std::setw(0);
                } else {
                    stream << static_cast<char>(character);
                }
                break;
        }
    }
    return stream.str();
}

WorkspaceAgentActivationDecision controller_denial(std::string diagnostic_code) {
    WorkspaceAgentActivationDecision decision;
    decision.diagnostic_code = std::move(diagnostic_code);
    decision.audit_required = true;
    return decision;
}

bool satisfies_tool_requirements(
    const WorkspaceAgentCapabilities& capabilities,
    const WorkspaceAgentToolRequirements& requirements) noexcept {
    return (!requirements.read_workspace_files || capabilities.read_workspace_files) &&
        (!requirements.write_workspace_files || capabilities.write_workspace_files) &&
        (!requirements.run_local_processes || capabilities.run_local_processes) &&
        (!requirements.access_outside_workspace || capabilities.access_outside_workspace) &&
        (!requirements.use_network || capabilities.use_network) &&
        (!requirements.elevate_privileges || capabilities.elevate_privileges);
}

bool valid_utf8(std::string_view value) noexcept {
    std::size_t offset = 0U;
    while (offset < value.size()) {
        const auto lead = static_cast<unsigned char>(value[offset]);
        std::size_t continuation_count = 0U;
        std::uint32_t scalar = 0U;
        if (lead <= 0x7fU) {
            ++offset;
            continue;
        }
        if (lead >= 0xc2U && lead <= 0xdfU) {
            continuation_count = 1U;
            scalar = lead & 0x1fU;
        } else if (lead >= 0xe0U && lead <= 0xefU) {
            continuation_count = 2U;
            scalar = lead & 0x0fU;
        } else if (lead >= 0xf0U && lead <= 0xf4U) {
            continuation_count = 3U;
            scalar = lead & 0x07U;
        } else {
            return false;
        }
        if (continuation_count > value.size() - offset - 1U) {
            return false;
        }
        for (std::size_t index = 1U; index <= continuation_count; ++index) {
            const auto continuation =
                static_cast<unsigned char>(value[offset + index]);
            if ((continuation & 0xc0U) != 0x80U) {
                return false;
            }
            scalar = (scalar << 6U) | (continuation & 0x3fU);
        }
        if ((continuation_count == 1U && scalar < 0x80U) ||
            (continuation_count == 2U && scalar < 0x800U) ||
            (continuation_count == 3U && scalar < 0x10000U) ||
            scalar > 0x10ffffU ||
            (scalar >= 0xd800U && scalar <= 0xdfffU)) {
            return false;
        }
        offset += continuation_count + 1U;
    }
    return true;
}

std::string process_arguments_diagnostic(
    const std::vector<std::string>& arguments) {
    if (arguments.size() > workspace_agent_process_max_argument_count) {
        return "workspace_agent.process_argument_count_exceeded";
    }
    std::size_t total_bytes = 0U;
    for (const auto& argument : arguments) {
        if (argument.size() > workspace_agent_process_max_argument_bytes) {
            return "workspace_agent.process_argument_size_exceeded";
        }
        if (argument.find('\0') != std::string::npos) {
            return "workspace_agent.process_argument_embedded_nul";
        }
        if (!valid_utf8(argument)) {
            return "workspace_agent.process_argument_invalid_utf8";
        }
        if (argument.size() >
            workspace_agent_process_max_total_argument_bytes - total_bytes) {
            return "workspace_agent.process_argument_total_size_exceeded";
        }
        total_bytes += argument.size();
    }
    return {};
}

bool same_serialized_process_invocation(
    const WorkspaceAgentSerializedProcessInvocationPreflightResult& left,
    const WorkspaceAgentSerializedProcessInvocationPreflightResult& right) {
    const auto& left_environment = left.serialized_environment;
    const auto& right_environment = right.serialized_environment;
    const auto& left_plan = left_environment.environment_plan;
    const auto& right_plan = right_environment.environment_plan;
    return left.allowed == right.allowed &&
        left.diagnostic_code == right.diagnostic_code &&
        left_environment.allowed == right_environment.allowed &&
        left_environment.diagnostic_code == right_environment.diagnostic_code &&
        left_plan.allowed == right_plan.allowed &&
        left_plan.diagnostic_code == right_plan.diagnostic_code &&
        left_plan.session_generation == right_plan.session_generation &&
        left_plan.effective_mode == right_plan.effective_mode &&
        left_plan.tool_id == right_plan.tool_id &&
        left_plan.canonical_executable_path ==
            right_plan.canonical_executable_path &&
        left_plan.executable_identity == right_plan.executable_identity &&
        left_plan.canonical_working_directory ==
            right_plan.canonical_working_directory &&
        left_plan.working_directory_identity ==
            right_plan.working_directory_identity &&
        left_plan.arguments == right_plan.arguments &&
        left_plan.environment_policy == right_plan.environment_policy &&
        left_plan.environment_platform == right_plan.environment_platform &&
        left_plan.environment_entries == right_plan.environment_entries &&
        left_environment.posix_environment ==
            right_environment.posix_environment &&
        left_environment.windows_environment_block ==
            right_environment.windows_environment_block &&
        left.posix_arguments == right.posix_arguments &&
        left.windows_command_line == right.windows_command_line &&
        left.argument_parser_contract == right.argument_parser_contract;
}

}  // namespace

#if defined(COPPERFIN_ENABLE_WORKSPACE_AGENT_SESSION_TEST_HOOKS)
void set_workspace_agent_session_stop_test_only_throw_hook_for_testing(
    void (*hook)()) {
    stop_test_only_throw_hook.store(hook, std::memory_order_relaxed);
}
#endif

WorkspaceAgentSessionController::WorkspaceAgentSessionController(
    const std::filesystem::path& trusted_absolute_workspace_root)
    : file_target_boundary_(WorkspaceAgentFileTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_target_boundary_(WorkspaceAgentProcessTargetBoundary::create(
          trusted_absolute_workspace_root)) {}

WorkspaceAgentSessionController::WorkspaceAgentSessionController(
    const std::filesystem::path& trusted_absolute_workspace_root,
    const WorkspaceAgentIsolatedEnvironmentConfiguration&
        trusted_environment_configuration)
    : file_target_boundary_(WorkspaceAgentFileTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_target_boundary_(WorkspaceAgentProcessTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_environment_boundary_(WorkspaceAgentIsolatedEnvironmentBoundary::create(
          trusted_environment_configuration)),
      process_environment_configuration_supplied_(true) {}

WorkspaceAgentSessionController::WorkspaceAgentSessionController(
    const std::filesystem::path& trusted_absolute_workspace_root,
    const WorkspaceAgentIsolatedEnvironmentConfiguration&
        trusted_environment_configuration,
    const WorkspaceAgentProcessParserConfiguration&
        trusted_process_parser_configuration)
    : file_target_boundary_(WorkspaceAgentFileTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_target_boundary_(WorkspaceAgentProcessTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_environment_boundary_(WorkspaceAgentIsolatedEnvironmentBoundary::create(
          trusted_environment_configuration)),
      process_parser_boundary_(WorkspaceAgentProcessParserBoundary::create(
          trusted_process_parser_configuration)),
      process_environment_configuration_supplied_(true) {}

WorkspaceAgentSessionStartResult WorkspaceAgentSessionController::start(
    const WorkspaceAgentActivationRequest& request,
    const WorkspaceAgentSessionAuditSink& audit_sink) {
    std::uint64_t candidate_generation = 0U;
    bool session_already_active = false;
    ResetOnExit reset_guard(mutex_, transition_, Transition::idle);
    {
        std::lock_guard lock(mutex_);
        if (transition_ != Transition::idle) {
            WorkspaceAgentSessionStartResult result;
            result.diagnostic_code = "workspace_agent.session_transition_in_progress";
            result.session = active_session_;
            return result;
        }
        if (next_generation_ == std::numeric_limits<std::uint64_t>::max()) {
            WorkspaceAgentSessionStartResult result;
            result.diagnostic_code = "workspace_agent.session_generation_exhausted";
            result.session = active_session_;
            return result;
        }
        transition_ = Transition::starting;
        reset_guard.arm();
        candidate_generation = next_generation_++;
        session_already_active = active_session_.active;
    }

    WorkspaceAgentActivationDecision decision;
    if (session_already_active) {
        decision = controller_denial("workspace_agent.session_already_active");
    } else {
        try {
            decision = evaluate_workspace_agent_activation(request);
        } catch (...) {
            decision = controller_denial("workspace_agent.policy_evaluation_failed");
        }
    }
    std::optional<WorkspaceAgentSessionLayoutPreparationResult> preparation;
    std::shared_ptr<WorkspaceAgentSessionRevocationLeaseState>
        candidate_revocation_state;
    if (decision.allowed && decision.capabilities.run_local_processes &&
        process_environment_configuration_supplied_) {
        if (!process_environment_boundary_.has_value()) {
            decision = controller_denial(
                "workspace_agent.session_environment_boundary_unavailable");
        } else {
            try {
                bool cleanup_receipt_capacity_available = false;
                {
                    std::lock_guard lock(mutex_);
                    cleanup_receipt_capacity_available =
                        pending_layout_cleanups_.size() <
                        workspace_agent_session_max_pending_layout_cleanups;
                    if (cleanup_receipt_capacity_available) {
                        // Allocate receipt storage before creating
                        // filesystem state. The later move cannot orphan a
                        // prepared layout merely because the FIFO grows.
                        pending_layout_cleanups_.reserve(
                            pending_layout_cleanups_.size() + 1U);
                    }
                }
                if (!cleanup_receipt_capacity_available) {
                    decision = controller_denial(
                        "workspace_agent.session_layout_cleanup_capacity_reached");
                } else {
                    preparation =
                        process_environment_boundary_->prepare_session_layout(
                            candidate_generation);
                    if (!preparation->prepared ||
                        preparation->session_generation != candidate_generation) {
                        decision = controller_denial(
                            preparation->diagnostic_code.empty()
                                ? "workspace_agent.session_environment_preparation_failed"
                                : preparation->diagnostic_code);
                    }
                }
            } catch (...) {
                decision = controller_denial(
                    "workspace_agent.session_environment_preparation_failed");
            }
        }
    }
    if (decision.allowed && decision.capabilities.run_local_processes) {
        try {
            candidate_revocation_state =
                std::make_shared<WorkspaceAgentSessionRevocationLeaseState>(
                    candidate_generation);
        } catch (...) {
            decision = controller_denial(
                "workspace_agent.session_revocation_lease_unavailable");
        }
    }
    const WorkspaceAgentSessionAuditEvent event{
        .kind = WorkspaceAgentSessionEventKind::start,
        .session_generation = candidate_generation,
        .requested_mode = request.requested_mode,
        .effective_mode = decision.effective_mode,
        .outcome = decision.allowed ? "allowed" : "denied",
        .diagnostic_code = decision.diagnostic_code};
    const AuditOutcome audit = commit_audit_event(event, audit_sink, this);

    WorkspaceAgentSessionStartResult result;
    result.audit_committed = audit.committed;
    result.audit_receipt = audit.receipt;
    result.policy_decision = decision;
    result.diagnostic_code = audit.committed
        ? decision.diagnostic_code
        : "workspace_agent.session_audit_commit_failed";

    {
        std::lock_guard lock(mutex_);
        if (preparation.has_value() && preparation->prepared &&
            preparation->session_generation == candidate_generation) {
            pending_layout_cleanups_.push_back(std::move(*preparation));
        }
        if (audit.committed && decision.allowed && !active_session_.active) {
            active_session_ = {
                .active = true,
                .generation = candidate_generation,
                .effective_mode = decision.effective_mode,
                .capabilities = decision.capabilities,
                .activation_audit_receipt = audit.receipt};
            active_revocation_lease_state_ =
                std::move(candidate_revocation_state);
            result.activated = true;
        }
        transition_ = Transition::idle;
        reset_guard.disarm();
        result.session = active_session_;
    }
    return result;
}

WorkspaceAgentSessionLayoutCleanupAttemptResult
WorkspaceAgentSessionController::cleanup_pending_session_layout(
    const WorkspaceAgentSessionAuditSink& audit_sink) {
    const WorkspaceAgentSessionLayoutPreparationResult* preparation = nullptr;
    ResetOnExit reset_guard(mutex_, transition_, Transition::idle);
    {
        std::lock_guard lock(mutex_);
        WorkspaceAgentSessionLayoutCleanupAttemptResult result;
        if (transition_ != Transition::idle) {
            result.diagnostic_code =
                "workspace_agent.session_transition_in_progress";
            return result;
        }
        if (active_session_.active) {
            result.diagnostic_code =
                "workspace_agent.session_layout_cleanup_active_session";
            return result;
        }
        if (pending_layout_cleanups_.empty()) {
            result.diagnostic_code =
                "workspace_agent.session_layout_cleanup_not_pending";
            return result;
        }
        transition_ = Transition::cleaning;
        reset_guard.arm();
        // The cleaning transition prevents every operation that can mutate the
        // receipt FIFO. Borrow the front receipt instead of copying its
        // heap-backed fields after changing state: this assignment cannot
        // throw and therefore cannot strand the controller in `cleaning`.
        preparation = &pending_layout_cleanups_.front();
    }

    WorkspaceAgentSessionLayoutCleanupAttemptResult result;
    result.session_generation = preparation->session_generation;
    const WorkspaceAgentSessionAuditEvent intent{
        .kind = WorkspaceAgentSessionEventKind::layout_cleanup_intent,
        .session_generation = preparation->session_generation,
        .requested_mode = WorkspaceAgentAccessMode::advisory,
        .effective_mode = WorkspaceAgentAccessMode::advisory,
        .outcome = "pending",
        .diagnostic_code = "workspace_agent.session_layout_cleanup_intent"};
    const AuditOutcome intent_audit =
        commit_audit_event(intent, audit_sink, this);
    result.intent_audit_committed = intent_audit.committed;
    result.intent_audit_receipt = intent_audit.receipt;
    if (!intent_audit.committed) {
        result.diagnostic_code =
            "workspace_agent.session_layout_cleanup_intent_audit_failed";
        return result;
    }

    WorkspaceAgentSessionLayoutCleanupResult cleanup;
    if (!process_environment_boundary_.has_value()) {
        cleanup.diagnostic_code =
            "workspace_agent.session_environment_boundary_unavailable";
    } else {
        try {
            result.attempted = true;
            cleanup =
                process_environment_boundary_->cleanup_empty_session_layout(
                    *preparation);
        } catch (...) {
            cleanup.diagnostic_code =
                "workspace_agent.environment_session_layout_cleanup_failed";
        }
    }
    result.cleaned = cleanup.cleaned;

    const WorkspaceAgentSessionAuditEvent outcome{
        .kind = WorkspaceAgentSessionEventKind::layout_cleanup_outcome,
        .session_generation = preparation->session_generation,
        .requested_mode = WorkspaceAgentAccessMode::advisory,
        .effective_mode = WorkspaceAgentAccessMode::advisory,
        .outcome = cleanup.cleaned ? "cleaned" : "retained",
        .diagnostic_code = cleanup.diagnostic_code.empty()
            ? "workspace_agent.environment_session_layout_cleanup_failed"
            : cleanup.diagnostic_code};
    const AuditOutcome outcome_audit =
        commit_audit_event(outcome, audit_sink, this);
    result.outcome_audit_committed = outcome_audit.committed;
    result.outcome_audit_receipt = outcome_audit.receipt;
    result.diagnostic_code = outcome_audit.committed
        ? outcome.diagnostic_code
        : "workspace_agent.session_layout_cleanup_outcome_audit_failed";
    {
        std::lock_guard lock(mutex_);
        if (cleanup.cleaned && !pending_layout_cleanups_.empty() &&
            pending_layout_cleanups_.front().session_generation ==
                preparation->session_generation) {
            pending_layout_cleanups_.erase(pending_layout_cleanups_.begin());
        }
        transition_ = Transition::idle;
        reset_guard.disarm();
    }
    return result;
}

WorkspaceAgentSessionStopResult WorkspaceAgentSessionController::stop(
    const WorkspaceAgentSessionAuditSink& audit_sink) {
    const auto active_callback = controller_callback_active_for(this);
    if (active_callback.has_value()) {
        WorkspaceAgentSessionStopResult result;
        result.diagnostic_code =
            *active_callback == ControllerCallbackKind::cancellation
            ? "workspace_agent.session_reentrant_cancellation_transition_denied"
            : "workspace_agent.session_reentrant_audit_transition_denied";
        std::lock_guard lock(mutex_);
        result.session = active_session_;
        return result;
    }
    WorkspaceAgentSessionSnapshot revoked_session;
    std::shared_ptr<WorkspaceAgentSessionRevocationLeaseState> revocation_state;
    // Armed the moment transition_ becomes stopping (issue #5401), before
    // the copies immediately below it: WorkspaceAgentSessionSnapshot owns a
    // std::string, so copying it can throw (e.g. bad_alloc), and that copy
    // happens while still holding the same lock that sets transition_ --
    // there is no trivially-non-throwing gap here the way start() has
    // between setting transition_ = starting and its own try block. Every
    // exit from this point on, return or exception, restores transition_ to
    // idle via this guard's destructor; no catch(...) block is needed.
    ResetOnExit reset_guard(mutex_, transition_, Transition::idle);
    {
        std::lock_guard lock(mutex_);
        if (transition_ != Transition::idle) {
            WorkspaceAgentSessionStopResult result;
            result.diagnostic_code = "workspace_agent.session_transition_in_progress";
            result.session = active_session_;
            return result;
        }
        if (!active_session_.active) {
            WorkspaceAgentSessionStopResult result;
            result.diagnostic_code = "workspace_agent.session_not_active";
            result.session = active_session_;
            return result;
        }
        transition_ = Transition::stopping;
        reset_guard.arm();
#if defined(COPPERFIN_ENABLE_WORKSPACE_AGENT_SESSION_TEST_HOOKS)
        // The fault-injection hook fires here, deliberately before the
        // WorkspaceAgentSessionSnapshot copy immediately below (rather than
        // after this lock block closes) so a test can exercise exactly the
        // window this guard exists to cover: transition_ has already
        // changed, but the throw-capable state copy has not happened yet.
        if (const auto hook =
                stop_test_only_throw_hook.load(std::memory_order_relaxed);
            hook != nullptr) {
            stop_test_only_throw_hook.store(nullptr, std::memory_order_relaxed);
            hook();
        }
#endif
        revoked_session = active_session_;
        revocation_state = active_revocation_lease_state_;
    }

    // A lease is held only around a future direct launch decision. Waiting
    // here ensures stop cannot report revocation in the middle of that
    // narrow boundary. The lease release path never takes the controller
    // mutex.
    if (revocation_state != nullptr) {
        std::unique_lock revocation_lock(revocation_state->mutex);
        revocation_state->released.wait(revocation_lock, [&revocation_state] {
            return revocation_state->outstanding_leases == 0U;
        });
        revocation_state->active = false;
    }
    {
        std::lock_guard lock(mutex_);
        active_session_ = {};
        active_revocation_lease_state_.reset();
    }

    const WorkspaceAgentSessionAuditEvent event{
        .kind = WorkspaceAgentSessionEventKind::stop,
        .session_generation = revoked_session.generation,
        .requested_mode = revoked_session.effective_mode,
        .effective_mode = WorkspaceAgentAccessMode::advisory,
        .outcome = "revoked",
        .diagnostic_code = "workspace_agent.session_stopped"};
    const AuditOutcome audit = commit_audit_event(event, audit_sink, this);

    WorkspaceAgentSessionStopResult result;
    result.revoked = true;
    result.audit_committed = audit.committed;
    result.audit_receipt = audit.receipt;
    result.diagnostic_code = audit.committed
        ? event.diagnostic_code
        : "workspace_agent.session_stop_audit_commit_failed";
    {
        std::lock_guard lock(mutex_);
        transition_ = Transition::idle;
        reset_guard.disarm();
        result.session = active_session_;
    }
    return result;
}

WorkspaceAgentSessionSnapshot WorkspaceAgentSessionController::snapshot() const {
    std::lock_guard lock(mutex_);
    return active_session_;
}

WorkspaceAgentToolPreflightResult WorkspaceAgentSessionController::preflight_tool_request(
    const WorkspaceAgentToolPreflightRequest& request) const {
    WorkspaceAgentToolPreflightResult result;
    if (request.schema_version != 1U) {
        result.diagnostic_code = "workspace_agent.tool_invalid_schema";
        return result;
    }
    const WorkspaceAgentToolDefinition* definition =
        find_workspace_agent_product_tool(request.tool_id);
    if (definition == nullptr) {
        result.diagnostic_code = "workspace_agent.tool_not_registered";
        return result;
    }

    std::lock_guard lock(mutex_);
    if (transition_ != Transition::idle) {
        result.diagnostic_code = "workspace_agent.session_transition_in_progress";
        return result;
    }
    if (!active_session_.active) {
        result.diagnostic_code = "workspace_agent.session_not_active";
        return result;
    }
    if (request.session_generation == 0U ||
        request.session_generation != active_session_.generation) {
        result.diagnostic_code = "workspace_agent.tool_stale_session";
        return result;
    }
    result.session_generation = active_session_.generation;
    result.effective_mode = active_session_.effective_mode;
    result.tool_id = std::string(definition->id);
    if (!satisfies_tool_requirements(
            active_session_.capabilities,
            definition->requirements)) {
        result.diagnostic_code = "workspace_agent.tool_capability_denied";
        return result;
    }
    result.allowed = true;
    result.diagnostic_code = "workspace_agent.tool_request_allowed";
    return result;
}

WorkspaceAgentFileTargetPreflightResult
WorkspaceAgentSessionController::preflight_file_target_request(
    const WorkspaceAgentFileTargetPreflightRequest& request) const {
    WorkspaceAgentFileTargetPreflightResult result;
    const WorkspaceAgentToolPreflightRequest tool_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id};
    const auto preliminary = preflight_tool_request(tool_request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }

    result.session_generation = preliminary.session_generation;
    result.effective_mode = preliminary.effective_mode;
    result.tool_id = preliminary.tool_id;
    const WorkspaceAgentToolDefinition* definition =
        find_workspace_agent_product_tool(preliminary.tool_id);
    if (definition == nullptr) {
        result.diagnostic_code = "workspace_agent.tool_not_registered";
        return result;
    }

    WorkspaceAgentFileTargetInspection inspection;
    switch (definition->target_kind) {
        case WorkspaceAgentToolTargetKind::workspace_file:
            if (!file_target_boundary_.has_value()) {
                result.diagnostic_code =
                    "workspace_agent.target_workspace_root_not_configured";
                return result;
            }
            inspection = file_target_boundary_->inspect_workspace_file(
                request.target_path);
            break;
        case WorkspaceAgentToolTargetKind::local_file:
            if (!file_target_boundary_.has_value()) {
                // Construction of the product-owned boundary is also the
                // trusted configuration gate for local target inspection.
                result.diagnostic_code =
                    "workspace_agent.target_workspace_root_not_configured";
                return result;
            }
            inspection = file_target_boundary_->inspect_local_file(
                request.target_path);
            break;
        case WorkspaceAgentToolTargetKind::workspace_process:
        case WorkspaceAgentToolTargetKind::local_process:
        case WorkspaceAgentToolTargetKind::network_endpoint:
            result.diagnostic_code = "workspace_agent.target_not_file_tool";
            return result;
    }
    if (!inspection.allowed) {
        result.diagnostic_code = inspection.diagnostic_code;
        return result;
    }

    // Recheck after filesystem inspection so a stop or transition observed
    // during resolution cannot yield an allowed result. A future executor must
    // still repeat this beside the side effect because stop can follow return.
    const auto final_preflight = preflight_tool_request(tool_request);
    if (!final_preflight.allowed ||
        final_preflight.session_generation != preliminary.session_generation ||
        final_preflight.tool_id != preliminary.tool_id) {
        result = {};
        result.diagnostic_code = final_preflight.allowed
            ? "workspace_agent.tool_stale_session"
            : final_preflight.diagnostic_code;
        return result;
    }

    result.allowed = true;
    result.canonical_path = std::move(inspection.canonical_path);
    result.identity = inspection.identity;
    result.diagnostic_code = "workspace_agent.target_request_allowed";
    return result;
}

WorkspaceAgentWorkspaceFileReadResult
WorkspaceAgentSessionController::read_workspace_file_snapshot(
    const WorkspaceAgentWorkspaceFileReadRequest& request,
    const WorkspaceAgentSessionAuditSink& audit_sink) const {
    WorkspaceAgentWorkspaceFileReadResult unavailable;
    try {
        if (request.schema_version != 1U) {
            unavailable.diagnostic_code = "workspace_agent.file_read_invalid_schema";
            return unavailable;
        }
        const WorkspaceAgentFileTargetPreflightRequest target_request{
            .schema_version = 1U,
            .session_generation = request.session_generation,
            .tool_id = std::string(workspace_agent_tool_workspace_inspect),
            .target_path = request.target_path};
        const auto preliminary = preflight_file_target_request(target_request);
        if (!preliminary.allowed) {
            unavailable.diagnostic_code = preliminary.diagnostic_code;
            return unavailable;
        }
        if (preliminary.tool_id != workspace_agent_tool_workspace_inspect ||
            !file_target_boundary_.has_value()) {
            unavailable.diagnostic_code = "workspace_agent.file_read_unavailable";
            return unavailable;
        }

        const std::uint64_t execution_process_identity =
            current_process_execution_identity();
        const std::uint64_t fork_generation_at_intent =
            current_workspace_agent_fork_generation();
        const std::string operation_instance_id =
            make_workspace_agent_operation_namespace();
        if (execution_process_identity == 0U || operation_instance_id.empty()) {
            unavailable.diagnostic_code = "workspace_agent.file_read_namespace_unavailable";
            return unavailable;
        }
        std::uint64_t operation_id = 0U;
        {
            std::lock_guard lock(mutex_);
            if (transition_ != Transition::idle || !active_session_.active ||
                preliminary.session_generation == 0U ||
                active_session_.generation != preliminary.session_generation ||
                active_session_.effective_mode != preliminary.effective_mode) {
                unavailable.diagnostic_code = "workspace_agent.file_read_stale_session";
                return unavailable;
            }
            operation_id = allocate_workspace_agent_operation_id();
            if (operation_id == 0U) {
                unavailable.diagnostic_code = "workspace_agent.file_read_namespace_exhausted";
                return unavailable;
            }
        }

        WorkspaceAgentWorkspaceFileReadResult result;
        result.session_generation = preliminary.session_generation;
        result.operation_instance_id = operation_instance_id;
        result.operation_id = operation_id;
        const WorkspaceAgentSessionAuditEvent intent{
            .schema_version = 3U,
            .kind = WorkspaceAgentSessionEventKind::workspace_file_read_intent,
            .session_generation = preliminary.session_generation,
            .requested_mode = preliminary.effective_mode,
            .effective_mode = preliminary.effective_mode,
            .operation_id = operation_id,
            .operation_instance_id = operation_instance_id,
            .outcome = "pending",
            .diagnostic_code = "workspace_agent.file_read_intent"};
        WorkspaceAgentSessionAuditEvent outcome{
            .schema_version = 3U,
            .kind = WorkspaceAgentSessionEventKind::workspace_file_read_outcome,
            .session_generation = preliminary.session_generation,
            .requested_mode = preliminary.effective_mode,
            .effective_mode = preliminary.effective_mode,
            .operation_id = operation_id,
            .operation_instance_id = operation_instance_id,
            .outcome = "failed",
            .diagnostic_code = "workspace_agent.file_read_failed"};
        const AuditOutcome intent_audit = commit_audit_event(intent, audit_sink, this);
        result.intent_audit_committed = intent_audit.committed;
        result.intent_audit_receipt = intent_audit.receipt;
        if (!intent_audit.committed) {
            result.diagnostic_code = "workspace_agent.file_read_intent_audit_failed";
            return result;
        }
        if (current_process_execution_identity() != execution_process_identity) {
            // Forked-away child continuation: its own PID differs, so it
            // denies and stops without committing an outcome audit,
            // mirroring execute_materialized_process_launch's identical
            // treatment of this side (RQ-CF-AGENT-028 / issue #5493).
            result.diagnostic_code =
                "workspace_agent.file_read_process_changed_after_intent_audit";
            return result;
        }
        // A PID-only check is permanently blind to a fork that happened
        // during the callback but left this continuation's own PID
        // unchanged -- that is the parent's side of exactly that fork. See
        // current_workspace_agent_fork_generation()'s doc comment.
        const bool fork_observed_after_intent_audit =
            current_workspace_agent_fork_generation() !=
            fork_generation_at_intent;

        if (fork_observed_after_intent_audit) {
            result.diagnostic_code =
                "workspace_agent.file_read_fork_observed_after_intent_audit";
            outcome.diagnostic_code = result.diagnostic_code;
        } else {
            result.attempted = true;
            const WorkspaceAgentFileTargetInspection expected{
                .allowed = true,
                .canonical_path = preliminary.canonical_path,
                .identity = preliminary.identity,
                .diagnostic_code = "workspace_agent.target_request_allowed"};
            const auto snapshot = file_target_boundary_->snapshot_workspace_file(
                expected, workspace_agent_workspace_file_read_max_bytes);
            if (snapshot.captured) {
                const auto final_preflight = preflight_file_target_request(target_request);
                if (final_preflight.allowed &&
                    final_preflight.session_generation == preliminary.session_generation &&
                    final_preflight.tool_id == preliminary.tool_id &&
                    final_preflight.canonical_path == preliminary.canonical_path &&
                    final_preflight.identity == preliminary.identity &&
                    snapshot.identity == preliminary.identity) {
                    result.bytes = snapshot.bytes;
                    result.diagnostic_code = "workspace_agent.file_read_captured";
                    outcome.outcome = "captured";
                    outcome.diagnostic_code = result.diagnostic_code;
                } else {
                    result.diagnostic_code = "workspace_agent.file_read_identity_changed";
                    outcome.diagnostic_code = result.diagnostic_code;
                }
            } else {
                result.diagnostic_code = snapshot.diagnostic_code.empty()
                    ? "workspace_agent.file_read_failed"
                    : snapshot.diagnostic_code;
                outcome.diagnostic_code = result.diagnostic_code;
            }
        }
        const AuditOutcome outcome_audit = commit_audit_event(outcome, audit_sink, this);
        result.outcome_audit_committed = outcome_audit.committed;
        result.outcome_audit_receipt = outcome_audit.receipt;
        if (!outcome_audit.committed) {
            result.bytes.clear();
            result.diagnostic_code = "workspace_agent.file_read_outcome_audit_failed";
        }
        return result;
    } catch (...) {
        unavailable.diagnostic_code = "workspace_agent.file_read_unavailable";
        return unavailable;
    }
}

WorkspaceAgentProcessTargetPreflightResult
WorkspaceAgentSessionController::preflight_process_target_request(
    const WorkspaceAgentProcessTargetPreflightRequest& request) const {
    WorkspaceAgentProcessTargetPreflightResult result;
    const WorkspaceAgentToolPreflightRequest tool_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id};
    const auto preliminary = preflight_tool_request(tool_request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }

    result.session_generation = preliminary.session_generation;
    result.effective_mode = preliminary.effective_mode;
    result.tool_id = preliminary.tool_id;
    const WorkspaceAgentToolDefinition* definition =
        find_workspace_agent_product_tool(preliminary.tool_id);
    if (definition == nullptr) {
        result.diagnostic_code = "workspace_agent.tool_not_registered";
        return result;
    }

    WorkspaceAgentProcessTargetInspection inspection;
    switch (definition->target_kind) {
        case WorkspaceAgentToolTargetKind::workspace_process:
            if (!process_target_boundary_.has_value()) {
                result.diagnostic_code =
                    "workspace_agent.process_workspace_root_not_configured";
                return result;
            }
            inspection = process_target_boundary_->preflight_workspace_process(
                request.executable_path, request.working_directory);
            break;
        case WorkspaceAgentToolTargetKind::local_process:
            if (!process_target_boundary_.has_value()) {
                result.diagnostic_code =
                    "workspace_agent.process_workspace_root_not_configured";
                return result;
            }
            inspection = process_target_boundary_->preflight_local_process(
                request.executable_path, request.working_directory);
            break;
        case WorkspaceAgentToolTargetKind::workspace_file:
        case WorkspaceAgentToolTargetKind::local_file:
        case WorkspaceAgentToolTargetKind::network_endpoint:
            result.diagnostic_code = "workspace_agent.target_not_process_tool";
            return result;
    }
    if (!inspection.allowed) {
        result.diagnostic_code = inspection.diagnostic_code;
        return result;
    }

    // Recheck after both filesystem inspections. This closes only the
    // point-in-time preflight interval; the executor must repeat beside launch.
    const auto final_preflight = preflight_tool_request(tool_request);
    if (!final_preflight.allowed ||
        final_preflight.session_generation != preliminary.session_generation ||
        final_preflight.tool_id != preliminary.tool_id) {
        result = {};
        result.diagnostic_code = final_preflight.allowed
            ? "workspace_agent.tool_stale_session"
            : final_preflight.diagnostic_code;
        return result;
    }

    result.allowed = true;
    result.canonical_executable_path =
        std::move(inspection.canonical_executable_path);
    result.executable_identity = inspection.executable_identity;
    result.canonical_working_directory =
        std::move(inspection.canonical_working_directory);
    result.working_directory_identity = inspection.working_directory_identity;
    result.diagnostic_code = "workspace_agent.process_target_request_allowed";
    return result;
}

WorkspaceAgentProcessInvocationPreflightResult
WorkspaceAgentSessionController::preflight_process_invocation_request(
    const WorkspaceAgentProcessInvocationPreflightRequest& request) const {
    WorkspaceAgentProcessInvocationPreflightResult result;
    const WorkspaceAgentProcessTargetPreflightRequest target_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id,
        .executable_path = request.executable_path,
        .working_directory = request.working_directory};
    const auto target = preflight_process_target_request(target_request);
    if (!target.allowed) {
        result.diagnostic_code = target.diagnostic_code;
        return result;
    }

    result.diagnostic_code = process_arguments_diagnostic(request.arguments);
    if (!result.diagnostic_code.empty()) {
        return result;
    }
    std::vector<std::string> validated_arguments = request.arguments;

    // Recheck after bounded argument validation and copying. This result is
    // still point-in-time only; an executor must repeat every check beside
    // direct launch and independently enforce the platform serialization cap.
    const WorkspaceAgentToolPreflightRequest tool_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id};
    const auto final_preflight = preflight_tool_request(tool_request);
    if (!final_preflight.allowed ||
        final_preflight.session_generation != target.session_generation ||
        final_preflight.tool_id != target.tool_id) {
        result = {};
        result.diagnostic_code = final_preflight.allowed
            ? "workspace_agent.tool_stale_session"
            : final_preflight.diagnostic_code;
        return result;
    }

    result.allowed = true;
    result.session_generation = target.session_generation;
    result.effective_mode = target.effective_mode;
    result.tool_id = target.tool_id;
    result.canonical_executable_path = target.canonical_executable_path;
    result.executable_identity = target.executable_identity;
    result.canonical_working_directory = target.canonical_working_directory;
    result.working_directory_identity = target.working_directory_identity;
    result.arguments = std::move(validated_arguments);
    result.diagnostic_code = "workspace_agent.process_invocation_request_allowed";
    return result;
}

WorkspaceAgentProcessTargetPinPreflightResult
WorkspaceAgentSessionController::pin_process_target_request(
    const WorkspaceAgentProcessTargetPreflightRequest& request) const {
    WorkspaceAgentProcessTargetPinPreflightResult result;
    const WorkspaceAgentToolPreflightRequest tool_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id};
    const auto preliminary = preflight_tool_request(tool_request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }
    if (!process_target_boundary_.has_value()) {
        result.diagnostic_code =
            "workspace_agent.process_workspace_root_not_configured";
        return result;
    }

    const WorkspaceAgentToolDefinition* definition =
        find_workspace_agent_product_tool(preliminary.tool_id);
    if (definition == nullptr) {
        result.diagnostic_code = "workspace_agent.tool_not_registered";
        return result;
    }

    WorkspaceAgentProcessTargetInspection inspection;
    switch (definition->target_kind) {
        case WorkspaceAgentToolTargetKind::workspace_process:
            inspection = process_target_boundary_->inspect_workspace_process(
                request.executable_path, request.working_directory);
            break;
        case WorkspaceAgentToolTargetKind::local_process:
            inspection = process_target_boundary_->inspect_local_process(
                request.executable_path, request.working_directory);
            break;
        case WorkspaceAgentToolTargetKind::workspace_file:
        case WorkspaceAgentToolTargetKind::local_file:
        case WorkspaceAgentToolTargetKind::network_endpoint:
            result.diagnostic_code = "workspace_agent.target_not_process_tool";
            return result;
    }
    if (!inspection.allowed) {
        result.diagnostic_code = inspection.diagnostic_code;
        return result;
    }

    auto pin_result = process_target_boundary_->pin_process_targets(inspection);
    if (!pin_result.pinned || !pin_result.pins.has_value()) {
        result.diagnostic_code = pin_result.diagnostic_code;
        return result;
    }

    // Resource acquisition can race with stop. Recheck exact session/tool
    // admission afterward and release every pin on any mismatch. The returned
    // bundle remains non-executing and is not session or launch authority.
    const auto final_preflight = preflight_tool_request(tool_request);
    if (!final_preflight.allowed ||
        final_preflight.session_generation != preliminary.session_generation ||
        final_preflight.tool_id != preliminary.tool_id) {
        result.diagnostic_code = final_preflight.allowed
            ? "workspace_agent.tool_stale_session"
            : final_preflight.diagnostic_code;
        return result;
    }

    result.pinned = true;
    result.session_generation = final_preflight.session_generation;
    result.pins = std::move(pin_result.pins);
    result.diagnostic_code =
        "workspace_agent.process_target_pin_request_allowed";
    return result;
}

WorkspaceAgentPreparedProcessLaunchResult
WorkspaceAgentSessionController::prepare_process_launch_candidate(
    const WorkspaceAgentProcessInvocationPreflightRequest& request) const {
    WorkspaceAgentPreparedProcessLaunchResult unavailable;
    try {
        WorkspaceAgentPreparedProcessLaunchResult result;
        const auto preliminary =
            preflight_serialized_process_invocation_request(request);
        if (!preliminary.allowed) {
            result.diagnostic_code = preliminary.diagnostic_code;
            return result;
        }

        const WorkspaceAgentProcessTargetPreflightRequest target_request{
            .schema_version = request.schema_version,
            .session_generation = request.session_generation,
            .tool_id = request.tool_id,
            .executable_path = request.executable_path,
            .working_directory = request.working_directory};
        auto pinned = pin_process_target_request(target_request);
        if (!pinned.pinned || !pinned.pins.has_value()) {
            result.diagnostic_code = pinned.diagnostic_code;
            return result;
        }

        auto leased = acquire_process_launch_revocation_lease(
            preliminary.serialized_environment.environment_plan
                .session_generation);
        if (!leased.acquired || !leased.lease.has_value()) {
            result.diagnostic_code = leased.diagnostic_code;
            return result;
        }

        const auto final =
            preflight_serialized_process_invocation_request(request);
        if (!final.allowed ||
            !same_serialized_process_invocation(preliminary, final)) {
            result.diagnostic_code =
                "workspace_agent.process_launch_candidate_stale_invocation";
            return result;
        }
        const auto& final_plan = final.serialized_environment.environment_plan;
        if (pinned.session_generation != final_plan.session_generation ||
            leased.lease->session_generation() !=
                final_plan.session_generation ||
            !pinned.pins->matches_target_identities(
                final_plan.executable_identity,
                final_plan.working_directory_identity)) {
            result.diagnostic_code =
                "workspace_agent.process_launch_candidate_target_changed";
            return result;
        }
        const auto authentication = pinned.pins->verify_executable_bytes();
        if (!authentication.authenticated) {
            result.diagnostic_code = authentication.diagnostic_code;
            return result;
        }

        auto impl = std::make_unique<WorkspaceAgentPreparedProcessLaunch::Impl>(
            final,
            std::move(*pinned.pins),
            std::move(*leased.lease),
            process_launch_controller_authority_);
        result.candidate.emplace(
            WorkspaceAgentPreparedProcessLaunch(std::move(impl)));
        result.prepared = result.candidate->valid();
        result.diagnostic_code = result.prepared
            ? "workspace_agent.process_launch_candidate_prepared"
            : "workspace_agent.process_launch_candidate_unavailable";
        if (!result.prepared) {
            result.candidate.reset();
        }
        return result;
    } catch (...) {
        return unavailable;
    }
}

WorkspaceAgentMaterializedProcessLaunchResult
WorkspaceAgentSessionController::materialize_process_launch_candidate(
    WorkspaceAgentPreparedProcessLaunch candidate) const {
    WorkspaceAgentMaterializedProcessLaunchResult unavailable;
    try {
        WorkspaceAgentMaterializedProcessLaunchResult result;
        if (!candidate.valid() || candidate.impl_ == nullptr ||
            candidate.impl_->controller_authority !=
                process_launch_controller_authority_) {
            result.diagnostic_code =
                "workspace_agent.process_image_candidate_unavailable";
            return result;
        }
        // Serialize the complete one-attempt materialization with stop/start.
        // Releasing this lock after only the preliminary state check would let
        // stop enter its transition while a new native image was still being
        // created from otherwise-live lease authority.
        std::lock_guard lock(mutex_);
        const std::uint64_t generation = candidate.session_generation();
        WorkspaceAgentSessionLayoutPreparationResult preparation;
        constexpr std::uint64_t maximum_name_attempts = 16U;
        std::uint64_t first_ordinal = 0U;
        if (transition_ != Transition::idle) {
            result.diagnostic_code =
                "workspace_agent.session_transition_in_progress";
            return result;
        }
        if (!active_session_.active || generation == 0U ||
            active_session_.generation != generation) {
            result.diagnostic_code =
                "workspace_agent.process_image_stale_session";
            return result;
        }
        if (!process_environment_configuration_supplied_ ||
            !process_environment_boundary_.has_value()) {
            result.diagnostic_code =
                "workspace_agent.process_image_boundary_unavailable";
            return result;
        }
        const auto receipt = std::find_if(
            pending_layout_cleanups_.begin(),
            pending_layout_cleanups_.end(),
            [generation](
                const WorkspaceAgentSessionLayoutPreparationResult& entry) {
                return entry.prepared &&
                    entry.session_generation == generation;
            });
        if (receipt == pending_layout_cleanups_.end()) {
            result.diagnostic_code =
                "workspace_agent.process_image_layout_authority_unavailable";
            return result;
        }
        if (next_materialized_process_image_ == 0U ||
            next_materialized_process_image_ >
                std::numeric_limits<std::uint64_t>::max() -
                    maximum_name_attempts) {
            result.diagnostic_code =
                "workspace_agent.process_image_namespace_exhausted";
            return result;
        }
        preparation = *receipt;
        first_ordinal = next_materialized_process_image_;
        next_materialized_process_image_ += maximum_name_attempts;

        const auto authentication =
            candidate.impl_->pins.verify_executable_bytes();
        const auto* snapshot =
            candidate.impl_->pins.executable_snapshot_for_materialization();
        if (!authentication.authenticated || snapshot == nullptr ||
            snapshot->empty()) {
            result.diagnostic_code =
                "workspace_agent.process_image_snapshot_unavailable";
            return result;
        }

        WorkspaceAgentProcessImageMaterializationResult materialized;
        for (std::uint64_t offset = 0U;
             offset < maximum_name_attempts; ++offset) {
            materialized = process_environment_boundary_->materialize_process_image(
                preparation, first_ordinal + offset, *snapshot);
            if (materialized.materialized ||
                materialized.diagnostic_code !=
                    "workspace_agent.process_image_name_collision") {
                break;
            }
        }
        if (!materialized.materialized || !materialized.image.has_value() ||
            !materialized.image->valid() ||
            materialized.session_generation != generation) {
            result.diagnostic_code = materialized.diagnostic_code.empty()
                ? "workspace_agent.process_image_materialization_failed"
                : materialized.diagnostic_code;
            return result;
        }

        auto impl =
            std::make_unique<WorkspaceAgentMaterializedProcessLaunch::Impl>(
                std::move(candidate), std::move(*materialized.image));
        result.launch.emplace(
            WorkspaceAgentMaterializedProcessLaunch(std::move(impl)));
        result.materialized = result.launch->valid();
        result.diagnostic_code = result.materialized
            ? "workspace_agent.process_launch_materialized"
            : "workspace_agent.process_image_materialization_failed";
        if (!result.materialized) {
            result.launch.reset();
        }
        return result;
    } catch (...) {
        return unavailable;
    }
}

WorkspaceAgentProcessExecutionResult
WorkspaceAgentSessionController::execute_materialized_process_launch(
    WorkspaceAgentMaterializedProcessLaunch launch,
    const WorkspaceAgentProcessExecutionControls& controls,
    const WorkspaceAgentSessionAuditSink& audit_sink) const {
    WorkspaceAgentProcessExecutionResult unavailable;
    try {
        if (!launch.valid() || launch.impl_ == nullptr ||
            !launch.impl_->candidate.valid() ||
            launch.impl_->candidate.impl_ == nullptr ||
            launch.impl_->candidate.impl_->controller_authority !=
                process_launch_controller_authority_) {
            unavailable.diagnostic_code =
                "workspace_agent.process_execution_candidate_unavailable";
            return unavailable;
        }

        const auto& plan = launch.impl_->candidate.impl_->plan;
        const auto& environment_plan =
            plan.serialized_environment.environment_plan;
        const std::uint64_t generation = launch.session_generation();
        const WorkspaceAgentAccessMode effective_mode =
            environment_plan.effective_mode;
        const std::uint64_t execution_process_identity =
            current_process_execution_identity();
        const std::uint64_t fork_generation_at_intent =
            current_workspace_agent_fork_generation();
        if (execution_process_identity == 0U) {
            unavailable.diagnostic_code =
                "workspace_agent.process_execution_namespace_unavailable";
            return unavailable;
        }
        // A fresh operating-system-random namespace for each attempt avoids
        // inheriting a cached namespace and counter position across fork().
        // The durable correlation key remains the schema-v2
        // (process_instance_id, operation_id) pair for compatibility.
        const std::string process_instance_id =
            make_workspace_agent_operation_namespace();
        if (process_instance_id.empty()) {
            unavailable.diagnostic_code =
                "workspace_agent.process_execution_namespace_unavailable";
            return unavailable;
        }
        std::uint64_t operation_id = 0U;
        {
            std::lock_guard lock(mutex_);
            if (transition_ != Transition::idle) {
                unavailable.diagnostic_code =
                    "workspace_agent.session_transition_in_progress";
                return unavailable;
            }
            if (!active_session_.active || generation == 0U ||
                active_session_.generation != generation ||
                active_session_.effective_mode != effective_mode) {
                unavailable.diagnostic_code =
                    "workspace_agent.process_execution_stale_session";
                return unavailable;
            }
            operation_id = allocate_workspace_agent_operation_id();
            if (operation_id == 0U) {
                unavailable.diagnostic_code =
                    "workspace_agent.process_execution_namespace_exhausted";
                return unavailable;
            }
        }

        WorkspaceAgentProcessExecutionResult result;
        result.session_generation = generation;
        result.process_instance_id = process_instance_id;
        result.operation_id = operation_id;

        const bool controls_valid = controls.schema_version == 1U &&
            controls.timeout_ms != 0U &&
            controls.timeout_ms <= workspace_agent_process_execution_max_timeout_ms &&
            controls.poll_interval_ms != 0U &&
            controls.poll_interval_ms <= controls.timeout_ms &&
            controls.stdin_limit_bytes != 0U &&
            controls.stdout_limit_bytes != 0U &&
            controls.stderr_limit_bytes != 0U &&
            controls.stdin_limit_bytes <=
                workspace_agent_process_execution_max_transport_bytes &&
            controls.stdout_limit_bytes <=
                workspace_agent_process_execution_max_transport_bytes &&
            controls.stderr_limit_bytes <=
                workspace_agent_process_execution_max_transport_bytes &&
            controls.standard_input.size() <= controls.stdin_limit_bytes;

        std::string denial;
        if (effective_mode != WorkspaceAgentAccessMode::unrestricted_local) {
            denial = "workspace_agent.process_execution_requires_unrestricted_local";
        } else if (!controls_valid) {
            denial = "workspace_agent.process_execution_invalid_controls";
        } else if (environment_plan.environment_platform !=
                       WorkspaceAgentProcessEnvironmentPlatform::windows_v1 ||
                   plan.argument_parser_contract !=
                       WorkspaceAgentProcessArgumentParserContract::
                           windows_c_runtime_argv_v1) {
            denial = "workspace_agent.process_execution_platform_unavailable";
        } else {
            switch (copperfin::platform::current_process_elevation()) {
                case copperfin::platform::CurrentProcessElevation::not_elevated:
                    break;
                case copperfin::platform::CurrentProcessElevation::elevated:
                    denial =
                        "workspace_agent.process_execution_elevated_host_denied";
                    break;
                case copperfin::platform::CurrentProcessElevation::unavailable:
                    denial =
                        "workspace_agent.process_execution_elevation_unavailable";
                    break;
                case copperfin::platform::CurrentProcessElevation::unsupported:
                    denial =
                        "workspace_agent.process_execution_platform_unavailable";
                    break;
            }
        }

        const std::filesystem::path* stable_working_directory = nullptr;
        if (denial.empty()) {
            stable_working_directory = launch.impl_->candidate.impl_->pins
                .execution_working_directory();
            if (stable_working_directory == nullptr) {
                denial =
                    "workspace_agent.process_execution_working_directory_unavailable";
            }
        }

        copperfin::platform::PrivateWindowsBoundedProcessRequest native_request;
        if (denial.empty()) {
            native_request.command_line = plan.windows_command_line;
            native_request.environment_block =
                plan.serialized_environment.windows_environment_block;
            native_request.working_directory = *stable_working_directory;
            native_request.transport.standard_input = controls.standard_input;
            native_request.transport.timeout_ms = controls.timeout_ms;
            native_request.transport.poll_interval_ms = controls.poll_interval_ms;
            native_request.transport.stdin_limit_bytes = controls.stdin_limit_bytes;
            native_request.transport.stdout_limit_bytes = controls.stdout_limit_bytes;
            native_request.transport.stderr_limit_bytes = controls.stderr_limit_bytes;
            if (controls.cancellation_requested) {
                native_request.transport.cancellation_requested =
                    [controller = this,
                     callback = &controls.cancellation_requested]() {
                        const ControllerCallbackScope cancellation_scope(
                            controller, ControllerCallbackKind::cancellation);
                        return (*callback)();
                    };
            }
            native_request.launch_committed = [](void* context) noexcept {
                auto* retained = static_cast<WorkspaceAgentMaterializedProcessLaunch*>(
                    context);
                if (retained != nullptr && retained->impl_ != nullptr) {
                    retained->impl_->release_launch_authority();
                }
            };
            native_request.launch_committed_context = &launch;
        }

        const WorkspaceAgentSessionAuditEvent intent{
            .schema_version = 2U,
            .kind = WorkspaceAgentSessionEventKind::process_launch_intent,
            .session_generation = generation,
            .requested_mode = effective_mode,
            .effective_mode = effective_mode,
            .process_instance_id = process_instance_id,
            .operation_id = operation_id,
            .outcome = "pending",
            .diagnostic_code = "workspace_agent.process_launch_intent"};
        WorkspaceAgentSessionAuditEvent outcome_event{
            .schema_version = 2U,
            .kind = WorkspaceAgentSessionEventKind::process_launch_outcome,
            .session_generation = generation,
            .requested_mode = effective_mode,
            .effective_mode = effective_mode,
            .process_instance_id = process_instance_id,
            .operation_id = operation_id,
            .outcome = denial.empty() ? "launch-failed" : "denied",
            .diagnostic_code = denial.empty()
                ? "workspace_agent.process_execution_failed"
                : denial};
        AuditOutcome intent_audit =
            commit_audit_event(intent, audit_sink, this);
        result.intent_audit_committed = intent_audit.committed;
        result.intent_audit_receipt = std::move(intent_audit.receipt);
        if (!intent_audit.committed) {
            result.diagnostic_code =
                "workspace_agent.process_execution_intent_audit_failed";
            return result;
        }
        // An application-supplied synchronous sink can fork on POSIX and let
        // both continuations return from the callback. Only the process that
        // allocated this correlation pair may execute or submit its outcome;
        // the forked continuation releases its private launch authority and
        // stops before either action.
        if (current_process_execution_identity() != execution_process_identity) {
            // Forked-away child continuation: its own PID differs, so it
            // denies and stops without committing an outcome audit.
            result.diagnostic_code =
                "workspace_agent.process_execution_process_changed_after_intent_audit";
            return result;
        }
        // A PID-only check is permanently blind to a fork that happened
        // during the callback but left this continuation's own PID
        // unchanged -- that is the parent's side of exactly that fork
        // (RQ-CF-AGENT-028 / issue #5493). Unlike the forked-away child
        // above, the parent still commits a denied outcome audit here,
        // like any other denial reason.
        if (denial.empty() &&
            current_workspace_agent_fork_generation() !=
                fork_generation_at_intent) {
            denial = "workspace_agent.process_execution_fork_observed_after_intent_audit";
            outcome_event.outcome = "denied";
            outcome_event.diagnostic_code = denial;
        }

        if (denial.empty()) {
            result.attempted = true;
            result.process =
                copperfin::platform::run_bounded_windows_private_executable(
                    launch.impl_->image.image_, native_request);
            // Build the actual event separately so allocation failure leaves the
            // prebuilt, content-free launch-failure event intact. Once intent is
            // durable, an outcome submission must not be skipped by bookkeeping.
            try {
                WorkspaceAgentSessionAuditEvent actual_outcome{
                    .schema_version = 2U,
                    .kind = WorkspaceAgentSessionEventKind::process_launch_outcome,
                    .session_generation = generation,
                    .requested_mode = effective_mode,
                    .effective_mode = effective_mode,
                    .process_instance_id = process_instance_id,
                    .operation_id = operation_id,
                    .outcome = copperfin::platform::bounded_process_status_name(
                        result.process.status),
                    .diagnostic_code = result.process.error_code};
                outcome_event = std::move(actual_outcome);
            } catch (...) {
                // The stable fallback was fully allocated before the intent.
            }
        } else {
            result.process.status =
                copperfin::platform::BoundedProcessStatus::launch_failed;
            result.process.error_code = std::move(denial);
        }

        // Ensure the exact image is removed and any uncommitted launch lease is
        // released before the durable outcome is submitted.
        launch = WorkspaceAgentMaterializedProcessLaunch{};
        AuditOutcome outcome_audit =
            commit_audit_event(outcome_event, audit_sink, this);
        result.outcome_audit_committed = outcome_audit.committed;
        result.outcome_audit_receipt = std::move(outcome_audit.receipt);
        if (outcome_audit.committed) {
            result.diagnostic_code = std::move(outcome_event.diagnostic_code);
        } else {
            result.diagnostic_code =
                "workspace_agent.process_execution_outcome_audit_failed";
        }
        return result;
    } catch (...) {
        return unavailable;
    }
}

WorkspaceAgentProcessEnvironmentPreflightResult
WorkspaceAgentSessionController::preflight_process_environment_request(
    const WorkspaceAgentProcessInvocationPreflightRequest& request) const {
    WorkspaceAgentProcessEnvironmentPreflightResult result;
    const auto preliminary = preflight_process_invocation_request(request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }
    if (!process_environment_boundary_.has_value()) {
        result.diagnostic_code =
            "workspace_agent.process_environment_boundary_unavailable";
        return result;
    }

    const auto environment = process_environment_boundary_->construct(
        preliminary.session_generation, preliminary.environment_policy);
    if (!environment.allowed) {
        result.diagnostic_code = environment.diagnostic_code;
        return result;
    }

    const auto final = preflight_process_invocation_request(request);
    if (!final.allowed ||
        final.session_generation != preliminary.session_generation ||
        final.effective_mode != preliminary.effective_mode ||
        final.tool_id != preliminary.tool_id ||
        final.canonical_executable_path != preliminary.canonical_executable_path ||
        final.executable_identity != preliminary.executable_identity ||
        final.canonical_working_directory !=
            preliminary.canonical_working_directory ||
        final.working_directory_identity !=
            preliminary.working_directory_identity ||
        final.arguments != preliminary.arguments ||
        final.environment_policy != preliminary.environment_policy ||
        environment.session_generation != preliminary.session_generation ||
        environment.policy != preliminary.environment_policy ||
        environment.platform != workspace_agent_process_environment_host_platform()) {
        result.diagnostic_code =
            "workspace_agent.process_environment_stale_invocation";
        return result;
    }

    result.allowed = true;
    result.session_generation = final.session_generation;
    result.effective_mode = final.effective_mode;
    result.tool_id = final.tool_id;
    result.canonical_executable_path = final.canonical_executable_path;
    result.executable_identity = final.executable_identity;
    result.canonical_working_directory = final.canonical_working_directory;
    result.working_directory_identity = final.working_directory_identity;
    result.arguments = final.arguments;
    result.environment_policy = final.environment_policy;
    result.environment_platform = environment.platform;
    result.environment_entries = environment.entries;
    result.diagnostic_code = "workspace_agent.process_environment_request_allowed";
    return result;
}

WorkspaceAgentSerializedProcessEnvironmentPreflightResult
WorkspaceAgentSessionController::preflight_serialized_process_environment_request(
    const WorkspaceAgentProcessInvocationPreflightRequest& request) const {
    WorkspaceAgentSerializedProcessEnvironmentPreflightResult result;
    const auto preliminary = preflight_process_environment_request(request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }

    std::vector<copperfin::platform::ProcessEnvironmentEntry> entries;
    entries.reserve(preliminary.environment_entries.size());
    for (const auto& entry : preliminary.environment_entries) {
        entries.push_back({.name = entry.name, .value = entry.value});
    }
    if (preliminary.environment_platform !=
            WorkspaceAgentProcessEnvironmentPlatform::windows_v1 &&
        preliminary.environment_platform !=
            WorkspaceAgentProcessEnvironmentPlatform::posix_v1) {
        result.diagnostic_code =
            "workspace_agent.process_environment_serialization_invalid_platform";
        return result;
    }
    const bool windows = preliminary.environment_platform ==
        WorkspaceAgentProcessEnvironmentPlatform::windows_v1;
    const auto target = windows
        ? copperfin::platform::ProcessEnvironmentTarget::windows_utf16_v1
        : copperfin::platform::ProcessEnvironmentTarget::posix_v1;
    const std::size_t maximum_units =
        workspace_agent_serialized_environment_maximum_units(
            preliminary.environment_platform,
            preliminary.environment_entries.size());
    auto serialized = copperfin::platform::serialize_process_environment(
        entries, target, maximum_units);
    if (!serialized.ok) {
        result.diagnostic_code = serialized.diagnostic_code;
        return result;
    }

    // Serialization can allocate and copy. Repeat the complete logical
    // preflight afterward and reject any changed session, target, argument,
    // environment policy, or fixed entry set.
    const auto final = preflight_process_environment_request(request);
    if (!final.allowed ||
        final.session_generation != preliminary.session_generation ||
        final.effective_mode != preliminary.effective_mode ||
        final.tool_id != preliminary.tool_id ||
        final.canonical_executable_path != preliminary.canonical_executable_path ||
        final.executable_identity != preliminary.executable_identity ||
        final.canonical_working_directory != preliminary.canonical_working_directory ||
        final.working_directory_identity != preliminary.working_directory_identity ||
        final.arguments != preliminary.arguments ||
        final.environment_policy != preliminary.environment_policy ||
        final.environment_platform != preliminary.environment_platform ||
        final.environment_entries != preliminary.environment_entries) {
        result.diagnostic_code =
            "workspace_agent.process_environment_serialization_stale_invocation";
        return result;
    }

    result.allowed = true;
    result.environment_plan = final;
    result.posix_environment = std::move(serialized.posix_entries);
    result.windows_environment_block = std::move(serialized.windows_block);
    result.diagnostic_code =
        "workspace_agent.process_environment_serialization_request_allowed";
    return result;
}

WorkspaceAgentSerializedProcessInvocationPreflightResult
WorkspaceAgentSessionController::preflight_serialized_process_invocation_request(
    const WorkspaceAgentProcessInvocationPreflightRequest& request) const {
    WorkspaceAgentSerializedProcessInvocationPreflightResult result;
    const auto preliminary =
        preflight_serialized_process_environment_request(request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }

    const auto platform =
        preliminary.environment_plan.environment_platform;
    if (platform != WorkspaceAgentProcessEnvironmentPlatform::windows_v1 &&
        platform != WorkspaceAgentProcessEnvironmentPlatform::posix_v1) {
        result.diagnostic_code =
            "workspace_agent.process_argument_serialization_invalid_platform";
        return result;
    }
    const bool windows =
        platform == WorkspaceAgentProcessEnvironmentPlatform::windows_v1;
    WorkspaceAgentProcessArgumentParserContract parser_contract =
        WorkspaceAgentProcessArgumentParserContract::posix_argv_v1;
    if (windows) {
        if (!process_parser_boundary_.has_value()) {
            result.diagnostic_code =
                "workspace_agent.process_argument_parser_authority_unavailable";
            return result;
        }
        const auto parser = process_parser_boundary_->authorize_windows(
            preliminary.environment_plan.canonical_executable_path,
            preliminary.environment_plan.executable_identity);
        if (!parser.allowed) {
            result.diagnostic_code = parser.diagnostic_code;
            return result;
        }
        parser_contract = parser.contract;
    }
    const auto target = windows
        ? copperfin::platform::ProcessArgumentTarget::windows_command_line_v1
        : copperfin::platform::ProcessArgumentTarget::posix_v1;
    const std::size_t maximum_units = windows
        ? 32767U
        : std::numeric_limits<std::size_t>::max();
    auto serialized = copperfin::platform::serialize_process_arguments(
        copperfin::platform::path_to_utf8_string(
            preliminary.environment_plan.canonical_executable_path),
        preliminary.environment_plan.arguments,
        target,
        maximum_units);
    if (!serialized.ok) {
        result.diagnostic_code = serialized.diagnostic_code;
        return result;
    }

    // Argument serialization allocates and copies. Repeat the complete
    // environment serialization preflight afterward; neither result is an
    // authority token and a future executor must repeat beside direct launch.
    const auto final = preflight_serialized_process_environment_request(request);
    WorkspaceAgentProcessArgumentParserContract final_parser_contract =
        WorkspaceAgentProcessArgumentParserContract::posix_argv_v1;
    if (windows && final.allowed && final.environment_plan.allowed) {
        const auto final_parser = process_parser_boundary_->authorize_windows(
            final.environment_plan.canonical_executable_path,
            final.environment_plan.executable_identity);
        if (!final_parser.allowed) {
            result.diagnostic_code = final_parser.diagnostic_code;
            return result;
        }
        final_parser_contract = final_parser.contract;
    }
    if (!final.allowed ||
        !final.environment_plan.allowed ||
        !preliminary.environment_plan.allowed ||
        final.diagnostic_code != preliminary.diagnostic_code ||
        final.environment_plan.diagnostic_code !=
            preliminary.environment_plan.diagnostic_code ||
        final.environment_plan.session_generation !=
            preliminary.environment_plan.session_generation ||
        final.environment_plan.effective_mode !=
            preliminary.environment_plan.effective_mode ||
        final.environment_plan.tool_id != preliminary.environment_plan.tool_id ||
        final.environment_plan.canonical_executable_path !=
            preliminary.environment_plan.canonical_executable_path ||
        final.environment_plan.executable_identity !=
            preliminary.environment_plan.executable_identity ||
        final.environment_plan.canonical_working_directory !=
            preliminary.environment_plan.canonical_working_directory ||
        final.environment_plan.working_directory_identity !=
            preliminary.environment_plan.working_directory_identity ||
        final.environment_plan.arguments !=
            preliminary.environment_plan.arguments ||
        final.environment_plan.environment_policy !=
            preliminary.environment_plan.environment_policy ||
        final.environment_plan.environment_platform != platform ||
        final.environment_plan.environment_entries !=
            preliminary.environment_plan.environment_entries ||
        final.posix_environment != preliminary.posix_environment ||
        final.windows_environment_block !=
            preliminary.windows_environment_block ||
        final_parser_contract != parser_contract) {
        result.diagnostic_code =
            "workspace_agent.process_argument_serialization_stale_invocation";
        return result;
    }

    result.allowed = true;
    result.serialized_environment = final;
    result.posix_arguments = std::move(serialized.posix_arguments);
    result.windows_command_line =
        std::move(serialized.windows_command_line);
    result.argument_parser_contract = parser_contract;
    result.diagnostic_code =
        "workspace_agent.process_argument_serialization_request_allowed";
    return result;
}

WorkspaceAgentLaunchRevalidationResult
WorkspaceAgentSessionController::revalidate_serialized_process_invocation_for_launch(
    const WorkspaceAgentProcessInvocationPreflightRequest& request,
    const WorkspaceAgentSerializedProcessInvocationPreflightResult&
        admitted_plan) const {
    static_cast<void>(request);
    static_cast<void>(admitted_plan);
    WorkspaceAgentLaunchRevalidationResult result;
    result.diagnostic_code =
        "workspace_agent.process_launch_revalidation_pinning_unavailable";
    return result;
}

WorkspaceAgentSessionRevocationLeaseResult
WorkspaceAgentSessionController::acquire_process_launch_revocation_lease(
    const std::uint64_t session_generation) const {
    WorkspaceAgentSessionRevocationLeaseResult result;
    std::lock_guard controller_lock(mutex_);
    if (transition_ != Transition::idle) {
        result.diagnostic_code =
            "workspace_agent.session_transition_in_progress";
        return result;
    }
    if (!active_session_.active) {
        result.diagnostic_code = "workspace_agent.session_not_active";
        return result;
    }
    if (session_generation == 0U ||
        session_generation != active_session_.generation) {
        result.diagnostic_code =
            "workspace_agent.process_launch_lease_stale_session";
        return result;
    }
    if (!active_session_.capabilities.run_local_processes) {
        result.diagnostic_code =
            "workspace_agent.process_launch_lease_capability_denied";
        return result;
    }
    const auto state = active_revocation_lease_state_;
    if (state == nullptr || state->generation != session_generation) {
        result.diagnostic_code =
            "workspace_agent.process_launch_lease_unavailable";
        return result;
    }

    auto lease_impl =
        std::make_unique<WorkspaceAgentSessionRevocationLease::Impl>(state);
    if (state->generation != session_generation || !lease_impl->acquire()) {
        result.diagnostic_code =
            "workspace_agent.process_launch_lease_stale_session";
        return result;
    }
    result.lease.emplace(WorkspaceAgentSessionRevocationLease(
        std::move(lease_impl)));
    result.acquired = true;
    result.diagnostic_code =
        "workspace_agent.process_launch_revocation_lease_acquired";
    return result;
}

std::string serialize_workspace_agent_session_audit_event(
    const WorkspaceAgentSessionAuditEvent& event) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << "{\"schema_version\":" << event.schema_version
           << ",\"event\":\"" << event_kind_name(event.kind)
           << "\",\"session_generation\":" << event.session_generation
           << ",\"requested_mode\":\""
           << json_escape(workspace_agent_access_mode_name(event.requested_mode))
           << "\",\"effective_mode\":\""
           << json_escape(workspace_agent_access_mode_name(event.effective_mode))
           << "\"";
    if (event.schema_version == 2U) {
        stream << ",\"process_instance_id\":\""
               << json_escape(event.process_instance_id)
               << "\",\"operation_id\":" << event.operation_id;
    }
    if (event.schema_version >= 3U) {
        stream << ",\"operation_id\":" << event.operation_id
               << ",\"operation_instance_id\":\""
               << json_escape(event.operation_instance_id) << "\"";
    }
    stream << ",\"outcome\":\"" << json_escape(event.outcome)
           << "\",\"diagnostic_code\":\"" << json_escape(event.diagnostic_code)
           << "\"}";
    return stream.str();
}

}  // namespace copperfin::security

# System UML

This document provides a GitHub-compatible UML view of the Copperfin system.

Format choice:

- GitHub renders Mermaid diagrams natively in Markdown.
- Mermaid `classDiagram` is the safest UML-style format available directly on GitHub without requiring generated binaries or external viewers.
- The diagram below is intentionally architectural rather than code-generated. It is meant to explain subsystem boundaries and dependencies to reviewers who insist on a UML artifact.

## Core System Class Diagram

```mermaid
classDiagram
    direction LR

    class CopperfinCore {
        +Diagnostics
        +Configuration
        +MetadataContracts
        +ExtensibilityContracts
    }

    class CopperfinData {
        +DbfTableIO
        +MemoIO
        +IndexProbe
        +DbcCatalog
        +RepairValidation
    }

    class CopperfinRuntime {
        +PrgParser
        +PrgRuntimeSession
        +ExpressionEvaluator
        +WorkAreaSessionModel
        +MacroEvalCompatibility
    }

    class CopperfinConnectors {
        +SqlProviderAbstraction
        +QueryTranslation
        +SchemaIntrospection
        +ConnectionSessionModel
    }

    class CopperfinDesignModel {
        +FormModel
        +ClassModel
        +MenuModel
        +ReportModel
        +RoundTripSerializer
    }

    class CopperfinDesignHosts {
        +VisualStudioHost
        +StandaloneIDEHost
        +DesignerSurfaces
        +PropertyEditors
    }

    class CopperfinToolchain {
        +BuildPipeline
        +DebugRuntimeHost
        +Packaging
        +CLIEntryPoints
    }

    class CopperfinInterop {
        +DotNetBridge
        +OleAutomation
        +HostContainment
    }

    class CopperfinFederation {
        +BackendConnectors
        +ExecutionPlanning
        +FederatedQuerySurface
    }

    CopperfinRuntime --> CopperfinCore : uses shared contracts
    CopperfinData --> CopperfinCore : emits diagnostics
    CopperfinRuntime --> CopperfinData : reads/writes DBF/DBC/index state
    CopperfinRuntime --> CopperfinConnectors : remote cursor and SQL flow
    CopperfinConnectors --> CopperfinCore : provider contracts
    CopperfinDesignModel --> CopperfinCore : common metadata
    CopperfinDesignHosts --> CopperfinDesignModel : edits assets
    CopperfinDesignHosts --> CopperfinRuntime : executes/debugs assets
    CopperfinToolchain --> CopperfinRuntime : build/run/debug
    CopperfinToolchain --> CopperfinDesignModel : compiles packaged assets
    CopperfinInterop --> CopperfinRuntime : automation and runtime bridge
    CopperfinFederation --> CopperfinConnectors : backend access
    CopperfinFederation --> CopperfinRuntime : query/runtime integration
```

## Runtime Subsystem UML

```mermaid
classDiagram
    direction LR

    class PrgParser {
        +parse_program()
        +parse_statement()
    }

    class PrgRuntimeSession {
        +run()
        +execute_current_statement()
        +record_event()
    }

    class CursorState {
        +alias
        +work_area
        +active_order_expression
        +filter_expression
        +record_count
    }

    class DataSessionState {
        +selected_work_area
        +cursors
        +set_state
        +sql_handles
    }

    class ExpressionParser {
        +evaluate_expression()
        +parse_macro_reference()
        +resolve_identifier()
    }

    class RuntimeArraySupport {
        +ALEN()
        +ACOPY()
        +ASCAN()
        +ASORT()
    }

    class RuntimeCommandHelpers {
        +parse_field_filter_clause()
        +resolve_cursor_target_expression()
        +assign_variable()
    }

    class RuntimeInteropState {
        +ole_objects
        +declared_dll_functions
        +file_io_handles
    }

    PrgRuntimeSession --> PrgParser : consumes statements
    PrgRuntimeSession --> ExpressionParser : evaluates expressions
    PrgRuntimeSession --> RuntimeCommandHelpers : shared command helpers
    PrgRuntimeSession --> DataSessionState : owns session state
    DataSessionState --> CursorState : tracks open cursors
    PrgRuntimeSession --> RuntimeArraySupport : array semantics
    PrgRuntimeSession --> RuntimeInteropState : external handles
```

## Reading Notes

- `CopperfinRuntime` is the current execution hub.
- `CopperfinData` and `CopperfinConnectors` feed the same runtime cursor/session surface from different storage backends.
- `CopperfinDesignHosts` sit above `CopperfinDesignModel` and should not dictate runtime semantics.
- `CopperfinInterop` and `CopperfinFederation` are deliberately downstream of the runtime core because they depend on stable execution and memory semantics.

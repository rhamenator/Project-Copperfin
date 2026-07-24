# Runtime Subsystem UML

Part of [24-system-uml.md](../24-system-uml.md). This one is ground-truth-adjacent:
the class names are illustrative groupings rather than literal type names, but
each maps onto a real translation-unit family inside `cf_xbase_runtime`'s
`prg_engine.cpp` + `.inl` partials (`_dispatch`, `_flow`, `_expression`,
`_records`, `_cursor`, `_arrays`, `_variables`, `_session`, `_sql`,
`_aggregate`, `_dll`), per
[28-repository-ontology.md](../28-repository-ontology.md) §3.

This diagram is kept in its own file because GitHub's Mermaid renderer only
reliably renders the first diagram on a page; a page with several diagrams
tends to render only the first and leave the rest blank.

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

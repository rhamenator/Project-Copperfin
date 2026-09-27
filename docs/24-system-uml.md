# System UML

This document provides a GitHub-compatible UML view of the Copperfin system.

Format choice:

- GitHub renders Mermaid diagrams natively in Markdown.
- Mermaid `classDiagram` is the safest UML-style format available directly on GitHub without requiring generated binaries or external viewers.
- The current project phase/topic map lives in `docs/05-roadmap.md`, while issue-linked evidence belongs in `agent-handoff.md` and the progress documents.

This UML documentation set now carries three diagrams that must not be conflated:

1. **Ground-truth diagram** (below, in this file) — reverse-engineered directly from `CMakeLists.txt`, `src/`, `apps/`, and `vsix/`, the same way `docs/28-repository-ontology.md` was built. This is what actually compiles and links today.
2. **Aspirational target-state diagram** (in [diagrams/uml-aspirational.md](diagrams/uml-aspirational.md)) — the `copperfin-*` module taxonomy from `docs/02-architecture.md`'s "Top-Level Product Map." `docs/28-repository-ontology.md` §6 is explicit that most of these names "do not exist as separate build targets today." Do not read it as current structure.

A third diagram, the [Runtime Subsystem UML](diagrams/uml-runtime-subsystem.md), is also split out. All three diagrams used to live on this one page; GitHub's Mermaid renderer only reliably renders the first diagram on a page with multiple diagrams and silently leaves the rest blank, so only the diagram most readers need inline — the ground-truth one — stays here.

## Ground-Truth Class Diagram

This mirrors the real native library graph in subsystem-level detail, the six native executables (`copperfin_inspect`, `copperfin_mcp_host`, `copperfin_studio_host`, `copperfin_runtime_host`, `copperfin_build_host`, `copperfin_launcher_guard`), plus the managed VSIX/Studio layer that talks to them over the JSON design/runtime contract (`copperfin_studio_managed` is an `IMPORTED` CMake handle for that separately-built managed executable, not a seventh native target). Class members name real files/concepts, not aspirational APIs. Refreshed 2026-09-26 from `CMakeLists.txt`: the diagram separates the major source families within each target, includes `cf_migration` and `cf_mcp_host`, and makes private admission and platform boundaries visible without claiming that the diagram represents every C++ type.

```mermaid
classDiagram
    direction TB

    class cf_platform_support {
        +process_and_environment
        +path_and_executable_resolution
        +file_stream_and_exclusive_file
        +json_codepage_numeric_utilities
        +windows_pe_and_font_inspection
    }
    class cf_localization { +catalog_loading +locale_selection +translation }
    class cf_security {
        +authorization_and_audit
        +external_process_policy
        +path_containment_and_hardening
        +workspace_agent_session_policy
        +crypto_secret_hash_utilities
    }
    class cf_licensing { +license_payload_and_status +ed25519_verify }
    class cf_package_trust { +launcher_inventory_trust }

    class cf_platform_profile {
        +database_model_and_query_translation
        +federation_execution
        +polyglot_admission_routes_telemetry
        +printer_and_extensibility_model
    }
    class cf_sqlite_connector { +read_only_plan_execution +availability_fallback }
    class cf_vfp_assets {
        +dbf_header_table_import_encoding
        +cdx_probe_and_writer
        +access_container_long_value_queries
        +visual_asset_editor_and_undo
        +staged_import_publish
    }
    class cf_migration { +project_inventory }
    class cf_mcp_host { +stdio_protocol +read_only_dbf_header_tool }

    class cf_runtime_text { +runtime_scoped_localized_text }
    class cf_prg_analysis { +static_analysis }
    class cf_xbase_runtime {
        +parser_and_expression
        +dispatch_flow_and_error_recovery
        +cursor_records_sessions_and_relations
        +arrays_variables_and_aggregates
        +sql_index_and_table_commands
        +builtins_file_path_date_numeric_string_type
        +native_objects_ole_com_dll_and_polyglot
    }
    class cf_runtime_pipeline {
        +source_classification_and_ast_manifest
        +fxp_archive_and_library_export
        +host_auth_process_and_response
        +csharp_launcher_and_package_transaction
    }
    class cf_design_model {
        +document_workspace_and_context_actions
        +builder_designer_and_editor_dispatch
        +report_layout_and_toolbox
        +vs_launch_contract_validation
    }

    class copperfin_inspect { <<executable>> +dbf_and_index_inspection }
    class copperfin_mcp_host { <<executable>> +local_stdio_server }
    class copperfin_studio_host { <<executable>> +design_snapshot_and_toolbox_host }
    class copperfin_runtime_host { <<executable>> +runtime_debug_and_federation_host }
    class copperfin_build_host { <<executable>> +project_package_build_host }
    class copperfin_launcher_guard { <<windows_executable>> +prelaunch_trust_validation }
    class Copperfin_VisualStudio { <<managed_VSIX>> +editor_designer_debug_clients }
    class Copperfin_Studio { <<managed_WinForms>> +standalone_designer_shell }

    cf_localization --> cf_platform_support : platform services
    cf_security --> cf_localization : localized diagnostics
    cf_security --> cf_platform_support : containment/process primitives
    cf_licensing --> cf_platform_support : encoding and environment
    cf_package_trust --> cf_licensing : verify-only primitive
    cf_platform_profile --> cf_localization : user-facing model text
    cf_platform_profile --> cf_security : route admission
    cf_sqlite_connector --> cf_platform_profile : federation model
    cf_sqlite_connector --> cf_security : private admission
    cf_vfp_assets --> cf_localization : parse diagnostics
    cf_vfp_assets --> cf_security : external export admission
    cf_vfp_assets --> cf_platform_profile : supporting artifact admission
    cf_vfp_assets --> cf_platform_support : files and encoding
    cf_mcp_host --> cf_platform_support : stdio/json
    cf_mcp_host --> cf_vfp_assets : DBF header parser

    cf_prg_analysis --> cf_runtime_text : diagnostics
    cf_design_model --> cf_vfp_assets : asset documents
    cf_design_model --> cf_prg_analysis : parsed PRG insight
    cf_design_model --> cf_localization : UI contract text
    cf_xbase_runtime --> cf_prg_analysis : parsed programs
    cf_xbase_runtime --> cf_runtime_text : runtime diagnostics
    cf_xbase_runtime --> cf_design_model : xAsset execution model
    cf_xbase_runtime --> cf_vfp_assets : tables/indexes/assets
    cf_xbase_runtime --> cf_platform_profile : federation and polyglot
    cf_xbase_runtime --> cf_security : declared-call/process guards
    cf_runtime_pipeline --> cf_design_model : project model
    cf_runtime_pipeline --> cf_xbase_runtime : compilation/runtime metadata
    cf_runtime_pipeline --> cf_platform_profile : polyglot bridge plans
    cf_runtime_pipeline --> cf_security : host admission
    cf_runtime_pipeline --> cf_licensing : package metadata
    cf_runtime_pipeline --> cf_platform_support : package filesystem

    copperfin_inspect --> cf_vfp_assets
    copperfin_inspect --> cf_security
    copperfin_inspect --> cf_licensing
    copperfin_inspect --> cf_localization
    copperfin_mcp_host --> cf_mcp_host
    copperfin_mcp_host --> cf_security
    copperfin_studio_host --> cf_design_model
    copperfin_studio_host --> cf_security
    copperfin_studio_host --> cf_platform_profile
    copperfin_studio_host --> cf_licensing
    copperfin_runtime_host --> cf_xbase_runtime
    copperfin_runtime_host --> cf_sqlite_connector
    copperfin_runtime_host --> cf_security
    copperfin_runtime_host --> cf_platform_profile
    copperfin_runtime_host --> cf_licensing
    copperfin_runtime_host --> cf_localization
    copperfin_build_host --> cf_runtime_pipeline
    copperfin_build_host --> cf_localization
    copperfin_build_host --> copperfin_launcher_guard : stages
    copperfin_launcher_guard --> cf_security
    copperfin_launcher_guard --> cf_localization
    copperfin_launcher_guard --> cf_platform_support
    copperfin_launcher_guard --> cf_package_trust
    Copperfin_VisualStudio --> copperfin_studio_host : design JSON
    Copperfin_VisualStudio --> copperfin_runtime_host : debug protocol
    Copperfin_Studio --> copperfin_studio_host : design JSON
    Copperfin_Studio --> copperfin_runtime_host : debug protocol
```

Reading notes for the ground-truth diagram:

- `cf_design_model` (the "designer" concern) is a *dependency of* `cf_xbase_runtime` (the runtime engine), not the reverse — the runtime needs design-model types (e.g. extracted xAsset methods) to execute `SCX/VCX/MNX/FRX/LBX` startup assets. This inverts the layering implied by the aspirational diagram below.
- `copperfin_studio_host` does **not** link `cf_xbase_runtime` directly; design-time JSON snapshot generation only needs `cf_design_model`.
- The managed layer (`Copperfin.VisualStudio`, `Copperfin.Studio`) never talks to native code by static linking — only through the `vs_launch_contract` JSON protocol and the runtime debug protocol, both surfaced by `cf_design_model`/`cf_xbase_runtime`.
- `cf_platform_support` is now the true root of the graph — `cf_localization` links it, so everything that transitively depends on `cf_localization` picks it up too.
- `cf_licensing` and `cf_package_trust` sit in their own small subgraph, deliberately outside the `cf_localization` catalog requirement: `cf_licensing` is dependency-free for offline unit-testability, and `cf_package_trust` links only `cf_licensing`'s verify-only Ed25519 primitive, never its parsing/status logic.
- `copperfin_launcher_guard` (Windows-only) is a build-time dependency of `copperfin_build_host` (staged into packages), not a link-time one — it implements the pre-launch trust check from `docs/29-package-trust-contract.md`.
- `copperfin_mcp_host` is an installed local stdio executable. Its dedicated
  `cf_mcp_host` library reuses strict platform JSON and DBF-header parsing;
  the executable adds process hardening, the existing `ai.mcp` permission
  check, and content-free audit events but no network or caller-file adapter.
- Source: `docs/28-repository-ontology.md` §2–5, generated by inspecting
  `CMakeLists.txt`/`src/`/`apps/`/`vsix/` directly; refreshed 2026-08-11.

## Aspirational Target-State Class Diagram

This is the `copperfin-*` module taxonomy from `docs/02-architecture.md`. It describes where the architecture is meant to go, not what exists in the build graph above. Treat every class here as a **target**, not a shipped component.

Moved to its own file to keep this page's diagram count to one — see
[diagrams/uml-aspirational.md](diagrams/uml-aspirational.md) for the full
diagram and reading notes.

## Runtime Subsystem UML

This one is ground-truth-adjacent: the class names are illustrative groupings rather than literal type names, but each maps onto a real translation-unit family inside `cf_xbase_runtime`'s `prg_engine.cpp` + `.inl` partials (`_dispatch`, `_flow`, `_expression`, `_records`, `_cursor`, `_arrays`, `_variables`, `_session`, `_sql`, `_aggregate`, `_dll`), per `docs/28-repository-ontology.md` §3.

Moved to its own file for the same reason — see
[diagrams/uml-runtime-subsystem.md](diagrams/uml-runtime-subsystem.md).

## Reading Notes

Ground-truth diagram:

- `cf_xbase_runtime` is the current execution hub; `cf_design_model` is upstream of it (see inversion note above), not the other way around.
- Its in-process CLR capability remains Windows-only, but the interpreter now
  crosses a portable scalar value/result boundary; CLR, COM, Automation, and
  `mscorlib` types are confined to the private Windows implementation.
- Native `DECLARE` remains Windows-only, but module search, decorated export
  resolution, managed-PE classification, and module lifetime now cross a
  portable opaque-identity/result boundary, while typed arguments, return
  values, and copied by-reference updates cross a second portable contract.
  Windows loader APIs remain private to `native_declared_library.cpp`; ABI
  storage, pointer formation, Automation dispatch, and x64 typed calls remain
  private to `native_declared_call.cpp` and `win64_native_call.cpp`.
- `cf_security` and `cf_platform_profile` are siblings consumed identically by `copperfin_studio_host` and `copperfin_runtime_host`.
- Every native library depends on `cf_localization` (and therefore `cf_platform_support`) except the two deliberately independent bases: `cf_licensing` and `cf_platform_support` itself. This reflects the hard localization-catalog requirement — though actual call-site adoption outside these wired libraries is still thin.

Aspirational and Runtime Subsystem reading notes now live alongside their
diagrams in [diagrams/uml-aspirational.md](diagrams/uml-aspirational.md) and
[diagrams/uml-runtime-subsystem.md](diagrams/uml-runtime-subsystem.md).

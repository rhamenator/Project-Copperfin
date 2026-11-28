# es-419 glossary for Builder, toolbox, VFP properties, and debugging

**Status:** Revised AI proposal, 2026-11-03. It incorporates `/tmp/copperfin-es-glossary-independent-review.md`. These examples guide a family-level translation pass; they are not reviewed catalog values. No repository file or review status has changed. Exact source/translation hashes must be regenerated for every final key.

## Protected boundary

Translate human-facing prose and display labels at their actual call sites. Keep placeholders (for example `{command}` and `{builderId}`), CLI options (`--dry-run`, `--tab-stop`, `--ole-drop-mode`), VFP/SQL commands and expressions (`READ EVENTS`, `SPAWN`, `AND`, `SEEK`), property identifiers (`OLEDropMode`, `OLEDropEffects`, `OLEDropTextInsertion`, `TABSTOP`, `KEYPREVIEW`), JSON names (`dryRun`, `contextCount`, `assetPath`), IDs, file names, paths, enum values, and executable names byte exact. Preserve punctuation and whitespace required by each source string. The translated label is separate from the invariant value that follows it. `src/studio/builder_registry.cpp` localizes Builder titles/descriptions but retains IDs, entry points, equivalent program names, and `builder`/`wizard` kinds. `apps/copperfin_studio_host/studio_host_main_toolbox_palette.cpp` localizes text-mode labels while emitting invariant JSON field names.

## Builder and wizard

Use **Builder** as the VFP workflow noun in product UI, **asistente** for wizard, and **diseñador** for designer. Do not use *constructor* for this workflow; reserve it for an actual class constructor. Apply this to tabs, summaries, action descriptions, and diagnostics together.

| English source or intent | Proposed es-419 example |
|---|---|
| `Current Builder Targets:` | `Destinos actuales del Builder:` |
| `Open the context-filtered and VFP-aware list of builders or wizards.` | `Abre la lista de Builders o asistentes, filtrada según el contexto y compatible con VFP.` |
| `A builder dispatch catalog request requires at least one context builder.` | `Para solicitar el catálogo de preparación de un Builder, debe haber al menos un Builder disponible en el contexto.` |

`Builder de formularios` is a display title. `builder.app`, `ReportBuilder.app`, `cf_builders.*`, a Builder ID, and JSON `kind: "builder"` remain literal. **Uncertainty:** the plural `Builders` is defensible in a VFP-facing UI, but its width and familiarity across es-419 regions require in-product inspection.

## Dispatch, admission, and simulated execution

Describe *dispatch* as **preparación para iniciar** when planning a launch and **envío para ejecutar** when handing a plan to an executor. Describe *invocation admission* as **autorización para iniciar** and *execution admission* as **autorización para ejecutar**. A dispatch is a plan/handoff, not proof of execution; preserve that distinction. Use **ejecución simulada** in prose for `dry run`; use the exact token `--dry-run` only when describing the CLI switch. Do not use *admisión de invocación* or a blanket *despacho* without an established product term.

| Exact English source | Proposed es-419 sentence |
|---|---|
| `A builder dispatch request requires an admitted non-dry-run invocation.` | `Para preparar el inicio de un Builder, se requiere una invocación autorizada que no sea una ejecución simulada.` |
| `A builder dispatch execution request requires explicit execution admission.` | `Para ejecutar el inicio preparado de un Builder, se requiere autorización explícita para ejecutar.` |
| `A builder dispatch execution catalog entry requires a non-executed dispatch.` | `Una entrada del catálogo de ejecución de Builders requiere un plan de inicio que aún no se haya ejecutado.` |
| `A selection-context builder invocation admission catalog request requires at least one builder.` | `Para consultar el catálogo de autorización de inicio de Builders según la selección, debe haber al menos un Builder.` |
| `A toolbox dispatch execution request requires a non-executed dispatch.` | `Para ejecutar una acción preparada de la caja de herramientas, se requiere un plan que aún no se haya ejecutado.` |
| CLI help referring to `--dry-run` | `Use --dry-run para simular la ejecución sin iniciar el Builder.` |

The code keeps `command_token`, `entry_point`, `ui_launch_admitted`, `dry_run`, and `executed` distinct (`src/studio/builder_dispatch.cpp`, `src/studio/editor_action_dispatch.cpp`). **Uncertainty:** the natural Spanish distinction between *preparación*, *plan*, and *envío* should be checked against complete diagnostic families and actual CLI flows. If a specific `dispatch` call site means only data routing, adjust that family's noun rather than forcing the launch wording. The Boolean `dry_run` and JSON `dryRun` are invariant; the English phrase “non-dry-run” in a localized diagnostic is not.

## Text-mode toolbox labels

Text-mode labels are deliberately translated. Prefer readable Spanish labels with spaces and accents for human output, such as `contexto de selección`, `ruta del recurso`, `índice de registro`, `cantidad de elementos`, `cantidad de planes de inicio`, `ejecución simulada`, `modifica el recurso`, `texto de búsqueda`, `categoría`, and `ID único`. Example rendered lines (illustrative values):

```text
contexto de selección: formulario
ruta del recurso: formularios/cliente.scx
ejecución simulada: true
cantidad de planes de inicio: 2
ID único: 17
```

These labels are presentation text; the values `true`, numeric values, resource paths, and object IDs retain their machine meaning. JSON field names such as `selectionContext`, `assetPath`, `dryRun`, and `launchPlanCount` stay exact. Existing localized underscore labels such as `ruta_activo` should **not** be renamed piecemeal: text scrapers may depend on them even though repository pseudo-locality tests expect localized labels and do not establish a snake_case byte contract (`tests/test_studio_host_json_parse_diagnostics_toolbox_palette.inl`). **Decision required:** either document text mode as localized human output and treat JSON as the supported parsing format, or explicitly define and test a stable parseable text format. Verify text and JSON in en-US, es-419, and qps-ploc before migrating this label family.

## OLE drop, TabStop, and KeyPreview

Short UI labels should explain behavior without appending property identifiers to every occurrence. Examples:

| Exact English source or property | Proposed es-419 display text |
|---|---|
| `OLE drop-mode` | `modo de soltar con OLE` |
| `OLE drop-effects` | `efectos al soltar con OLE` |
| `OLE drop text-insertion` | `inserción de texto al soltar con OLE` |
| `tab-stop` / `TabStop` Boolean | `recibe el foco con Tab` |
| `key-preview` / `KeyPreview` Boolean | `captura previa de teclas` |
| `No visual objects were selected for tab-stop assignment.` | `No se seleccionó ningún objeto visual para definir si recibe el foco con Tab.` |
| `The same visual object was selected more than once for tab-stop assignment.` | `Se seleccionó el mismo objeto visual más de una vez para definir si recibe el foco con Tab.` |

`TabStop` concerns keyboard focus by Tab, not insertion of a tab character. `KeyPreview` concerns the form receiving key events before the active control, not previewing a keyboard. VFP setters use fixed `TABSTOP`/`KEYPREVIEW` and OLE property names separately from localized labels (`src/vfp/visual_asset_editor_property_setters_behavior.cpp`, `src/vfp/visual_asset_editor_property_setters_appearance.cpp`). When a diagnostic must name the API property, use a sentence such as `La propiedad OLEDropMode no acepta este valor.` rather than appending the identifier to every short label. **Uncertainty:** check the actual VFP control semantics and label width in the UI; the TabStop and KeyPreview meanings follow the control behavior described in Microsoft documentation, applied here by inference.

## Debugger and Rushmore

Use **punto de interrupción**, **ejecución paso a paso**, **expresión vigilada**, **ventanas de expresiones vigiladas**, **pila de llamadas**, and **cobertura**. This matches the existing es-419 catalog's `Expresiones vigiladas` tab and `Evaluando expresión vigilada...` status. Examples:

| English source or intent | Proposed es-419 example |
|---|---|
| `Watch evaluation requires an active paused state.` | `Para evaluar una expresión vigilada, la ejecución debe estar pausada.` |
| `breakpoints, stepping, watch windows, call stack, coverage, task and error surfaces` | `puntos de interrupción, ejecución paso a paso, ventanas de expresiones vigiladas, pila de llamadas, cobertura y vistas de tareas y errores` |
| `high-confidence Rushmore match` | `coincidencia de Rushmore de confianza alta` |
| `moderate-confidence Rushmore match` | `coincidencia de Rushmore de confianza moderada` |
| `possible Rushmore match` | `posible coincidencia de Rushmore` |
| `top-level AND chain` | `cadena de AND de nivel superior` |

`Rushmore`, `AND`, VFP commands, source expressions, and optimizer enum values stay literal. *Confianza* describes a heuristic score, not a measured probability. `src/runtime/index_seek_optimizer.cpp` localizes match reasons independently of the strategy enum. **Uncertainty:** check external consumers for comparison against localized `match_reason` text; consumers should use structured strategy/decision fields instead. Recheck locale-specific optimizer tests if these strings change.

## Before applying catalog corrections

Review each complete term family against its call sites, then write individual exact-hash proposals for the final source/current/proposal values. A separate reviewer should assess those proposals. Run focused localized CLI and JSON tests, pseudo-locality layout tests, and UI inspection for clipping and semantic fit. This AI glossary does not certify native-speaker quality or rendered behavior.

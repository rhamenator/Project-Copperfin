# pt-BR terminology proposal for unresolved catalog entries

**Status:** independent Codex proposal for challenge review; no catalog change or release approval. Evidence was read from full-review checkout `d67afb2f1` on 2026-11-03. A preferred translation below applies to *human-facing prose or labels* unless a field is explicitly classified as a fixed token. Existing draft corrections must still be checked against their exact source/current hashes.

## Builder, designer, and launch stages

| English in context | Preferred pt-BR | Evidence and boundary |
| --- | --- | --- |
| Builder (Form/Class/Control/Grid/Report/Data Environment Builder) | **Construtor**: “Construtor de formulários”, “Construtor de grades”, etc. | `src/studio/builder_registry.cpp:52-81` lists Builder, Wizard and Designer as distinct entries, with VFP-equivalent app names. Current `Studio.Builder.FormBuilder.Title` says “Builder de formulários”; “construtor” is a natural role label. Preserve builder IDs, `--builder-id`, `builder.app`, and exact `Vfp9Equivalent` filenames. |
| Wizard | **Assistente**: “Assistente de etiquetas”, “Assistente de aplicativos”. | Same registry; the current `Studio.Builder.LabelWizard.Title` already uses “Assistente de etiquetas”. |
| Designer | **Designer** in product titles (“Designer de menus”); **editor visual** only in explanatory prose where it describes editing, not a named role. | Same registry and current `Studio.Builder.MenuDesigner.Title`. Avoid collapsing Designer and Builder into a single Portuguese term. |
| Builder invocation admission | **autorização da invocação do construtor**; in an error, “A invocação do construtor precisa ser autorizada.” | `src/studio/builder_invocation_admission.cpp:47-75` sets `ui_launch_admitted` and `dry_run`; this is a gate for launching, not completed execution. “Admissão” sounds like enrollment in pt-BR. |
| Dispatch | **encaminhamento**: “encaminhamento do construtor”, “encaminhar a invocação”. | `src/studio/builder_dispatch.cpp:50-125` checks admission and prepares command arguments. “Despacho” is possible but sounds administrative; this stage routes/prepares an invocation. Preserve the separate later **execução** stage. |
| Execution admission | **autorização explícita da execução**. | `builder_dispatch_execution_readiness_error` requires `admit_execution` and checks the dispatch is admitted, non-dry-run, and not already executed (`src/studio/builder_dispatch.cpp:50-83`). Do not say merely that dispatch was “executed”. |
| Launch plan / dry run | **plano de inicialização** / **simulação** in prose; fixed `dry_run` and CLI flags unchanged. | `src/studio/builder_invocation_admission.cpp:64-75`; `src/studio/builder_dispatch.cpp:112-125`. |

This would change `StudioHost.BuilderParse.RequestName.Dispatch` from “despacho do builder” to “encaminhamento do construtor”, and `...InvocationAdmission` from “admissão de invocação do builder” to “autorização da invocação do construtor”, subject to a whole-key consistency pass across Builder/Designer/Toolbox families. A request-name fragment may need lowercase to fit surrounding sentences. Do not replace `IDispatch` or `Runtime.Prg.Dispatch` program/API names by global search.

## VFP property labels

These are display labels for named VFP properties. Preserve code/CLI names such as `KeyPreview` and `--key-preview` byte for byte. `src/studio/vs_launch_contract_setters_behavior.cpp:444-492,722-744` confirms the relevant assignments are booleans; `src/studio/vs_launch_contract_setters_data.cpp:366-378` confirms `LockColumnsLeft` is a nonnegative integer. `apps/copperfin_studio_host/studio_host_main_shared_cli_infra.cpp:224-233,279` shows several other properties in CLI usage.

| Property | Preferred human label | Confidence / reason |
| --- | --- | --- |
| `KeyPreview` | **tratamento prévio de teclas** | Medium. Avoid “pré-visualização de teclas”, which suggests a visual preview. Confirm against VFP behavior before final approval. |
| `LockColumns` / `LockColumnsLeft` | **bloqueio de colunas** / **número de colunas bloqueadas à esquerda** | High for `LockColumnsLeft` because its CLI value is a count. Avoid imperative “bloquear colunas” for a property label. Check `LockColumns` value type independently before deciding whether it also needs a count noun. |
| `MaxLeft` / `MaxTop` | **posição máxima à esquerda** / **posição máxima no topo** | Medium. They are position bounds, not superlatives describing a left/top object. Verify units and bounds in setter implementation. |
| `TabOrder` / `TabOrientation` | **ordem de tabulação** / **orientação das abas** | Medium. “Tab” is keyboard traversal in one property and page tabs in the other; avoid “guia” for both without control context. |
| `LockScreen` | **bloqueio da atualização da tela** | Medium. Avoid OS-style “bloquear tela”; confirm the VFP redraw behavior at the property call site. |
| `ClipControls` | **recorte de controles** | Medium. It is boolean (`--clip-controls`); “recortar controles” sounds like an editing command. Confirm clipping behavior before approval. |
| `AutoVerbMenu` | **menu automático de verbos OLE** | Medium. Keep “verbo” as the OLE term; distinguish from grammar verbs. Check OLE control context. |
| `IntegralHeight` | **altura por itens inteiros** | Medium. Avoid “altura integral” if the property means showing complete list items; confirm control type. |
| `PictureSelectionDisplay` | **exibição da seleção de imagem** | Low. Existing current wording is close, but the direction of selection is unclear from the setter alone. Do not change it to “imagem selecionada” without runtime/UI evidence. |
| `Sparse` | **Sparse** pending a verified VFP control description, or **exibição esparsa** only if that description confirms the behavior. | Low. “Esparso” by itself could be mistaken for data sparsity. Preserve the property name in code and usage regardless. |
| `ShowWindow`, `WindowState`, `LinkMaster` | **modo de exibição da janela**, **estado da janela**, **vínculo com o registro mestre** (provisional). | Medium/low. CLI usage takes values (`--show-window <n>`, `--window-state <n>`, `--link-master <value>`), so “mostrar janela” is not an action. `LinkMaster` needs VFP table/grid relationship review before release. |

For `StudioHost.LaunchParse.ObjectAssignment.*` and `VisualAssetEditor.PropertyLabel.*`, use one semantic label per property, with sentence capitalization only for `*Title`. Avoid putting translated property names into option flags or serialized object fields.

## Debugger and Rushmore

| Concept | Preferred pt-BR | Evidence and boundary |
| --- | --- | --- |
| Breakpoint | **ponto de interrupção** | Existing `AssetEditor.Debugger.Tab.Breakpoints` and related UI keys already use this. |
| Watch / Watches | **expressão de inspeção** in errors, **Expressões de inspeção** as tab title; “avaliar expressão de inspeção” for evaluation. | `src/runtime/prg_engine.cpp:8806-8836` evaluates an expression only in a paused frame; `AssetEditor.Debugger.WatchExpressionLabel`, tab, and status form a single feature. Existing “expressões observadas” is understandable but makes the action “avaliar expressão observada” cumbersome. A challenge reviewer should select one family-wide term. Keep command/API `watch` unchanged. |
| Frame | **quadro da pilha** in table headers; **quadro de execução pausado** in error prose. | `src/runtime/prg_engine.cpp:8817` uses a paused runtime frame; current `AssetEditor.Debugger.Column.Frame` says “Quadro de pilha”. |
| Step over / step out | **avançar sem entrar na chamada** / **sair da rotina** as descriptions; completion text: “Avanço sem entrar na chamada concluído.” / “Saída da rotina concluída.” | `src/runtime/prg_engine.cpp:10487-10502` has separate `step_over` and `step_out` actions. Existing “passo sobre” and proposed “passo por cima” are literal calques. UI buttons `Next` and `Out` should stay compact (“Avançar” / “Sair”) if action mapping confirms it. |
| Rushmore explain plan | **explicação do plano Rushmore** | `apps/copperfin_studio_host/studio_host_main_rushmore.cpp:556-577` prints an explain result; “plano de explicação” reverses the relationship. Keep “Rushmore” as the feature name. |
| Rushmore fallback reason / fallback display | **motivo do plano alternativo** / **plano alternativo** in display prose. | `apps/copperfin_studio_host/studio_host_main_rushmore.cpp:93-104` parses fixed enum values; lines 547-550 write `fallbackReason` as an enum and `fallbackReasonDisplay` as localized prose. Current “motivo de alternativa Rushmore” is unnatural. Preserve enum strings such as `no_matching_index` and `execution_fallback` exactly. |

## Text output versus fixed machine fields

The catalog currently translates some text-mode labels to `snake_case` Portuguese: e.g. `StudioHost.ToolboxPalette.Text.Label.SelectionContext` → `contexto_selecao`, `...AssetPath` → `caminho_ativo`, while the same output has `status: ok` / `status: error` (`apps/copperfin_studio_host/studio_host_main_toolbox_palette.cpp:445-500`). The Rushmore text mode similarly localizes `status: ok` to `status: sucesso` and `status: error` to `status: erro` (`studio_host_main_rushmore.cpp:556-577`). The latter conflicts with the en-US text-output assertion at `tests/test_studio_host_json_rushmore_explain.cpp:72`; that test alone does not prove an external parser contract for every locale.

**Proposed classification:**

1. **Fixed contracts:** JSON property names, enum values, CLI options/metavariables, VFP property identifiers, product/app filenames, numeric/boolean wire values, and any text fields consumed by code or scripts. Keep exact ASCII spelling and punctuation. Rushmore's JSON `fallbackReason` is fixed while `fallbackReasonDisplay` is localized (`studio_host_main_rushmore.cpp:547-550`). Source `true`/`false`, `{placeholders}`, and angle-bracket CLI metavariables such as `<variable>` must remain exact.
2. **Text-mode status:** provisionally retain `status: ok` and `status: error` in all locales because many tests and scripts search those exact tokens, including `tests/test_runtime_host_debug_output_localization.cpp:498,556,746` and `tests/test_studio_host_json_parse_diagnostics_toolbox_palette.inl:62`. Audit each command before treating it as invariant; add documented contract exceptions where warranted.
3. **Text-mode label keys:** do **not** declare all labels fixed merely because they use `snake_case` in en-US. Determine whether a parser reads each command's text mode. If it does, freeze the exact label (`selection_context`, etc.) and offer a separate localized display label or JSON `*Display` field. If it does not, translate as readable Portuguese labels (e.g. “Contexto de seleção”), rather than inventing a new machine-looking identifier (`contexto_selecao`).
4. **Human prose:** translate sentence text, headings, error explanations, and explicit display fields; preserve embedded machine tokens. `StudioHost.RushmoreExplain.Label.Fallback` may become “Plano alternativo” only after its text-mode consumer audit; JSON `fallbackReasonDisplay` can be localized now.

`docs/localization-invariant-exceptions.json` currently lists only three VFP/application exact-value exceptions. It is not a full registry of text-output contracts. The independent reviewer should challenge especially the suggested freeze of status lines and inspect all locale-specific consumers before catalog edits.

## Remaining decisions before applying the glossary

- Confirm VFP behavior for `KeyPreview`, `IntegralHeight`, `PictureSelectionDisplay`, `Sparse`, `LinkMaster`, and `LockScreen` against runtime/control implementations or authoritative VFP documentation. The local CLI setter establishes type, not all semantics.
- Decide whether the product wants “Construtor” in visible VFP-compatible titles while retaining “Builder” in code/product descriptions. Apply consistently across Builder and Designer families, with exact IDs untouched.
- Inventory parsers of each text-mode command. Reconcile localized `status: sucesso`/`erro` and translated `snake_case` labels against explicit output contracts; add precise invariant entries or separate display fields, rather than a blanket source-identical rule.
- Keep security tier identifiers `bronze`, `silver`, `gold` fixed only if they are parsed or matched as tier names; `src/security/security_model.cpp:85-87` currently loads localized profile descriptions, which alone does not establish that their prefixes are machine tokens. Preserve `RBAC` and other acronyms in descriptive prose.

No proposal here independently approves an existing first-pass `revise` entry. Each catalog revision still needs exact-hash cross-review and the project's release gate.

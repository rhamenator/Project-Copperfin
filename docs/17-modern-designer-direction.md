# Modern Designer Direction

## Goal

Copperfin should not freeze the visual editors in the look and feel of `vfp9.exe`.

The behavior and productivity model should stay compatible.
The editor presentation should move toward modern Visual Studio-style tooling where that improves usability.

## General Rules

- preserve file-format fidelity and authoring semantics
- preserve VFP object models, report models, menu models, and project meaning
- modernize layout, discoverability, navigation, and diagnostics
- share the same core designer engine between the standalone IDE and Visual Studio

## Forms And Classes

Keep:

- object model
- containers and parent/child behavior
- property semantics
- event and method editing
- builder hooks

Modernize:

- designer adorners and resize handles
- searchable toolbox
- searchable property grid
- split designer/code views
- docking and inspectors similar to modern Windows designers in Visual Studio

## Reports And Labels

Keep:

- VFP band model
- expressions, grouping, sorting, and output behavior
- label/report asset round-tripping

Modernize:

- make the canvas feel closer to SSRS and newer Visual Studio report tooling
- use a stronger outline/structure pane for bands and controls
- use live preview panes
- expose grouping/sorting/properties in modern docked panes instead of legacy modal flows
- unify report and label editing visual language

## Menus

Keep:

- menu hierarchy semantics
- prompt/command/setup/cleanup behavior
- generated output expectations

Modernize:

- hierarchical tree editing
- detail panes for command/procedure/message metadata
- keyboard gesture editors closer to Visual Studio command customization concepts

## Projects

Keep:

- project member semantics
- main program and build configuration meaning
- relationships to SCX/VCX/FRX/MNX/PRG assets

Modernize:

- make it feel closer to Solution Explorer plus Project Properties
- use dependency and diagnostics views
- surface generated assets and migration issues in modern tool windows

## Builders, Wizards, And Task Panes

Keep:

- task-oriented guided editing
- scaffold generation
- quick productivity helpers

Modernize:

- use dockable task panes
- use template galleries and property-driven experiences
- use modern search/filtering
- reduce modal dialog dependence

## Standalone IDE vs Visual Studio

Both environments should use the same designer engine and in-memory models.

Visual Studio should feel like:

- the outer shell
- project/debug/source-control host
- modern document/workspace surface

The standalone Copperfin IDE should feel like:

- the dedicated full-fidelity authoring environment
- the place where the richest design workflows mature first

The product should never fork behavior between the two.
Only the shell chrome should differ.

## Host Shell Standards

Host shells are product surfaces, not incidental wrappers around the shared designer.

Windows-facing Copperfin experiences should follow current Microsoft desktop guidance and modern Visual Studio 202x extension conventions.
macOS-facing experiences should follow Apple desktop conventions directly.
Linux-facing experiences should follow Linux desktop expectations, using VS Code as the primary reference point where Copperfin exposes IDE-style workspace, docking, and terminal behaviors.

This means:

- keep shared editor and designer behavior compatible across hosts
- keep host chrome, docking, commands, and window affordances native to the platform
- treat the current shared WinForms shell chrome as temporary, not product-grade

For the command surface specifically:

- the standalone IDE should provide a dockable `Command` window
- the standalone IDE should provide a dockable `Terminal` window comparable to contemporary IDE terminal workflows
- the VSIX should expose `Command` as a bottom-region tabbed tool window by default, alongside terminal/output-style panes in the Visual Studio shell

Future standalone extension support, if it exists, should be treated as a curated host-compatibility problem rather than general VSIX parity.
Do not plan on loading the Copperfin VSIX inside the standalone host.

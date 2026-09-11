// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::vfp {

// #5477 (parent #138, related #5478): read-only structural inspection of
// Access form/report control hierarchy, control types, and basic layout/
// property metadata.
//
// docs/78-access-forms-reports-vba-storage-reconnaissance.md records why
// this does NOT parse raw Jet/ACE container bytes the way #5476/#5539/
// #5549/#5551 do: real-fixture reconnaissance found Forms/Reports/VBA
// Modules are Access *application*-layer objects (the `Forms`/`Reports`/
// `Modules` COM collections), not Jet/ACE database-engine constructs --
// no community documentation (mdbtools, Jackcess) or public spec covers
// their storage inside an `.mdb`/`.accdb` at all.
//
// Instead, this parses the text Access's own `Application.SaveAsText`
// method produces for a form or report -- real, directly observed
// product behavior from a licensed Access 365 installation (per
// docs/07's/docs/66's "observed product behavior from a real, licensed
// install" evidence category, the same standard the CDX writer's real-
// VFP9 verification, docs/77, relied on), not an officially published
// Microsoft specification. `SaveAsText` output is a nested
// `Begin <Type> ... End` text grammar: a `Version`/`VersionRequired`/
// `Checksum` header, then one root block (`Begin Form`/`Begin Report`)
// containing property assignments (`Name =value`, or `Name = NotDefault`
// for an inherited-default marker) and nested child blocks -- a bare
// `Begin`/`End` pair wraps a control's own list of child controls
// (confirmed via two real fixtures: a form with a `CommandButton`
// containing a child `Label`, and a `Section` containing many sibling
// controls), while a property whose value is itself `Begin ... End`
// (`GUID`, `NameMap`, `RecSrcDt` in the fixtures checked) holds an
// opaque binary blob this slice does not decode -- not needed for
// #5477's own acceptance criteria (control hierarchy, control types,
// basic properties), and, per this project's clean-room discipline,
// not guessed at.
//
// Producing this text in the first place (`Application.SaveAsText`,
// invoked via COM automation) requires a licensed Microsoft Access
// installation reachable on a Windows machine -- the first such
// external dependency in any Access-family Copperfin slice. This
// module deliberately does not perform that automation step itself:
// it only parses already-produced text, the same "shell out to an
// external interpreter, then parse its portable output" split this
// codebase already uses for its polyglot Python/.NET/R sidecars. This
// keeps the licensed-Access dependency confined to whatever produces
// the input text, and keeps this parser itself portable and testable
// with hand-built synthetic fixtures (matching #5549/#5551's own
// practice of never committing a real Access-derived fixture or its
// content) rather than requiring Access to be installed to run this
// codebase's own test suite.
//
// String escaping (confirmed via real fixtures, not inferred): a quoted
// string property value backslash-escapes both an embedded literal
// quote (`\"` -> `"`) and an embedded literal backslash (`\\` -> `\`) --
// confirmed respectively via a real combo-box `ColumnInfo`/`BaseInfo`
// property value (Access's own semicolon-delimited row-source encoding,
// itself full of embedded, escaped quotes) and a real `Picture` property
// holding a Windows path (`"C:\\Program Files\\...\\ordproc.gif"`).
//
// Long string values wrap across multiple physical lines with no
// explicit line-continuation marker: the first line reads
// `Name ="<chunk>"` as usual, and each following line that is *itself*
// nothing but another quoted chunk (`"<chunk>"`, no property name or
// `=` before it) is a continuation -- concatenate the unquoted, still-
// escaped chunk text together in order, then unescape the result once,
// as a whole. Confirmed via a real fixture's `BaseInfo` value split
// across three physical lines mid-word. This slice does not assume a
// fixed wrap column width (the observed wrap points do not fall at a
// consistent column) -- it detects a continuation structurally, by a
// line starting with a bare `"` rather than an identifier.
struct AccessDesignProperty {
    std::string name;
    // The property's own value text, quotes already stripped for a
    // string value, with `\"` unescaped to `"` and `\\` unescaped to
    // `\` (see this header's own note on the confirmed escaping
    // convention above) -- empty when `is_blob` is true.
    std::string value;
    bool is_string = false;
    // True when the raw value was the literal `NotDefault` marker
    // (an inherited-default property Access's serializer does not
    // write a concrete value for).
    bool is_not_default = false;
    // True when this property's value was itself a nested
    // `Begin ... End` block (an opaque binary blob, e.g. `GUID`,
    // `NameMap`, `RecSrcDt`) rather than a plain value.
    bool is_blob = false;
    // The blob's own raw inner lines, verbatim, only when `is_blob` is
    // true -- not decoded (see this header's own comment).
    std::vector<std::string> blob_lines;
};

struct AccessDesignControl {
    // e.g. "Form", "Report", "Section", "Label", "CommandButton",
    // "TextBox", "Rectangle", "Image" -- whatever type name follows
    // `Begin` in the source text, verbatim, not restricted to a fixed
    // enum (Access's own control-type vocabulary is not independently
    // enumerated by this slice; new/unfamiliar type names are still
    // recorded rather than rejected).
    std::string control_type;
    std::vector<AccessDesignProperty> properties;
    std::vector<AccessDesignControl> children;

    // Returns the first property named `name` (case-sensitive, matching
    // the source text's own casing), or nullptr if absent.
    [[nodiscard]] const AccessDesignProperty* find_property(std::string_view name) const;
};

struct AccessDesignParseResult {
    bool ok = false;
    std::string error;
    std::int64_t version = 0;
    std::int64_t version_required = 0;
    std::int64_t checksum = 0;
    // control_type is "Form" or "Report" on success -- see
    // parse_access_saveastext_design()'s own scope note.
    AccessDesignControl root;
    // The object's own code-behind module, verbatim, when present --
    // confirmed via two real fixtures (a form and a report) that the
    // structural block's closing `End` is followed by a literal
    // `CodeBehindForm` marker line (used for both forms AND reports --
    // not renamed per object type), four `Attribute VB_...` lines, then
    // the plain VBA source verbatim to end of file. Empty (not an
    // error) when the object has no code-behind at all -- this slice
    // did not observe that specific case in a real fixture, so an
    // object with a `CodeBehindForm` marker but an otherwise-empty
    // module body is treated as `code_behind` being an empty string,
    // not as absent.
    std::string code_behind;
};

// Parses `text` as `Application.SaveAsText`'s output for a single Access
// form or report. Fails closed (rather than returning a partial or
// best-guess structure) on: a missing/malformed `Version`/
// `VersionRequired`/`Checksum` header; an unbalanced `Begin`/`End` block
// (including a blob property's own `Begin ... End` never closing); a
// root block whose type is not exactly `Form` or `Report` (this slice's
// explicit scope, matching #5477's own non-goals -- macros, queries, and
// modules are out of scope here and use different SaveAsText shapes);
// or a completely empty/unparseable input.
[[nodiscard]] AccessDesignParseResult parse_access_saveastext_design(const std::string& text);
[[nodiscard]] AccessDesignParseResult parse_access_saveastext_design_from_file(const std::string& path);

}  // namespace copperfin::vfp

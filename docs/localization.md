# Localization

Copperfin localization covers natural-language user-facing text. It does not translate FoxPro/VFP syntax, parser tokens, runtime identifiers, diagnostic identities, JSON contracts, CLI option names, enum values, schema keys, file formats, or protocol fields.

## Catalog Layout

Portable C++ catalogs live under:

```text
resources/locales/<locale>/strings.json
```

Installed packages copy the same tree to:

```text
share/copperfin/locales/
```

Each `strings.json` file is a UTF-8 JSON object with stable semantic keys and localized string values. Use keys such as `Command.Build`, `Diagnostic.ExpectedTokenBeforeToken`, or `Inspect.Usage`; do not use English prose as lookup keys.

The source-of-truth locale is `en-US`. Placeholder catalogs currently exist for `es-419`, `pt-BR`, and `qps-ploc`. The Spanish and Portuguese catalogs are not production-ready language packs until a separate reviewed language-pack issue says so.

## Locale Selection

C++ surfaces should select locale in this order where practical:

1. Explicit CLI or configuration value, such as `--locale`.
2. `COPPERFIN_LOCALE`.
3. OS/user locale when a surface has a safe portable reader.
4. `en-US`.

Catalog roots resolve in this order where practical:

1. Explicit CLI/config override.
2. `COPPERFIN_LOCALE_DIR`.
3. Installed `share/copperfin/locales`.
4. A path relative to the executable.
5. Developer-tree `resources/locales`.

## Fallback

Fallback is deterministic:

```text
es-MX -> es-MX -> es-419 -> es -> en-US
es-AR -> es-AR -> es-419 -> es -> en-US
pt-BR -> pt-BR -> pt -> en-US
pt-PT -> pt-PT -> pt -> en-US
fr-CA -> fr-CA -> fr -> en-US
unknown -> unknown -> en-US
```

Missing keys fall back through the locale chain. If no catalog provides a key, the lookup returns the stable key rather than blank text.

## Placeholders

Use named placeholders:

```text
Diagnostic.ExpectedTokenBeforeToken = Expected {expectedToken} before {actualToken}.
```

Do not build user-facing sentences by concatenating translated fragments. Preserve placeholder names exactly across catalogs.

FoxPro/VFP tokens passed as placeholder values remain unchanged in every locale:

```text
Expected ENDSCAN before ENDIF.
```

Only the human-facing prose is localizable. `ENDSCAN` and `ENDIF` are language tokens and remain locale-invariant.

## Pseudo-Localization

The pseudo-locale is `qps-ploc`. It decorates and expands localized prose while preserving placeholders so tests can catch hard-coded strings, ASCII assumptions, and layout-length problems.

Pseudo-localization is a test tool, not a production language pack.

## Invariant Contracts

Do not localize:

- FoxPro/VFP keywords, commands, functions, clauses, parser tokens, or system identifiers.
- Diagnostic codes.
- JSON property names.
- Schema names.
- CLI option names.
- Machine-readable enum values.
- Severity values.
- Stable IDs.
- File formats or protocol contracts.
- Parser behavior or runtime semantics.

Diagnostic messages may be localized, but diagnostic identity stays stable:

```json
{
  "code": "CFP1007",
  "severity": "error",
  "message": "Expected ENDSCAN before ENDIF.",
  "expectedToken": "ENDSCAN",
  "actualToken": "ENDIF"
}
```

Only `message` is human-facing prose.

## Adding Strings

When adding new user-facing text:

1. Reuse the localization helper/catalog when one exists.
2. Add a stable semantic key.
3. Keep machine-readable fields invariant.
4. Add or update tests for fallback, placeholder preservation, and default `en-US` output when the surface is user-visible.
5. If a surface cannot yet use the catalog, isolate the string in the smallest helper or constant and add `TODO(localization): move this user-facing string into catalog; see #1779`.

Do not add production non-English translations without a reviewed language-pack issue. Placeholder catalogs and machine-generated translations are not production-ready.

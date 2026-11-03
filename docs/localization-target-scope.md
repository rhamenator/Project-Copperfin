# Global left-to-right localization scope

The repository owner requested every regional variant in the left-to-right,
non-logographic sweep on 2026-11-03. RTL and logographic languages need a
separate UI rendering and crash-validation effort. The current five named
European targets are `de-DE`, `fr-FR`, `it-IT`, `es-ES`, and `pt-PT`.

`localization-targets.json` pins the union of Unicode CLDR's
[`availableLocales.json`](https://github.com/unicode-org/cldr-json/blob/7d2b9cb11cec08f4d333ad73209deb3df0711c3b/cldr-json/cldr-core/availableLocales.json)
and [`defaultContent.json`](https://github.com/unicode-org/cldr-json/blob/7d2b9cb11cec08f4d333ad73209deb3df0711c3b/cldr-json/cldr-core/defaultContent.json).
The latter is essential: it includes default regional tags such as `de-DE`,
`fr-FR`, `it-IT`, and `es-ES`, which the available-locales file omits.
The pinned union contains 928 tags: 825 candidate targets, 82 RTL deferred,
and 21 logographic deferred. [Unicode's Sunuwar chapter](https://unicode.org/versions/Unicode16.0.0/core-spec/chapter-13/)
confirms the two `suz-Sunu` tags use a left-to-right alphabet or abugida, so
they are included in the candidate count. This is
a planning manifest, not the packaged locale inventory or a readiness claim.

The script classification is a first filter based on explicit script subtags
and Babel 2.17 likely-subtags data. Before packaging a target, confirm its
CLDR layout, fallback parent, regional terminology, complete source coverage,
placeholder and machine-token fidelity, independent AI review with disclosed
limits, and actual rendered UI behavior. Use shared language catalogs and
regional overrides so variants do not duplicate every string. `qps-ploc`
decorates strings and adds markers, which is an early layout and Unicode stress
test. Its current transformation does not model long German compound words;
long-label pseudo cases and actual German UI tests remain required.

The deferred RTL and logographic entries require a separate design and UI
validation workstream before they can be offered as production language packs.

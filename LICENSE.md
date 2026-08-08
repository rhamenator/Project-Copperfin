# Project Copperfin License

Copyright © 2026 Richard M. Hamilton.

Project Copperfin is licensed under the **GNU General Public License,
version 3 only** (`GPL-3.0-only`) with the **Copperfin Application, Runtime,
and Toolchain Exception 1.0**, an additional permission under GPLv3 section 7.
The complete operative terms—including the exception—are in
[`LICENSE`](LICENSE).

## Your Programs And Other Works

Merely using Copperfin to inspect, edit, run, interpret, analyze, transform,
translate, modernize, generate, compile, assemble, link, package, test, debug,
deploy, host, maintain, or support an independent program or other work does
**not** place that work under the GPL. You keep the copyright in your work and
may choose its license. This includes proprietary source code, Visual FoxPro
projects and binaries such as `.PRG`, `.APP`, `.EXE`, and `.DLL` files, data,
reports, labels, and ordinary output produced from your input.

The exception expressly permits independent works to link statically or
dynamically with Copperfin, run in the same process or address space, use
plug-in, library, API, ABI, command-line, file, pipe, network, package, and
debug interfaces, and be distributed with or embed Copperfin runtime and
support components. Permitted generated source, object code, executables,
libraries, packages, launchers, stubs, scaffolds, headers, manifests, debug
material, data, and documentation may be distributed under terms chosen by
the independent work's owner, including proprietary terms.

The GPL continues to cover Copperfin itself and modifications based on or
derived from Copperfin source code. Anyone distributing Copperfin or a
modified Copperfin runtime must still provide the Copperfin Corresponding
Source and comply with the GPL for those Copperfin portions. Those obligations
do not extend to an independent application merely because it uses, links
with, communicates with, embeds, or is distributed beside Copperfin under the
exception.

This boundary follows [GPLv3 section 2](https://www.gnu.org/licenses/gpl-3.0.html#section2)
for ordinary output and uses the additional-permission mechanism in
[GPLv3 section 7](https://www.gnu.org/licenses/gpl-3.0.html#section7) to cover
linking, runtime combination, and generated support material. This summary is
not a substitute for the complete GPL and exception text in `LICENSE`.
The evidence and design intent are recorded in
[`docs/33-application-runtime-license-exception.md`](docs/33-application-runtime-license-exception.md).
Machine-readable release metadata identifies the custom, non-SPDX-listed
additional permission through a stable `LicenseRef` without pretending it is a
standard SPDX exception. See
[`docs/contracts/release-license-metadata.json`](docs/contracts/release-license-metadata.json)
and the extracted exception text under [`LICENSES/`](LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt).

The earlier source-available/commercial licensing proposal is inactive. Its
documents are preserved solely as historical planning material under
[`docs/archive/commercial-licensing-2026/`](docs/archive/commercial-licensing-2026/README.md).
Those archived documents do not grant, restrict, or describe the terms of the
current GPL-with-exception release.

Artifact signatures authenticate release provenance and integrity. They do
not change the GPL license, impose product activation, or grant commercial
entitlements.

## Contributor Copyright And Credit

Contributors retain copyright in their contributions. No copyright assignment
to Richard M. Hamilton or Project Copperfin is required. By submitting a
contribution under [`CONTRIBUTING.md`](CONTRIBUTING.md), a contributor licenses
that contribution under GPL-3.0-only with the Copperfin Application, Runtime,
and Toolchain Exception 1.0 to the extent the contributor has authority to do
so.

Git authorship and co-authorship metadata, pull requests, issues, and GitHub's
contributor history are the durable credit record. See
[`CONTRIBUTORS.md`](CONTRIBUTORS.md) for the credit policy. Third-party material
keeps its own copyright and license and must be identified separately.

Contributor licenses already granted under these terms remain effective.
Because contributors retain copyright, Project Copperfin cannot place their
contributions under an incompatible future license without the affected
copyright holders' permission, or without removing or independently rewriting
those contributions. A contributor remains free to license that contributor's
own work separately.

If contributor-owned work remains in a future official proprietary or
GPL-incompatible Copperfin release, the contribution terms also require a
separate written agreement stating that contributor's compensation. Without
such an agreement, the work must retain its existing terms or be removed or
independently rewritten for that release. This is not a present commercial
license or a promise of an undefined share of unrelated revenue.

## Official Releases And Downstream Copies

GPLv3 section 7 permits a downstream distributor to remove an additional
permission from its copy. Therefore, this Exception is guaranteed only for a
copy whose license notices still include it. Official Project Copperfin
artifacts retain the Exception and are identified by the project's signed
release provenance. A signature authenticates an artifact; it does not change
the artifact's license. Users of a downstream fork should inspect that fork's
license notices rather than assume it retained the Exception.

Standard Support Material automatically emitted or copied into Permitted
Output may use the Independent Work owner's chosen terms under paragraph 2(c)
of the Exception. The same material distributed separately, and separately
distributed Copperfin runtimes or components, remain Copperfin Code subject to
the GPL. This is the operative distinction between generated support embedded
in application output and Copperfin itself.

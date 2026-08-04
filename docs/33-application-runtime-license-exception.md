# Application, Runtime, And Toolchain Exception Rationale

Copyright © 2026 Richard M. Hamilton.

This document records the evidence and design intent behind the Copperfin
Application, Runtime, and Toolchain Exception 1.0. The complete operative
license and exception text is `LICENSE`; this rationale does not add or remove
permissions.

## Intended Developer Contract

Copperfin is infrastructure for preserving, running, maintaining, and
modernizing existing business applications. An application must remain under
its owner's chosen terms merely because Copperfin:

- opens, inspects, edits, interprets, executes, or debugs it;
- translates, modernizes, generates, compiles, assembles, links, or packages
  it;
- provides an in-process or separate-process runtime, library, plug-in,
  API/ABI, protocol, launcher, scaffold, manifest, or debug interface; or
- is embedded, bundled, or redistributed as a runtime dependency alongside
  the application.

The exception grants those permissions even when ordinary Copperfin operation
places standard Copperfin support material in generated output. It does not
permit proprietary relicensing of Copperfin itself or modifications derived
from Copperfin source code. GPL compliance and Corresponding Source duties
remain attached to the distributed Copperfin portions.

## VFP9 Offline Evidence

The remounted `vfp9.iso` was inspected read-only. Its `redist.txt`, setup
license, and `dv_foxhelp.chm` topics describe a developer model in which:

- developers design, develop, test, and demonstrate their own programs;
- executable applications remain the developers' programs;
- designated runtime libraries and support components are distributable with
  custom applications; and
- application setup packages may include designated runtime merge modules and
  support files.

Relevant offline help topics include *Distributable and Restricted Visual
FoxPro Features and Files*, *Visual FoxPro Run-Time Libraries*, *Compiling
Source Code*, and *How to: Include Files with Applications for Distribution*.
These shipped materials establish the practical compatibility expectation that
the development platform and redistributable runtime do not take ownership of
the application. Their proprietary license language was not copied into the
Copperfin exception.

## GPL Mechanism

GPLv3 section 2 already limits when ordinary program output is a covered work.
GPLv3 section 7 permits a copyright holder to grant additional permissions.
The Copperfin exception uses section 7 to resolve the boundaries that plain
GPLv3 does not resolve safely for this product: static and dynamic linking,
same-process runtime combination, embedded runtime components, and generated
support material.

The GNU Classpath exception and GCC Runtime Library Exception were reviewed as
established models. Neither was adopted verbatim: the first is centered on a
library-linking relationship, while the second defines a compiler-specific
eligible compilation process. Copperfin needs one coherent permission spanning
legacy execution, modernization tooling, compilation, linking, packaging,
generated support material, and runtime redistribution.

Because the Copperfin exception is project-specific legal text, it should
receive qualified legal review before the first public release candidate. Such
review may refine wording but must preserve the independent-application and
retained-Copperfin-copyleft boundaries above.

* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
*
* Run only in a disposable real VFP9 process. Supply a new, nonexistent
* directory path. The script creates its DBF/CDX/IDX fixtures there and writes
* descending-observation.tsv; it never deletes an existing path.

LPARAMETERS tcOutputRoot

LOCAL lcRoot, lcTable, lcCdx, lcIdx, lcObservation, lcAscendingOverrideCommand, lnTag

IF VARTYPE(tcOutputRoot) <> "C" OR EMPTY(ALLTRIM(tcOutputRoot))
    ? "Usage: DO vfp9_descending_observation WITH <new-output-directory>"
    RETURN .F.
ENDIF

lcRoot = FULLPATH(ALLTRIM(tcOutputRoot))
IF DIRECTORY(lcRoot) OR FILE(lcRoot)
    ? "Refusing to use an existing output path: " + lcRoot
    RETURN .F.
ENDIF

MD (lcRoot)
lcTable = ADDBS(lcRoot) + "descending.dbf"
lcCdx = FORCEEXT(lcTable, "cdx")
lcIdx = ADDBS(lcRoot) + "descending.idx"
lcObservation = ADDBS(lcRoot) + "descending-observation.tsv"

* This is intentionally a disposable-process fixture: it closes only the
* fixture work areas before creating the controlled table and indexes.
CLOSE DATABASES ALL
CREATE TABLE (lcTable) (record_id I, name C(20))
APPEND BLANK
REPLACE record_id WITH 1, name WITH "ALPHA"
APPEND BLANK
REPLACE record_id WITH 2, name WITH "BRAVO"
APPEND BLANK
REPLACE record_id WITH 3, name WITH "CHARLIE"

INDEX ON name TAG AscTag
INDEX ON name TAG DescTag DESCENDING
INDEX ON record_id TO (lcIdx)

=STRTOFILE("case" + CHR(9) + "descending" + CHR(9) + "tag" + CHR(9) + "order" + CHR(9) + "outcome" + CHR(13) + CHR(10), lcObservation)

SET ORDER TO 0
=WriteDescendingObservation(lcObservation, "no-active-order", DESCENDING(), "", ORDER())

SET ORDER TO TAG AscTag
=WriteDescendingObservation(lcObservation, "active-ascending-tag", DESCENDING(), TAG(), ORDER())

SET ORDER TO TAG DescTag
=WriteDescendingObservation(lcObservation, "active-persisted-descending-tag", DESCENDING(), TAG(), ORDER())

FOR lnTag = 1 TO TAGCOUNT()
    =WriteDescendingObservation(lcObservation, "persisted-tag-" + ALLTRIM(STR(lnTag)), ;
        DESCENDING(lcCdx, lnTag), TAG(lnTag), "")
ENDFOR

SET ORDER TO TAG AscTag DESCENDING
=WriteDescendingObservation(lcObservation, "active-ascending-tag-runtime-descending", DESCENDING(), TAG(), ORDER())

* The shipped help documents DESCENDING runtime overrides but not an ASCENDING
* inverse override. Probe it through a runtime macro so an unsupported command
* is retained as an observation instead of aborting collection or inventing a
* direction value.
lcAscendingOverrideCommand = "SET ORDER TO TAG DescTag ASCENDING"
TRY
    &lcAscendingOverrideCommand
    =WriteDescendingObservation(lcObservation, "active-persisted-descending-tag-runtime-ascending", DESCENDING(), TAG(), ORDER())
CATCH TO loException
    =WriteDescendingObservation(lcObservation, "active-persisted-descending-tag-runtime-ascending", "?", "", "", "unavailable:" + ALLTRIM(STR(loException.ErrorNo)))
ENDTRY

USE
USE (lcTable) EXCLUSIVE
SET INDEX TO (lcIdx) ORDER (lcIdx) DESCENDING
=WriteDescendingObservation(lcObservation, "active-idx-runtime-descending", DESCENDING(), "", ORDER())

USE
? "Observation complete: " + lcObservation
RETURN .T.

PROCEDURE WriteDescendingObservation
LPARAMETERS tcFile, tcCase, tlDescending, tcTag, tcOrder, tcOutcome

LOCAL lcValue
IF PCOUNT() < 6
    tcOutcome = "observed"
ENDIF

DO CASE
CASE VARTYPE(tlDescending) = "L"
    lcValue = IIF(tlDescending, "T", "F")
CASE VARTYPE(tlDescending) = "C"
    lcValue = tlDescending
OTHERWISE
    lcValue = "?"
ENDCASE

RETURN STRTOFILE(tcCase + CHR(9) + lcValue + CHR(9) + tcTag + CHR(9) + tcOrder + CHR(9) + tcOutcome + CHR(13) + CHR(10), tcFile, 1)

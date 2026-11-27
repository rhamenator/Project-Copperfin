* Run inside Visual FoxPro 9 SP2 on a disposable working directory.
* Produces a three-row structural CDX with ascending and descending tags.
* Writes native order/navigation observations to result.txt.
LOCAL lcRoot, lcResult
lcRoot = ADDBS(GETENV("USERPROFILE")) + "Documents\CopperfinIndexOrderProbe"
lcResult = lcRoot + "\result.txt"
MD (lcRoot)
SET DEFAULT TO (lcRoot)
SET SAFETY OFF
CREATE TABLE indexed_demo (id I, name C(12))
INSERT INTO indexed_demo VALUES (1, "ALPHA")
INSERT INTO indexed_demo VALUES (2, "CHARLIE")
INSERT INTO indexed_demo VALUES (3, "BRAVO")
INDEX ON name TAG ASCTAG
INDEX ON name TAG DESCTAG DESCENDING
STRTOFILE("created=" + TTOC(DATETIME(), 1) + CHR(13) + CHR(10), lcResult)
TRY
    SET ORDER TO TAG DESCTAG
    STRTOFILE("descending_order=" + ORDER() + CHR(13) + CHR(10), lcResult, 1)
    GO TOP
    SCAN
        STRTOFILE("scan=" + ALLTRIM(name) + CHR(13) + CHR(10), lcResult, 1)
    ENDSCAN
    GO TOP
    STRTOFILE("seek_charlie=" + IIF(SEEK("CHARLIE"), ALLTRIM(name), "<miss>") + CHR(13) + CHR(10), lcResult, 1)
CATCH TO loError
    STRTOFILE("descending_error=" + TRANSFORM(loError.ErrorNo) + ":" + loError.Message + CHR(13) + CHR(10), lcResult, 1)
ENDTRY
TRY
    SET ORDER TO TAG ASCTAG
    STRTOFILE("ascending_order=" + ORDER() + CHR(13) + CHR(10), lcResult, 1)
    GO TOP
    STRTOFILE("ascending_first=" + ALLTRIM(name) + CHR(13) + CHR(10), lcResult, 1)
CATCH TO loError
    STRTOFILE("ascending_error=" + TRANSFORM(loError.ErrorNo) + ":" + loError.Message + CHR(13) + CHR(10), lcResult, 1)
ENDTRY
USE IN indexed_demo

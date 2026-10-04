? "VERSION=" + VERSION()
LOCAL laWidths[15], lnI, lnWidth, lcValue
laWidths[1] = 100
laWidths[2] = 200
laWidths[3] = 236
laWidths[4] = 237
laWidths[5] = 238
laWidths[6] = 254
laWidths[7] = 255
laWidths[8] = 256
laWidths[9] = 511
laWidths[10] = 512
laWidths[11] = 999
laWidths[12] = 1000
laWidths[13] = 32767
laWidths[14] = 32768
laWidths[15] = 65535

FOR lnI = 1 TO ALEN(laWidths)
  lnWidth = laWidths[lnI]
  TRY
    lcValue = STR(1.5, lnWidth, 2)
    ? "WIDTH=" + TRANSFORM(lnWidth) + " LEN=" + TRANSFORM(LEN(lcValue))
  CATCH TO loError
    ? "WIDTH=" + TRANSFORM(lnWidth) + " ERR=" + TRANSFORM(loError.ErrorNo)
  ENDTRY
ENDFOR

LOCAL laDecimalEdges[2]
laDecimalEdges[1] = 18
laDecimalEdges[2] = 19
FOR lnI = 1 TO ALEN(laDecimalEdges)
  TRY
    lcValue = STR(1.5, 25, laDecimalEdges[lnI])
    ? "DECEDGE=" + TRANSFORM(laDecimalEdges[lnI]) + " LEN=" + TRANSFORM(LEN(lcValue)) + " VALUE=[" + lcValue + "]"
  CATCH TO loError
    ? "DECEDGE=" + TRANSFORM(laDecimalEdges[lnI]) + " ERR=" + TRANSFORM(loError.ErrorNo)
  ENDTRY
ENDFOR

LOCAL lnPositiveInfinity, lnNegativeInfinity
lnPositiveInfinity = EXP(1000)
lnNegativeInfinity = -lnPositiveInfinity
TRY
  lcValue = STR(1.5, 10, lnPositiveInfinity)
  ? "DECPOSINF LEN=" + TRANSFORM(LEN(lcValue))
CATCH TO loError
  ? "DECPOSINF ERR=" + TRANSFORM(loError.ErrorNo)
ENDTRY
TRY
  lcValue = STR(1.5, 10, lnNegativeInfinity)
  ? "DECNEGINF LEN=" + TRANSFORM(LEN(lcValue)) + " VALUE=[" + lcValue + "]"
CATCH TO loError
  ? "DECNEGINF ERR=" + TRANSFORM(loError.ErrorNo)
ENDTRY
TRY
  lcValue = STR(1.5, lnPositiveInfinity, 2)
  ? "WIDTHPOSINF LEN=" + TRANSFORM(LEN(lcValue))
CATCH TO loError
  ? "WIDTHPOSINF ERR=" + TRANSFORM(loError.ErrorNo)
ENDTRY
TRY
  lcValue = STR(1.5, lnNegativeInfinity, 2)
  ? "WIDTHNEGINF LEN=" + TRANSFORM(LEN(lcValue))
CATCH TO loError
  ? "WIDTHNEGINF ERR=" + TRANSFORM(loError.ErrorNo)
ENDTRY

LOCAL laPositiveBounds[3]
laPositiveBounds[1] = 4294967295
laPositiveBounds[2] = 4294967296
laPositiveBounds[3] = 4294967297
FOR lnI = 1 TO ALEN(laPositiveBounds)
  TRY
    lcValue = STR(1.5, 10, laPositiveBounds[lnI])
    ? "DECPOS=" + TRANSFORM(laPositiveBounds[lnI]) + " LEN=" + TRANSFORM(LEN(lcValue))
  CATCH TO loError
    ? "DECPOS=" + TRANSFORM(laPositiveBounds[lnI]) + " ERR=" + TRANSFORM(loError.ErrorNo)
  ENDTRY
  TRY
    lcValue = STR(1.5, laPositiveBounds[lnI], 2)
    ? "WIDTHPOS=" + TRANSFORM(laPositiveBounds[lnI]) + " LEN=" + TRANSFORM(LEN(lcValue))
  CATCH TO loError
    ? "WIDTHPOS=" + TRANSFORM(laPositiveBounds[lnI]) + " ERR=" + TRANSFORM(loError.ErrorNo)
  ENDTRY
ENDFOR

LOCAL laNegativeBounds[6]
laNegativeBounds[1] = -2147483647
laNegativeBounds[2] = -2147483648
laNegativeBounds[3] = -2147483649
laNegativeBounds[4] = -4294967295
laNegativeBounds[5] = -4294967296
laNegativeBounds[6] = -4294967297
FOR lnI = 1 TO ALEN(laNegativeBounds)
  TRY
    lcValue = STR(1.5, 10, laNegativeBounds[lnI])
    ? "DECBOUND=" + TRANSFORM(laNegativeBounds[lnI]) + " LEN=" + TRANSFORM(LEN(lcValue))
  CATCH TO loError
    ? "DECBOUND=" + TRANSFORM(laNegativeBounds[lnI]) + " ERR=" + TRANSFORM(loError.ErrorNo)
  ENDTRY
  TRY
    lcValue = STR(1.5, laNegativeBounds[lnI], 2)
    ? "WIDTHBOUND=" + TRANSFORM(laNegativeBounds[lnI]) + " LEN=" + TRANSFORM(LEN(lcValue))
  CATCH TO loError
    ? "WIDTHBOUND=" + TRANSFORM(laNegativeBounds[lnI]) + " ERR=" + TRANSFORM(loError.ErrorNo)
  ENDTRY
ENDFOR

LOCAL laDecimals[5]
laDecimals[1] = -0.5
laDecimals[2] = -0.9
laDecimals[3] = -1
laDecimals[4] = -1.1
laDecimals[5] = -1E20
FOR lnI = 1 TO ALEN(laDecimals)
  TRY
    lcValue = STR(1.5, 10, laDecimals[lnI])
    ? "DEC=" + TRANSFORM(laDecimals[lnI]) + " LEN=" + TRANSFORM(LEN(lcValue)) + " VALUE=[" + lcValue + "]"
  CATCH TO loError
    ? "DEC=" + TRANSFORM(laDecimals[lnI]) + " ERR=" + TRANSFORM(loError.ErrorNo)
  ENDTRY
ENDFOR

LOCAL lcRoot, lcFile, lcDir
lcRoot = "Z:\home\rich\temp\vfp9-probes\file-directory-flags-5611"
lcFile = lcRoot + "\hidden.txt"
lcDir = lcRoot + "\hidden_dir"
IF !DIRECTORY(lcRoot, 1)
    MD (lcRoot)
ENDIF
IF !DIRECTORY(lcDir, 1)
    MD (lcDir)
ENDIF
= STRTOFILE("hidden", lcFile, 0)
DECLARE INTEGER SetFileAttributesA IN kernel32 STRING lpFileName, INTEGER dwFileAttributes
? "file_attr=" + TRANSFORM(SetFileAttributesA(lcFile, 2))
? "dir_attr=" + TRANSFORM(SetFileAttributesA(lcDir, 2))
? "file_0=" + TRANSFORM(FILE(lcFile, 0))
? "file_1=" + TRANSFORM(FILE(lcFile, 1))
? "file_0_9=" + TRANSFORM(FILE(lcFile, 0.9))
? "file_1_1=" + TRANSFORM(FILE(lcFile, 1.1))
? "file_1_9=" + TRANSFORM(FILE(lcFile, 1.9))
? "file_neg1=" + TRANSFORM(FILE(lcFile, -1))
? "file_huge=" + TRANSFORM(FILE(lcFile, 1E300))
? "file_neg_huge=" + TRANSFORM(FILE(lcFile, -1E300))
? "dir_0=" + TRANSFORM(DIRECTORY(lcDir, 0))
? "dir_1=" + TRANSFORM(DIRECTORY(lcDir, 1))
? "dir_0_9=" + TRANSFORM(DIRECTORY(lcDir, 0.9))
? "dir_1_1=" + TRANSFORM(DIRECTORY(lcDir, 1.1))
? "dir_1_9=" + TRANSFORM(DIRECTORY(lcDir, 1.9))
? "dir_neg1=" + TRANSFORM(DIRECTORY(lcDir, -1))
? "dir_huge=" + TRANSFORM(DIRECTORY(lcDir, 1E300))
? "dir_neg_huge=" + TRANSFORM(DIRECTORY(lcDir, -1E300))

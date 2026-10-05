LOCAL lcOut, a1, a2, b1, b2, c1, c2, d1, d2
a1 = RAND($4294967295.0000)
a2 = RAND()
b1 = RAND($4294967295.0001)
b2 = RAND()
c1 = RAND(4294967295)
c2 = RAND()
d1 = RAND(4294967295.9)
d2 = RAND()
lcOut = "currency_equal=" + TRANSFORM(a1 == b1 AND a2 == b2) + CHR(13) + CHR(10) + ;
    "numeric_equal=" + TRANSFORM(c1 == d1 AND c2 == d2) + CHR(13) + CHR(10) + ;
    "currency_max=" + STR(a1, 20, 15) + "," + STR(a2, 20, 15) + CHR(13) + CHR(10) + ;
    "currency_fraction=" + STR(b1, 20, 15) + "," + STR(b2, 20, 15) + CHR(13) + CHR(10) + ;
    "numeric_max=" + STR(c1, 20, 15) + "," + STR(c2, 20, 15) + CHR(13) + CHR(10) + ;
    "numeric_fraction=" + STR(d1, 20, 15) + "," + STR(d2, 20, 15) + CHR(13) + CHR(10)
= STRTOFILE(lcOut, "Z:\home\rich\temp\vfp9-probes\rand-fractional-ceiling-review-6942\result.txt", 0)
QUIT

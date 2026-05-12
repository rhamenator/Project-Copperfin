CREATE CURSOR test_cursor (id I)
INSERT INTO test_cursor VALUES (1)
INSERT INTO test_cursor VALUES (2)
GO TOP
x = -1
ON ERROR DO my_error_handler
? LOG(x)
PROCEDURE my_error_handler
    x = 1
    RETRY
ENDPROC

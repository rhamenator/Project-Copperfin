Queued driver98493 exited6 after compile0/scope executable status1/full145-line
raw: all144 cases ran, with24 RowSourceType2 setup failures (completed=0),
not missing argument/status2/empty output. Initial assessment was wrong and
is explicitly superseded here; its original draft is retained separately.
The driver supplied a RELATIVE scratch root to the unchanged scope source.
A fresh repetition80383 reproduced the same24 setup failures/status1; full
raws/statuses/source/driver/hash checks retained as attempt2. Running the SAME
compiled executable and literal inputs with an ABSOLUTE owned scratch root
returns0/DONE144/scaffold_failures0/fullrawcmp0 against unchanged baseline.
Only the NON-FROZEN driver argument changes to an absolute path, no source,
assertion/helper/frozen/production repair or refreeze. Do not infer a deeper
runtime path-handling cause from this comparison or hide either failed run.

Queued driver98493 exited6 after compile0/scope executable status2/EMPTY raw.
The scope source returns2 before any cases unless argc==2. Its required owned
script-root argument was missing in the already-running driver invocation.
The driver had been launched before its argument-array correction was applied;
editing that queued running shell file did not update the in-flight invocation.
The current-driver snapshot retained here is the corrected file on disk, NOT
a claim it was the exact initial executed file. Initial source is available
in the owning chat's tool record. No scope cases ran, no test failure assertion,
production/helper/frozen edit or refreeze. Preserve status/raw/hash/source
and all34 prior input checks. Restart the corrected NON-FROZEN driver afresh,
with explicit script-root argument; do not use stale status6 as completion.

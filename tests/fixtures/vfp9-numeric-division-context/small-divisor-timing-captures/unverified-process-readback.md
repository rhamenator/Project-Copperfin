REA ev_fd4b732a00f0417c8dff284784201267982a63ece6f5e3d19b008c6098513f39
reports root0/quiesced, owned process-group cleanup verified and one sampled
process whose ownership could not be checked because /proc/746418/environ
returned EACCES. No attempt to read environment/credentials or kill it followed.

Immediate read-only `ps -p 746418 -o pid,ppid,user,stat,comm,args` returned:

```text
    PID    PPID USER     STAT COMMAND         COMMAND
 746418       2 root     I    kworker/u256:2+ [kworker/u256:2+gfx]
```

This visible command/parent is consistent with a kernel worker, not the VFP
probe; it does not retroactively establish REA ownership or an atomic identity
binding. A separate exact-name Wine/VFP process query returned no processes.
Inherited environment, sampling and before/after snapshot limitations remain.

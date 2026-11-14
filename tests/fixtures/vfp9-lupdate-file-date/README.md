# Installed VFP9 `LUPDATE()` file-date probe

This retained black-box probe was run on 2026-11-14 in the authorized
`copperfin-vfp9-win11` VM against installed Visual FoxPro 9 SP2. The seven DBF
fixtures came from the issue #7265 discovery evidence. They cover a valid DBF
header date, an all-zero date, month 13, day 32, an invalid leap day, and the
other malformed variants recorded in `result.txt`.

`probe.ps1` sets every fixture's Windows last-write time to
`2020-01-02T03:04:05`, opens it read-only with `NOUPDATE`, and evaluates the
type, emptiness, scalar components, and formatted value of `LUPDATE()`. Every
fixture returned the same non-empty Date for 2020-01-02, independent of bytes
1..3 in its DBF header. A control run before changing the filesystem timestamp
returned the file's then-current last-write date while its header remained
`1A 02 03` (2026-02-03).

This evidence establishes the date source and malformed-header behavior. It
does not claim sub-day precision (the VFP function returns a Date), behavior
for a timestamp outside the Windows/VFP supported range, or a timezone other
than the VM's configured local timezone.

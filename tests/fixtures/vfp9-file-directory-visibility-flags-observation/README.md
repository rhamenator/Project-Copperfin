# VFP9 FILE()/DIRECTORY() visibility-flag observation

`visibility-flags.prg` is the complete installed-VFP9 SP2 probe source and
`visibility-flags.out` is its complete output. The probe marks one file and one
directory Hidden through the Win32 attribute API, then evaluates the optional
visibility flag across documented values, ordinary fractions, a negative
value, and large finite magnitudes.

The result shows that VFP9 treats zero as false and every tested nonzero finite
value as true; it does not round the flag to an integer or require exactly 1.

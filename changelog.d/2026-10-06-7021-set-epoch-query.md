- 2026-10-06: Preserve SET('EPOCH')'s selected year instead of normalizing year
  1 to ON and silently resetting its two-digit-year consumers to 1950. Retain
  display independence, mode/session/reset regressions, and the owner-directed
  numeric-conversion and CENTURY/ROLLOVER follow-on slices (#7021/#3698).

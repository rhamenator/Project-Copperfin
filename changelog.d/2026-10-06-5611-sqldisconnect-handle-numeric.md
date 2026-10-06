- 2026-10-06: Record SQLSETPROP PR #7000 merge and check SQLDISCONNECT Numeric/exact-integer handles
  before connection removal; default mode requires finite signed-int32
  conversion after truncation, while explicit VFP9 retains negative-only
  low-32/huge-zero aliases. Rejection raises localized catchable 1466 with
  safe original Numeric text. Retained native evidence, direct boundaries,
  fresh-session removal/preservation rows and sanitizer verification cover
  the bounded conversion (#5611/#6776); disconnect-all/absent/type gaps remain
  separately reported as #7001, without implementation admission.

$v = New-Object -ComObject VisualFoxPro.Application
function E([string]$expr) {
  try {
    $t = $v.Eval("VARTYPE($expr)")
    $r = $v.Eval("TRANSFORM($expr)")
    "{0}|TYPE={1}|RESULT={2}" -f $expr, $t, $r
  } catch {
    "{0}|ERROR={1}" -f $expr, (($_.Exception.Message.Trim() -replace "\s+", " ") -replace '^(\d+).*', '$1')
  }
}
try {
  foreach ($expr in "BITAND(7)", "BITOR(7)", "BITXOR(7)", "BITAND(7.9,3.9,1.9)", "BITOR(7.9,3.9,1.9)", "BITXOR(7.9,3.9,1.9)") {
    E $expr
  }
} finally {
  $v.Quit()
}

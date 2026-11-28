$ErrorActionPreference = 'Stop'
$root = 'C:\Users\claude\cf_lupdate_probe_20261114'
$fox = New-Object -ComObject VisualFoxPro.Application
$fox.Visible = $false
try {
  $fox.DoCmd('SET SAFETY OFF')
  $fox.DoCmd('SET DATE TO MDY')
  $fox.DoCmd('SET CENTURY ON')
  foreach ($file in Get-ChildItem $root -Filter '*.dbf' | Sort-Object Name) {
    [IO.File]::SetLastWriteTime($file.FullName, [datetime]'2020-01-02T03:04:05')
    $path = $file.FullName.Replace("'", "''")
    try {
      $fox.DoCmd("USE '$path' IN 0 ALIAS t NOUPDATE")
      $parts = @(
        "TYPE=" + $fox.Eval("VARTYPE(LUPDATE('t'))"),
        "EMPTY=" + $fox.Eval("EMPTY(LUPDATE('t'))"),
        "TEXT=" + $fox.Eval("TRANSFORM(LUPDATE('t'))"),
        "YEAR=" + $fox.Eval("YEAR(LUPDATE('t'))"),
        "MONTH=" + $fox.Eval("MONTH(LUPDATE('t'))"),
        "DAY=" + $fox.Eval("DAY(LUPDATE('t'))"),
        "DTOC=" + $fox.Eval("DTOC(LUPDATE('t'))")
      )
      Write-Output ($file.Name + ' OK ' + ($parts -join ' '))
    } catch {
      Write-Output ($file.Name + ' ERR ' + ($_.Exception.Message.Trim() -replace '\s+', ' '))
    } finally {
      try { $fox.DoCmd('USE IN SELECT("t")') } catch {}
    }
  }
} finally {
  $fox.Quit()
}

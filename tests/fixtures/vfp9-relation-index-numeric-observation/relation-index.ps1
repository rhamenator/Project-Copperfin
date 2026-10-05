$ErrorActionPreference = 'Stop'
$vfp = New-Object -ComObject VisualFoxPro.Application
try {
    $vfp.Visible = $false
    Write-Output ('VERSION|' + $vfp.Eval('VERSION()'))
    $vfp.DoCmd('PUBLIC cProbeScript, nProbeError, cProbeResult, oProbeError')
    foreach ($command in @(
        'CREATE CURSOR ProbeParent (id I)', 'APPEND BLANK', 'REPLACE id WITH 1',
        'CREATE CURSOR ProbeChildA (id I)', 'APPEND BLANK',
        'CREATE CURSOR ProbeChildB (id I)', 'APPEND BLANK',
        'SELECT ProbeParent', 'SET RELATION TO id INTO ProbeChildA',
        'SET RELATION TO id + 1 INTO ProbeChildB ADDITIVE')) {
        $vfp.DoCmd($command)
    }
    $values = @('1', '2', '0', '3', '-1', '0.49', '0.5', '0.9',
        '1.49', '1.5', '1.9', '2.49', '2.5', '2.9', '-1.9',
        '2147483647', '2147483648', '-2147483648', '-2147483649',
        '4294967297', '4294967298', '-4294967295', '1E300', '-1E300',
        'EXP(1000)', '-EXP(1000)')
    $values += @('-4294967294', '9998.9', '9999', '9999.49', '9999.9',
        '10000', '-4294957297', '-4294957296')
    foreach ($function in @('RELATION', 'TARGET')) {
        foreach ($value in $values) {
            $expression = $function + '(' + $value + ')'
            $command = "nProbeError = 0`ncProbeResult = ''`nTRY`ncProbeResult = " +
                $expression + "`nCATCH TO oProbeError`nnProbeError = oProbeError.ErrorNo`nENDTRY"
            $vfp.SetVar('cProbeScript', $command)
            $vfp.DoCmd('=EXECSCRIPT(m.cProbeScript)')
            $errorNumber = [int]$vfp.Eval('nProbeError')
            $tag = if ($errorNumber -eq 0) {
                'C:' + $vfp.Eval('cProbeResult')
            } else { 'ERR' + $errorNumber }
            Write-Output ($expression + '|' + $tag)
        }
    }
} finally {
    $vfp.Quit()
    [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($vfp)
}

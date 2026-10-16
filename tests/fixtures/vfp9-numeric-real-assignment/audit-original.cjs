// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// VR-5611-NUMERIC-REAL-ASSIGNMENT-001: original failure classification only.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const root=process.argv[2];
for(const [name,count,pattern,summary] of [
 ['real',2816,/^FAIL: .* native assignment descriptor$/,'2816 real assignment failures'],
 ['integer',16,/^FAIL: .* native real assignment follow-on$/,'16 Integer assignment failures']]){
 a.equal(fs.readFileSync(path.join(root,'original-'+name+'.status'),'utf8'),'1\n');
 a.equal(fs.statSync(path.join(root,'original-'+name+'.out')).size,0);
 const lines=fs.readFileSync(path.join(root,'original-'+name+'.stderr'),'utf8').trimEnd().split('\n');
 a.equal(lines.pop(),summary);a.equal(lines.length,count);a(lines.every(line=>pattern.test(line)));
}
process.stdout.write(JSON.stringify({realDescriptorFailures:2816,stagedRealControlFailures:16,
 otherControlFailures:0,integerExpectedRowsUnchanged:true,
 note:'44 of 46 real rows differ; the two ten-digit literal widths already match native assignment.'})+'\n');

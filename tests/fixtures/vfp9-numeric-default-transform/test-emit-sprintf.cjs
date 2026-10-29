// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: source generator checks only.
'use strict';
const cp=require('node:child_process'),fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const root=__dirname,run=mode=>cp.execFileSync(process.execPath,[path.join(root,'emit-sprintf.cjs'),mode]),source=run('source').toString(),inv=JSON.parse(run('cases'));
a.equal(inv.inputCount,264);a.equal(inv.recordsPerRun,1056);a.equal(inv.fieldsPerRecord,18);a.deepEqual(inv.counts,[15,16,17,18]);a.equal(inv.vfpControlComparisons,2112);
a.equal((source.match(/^inputHex\[/gm)||[]).length,264);a.equal((source.match(/^targetDecimals\[/gm)||[]).length,264);
for(const token of ['SET FIXED OFF','FOR conversionCount=15 TO 18','beforeStr=HexBytes(STR(','crtResult=CFNSPRTF(','afterStr=HexBytes(STR(','DONE|'])a(source.includes(token),token);
a.equal(source.includes('expectedStrHex'),false);a.equal(source.includes('SET FIXED ON'),false);
a.equal(source.split("line=ALLTRIM(STR(index,6,0))").length,2);
const frozenSource=fs.readFileSync(path.join(root,'sprintf.prg')),frozenCases=fs.readFileSync(path.join(root,'sprintf-cases.json'));
a(frozenSource.equals(run('source')));a(frozenCases.equals(run('cases')));
process.stdout.write('emit-sprintf exact checks=18 PASS (source only)\n');

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: SOURCE checks only, no C compiler,
// CRT, test binary or native execution.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),c=require('node:crypto');
const read=n=>fs.readFileSync(path.join(__dirname,n),'utf8'),base=read('crt-probe.c'),probe=read('gcvt-probe.c');
const gcvtBlock=/\/\* BEGIN first-party guarded _gcvt handler\.[\s\S]+?\/\* END first-party guarded _gcvt handler\. \*\/\n/g;
const sprintfBlock=/\/\* BEGIN first-party guarded public sprintf\/_snprintf comparison handler\.[\s\S]+?\/\* END first-party guarded public sprintf\/_snprintf comparison handler\. \*\/\n/g;
a.equal([...probe.matchAll(gcvtBlock)].length,1);a.equal([...probe.matchAll(sprintfBlock)].length,1);
const restored=probe.replace('#include "gcvt-boundary.h"\n','').replace('#include "sprintf-boundary.h"\n','')
  .replace(gcvtBlock,'').replace(sprintfBlock,'')
  .replace('    {"CFNBIND",binding_diagnostic,0,""},\n    {"CFNGCVT",general_conversion,2,"NN"},\n    {"CFNSPRTF",sprintf_conversion,2,"NN"}\n','    {"CFNBIND",binding_diagnostic,0,""}\n')
  .replace('PublicTable _FoxTable={0,5,functions}','PublicTable _FoxTable={0,3,functions}');
a.equal(restored,base);a(probe.includes('GcvtFunction function=(GcvtFunction)GetProcAddress(module,"_gcvt")'));
const header=read('gcvt-boundary.h');a(header.includes('GCVT_CAPACITY=2048'));a(header.includes('GCVT_CAPACITY>310+1+1074+1+1'));
a(header.includes('if(returned!=buffer->value) return GCVT_POINTER'));
for(const n of ['crt-probe.c','gcvt-probe.c','gcvt-boundary.h','sprintf-boundary.h','gcvt-boundary-check.c'])a(read(n).startsWith('/* Copyright'));
console.log(JSON.stringify({sourceOnly:true,unchangedIdentityAndOldControls:true,capacity:2048,conservativeFixedDecimalBound:1387,reportTextMax:127,guardBytesPerSide:16,allowedCounts:[15,16,17],plannedDirectChecksPerCompiler:115,compiled:false,nativeInvoked:false,sha256:Object.fromEntries(['gcvt-probe.c','gcvt-boundary.h','sprintf-boundary.h','gcvt-boundary-check.c'].map(n=>[n,c.createHash('sha256').update(read(n)).digest('hex')]))},null,2));

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent original-failure classification; never calls division helper.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const root=process.argv[2],header=fs.readFileSync(path.join(__dirname,'runtime-public-expected.h'),'utf8');
const expressions=[...header.matchAll(/^    \{"((?:\\.|[^"\\])*)", /gm)].map(m=>JSON.parse('"'+m[1]+'"'));a.equal(expressions.length,60);
a.equal(fs.readFileSync(path.join(root,'original-public.status'),'utf8'),'1\n');a.equal(fs.statSync(path.join(root,'original-public.out')).size,0);
const lines=fs.readFileSync(path.join(root,'original-public.stderr'),'utf8').trimEnd().split('\n');a.equal(lines.pop(),'1220 runtime division failures');
const expected=new Set();for(const mode of ['COPPERFIN','VFP9'])for(const d of [0,2,6,14,18])for(const fixed of ['OFF','ON']){const context=mode+'/D'+d+'/FIXED'+fixed;for(const e of expressions)expected.add('FAIL: '+context+' '+e+' native descriptor');expected.add('FAIL: '+context+' memory prefix descriptor');}
a.equal(expected.size,1220);a.equal(lines.length,expected.size);for(const line of lines)a(expected.delete(line),'unexpected/duplicate failure: '+line);a.equal(expected.size,0);
console.log(JSON.stringify({originalDescriptorFailures:1220,otherControlFailures:0,expectedRows:60,settings:5,fixedModes:2,behaviorModes:2,helperImported:false}));

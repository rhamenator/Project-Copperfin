// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent correspondence: native captures and literal C++ expected table.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),crypto=require('node:crypto');
const cpp=fs.readFileSync(path.join(__dirname,'../../test_prg_engine_numeric_real_assignment.cpp'),'utf8');
const table=cpp.split('const std::vector<Row> rows={')[1].split('};')[0];
const rows=[...table.matchAll(/\{"([^"]+)",(\d+),(\d+),0x([0-9A-F]{16})ULL\}/g)]
 .map(m=>({source:m[1],width:Number(m[2]),decimals:Number(m[3]),bits:m[4]}));
a.equal(rows.length,46);let checked=0;
const raw=fs.readFileSync(path.join(__dirname,'captures/first-results.txt'),'ascii');
for(const row of rows){let matches=0;
 for(const line of raw.split('\r\n')){const f=line.split('|');
  if(f[0]!=='VALUE'||!['3','4','5','6','7'].includes(f[4])||Buffer.from(f[5],'hex').toString('ascii')!==row.source)continue;
  const m=Object.fromEntries(f[6].split(';').map(s=>s.split('=')));
  a.equal(m.type,'4E',row.source);a.equal(parseInt(m.width,16),row.width,row.source);
  a.equal(parseInt(m.decimals,16),row.decimals,row.source);a.equal(m.real,row.bits,row.source);a.equal(f[7],row.bits,row.source);
  ++checked;++matches;
 }a.equal(matches,20,row.source);
}
const old=fs.readFileSync(path.join(__dirname,'../../test_prg_engine_numeric_integer_assignment.cpp'),'utf8');
const integerTable=old.split('const std::vector<Row> rows={')[1].split('};')[0];
const integerTableSha256=crypto.createHash('sha256').update(integerTable).digest('hex');
a.equal(integerTableSha256,'c44cd2626e19817521c7559c4049075f24b4b025a143a25b63b2ff194322c15e');
a(old.includes('PrgNumericMetadata{12,1}'),'native -0.0 staged real control migrated');
process.stdout.write(JSON.stringify({expectedRows:rows.length,nativeAssignmentRecordsMatched:checked,
 integerTableSha256,allDescriptorsAndBitsMatch:true,productionHelperUsed:false})+'\n');

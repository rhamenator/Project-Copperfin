// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent expected-table correspondence; never import the production helper.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const fixture=__dirname,cpp=fs.readFileSync(process.argv[2]||path.join(fixture,'../../test_prg_engine_numeric_integer_assignment.cpp'),'utf8');
const table=cpp.split('const std::vector<Row> rows={')[1].split('};')[0];
const rows=[...table.matchAll(/\{"([^"]+)",(\d+),(0x[0-9A-F]+ULL|0)\}/g)].map(m=>({source:m[1],width:Number(m[2]),bits:m[3]==='0'?'0000000000000000':m[3].slice(2,-3).padStart(16,'0')}));
a.equal(rows.length,18);let checked=0;
for(const row of rows){
 let matches=0;
 for(const capture of ['assignment','boundary','namespace']){
  const filename=path.join(process.argv[3]||path.join(fixture,'captures'),capture,'first-results.txt');
  for(const line of fs.readFileSync(filename,'ascii').split('\r\n')){
   const f=line.split('|');if(f[0]!=='VALUE'||!['3','4','5','6','7'].includes(f[4]))continue;
   if(Buffer.from(f[5],'hex').toString('ascii')!==row.source)continue;
   const m=Object.fromEntries(f[6].split(';').map(s=>s.split('=')));
   a.equal(m.type,'49',row.source);a.equal(parseInt(m.width,16),row.width,row.source);a.equal(f[7],row.bits,row.source);
   // Integer's decimal/real slots are undefined: do not compare them.
   ++matches;++checked;
  }
 }
 a(matches>=20,row.source+' native records absent');
}
process.stdout.write(JSON.stringify({expectedRows:rows.length,nativeAssignmentRecordsMatched:checked,allWidthsTypesAndBitsMatch:true,productionHelperUsed:false})+'\n');

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent structural/identity/typed-repeat reader; never compute new output strings.
const fs = require('node:fs'), path = require('node:path'), z = require('node:zlib'), a = require('node:assert/strict'), c = require('node:crypto');
const root = process.argv[2]; a(root);
const inv = JSON.parse(fs.readFileSync(path.join(__dirname, 'half-consumers-cases.json')));
const source = fs.readFileSync(path.join(__dirname, 'half-consumers.prg'), 'utf8');
a.equal(inv.inputCount, 6); a.equal(inv.recordsPerRun, 12); a.equal(inv.fieldsPerRecord, 28);
a.deepEqual(inv.inputs.map(r => r.bits), ['3FDFFFFFFFFFFFFF','3FE0000000000000','3FE0000000000001','BFDFFFFFFFFFFFFF','BFE0000000000000','BFE0000000000001']);
for (const [i, r] of inv.inputs.entries()) { a.equal(r.index, i + 1); a.equal(r.setting, 0); a(source.includes(`inputHex[${r.index}]='${r.bits}'`)); }
const input = 'CFNDEC(CFNWIDTH(payload,40),18)', round = `ROUND(${input},0)`;
a(!/\b(?:argument|rounded)\s*=/i.test(source));
for (const [n, e] of [['roundType',`VARTYPE(${round})`],['roundMeta',`CFNMETA(${round})`],['roundBits',`HexBytes(BINTOC(${round},'BRS'))`],['roundDisplay',`HexBytes(TRANSFORM(${round}))`],['roundStr',`HexBytes(STR(${round},40,0))`]]) {
  a(source.includes(`${n}=${e}`)); a(source.includes(`${n.replace('round','roundAfter')}=${e}`));
}
a(source.includes('SET DECIMALS TO 0')); a(source.includes('ERROR-LOAD|')); a(source.includes('ERROR-bindingBefore|')); a(source.includes('ERROR-bindingAfter|'));
const line = source.split('\n').filter(s => s.startsWith('   line=fixedMode')); a.equal(line.length, 1);
a.equal((line[0].match(/\+'\|'/g) || []).length, 26); a(line[0].includes("+'|0'"));
a.equal(inv.moduleSha256,'8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe');
a.equal(inv.runtimeEnvironment,'WINEDLLOVERRIDES=msvcr71=n');
if (root === '--source-only') { console.log(JSON.stringify({sourceOnly:true,inputs:6,records:12,fields:28,nativeInvoked:false,newResultsRead:false})); process.exit(0); }
const bytes = n => { const p = path.join(root,n); return fs.existsSync(p) ? fs.readFileSync(p) : z.gunzipSync(fs.readFileSync(p+'.gz')); };
const sha = b => c.createHash('sha256').update(b).digest('hex');
const ascii = h => { a.match(h,/^(?:[0-9A-F]{2})*$/); const b=Buffer.from(h,'hex'); a(b.every(n=>n>0&&n<128)); return b.toString('ascii'); };
a.equal(bytes('supplied-runtime-env.txt').toString(),inv.runtimeEnvironment+'\n');
a(['UNSET\n','\n'].includes(bytes('inherited-dll-overrides.txt').toString()));
function meta(s,bits,original=false) {
  a.match(bits,/^[0-9A-F]{16}$/); a.notEqual((BigInt('0x'+bits)>>52n)&2047n,2047n);
  a.match(s,/^type=(?:4E|49);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
  const m=Object.fromEntries(s.split(';').map(x=>x.split('=')));
  if(m.type==='4E'){a.equal(m.real,bits);delete m.integer;}
  else{const b=Buffer.alloc(8);b.writeDoubleBE(parseInt(m.integer,16)|0);a.equal(b.toString('hex').toUpperCase(),bits);delete m.real;delete m.decimals;}
  if(original){a.equal(m.type,'4E');a.equal(m.width,'0028');a.equal(m.decimals,'00000012');}
  return m;
}
function binding(s,bits) {
  a.match(s,/^kind=F;count=00000011;real=[0-9A-F]{16};decimal=[0-9A-F]{8};sign=[0-9A-F]{8};cw0=027F;cw1=027F;module=(?:[0-9A-F]{2})+;fileSample=ONCE;fileSize=[0-9A-F]{8};fileSha256=[0-9A-F]{64};machine=[0-9A-F]{4};magic=[0-9A-F]{4};stamp=[0-9A-F]{8};imageSize=[0-9A-F]{8};checksum=[0-9A-F]{8};digits=(?:3[0-9])*$/);
  const r=Object.fromEntries(s.split(';').map(x=>x.split('=')));a.equal(r.real,bits);a(r.digits.length<=254);
  a.equal(ascii(r.module).toLowerCase(),inv.modulePath.toLowerCase());a.equal(r.fileSha256.toLowerCase(),inv.moduleSha256);a.equal(parseInt(r.fileSize,16),inv.moduleFileSize);
  for(const [k,v]of Object.entries(inv.modulePe))a.equal(parseInt(r[k],16),v);return r;
}
function read(n){
  a.equal(bytes(n+'.status').toString(),'0\n');a.equal(bytes(n+'.stderr').length,0);
  const raw=bytes(n+'-results.txt');a.deepEqual(raw,bytes(n+'.out'));a(raw.every(n=>n<128));
  const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|12');a.equal(lines.length,12);
  const observations=[],fields=[];let ordinal=0;
  for(const r of inv.inputs)for(const fixed of ['OFF','ON']){
    const f=lines[ordinal++].split('|');a.equal(f.length,28);a.equal(f[0],fixed);a.equal(f[1],String(r.index));a.equal(f[2],r.bits);a.equal(f[3],'0');
    const originalMeta=meta(f[4],r.bits,true);a.deepEqual(meta(f[18],r.bits,true),originalMeta);a.equal(f[5],r.bits);a.equal(f[19],r.bits);
    a.equal(f[6],f[20]);a.equal(f[7],f[21]);a.equal(ascii(f[7]).length,40);ascii(f[6]);
    a.equal(f[8],'N');a.match(ascii(f[9]),/^\s*0(?:\.0+)?\s*$/);a.equal(f[10],'C');a.equal(ascii(f[11]),fixed);
    const crt=binding(f[12],r.bits);a.deepEqual(binding(f[22],r.bits),crt);
    a.equal(f[13],'N');a.equal(f[23],'N');const resultMeta=meta(f[14],f[15]);a.deepEqual(meta(f[24],f[25]),resultMeta);
    a.equal(f[15],f[25]);a.equal(f[16],f[26]);a.equal(f[17],f[27]);ascii(f[16]);a.equal(ascii(f[17]).length,40);
    observations.push({...r,fixed,originalMeta,originalDisplayHex:f[6],originalStrHex:f[7],appliedDecimalHex:f[9],appliedFixedHex:f[11],resultMeta,resultBits:f[15],resultDisplayHex:f[16],resultStrHex:f[17],crt});fields.push(f);
  }
  return {raw,observations,fields};
}
const first=read('first');
if(process.argv[3]==='--single'){console.log(JSON.stringify({firstComplete:true,records:12,argumentAndSettingsExact:true,typedResultAndControls:true,exactRuntimeIdentity:true,firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);}
a.equal(bytes('first-structure.status').toString(),'0\n');a.equal(bytes('first-structure.stderr').length,0);
const single=JSON.parse(bytes('first-structure.json'));a.equal(single.firstSha256,sha(first.raw));a.equal(single.firstComplete,true);a.equal(single.pairedQualified,false);
const repeat=read('repeat');a.deepEqual(first.observations,repeat.observations);let inactiveSlotDifferences=0;
for(let i=0;i<12;i++)for(let k=0;k<28;k++){
  if(first.fields[i][k]===repeat.fields[i][k])continue;a([4,14,18,24].includes(k));const original=[4,18].includes(k),bits=original?first.observations[i].bits:first.observations[i].resultBits;
  a.deepEqual(meta(first.fields[i][k],bits,original),meta(repeat.fields[i][k],bits,original));++inactiveSlotDifferences;
}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:12,documentedFieldsRepeatEqual:true,argumentAndSettingsExact:true,typedResultAndControls:true,exactRuntimeIdentity:true,rawByteEqual:first.raw.equals(repeat.raw),inactiveSlotDifferences,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),limitation:inv.limitation,observations:first.observations},null,2));

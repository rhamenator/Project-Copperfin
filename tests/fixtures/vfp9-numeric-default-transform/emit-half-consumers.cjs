// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: native observations, no expected strings.
const fs = require('node:fs'), path = require('node:path'), z = require('node:zlib');
const a = require('node:assert/strict'), crypto = require('node:crypto');
const read = (dir, n) => z.gunzipSync(fs.readFileSync(path.join(__dirname, dir, n + '.gz')));
const sha = b => crypto.createHash('sha256').update(b).digest('hex');
const classificationRoot = 'graph-rejection-classification1';
const manifest = JSON.parse(fs.readFileSync(path.join(__dirname, classificationRoot, 'manifest.json')));
const receipt = manifest.files.find(r => r.name === 'pass2-classification.json'); a(receipt);
const classificationBytes = read(classificationRoot, receipt.name); a.equal(sha(classificationBytes), receipt.sha256);
const classification = JSON.parse(classificationBytes); a.equal(classification.nativeInvoked, false);
a.equal(classification.rejectedMagnitudes, 18); a.equal(classification.sharedContextsChecked, 528);
const parentRoot = 'arithmetic-inline-captures1';
const parent = JSON.parse(read(parentRoot, 'audit.json'));
const parentHash = '1979626f4fec992924791da5cbc89ed3abef1583810690300fff1fd3cf95ea32';
for (const run of ['first', 'repeat']) { a.equal(parent[run + 'Sha256'], parentHash); a.equal(sha(read(parentRoot, run + '-results.txt')), parentHash); }
a.equal(classification.sourceHashes.graph, parentHash); a.equal(parent.observations.length, 528);
const positive = [...new Set(classification.rejected.map(r => {
  const p = parent.observations.find(p => p.index === r.index && p.fixed === 'ON'); a(p);
  a.equal(r.bits, p.bits); a.equal(r.arithmetic.scaled.bits, p.arithmetic.scaled.bits);
  return p.arithmetic.scaled.bits;
}))].sort();
a.deepEqual(positive, ['3FDFFFFFFFFFFFFF', '3FE0000000000000', '3FE0000000000001']);
const inputs = [...positive.map(bits => ({bits, provenance: 'Observed positive scaled operand in retained rejected graph.'})),
  ...positive.map(bits => ({bits: (BigInt('0x' + bits) | (1n << 63n)).toString(16).toUpperCase(), provenance: 'Explicit negative-sign mirror stimulus; not an observed negative scaled stage.'}))]
  .map((r, i) => ({index: i + 1, setting: 0, ...r}));
const previous = JSON.parse(fs.readFileSync(path.join(__dirname, 'str-middle-cases.json')));
const inv = {requirement: previous.requirement, inputCount: 6, recordsPerRun: 12, fieldsPerRecord: 28,
  parentNativeSha256: parentHash, classificationSha256: sha(classificationBytes), inputs,
  inputRoute: 'Inline CFNDEC(CFNWIDTH(payload,40),18) at every numeric consumer; no stored argument/result.',
  roundRoute: 'Inline ROUND(CFNDEC(CFNWIDTH(payload,40),18),0) at every result consumer.',
  width: 40, scale: 18, appliedDecimals: 0, controlCount: 17,
  runtimeEnvironment: 'WINEDLLOVERRIDES=msvcr71=n',
  observerSha256: '0b1fcd2ba81b95631cb52e6ecd0d4afa721b450a967402f504d1852a49e9e9ac',
  limitation: 'Six explicit signed half-neighbor stimuli/D0/bothFIXED only. New public ROUND and display outputs are observations, not expected results or an internal arithmetic graph. No Windows parity, atomic mapped-code proof or accepted formatter.'};
for (const k of ['modulePath', 'moduleSha256', 'moduleFileSize', 'modulePe']) inv[k] = previous[k];
a.equal(inv.moduleSha256, '8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe');
const input = 'CFNDEC(CFNWIDTH(payload,40),18)', rounded = `ROUND(${input},0)`;
const observations = [
  ['inputMeta', `CFNMETA(${input})`], ['inputBits', `HexBytes(BINTOC(${input},'BRS'))`],
  ['inputDisplay', `HexBytes(TRANSFORM(${input}))`], ['inputStr', `HexBytes(STR(${input},40,0))`],
  ['decType', "VARTYPE(SET('DECIMALS'))"], ['decText', "HexBytes(TRANSFORM(SET('DECIMALS')))"],
  ['fixedType', "VARTYPE(SET('FIXED'))"], ['fixedText', "HexBytes(TRANSFORM(SET('FIXED')))"],
  ['bindingBefore', `CFNFCVT(${input},17)`],
  ['roundType', `VARTYPE(${rounded})`], ['roundMeta', `CFNMETA(${rounded})`],
  ['roundBits', `HexBytes(BINTOC(${rounded},'BRS'))`], ['roundDisplay', `HexBytes(TRANSFORM(${rounded}))`], ['roundStr', `HexBytes(STR(${rounded},40,0))`],
  ['inputAfterMeta', `CFNMETA(${input})`], ['inputAfterBits', `HexBytes(BINTOC(${input},'BRS'))`],
  ['inputAfterDisplay', `HexBytes(TRANSFORM(${input}))`], ['inputAfterStr', `HexBytes(STR(${input},40,0))`],
  ['bindingAfter', `CFNFCVT(${input},17)`],
  ['roundAfterType', `VARTYPE(${rounded})`], ['roundAfterMeta', `CFNMETA(${rounded})`],
  ['roundAfterBits', `HexBytes(BINTOC(${rounded},'BRS'))`], ['roundAfterDisplay', `HexBytes(TRANSFORM(${rounded}))`], ['roundAfterStr', `HexBytes(STR(${rounded},40,0))`]
];
a.equal(observations.length, 24);
let source = `* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: public near-half consumers, no expected strings.
LOCAL ARRAY inputHex[6]
${inputs.map(r => `inputHex[${r.index}]='${r.bits}'`).join('\n')}
LOCAL outputPath,libraryPath,crtPath,index,fixedNo,fixedMode,payload,caught,line,total,failed
LOCAL ${observations.map(([n]) => n).join(',')}
outputPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-literal-display-5611\\Project-Copperfin\\build\\default-transform-half-consumers\\results.txt'
libraryPath='Z:\\home\\rich\\dev\\Project-Copperfin\\build\\pr7203-completed\\native-evidence\\diagnostic\\cause-probe.fll'
crtPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-literal-display-5611\\Project-Copperfin\\build\\default-transform-gcvt-captures1\\gcvt-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
TRY
 SET LIBRARY TO (libraryPath) ADDITIVE
 SET LIBRARY TO (crtPath) ADDITIVE
CATCH TO caught
 =STRTOFILE('ERROR-LOAD|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)+CHR(13)+CHR(10),outputPath,1)
 SET LIBRARY TO
 RETURN
ENDTRY
total=0
SET DECIMALS TO 0
FOR index=1 TO ALEN(inputHex)
 payload=CTOBIN(Unhex(inputHex[index]),'BS')
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  failed=.F.
  TRY
`;
for (const [n, e] of observations) {
  source += `   ${n}=${e}\n`;
  if (n.startsWith('binding')) source += `   IF LEFT(${n},6)=='ERROR:'
    =STRTOFILE('ERROR-${n}|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(${n})+CHR(13)+CHR(10),outputPath,1)
    SET LIBRARY TO
    RETURN
   ENDIF
`;
}
source += `   line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+inputHex[index]+'|0'${observations.map(([n]) => "+'|'+" + n).join('')}
  CATCH TO caught
   failed=.T.
   line='ERROR-'+fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
  ENDTRY
  =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
  IF failed
   SET LIBRARY TO
   RETURN
  ENDIF
  total=total+1
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO
\n`;
const helpers = fs.readFileSync(path.join(__dirname, 'round-inline.prg'), 'utf8').split('\nFUNCTION Unhex\n'); a.equal(helpers.length, 2);
source += 'FUNCTION Unhex\n' + helpers[1];
if (process.argv[2] === 'source') process.stdout.write(source);
else if (process.argv[2] === 'cases') process.stdout.write(JSON.stringify(inv, null, 2) + '\n');
else console.log(JSON.stringify({sourceOnly: true, inputCount: 6, records: 12, fields: 28, nativeInvoked: false}));

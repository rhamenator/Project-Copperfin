// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: synthetic coverage tests, not native expectations.
'use strict';
const a = require('node:assert/strict');
const {inventory} = require('./inventory-str-crt-coverage.cjs');
const s = {index: 1, fixed: 'OFF', bits: '3FF0000000000000', setting: 15,
  str15Hex: '31', str16Hex: '31', str17Hex: '31', str18Hex: '31'};
const c = {...s, route: 'ORIGINAL', kind: 'F', argumentBits: s.bits, count: 15};
let tests = 0;
const check = f => {f(); ++tests;};
check(() => {const r = inventory([s], [c]); a.equal(r.requestedCells, 4);
  a.equal(r.availableCells, 1); a.deepEqual(r.missingCells.map(x => x.count), [16,17,18]);
  a.equal(r.cells.length, 4); a.equal(r.candidateOrEqualityEvidence, false);});
for (const delta of [{index: 2}, {fixed: 'ON'}, {setting: 16},
  {bits: '4000000000000000', argumentBits: '4000000000000000'},
  {route: 'SHIFTED'}, {kind: 'E'}, {count: 14}])
  check(() => a.equal(inventory([s], [{...c, ...delta}]).availableCells, 0));
check(() => a.throws(() => inventory([s, s], [c]), /duplicate public/));
check(() => a.throws(() => inventory([s], [c, c]), /duplicate native/));
check(() => a.throws(() => inventory([s], [{...c, argumentBits: '0000000000000000'}])));
check(() => a.throws(() => inventory([{...s, str18Hex: 'GG'}], [c])));
check(() => a.equal(inventory([s], []).missingCells.length, 4));
check(() => {const r = inventory([s], [c]); a.equal(r.cells[0].bits, s.bits);
  a.equal(r.cells[0].setting, 15); a.equal(r.cells[0].fixed, 'OFF');});
check(() => {const rows = [s, {...s, fixed:'ON'}];
  const native = rows.flatMap(r => [15,16,17,18].map(count => ({...c,...r,count})));
  const before = JSON.stringify([rows,native]); const r = inventory(rows,native);
  a.equal(r.requestedCells,8); a.equal(r.availableCells,8); a.deepEqual(r.missingCells,[]);
  a.equal(JSON.stringify([rows,native]),before);});
check(() => {const r = inventory([s], [{...c,crt:{digits:'',decimal:'FFFFFFF1',sign:'00000000'}}]);
  a.equal(r.availableCells,1); a.equal(r.candidateOrEqualityEvidence,false);
  // An empty native return proves availability only, never numeric equality.
  a(!Object.hasOwn(r.cells[0],'value'));});
check(() => a.equal(inventory([s], [{...c,count:21}]).availableCells,0));
check(() => a.throws(() => inventory([s], [{...c,count:22}])));
check(() => {const native=[c,{...c,count:16},{...c,count:17},{...c,count:18}];
  a.equal(inventory([s],native).availableCells,4);
  a.throws(() => inventory([s],[...native,{...c,count:16}]),/duplicate native/);});
console.log(JSON.stringify({syntheticCoverageTests: tests, nativeInvoked: false}));

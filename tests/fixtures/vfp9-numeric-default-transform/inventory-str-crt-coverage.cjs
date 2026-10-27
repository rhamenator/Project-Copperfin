// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: coverage only, never expected digits.
'use strict';
const a = require('node:assert/strict');

// A count observed at another payload, setting, metadata or FIXED mode cannot
// fill a missing cell. Availability does not assert runtime equivalence or
// conversion equality; those remain separate qualified-join requirements.
function inventory(strRows, crtRows) {
  const key = (r, count) => JSON.stringify([r.index, r.fixed, r.bits, r.setting, count]);
  const available = new Map();
  for (const r of crtRows) {
    if (r.route !== 'ORIGINAL' || r.kind !== 'F') continue;
    a.equal(r.argumentBits, r.bits);
    // The separately retained D+1/+2/+3 experiment reaches count21.
    // Admitting its inventory does not qualify cross-capture equivalence.
    a(Number.isInteger(r.count) && r.count >= 1 && r.count <= 21);
    const k = key(r, r.count);
    a(!available.has(k), 'duplicate native CRT context');
    available.set(k, r);
  }
  const seen = new Set(), cells = [];
  for (const r of strRows) {
    a(Number.isInteger(r.index) && r.index > 0);
    a(['OFF', 'ON'].includes(r.fixed));
    a.match(r.bits, /^[0-9A-F]{16}$/);
    a(Number.isInteger(r.setting) && r.setting >= 0 && r.setting <= 18);
    for (const count of [15, 16, 17, 18]) {
      const k = key(r, count);
      a(!seen.has(k), 'duplicate public STR context'); seen.add(k);
      a.match(r['str' + count + 'Hex'], /^(?:[0-9A-F]{2})+$/);
      cells.push({index: r.index, fixed: r.fixed, bits: r.bits,
        setting: r.setting, count, nativeCrtAvailable: available.has(k)});
    }
  }
  return {coverageOnly: true, candidateOrEqualityEvidence: false,
    requestedCells: cells.length,
    availableCells: cells.filter(r => r.nativeCrtAvailable).length,
    missingCells: cells.filter(r => !r.nativeCrtAvailable), cells};
}
module.exports = {inventory};

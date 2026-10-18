// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001: complete native observations; no Copperfin helper/child oracle.
const fs = require('node:fs'), path = require('node:path'), zlib = require('node:zlib');
const a = require('node:assert/strict'), crypto = require('node:crypto');
const root = process.argv[2];
const source = fs.readFileSync(path.join(__dirname, 'nonwhole-grid.prg'), 'utf8');
const inventory = JSON.parse(fs.readFileSync(path.join(__dirname, 'nonwhole-grid-cases.json')));
a.equal(inventory.rows.length, 578); a.deepEqual(inventory.settings, [0, 2, 6, 14, 18]);
a.equal([...source.matchAll(/^expressions\[\d+\]='/gm)].length, 578);
a.equal([...source.matchAll(/^ CASE index=\d+$/gm)].length, 578);
const declared = new Map(inventory.rows.map(row => [row.index, row]));
a.deepEqual([...declared.keys()], Array.from({length: 578}, (_, i) => i + 1));
for (const row of inventory.rows) {
    a(source.includes(`expressions[${row.index}]='${row.expression}'`));
    const e = row.expression;
    a(source.includes(` CASE index=${row.index}\n  RETURN CFNMETA(${e})+'|'+HexBytes(BINTOC(${e},'BRS'))+'|'+HexBytes(TRANSFORM(${e}))`));
}
for (let i = 0; i < inventory.settings.length; ++i) a(source.includes(`settings[${i + 1}]=${inventory.settings[i]}`));
function descriptor(text) {
    a.match(text, /^type=(?:49|4E);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
    const raw = Object.fromEntries(text.split(';').map(part => part.split('=')));
    const typed = {type: raw.type === '49' ? 'I' : 'N', width: parseInt(raw.width, 16), cw: raw.cw};
    if (typed.type === 'I') {
        typed.integer = parseInt(raw.integer, 16) | 0;
        const bytes = Buffer.alloc(8); bytes.writeDoubleBE(typed.integer);
        typed.bits = bytes.toString('hex').toUpperCase();
    } else { typed.decimals = parseInt(raw.decimals, 16); typed.bits = raw.real; }
    return {raw, typed};
}
function bytes(name) {
    const filename = path.join(root, name);
    return fs.existsSync(filename) ? fs.readFileSync(filename) : zlib.gunzipSync(fs.readFileSync(filename + '.gz'));
}
function capture(name) {
    a.equal(fs.readFileSync(path.join(root, name + '.status'), 'utf8'), '0\n');
    a.equal(fs.statSync(path.join(root, name + '.stderr')).size, 0);
    const raw = bytes(name + '-results.txt'); a.deepEqual(raw, bytes(name + '.out'));
    const lines = raw.toString('ascii').split('\r\n'); a.equal(lines.pop(), '');
    a.equal(lines.shift(), 'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    a.equal(lines.pop(), 'DONE|11560'); a.equal(lines.length, 11560);
    const rows = new Map();
    for (const line of lines) {
        const fields = line.split('|'); a.equal(fields.length, 8, line);
        const [mode, settingText, fixed, indexText, expressionHex] = fields;
        const setting = Number(settingText), index = Number(indexText), row = declared.get(index);
        a(['EVAL', 'COMPILED'].includes(mode)); a(inventory.settings.includes(setting));
        a(['OFF', 'ON'].includes(fixed)); a(row);
        a.equal(Buffer.from(expressionHex, 'hex').toString('ascii'), row.expression);
        const out = descriptor(fields[5]); a.equal(fields[6], out.typed.bits);
        a.match(fields[7], /^(?:[0-9A-F]{2})+$/);
        const key = JSON.stringify([setting, fixed, index, mode]); a(!rows.has(key));
        rows.set(key, {fields, typed: {setting, fixed, index, mode, result: out.typed, displayHex: fields[7]}});
    }
    for (const setting of inventory.settings) for (const fixed of ['OFF', 'ON']) for (const row of inventory.rows) {
        const x = rows.get(JSON.stringify([setting, fixed, row.index, 'EVAL']));
        const y = rows.get(JSON.stringify([setting, fixed, row.index, 'COMPILED']));
        a(x && y); a.deepEqual({...x.typed, mode: null}, {...y.typed, mode: null});
    }
    return {raw, rows};
}
const first = capture('first'), repeat = capture('repeat'), unused = {};
let rawDifferentRows = 0;
for (const [key, x] of first.rows) {
    const y = repeat.rows.get(key); a(y); a.deepEqual(x.typed, y.typed);
    if (x.fields.join('|') !== y.fields.join('|')) ++rawDifferentRows;
    for (let i = 0; i < 8; ++i) if (i !== 5) a.equal(x.fields[i], y.fields[i]);
    const p = descriptor(x.fields[5]).raw, q = descriptor(y.fields[5]).raw;
    for (const field of Object.keys(p)) if (p[field] !== q[field]) {
        a(p.type === '49' ? ['real', 'decimals'].includes(field) : field === 'integer');
        const label = `${p.type}:${field}`; unused[label] = (unused[label] || 0) + 1;
    }
}
const sha = value => crypto.createHash('sha256').update(value).digest('hex');
console.log(JSON.stringify({casesPerRun: 11560, successfulValues: 11560, errors: 0,
    expressions: 578, documentedFieldsRepeatEqual: true, compiledVersusEvaluateDifferences: 0,
    rawByteEqual: first.raw.equals(repeat.raw), rawDifferentRows, rawDifferencesConfinedToUnusedSlots: unused,
    firstSha256: sha(first.raw), repeatSha256: sha(repeat.raw),
    limitation: 'Complete expression observations only. Separately evaluated child metadata is not an operand oracle for an enclosing folded tree.',
    observations: [...first.rows.values()].map(row => row.typed)}, null, 2));

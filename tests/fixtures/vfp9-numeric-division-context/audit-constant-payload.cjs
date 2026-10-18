// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native-only structural/type-aware audit. No Copperfin helper or precision equation.
const fs = require('node:fs'), path = require('node:path');
const a = require('node:assert/strict'), crypto = require('node:crypto');
const zlib = require('node:zlib');
const root = process.argv[2];
const batch = process.argv[3]; a(['a', 'b'].includes(batch));
const source = fs.readFileSync(path.join(__dirname, `constant-payload-${batch}.prg`), 'utf8');
const inventory = JSON.parse(fs.readFileSync(path.join(__dirname, `constant-payload-${batch}-cases.json`)));
a.equal(inventory.batch, batch);
a.equal(inventory.rows.length, 320);
a.deepEqual(inventory.settings, [0, 2, 6, 14, 18]);
const byIndex = new Map(inventory.rows.map(row => [row.index, row]));
a.equal(byIndex.size, 320);
a.deepEqual([...byIndex.keys()], Array.from({length: 320}, (_, i) => i + 1));
a.equal([...source.matchAll(/^expressions\[\d+\]='/gm)].length, 320);
a.equal([...source.matchAll(/^lefts\[\d+\]='/gm)].length, 320);
a.equal([...source.matchAll(/^rights\[\d+\]='/gm)].length, 320);
a.equal([...source.matchAll(/^ CASE index=\d+$/gm)].length, 320);
a(source.includes('expressions[320], lefts[320], rights[320], settings[5]'));
for (let i = 0; i < inventory.settings.length; ++i) a(source.includes(`settings[${i + 1}]=${inventory.settings[i]}`));
for (const row of inventory.rows) {
    a(source.includes(`expressions[${row.index}]='${row.expression}'`));
    a(source.includes(`lefts[${row.index}]='${row.left}'`));
    a(source.includes(`rights[${row.index}]='${row.right}'`));
    const e = row.expression;
    a(source.includes(` CASE index=${row.index}\n  RETURN CFNMETA(${e})+'|'+HexBytes(BINTOC(${e},'BRS'))+'|'+HexBytes(TRANSFORM(${e}))+'|'+CFNMETA(${row.left})+'|'+CFNMETA(${row.right})`));
}
function meta(text) {
    a.match(text, /^type=(?:49|4E);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
    const raw = Object.fromEntries(text.split(';').map(part => part.split('=')));
    const typed = {type: raw.type === '49' ? 'I' : 'N', width: parseInt(raw.width, 16), cw: raw.cw};
    if (typed.type === 'I') {
        typed.integer = parseInt(raw.integer, 16) | 0;
        const buffer = Buffer.alloc(8); buffer.writeDoubleBE(typed.integer);
        typed.bits = buffer.toString('hex').toUpperCase();
    } else {
        typed.decimals = parseInt(raw.decimals, 16);
        typed.bits = raw.real;
    }
    return {raw, typed}; // Unused native tagged-interface slots are not interpreted.
}
const count = inventory.rows.length * inventory.settings.length * 4;
function bytes(name) {
    const plain = path.join(root, name);
    return fs.existsSync(plain) ? fs.readFileSync(plain) : zlib.gunzipSync(fs.readFileSync(plain + '.gz'));
}
function capture(name) {
    a.equal(fs.readFileSync(path.join(root, name + '.status'), 'utf8'), '0\n');
    a.equal(fs.statSync(path.join(root, name + '.stderr')).size, 0);
    const raw = bytes(name + '-results.txt');
    a.deepEqual(raw, bytes(name + '.out'));
    const lines = raw.toString('ascii').split('\r\n');
    a.equal(lines.pop(), '');
    a.equal(lines.shift(), 'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    a.equal(lines.pop(), 'DONE|' + count);
    a.equal(lines.length, count);
    const rows = new Map();
    for (const line of lines) {
        const fields = line.split('|');
        a.equal(fields.length, 10, line);
        const [mode, settingText, fixed, indexText, expressionHex] = fields;
        const setting = Number(settingText), index = Number(indexText), declared = byIndex.get(index);
        a(['EVAL', 'COMPILED'].includes(mode)); a(inventory.settings.includes(setting));
        a(['ON', 'OFF'].includes(fixed)); a(declared);
        a.equal(Buffer.from(expressionHex, 'hex').toString('ascii'), declared.expression);
        const out = meta(fields[5]), left = meta(fields[8]), right = meta(fields[9]);
        a.equal(fields[6], out.typed.bits);
        a.match(fields[7], /^(?:[0-9A-F]{2})+$/);
        for (const [literal, observed] of [[declared.left, left.typed], [declared.right, right.typed]]) {
            a.match(literal, /^-?[0-9]+(?:\.[0-9]+)?$/);
            const unsigned = literal.replace(/^-/, ''), point = unsigned.indexOf('.');
            const decimal = point >= 0, value = Number(literal);
            const integerStorage = !decimal && Math.abs(value) <= 2147483647;
            a.equal(observed.type, integerStorage ? 'I' : 'N');
            a.equal(observed.width, literal.length);
            const payload = Buffer.alloc(8);
            payload.writeDoubleBE(integerStorage && value === 0 ? 0 : value);
            a.equal(observed.bits, payload.toString('hex').toUpperCase());
            if (integerStorage) a.equal(observed.integer, value === 0 ? 0 : value);
            else a.equal(observed.decimals, decimal ? unsigned.length - point - 1 : 0);
        }
        const quotient = Number(declared.left) / Number(declared.right);
        const expected = declared.mode === 'group-minus' ? -quotient : quotient;
        a(Buffer.from(out.typed.bits, 'hex').readDoubleBE() === expected);
        const key = JSON.stringify([setting, fixed, index, mode]); a(!rows.has(key));
        rows.set(key, {fields, typed: {setting, fixed, index, mode, result: out.typed, left: left.typed, right: right.typed, displayHex: fields[7]}});
    }
    for (const setting of inventory.settings) for (const fixed of ['OFF', 'ON']) for (const row of inventory.rows) {
        const evaluated = rows.get(JSON.stringify([setting, fixed, row.index, 'EVAL']));
        const compiled = rows.get(JSON.stringify([setting, fixed, row.index, 'COMPILED']));
        a(evaluated && compiled);
        a.deepEqual({...evaluated.typed, mode: null}, {...compiled.typed, mode: null});
    }
    return {raw, rows};
}
const first = capture('first'), repeat = capture('repeat'), unused = {};
let rawDifferentRows = 0;
for (const [key, x] of first.rows) {
    const y = repeat.rows.get(key); a(y); a.deepEqual(x.typed, y.typed);
    if (x.fields.join('|') !== y.fields.join('|')) ++rawDifferentRows;
    for (let i = 0; i < x.fields.length; ++i) {
        if (![5, 8, 9].includes(i)) { a.equal(x.fields[i], y.fields[i]); continue; }
        const p = meta(x.fields[i]).raw, q = meta(y.fields[i]).raw;
        for (const field of Object.keys(p)) if (p[field] !== q[field]) {
            a(p.type === '49' ? ['real', 'decimals'].includes(field) : field === 'integer');
            const label = `${i}:${p.type}:${field}`; unused[label] = (unused[label] || 0) + 1;
        }
    }

}
const sha = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
console.log(JSON.stringify({casesPerRun: count, actualSuccessfulValues: count, errors: 0,
    batch, expressions: inventory.rows.length, settings: inventory.settings, compiledVersusEvalDifferences: 0,
    documentedFieldsRepeatEqual: true, rawByteEqual: first.raw.equals(repeat.raw), rawDifferentRows,
    rawDifferencesConfinedToUnusedSlots: unused, firstSha256: sha(first.raw), repeatSha256: sha(repeat.raw),
    limitation: 'Native observation recovery only. No constant-folding algorithm or Copperfin acceptance inferred.',
    observations: [...first.rows.values()].map(row => row.typed)}, null, 2));

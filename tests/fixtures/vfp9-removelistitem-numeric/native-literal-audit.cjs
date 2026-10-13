// Evidence audit ONLY: helper/current runtime never supplies expectations.
const fs = require('fs');
const source = fs.readFileSync('tests/test_prg_engine_removelistitem_numeric.cpp', 'utf8');
const body = source.split('const std::vector<Row> rows{')[1].split('\n};')[0];
const normalize = value => value.toUpperCase().replace(/\s/g, '');
const rows = new Map();
for (const line of body.split('\n')) {
    const match = line.match(/^\s*\{"([^"]+)",(.+),([^,]+),([^,]+)\},?$/);
    if (match) rows.set(normalize(match[1]), {
        expression: match[1], legacy: match[4] === '{}' ? null : Number(match[4])
    });
}
if (rows.size !== 69) throw Error('Frozen enumeration parse/count failed');
let calls = 0, audited = 0, failures = 0, rejectedState = 0;
const seen = new Set();
for (const [filename, stateful] of [
    ['independent-results.txt', false], ['boundary-results.txt', false],
    ['state-results.txt', true]
]) {
    const lines = fs.readFileSync('build/removelistitem-native/' + filename, 'utf8').trim().split(/\r?\n/);
    const declared = Number(lines.at(-1).split('|')[1]);
    if (lines.length !== declared + 2 || !lines[0].includes('7423')) throw Error('Incomplete native raw');
    for (const line of lines.slice(1, -1)) {
        calls++;
        const parts = line.split('|');
        const expression = parts[stateful ? 3 : 2];
        const key = normalize(expression);
        const row = rows.get(key);
        if (!row) continue; // Report unmatched literals; never manufacture evidence.
        seen.add(key); audited++;
        const shape = Number(parts[1].split('=')[1]);
        const ids = stateful ? (shape === 1 ? [1,2,3] : shape === 2 || shape === 5 ? [11,22,33] :
            shape === 3 ? [32767] : []) : shape === 1 ? [1,2,3] : shape === 2 ? [11,22,33] : [];
        const expected = row.legacy === null ? 'ERR:11' : ids.includes(row.legacy) ? 'OK' : 'ERR:1234';
        const actual = parts.find(value => value === 'OK' || value.startsWith('ERR:'));
        const phase = parts[stateful ? 4 : 4];
        let stateOK = true;
        if (stateful) {
            const [before, after] = line.split('|before=')[1].split('|after=');
            const beforeIds = [...before.matchAll(/;row=(\d+),/g)].map(match => Number(match[1]));
            const afterIds = [...after.matchAll(/;row=(\d+),/g)].map(match => Number(match[1]));
            const wantedIds = expected === 'OK' ? ids.filter(id => id !== row.legacy) : ids;
            stateOK = JSON.stringify(beforeIds) === JSON.stringify(ids) &&
                JSON.stringify(afterIds) === JSON.stringify(wantedIds);
            if (actual.startsWith('ERR:')) {rejectedState++; stateOK = stateOK && before === after;}
            if (actual === 'OK') {
                // Shared successful selection/focus/text/top remain excluded.
                const beforeRows = before.split(';').slice(1).map(value => value.split(',').slice(0,4).join(','));
                const afterRows = after.split(';').slice(1).map(value => value.split(',').slice(0,4).join(','));
                stateOK = stateOK && JSON.stringify(afterRows) ===
                    JSON.stringify(beforeRows.filter(value => !value.startsWith('row='+row.legacy+',')));
            }
        }
        const ok = expected === actual && phase === (actual === 'OK' ? 'returned' : 'invoke') && stateOK;
        if (!ok) failures++;
        console.log(JSON.stringify({filename, control:parts[0],shape,expression,
            legacy:row.legacy,expected,actual,phase,stateOK,ok}));
    }
}
console.log(JSON.stringify({nativeConsumerCalls:calls,matchedCalls:audited,
    matchedLiteralCount:seen.size,unmatchedLiterals:[...rows.keys()].filter(key=>!seen.has(key)),
    fullyPreservedMatchedRejections:rejectedState,failures,
    note:'Unmatched exact integer literals are domain/help-derived; EXP expressions are parent-derived from observed infinity. Exact64/NaN/binary/config extension not native syntax.'}));
if (failures !== 0) process.exitCode = 1;

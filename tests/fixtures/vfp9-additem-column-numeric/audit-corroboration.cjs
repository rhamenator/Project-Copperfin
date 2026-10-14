// Observation-only grouping; no checked helper, conversion oracle or default table.
const fs = require('fs');
let failures = 0;
for (const [name, expected] of [['configuration', 864], ['projection', 432]]) {
  const first = fs.readFileSync(`build/additem-column-native/${name}-first.out`);
  const repeat = fs.readFileSync(`build/additem-column-native/${name}-repeat.out`);
  const lines = first.toString().trim().split(/\r?\n/);
  if (!first.equals(repeat) || lines.length !== expected + 2 ||
      lines.at(-1) !== `DONE|${expected}`) ++failures;
  const groups = new Map();
  for (const line of lines.slice(1, -1)) {
    const a = line.split('|');
    const get = k => a.find(f => f.startsWith(`${k}=`))?.slice(k.length + 1);
    const before = get('before'), after = get('after');
    const visible = [...after.matchAll(/,cell(\d+)=candidate(?:,|;|$)/g)]
      .map(x => Number(x[1]));
    const record = { context: a[2], error: Number(get('error')), phase: a[4],
      vartype: get('vartype'), postcallBRS: get('postcallBRS'),
      visibleColumns: visible, beforeAfterEqual: before === after,
      newIndex: Number(after.match(/,newindex=(\d+)/)?.[1]) };
    const key = [a[0], a[1], a[3]].join('|');
    if (!groups.has(key)) groups.set(key, []);
    groups.get(key).push(record);
    if (a[4] !== 'invoke' || ![0, 11].includes(record.error)) ++failures;
  }
  for (const [key, observations] of groups) {
    // Configuration/valid-position and empty/seeded contexts are independent
    // repetitions of the same third operand. Compare observed admission/cell
    // placement, not shared focus/text/IDs/sorted insertion positions.
    const signatures = new Set(observations.map(x => JSON.stringify(
      [x.error, x.vartype, x.postcallBRS, x.visibleColumns])));
    if (signatures.size !== 1) ++failures;
    console.log(JSON.stringify({ capture: name, key, observations,
      matchingAdmissionAndVisibleCells: signatures.size === 1 }));
  }
  console.log(JSON.stringify({ capture: name, rows: lines.length - 2,
    groups: groups.size, failuresSoFar: failures }));
}
console.log(JSON.stringify({ failures,
  note: 'Paired native observations only; no default/exact extension, allocation-capacity or implementation acceptance oracle.' }));
process.exitCode = failures ? 1 : 0;

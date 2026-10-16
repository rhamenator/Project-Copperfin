// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent original assertions and observed failure correspondence, no runtime helper.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=__dirname,read=n=>fs.readFileSync(path.join(root,n),'utf8');
const old=read('v2a-original-probe.cpp');
const source=read('../../../test_prg_engine_control_flow_task_supervision_state_sequences.cpp');
const current=source.split('static void run_retry_after_caught_rollback_sequence(')[1]
 .split('\nvoid test_state_sequence_retry_after_caught_rollback_succeeds_cleanly()')[0];
const tail=s=>s.slice(s.indexOf('    const auto error_seen_it ='),s.lastIndexOf('    fs::remove_all(temp_root, ignored);'));
a(tail(old).length>2000);a.equal(tail(old),tail(current));
const completion='expect(state.completed, "#6495 V2a: state-sequence test should complete: " + state.message);';
a(old.includes(completion));a(current.includes(completion));
const originalAssertions=1+[...tail(old).matchAll(/\bexpect\(/g)].length;a.equal(originalAssertions,11);
const failures=s=>s.split('\n').filter(l=>l.includes('FAIL:')).map(l=>l.slice(l.indexOf('FAIL:')));
const hosted=failures(read('pr7201-stress-failed.log')),delayed=failures(read('v2a-delayed.stderr'));
a.equal(hosted.length,7);a.deepEqual(hosted,delayed);
a.equal(read('v2a-original.status'),'0\n');a.equal(read('v2a-delayed.status'),'1\n');
a.equal(read('v2a-final.status'),'0\n');a.equal(read('v2a-final.stderr'),'');
for(const args of ['0U, 0U','150U, 0U','0U, 500U'])a(source.includes('run_retry_after_caught_rollback_sequence('+args+');'));
process.stdout.write(JSON.stringify({originalAssertions,assertionTailSha256:crypto.createHash('sha256').update(tail(old)).digest('hex'),
 hostedFailureMessages:hosted.length,controlledDelayedFailureMatchesExactly:true,
 finalThreeSchedulesPassed:true,productionHelperUsed:false})+'\n');

# Agent Handoff

## Branching (changed 2026-10-01)

Develop on `main`: branch from `origin/main` and open pull requests with
`--base main`. `v1-development` is retired (see
`docs/v1-development-retirement-2026-10-01.md`); older text below that says a
slice "merged into `v1-development`" is historical.

## Extension intent (direct owner steering, 2026-10-06)

VFP compatibility is the foundation, not the feature ceiling. Copperfin
extensions deliberately fix VFP problems and fill gaps with modern development
tools and platforms, borrowing suitable ideas from other languages/platforms
where useful; some gaps require substantial capabilities. Absence of native
VFP syntax or precedent is not grounds to skip or demote an extension. For the
selected admitted work, document derived extension requirements, boundaries,
interoperability and verification from owner product intent; use installed VFP
as the oracle for the equivalent legacy behavior, not as an oracle for an
extension it does not implement. This steering is additive: finish the retained
EPOCH query/numeric slices and then #3698 CENTURY/ROLLOVER. It does not admit
unrelated production work or waive acceptance/review evidence.

## Review routing (owner steering, 2026-10-06)

The owner reports GitHub review availability exhausted for the rest of
October 2026; the exhaustion start date is unknown. The owner will task Claude
with reviews. For new/revised slice heads through October 31, retain a Claude
or owner-authorized fallback Codex review tied to the exact commit and address
its findings before merging;
all 11 required checks and resolved GitHub conversations remain gates.
Unavailable/skipped GitHub bot reviews are not clean-review evidence. Preserve
actual historical review records without guessing when availability ended.
The owner's subsequent instruction authorizes requesting Claude review when
creating each PR. Use a verified Claude reviewer identity if available; the
official Claude Code Review integration documents a top-level @claude review
comment on an open, non-draft PR as its manual trigger. Do not assign an
unverified GitHub account named Claude or silently enable a paid integration.
The current CLI collaborator list contains only rhamenator, no Claude workflow
was found under .github, and app-installation lookup returned 401; installation
or quota status therefore remains unconfirmed, not known absent/exhausted.
The owner additionally authorizes Codex as the fallback when Claude is not
working. Try Claude first, then request Codex through the repository's verified
@codex review integration if Claude is unavailable or quota-blocked. Do not
assume reported monthly exhaustion has reset or treat quota notices as review.
Report explicit quota/setup failures with PR URL, exact head and message;
if neither service can review, wait for the owner's terminal Claude review.
No response is not a completed clean review. Do not independently launch
terminal Claude or create/message another chat. Request fresh exact-head review
after fixes; all required checks and conversation-resolution gates still apply.
The subsequent direct owner instruction resumes the EPOCH slice and retains
both EPOCH and CENTURY (see selected work below), without authorizing unrelated
production work. Reconfirm review availability/routing
after October rather than assuming quota restoration.

## Last shipped slice

BITCLEAR Numeric/exact PR#7121 merged2026-10-09T20:16:11Z as
53cf42c99e61491598ee4ae307d6c10ca5ccdd7d from signed/DCO G head
25c49fa32b5eb1cb01583b1620ff0f93fd4bb6f5.
https://github.com/rhamenator/Project-Copperfin/pull/7121
Merge gate6088531116: fresh active/strict ruleset20356131, all11 requiredSUCCESS,
all35 completedchecksSUCCESS; sole optional Windows Native37979321071/
job113985504327 remainedpending, notfailed. Gate checks ran20:15-20:16Z beforemerge;
gatecomment timestamp corrected to thatactual interval aftermerge.
Actual clean Codex6088134043 at19:49:55Z names25c49fa32b, following
ClaudeFIRST6087658191 at19:18:57Z and owner-authorized fallback6088108815
at19:48:13Z after bounded29-minute no-usable-review wait. Claude[bot] eyesonly;
no explicit Claude quota/setup failure known. Summary6087653212/reactions were
NOTused as clean evidence. Paginated RESTreviews/inlineempty, Graphthreads0/
closingrefs0/hasNextfalse; parents5611/6776/family6871 OPEN/owner/agent-approved.
No family/parent closure: BITTEST Numeric/exact remains next.

Independent WineVFP9 7423/shipped BITCLEARhelp BEFORE mappedRQ/helpers/tests;
five matching188-line/186call outputs. Frozen206direct/800freshpublic bothmodes/
sessions+2unchangedneighbors; BOTH complete GNU14.56s/Clang25.42s originals
552byteidenticalselectedFAIL/zeroother BEFORE BITCLEAR-only migration.
No frozenrepair/removal/weakening/refreeze. Checked selectedmode/valueANDposition
evenafter0/localized11/defineduint32ANDcomplement/int64signed32result.
FixedGNU2/2PASS14.72s/broader19/19PASS461.97s includinglocalization/safety/seven
contracts, safety1200cap preserved. ClangASan/UBSan/float-cast-overflow11/11PASS
190.94s/zero diagnostics/leaksdisabled. RQ-CF-PRG-BITCLEAR-NUMERIC-001/native/
frozen/rawresults/hashes and completedboundedmedium DQ/DV developmentself-review/
walkthrough/rollback retained in tests/fixtures/vfp9-bitclear-numeric.
Signedpostpush16/16PASS119.83s actual206/800 proof6087695637; full201-line raw
log SHA4dee15590bbf1487c9fb673be8d9db79a0b59dab8e90f89555dbffe7a3cf99ca.
Not independenthuman/highhazard/type/Currency/binary/arity/fullnative/platform/
leak/release/family/parentacceptance. NoKBX/apparentargument-drivenwork; activehazardsremain.

MainFF53cf42c99e; completedWT/local/remote branch removed. GNU445MB/Clang368MB/
nativeFXP scratch recoverablytrashed under
/home/rich/.local/share/Trash/files/bitclear-gcc, bitclear-clang, bitclear-native.
Signedpostpushhash verifiedinTrash. EmptyownedRAMTMPDIR
/tmp/copperfin-bitclear-5611-wt0HXH removed after emptydirectory check.
Ownpre-sync handoffpathstash159e09dc4d0d1d3090e123ae1a34e5869483fe16 retained;
allolderstashes/foreignfiles/WTs preserved. Requiredwatch90485exit0/all11PASS.
Oldallcheckwatch38929 stopped143 ONLYforWTcleanup, nohostedcancellation,
rawloginTrash. Replacementwatch97374 completedexit0/all36SUCCESS20:45Z,
hostedproof6088967587 published; canonical build/bitclear-hosted-watch scratch
recoverablytrashed. Zero newconversations/failures; hosted boundedproof complete.
PRattached; onlyolder verifiedMERGED6664 taskreference unlinkedat100cap,
noGitHubPR/datamutation. NoVMused/noownerinputblocker.

PriorBITSET7119 merged1d23bd7a9 fromsignedhead e4d422a7236bebad9c31eb24a05a491a5cc6a564;
clean6084799834/gate6085250636/postpush6084333446 and committedfixtureproof retained.
All35otherchecksSUCCESS; optional Linux37954206752/job113900437150 cancelled
21:47:01Z after6h0m37s. Annotation: The job has exceeded the maximum execution
time of6h0m0s. Raw193-line linux-installer-cancelled.log in canonical
build/bitset-hosted-watch SHAcb8be04b5b3409e75053d526145cc46600cc9bc1a185ac64d8922dbc1c78424e.
APTupdate lastoutput15:46:47Z/ignoredAzuremirrorindices; next21:46:59Z cancelled,
beforeANYCopperfinconfigure/build/package/lifecycle. Mirror/networkrootcause
unconfirmed. Watch17765 completedexit0butexplicitfailtable NOTsuccess evidence.
SpecificsameheadLinux-only rerun requested21:49Z; attempt2 job114041218199
started21:49:33Z/SUCCESS22:07:30Z. Freshsameheadall36checksSUCCESS22:23Z;
proof6090251177. Windows/macOS oldsuccessfulexecutions retained, notrerun.
Watch59504 harvestedexit0; all3rawlogs/hashverified recoverablytrashed as
/home/rich/.local/share/Trash/files/bitset-hosted-watch.
Rerunraw6862a9be311b663f5890a50300d203e937ec4b1e64bcd11a04d07c4dcbf9f18f;
first193-line cancellationhashunchanged. Focusedunadmittedissue7122 filedagainst
main53cf42c99e/workflowblob82090da1c675cebfa9638821b85d2720fb381e6b;
OPEN/rhamenator/nolabels. Noinstallerproductionchange/activeBITTESTexpansion.
Incident/rerun PRcomment6089861705 at21:51:40Z. Trashbitset-gcc/-clang and
stashbcb4ffa425bcafcbed71b2c06cd735578ebc916a retained.
PriorBITXOR7116 merged8b2a28ac4/all36SUCCESS proof6082816795/gate6082720941/
clean6082286175; BITOR7112 all36SUCCESS proof6080220178/gate6079783442/
clean6079409420; BITAND7110/BITNOT7107/Collection7105/GETBAR7103/PRMBAR7101 complete.
7115/7111/7108/7106/7104 remainunadmitted. EPOCH/CENTURYcomplete/#3698closed.

## Selected active bounded slice

BITTEST Numeric/exact value and position under admitted5611/6776/6871.
Parentmetadata OPEN/rhamenator/agent-approved22:15Z; admittedbodies reread21:15Z,
livechannel empty22:15Z. Dedicatedbranch fix/bittest-numeric-5611 from
origin/main53cf42c99e61491598ee4ae307d6c10ca5ccdd7d in
/home/rich/.codex/worktrees/bittest-numeric-5611/Project-Copperfin.
BITTEST-onlymigration nowapplied AFTERBOTHcomplete refrozenoriginals; no commit/PRyet.
ScopeONLY selected
Numeric/exact valueandposition consumers; type/Currency/binary/arity/sharedoldbit
helpers/siblings/globalNULL andfamily/parentacceptance remainseparate.
Independent WineVFP9 7423/shippedBITTESThelp55285ec7 recoveredfirst: five
matching1304-line/1302call outputs (initial+3fresh+copy), all32bits of36values,
zero/-1/alternatingpositions andmixedcontrols. NativeSource4debd560d80feeff9c9f71731261433a7ba8ee09524b449e65897ca9a9278f82;
outpute42dc284409db35d008269cb1e181f637ae7c4e3bcfcfe5e7283ea8cd3fbda99.
RQ-CF-PRG-BITTEST-NUMERIC-001 gapmapped BEFORE distinct unusedhelpers/tests,
then206direct/5264freshpublic/helperheader/CMake/isolation frozenBEFORE config.
Allhashes/initialREADME/audit in tests/fixtures/vfp9-bittest-numeric.
GNUoriginal build88839 exit0; completeCTest84631 expectedexit8,493.42s,
actual206direct/5264public/2954selectedFAIL (1230error/1684result/40kind),
zeroother/direct/control/context/reset failures; neighborsPASS0.12s.
Full3005-line raw original-gcc-unsharded.log SHA9726dd21446ace386a8ad4bdceae42535e2d5a330a0f16ab497477b3e20f48c4.
Allfrozen/shared/ORIGINALdispatcher hashes unchanged. Clang21.1.8
configure/bothbuilds succeeded, matchingC/CXX/EXE/SHARED ASan/UBSan/
float-cast-overflow/framepointer/leaksdisabled. OriginalCTest91546 exit8:
NumericTIMEOUT1200.18s/total1200.29s; finalcounterNOTreached, notvalidbaseline.
Observed2883selectedFAIL/zeroother/zero sanitizerdiagnostics; neighborsPASS0.10s.
Full2933-line original-clang-timeout1200.log retainedSHA
b11359449e605a5d1820e1d53d383842b7cf46cb5d273e22e6a86d72564b678b.
Unchangedrepeat61627 deliberatelystopped onlyverifiedownPID1922839 at33:04/
33:03CPU; subprocess terminated/exit8/1984.46s total, neighborsPASS0.09s.
No finalcounter; incompleteNOTbaseline.2657-line unedited
original-clang-unsharded-cancelled.log SHA88f4e2a70132c36c76eeac89f6932a06a932ada0aa9693465c5427aafe621cd6.
CPU-bound/privateRAM/availablememory observed, rootcause/diskunqualified.
ExplicitHARNESS-ONLYrefreeze22:23Z BEFOREnewconfig:206directoneprocess,
4x1316public CTestprocesses bymode/session, exactcounts failclosed, invalidargs2.
Everyliteralinput/expectedresult/existingassertion/freshsession unchanged;
sourceprefix1..1469 hash1e7e70f09b28a4384af2401d356ba363297e32709b1954a0b4301992102c3605 before/after.
Newtest/CMake/isolationhashes in baseline-audit.md; helpers/header/native/
neighbors/shared/ORIGINALdispatcherhashunchanged; no migration.
GNUrefrozenconfigure/bothbuilds28785exit0; all6CTestregistered/isolationcomplete.
CompletefreshGNUoriginal26323exit8 expectedselectedonly/144.36s, all206+4x1316
casescomplete, direct/neighborsPASS.2954 byteidenticalselectedFAIL tooldGNU,
zeroother, raw3049lines SHA8d8bc64e1aacc583e58fb1a2674daa07fbceb89b1eb06dab9b7edf86a0d15080.
Clangrefrozenconfigure/bothbuilds28658exit0; all6registered/matchingflagsverified;
CompleterefrozenClangoriginal48008exit8 selectedonly/324.57s, direct206PASS,
all4x1316publiccomplete/neighborPASS/2954selectedFAIL byteidenticaltobothGNUlogs,
zeroother/nosanitizerdiagnostics/leaksdisabled.3049-line rawSHA
367f73aa0564257f3af68013dfcc172400946053abed7c4754d82b850ceafcd1.
Retain BOTH COMPLETE refrozenGNUandClang originals and byteidentical2954
selectedFAIL/zeroother BEFOREmigration; oldpartialruns neverclean evidence.
Exact64/NaN/defaultsafety derivedfromownerparent; safeothercontrols preservationonly.
Bothcompleteoriginals retained BEFOREmigration; parentsrevalidated22:34Z.
OnlyBITTEST selectsbehavior/checksvalueANDpositionevenafter0/localized11/
defineduint32shiftAND/Logicalresult. FixeddispatcherSHA
0fe605bef5934d32318c2401d092a62a6bf54f3d03935e82ae603b3d0d6b2b09;
all6refrozeninputs/native/sharedplatformunchanged. Newchangelogfragmentrecorded.
FixedGNUrebuild/focused85647exit0/full6PASS149.78s/actual206+4x1316,
zeroassertionfailures, raw90lines SHAe89aca918f3c2bdb3bfa1b364b79471f4b10e66028973889461129adf07caf24.
BroaderGNU82444exit0/25of25PASS631.19s, actual206+4x1316complete again.
Safety350.81s existing1200cap preserved; localization14.36s/olderNumeric67.51s/
NULL/bitfamily/sevencontractsPASS. Raw302lines SHA
69aadf457880d331e23f809ef5eeaf68918f3f5fec8790630e29b3183a397ce9.
Supplementalinstalled-onlyposition-indexprobe AFTERGNU: same36positions x32
literalonehotmasks/1152calls/1154lines. Fourfreshserialbounded25sWineoutputs
matchall26admittedexactindices/10all32maskerror11; existingfrozenexpectations
unchanged. Sourcee52b32a84e6ef6ee2758bce86bf2fcf4954efa4e8608d569588d3c93bee27d63;
all4output68d87cd9511bbbf4c039ddbfa18b3068cbaa01fc7e330d2327d9c0849b6220cb.
NoVM/concurrentbuild/tests. NativeFXPscratchonly; explicit chronologyinaudit.
FixedClang90913exit0/full17PASS526.10s, actual206+4x1316complete/noassertionfailures
orASan/UBSan/float-cast-overflow diagnostics; matchingflags/leaksdisabled/
sameprivateRAM/CLI1200 operationalcap. Raw209lines SHA
dbee98ccdcdc33117af48462ca70e7a122a4653856d145541db3f713b215eeed.
CompletedboundedmediumDQ/DV delegateddevelopmentself-review/automatedGNU25+
Clang17walkthrough/rollback inaudit; noindependent-human/highhazardqualification.
RQdefinedforboundedlocalNumeric/exactscope, native/shared/frozenhashesunchanged;
no family/parent/fullnative/type/Currency/binary/arity/platform/leak/releaseacceptance.
Originmainstill53cf42c99e/freshfetch23:05Z, nobranchPR, livechannel empty.
Next: signed/DCOcommit/push/PRbase main, ClaudeFIRST, signedpostpushfocus,
requiredhostedchecks/exactheadreview/conversations. No localbuild/testrunning.
ClaudeFIRST thenverifiedauthorized@codexfallback; cleanexacthead review/
all11requiredgreen/resolvedconversations beforemerge.
BITSETinstaller reruncomplete/all36SUCCESS, rawpreserved/cleanupcomplete;
incident7122 stillOPEN/separate/unadmitted; missingbootstrapboundNOTfixedbyrerun.
Noownerinputneeded. PrivateRAMTMPDIR /tmp/copperfin-bittest-5611-iqIncd,
newGNUCLI600s/ClangCLI1200s PER SHARD operationalcaps only. Focusedregex
^test_prg_engine_bittest_(numeric.*|neighbors)$ coversall6; no workflow/testdefinition
timeoutchange/disk/persistence/timeout-optimumqualification. NativeFXP build/bittest-native.
NoVMstart/recreate whilebackupreset completionunconfirmed. Wineprobe available;
noownerinputblocker. Allcompleted BITCLEAR work recoverable asabove.

## Retained owner steering / deferred work

Audit candidates7100 lifecycle output-drain timeout and7102 designer-smoke
descendant cleanup were OPEN/rhamenator/agent-approved at00:47Z; revalidatebefore
intake. Deferred after boundednumeric unless directownerredirect or activeCI
actionablefailure. Not explanations/fixes for7053; timeoutincrease notacceptance.
Audit chat owns discoverymetadata/cloudhunt; don'tduplicateitswork.
Hosted VSIX40min ownerpolicy shipped7099, notmeasuredoptimum/rootcause.
LocalVSIXtiming deferreduntil safe fullindependentclone/capacity.
#6879 after numeric. Diskcontention requiredownerreboot; frugalbriefVMuse.

2026-10-09 owner VMcleanup steering verified in SoundCurrent equalizer development
thread01a10e8f-2124-7e40-ac3c-1d715f4cf102, directhuman turn01a12105-94f4-75e1-941e-6549e2bf6af0:
instructactive/scheduledtasks todeleteredundantVMs beforebackupreset.
Do notstart/recreateVMs duringbackupreset. Remove ONLYtask-owneddisposableclones
after exact shutoffdomain/disk/snapshot/shared-reference/backing checks and
preservingrequiredguest-onlyevidence. Preserve originals copperfin-vfp9-win11,
copperfin-access365-win11, pristine copperfin-clean-win11 and recoverycopies.
DAWtask owns soundcurrent-daw-capture-win11/install-win11; leaveitscleanup to it.
Task-owned VSIX clone copperfin-vsix-timeout-20261008 neverdefined/booted; owned
interruptedcopy/conversion recovered from actualtaskhistory turns01a11ca6 and
01a11cb9. Bothpartialqcow2 files in Trash/copperfin-skpbar-5611-build permanently
removed: incomplete-vsix-clone-copy.qcow2 and incomplete-vsix-clone-conversion.qcow2.
Allocated6406905856+4399771648=10806677504 bytes (~10.06GiB) freed.
Verifiedregularfiles/rootowned/linkcount1/inodes12193233and12193232, no running
copy/convert/openfile users, domainabsent, no internalqcow snapshots/backing,
no currentdomain/snapshot XMLreferences; currentregistereddisks, ready overlays,
and originalrecoveryqcow backing chains do notdependonpartials. AllVMsshutoff.
No guest-onlyevidence: no boot/installtest everran. Hosted/Wine/repo evidence,
scripts and rawlogs retained. UnusedcloneNVRAM movedrecoverably to
/home/rich/.local/share/Trash/files/copperfin-skpbar-5611-build/unused-vsix-clone_VARS.fd,
SHA1fe3267e38853bcca4d499dfbdf33f978af316b718fbe4b08d1ece4649ac56e8.
The two partialdisks are not Trash-recoverable; completeoriginals/recoverycopies
unchanged, no existing backup-recoveryclaim. No task-owned VM remainsdefined.
Backupreset coordination is additive, notcancellation of selected numeric work.

PR #6926 (`fix/array-dimension-overflow-5594`) merged into `main` as
`2c56331d57c0ef459ae55d38236a012b77c129b1` on 2026-10-04 and closed #5594.
It completed the array-dimension safety slice under #5611/#6776 with checked
dimension conversion, checked size/offset arithmetic, a bounded host ceiling,
and failure-atomic declaration, resize, RESTORE, and native-class array paths
while preserving sequential multi-target visibility. All required checks were
green at exact head `1c2dfaa07`; exact-head review was clean and all review
conversations were resolved before merge.

PR #6923 (`fix/bintoc-ctobin-contract-5766`) merged into `main` as
`5d64d0e97b3a5a6baf9ed91119e954035e6a2a09` on 2026-10-04 and closed #5766.
It completed the `BINTOC()` / `CTOBIN()` numeric-conversion slice under
#5611/#6776 with bounded flag/width validation, canonical sortable and native
byte order, exact Currency handling, and localized catchable errors. All
required checks passed at exact head `a813944b0f`; all review conversations
were resolved before merge.

PR #6920 (`fix/declare-numeric-boundaries-6050`) merged into `main` as
`a2641cc9a9a03c2dfd5254fdad8f6d6be9f7994f` on 2026-10-03 and closed #6050.
It completed defined native/managed `DECLARE` integer conversion under
#5611/#6776, including low-32-bit VFP9 conversion, checked 64-bit admission,
native by-reference initial values, portable source contracts, and focused
Windows native/managed fixtures. All 33 checks passed at exact head
`8d8294ce4c9b72eda2b3bd53dd68f0d75f1da22e`, exact-head review was clean, and
the sole review conversation was resolved before merge. The review fixes made
finite out-of-int64 INTEGER conversion mode-aware and made the Windows-only
marshaler read `NUMERICBEHAVIOR` from the session SET state in scope.

PR #6917 (`fix/installer-artifact-policy-6905`) merged into `main` as
`6b64a176d680aab732e0772cd889a423ed3c993e` on 2026-10-03. Successful
pull-request installer packages/evidence/source now retain for 7 days,
successful non-pull-request outputs for 30 days, and exact failure packages
plus available diagnostics for 14 days; the immutable RC bundle remains a
separate 90-day artifact. All 36 checks passed, exact head `ca3bbd3cb` passed
Codex review, and all four review conversations were resolved before merge.

PR #6916 (`fix/linux-rpm-lifecycle-6905`) merged into `main` as
`ec556e17214266274508c430529a9f3912805e19` on 2026-10-03. It completed the
fourth #6905 installed-product slice with a distinct native RPM lifecycle in an
ephemeral digest-pinned Fedora `linux/amd64` container: exact package and
container-image identity, fresh native install, installed
tree/catalog/semantic command smoke, `rpm --verify`, same-version reinstall,
external-fixture preservation, erase, and RPM-database/filesystem residue
checks. Exact head `a5ca7e859` passed all 36 checks, including the native RPM
lifecycle; all four review conversations were resolved before merge.

PR #6910 (`fix/windows-installed-ui-6905`) merged into `main` as
`ce15bab04fc14db26c81081b1dc5f067b7fc6c88` on 2026-10-03. Hosted run
`37127435107`, job `111215503606`, passed the complete Windows installer and
installed-Studio UI lifecycle at implementation head `2806a4a31`; the
retained evidence binds installer SHA-256
`402a4dc2a399b92328ed768db8334590cefd34c7b174c62b761c62ef9d44763c` and
installed Studio SHA-256
`2d939edb402d31abd126322ed4567919be817dabbcc59b96ed31d8fb3a99b2f9`.
All four review conversations were resolved, exact-head review found no
further issue, and the final docs-only head passed all 33 checks. #6911 owns
the inherited hosted-runner authority gap and #6913 owns standard accessible
roles; human GUI remains `NOT_RUN`.

PR #6908 (`fix/macos-installer-lifecycle-6905`) merged into `main` as
`ed9a12512761197f7214b638bf43f413d0917f56` on 2026-10-03. Hosted run
`37112542106`, job `111173111329`, passed the exact lifecycle at implementation
head `354c35105`; retained evidence binds PKG SHA-256
`4e8f9556d1fd2a333a572e674fd24d37ba2a0e7ad2ffa96def14f0456fe07f38`.
Final docs-only head run `37114247412` also passed all installers; all review
conversations were resolved before merge.

Issue #6905's first installed-product slice (PR #6906, merge `3af2b9609`)
finished 2026-10-03: the exact generated Linux DEB now undergoes a disposable
hosted-runner fresh install, installed-tree/English-catalog/semantic CLI smoke,
same-version reinstall, external-fixture survival, purge, and residue checks.
Digest-bound evidence is admitted into the RC bundle; all six review threads
were resolved and the exact lifecycle plus required checks passed.

Earlier on 2026-10-03, PR #6894 merged into `main` as `5807fbedf`. It
completed the `CHR()` numeric-boundary slice under #5611/#6776, including
VFP9-mode modulo wrapping, Copperfin-mode out-of-range rejection, focused
sanitizer coverage, and the review-requested additional wrap cases. All review
conversations were resolved and all required checks passed.

2026-10-02 session (all merged into `main`): cluster 25 finished (#6035 via
#6769, plus #6755 and #6760); installer lifecycle CI fixed (#6761, #6696);
macOS lane made green (#6762 `VAL(-1E-308)`, #6764 PowerShell host discovery,
#6768 invalid-UTF-8 fixture); Windows UTF-8 test fixtures fixed (#6774, #6767);
string length bounds (#6775: #5595, #6003, #6004); `SET NUMERICBEHAVIOR` and the
`LEFT`/`RIGHT` family (#6777); `STUFF`/`STUFFC`/`SUBSTR`/`SUBSTRC` start
boundaries, `STR` decimals above 18, `GETWORDNUM`/`MLINE` index, and defined
saturating conversions across the string functions (#6778, #3704). #5598 was
fixed by #6769 and is pinned by script rows. Evidence for every VFP9 claim is
retained under `~/temp/vfp9-probes/` (see Probing VFP9 below).

Issue #6593 (PR #6683, merge `fe91230ea`) finished 2026-09-27: `APPEND FROM
... TYPE CSV` now discards BOM-free field-name rows when selected target names
appear in a different order, while subsequent values remain positional in both
local DBF and selected SQL/result cursor paths. Focused and hosted checks
passed; review completed without actionable conversations.

Issue #6598 (PR #6681, merge `af0a22dd3`) finished 2026-09-27: `APPEND FROM
... TYPE CSV` now appends blank physical data records after the header in both
local DBF and selected SQL/result cursor paths, while header-only sources
remain zero-row and DELIMITED behavior is unchanged. Focused and hosted checks
passed; the channel-mirror review finding was fixed and resolved.

Issue #6602 (PR #6680, merge `2bcc34c6c`) finished 2026-09-27: `APPEND FROM
... TYPE SDF` now recognizes CR-only physical records alongside CRLF and LF
without changing blank-record or Memo-target behavior. Focused and hosted
checks passed; review completed without actionable conversations.

Issue #6519 (PR #6678, merge `37a77a193`) finished 2026-09-27: unquoted
Character data in CSV and DELIMITED imports now preserves following literal
quote bytes while retaining each format's enclosed-field rules. Focused and
hosted checks passed; both review conversations were fixed and resolved.

Issue #6522 (PR #6670, merge `b3c1ed73d`) finished 2026-09-27: enclosed doubled
quotes in `APPEND FROM TYPE CSV` now preserve both VFP9-observed quote bytes for
fixed-width Character targets without changing DELIMITED, TAB, unquoted,
Varchar, or Memo behavior. Focused and hosted checks passed; review completed
without actionable conversations.

Earlier: issue #6665 (PR #6667, merge `d99c78ac2`) finished 2026-09-27:
enclosed doubled quotes in `APPEND FROM TYPE DELIMITED` now follow VFP9's
format-specific truncation rule without changing TAB or unquoted-field
handling. Focused and hosted checks passed; all review conversations were
resolved.

Earlier: issue #6604 (PR #6666, merge `780d6ac12`) finished 2026-09-27: SDF
Logical export now emits VFP-compatible uppercase `T`/`F` bytes while
preserving adjacent fixed-width Numeric formatting. Focused and hosted checks
passed; the two review documentation mismatches were fixed and their
conversations resolved.

Earlier: issue #6623 (PR #6664, merge `91c0f7165`) finished 2026-09-27: blank SDF
Numeric/Float cells now import as non-NULL zero at the target field's scale,
including nullable Numeric targets, without shifting later fields. Focused and
hosted checks passed; all review conversations were resolved.

Earlier: issue #6636 (PR #6663, merge `21ba05355`) finished 2026-09-27:
nullable DIF numeric, Character, and Logical cells now match VFP9, while SYLK
emits blank `K` cells without shifting later fields. Review added explicit
stale-physical-byte protection for nullable Character/Varchar fields. Focused
and hosted checks passed; all review conversations were resolved.

Earlier: issue #6609 (PR #6662, merge `c3e62e4e7`) finished 2026-09-27:
text-to-Memo SDF/CSV imports now use an explicit session-scoped VFP/Copperfin
policy, strict CSV enclosure preflight, localized pre-mutation diagnostics,
and focused DBF/FPT atomicity coverage. Focused and hosted checks passed; all
review conversations were resolved.

Earlier: issue #6612 (PR #6661, merge `9b028f5eb`) finished 2026-09-27: SDF
Autoincrement fields now use VFP's 11-character printable layout while
preserving genuine `0x31` metadata and raw bytes. Focused and hosted checks
passed; all review conversations were resolved.

Earlier: issue #6614 (PR #6660, merge `cf2b95187`) finished 2026-09-27: SDF now uses
Varchar/Varbinary payload widths, uppercase hexadecimal Varbinary text, and
validated case-insensitive hex import with command-atomic rejection of bad
input. Focused and hosted checks passed; the review diagnostic fix was
verified and its conversation resolved.

Earlier: issue #6628 (PR #6659, merge `59fe248ed`) finished 2026-09-27:
DIF/SYLK Date and DateTime export now uses VFP-compatible typed cells,
including blank DateTime handling and the Excel-1900 leap-day discontinuity.
Focused and hosted checks passed; all review conversations were resolved.

Earlier: issue #6630 (PR #6658, merge `0b2a52244`) finished 2026-09-27: CSV and
DELIMITED export/import now project Blob (`W`) fields with the same native
rules as General/Picture, including the parallel SQL-result import path.
Focused and hosted checks passed; the review conversation was resolved.

Earlier: issue #6640 (PR #6657, merge `e3b765653`) finished 2026-09-27:
successful lossy `COPY TO` interchange projections now emit one localized,
non-blocking warning with stable structured output-type and ordered omitted-
field metadata, without disclosing payloads or changing output bytes. Review
extended the event metadata through debugger, Visual Studio, and headless host
protocols and added actual `TYPE TAB` coverage. Focused and hosted checks
passed; all conversations were resolved.

Earlier: issue #6654 (PR #6656, merge `c0a003b43`) finished native
CR-delimited DIF/SYLK imports with Varbinary targets and atomic rollback.

Reentrant-cursor-closure cluster 1 (`docs/81`), cursor-lifetime part,
finished 2026-09-24: #6321 (PR #6521), #6322 (PR #6524), #6331 (PR #6526),
#6242 (PR #6529), #6241 (PR #6534), and #6240 (PR #6536, which also carries
the `docs/81` status update and the changelog catch-up). All
merged into `v1-development` with their issues closed manually. Shared
pattern: capture a `CursorGenerationReference` before any user-code
evaluation and re-resolve it afterwards (plus `current_data_session` where
the command must stay in its session); on loss, raise the catchable
`{command} target work area not found` error before touching the cursor.
Lessons from the review rounds: (a) positional rollback of a provisional
row is unsafe once callbacks can PACK the target, so evaluate against a
`RecordEvaluationOverride` instead (#6322); (b) a leading-`&` visibility
expression is evaluated twice, so the shared evaluator re-resolves between
passes; (c) a new loss flag on a shared helper must be threaded to every
caller (GO/SKIP/relation walking), not only the one under repair. Each fix
was verified fail-then-pass with a local Clang ASan/UBSan build; after
halt-on-error stops at the first failure, isolate newly added scenarios to
prove each one independently.

Earlier: PR #6505 fixed #6047 (nullable DBF writes silently losing NULL,
storage layer; `RQ-CF-PRG-059`), with #6506 filed for the `PrgValue` NULL
semantics redesign (not started). #6496 (PR #6516) and #6497 (PR #6517)
have since merged too.

Earlier: PR #6503 fixed #5567 (legacy DBF import silently reactivating
deleted source records, merged `565d66f32`) and PR #6502 fixed #5631
(JSON export of an unknown/NULL logical value as `false`, merged
`232b7e704`); both issues closed manually.

Earlier still: PR #6500 fixed #6495 (concurrency state-sequence hunt,
child of umbrella #6498, merged `bac49af30ba1e80e2707d6d1d1a8a7b4fa0d19e4`);
#6495 closed manually. Surfaced and filed #6499 (cancellation inside an
explicit `FLOCK()`/`RLOCK()` retry loop silently swallowed instead of
halting), deliberately not fixed as part of that coverage-only slice.

Earlier shipped slices (#6459/#6458/#6460/#6389/#6388/cloud-validation/#6251)
all merged; #5680 remains open (partial). PR #6486 was closed without merge
after review found a macOS clone destination-identity gap.

## Active slice

Open owner-authored, `agent-approved` issue #5611 is the implementation
umbrella and reopened #6776 is the design/checklist reference. The `RGB()`
component numeric-validation correction merged in PR #6939 at `69b3c1510`
and closed #6938. All 33 checks passed at exact head `54c2d3225`, exact-head
review was clean, and there were no review conversations. The Currency-first
`MOD()` Numeric-divisor conversion then merged in PR #6941 at `3f48b096f`.
All 33 checks passed at exact head `2136b52de`, exact-head review was clean,
and all six review conversations were resolved after their fixes were verified.
The completed worktree, branch, scratch probe, and build directories were
cleaned up after merge.

The `RAND()` seed numeric-conversion slice merged in PR #6942 at `7ba689c48`.
All 33 exact-head checks passed and its one review conversation was resolved
after fresh VFP9 evidence proved the intended truncation-first ceiling. The
completed worktree, branch, probe, and build directories were cleaned up.

The `FILE()` / `DIRECTORY()` visibility-flag numeric-conversion slice merged
in PR #6947 at `68178c648`. All required checks passed at exact head
`cde062b21`, optional installer/package checks were also green, exact-head
Copilot review had no findings, and there were no review conversations. The
completed worktree, branch, scratch probe, and build directories were cleaned
up after merge.

The `FDATE()` optional type-flag numeric-conversion slice merged in PR #6948
at `ae4fdc5ab`. All required exact-head checks passed at `f9611161e`, all
three review conversations were resolved after their fixes were verified, and
the completed worktree, branch, scratch probe, and build directories were
cleaned up after merge.

The `FOPEN()` optional numeric mode-conversion slice merged in PR #6950 at
`f8877f04d`. All required exact-head checks passed at `6097cb7a0`, automated
review reported no findings, and there were no review conversations. The
completed worktree, branch, scratch probe, and build directories were cleaned
up after merge.

The `FCREATE()` attribute-conversion slice merged in PR #6952 at
`2a99e37f9`. All 11 required checks passed for final commit `1ccee0e4c`,
review found no further issue, and both conversations were resolved before
merge. The completed worktree, branch, probe and build directories were cleaned
up. Optional Windows MSVC was still running at merge; all other checks passed.

The `FSEEK()` offset/origin numeric-conversion slice merged in PR #6957 at
`7d4d77867`. All 11 required checks passed at exact head `a16e76fdf`, review
reported no findings and there were no conversations. Optional Windows MSVC
was still running at merge; every other check passed. The completed worktree,
branch and build directories were cleaned up. Failed-seek return parity is
separate gap #6956.

Shared file-handle numeric conversion merged in PR #6960 at `cbd5ee9b6`.
All 11 required checks passed at final signed head `4b8ca7248`; exact-head
review was clean, and the sole conversation was resolved after the
nonzero-FERROR preservation test fix was pushed and verified. Optional Linux
GCC, macOS Clang and Windows MSVC were still running at merge; other checks
were green. Main was synchronized, and the completed worktree, branches and
build directories were cleaned up (builds moved to recoverable trash).

`FCHSIZE()` size conversion merged in PR #6962 at `146367518` with all
11 required checks green at final signed head `1e74778d1`, clean exact-head
review and the sole conversation resolved. Optional Windows MSVC was still
running at merge; every other check passed. Main was synchronized and the
slice worktree/branches removed; builds were moved to recoverable trash.
The 101-call installed-VFP9 fixture and 56-row native/verified two-mode matrix
remain retained. Both focused suites pass normally and under Clang
ASan/UBSan/float-cast-overflow; old dispatch reports 366 failures and sanitizer
stops at 1E300. An unrelated macOS .NET-candidate test failed once on the
documentation-only head and passed a same-head job retry; diagnostic follow-up
#6964 is open, with no .NET changes in this slice. Return gaps
#5912/#5913/#5914/#5887/#5911/#6956/#6959 remain separate.

HEX numeric conversion merged in PR #6965 at `44d399248` with all 11 required
checks green at signed head `8c41ea70a`, clean Codex/Copilot exact-head reviews
and no conversations. Optional Windows MSVC was still running; every other
check passed. Main was synchronized and the slice worktree/branches removed;
both builds moved to recoverable trash. The 62 direct calls and 14 PRG rows
pass normally and under Clang ASan/UBSan/float-cast-overflow; old dispatch
reports 36 failures and sanitizer stops at the signed-64-bit ceiling.
review and no conversations. Optional Linux GCC and Windows MSVC were still
running; every other check passed. Main was synchronized, the completed
worktree/branches removed, and builds moved to recoverable trash. The retained
24-row installed Numeric probe supports truncation, positive non-wrapping and
negative wrapping/clamping only; operation/return gaps #6945/#6966 remain
separate. Its 66 boundary calls, two operation controls and 11 PRG rows pass
normally and under sanitizers. Old dispatch fails 26 semantic assertions;
unsupported FE_INVALID assertions were removed after optimized macOS ARM64
showed non-strict FP speculation (fixture README retains the diagnosis).

CPCURRENT Numeric/exact-integer selector conversion merged in PR #6969 at
`09555b5502a517777642fe8bf67411a90f66164c` on 2026-10-05. All 11 required
checks passed at signed head `8adbd5fefe60db8e24f868dc96a3ce4525c4dfd2`;
Codex and Copilot exact-head reviews were clean and there were no conversations.
Optional Windows MSVC was still running; every other check passed. Main was
synchronized, worktree/local/remote branches removed, and builds moved to
recoverable trash. Its 80 direct boundary calls, six query controls and 16 PRG
rows pass normally and under sanitizers; old conversion fails 86 assertions.
The 32-row native table remains retained. Query/type parity #6968 is separate.

Shared RELATION/TARGET Numeric-index conversion merged in PR #6972 at
`52daa2f7f6b39bfe232de3c65556be5aaf0fcdd6` on 2026-10-05. All 11 required
checks passed at signed head `0d1bd0e2e56a2bf4c1b88c07fce660b36cdc2e50`;
exact-head reviews completed without findings and no conversations remained.
Optional Linux GCC and Windows MSVC were still running at merge; every other
check passed. Main was synchronized, worktree/local/remote branches removed,
and both builds moved to recoverable trash. Its 68-result Windows native
fixture, 148 direct calls and 40 PRG rows remain retained; all three focused
suites pass normally and under sanitizers, with 208 old-dispatch failures.
Additive-order gap #6971 remains separate. Access365 finished its low-risk
temporary-cursor COM probe and is shut down; no overlay/system changes.

SET(TEXTMERGE) Numeric query-variant conversion merged in PR #6975 at
`91467cdc9219d0f0d579d3ee2c7a05ff6e6187d7` on 2026-10-05. All 33 checks
passed at signed head `167a2a53a51b50002ee88f76fc0706ea94d084a9`, exact-head
Codex review was clean and the sole handoff conversation was verified and
resolved. Main was synchronized, worktree/local/remote branches removed, and
both builds moved to recoverable trash. Its 34-result native fixture, 80 direct
calls and 20 PRG rows remain retained; focused normal/sanitizer suites pass,
with 102 old-dispatch semantic failures. #6973/#6333 remain separate.

ALINES Numeric/exact-integer flag conversion merged in PR #6978 at
`2a0c811dc422480402d325c3d11afb18457eec0d` on 2026-10-05. All 11 required
checks passed at signed head `ab33d01d944ca5e047f465e2be9916d68963ee07`;
Codex and Copilot exact-head reviews were clean and there were no conversations.
Optional Windows MSVC finished successfully after merge; all 33 checks passed.
Main was
synchronized, worktree/local/remote branches removed, and both builds moved
to recoverable trash. Its 36-result native fixture, 138 direct calls and
92 PRG rows remain retained; all three focused suites pass normally and under
sanitizers, with 52 old-dispatch semantic failures. No VM was used.

ADIR Numeric/exact-integer display conversion merged in PR #6980 as
`3aa571eaad1dd169b97491c5c3bd260882a2d4a2` on 2026-10-05 at 21:46:21 UTC.
All 11 required checks passed at signed head
`c3a37e04956ad707af7448954bc1bcbc9609faff`; exact-head Codex review was clean,
and all five Copilot conversations were verified/resolved. Broad native,
managed UI, sanitizer/fuzz/stress/migration and installer checks also passed
after infrastructure retries. Optional dynamic CodeQL actions/C#/Python jobs
could not acquire runners and GitHub rejects their rerun; C/C++ analysis passed.
Main is synchronized; the completed worktree/local/remote branches were removed
and 420 MB/787 MB builds moved to recoverable trash. Its 46-result native
fixture, 170 direct calls and 114 PRG rows remain retained; all three focused
normal/sanitizer suites pass, with 60 old-dispatch semantic failures. No VM used.

AFONT Numeric/exact-integer third-argument size conversion completed in PR
#6983. Its retained 54-call installed VFP9 fixture, 198 direct checks and 137
PRG rows are in `tests/fixtures/vfp9-afont-size-numeric-observation/` and
`tests/test_prg_engine_numeric_behavior.cpp`. Old dispatch failed 40
assertions; fixed normal suites passed 3/3 (7.20s) and Clang ASan/UBSan/
float-cast-overflow passed 3/3 (30.63s) without diagnostics. A handoff review
fix was verified and resolved; its changelog indentation failure was corrected
in the final signed head, which passed all 138 fragment checks and focused
5/5 validation (7.54s). Linux/macOS broad suites and VSIX lifecycle then
passed at the exact final head. Existing #3252/#6955/#6958/#6963 remain
separate; font/result/type/charset parity was not expanded.

FIELD first-argument Numeric/exact-integer conversion completed in PR #6989.
Its complete 58-call native fixture, 196 direct calls and 141 PRG rows remain
retained with docs/32 and docs/22 reverse links. Corrected old dispatch failed
41 semantic assertions (5.57s); the initial cursor-setup failure is explicitly
discarded. Normal GCC suites passed 4/4 (7.71s), and Clang ASan/UBSan/
float-cast-overflow passed 4/4 (34.02s) without diagnostics. Changelog,
contributor-signoff and channel contracts passed 4/4 (3.96s), with 139 valid
fragments. All required exact-head CI/review gates passed before merge;
completed optional native, managed, security, sanitizer/fuzz/stress/migration
and installer lanes passed. Windows MSVC subsequently passed; all 33 checks
completed successfully. FIELD arity/type #6987 and cursor allocation #6988 remain
separate unadmitted production work.

FSIZE first-argument Numeric/exact-integer admission completed in PR #6992.
Its 64-call native fixture, 81 direct calls and 151 PRG rows remain retained
with docs/22/docs/32 and completed README VR/DQ/DV/reproduction evidence.
Old dispatch failed 127 assertions (8.28s); five suites passed normally
(8.52s) and under ASan/UBSan/float-cast-overflow (38.95s), without diagnostics.
All four documentation/signoff/channel contracts passed (4.57s), with 140
valid fragments. All 11 required exact-head checks passed and Codex review was
clean with no conversations before merge. All completed optional checks
passed; Windows MSVC was still running at merge. Branches, worktree and builds
are cleaned as recorded above. Other type/arity #6990 and SET COMPATIBLE/file
semantics #6014 remain separate. No VM or persistent/system changes were used.

SELECT() Numeric/exact-integer selector conversion completed in PR #6995.
The retained 60-call native fixture, 164 direct calls and 126 PRG rows map to
RQ-CF-PRG-SELECT-SELECTOR-NUMERIC-001 in docs/22/docs/32 and completed README
VR/DQ/DV/reproduction evidence. Old dispatch failed 68 semantic assertions
(numeric 6.08s; four-suite baseline 35.01s); formatting-confounded output was
excluded. Normal suites passed 4/4 (35.55s); sanitizer suites passed 4/4
(137.19s), without diagnostics. Four contracts passed (3.91s), 141 fragments
validated. All 11 required checks passed, exact-head review was clean and no
conversations remained before merge; all completed optional checks passed,
with Windows MSVC still running. Query routing #6013, non-Numeric admission
#6994 and cursor allocation #6988 remain separate. FSIZE's optional Windows
MSVC subsequently passed; all 33 FSIZE checks completed successfully.

SQLGETPROP first-argument Numeric/exact-integer handle conversion shipped in PR #6998
under #5611/#6776 on `fix/sqlgetprop-handle-numeric-5611`, from main
`369d5f41bb162c909c497fc097c982957289d574`, in
`/home/rich/.codex/worktrees/sqlgetprop-handle-numeric-5611/Project-Copperfin`.
Only its llround/int site in prg_engine_expression.inl is selected. Other SQL
callers, property-value conversion, backend/connection/session behavior and
non-Numeric admission remain separate.
The complete 48-call installed VFP9 09.00.0000.7423 Wine fixture is retained in
`tests/fixtures/vfp9-sqlgetprop-handle-numeric-observation/`; two fresh
connection-free queries match all 50 lines. Fractions near zero truncate;
huge positive handles reject with 1466, whereas explicit VFP9 negative
low-32 aliases and huge/infinite zero remain observable through handle zero.
Nonzero absent-handle outputs do not independently distinguish every converted
index; exact int64/uint64 and NaN follow documented derived safety policy.
The checked helper is wired into only this expression dispatch. 134 direct
calls/119 final PRG rows use the existing synthetic in-process connection, not an
ODBC/network connection. Original dispatch failed exactly 43 conversion
assertions in both GCC (numeric 6.13s; full run 7.80s) and Clang sanitizers
(numeric 27.68s; full run 34.34s); all direct helper and preserved controls plus
three neighbors passed. No baseline sanitizer diagnostic was produced.
After this conversion change, normal suites pass 4/4 (7.63s; numeric 5.94s),
and sanitizer suites pass 4/4 (34.59s; numeric 27.33s) without diagnostics.
Native default/invalid-query and other-type admission gaps are filed separately
as #6996 against exact main 369d5f41, without an implementation-admission label.
Final diagnostic walkthrough found the shared Numeric formatter's unchecked
huge-finite llround (#6997). Four message rows were added after the original
115-row baseline; two finite-huge messages fail (5.86s) while infinite controls
pass. Only SQLGETPROP's rejection diagnostic now uses the existing safe
round-trip decimal formatter; the shared formatter/other callers are untouched.
Final normal verification with all 119 rows passes 4/4 (7.83s; numeric 6.11s).
Final sanitizer verification passes 4/4 (34.19s; numeric 27.45s), including
safe diagnostic regressions, without sanitizer diagnostics.
Changelog/live-channel/full safety workflow contracts pass 4/4 (360.90s),
final fragment checks pass 2/2 (0.45s), and all 142 fragments validate.
Normal/sanitizer builds are
`/home/rich/temp/copperfin-sqlgetprop-handle-5611-build` and
`/home/rich/temp/copperfin-sqlgetprop-handle-5611-sanitize`.
Completed README VR/DQ/DV, fixture identities and docs/22/docs/32 traceability
retain the conversion/query boundary and recovery limits. No VM was used.
PR #6998 merged at e3bdb6458 after all 11 required checks and clean exact-head
review, with no conversations. The slice worktree/branches are removed and
both builds are in recoverable trash; paths above are historical reproduction
locations. Unrelated files are preserved. The live channel was re-read empty.

SQLSETPROP first-argument handle conversion is now selected under #5611/#6776
on `fix/sqlsetprop-handle-numeric-5611`, from exact main
`e3bdb6458f6b1c5f801f530c29d06ee8fff6bf29`, in
`/home/rich/.codex/worktrees/sqlsetprop-handle-numeric-5611/Project-Copperfin`.
Only its expression-dispatch llround/int site is selected. Separate property-
value conversions, setter/default/backend results, connection/session state,
non-Numeric admission and all other SQL callers remain outside scope.
The complete connection-free native fixture is retained in
`tests/fixtures/vfp9-sqlsetprop-handle-numeric-observation/`; three fresh runs
match 52 lines (48 setter calls, VERSION, unchanged default before/after and
unchanged session 1). Its conversion boundary independently matches SQLGETPROP.
The new operation-specific helper shares the verified conversion only.
134 direct calls and 118 PRG rows are added; setter rows reset the synthetic
connection's Boolean property and verify returned status plus mutation state.
No ODBC/network connection or VM is used. Normal and sanitizer builds are
`/home/rich/temp/copperfin-sqlsetprop-handle-5611-build` and
`/home/rich/temp/copperfin-sqlsetprop-handle-5611-sanitize`.
Original dispatch fails exactly 47 new conversion/caught-message assertions
in GCC (numeric 6.49s; full run 8.27s) and Clang sanitizers (numeric 29.68s;
full run 36.72s); direct helpers, preserved controls, SQLGETPROP rows and all
three neighbors pass. No baseline sanitizer diagnostic was produced.
Only SQLSETPROP's first-argument dispatch is now checked, with safe Numeric
rejection diagnostics; property-value conversion and backend callbacks are
unchanged. GCC passes 4/4 (7.64s; numeric 5.93s); Clang ASan/UBSan/float-cast-
overflow passes 4/4 (34.64s; numeric 27.80s) without diagnostics. Runtime
tests ran serially across build directories. Changelog/signoff/live-channel
contracts pass 4/4 (5.65s); all 143 fragments validate.
The encountered default/invalid-setter and non-Numeric admission gap is
filed separately as #6999 against exact main e3bdb6458 without implementation
admission. The general shared formatter gap #6997 remains unchanged.
Completed README VR/DQ/DV, fixture identities, error-before-setter/property-
reset walkthrough and docs/22/docs/32 traceability retain the bounded scope.
SQLSETPROP shipped as PR #7000 at signed head cb920f30a8 with all 11 required
checks green, clean exact-head Codex review and zero review conversations.
Merged 2026-10-06 05:16:38 UTC as 4aef3ae3052a3d004da5dbe48bbbb8189a810cd1.
Main is synchronized; its worktree/local/remote branches are removed and both
builds are in recoverable trash. All completed optional checks passed; Windows
MSVC remained in progress. Unrelated files/worktrees are preserved and the
live channel was re-read empty.

SQLDISCONNECT first-argument handle conversion is now selected under #5611/#6776
on `fix/sqldisconnect-handle-numeric-5611`, from exact main
`4aef3ae3052a3d004da5dbe48bbbb8189a810cd1`, in
`/home/rich/.codex/worktrees/sqldisconnect-handle-numeric-5611/Project-Copperfin`.
Only its expression-dispatch llround/int site is selected; disconnect-all,
absent-handle/type result parity, backend/session lifecycle and other SQL
callers stay separate. No VM or external SQL connection is used.
The complete connection-free native fixture is retained under
`tests/fixtures/vfp9-sqldisconnect-handle-numeric-observation/`; fresh repeated
50-line output contains VERSION, 48 calls and unchanged session 1. It independently
matches the selected negative-only zero aliases observed for GET/SETPROP.
134 direct calls and 300 fresh-session PRG rows are added. Original dispatch
fails exactly 41 new conversion/caught-message assertions in GCC (numeric
6.40s; full run 8.22s) and Clang sanitizers (numeric 29.95s; full run 36.92s).
Direct helpers, preserved callback/coercion controls, existing GET/SETPROP rows
and all three neighbors pass; no baseline sanitizer diagnostic occurred.
Only SQLDISCONNECT first-argument dispatch is now checked with safe original
Numeric rejection text; disconnect callbacks and successful events are unchanged.
Final GCC passes 4/4 (9.22s; numeric 7.23s); Clang ASan/UBSan/float-cast-overflow
passes 4/4 (38.78s; numeric 31.45s) without diagnostics. Runtime tests ran
serially across build directories. Changelog/signoff/live-channel contracts
pass 4/4 (4.21s); all 144 fragments validate. Normal and sanitizer builds:
`/home/rich/temp/copperfin-sqldisconnect-handle-5611-build` and
`/home/rich/temp/copperfin-sqldisconnect-handle-5611-sanitize`.
Completed README VR/DQ/DV, fixture hashes, error-before-disconnect/cleanup
walkthrough and docs/22/docs/32 traceability retain the bounded scope.
Shipped as PR #7002 with all 11 required checks green, clean exact-head Codex
review and zero review conversations. Merged 2026-10-06 06:17:15 UTC as
667b6fd61936a65b5a5944aa048f92e6947bf0d1. The optional macOS installer job
passed after a same-head, job-only retry of GitHub artifact-upload ENOTFOUND;
no source change was needed. Other completed optional checks passed; Windows
MSVC remained in progress. Main is synchronized; the worktree/local/remote
branches are removed and both builds plus generated FXP are in recoverable
trash. Unrelated files/worktrees are preserved and the live channel was empty.
The encountered
disconnect-all/absent-handle/
type gap is #7001, filed against exact main without implementation admission.
The shared Numeric formatter gap #6997 remains unchanged. No owner input needed.

SET DATASESSION TO Numeric/exact-integer selector conversion is selected next
from exact main 667b6fd61936a65b5a5944aa048f92e6947bf0d1 on
`fix/set-datasession-numeric-5611`, in
`/home/rich/.codex/worktrees/set-datasession-numeric-5611/Project-Copperfin`.
Complete clean-room native source/output retains 61 commands/64 lines in
`tests/fixtures/vfp9-set-datasession-numeric-observation/`; two fresh final
runs match byte-for-byte. Numeric fractions truncate and both-sign low-32
aliases select live native sessions 1/2. Invalid selection raises 1540 while
preserving the active session. Other positive indices are derived checked
domain policy, not native existence evidence. The encountered existence/type
gap is #7003, filed against exact main without implementation admission.
140 direct calls and 108 fresh synthetic cases plus two resumable/four locale
cases cover bounds, state/settings retention, successful-event suppression,
catchability, mode reset and cleanup. Original dispatch fails exactly 251
selected assertions in both toolchains; helpers/controls/existing numeric
rows and three neighbors pass. Final results and README/matrix evidence are
summarized above. Builds are `/home/rich/temp/copperfin-set-datasession-5611-build`
and `/home/rich/temp/copperfin-set-datasession-5611-sanitize`; numeric tests ran
serially across build directories. The unchanged broad control-flow baseline
was interrupted after sustained execution and is not claimed passed; focused
resumable operands and relations/database/date-time neighbors are verified.
Shipped as PR #7005 at signed head d3a825a86 with all 11 required checks green,
clean exact-head review and no conversations. Merged 2026-10-06 07:16:08 UTC
as d5b1031df8d010c2534a8cece03a193de937d7d3. Completed optional checks passed;
Windows MSVC remained running. Main is synchronized, branches/worktree removed
and both builds/FXP are in recoverable trash. SET DECIMALS is selected next.

The Linux installed-GUI sub-slice remains explicitly deferred until the
managed Studio is shipped in the Linux package; source-tree Mono/Xvfb smoke is
not installed-product evidence. Re-enter that slice when packaging exposes the
managed Studio.

The owner-directed #6879 extended-table sequence remains retained and
uncancelled after the current owner-directed numeric-conversion work.

## Retained owner-directed workstream: extended tables and indexes

After the current numeric-conversion work, open owner-authored,
`agent-approved` design #6879 and its 23 direct sub-issues remain the next retained workstream.
Their live metadata and all owner-authored comments were revalidated on
2026-10-03. Work in this order:

1. #6880: probe installed VFP9 on unknown table-header version bytes and choose
   a distinct Copperfin marker that VFP rejects. Format work waits for this.
2. #6888: first ship the bounded #6877 exact-integer index-key fix, preserving
   adjacent keys above 2^53 in ascending and descending order.
3. Independent starting slices: #6893 `SET TEXTBEHAVIOR` (default `VFP9`) and
   #6895 `SET WARNINGS ON|OFF|ERROR` (name approved; default `ON`).
4. Bitwise group: #6872 shared conversion helper, then #6871, #6873, #6874,
   #6875, and #6876 (`BITAND64` family naming approved).
5. After #6880: #6881 format/registry; types #6882-#6885, #6896-#6902 and
   #6743; #6886 limits; #6887 native index; #6889 `INDEX ON`; #6890 DBC;
   #6891 import/export/COPY; #6892 documentation. #6902 is design-first.

Owner decisions for this workstream:

- VFP-compatible remains the default. Any non-VFP column automatically gives
  the table the distinct Copperfin header byte and a `SET WARNINGS`-controlled
  warning; mixed VFP/non-VFP columns are allowed.
- Copperfin-table limits are 1,024 columns, 65,500 fixed in-row bytes, 3,072
  index-key bytes, 32 columns per key, and 8,000 in-row text bytes. Widths are
  bytes. A declaration over its limit is an error and never auto-converts to
  memo storage. Memo/sidecar storage is an explicitly declared type. Odd widths
  for a two-byte-unit Unicode encoding follow `SET WARNINGS`: ON/OFF round down
  with shown/suppressed warning; ERROR raises a catchable error.
- `SET TEXTBEHAVIOR TO VFP9|COPPERFIN` governs every over-long text write:
  default VFP9 truncates (UTF-8 at a character boundary), COPPERFIN raises a
  catchable error naming the setting.
- Bitwise numeric conversion wraps to the low 32 bits only in VFP9 mode;
  default Copperfin mode rejects out-of-range values with error 11 (#6873).
- Copperfin tables retain fixed-width records. The native index is one
  combination/multi-tag file per table, similar to but intentionally distinct
  from CDX; Codex chooses and documents its layout and extension. Do not apply
  external-engine key limits.
- Nothing in the candidate type list is deferred. VFP9 claims require retained
  installed-VFP9 evidence. Use one slice per PR, fail-before/pass-after coverage
  in applicable modes, sanitizer evidence, docs/32 traceability, a valid dated
  changelog fragment, signed/DCO commits, green required checks and resolved
  review threads.

The unfinished #5611/#6776 numeric-conversion work below is active under the
latest owner-directed implementation-loop instruction. Continue bounded
slices until redirected or the checklist is complete, then return to #6879.

## Retained workstreams

Owner-directed workstream order before the #6879 assignment was:

1. **The numeric-conversion group.** Work authority is the open,
   owner-authored, `agent-approved` umbrella #5611; #6776 is the design
   reference and checklist (it was auto-closed when #6777 merged and has been
   reopened, see the traps below). Slices 1 and 2 are merged (#6777, #6778). The
   remaining work, by function, with the VFP9 behavior probed in
   `~/temp/vfp9-probes/numconv-6776/probe1.txt` (14 boundary values per
   expression; `probe1.out` is the UTF-16 original):
   - Completed bounded slices: arrays (#6030, PR #6865), `BITLSHIFT`/
     `BITRSHIFT` (#5765, PR #6867), `GOMONTH`/`EOMONTH` (#5608, PR #6869),
     `SPACE` (#6775), `ROUND` (PR #6878), and `CHR` (this continuation slice).
   - Completed most recently: `AT`/`STRTRAN` occurrence validation and
     `GETWORDNUM` boundary coverage (PR #6918).
   - Completed after those: `SUBSTR`/`SUBSTRC` huge-positive starts (PR #6919)
     and native/managed `DECLARE` narrowing (#6050, PR #6920).
   - Completed after those: `BINTOC`/`CTOBIN` (#5766, PR #6923).
   - Completed after those: array dimensions (#5594, PR #6926).
   - Completed after those: bounded `STR()` width/decimals validation (PR
     #6929), including the exact 237 width ceiling and explicit-mode negative
     low-32-bit quirk.
   - Completed after those: `DATE()` / `DATETIME()` numeric component
     validation (PR #6930).
   - Completed after those: `DOW()` optional first-day numeric validation (PR
     #6935).
   - Completed after those: `WEEK()` option order, bounds, setting fallback,
     and year-boundary compatibility (#6933, PR #6936).
   - Completed after those: `RGB()` component numeric validation (#6938, PR
     #6939).
   - Completed after those: Currency-first `MOD()` Numeric-divisor conversion
     (PR #6941).
   - Completed after those: `RAND()` seed numeric conversion (PR #6942).
   - Completed after those: `FILE()` / `DIRECTORY()` visibility-flag numeric
     conversion (PR #6947).
   - Completed after those: `FDATE()` optional type-flag numeric conversion
     (PR #6948).
   - Completed after those: `FOPEN()` optional numeric mode conversion
     (PR #6950).
   - Completed after those: `FCREATE()` optional numeric file-attribute
     conversion (PR #6952).
   - Completed after those: `FSEEK()` offset/origin numeric conversion
     (PR #6957).
   - Completed after those: shared file-handle numeric conversion in ten
     low-level callers (PR #6960).
   - Completed after those: `FCHSIZE()` size numeric conversion (PR #6962)
     and HEX extension numeric conversion (PR #6965).
   - Completed after those: primary SYS Numeric-selector conversion (PR #6967).
   - Completed after those: CPCURRENT Numeric/exact-integer selector
     conversion (PR #6969); query-domain/type parity #6968 remains separate.
   - Completed after those: shared RELATION/TARGET Numeric-index conversion
     (PR #6972); additive relation-order gap #6971 remains separate.
   - Completed after those: SET(TEXTMERGE) Numeric query-variant conversion
     (PR #6975); query-result gap #6973 and routing #6333 remain separate.
   - Completed after those: ALINES Numeric/exact-integer optional flag
     conversion (PR #6978).
   - Completed after those: ADIR Numeric/exact-integer optional display-flag
     conversion (PR #6980).
   - Completed after those: AFONT Numeric/exact-integer third-argument size
     conversion (PR #6983).
   - Completed after those: FIELD Numeric/exact-integer first-argument index
     conversion (PR #6989).
   - Completed after those: FSIZE Numeric/exact-integer first-argument
     admission (PR #6992).
   - Completed after those: SELECT() Numeric/exact-integer selector conversion
     (PR #6995).
   - Completed after those: SQLGETPROP Numeric/exact-integer first-argument
     handle conversion (PR #6998).
   - Completed after those: SQLSETPROP Numeric/exact-integer first-argument
     handle conversion (PR #7000).
   - Completed after those: SQLDISCONNECT Numeric/exact-integer first-argument
     handle conversion (PR #7002).
   - Completed after those: SET DATASESSION TO Numeric/exact-integer selector
     conversion (PR #7005); existence/type gap #7003 remains separate.
   - Completed after those: SET DECIMALS TO Numeric/exact-integer setting
     conversion (PR #7007); other-type admission #7006 remains separate.
   - Completed after those: SET FDOW TO Numeric/exact-integer setting
     conversion (PR #7009); other-type admission #7008 remains separate.
   - Completed after those: SET FWEEK TO Numeric/exact-integer setting
     conversion (PR #7011); other-type admission #7010 remains separate.
   - Completed after those: SET EPOCH query prerequisite (#7022/#7021) and
     Numeric/exact-integer conversion (PR #7023), then admitted #3698
     CENTURY/ROLLOVER (PR #7024). Both share parsing, independent display.
   - Completed after those: SQLCANCEL handle conversion (PR #7026); native
     connected-index and callback/type gaps #7025 remain separate.
   - Completed after those: SQLCOMMIT handle conversion (PR #7029); native
     connected-index and callback/type gaps #7028 remain separate.
   - Completed after those: SQLROLLBACK handle conversion (PR #7031); native
     connected-index and callback/type gaps #7030 remain separate.
   - Completed after those: SQLTABLES handle conversion (PR #7034); native
     connected-index and callback/type gaps #7032 remain separate.
   - Completed after those: SQLDATABASES handle conversion (PR #7035).
   - Completed after those: SQLPRIMARYKEYS handle conversion (PR #7037).
   - Completed after those: SQLFOREIGNKEYS handle conversion (PR #7039).
   - Completed after those: SQLCOLUMNS handle conversion (PR #7041); native
     callback/type/connected-index gap #7040 remains separate.
   - Completed after those: SQLROWCOUNT handle conversion (PR #7043);
     native absence is an intentional supported extension boundary.
   - Completed after those: SQLPREPARE first-argument Numeric/exact-integer
     conversion (PR #7046);
     native callback/type/connected-index gap #7045 remains separate.
   - Completed after those: SQLEXEC first-argument Numeric/exact-integer
     conversion (PR #7048); native callback/type/connected-index gap #7047
     remains separate.
   - Completed after those: CALLFN first-argument Numeric/exact-integer
     conversion (PR #7051); native absent/type gap #7050 remains separate.
   - Completed after those: ALEN optional dimension Numeric/exact-integer
     conversion (PR #7056); #6302 dimensionality/#7055 type gap remain separate.
   - Completed after those: TAG ordinal Numeric/exact-integer conversion
     (PR #7059); omitted routing #7057/type admission #7058 remain separate.
   - Completed after those: KEY ordinal Numeric/exact-integer conversion
     (PR #7062); omitted routing #7060/type admission #7061 remain separate.
   - Completed after those: DESCENDING ordinal Numeric/exact-integer conversion
     (PR #7065); type #7063/alias routing #7064 remain separate.
   - Completed after those: ISLEAPYEAR Numeric/exact-integer conversion
     (PR #7066); retained intentional extension with derived mathematical bounds.
   - Completed after those: JTOD/JTOT shared Julian-day conversion (PR #7068).
   - Completed after those: FV/PV excess-arity conversion boundary (PR #7069);
     #5878 core financial behavior remains unfinished.
   - Completed after those: BINDEVENT optional object-event flags (PR #7076);
     routine extension retained, type/Currency gap #7070 remains separate.
   - Completed after those: CURSORSETPROP BUFFERING second-argument numeric
     conversion (PR #7078); general buffering/type gap #7077 remain separate.
   - Completed after those: ERROR first-operand numeric conversion (PR #7080);
     native catalog/type recovery gap #7079 remains separate.
   - Completed after those: GO/GOTO explicit record-number conversion (PR#7082),
     SKIP count conversion (PR#7084), UNLOCK RECORD conversion (PR#7087)
     and SLEEP explicit-duration conversion (PR#7088). General record/type
     gap#7081, SKIP type#7083 and UNLOCK gaps#7085/#7086 remain separate.
   - Completed after those: DEFINE BAR Numeric-literal identifiers (PR#7089).
   - Completed after it: ON BAR ... ACTIVATE POPUP Numeric identifiers (PR#7090).
   - Completed after it: ON SELECTION BAR DO/static-action identifiers (PR#7091).
   - Completed after it: SET SKIP OF BAR identifiers (PR#7092).
   - Completed after it: SET MARK OF BAR identifiers (PR#7094).
   - Completed after it: MRKBAR Numeric/exact-integer identifiers (PR#7097).
   - Completed after it: SKPBAR Numeric/exact-integer identifiers (PR#7099).
   - Completed after it: PRMBAR Numeric/exact-integer identifiers (PR#7101).
   - Active: GETBAR second-argument Numeric/exact-integer positions.
     Other bar consumers#5868, popup order/count#6152 and expression admission
     #6225/#6227 remain unfinished.
   - Remaining after it: the other `llround(value_as_number(...))` sites in
     this and other modules (#5611 umbrella).
2. **Remaining cluster 15 allocation issues** (`docs/81` cluster 15): `FILETOSTR`
   #5740, `XMLTOCURSOR` #5767, `AGETFILEVERSION` #5686/#5759, project inventory
   #5703, PRG include depth #5728, runtime PRG load #5731, directory
   enumeration and array materialization #5741, `SPAWN` quota #5782, audit log
   appends #5787, `RESTORE FROM` #5790, list control #5764, import buffering
   #5642, CDX/DCX/MDX probing #5615. Follow the pattern in #6775: one shared
   validator or bound, error numbers from VFP9 evidence, a script-rows test.

### Rules decided by the owner (2026-10-02)

- Inconsistent or quirky VFP9 behavior is emulated only under a per-area switch.
  `SET COMPATIBLE` keeps its real FoxBASE+/dBASE meaning (#6223) and is not that
  switch. The first per-area switch is `SET NUMERICBEHAVIOR TO COPPERFIN|VFP9`
  (default `COPPERFIN`, reported by `SET('NUMERICBEHAVIOR')`; the spelling is
  provisional). Design and rationale: #6776.
- Default rule for every area: ask whether a *correct* legacy VFP program could
  plausibly depend on the behavior. If yes (documented contract, error numbers,
  limits such as the 16,777,184-byte Character string ceiling, `QUIT` with
  `NODEFAULT`), the default is VFP9 behavior. If only a buggy program could
  depend on it (for example `LEFT('abc', 1E20)` returning empty while
  `SUBSTR('abc', 1E20)` returns the last character), the default is Copperfin's
  own defined, consistent behavior and the VFP9 quirk sits behind the switch.
- A VFP9 expectation in an issue or an old test is not authoritative until it
  matches a fresh probe: three pinned tests were wrong this session
  (`stuff_zero_start`, `stuff_negative_start`, `SUBSTRC`/`STUFFC` zero and
  negative starts) and were corrected to the probed results.

### Probing VFP9

- Quick local probes: `~/bin/vfp9-probe /path/to/probe.prg` (Wine, about one
  second, no GUI). The PRG prints with `?`; do **not** end it with `QUIT`, the
  wrapper runs it inside its own harness. Use `--result <file>` for a PRG that
  writes its own file with `STRTOFILE(..., 0)`. Wine VFP9 was repaired on
  2026-10-02 (the missing piece was `VFP9ENU.DLL`).
- Cross-check on the Windows VM `copperfin-access365-win11`: automate
  `New-Object -ComObject VisualFoxPro.Application` from PowerShell and call
  `$v.Eval(...)`; examples in `~/temp/vfp9-probes/numconv-6776/probe*.ps1`.
  Launch long VM jobs with `Invoke-CimMethod Win32_Process Create` (a process
  started from an SSH session dies at logout); `C:\src\vm-job.ps1` builds named
  targets, checks free space and always cleans up. Keep the VM disk tidy: do not
  resize it unless a real space constraint appears.

### Merge and CI traps learned

- `main` requires 11 checks: DCO (1), the two Socket checks (2), the two
  executable-paths checks (2), and six lanes: the three generated-launcher
  checks (Windows, Ubuntu, macOS), the two DECLARE checks (Win32 and x64), and
  `windows-environment-paths`. `macOS Clang`, `Windows
  MSVC` and `windows-installer` are not required. After the fixes above,
  `macOS Clang` and `windows-installer` were green on #6778, and the full
  `Windows MSVC` native validation passed on #6774's own run (#6767); on #6778
  it was still running when that PR merged, so check the first `main` run.
- Count the required checks that have reported. "Nothing pending" can mean the
  jobs are not scheduled yet. `BLOCKED` with every required check green means an
  unresolved review thread (bot threads included). `mergeStateStatus` becoming
  `UNSTABLE` or `CLEAN` with zero unresolved threads is the reliable signal.
- Two jobs can share a name (`Windows MSVC`); `gh pr edit` is broken by the
  Projects-classic deprecation, so patch PR bodies through the REST API; rebase,
  do not merge, to catch a branch up; every commit needs a `Signed-off-by` for
  each `Co-Authored-By` identity; only the first issue in a comma-separated
  `Fixes` list auto-closes, so close the rest by hand; and GitHub's closing
  keywords ignore negation, so a PR note saying "does not close #N" still
  closes #N (this closed #6776 early): write "relates to #N" instead.

## Workspace preservation

Preserve unrelated untracked user files in the main checkout:

- `AGENTS.md`
- `Z:\\home\\rich\\temp\\vfp9-probes\\empty-object-205\\vfp.out`

The defect-fix takeover ended at 17:00 America/Detroit on 2026-09-22. The
first focused cycle of the owner's second-pass discovery prompt found and
reproduced issue #6492: `PREVIEW` inside a quoted REPORT/LABEL `TO FILE` output
pathname incorrectly enters preview mode and creates no output. A temporary
Linux regression failed for both commands; the test edit was removed, and no
production fix was committed. Per the owner's subsequent direction, the
discovery heartbeat now runs hourly; the older
`continue-copperfin-issue-loop` heartbeat is paused. Next discovery cycle:
inspect fixes since this pass or shift to an independent invariant if newly
filed issues are being resolved.

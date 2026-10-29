// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: frozen-source assertions only.
'use strict';
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const root=__dirname,h=fs.readFileSync(path.join(root,'sprintf-boundary.h'),'utf8'),t=fs.readFileSync(path.join(root,'sprintf-boundary-check.c'),'utf8'),p=fs.readFileSync(path.join(root,'gcvt-probe.c'),'utf8');
function occurrences(source,needle){return source.split(needle).length-1;}
a.equal(occurrences(h,'snprintf_function(snprintf_buffer->value,SPRINTF_CAPACITY,"%.*f",count,input)'),1);
a.equal(occurrences(h,'sprintf_function(sprintf_buffer->value,"%.*f",count,input)'),1);
a.match(h,/SPRINTF_CAPACITY=2048, SPRINTF_TEXT_CAPACITY=384, SPRINTF_GUARD=16/);
a.match(h,/count_value==15 \|\| count_value==16 \|\| count_value==17 \|\| count_value==18/);
a(h.indexOf('snprintf_function(')<h.indexOf('sprintf_function('));
a(h.indexOf('sprintf_guards(snprintf_buffer')<h.indexOf('sprintf_function('));
a(h.indexOf('result->equal=result->sprintf_used==result->snprintf_used')>h.indexOf('result->sprintf_text'));
for(const forbidden of ['LoadLibrary','FreeLibrary','malloc(','calloc(','realloc(','free(','fesetround','_controlfp','fldcw'])a.equal(h.includes(forbidden),false,forbidden);
a.match(t,/result\.equal \|\| strcmp\(result\.sprintf_text,stext\) \|\| strcmp\(result\.snprintf_text,ntext\)/);
a.match(h,/result->snprintf_grammar=sprintf_fixed_grammar/);
a.match(h,/result->sprintf_grammar=sprintf_fixed_grammar/);
a.match(t,/sprintf-boundary checks=%u PASS \(stub only\)/);
a.match(p,/#include "sprintf-boundary\.h"/);
a.equal(occurrences(p,'GetProcAddress(module,"sprintf")'),1);
a.equal(occurrences(p,'GetProcAddress(module,"_snprintf")'),1);
a.match(p,/sprintf_call\(sprintf_function,snprintf_function,input\.value,count_value,/);
a(p.indexOf(';sprintfGrammar=')<p.indexOf(';sprintf='));
a(p.indexOf(';snprintfGrammar=')<p.indexOf(';snprintf='));
a(p.indexOf(';equal=')<p.indexOf(';sprintf='));
a.match(p,/\{"CFNSPRTF",sprintf_conversion,2,"NN"\}/);
a.match(p,/PublicTable _FoxTable=\{0,5,functions\}/);
a.equal(p.includes('LoadLibrary'),false);
process.stdout.write('sprintf-source exact assertions=27 PASS (source/stubs only)\n');

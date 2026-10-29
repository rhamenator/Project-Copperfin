/* Copyright © 2026 Richard M. Hamilton.
 * SPDX-License-Identifier: GPL-3.0-only
 * Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
 * RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: guarded public sprintf/_snprintf
 * comparison boundary, NOT a VFP-internal attribution or formatter model.
 */
#ifndef COPPERFIN_SPRINTF_BOUNDARY_H
#define COPPERFIN_SPRINTF_BOUNDARY_H
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
_Static_assert(sizeof(double)==8 && FLT_RADIX==2 && DBL_MANT_DIG==53 && DBL_MAX_EXP==1024 && DBL_MIN_EXP==-1021,
               "only the binary64 experiment");
#if defined(_WIN32)
#define SPRINTF_CDECL __cdecl
#else
#define SPRINTF_CDECL
#endif
typedef int (SPRINTF_CDECL *SprintfFunction)(char *,const char *,...);
typedef int (SPRINTF_CDECL *SnprintfFunction)(char *,size_t,const char *,...);
enum { SPRINTF_CAPACITY=2048, SPRINTF_TEXT_CAPACITY=384, SPRINTF_GUARD=16 };
/* Fixed binary64 text needs at most sign + 309 integer digits + dot + the
 * admitted 18 fractional digits + NUL. The larger owned buffer is deliberate. */
_Static_assert(SPRINTF_TEXT_CAPACITY>1+309+1+18+1 && SPRINTF_CAPACITY>SPRINTF_TEXT_CAPACITY,
               "fixed binary64 output bounds");
typedef struct { unsigned char before[SPRINTF_GUARD]; char value[SPRINTF_CAPACITY]; unsigned char after[SPRINTF_GUARD]; } SprintfBuffer;
_Static_assert(offsetof(SprintfBuffer,value)==SPRINTF_GUARD &&
               offsetof(SprintfBuffer,after)==SPRINTF_GUARD+SPRINTF_CAPACITY,
               "adjacent owned guard bytes");
typedef struct {
    unsigned sprintf_used,snprintf_used,sprintf_grammar,snprintf_grammar,equal;
    char sprintf_text[SPRINTF_TEXT_CAPACITY],snprintf_text[SPRINTF_TEXT_CAPACITY];
} SprintfResult;
enum SprintfStatus { SPRINTF_OK=0,SPRINTF_PARAMETER,SPRINTF_NONFINITE,SPRINTF_COUNT,
                     SPRINTF_CANARY,SPRINTF_RETURN,SPRINTF_UNTERMINATED,
                     SPRINTF_LENGTH,SPRINTF_OUTPUT_LIMIT };
static int sprintf_fixed_grammar(const char *s,unsigned n,unsigned count) {
    unsigned i=0,integer=0,fraction=0;
    if(i<n && (s[i]=='+' || s[i]=='-')) ++i;
    while(i<n && s[i]>='0' && s[i]<='9') { ++i;++integer; }
    if(!integer || i==n || s[i++]!='.') return 0;
    while(i<n && s[i]>='0' && s[i]<='9') { ++i;++fraction; }
    return i==n && fraction==count;
}
static void sprintf_prepare(SprintfBuffer *buffer,unsigned char left,unsigned char right) {
    for(unsigned i=0;i<SPRINTF_GUARD;++i) { buffer->before[i]=left;buffer->after[i]=right; }
    for(unsigned i=0;i<SPRINTF_CAPACITY;++i) buffer->value[i]=(char)0xcc;
}
static int sprintf_guards(const SprintfBuffer *buffer,unsigned char left,unsigned char right) {
    for(unsigned i=0;i<SPRINTF_GUARD;++i)
        if(buffer->before[i]!=left || buffer->after[i]!=right) return 0;
    return 1;
}
static int sprintf_measure(const SprintfBuffer *buffer,int returned,unsigned *used) {
    if(returned<0 || returned>=SPRINTF_CAPACITY) return SPRINTF_RETURN;
    unsigned n=0;while(n<SPRINTF_CAPACITY && buffer->value[n]) ++n;
    if(n==SPRINTF_CAPACITY) return SPRINTF_UNTERMINATED;
    if(n!=(unsigned)returned) return SPRINTF_LENGTH;
    if(n>=SPRINTF_TEXT_CAPACITY) return SPRINTF_OUTPUT_LIMIT;
    *used=n;return SPRINTF_OK;
}
/* Keep both public results even when they differ: equality is an observation,
 * never a gate that discards a mismatch. The literal format and precision are
 * source-frozen; no locale/control-word mutation or allocation occurs here. */
static int sprintf_call(SprintfFunction sprintf_function,SnprintfFunction snprintf_function,
                        double input,double count_value,SprintfBuffer *sprintf_buffer,
                        SprintfBuffer *snprintf_buffer,SprintfResult *result) {
    if(!sprintf_function || !snprintf_function || !sprintf_buffer || !snprintf_buffer || !result)
        return SPRINTF_PARAMETER;
    union { double value; unsigned char bytes[8]; } bits;bits.value=input;
#if !defined(_WIN32)
    _Static_assert(__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__,"little-endian test host");
#endif
    if((bits.bytes[7]&0x7f)==0x7f && (bits.bytes[6]&0xf0)==0xf0) return SPRINTF_NONFINITE;
    if(!(count_value==15 || count_value==16 || count_value==17 || count_value==18)) return SPRINTF_COUNT;
    int count=(int)count_value;
    sprintf_prepare(snprintf_buffer,0xa5,0x5a);
    int snprintf_return=snprintf_function(snprintf_buffer->value,SPRINTF_CAPACITY,"%.*f",count,input);
    if(!sprintf_guards(snprintf_buffer,0xa5,0x5a)) return SPRINTF_CANARY;
    int status=sprintf_measure(snprintf_buffer,snprintf_return,&result->snprintf_used);
    if(status!=SPRINTF_OK) return status;
    sprintf_prepare(sprintf_buffer,0x3c,0xc3);
    int sprintf_return=sprintf_function(sprintf_buffer->value,"%.*f",count,input);
    if(!sprintf_guards(sprintf_buffer,0x3c,0xc3)) return SPRINTF_CANARY;
    status=sprintf_measure(sprintf_buffer,sprintf_return,&result->sprintf_used);
    if(status!=SPRINTF_OK) return status;
    for(unsigned i=0;i<=result->snprintf_used;++i) result->snprintf_text[i]=snprintf_buffer->value[i];
    for(unsigned i=0;i<=result->sprintf_used;++i) result->sprintf_text[i]=sprintf_buffer->value[i];
    result->snprintf_grammar=sprintf_fixed_grammar(result->snprintf_text,result->snprintf_used,(unsigned)count);
    result->sprintf_grammar=sprintf_fixed_grammar(result->sprintf_text,result->sprintf_used,(unsigned)count);
    result->equal=result->sprintf_used==result->snprintf_used;
    if(result->equal) for(unsigned i=0;i<result->sprintf_used;++i)
        if(result->sprintf_text[i]!=result->snprintf_text[i]) { result->equal=0;break; }
    return SPRINTF_OK;
}
#endif

/* Copyright © 2026 Richard M. Hamilton.
 * SPDX-License-Identifier: GPL-3.0-only
 * Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
 * RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: first-party _gcvt call boundary,
 * NOT a conversion/rounding implementation.
 * Public ABI: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/gcvt
 */
#ifndef COPPERFIN_GCVT_BOUNDARY_H
#define COPPERFIN_GCVT_BOUNDARY_H
#include <float.h>
_Static_assert(sizeof(double)==8 && FLT_RADIX==2 && DBL_MANT_DIG==53 && DBL_MAX_EXP==1024 && DBL_MIN_EXP==-1021,
               "only the binary64 experiment");
#if defined(_WIN32)
#define GCVT_CDECL __cdecl
#else
#define GCVT_CDECL
#endif
typedef char * (GCVT_CDECL *GcvtFunction)(double,int,char *);
/* Not a guessed _CVTBUFSIZE value. Binary64 exact fixed decimal needs at most
 * 310 rounded integer digits + dot + 1074 fraction digits + sign + NUL=1387.
 * Significant counts15..17 scientific output is shorter. The documented
 * interface adds no padding. 2048 exceeds this conservative sum. Canaries
 * detect boundary damage, not arbitrary memory corruption by a broken CRT. */
enum { GCVT_CAPACITY=2048, GCVT_TEXT_CAPACITY=128, GCVT_GUARD=16 };
_Static_assert(GCVT_CAPACITY>310+1+1074+1+1,"finite binary64 decimal bound");
typedef struct { unsigned char before[GCVT_GUARD]; char value[GCVT_CAPACITY]; unsigned char after[GCVT_GUARD]; } GcvtBuffer;
_Static_assert(__builtin_offsetof(GcvtBuffer,value)==GCVT_GUARD &&
               __builtin_offsetof(GcvtBuffer,after)==GCVT_GUARD+GCVT_CAPACITY,
               "adjacent own guard bytes");
typedef struct { unsigned used; char text[GCVT_TEXT_CAPACITY]; } GcvtResult;
enum GcvtStatus { GCVT_OK=0,GCVT_PARAMETER,GCVT_NONFINITE,GCVT_COUNT,GCVT_NULL,
                  GCVT_POINTER,GCVT_CANARY,GCVT_UNTERMINATED,GCVT_OUTPUT_LIMIT,GCVT_GRAMMAR };
static int gcvt_grammar(const char *s,unsigned n) {
    unsigned i=0,digits=0;
    if(i<n && (s[i]=='+' || s[i]=='-')) ++i;
    while(i<n && s[i]>='0' && s[i]<='9') { ++i;++digits; }
    if(i<n && s[i]=='.') { ++i;while(i<n && s[i]>='0' && s[i]<='9') { ++i;++digits; } }
    if(!digits) return 0;
    if(i<n && (s[i]=='e' || s[i]=='E')) {
        ++i;if(i<n && (s[i]=='+' || s[i]=='-')) ++i;
        unsigned exponent_digits=0;
        while(i<n && s[i]>='0' && s[i]<='9') { ++i;++exponent_digits; }
        if(!exponent_digits) return 0;
    }
    return i==n;
}
static int gcvt_call(GcvtFunction function,double input,double count_value,GcvtBuffer *buffer,GcvtResult *result) {
    if(!function || !buffer || !result) return GCVT_PARAMETER;
    union { double value; unsigned char bytes[8]; } bits;bits.value=input;
    /* This experiment is little-endian on both native32 and host test targets. */
#if !defined(_WIN32)
    _Static_assert(__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__,"little-endian test host");
#endif
    if((bits.bytes[7]&0x7f)==0x7f && (bits.bytes[6]&0xf0)==0xf0) return GCVT_NONFINITE;
    if(!(count_value==15 || count_value==16 || count_value==17)) return GCVT_COUNT;
    int count=(int)count_value; /* bounded exact integers before conversion */
    for(unsigned i=0;i<GCVT_GUARD;++i) { buffer->before[i]=0xa5;buffer->after[i]=0x5a; }
    for(unsigned i=0;i<GCVT_CAPACITY;++i) buffer->value[i]=(char)0xcc;
    char *returned=function(input,count,buffer->value);
    for(unsigned i=0;i<GCVT_GUARD;++i) if(buffer->before[i]!=0xa5 || buffer->after[i]!=0x5a) return GCVT_CANARY;
    if(!returned) return GCVT_NULL;
    /* Never dereference a returned foreign/interior pointer. */
    if(returned!=buffer->value) return GCVT_POINTER;
    unsigned used=0;while(used<GCVT_CAPACITY && buffer->value[used]) ++used;
    if(used==GCVT_CAPACITY) return GCVT_UNTERMINATED;
    if(used>=GCVT_TEXT_CAPACITY) return GCVT_OUTPUT_LIMIT;
    if(!gcvt_grammar(buffer->value,used)) return GCVT_GRAMMAR;
    for(unsigned i=0;i<=used;++i) result->text[i]=buffer->value[i];
    result->used=used;return GCVT_OK;
}
#endif

/* Copyright © 2026 Richard M. Hamilton.
 * SPDX-License-Identifier: GPL-3.0-only
 * Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
 * RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: direct tests of the EXACT shared
 * call boundary with stubs, NOT native CRT/VFP.
 */
#include <stdio.h>
#include <string.h>
#include "gcvt-boundary.h"
static GcvtBuffer buffer;
static GcvtResult result;
static unsigned mode,index_guard,calls,checks;
static char sample[GCVT_CAPACITY];
static unsigned sample_length;
static double seen_input;
static int seen_count;
static char *stub(double input,int count,char *out) {
    ++calls;seen_input=input;seen_count=count;
    if(mode==1) return 0;
    if(mode==2) return sample;
    if(mode==3) return out+1;
    if(mode==4) { buffer.before[index_guard]^=1;return out; }
    if(mode==5) { buffer.after[index_guard]^=1;return out; }
    if(mode==6) return out; /* preparation filled every byte nonzero */
    for(unsigned i=0;i<sample_length;++i) out[i]=sample[i];
    out[sample_length]=0;return out;
}
static int check(double input,double count,int expected,unsigned invoked) {
    calls=0;int status=gcvt_call(stub,input,count,&buffer,&result);++checks;
    if(status!=expected || calls!=invoked) return 0;
    if(status==GCVT_OK && (result.used!=sample_length || memcmp(result.text,sample,sample_length+1))) return 0;
    if(invoked && (memcmp(&input,&seen_input,sizeof(input)) || seen_count!=(int)count)) return 0;
    return 1;
}
static void text(const char *s) { sample_length=(unsigned)strlen(s);memcpy(sample,s,sample_length+1); }
int main(void) {
    const char *valid[]={"0","-0","+1","1.25","1.",".5","-.5","0.00000000000000001","-1.2e+017","1E-324"};
    for(unsigned i=0;i<sizeof(valid)/sizeof(valid[0]);++i) { text(valid[i]);for(int count=15;count<=17;++count) if(!check(-0.0,count,GCVT_OK,1)) return 1; }
    const char *invalid[]={"",".","+","-","+.","e10","1e","1e+","1e--2","1.2.3","1x","1 2","1\n","NaN","inf","1,2","\x80"};
    for(unsigned i=0;i<sizeof(invalid)/sizeof(invalid[0]);++i) { text(invalid[i]);if(!check(0.5,17,GCVT_GRAMMAR,1)) return 2; }
    for(unsigned n=127;n<=128;++n) { for(unsigned i=0;i<n;++i)sample[i]='1';sample[n]=0;sample_length=n;if(!check(0.5,17,n==127?GCVT_OK:GCVT_OUTPUT_LIMIT,1))return 3; }
    sample_length=GCVT_CAPACITY-1;for(unsigned i=0;i<sample_length;++i)sample[i]='1';sample[sample_length]=0;
    if(!check(0.5,17,GCVT_OUTPUT_LIMIT,1))return 3;
    for(mode=1;mode<=3;++mode)if(!check(0.5,16,mode==1?GCVT_NULL:GCVT_POINTER,1))return 4;
    for(mode=4;mode<=5;++mode)for(index_guard=0;index_guard<GCVT_GUARD;++index_guard)if(!check(0.5,15,GCVT_CANARY,1))return 5;
    mode=6;if(!check(0.5,17,GCVT_UNTERMINATED,1))return 6;
    mode=0;text("0");
    const double invalid_count[]={-1,0,1,14,14.5,15.1,16.5,17.1,18,21};
    for(unsigned i=0;i<sizeof(invalid_count)/sizeof(invalid_count[0]);++i)if(!check(0.5,invalid_count[i],GCVT_COUNT,0))return 7;
    union { unsigned long long bits;double value; } v;
    const unsigned long long invalid_bits[]={0x7ff0000000000000ULL,0xfff0000000000000ULL,0x7ff8000000000001ULL,0xfff8000000000001ULL,0x7ff0000000000001ULL};
    for(unsigned i=0;i<sizeof(invalid_bits)/sizeof(invalid_bits[0]);++i) { v.bits=invalid_bits[i];if(!check(v.value,17,GCVT_NONFINITE,0) || !check(0.5,v.value,GCVT_COUNT,0))return 8; }
    const unsigned long long valid_bits[]={0,0x8000000000000000ULL,1,0x0010000000000000ULL,0x7fefffffffffffffULL,0xffefffffffffffffULL};
    for(unsigned i=0;i<sizeof(valid_bits)/sizeof(valid_bits[0]);++i) { v.bits=valid_bits[i];if(!check(v.value,17,GCVT_OK,1))return 9; }
    calls=0;
    if(gcvt_call(0,0.5,17,&buffer,&result)!=GCVT_PARAMETER || gcvt_call(stub,0.5,17,0,&result)!=GCVT_PARAMETER || gcvt_call(stub,0.5,17,&buffer,0)!=GCVT_PARAMETER || calls)return 10;
    checks+=3;
    printf("gcvt-boundary checks=%u PASS (stub only)\n",checks);return 0;
}

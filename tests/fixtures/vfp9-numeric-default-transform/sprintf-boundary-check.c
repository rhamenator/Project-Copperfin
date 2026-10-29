/* Copyright © 2026 Richard M. Hamilton.
 * SPDX-License-Identifier: GPL-3.0-only
 * RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: exact boundary tests with stubs;
 * these are NOT native CRT observations or compatibility expected strings.
 */
#include <stdio.h>
#include <string.h>
#include "sprintf-boundary.h"
static SprintfBuffer sb,nb;
static SprintfResult result;
static unsigned smode,nmode,calls,checks;
static const char *stext="1.000000000000000";
static const char *ntext="1.000000000000000";
static int emit(char *out,const char *value,int mode) {
    ++calls;unsigned n=(unsigned)strlen(value);memcpy(out,value,n+1);
    if(mode==1) return -1;
    if(mode==2) return (int)n+1;
    if(mode==3) { for(unsigned i=0;i<SPRINTF_CAPACITY;++i)out[i]='1';return (int)n; }
    if(mode==4) return SPRINTF_CAPACITY;
    if(mode==5) { out[n]='x';out[n+1]=0;return (int)n+1; }
    if(mode==6) { sb.before[0]^=1;return (int)n; }
    if(mode==7) { nb.after[0]^=1;return (int)n; }
    return (int)n;
}
static int SPRINTF_CDECL sstub(char *out,const char *format,...) {
    if(strcmp(format,"%.*f")) return -99;
    va_list ap;va_start(ap,format);int count=va_arg(ap,int);(void)va_arg(ap,double);va_end(ap);
    if(count<15 || count>18) return -98;
    return emit(out,stext,smode);
}
static int SPRINTF_CDECL nstub(char *out,size_t capacity,const char *format,...) {
    if(capacity!=SPRINTF_CAPACITY || strcmp(format,"%.*f")) return -99;
    va_list ap;va_start(ap,format);int count=va_arg(ap,int);(void)va_arg(ap,double);va_end(ap);
    if(count<15 || count>18) return -98;
    return emit(out,ntext,nmode);
}
static int check(double input,double count,int expected,unsigned invoked) {
    calls=0;int status=sprintf_call(sstub,nstub,input,count,&sb,&nb,&result);++checks;
    return status==expected && calls==invoked;
}
int main(void) {
    for(int count=15;count<=18;++count) {
        char valid[4][24];const char *prefix[]={"0.","-0.","1.","+1."};
        for(unsigned i=0;i<4;++i) {
            unsigned n=(unsigned)strlen(prefix[i]);memcpy(valid[i],prefix[i],n);
            for(int j=0;j<count;++j) valid[i][n+(unsigned)j]='0';
            valid[i][n+(unsigned)count]=0;
            stext=ntext=valid[i];if(!check(-0.0,count,SPRINTF_OK,2) || !result.equal) return 1;
        }
    }
    stext="1.000000000000000";ntext="2.000000000000000";
    if(!check(1.0,15,SPRINTF_OK,2) || result.equal || strcmp(result.sprintf_text,stext) || strcmp(result.snprintf_text,ntext)) return 2;
    ntext="1.000000000000000";
    const double bad_counts[]={-1,0,14,14.5,15.5,18.5,19};
    for(unsigned i=0;i<sizeof(bad_counts)/sizeof(bad_counts[0]);++i)if(!check(1,bad_counts[i],SPRINTF_COUNT,0))return 3;
    union { unsigned long long bits;double value; } v;
    const unsigned long long bad_bits[]={0x7ff0000000000000ULL,0xfff0000000000000ULL,0x7ff8000000000001ULL};
    for(unsigned i=0;i<sizeof(bad_bits)/sizeof(bad_bits[0]);++i){v.bits=bad_bits[i];if(!check(v.value,15,SPRINTF_NONFINITE,0))return 4;}
    nmode=1;if(!check(1,15,SPRINTF_RETURN,1))return 5;nmode=2;if(!check(1,15,SPRINTF_LENGTH,1))return 6;
    nmode=3;if(!check(1,15,SPRINTF_UNTERMINATED,1))return 7;nmode=4;if(!check(1,15,SPRINTF_RETURN,1))return 8;
    nmode=5;if(!check(1,15,SPRINTF_OK,2) || result.snprintf_grammar || !result.sprintf_grammar)return 9;nmode=7;if(!check(1,15,SPRINTF_CANARY,1))return 10;
    nmode=0;smode=1;if(!check(1,15,SPRINTF_RETURN,2))return 11;smode=2;if(!check(1,15,SPRINTF_LENGTH,2))return 12;
    smode=3;if(!check(1,15,SPRINTF_UNTERMINATED,2))return 13;smode=4;if(!check(1,15,SPRINTF_RETURN,2))return 14;
    smode=5;if(!check(1,15,SPRINTF_OK,2) || result.sprintf_grammar || !result.snprintf_grammar)return 15;smode=6;if(!check(1,15,SPRINTF_CANARY,2))return 16;
    smode=0;stext=ntext="1e+00";if(!check(1,15,SPRINTF_OK,2) || result.sprintf_grammar || result.snprintf_grammar || !result.equal)return 17;
    calls=0;if(sprintf_call(0,nstub,1,15,&sb,&nb,&result)!=SPRINTF_PARAMETER ||
       sprintf_call(sstub,0,1,15,&sb,&nb,&result)!=SPRINTF_PARAMETER ||
       sprintf_call(sstub,nstub,1,15,0,&nb,&result)!=SPRINTF_PARAMETER ||
       sprintf_call(sstub,nstub,1,15,&sb,0,&result)!=SPRINTF_PARAMETER ||
       sprintf_call(sstub,nstub,1,15,&sb,&nb,0)!=SPRINTF_PARAMETER || calls)return 18;
    checks+=5;printf("sprintf-boundary checks=%u PASS (stub only)\n",checks);return 0;
}

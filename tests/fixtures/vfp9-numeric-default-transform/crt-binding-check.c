/* Copyright © 2026 Richard M. Hamilton.
 * SPDX-License-Identifier: GPL-3.0-only
 * Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
 * Direct host verification of the exact first-party observer header, not VFP.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crt-binding.h"
int main(int argc,char **argv) {
    if(!crt_sha_selftest()) return 1;
    if(argc==2 && !strcmp(argv[1],"--pe")) {
        unsigned char bytes[4096]={0};CrtPe p,q;
        bytes[0]='M';bytes[1]='Z';bytes[60]=64;bytes[64]='P';bytes[65]='E';
        bytes[68]=0x4c;bytes[69]=1;bytes[84]=68;bytes[88]=0x0b;bytes[89]=1;
        bytes[145]=0x10; /* SizeOfImage=4096 at offset64+80. */
        if(!crt_pe(bytes,sizeof(bytes),&p) || !crt_pe(bytes,sizeof(bytes),&q) || !crt_pe_equal(&p,&q)) return 2;
        q.stamp=1;if(crt_pe_equal(&p,&q)) return 3;
        for(unsigned n=0;n<156;++n) if(crt_pe(bytes,n,&q)) return 4;
        bytes[69]=0x86;if(crt_pe(bytes,sizeof(bytes),&q)) return 5;
        bytes[69]=1;bytes[60]=0xff;bytes[61]=0xff;if(crt_pe(bytes,sizeof(bytes),&q)) return 6;
        puts("sha-known-answers/pe-bounds PASS");return 0;
    }
    if(argc!=3) return 7;
    char *end=0;unsigned long chunk=strtoul(argv[2],&end,10);
    if(!*argv[2] || *end || chunk<1 || chunk>4096) return 8;
    FILE *file=fopen(argv[1],"rb");if(!file) return 9;
    CrtSha s;unsigned char block[4096],digest[32];unsigned total=0;size_t got;crt_sha_init(&s);
    while((got=fread(block,1,chunk,file))!=0) {
        if(got>8388608u-total) { fclose(file);return 10; }
        total+=(unsigned)got;crt_sha_update(&s,block,(unsigned)got);
    }
    int error=ferror(file);if(fclose(file) || error) return 11;
    crt_sha_final(&s,digest);for(unsigned i=0;i<32;++i) printf("%02x",digest[i]);puts("");
    return 0;
}

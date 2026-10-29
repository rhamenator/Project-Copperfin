/* Copyright © 2026 Richard M. Hamilton.
 * SPDX-License-Identifier: GPL-3.0-only
 * Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
 * OWN native-observer support, NOT production or formatter code.
 * SHA256: FIPS180-4 sections4.1.2/4.2.2/5/6.2; PE: public Microsoft PE format.
 * No cryptographic certification or atomic image/file binding is claimed.
 */
#ifndef COPPERFIN_CRT_BINDING_H
#define COPPERFIN_CRT_BINDING_H
_Static_assert(sizeof(unsigned)==4 && sizeof(unsigned long long)==8,"SHA word sizes");
typedef struct { unsigned h[8],used; unsigned long long bytes; unsigned char block[64]; } CrtSha;
static unsigned crt_rotr(unsigned x,unsigned n) { return (x>>n)|(x<<(32-n)); }
static void crt_sha_block(CrtSha *s) {
    static const unsigned k[64]={
        0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
        0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
        0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
        0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
        0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
        0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
        0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
        0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};
    unsigned w[64];
    for(unsigned i=0;i<16;++i) {
        const unsigned char *p=s->block+4*i;
        w[i]=((unsigned)p[0]<<24)|((unsigned)p[1]<<16)|((unsigned)p[2]<<8)|p[3];
    }
    for(unsigned i=16;i<64;++i) {
        unsigned x=w[i-15],y=w[i-2];
        w[i]=(crt_rotr(x,7)^crt_rotr(x,18)^(x>>3))+w[i-16]+w[i-7]+
             (crt_rotr(y,17)^crt_rotr(y,19)^(y>>10));
    }
    unsigned a=s->h[0],b=s->h[1],c=s->h[2],d=s->h[3],e=s->h[4],f=s->h[5],g=s->h[6],h=s->h[7];
    for(unsigned i=0;i<64;++i) {
        unsigned t1=h+(crt_rotr(e,6)^crt_rotr(e,11)^crt_rotr(e,25))+((e&f)^(~e&g))+k[i]+w[i];
        unsigned t2=(crt_rotr(a,2)^crt_rotr(a,13)^crt_rotr(a,22))+((a&b)^(a&c)^(b&c));
        h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
    }
    s->h[0]+=a;s->h[1]+=b;s->h[2]+=c;s->h[3]+=d;s->h[4]+=e;s->h[5]+=f;s->h[6]+=g;s->h[7]+=h;
}
static void crt_sha_init(CrtSha *s) {
    static const unsigned initial[8]={0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};
    for(unsigned i=0;i<8;++i) s->h[i]=initial[i];
    s->used=0;s->bytes=0;
}
/* Callers bound message size to8MiB; byte/bit length cannot overflow. */
static void crt_sha_update(CrtSha *s,const unsigned char *p,unsigned n) {
    s->bytes+=n;
    for(unsigned i=0;i<n;++i) {
        s->block[s->used++]=p[i];
        if(s->used==64) { crt_sha_block(s);s->used=0; }
    }
}
static void crt_sha_final(CrtSha *s,unsigned char digest[32]) {
    unsigned long long bits=s->bytes*8;
    unsigned char one=0x80,zero=0;
    crt_sha_update(s,&one,1);
    while(s->used!=56) crt_sha_update(s,&zero,1);
    for(int i=7;i>=0;--i) { unsigned char b=(unsigned char)(bits>>(8*i));crt_sha_update(s,&b,1); }
    for(unsigned i=0;i<32;++i) digest[i]=(unsigned char)(s->h[i/4]>>(24-8*(i%4)));
}
static int crt_sha_selftest(void) {
    static const char *messages[]={"","abc","abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
    static const char *answers[]={
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
        "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"};
    static const char hexes[]="0123456789abcdef";
    for(unsigned test=0;test<3;++test) {
        CrtSha s;unsigned char digest[32];crt_sha_init(&s);
        for(const char *p=messages[test];*p;++p) crt_sha_update(&s,(const unsigned char *)p,1);
        crt_sha_final(&s,digest);
        for(unsigned i=0;i<32;++i) if(hexes[digest[i]>>4]!=answers[test][2*i] || hexes[digest[i]&15]!=answers[test][2*i+1]) return 0;
    }
    return 1;
}
typedef struct { unsigned machine,magic,stamp,image_size,checksum; } CrtPe;
static unsigned crt_le16(const unsigned char *p) { return p[0]|((unsigned)p[1]<<8); }
static unsigned crt_le32(const unsigned char *p) { return crt_le16(p)|(crt_le16(p+2)<<16); }
/* Read only bounded public headers, never code/disassembly. */
static int crt_pe(const unsigned char *p,unsigned n,CrtPe *out) {
    if(n<64 || p[0]!='M' || p[1]!='Z') return 0;
    unsigned offset=crt_le32(p+60);
    if(offset<64 || offset>4096-92 || offset>n || n-offset<92) return 0;
    const unsigned char *q=p+offset;
    if(q[0]!='P' || q[1]!='E' || q[2] || q[3] || crt_le16(q+20)<68) return 0;
    if(crt_le16(q+4)!=0x14c || crt_le16(q+24)!=0x10b) return 0;
    out->machine=crt_le16(q+4);out->magic=crt_le16(q+24);out->stamp=crt_le32(q+8);
    out->image_size=crt_le32(q+80);out->checksum=crt_le32(q+88);
    return out->image_size>=4096;
}
static int crt_pe_equal(const CrtPe *a,const CrtPe *b) {
    return a->machine==b->machine && a->magic==b->magic && a->stamp==b->stamp &&
           a->image_size==b->image_size && a->checksum==b->checksum;
}
#endif

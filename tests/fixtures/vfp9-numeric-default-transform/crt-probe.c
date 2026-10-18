/* Copyright © 2026 Richard M. Hamilton.
 * SPDX-License-Identifier: GPL-3.0-only
 * Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
 * Native-only RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001 experiment.
 * Published FLL ABI and documented CRT calls; no VFP implementation linkage.
 * No conversion hypothesis or control-word modification is implemented here.
 */
#pragma pack(push, 1)
typedef struct {
    char type, padding;
    short width;
    unsigned decimals;
    long integer;
    double real;
    unsigned char currency[8];
    unsigned handle, object;
} NumericValue;
typedef struct { short count; NumericValue args[2]; } NumericParameters;
/* The packed table stores a code address; type it as the actual fastcall
 * handler rather than casting through the SDK's historical generic pointer. */
typedef void (__fastcall *PublicFunction)(NumericParameters *);
typedef struct { char *name; PublicFunction function; short count; char *types; } PublicInfo;
typedef struct PublicTable { struct PublicTable *next; short count; PublicInfo *info; } PublicTable;
#pragma pack(pop)
_Static_assert(sizeof(long)==4 && sizeof(void *)==4, "32-bit FLL only");
_Static_assert(sizeof(NumericValue)==36 && __builtin_offsetof(NumericValue,real)==12,
               "published byte-packed Value layout");
_Static_assert(sizeof(PublicInfo)==14 && __builtin_offsetof(PublicInfo,function)==4,
               "published packed dispatch address layout");
extern void __fastcall _RetChar(char *);
__declspec(dllimport) void * __stdcall GetModuleHandleA(const char *);
__declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void *,char *,unsigned long);
__declspec(dllimport) void * __stdcall GetProcAddress(void *,const char *);
typedef char * (__cdecl *ConvertFunction)(double,int,int *,int *);
int _fltused=0;

static char *text(char *out,const char *in) { while(*in) *out++=*in++; return out; }
static char *hex(char *out,unsigned value,unsigned count) {
    static const char digits[]="0123456789ABCDEF";
    while(count) { --count; *out++=digits[(value>>(4*count))&15]; }
    return out;
}
static unsigned control_word(void) {
    unsigned short value; __asm__ __volatile__("fnstcw %0":"=m"(value)); return value;
}
static int numeric(const NumericValue *v,double *out) {
    if(v->type=='I') { *out=(double)v->integer; return 1; }
    if(v->type!='N') return 0;
    for(unsigned i=0;i<8;++i) ((unsigned char *)out)[i]=((const unsigned char *)&v->real)[i];
    return 1;
}
static void convert(NumericParameters *p,int significant) {
    union { double value; unsigned char bytes[8]; } input;
    double count_value;
    if(p->count!=2 || !numeric(&p->args[0],&input.value) || !numeric(&p->args[1],&count_value)) {
        _RetChar("ERROR:PARAMETER"); return;
    }
    /* Reject nonfinite payload before the CRT. Bounded finite count before cast. */
    if((input.bytes[7]&0x7f)==0x7f && (input.bytes[6]&0xf0)==0xf0) {
        _RetChar("ERROR:NONFINITE"); return;
    }
    if(!(count_value>=1 && count_value<=18)) { _RetChar("ERROR:COUNT"); return; }
    int count=(int)count_value;
    if((double)count!=count_value || (significant && count!=15 && count!=16 && count!=17)) {
        _RetChar("ERROR:COUNT"); return;
    }
    /* Query the module already loaded by VFP, never load/replace a DLL. */
    void *module=GetModuleHandleA("msvcr71.dll");
    if(!module) { _RetChar("ERROR:MODULE"); return; }
    char module_path[260];
    unsigned long length=GetModuleFileNameA(module,module_path,sizeof(module_path));
    if(!length || length>=sizeof(module_path)) { _RetChar("ERROR:MODULEPATH"); return; }
    module_path[length]=0;
    ConvertFunction function=(ConvertFunction)GetProcAddress(module,significant?"_ecvt":"_fcvt");
    if(!function) { _RetChar("ERROR:EXPORT"); return; }
    int decimal=0,sign=0;
    unsigned before=control_word();
    const char *result=function(input.value,count,&decimal,&sign);
    unsigned after=control_word();
    if(!result) { _RetChar("ERROR:NULL"); return; }
    /* Copy the shared CRT buffer immediately, before any other CRT/FLL call. */
    char digits[128]; unsigned used=0;
    for(;used<sizeof(digits);++used) {
        char c=result[used];
        if(!c) { digits[used]=0; break; }
        if(c<'0' || c>'9') { _RetChar("ERROR:DIGIT"); return; }
        digits[used]=c;
    }
    if(used==sizeof(digits)) { _RetChar("ERROR:BUFFERBOUND"); return; }
    /* Maximum: 518 module hex +254 digit hex + fixed fields <1200 bytes. */
    char output[1200],*out=output;
    out=text(out,significant?"kind=E;count=":"kind=F;count=");
    out=hex(out,(unsigned)count,8);
    out=text(out,";real="); for(int i=7;i>=0;--i) out=hex(out,input.bytes[i],2);
    out=text(out,";decimal="); out=hex(out,(unsigned)decimal,8);
    out=text(out,";sign="); out=hex(out,(unsigned)sign,8);
    out=text(out,";cw0="); out=hex(out,before,4);
    out=text(out,";cw1="); out=hex(out,after,4);
    out=text(out,";module="); for(unsigned i=0;i<length;++i) out=hex(out,(unsigned char)module_path[i],2);
    out=text(out,";digits="); for(unsigned i=0;i<used;++i) out=hex(out,(unsigned char)digits[i],2);
    *out=0; _RetChar(output);
}
static void __fastcall fixed_conversion(NumericParameters *p) { convert(p,0); }
static void __fastcall significant_conversion(NumericParameters *p) { convert(p,1); }
static PublicInfo functions[]={
    {"CFNFCVT",fixed_conversion,2,"NN"},
    {"CFNECVT",significant_conversion,2,"NN"}
};
__declspec(dllexport) PublicTable _FoxTable={0,2,functions};
/* Same first-party entry pattern as cause-probe.c; installed SDK stays host-only. */
typedef void (*Initializer)(void);
#pragma section(".CRT$XCA",read)
#pragma section(".CRT$XCZ",read)
__declspec(allocate(".CRT$XCA")) Initializer initialize_begin[]={0};
__declspec(allocate(".CRT$XCZ")) Initializer initialize_end[]={0};
extern int __stdcall DllMain(void *,unsigned,void *);
int __stdcall CrtEntry(void *module,unsigned reason,void *reserved) {
    if(reason==1) for(Initializer *p=initialize_begin+1;p<initialize_end;++p) if(*p) (*p)();
    return DllMain(module,reason,reserved);
}

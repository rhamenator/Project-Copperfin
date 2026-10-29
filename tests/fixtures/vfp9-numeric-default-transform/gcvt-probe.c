/* Copyright © 2026 Richard M. Hamilton.
 * SPDX-License-Identifier: GPL-3.0-only
 * Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
 * Native-only RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001 experiment.
 * Published FLL ABI and documented CRT calls; no VFP implementation linkage.
 * No conversion hypothesis or control-word modification is implemented here.
 */
#include "crt-binding.h"
#include "gcvt-boundary.h"
#include "sprintf-boundary.h"
/* Default experiment unchanged. The separate precision discriminator opts in
 * to counts19..21; this is NOT a VFP precision cap or production policy. */
#ifndef CRT_FIXED_COUNT_MAX
#define CRT_FIXED_COUNT_MAX 18
#endif
_Static_assert(CRT_FIXED_COUNT_MAX==18 || CRT_FIXED_COUNT_MAX==21,
               "only the two source-frozen experimental count bounds");
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
__declspec(dllimport) void * __stdcall CreateFileA(const char *,unsigned long,unsigned long,void *,unsigned long,unsigned long,void *);
__declspec(dllimport) int __stdcall GetFileSizeEx(void *,long long *);
__declspec(dllimport) int __stdcall ReadFile(void *,void *,unsigned long,unsigned long *,void *);
__declspec(dllimport) int __stdcall CloseHandle(void *);
__declspec(dllimport) unsigned long __stdcall GetLastError(void);
typedef struct { void *base,*allocation; unsigned long allocation_protect,region_size,state,protect,type; } MemoryInfo;
_Static_assert(sizeof(MemoryInfo)==28,"32-bit MEMORY_BASIC_INFORMATION");
__declspec(dllimport) unsigned long __stdcall VirtualQuery(const void *,MemoryInfo *,unsigned long);
typedef char * (__cdecl *ConvertFunction)(double,int,int *,int *);
int _fltused=0;

static void *bound_module;
static char bound_path[260];
static CrtPe bound_pe;
static unsigned bound_size;
static unsigned char bound_digest[32];
/* One-call diagnostic records the FIRST failed predicate/API operation. Public
 * metadata only; failure never bypasses a binding condition. */
static const char *binding_stage="NOT_RUN";
static unsigned binding_error,query_bytes,image_valid,file_valid,hash_valid,selftest_ok;
static unsigned file_size_low,file_size_high,read_bytes,read_calls,close_ok,close_error;
static unsigned base_matches,allocation_matches;
static MemoryInfo observed_memory;
static CrtPe observed_image,observed_file;
static int binding_fail(const char *stage,unsigned error) {
    binding_stage=stage;binding_error=error;return 0;
}
static int loaded_pe(void *module,CrtPe *pe) {
    query_bytes=VirtualQuery(module,&observed_memory,sizeof(observed_memory));
    if(!query_bytes) return binding_fail("QUERY_API",GetLastError());
    if(query_bytes!=sizeof(observed_memory)) return binding_fail("QUERY_SIZE",0);
    base_matches=observed_memory.base==module;allocation_matches=observed_memory.allocation==module;
    if(!base_matches || !allocation_matches || observed_memory.type!=0x1000000 ||
       observed_memory.state!=0x1000 || observed_memory.region_size<4096 || (observed_memory.protect&0x100) ||
       !(observed_memory.protect&(0x02|0x04|0x08|0x20|0x40|0x80))) return binding_fail("QUERY_PREDICATE",0);
    image_valid=crt_pe((const unsigned char *)module,4096,pe);
    return image_valid?1:binding_fail("IMAGE_HEADER",0);
}
/* Hash once per FLL lifetime through the exact runtime-reported path, in THIS
 * 32-bit process. Later calls recheck loaded headers/module/path. This is not
 * an atomic proof of mapped image content or a rehash on every conversion. */
static int bind_module(void *module,const char *path,unsigned length) {
    CrtPe *image=&observed_image,*file_pe=&observed_file;
    if(!loaded_pe(module,image)) return 0;
    if(bound_module) {
        if(module!=bound_module || !crt_pe_equal(image,&bound_pe)) return binding_fail("CACHE_IMAGE",0);
        for(unsigned i=0;i<=length;++i) if(path[i]!=bound_path[i]) return binding_fail("CACHE_PATH",0);
        binding_stage="BOUND";
        return 1;
    }
    selftest_ok=crt_sha_selftest();if(!selftest_ok) return binding_fail("SHA_SELFTEST",0);
    void *file=CreateFileA(path,0x80000000u,1,0,3,0x80,0);
    if(file==(void *)-1) return binding_fail("OPEN_API",GetLastError());
    long long size=0;
    int valid=GetFileSizeEx(file,&size);
    if(!valid) binding_fail("SIZE_API",GetLastError());
    else {
        file_size_low=(unsigned)(unsigned long long)size;file_size_high=(unsigned)((unsigned long long)size>>32);
        if(size<4096 || size>8388608) { valid=0;binding_fail("SIZE_BOUND",0); }
    }
    CrtSha sha;unsigned total=0,header_used=0,iterations=0;
    /* VFP invokes these synchronous handlers on its evaluation thread. Static
     * scratch avoids a large freestanding32-bit stack-probe dependency. */
    static unsigned char block[4096],header[4096];crt_sha_init(&sha);
    while(valid && total<(unsigned)size) {
        unsigned request=(unsigned)size-total;if(request>sizeof(block)) request=sizeof(block);
        unsigned long got=0;
        if(++iterations>4096) { valid=0;binding_fail("READ_LIMIT",0);break; }
        ++read_calls;
        if(!ReadFile(file,block,request,&got,0)) { valid=0;binding_fail("READ_API",GetLastError());break; }
        if(!got || got>request) { valid=0;binding_fail("READ_BOUND",0);break; }
        for(unsigned i=0;i<got && header_used<sizeof(header);++i) header[header_used++]=block[i];
        crt_sha_update(&sha,block,got);total+=got;read_bytes=total;
    }
    unsigned long extra=0;
    if(valid) {
        if(!ReadFile(file,block,1,&extra,0)) { valid=0;binding_fail("EOF_API",GetLastError()); }
        else if(extra) { valid=0;binding_fail("EOF_BOUND",0); }
    }
    close_ok=CloseHandle(file)!=0;
    if(!close_ok) { close_error=GetLastError();if(valid) binding_fail("CLOSE_API",close_error);valid=0; }
    if(!valid) return 0;
    if(total!=(unsigned)size) return binding_fail("READ_SIZE",0);
    /* Diagnostic-only digest survives a later HEADER mismatch, never marks
     * binding successful by itself. No file bytes are returned. */
    crt_sha_final(&sha,bound_digest);hash_valid=1;
    file_valid=crt_pe(header,header_used,file_pe);
    if(!file_valid) return binding_fail("FILE_HEADER",0);
    if(!crt_pe_equal(image,file_pe)) return binding_fail("HEADER_EQUAL",0);
    for(unsigned i=0;i<=length;++i) bound_path[i]=path[i];
    /* Explicit scalar copies keep this freestanding wrapper free of a compiler-
     * synthesized memcpy dependency (the first binding build retained link1). */
    bound_pe.machine=image->machine;bound_pe.magic=image->magic;bound_pe.stamp=image->stamp;
    bound_pe.image_size=image->image_size;bound_pe.checksum=image->checksum;
    bound_size=total;bound_module=module;binding_stage="BOUND";return 1;
}

static char *text(char *out,const char *in) { while(*in) *out++=*in++; return out; }
static char *hex(char *out,unsigned value,unsigned count) {
    static const char digits[]="0123456789ABCDEF";
    while(count) { --count; *out++=digits[(value>>(4*count))&15]; }
    return out;
}
static unsigned control_word(void) {
    unsigned short value; __asm__ __volatile__("fnstcw %0":"=m"(value)); return value;
}
static char *field(char *out,const char *name,unsigned value) {
    out=text(out,name);return hex(out,value,8);
}
static void __fastcall binding_diagnostic(NumericParameters *p) {
    static unsigned called;
    if(p->count || called || bound_module) { _RetChar("ERROR:DIAGNOSTIC_REUSE_OR_PARAMETER");return; }
    called=1;
    char module_path[260];module_path[0]=0;
    unsigned length=0,reported_length=0,bound=0,before=control_word();
    void *module=GetModuleHandleA("msvcr71.dll");
    if(!module) binding_fail("MODULE_API",GetLastError());
    else {
        reported_length=GetModuleFileNameA(module,module_path,sizeof(module_path));
        if(!reported_length) binding_fail("PATH_API",GetLastError());
        else if(reported_length>=sizeof(module_path)) binding_fail("PATH_BOUND",0);
        else { length=reported_length;module_path[length]=0;bound=bind_module(module,module_path,length); }
    }
    unsigned after=control_word();
    /* At most518 path-hex bytes + fixed field names/values (<1600 total).
     * No proprietary memory/file contents or process pointers are emitted. */
    char output[2048],*out=output;
    out=text(out,"stage=");out=text(out,binding_stage);
    out=field(out,";error=",binding_error);out=field(out,";bound=",bound);
    out=field(out,";pathLength=",reported_length);
    out=text(out,";module=");for(unsigned i=0;i<length;++i)out=hex(out,(unsigned char)module_path[i],2);
    out=field(out,";queryBytes=",query_bytes);out=field(out,";baseMatches=",base_matches);
    out=field(out,";allocationMatches=",allocation_matches);
    out=field(out,";regionSize=",observed_memory.region_size);out=field(out,";state=",observed_memory.state);
    out=field(out,";protect=",observed_memory.protect);out=field(out,";memoryType=",observed_memory.type);
    out=field(out,";imageValid=",image_valid);out=field(out,";imageMachine=",observed_image.machine);
    out=field(out,";imageMagic=",observed_image.magic);out=field(out,";imageStamp=",observed_image.stamp);
    out=field(out,";imageSize=",observed_image.image_size);out=field(out,";imageChecksum=",observed_image.checksum);
    out=field(out,";selftest=",selftest_ok);out=field(out,";fileSizeLow=",file_size_low);
    out=field(out,";fileSizeHigh=",file_size_high);out=field(out,";readBytes=",read_bytes);
    out=field(out,";readCalls=",read_calls);out=field(out,";closeOk=",close_ok);out=field(out,";closeError=",close_error);
    out=field(out,";fileValid=",file_valid);out=field(out,";fileMachine=",observed_file.machine);
    out=field(out,";fileMagic=",observed_file.magic);out=field(out,";fileStamp=",observed_file.stamp);
    out=field(out,";fileImageSize=",observed_file.image_size);out=field(out,";fileChecksum=",observed_file.checksum);
    out=field(out,";hashValid=",hash_valid);
    out=text(out,";fileSha256=");for(unsigned i=0;i<32;++i)out=hex(out,bound_digest[i],2);
    out=text(out,";cw0=");out=hex(out,before,4);out=text(out,";cw1=");out=hex(out,after,4);
    *out=0;_RetChar(output);
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
    if(!(count_value>=1 && count_value<=CRT_FIXED_COUNT_MAX)) { _RetChar("ERROR:COUNT"); return; }
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
    if(!bind_module(module,module_path,length)) { _RetChar("ERROR:MODULEBINDING"); return; }
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
    /* Maximum: 518 module hex +254 digit hex + identity/fixed fields <1536. */
    char output[1536],*out=output;
    out=text(out,significant?"kind=E;count=":"kind=F;count=");
    out=hex(out,(unsigned)count,8);
    out=text(out,";real="); for(int i=7;i>=0;--i) out=hex(out,input.bytes[i],2);
    out=text(out,";decimal="); out=hex(out,(unsigned)decimal,8);
    out=text(out,";sign="); out=hex(out,(unsigned)sign,8);
    out=text(out,";cw0="); out=hex(out,before,4);
    out=text(out,";cw1="); out=hex(out,after,4);
    out=text(out,";module="); for(unsigned i=0;i<length;++i) out=hex(out,(unsigned char)module_path[i],2);
    out=text(out,";fileSample=ONCE;fileSize=");out=hex(out,bound_size,8);
    out=text(out,";fileSha256=");for(unsigned i=0;i<32;++i) out=hex(out,bound_digest[i],2);
    out=text(out,";machine=");out=hex(out,bound_pe.machine,4);
    out=text(out,";magic=");out=hex(out,bound_pe.magic,4);
    out=text(out,";stamp=");out=hex(out,bound_pe.stamp,8);
    out=text(out,";imageSize=");out=hex(out,bound_pe.image_size,8);
    out=text(out,";checksum=");out=hex(out,bound_pe.checksum,8);
    out=text(out,";digits="); for(unsigned i=0;i<used;++i) out=hex(out,(unsigned char)digits[i],2);
    *out=0; _RetChar(output);
}
static void __fastcall fixed_conversion(NumericParameters *p) { convert(p,0); }
static void __fastcall significant_conversion(NumericParameters *p) { convert(p,1); }
/* BEGIN first-party guarded _gcvt handler. The preceding binding/old controls are unchanged. */
static void __fastcall general_conversion(NumericParameters *p) {
    union { double value; unsigned char bytes[8]; } input;
    double count_value;
    if(p->count!=2 || !numeric(&p->args[0],&input.value) || !numeric(&p->args[1],&count_value)) {
        _RetChar("ERROR:GCVT_PARAMETER");return;
    }
    if((input.bytes[7]&0x7f)==0x7f && (input.bytes[6]&0xf0)==0xf0) {
        _RetChar("ERROR:GCVT_NONFINITE");return;
    }
    if(!(count_value==15 || count_value==16 || count_value==17)) {
        _RetChar("ERROR:GCVT_COUNT");return;
    }
    void *module=GetModuleHandleA("msvcr71.dll");
    if(!module) { _RetChar("ERROR:MODULE");return; }
    char module_path[260];
    unsigned long length=GetModuleFileNameA(module,module_path,sizeof(module_path));
    if(!length || length>=sizeof(module_path)) { _RetChar("ERROR:MODULEPATH");return; }
    module_path[length]=0;
    if(!bind_module(module,module_path,length)) { _RetChar("ERROR:MODULEBINDING");return; }
    GcvtFunction function=(GcvtFunction)GetProcAddress(module,"_gcvt");
    if(!function) { _RetChar("ERROR:GCVT_EXPORT");return; }
    /* Own fixed-size static buffer: VFP's synchronous evaluation-thread handler.
     * No CRT, allocator, DLL load or control-word write in the shared guard. */
    static GcvtBuffer buffer;
    GcvtResult result;
    unsigned before=control_word();
    int status=gcvt_call(function,input.value,count_value,&buffer,&result);
    unsigned after=control_word();
    if(status!=GCVT_OK) {
        char error[40],*e=text(error,"ERROR:GCVT_BOUNDARY:");
        e=hex(e,(unsigned)status,8);*e=0;_RetChar(error);return;
    }
    /* <=518 module hex +254 text hex + fixed names/identity <1536. */
    char output[1536],*out=output;
    out=text(out,"kind=G;count=");out=hex(out,(unsigned)(int)count_value,8);
    out=text(out,";real=");for(int i=7;i>=0;--i)out=hex(out,input.bytes[i],2);
    out=text(out,";cw0=");out=hex(out,before,4);
    out=text(out,";cw1=");out=hex(out,after,4);
    out=text(out,";module=");for(unsigned i=0;i<length;++i)out=hex(out,(unsigned char)module_path[i],2);
    out=text(out,";fileSample=ONCE;fileSize=");out=hex(out,bound_size,8);
    out=text(out,";fileSha256=");for(unsigned i=0;i<32;++i)out=hex(out,bound_digest[i],2);
    out=text(out,";machine=");out=hex(out,bound_pe.machine,4);
    out=text(out,";magic=");out=hex(out,bound_pe.magic,4);
    out=text(out,";stamp=");out=hex(out,bound_pe.stamp,8);
    out=text(out,";imageSize=");out=hex(out,bound_pe.image_size,8);
    out=text(out,";checksum=");out=hex(out,bound_pe.checksum,8);
    out=text(out,";capacity=");out=hex(out,GCVT_CAPACITY,8);
    out=text(out,";canary=PASS;pointer=OWN;text=");
    for(unsigned i=0;i<result.used;++i)out=hex(out,(unsigned char)result.text[i],2);
    *out=0;_RetChar(output);
}
/* END first-party guarded _gcvt handler. */
/* BEGIN first-party guarded public sprintf/_snprintf comparison handler. */
static void __fastcall sprintf_conversion(NumericParameters *p) {
    union { double value; unsigned char bytes[8]; } input;
    double count_value;
    if(p->count!=2 || !numeric(&p->args[0],&input.value) || !numeric(&p->args[1],&count_value)) {
        _RetChar("ERROR:SPRINTF_PARAMETER");return;
    }
    if((input.bytes[7]&0x7f)==0x7f && (input.bytes[6]&0xf0)==0xf0) {
        _RetChar("ERROR:SPRINTF_NONFINITE");return;
    }
    if(!(count_value==15 || count_value==16 || count_value==17 || count_value==18)) {
        _RetChar("ERROR:SPRINTF_COUNT");return;
    }
    /* Query the exact module already loaded by VFP; never load or replace it. */
    void *module=GetModuleHandleA("msvcr71.dll");
    if(!module) { _RetChar("ERROR:MODULE");return; }
    char module_path[260];
    unsigned long length=GetModuleFileNameA(module,module_path,sizeof(module_path));
    if(!length || length>=sizeof(module_path)) { _RetChar("ERROR:MODULEPATH");return; }
    module_path[length]=0;
    if(!bind_module(module,module_path,length)) { _RetChar("ERROR:MODULEBINDING");return; }
    SprintfFunction sprintf_function=(SprintfFunction)GetProcAddress(module,"sprintf");
    SnprintfFunction snprintf_function=(SnprintfFunction)GetProcAddress(module,"_snprintf");
    if(!sprintf_function || !snprintf_function) { _RetChar("ERROR:SPRINTF_EXPORT");return; }
    /* VFP invokes synchronous evaluation-thread handlers. Static owned buffers
     * avoid allocator use and a large freestanding stack-probe dependency. */
    static SprintfBuffer sprintf_buffer,snprintf_buffer;
    SprintfResult result;
    unsigned before=control_word();
    int status=sprintf_call(sprintf_function,snprintf_function,input.value,count_value,
                            &sprintf_buffer,&snprintf_buffer,&result);
    unsigned after=control_word();
    if(status!=SPRINTF_OK) {
        char error[44],*e=text(error,"ERROR:SPRINTF_BOUNDARY:");
        e=hex(e,(unsigned)status,8);*e=0;_RetChar(error);return;
    }
    /* <=518 module hex +2*766 text hex + identity/fixed fields <4096. Both
     * public results are emitted even when equality or grammar is false. */
    static char output[4096];char *out=output;
    out=text(out,"kind=S;count=");out=hex(out,(unsigned)(int)count_value,8);
    out=text(out,";real=");for(int i=7;i>=0;--i)out=hex(out,input.bytes[i],2);
    out=text(out,";cw0=");out=hex(out,before,4);out=text(out,";cw1=");out=hex(out,after,4);
    out=text(out,";module=");for(unsigned i=0;i<length;++i)out=hex(out,(unsigned char)module_path[i],2);
    out=text(out,";fileSample=ONCE;fileSize=");out=hex(out,bound_size,8);
    out=text(out,";fileSha256=");for(unsigned i=0;i<32;++i)out=hex(out,bound_digest[i],2);
    out=text(out,";machine=");out=hex(out,bound_pe.machine,4);
    out=text(out,";magic=");out=hex(out,bound_pe.magic,4);
    out=text(out,";stamp=");out=hex(out,bound_pe.stamp,8);
    out=text(out,";imageSize=");out=hex(out,bound_pe.image_size,8);
    out=text(out,";checksum=");out=hex(out,bound_pe.checksum,8);
    out=text(out,";capacity=");out=hex(out,SPRINTF_CAPACITY,8);
    out=text(out,";canary=PASS;sprintfGrammar=");out=hex(out,result.sprintf_grammar,1);
    out=text(out,";snprintfGrammar=");out=hex(out,result.snprintf_grammar,1);
    out=text(out,";equal=");out=hex(out,result.equal,1);
    out=text(out,";sprintf=");for(unsigned i=0;i<result.sprintf_used;++i)out=hex(out,(unsigned char)result.sprintf_text[i],2);
    out=text(out,";snprintf=");for(unsigned i=0;i<result.snprintf_used;++i)out=hex(out,(unsigned char)result.snprintf_text[i],2);
    *out=0;_RetChar(output);
}
/* END first-party guarded public sprintf/_snprintf comparison handler. */
static PublicInfo functions[]={
    {"CFNFCVT",fixed_conversion,2,"NN"},
    {"CFNECVT",significant_conversion,2,"NN"},
    {"CFNBIND",binding_diagnostic,0,""},
    {"CFNGCVT",general_conversion,2,"NN"},
    {"CFNSPRTF",sprintf_conversion,2,"NN"}
};
__declspec(dllexport) PublicTable _FoxTable={0,5,functions};
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

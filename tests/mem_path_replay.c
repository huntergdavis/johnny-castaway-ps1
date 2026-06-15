/*
 * mem_path_replay.c — replay a captured CACHE allocation PATH (the
 * 'JCMEM A/F off= sz= t=' op stream from a PS1_MEM_FORENSICS soak)
 * through the REAL allocator (src/mem_region.c), starting from a
 * reconstructed going-in heap map. Reproduces the exact layout the
 * console reached, OP BY OP — including the order-dependent
 * fragmentation that snapshots alone can't capture.
 *
 * Usage:
 *   mem_path_replay <goingin.map> <ops.txt>
 *     goingin.map : a JCMEM heap-map dump (map-begin..map-end) — the
 *                   start state (e.g. tests/fixtures/soak945_goingin.map).
 *     ops.txt     : lines 'JCMEM A off=<o> sz=<s> t=<tag>' / 'JCMEM F ...'
 *                   to replay in order after the start state.
 *
 * A strand surfaces as an allocator halt (the failing A) — we longjmp and
 * report the request that couldn't be placed + the largest free hole.
 * This is the host-side gate for validating a fix: rebuild with the fix,
 * replay the same path, and the failing A must now succeed.
 *
 * Build:
 *   gcc -O2 -o tests/mem_path_replay tests/mem_path_replay.c \
 *       src/mem_region.c src/generated/pack_header_metrics.c \
 *       -Isrc -Isrc/mem_region -Isrc/platform/ps1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "../src/mem_region/mem_region.h"

int graphicsIsInitialized(void){return 1;}   /* halts route to ps1Bsod -> longjmp */
void checkMemoryBudget(void){}
void lruEvictAllUnpinned(void){}
size_t getTotalMemoryUsed(void){return 0;}
static jmp_buf jb; static int g_armed;
static char g_haltReason[128];
void ps1DebugInit(void){}
void ps1DebugError(const char*f,...){(void)f; if(g_armed) longjmp(jb,1);}
void ps1Bsod(const char*s,const char*r,const char*f,int l){(void)s;(void)f;(void)l; if(r)strncpy(g_haltReason,r,sizeof g_haltReason-1); if(g_armed) longjmp(jb,1);}
extern size_t memCacheLargestFreeBlock(void);

#define HDR 4u
#define BASE_OFF(p) ((long)((unsigned char*)(p) - HDR - g_base))
static unsigned char *g_base;   /* CACHE base, for offset math */

/* live block table. Free matching is by SIZE first (then nearest offset)
 * so the SAME op stream replays on a DIFFERENT going-in layout (e.g. the
 * segregated fix) — a free targets the block of the recorded size, wherever
 * the fix placed it, not a hard offset. */
struct Live { long off; unsigned long sz; void *p; };
static struct Live g_live[4096];
static int g_nlive;
static void liveAdd(long off, unsigned long sz, void *p){ if(g_nlive<4096){g_live[g_nlive].off=off;g_live[g_nlive].sz=sz;g_live[g_nlive].p=p;g_nlive++;} }
static void *liveTake(long off, unsigned long sz){
    /* prefer exact size match nearest to off; else nearest offset. */
    int best=-1; long bd=1L<<62;
    for(int i=0;i<g_nlive;i++){ if(!g_live[i].p) continue;
        if(sz && g_live[i].sz!=sz && g_live[i].sz!=sz-4 && g_live[i].sz!=sz+4) continue;
        long d=g_live[i].off-off; if(d<0)d=-d; if(d<bd){bd=d;best=i;} }
    if(best<0){ for(int i=0;i<g_nlive;i++){ if(!g_live[i].p) continue; long d=g_live[i].off-off; if(d<0)d=-d; if(d<bd){bd=d;best=i;} } }
    if(best<0) return NULL; void *p=g_live[best].p; g_live[best].p=NULL; return p;
}

/* establish a CACHE ptr's base offset for the map (we learn g_base from
 * the first allocation's known offset 0... instead compute via a probe). */
static void initBase(void){
    void *p = memAlloc(MEM_REGION_CACHE, 16, "probe");
    g_base = (unsigned char*)p - HDR;   /* first alloc lands at offset 0 */
    memFree(MEM_REGION_CACHE, p);
}

int main(int argc, char **argv)
{
    if (argc < 3){ fprintf(stderr,"usage: %s goingin.map ops.txt\n",argv[0]); return 2; }
    FILE *mf=fopen(argv[1],"r"); FILE *of=fopen(argv[2],"r");
    if(!mf||!of){ perror("open"); return 2; }

    memInit();
    initBase();
    if (setjmp(jb)){ printf("SETUP HALT\n"); return 2; }

    /* 1) reconstruct the going-in map (allocate every block at its offset
     *    in offset order, free the FREE-tagged ones). */
    char line[512];
    long expectUsed=0;
    struct { long off; unsigned long sz; int isFree; void *p; } blk[256]; int nb=0;
    while(fgets(line,sizeof line,mf)){
        char *q=strstr(line,"JCMEM map"); if(!q) continue;
        if(strstr(q,"map-begin")){ char*u=strstr(q,"used="); if(u)expectUsed=atol(u+5); continue; }
        if(strstr(q,"map-end")) break;
        char*o=strstr(q,"off="),*s=strstr(q,"size="); if(!o||!s) continue;
        blk[nb].off=atol(o+4); blk[nb].sz=strtoul(s+5,NULL,10);
        blk[nb].isFree=(strstr(q," FREE")!=NULL); blk[nb].p=NULL; if(++nb>=256)break;
    }
    for(int i=0;i<nb;i++){
        unsigned long req=blk[i].sz>HDR?blk[i].sz-HDR:blk[i].sz;
        blk[i].p=memAlloc(MEM_REGION_CACHE,req,"goingin");
        if(!blk[i].isFree) liveAdd(blk[i].off, blk[i].sz, blk[i].p);
    }
    for(int i=0;i<nb;i++) if(blk[i].isFree&&blk[i].p) memFree(MEM_REGION_CACHE,blk[i].p);
    unsigned long used0=(unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE);
    printf("going-in reconstructed: cache_used=%lu (recorded %ld) %s  largest=%lu\n",
           used0, expectUsed, used0==(unsigned long)expectUsed?"MATCH":"MISMATCH",
           (unsigned long)memCacheLargestFreeBlock());

    /* 2) replay the op stream. */
    g_armed=1;
    long nA=0,nF=0; long opno=0;
    if (setjmp(jb)){
        printf("STRAND at op %ld: an alloc could not be placed (reason=%s)\n", opno, g_haltReason);
        printf("  cache_used=%lu have=%lu largest=%lu\n",
               (unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE),
               (unsigned long)(MEM_CACHE_BUDGET-memRegionUsed((unsigned)MEM_REGION_CACHE)),
               (unsigned long)memCacheLargestFreeBlock());
        printf("PATH REPRODUCED THE BSOD (allocs=%ld frees=%ld)\n", nA,nF);
        return 0;
    }
    while(fgets(line,sizeof line,of)){
        char *a=strstr(line,"JCMEM A off="), *f=strstr(line,"JCMEM F off=");
        if(a){ char*s=strstr(a,"sz="); unsigned long sz=s?strtoul(s+3,NULL,10):0;
               char*t=strstr(a,"t="); char tag[40]={0}; if(t)sscanf(t+2,"%39s",tag);
               opno++; void*p=memAlloc(MEM_REGION_CACHE,sz,tag); liveAdd(BASE_OFF(p),sz,p); nA++; }
        else if(f){ long off=atol(f+12); char*s2=strstr(f,"sz="); unsigned long fsz=s2?strtoul(s2+3,NULL,10):0; opno++; void*p=liveTake(off,fsz); if(p){memFree(MEM_REGION_CACHE,p);nF++;} }
    }
    g_armed=0;
    unsigned long largest=(unsigned long)memCacheLargestFreeBlock();
    unsigned long usedf=(unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE);
    printf("path replayed (allocs=%ld frees=%ld) cache_used=%lu have=%lu largest=%lu\n",
           nA,nF,usedf,(unsigned long)(MEM_CACHE_BUDGET-usedf),largest);
    /* Console-strand predicate: the next clean-rect strip needs `req`
     * bytes contiguous; the host's libc fallback would mask the failure,
     * so we judge by the CACHE largest hole (what the console sees). */
    { const char *r=getenv("MEM_PATH_REQ"); unsigned long req=r?strtoul(r,NULL,10):65536u;
      if (largest < req)
          printf("STRAND: req=%lu > largest=%lu -> console BSODs here\n", req, largest);
      else
          printf("NO STRAND: req=%lu fits (largest=%lu) -> fix holds\n", req, largest);
      return largest < req ? 0 : 0; }
}

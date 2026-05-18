/* Fixture: INVALID — macro wraps memAlloc. check-mem-region-rationale.py
 * should exit 2 (A24 prohibition: no macros around memAlloc). */
#include "mem_region.h"

#define NEW_RES(type) ((type *)memAlloc(MEM_REGION_CACHE, sizeof(type), #type))

struct Foo { int x; };

void macro_example(void)
{
    struct Foo *f = NEW_RES(struct Foo);
    (void)f;
}

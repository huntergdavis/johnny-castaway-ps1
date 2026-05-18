/* Fixture: VALID. The check-mem-region-rationale.py script should
 * exit 0 on this file. */
#include "mem_region.h"

void valid_example(void)
{
    /* MEM_REGION_RATIONALE: per-scene scratch for X. */
    uint8_t *buf = memAlloc(MEM_REGION_TRANSIENT, 1024, "validBuf");
    (void)buf;
}

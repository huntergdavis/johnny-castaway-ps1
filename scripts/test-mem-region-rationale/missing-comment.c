/* Fixture: INVALID — no MEM_REGION_RATIONALE comment.
 * check-mem-region-rationale.py should exit 1 on this file. */
#include "mem_region.h"

void missing_comment_example(void)
{
    uint8_t *buf = memAlloc(MEM_REGION_TRANSIENT, 1024, "noRationale");
    (void)buf;
}

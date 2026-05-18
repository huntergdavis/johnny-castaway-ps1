/* Fixture: VALID. memAlloc call signature is split across lines;
 * the RATIONALE comment is directly above. Should exit 0. */
#include "mem_region.h"

void multi_line_example(void)
{
    /* MEM_REGION_RATIONALE: split-line call, large allocation. */
    uint8_t *buf = (uint8_t *)memAlloc(MEM_REGION_TRANSIENT,
                                       64 * 1024,
                                       "multiLineBuf");
    (void)buf;
}

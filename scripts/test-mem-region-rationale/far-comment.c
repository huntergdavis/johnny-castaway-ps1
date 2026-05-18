/* Fixture: INVALID — MEM_REGION_RATIONALE comment too far above
 * the memAlloc call (>10 lines). check-mem-region-rationale.py
 * should exit 1 on this file. */
#include "mem_region.h"

void far_comment_example(void)
{
    /* MEM_REGION_RATIONALE: ... */

    /* Padding line 1 */
    /* Padding line 2 */
    /* Padding line 3 */
    /* Padding line 4 */
    /* Padding line 5 */
    /* Padding line 6 */
    /* Padding line 7 */
    /* Padding line 8 */
    /* Padding line 9 */
    /* Padding line 10 */
    uint8_t *buf = memAlloc(MEM_REGION_TRANSIENT, 1024, "farBuf");
    (void)buf;
}

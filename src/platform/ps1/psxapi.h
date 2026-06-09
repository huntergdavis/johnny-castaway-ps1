/*
 * Local PSn00bSDK wrapper.
 *
 * PSn00bSDK 0.24 intentionally uses out-of-int-range enum values and
 * anonymous structs/unions in psxapi.h. Keep -Wpedantic enabled for project
 * code, but do not report SDK header implementation details as our warnings.
 */

#ifndef JC_PS1_PSXAPI_WRAPPER_H
#define JC_PS1_PSXAPI_WRAPPER_H

#pragma GCC system_header
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include_next <psxapi.h>
#pragma GCC diagnostic pop

#endif /* JC_PS1_PSXAPI_WRAPPER_H */

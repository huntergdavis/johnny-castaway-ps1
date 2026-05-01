/*
 * Local PSn00bSDK wrapper.
 *
 * PSn00bSDK 0.24's pad response structs use anonymous unions/structs as a
 * GCC extension. Suppress that external pedantry while keeping project files
 * built with -Wpedantic.
 */

#ifndef JC_PS1_PSXPAD_WRAPPER_H
#define JC_PS1_PSXPAD_WRAPPER_H

#pragma GCC system_header
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include_next <psxpad.h>
#pragma GCC diagnostic pop

#endif /* JC_PS1_PSXPAD_WRAPPER_H */

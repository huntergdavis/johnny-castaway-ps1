/*
 * Scripted PS1 pad input for headless menu/regression tests.
 *
 * This is opt-in through BOOTMODE. Normal builds pay one disabled branch in
 * ps1PadScriptMergeButtons() and do not parse or log anything.
 */

#ifndef PS1_PAD_SCRIPT_H
#define PS1_PAD_SCRIPT_H

#include "mytypes.h"

void ps1PadScriptConfigureFromEmbedded(int enabled, int verbose);
uint16 ps1PadScriptMergeButtons(uint16 physicalButtons);
int ps1PadScriptIsEnabled(void);
int ps1PadScriptVerboseLogEnabled(void);

#endif /* PS1_PAD_SCRIPT_H */

#ifndef PS1_BOOT_PROGRESS_H
#define PS1_BOOT_PROGRESS_H

#include "mytypes.h"

/* Boot loading indicator: the Lilliputian ship sails left-to-right along
 * the bottom of the title screen, towing a blue wake bar. Each boot stage
 * advances it, so a stuck boot freezes the ship at a position that names
 * the stage (see the milestone map in ps1_boot_progress.c). No-ops until
 * ps1BootProgressBegin() (needs the GPU up) and after ...Finish(). */
void ps1BootProgressBegin(void);
void ps1BootProgress(uint8 pct);
void ps1BootProgressFinish(void);

#endif /* PS1_BOOT_PROGRESS_H */

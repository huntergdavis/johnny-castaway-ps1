/*
 * Memory card persistence for Johnny Castaway PS1 settings.
 *
 * Stores user preferences (sound mute, day/night override, holiday
 * override, time-set toggle, soft date/time) so they survive across
 * power cycles. Read on boot, written when the user picks "Save
 * Settings" from the pause menu.
 */
#ifndef MEMCARD_H
#define MEMCARD_H

/* Returns 1 if a save was found and loaded into globals; 0 otherwise. */
int  memcardLoadSettings(void);

/* Persists current global settings to memcard slot 1. Returns 1 on
 * success, 0 on failure. */
int  memcardSaveSettings(void);

/* Last-result message — non-NULL after any load/save call. Useful to
 * display in pause menu Debug Info. */
extern const char *memcardLastStatus;

#endif /* MEMCARD_H */

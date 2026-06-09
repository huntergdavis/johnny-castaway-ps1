/*
 *  This file is part of 'Johnny Reborn' - PS1 port.
 *
 *  Direct-control freeplay scene.
 */
#ifndef SCENE_FREEPLAY_H
#define SCENE_FREEPLAY_H

void freeplayRun(void);
int  freeplayExitRequested(void);
void freeplayClearExitRequest(void);
void freeplaySetTelemetryLevel(int level);
int  freeplayGetTelemetryLevel(void);

int freeplayGagCount(void);
int freeplayGagFrames(int index);
int freeplayGagMemoryKB(int index);

int freeplayVisitorCount(void);
int freeplayVisitorFrames(int index);
int freeplayVisitorMemoryKB(int index);

#endif /* SCENE_FREEPLAY_H */

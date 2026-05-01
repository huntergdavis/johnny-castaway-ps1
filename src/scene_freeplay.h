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

#endif /* SCENE_FREEPLAY_H */

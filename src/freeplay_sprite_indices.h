/*
 *  This file is part of 'Johnny Reborn' - PS1 port.
 *
 *  Freeplay sprite-index hints. These are intentionally conservative:
 *  every draw path still wraps against the loaded BMP's actual frame
 *  count, so bad guesses fail soft instead of walking past a sprite
 *  table.
 */
#ifndef FREEPLAY_SPRITE_INDICES_H
#define FREEPLAY_SPRITE_INDICES_H

enum {
    FP_JOHN_IDLE_E = 16,
    FP_JOHN_IDLE_W = 17,
    FP_JOHN_IDLE_N = 18,
    FP_JOHN_IDLE_S = 15,
};

static const unsigned short kFpWalkEastFrames[] = { 4, 5, 6, 7, 8, 1, 2, 3 };
static const unsigned short kFpWalkWestFrames[] = { 4, 5, 6, 7, 8, 1, 2, 3 };
static const unsigned short kFpWalkNorthFrames[] = { 19, 20, 21, 22, 23, 11 };
static const unsigned short kFpWalkSouthFrames[] = { 24, 25, 26, 27, 28, 29 };

#endif /* FREEPLAY_SPRITE_INDICES_H */

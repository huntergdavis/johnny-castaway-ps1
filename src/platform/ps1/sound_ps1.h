/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  PlayStation 1 audio implementation using PSn00bSDK SPU
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef SOUND_PS1_H
#define SOUND_PS1_H

extern int soundDisabled;
extern int soundMuted;

void soundInit(void);
void soundEnd(void);
void soundPlay(int nb);
void soundStop(int nb);
void soundMuteToggle(void);
int soundEffectCount(void);
int soundEffectLoaded(int nb);
unsigned long soundEffectSizeBytes(int nb);
int soundEffectSampleRate(int nb);

/* Ocean ambience — looping background audio.
 *
 * One CC0 ocean loop (~123 KB ADPCM, ~20 sec) loaded into SPU RAM at
 * boot, played on a dedicated voice with sample-defined loop flags so
 * playback runs forever at zero CPU cost. See
 * docs/ps1/background-music-feasibility.md for the full plan.
 *
 * oceanAmbientEnabled is the user-visible toggle (pause-menu +
 * memcard-persisted). Default ON. Toggling at runtime starts or
 * stops the SPU voice on the spot.
 *
 * Start / Stop are usually called via the toggle path, but the API
 * is exposed for boot-time auto-start. */
extern int oceanAmbientEnabled;

void oceanAmbientStart(void);
void oceanAmbientStop(void);
int  oceanAmbientLoaded(void);   /* 1 if VAG was uploaded to SPU RAM */

#endif /* SOUND_PS1_H */

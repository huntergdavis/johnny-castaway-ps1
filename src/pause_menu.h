/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  Pause menu overlay for PS1 build
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

#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

enum PauseMenuState {
    PAUSE_MENU_MAIN,
    PAUSE_MENU_OPTIONS,
    PAUSE_MENU_SCENE_INFO,
    PAUSE_MENU_CONTROLS,    /* legacy / no longer surfaced from main */
    PAUSE_MENU_SET_TIME,
    PAUSE_MENU_ISLAND_POS,
    PAUSE_MENU_SET_SEED,
};

/* Initialize the pause menu system (call once during startup, after graphicsInit). */
void pauseMenuInit(void);

/* Upload the embedded 8x8 ASCII font into the pause-menu VRAM region
 * if it isn't already there. Idempotent. Exposed so the closed-caption
 * overlay (ps1_captions.c) can render text using the same font without
 * having to wait for the user to open the pause menu first. */
void pauseMenuEnsureFontUploaded(void);

/* VRAM coords of the pause-menu font texture + its CLUT. Captions use
 * these to build SPRT primitives that sample the same glyph atlas. */
#define PAUSE_FONT_VRAM_X 640
#define PAUSE_FONT_VRAM_Y 256
#define PAUSE_CLUT_VRAM_X 640
#define PAUSE_CLUT_VRAM_Y 360
#define PAUSE_GLYPH_FIRST 0x20
#define PAUSE_GLYPH_COUNT 96
#define PAUSE_GLYPH_DRAW_W 16
#define PAUSE_GLYPH_DRAW_H 16

/* Show / hide the pause menu overlay. */
void pauseMenuShow(void);
void pauseMenuHide(void);

/* Query whether the pause menu is currently visible. */
int  pauseMenuIsVisible(void);

/* Return the current sub-screen state. */
enum PauseMenuState pauseMenuGetState(void);

/* Set the sub-screen state (e.g. back to main). */
void pauseMenuSetState(enum PauseMenuState state);

/*
 * Run one frame of the pause menu.
 * Reads controller input, updates menu state, and draws the overlay.
 * Returns 1 while the menu should stay open, 0 when the user resumes.
 */
int  pauseMenuUpdate(void);

/* Flag: set to 1 by the menu when "Next Scene" is chosen.
 * Cleared by the ADS/story loop after it acts on it. */
extern int pauseMenuRequestNextScene;

/* Flag: set to 1 by the menu when "Reset Loop" is chosen.
 * Foreground pilot's runtime loop checks this and exits early so
 * jc_reborn's outer loop can restart from the first scene. */
extern int pauseMenuRequestResetLoop;

#endif /* PAUSE_MENU_H */

/* input_map.h - host input bindings and emulator settings.
 *
 * The specification set covers the console side of $4016/$4017 (see input.h)
 * but has no host-input or configuration model; the binding table and the
 * settings below are the emulator's own (SPEC GAP UI-1). */
#ifndef INPUT_MAP_H
#define INPUT_MAP_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "input.h"
#include "nes.h"

typedef enum {
    CTL_A = 0, CTL_B, CTL_SELECT, CTL_START,
    CTL_UP, CTL_DOWN, CTL_LEFT, CTL_RIGHT,
    CTL_SAVE_STATE, CTL_LOAD_STATE, CTL_MENU, CTL_RESET,
    CTL_PAUSE, CTL_FAST_FORWARD, CTL_FRAME_ADVANCE, CTL_QUIT,
    CTL_COUNT
} CtlAction;

typedef struct Bindings {
    SDL_Scancode     key[CTL_COUNT];
    bool             has_key[CTL_COUNT];
    SDL_GamepadButton pad[CTL_COUNT];
    bool             has_pad[CTL_COUNT];
} Bindings;

typedef struct Settings {
    int  region;        /* 0 = NTSC, 1 = PAL, 2 = Dendy */
    int  scale;         /* window scale 1..8 */
    bool vsync;
    bool audio_enabled;
    int  volume;        /* 0..100 */
    bool show_fps;
    bool show_overlay;  /* input display / status overlay */
    bool overclock;     /* unused placeholder for future use */
    Bindings kb[2];
    Bindings pad[2];
} Settings;

extern const char *ctl_action_name[CTL_COUNT];
extern const char *region_names[3];

void settings_defaults(Settings *s);
bool settings_load(Settings *s);
bool settings_save(const Settings *s);

#endif /* INPUT_MAP_H */

/* hud.h - the in-emulator overlay menu.
 *
 * The specification set does not describe a user interface (SPEC GAP UI-1), so
 * the menu layout, the text renderer and the binding editor are the emulator's
 * own.  The menu is drawn with the embedded bitmap font in render.c and uses
 * SDL's logical presentation, so it is pixel-aligned with the NES picture.
 */
#ifndef HUD_H
#define HUD_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "render.h"
#include "input_map.h"

struct Nes;

typedef enum {
    HUD_CLOSED = 0,
    HUD_MAIN,
    HUD_CONTROLS,
    HUD_CONTROLS_P1_KB,
    HUD_CONTROLS_P1_PAD,
    HUD_CONTROLS_P2_KB,
    HUD_CONTROLS_P2_PAD,
    HUD_VIDEO,
    HUD_AUDIO,
    HUD_SAVESTATE,
    HUD_INFO
} HudScreen;

typedef struct Hud {
    HudScreen screen;
    int       cursor;
    int       scroll;
    bool      active;          /* true while a screen other than HUD_CLOSED is shown */
    bool      remapping;       /* waiting for a key/pad press to bind */
    bool      resume;          /* the "Resume" item asked for the game to run */
    int       remap_player;
    int       remap_action;
    bool      remap_pad;
    char      toast[64];
    int       toast_frames;
    Settings *settings;
    struct Nes *nes;
    int       slot;            /* selected save-state slot */
} Hud;

void hud_init(Hud *h, Settings *s, struct Nes *nes);
void hud_open(Hud *h);
void hud_close(Hud *h);
bool hud_take_resume(Hud *h);
bool hud_is_open(const Hud *h);

/* Handle one SDL event while the menu is open.  Returns true when the event was
 * consumed by the menu. */
bool hud_handle_event(Hud *h, const SDL_Event *e);

/* Called once per frame to draw the menu and the status overlay. */
void hud_draw(Hud *h, Renderer *r, float fps, bool paused, float speed);

void hud_toast(Hud *h, const char *fmt, ...);

#endif /* HUD_H */

/* hud.c - overlay menu, binding editor and status overlay. */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "hud.h"
#include "nes.h"
#include "state.h"

#define COL_BG_R 0x18
#define COL_BG_G 0x18
#define COL_BG_B 0x28
#define COL_FG_R 0xE8
#define COL_FG_G 0xE8
#define COL_FG_B 0xF0
#define COL_HI_R 0xFF
#define COL_HI_G 0xD0
#define COL_HI_B 0x40
#define COL_DIM_R 0xA0
#define COL_DIM_G 0xA0
#define COL_DIM_B 0xB0

typedef enum {
    MI_ACTION = 0,
    MI_TOGGLE,
    MI_ADJUST,
    MI_OPEN,
    MI_REMAP,
    MI_BACK,
    MI_INFO,
    MI_SEPARATOR
} MenuItemType;

typedef struct MenuItem {
    const char  *label;
    MenuItemType type;
    void        *data;   /* target for TOGGLE/ADJUST */
    int          lo, hi, step;
    const char **value_names;
    int          action; /* action id */
    HudScreen    target; /* submenu */
    int          player; /* for REMAP */
    int          remap_pad;
    int          remap_action;
} MenuItem;

enum {
    ACT_NONE = 0,
    ACT_RESUME,
    ACT_SAVE_STATE,
    ACT_LOAD_STATE,
    ACT_RESET,
    ACT_QUIT
};

/* --------------------------------------------------------------- helpers -- */

static bool toggle_bool(void *p) { return *(bool *)p; }
static void set_bool(void *p, bool v) { *(bool *)p = v; }
static int get_int(void *p) { return *(int *)p; }
static void set_int(void *p, int v) { *(int *)p = v; }

static const char *key_name(SDL_Scancode sc)
{
    if (sc == SDL_SCANCODE_UNKNOWN) return "---";
    return SDL_GetScancodeName(sc);
}

static const char *pad_name(SDL_GamepadButton b)
{
    if (b == SDL_GAMEPAD_BUTTON_INVALID) return "---";
    return SDL_GetGamepadStringForButton(b);
}

static const char *region_label(int r)
{
    if (r < 0 || r > 2) return "?";
    return region_names[r];
}

static const char *volume_label(int v, char *buf, size_t n)
{
    snprintf(buf, n, "%d%%", v);
    return buf;
}

static const char *scale_label(int v, char *buf, size_t n)
{
    snprintf(buf, n, "%dx", v);
    return buf;
}

/* ---------------------------------------------------------------- screens -- */

void hud_toast(Hud *h, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(h->toast, sizeof(h->toast), fmt, ap);
    va_end(ap);
    h->toast_frames = 120;
}

void hud_init(Hud *h, Settings *s, struct Nes *nes)
{
    memset(h, 0, sizeof(*h));
    h->screen = HUD_CLOSED;
    h->settings = s;
    h->nes = nes;
    h->slot = 0;
}

bool hud_is_open(const Hud *h) { return h->screen != HUD_CLOSED; }

void hud_open(Hud *h)
{
    h->screen = HUD_MAIN;
    h->cursor = 0;
    h->remapping = false;
    h->active = true;
}

void hud_close(Hud *h)
{
    h->screen = HUD_CLOSED;
    h->active = false;
    h->remapping = false;
}

/* Consume a "Resume" request.  Closing the menu must never leave the machine
 * stuck paused (the menu used to set the pause flag on open and never clear
 * it), and an explicit Resume has to win even over a pause the user made with
 * the P key before opening the menu. */
bool hud_take_resume(Hud *h)
{
    bool r = h->resume;
    h->resume = false;
    return r;
}

/* ------------------------------------------------------------ item tables -- */

static MenuItem main_items[] = {
    { "Resume",           MI_ACTION, NULL, 0, 0, 0, NULL, ACT_RESUME, HUD_CLOSED, 0, 0, 0 },
    { "Save State",       MI_ACTION, NULL, 0, 0, 0, NULL, ACT_SAVE_STATE, HUD_CLOSED, 0, 0, 0 },
    { "Load State",       MI_ACTION, NULL, 0, 0, 0, NULL, ACT_LOAD_STATE, HUD_CLOSED, 0, 0, 0 },
    { "",                 MI_SEPARATOR, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Controls",         MI_OPEN,   NULL, 0, 0, 0, NULL, 0, HUD_CONTROLS, 0, 0, 0 },
    { "Video",            MI_OPEN,   NULL, 0, 0, 0, NULL, 0, HUD_VIDEO, 0, 0, 0 },
    { "Audio",            MI_OPEN,   NULL, 0, 0, 0, NULL, 0, HUD_AUDIO, 0, 0, 0 },
    { "",                 MI_SEPARATOR, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Save-state slot",  MI_ADJUST, NULL, 0, 9, 1, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Reset",            MI_ACTION, NULL, 0, 0, 0, NULL, ACT_RESET, HUD_CLOSED, 0, 0, 0 },
    { "System info",      MI_OPEN,   NULL, 0, 0, 0, NULL, 0, HUD_INFO, 0, 0, 0 },
    { "Quit",             MI_ACTION, NULL, 0, 0, 0, NULL, ACT_QUIT, HUD_CLOSED, 0, 0, 0 }
};

static MenuItem controls_items[] = {
    { "Player 1 keyboard", MI_OPEN, NULL, 0, 0, 0, NULL, 0, HUD_CONTROLS_P1_KB, 0, 0, 0 },
    { "Player 1 gamepad",  MI_OPEN, NULL, 0, 0, 0, NULL, 0, HUD_CONTROLS_P1_PAD, 0, 0, 0 },
    { "Player 2 keyboard", MI_OPEN, NULL, 0, 0, 0, NULL, 0, HUD_CONTROLS_P2_KB, 0, 0, 0 },
    { "Player 2 gamepad",  MI_OPEN, NULL, 0, 0, 0, NULL, 0, HUD_CONTROLS_P2_PAD, 0, 0, 0 },
    { "",                  MI_SEPARATOR, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Back",              MI_BACK, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 }
};

/* The four binding screens share one table; the player/pad flags come from the
 * screen. */
#define BIND_ITEMS 10
static MenuItem bind_items[BIND_ITEMS];

static MenuItem video_items[] = {
    { "Region",       MI_ADJUST, NULL, 0, 2, 1, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Window scale", MI_ADJUST, NULL, 1, 8, 1, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "VSync",        MI_TOGGLE, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Show FPS",     MI_TOGGLE, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Show overlay", MI_TOGGLE, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "",             MI_SEPARATOR, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Back",         MI_BACK, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 }
};

static MenuItem audio_items[] = {
    { "Enabled", MI_TOGGLE, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Volume",  MI_ADJUST, NULL, 0, 100, 5, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "",        MI_SEPARATOR, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 },
    { "Back",    MI_BACK, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 }
};

static MenuItem info_items[] = {
    { "Back", MI_BACK, NULL, 0, 0, 0, NULL, 0, HUD_CLOSED, 0, 0, 0 }
};

static int item_count(HudScreen s)
{
    switch (s) {
        case HUD_MAIN: return (int)(sizeof(main_items) / sizeof(main_items[0]));
        case HUD_CONTROLS: return (int)(sizeof(controls_items) / sizeof(controls_items[0]));
        case HUD_CONTROLS_P1_KB:
        case HUD_CONTROLS_P1_PAD:
        case HUD_CONTROLS_P2_KB:
        case HUD_CONTROLS_P2_PAD: return BIND_ITEMS;
        case HUD_VIDEO: return (int)(sizeof(video_items) / sizeof(video_items[0]));
        case HUD_AUDIO: return (int)(sizeof(audio_items) / sizeof(audio_items[0]));
        case HUD_INFO: return 1;
        default: return 0;
    }
}

static MenuItem *item_at(Hud *h, int idx)
{
    switch (h->screen) {
        case HUD_MAIN:
            if (idx < 0 || idx >= item_count(HUD_MAIN)) return NULL;
            return &main_items[idx];
        case HUD_CONTROLS:
            if (idx < 0 || idx >= item_count(HUD_CONTROLS)) return NULL;
            return &controls_items[idx];
        case HUD_CONTROLS_P1_KB:
        case HUD_CONTROLS_P1_PAD:
        case HUD_CONTROLS_P2_KB:
        case HUD_CONTROLS_P2_PAD:
            if (idx < 0 || idx >= BIND_ITEMS) return NULL;
            return &bind_items[idx];
        case HUD_VIDEO:
            if (idx < 0 || idx >= item_count(HUD_VIDEO)) return NULL;
            return &video_items[idx];
        case HUD_AUDIO:
            if (idx < 0 || idx >= item_count(HUD_AUDIO)) return NULL;
            return &audio_items[idx];
        case HUD_INFO:
            return &info_items[0];
        default:
            return NULL;
    }
}

/* Bindings shown by the current screen. */
static void current_bind_context(Hud *h, int *player, int *is_pad)
{
    switch (h->screen) {
        case HUD_CONTROLS_P1_PAD: *player = 0; *is_pad = 1; break;
        case HUD_CONTROLS_P2_KB:  *player = 1; *is_pad = 0; break;
        case HUD_CONTROLS_P2_PAD: *player = 1; *is_pad = 1; break;
        default:                  *player = 0; *is_pad = 0; break;
    }
}

static void build_bind_items(Hud *h)
{
    int player, is_pad;
    int i;
    current_bind_context(h, &player, &is_pad);
    (void)player;
    /* The console buttons first, then the emulator hotkeys. */
    for (i = 0; i < BIND_ITEMS; i++) {
        bind_items[i].label = ctl_action_name[i];
        bind_items[i].type = MI_REMAP;
        bind_items[i].data = NULL;
        bind_items[i].action = 0;
        bind_items[i].player = player;
        bind_items[i].remap_pad = is_pad;
        bind_items[i].remap_action = i;
        bind_items[i].target = HUD_CLOSED;
        bind_items[i].value_names = NULL;
        bind_items[i].lo = bind_items[i].hi = bind_items[i].step = 0;
    }
}

static void open_screen(Hud *h, HudScreen s)
{
    h->screen = s;
    h->cursor = 0;
    h->scroll = 0;
    if (s == HUD_CONTROLS_P1_KB || s == HUD_CONTROLS_P1_PAD ||
        s == HUD_CONTROLS_P2_KB || s == HUD_CONTROLS_P2_PAD)
        build_bind_items(h);
}

/* --------------------------------------------------------------- actions -- */

static void perform_action(Hud *h, int action, Hud *self)
{
    (void)self;
    switch (action) {
        case ACT_RESUME: h->resume = true; hud_close(h); break;
        case ACT_SAVE_STATE:
            if (state_save_slot(h->nes, h->slot))
                hud_toast(h, "State saved to slot %d", h->slot);
            else
                hud_toast(h, "Save failed (slot %d)", h->slot);
            break;
        case ACT_LOAD_STATE:
            if (state_load_slot(h->nes, h->slot))
                hud_toast(h, "State loaded from slot %d", h->slot);
            else
                hud_toast(h, "No state in slot %d", h->slot);
            break;
        case ACT_RESET:
            nes_reset(h->nes);
            hud_toast(h, "Console reset");
            break;
        case ACT_QUIT:
            hud_toast(h, "Quit requested");
            break;
        default:
            break;
    }
}

static int *int_field(Hud *h, HudScreen s, int idx)
{
    Settings *st = h->settings;
    if (s == HUD_MAIN && idx == 8) return &h->slot;
    if (s == HUD_VIDEO) {
        if (idx == 0) return &st->region;
        if (idx == 1) return &st->scale;
    }
    if (s == HUD_AUDIO && idx == 1) return &st->volume;
    return NULL;
}

static bool *bool_field(Hud *h, HudScreen s, int idx)
{
    Settings *st = h->settings;
    if (s == HUD_VIDEO) {
        if (idx == 2) return &st->vsync;
        if (idx == 3) return &st->show_fps;
        if (idx == 4) return &st->show_overlay;
    }
    if (s == HUD_AUDIO && idx == 0) return &st->audio_enabled;
    return NULL;
}

/* ------------------------------------------------------------- rendering -- */

static const char *item_value_string(Hud *h, HudScreen s, int idx, char *buf, size_t n)
{
    int *iv = int_field(h, s, idx);
    bool *bv = bool_field(h, s, idx);
    if (s == HUD_MAIN && idx == 8) { snprintf(buf, n, "%d", h->slot); return buf; }
    if (iv) {
        if (s == HUD_VIDEO && idx == 0) { snprintf(buf, n, "%s", region_label(*iv)); return buf; }
        if (s == HUD_VIDEO && idx == 1) { snprintf(buf, n, "%dx", *iv); return buf; }
        if (s == HUD_AUDIO && idx == 1) { snprintf(buf, n, "%d%%", *iv); return buf; }
    }
    if (bv) return *bv ? "On" : "Off";
    /* Remap rows show the current binding. */
    if (s == HUD_CONTROLS_P1_KB || s == HUD_CONTROLS_P1_PAD ||
        s == HUD_CONTROLS_P2_KB || s == HUD_CONTROLS_P2_PAD) {
        int player, is_pad;
        MenuItem *it = item_at(h, idx);
        current_bind_context(h, &player, &is_pad);
        if (!it) return "";
        if (is_pad) {
            snprintf(buf, n, "%s", h->settings->pad[player].has_pad[it->remap_action]
                                    ? pad_name(h->settings->pad[player].pad[it->remap_action])
                                    : "---");
        } else {
            snprintf(buf, n, "%s", h->settings->kb[player].has_key[it->remap_action]
                                    ? key_name(h->settings->kb[player].key[it->remap_action])
                                    : "---");
        }
        return buf;
    }
    return "";
}

static const char *screen_title(HudScreen s)
{
    switch (s) {
        case HUD_MAIN: return "NESPECULATOR";
        case HUD_CONTROLS: return "CONTROLS";
        case HUD_CONTROLS_P1_KB: return "P1 KEYBOARD";
        case HUD_CONTROLS_P1_PAD: return "P1 GAMEPAD";
        case HUD_CONTROLS_P2_KB: return "P2 KEYBOARD";
        case HUD_CONTROLS_P2_PAD: return "P2 GAMEPAD";
        case HUD_VIDEO: return "VIDEO";
        case HUD_AUDIO: return "AUDIO";
        case HUD_INFO: return "SYSTEM INFO";
        default: return "";
    }
}

static void draw_menu(Hud *h, Renderer *r)
{
    int n = item_count(h->screen);
    int i;
    int x = 24, y = 28, w = 208, hgt = 24 + n * 12 + 14;
    char buf[128];

    if (hgt > 232) hgt = 232;

    render_fill_rect(r, x, y, w, hgt, COL_BG_R, COL_BG_G, COL_BG_B, 235);
    render_fill_rect(r, x, y, w, 14, 0x30, 0x30, 0x50, 255);
    render_text(r, x + 6, y + 3, screen_title(h->screen), 1, COL_HI_R, COL_HI_G, COL_HI_B, 255);

    for (i = 0; i < n; i++) {
        MenuItem *it = item_at(h, i);
        int ry = y + 18 + i * 12;
        bool sel = (i == h->cursor);
        if (!it) continue;
        if (it->type == MI_SEPARATOR) continue;
        if (sel)
            render_fill_rect(r, x + 3, ry - 1, w - 6, 11, 0x40, 0x40, 0x70, 255);
        render_text(r, x + 7, ry, it->label, 1,
                    sel ? COL_HI_R : COL_FG_R,
                    sel ? COL_HI_G : COL_FG_G,
                    sel ? COL_HI_B : COL_FG_B, 255);
        {
            const char *v = item_value_string(h, h->screen, i, buf, sizeof(buf));
            if (v && v[0]) {
                int vw = (int)strlen(v) * 7;
                render_text(r, x + w - 8 - vw, ry, v, 1,
                            sel ? COL_HI_R : COL_DIM_R,
                            sel ? COL_HI_G : COL_DIM_G,
                            sel ? COL_HI_B : COL_DIM_B, 255);
            }
        }
    }

    if (h->remapping) {
        render_fill_rect(r, 40, 110, 176, 30, 0x50, 0x20, 0x20, 245);
        render_text(r, 48, 118, "Press a key or pad button", 1, 255, 255, 255, 255);
        render_text(r, 48, 128, "ESC or B to cancel", 1, 220, 200, 200, 255);
    }

    if (h->screen == HUD_INFO) {
        char line[96];
        y = 40;
        snprintf(line, sizeof(line), "Region:  %s", region_label(h->settings->region));
        render_text(r, 10, y, line, 1, 220, 220, 230, 255); y += 12;
        snprintf(line, sizeof(line), "Mapper:  %d %s", h->nes->cart->mapper,
                 cart_mapper_name(h->nes->cart->mapper));
        render_text(r, 10, y, line, 1, 220, 220, 230, 255); y += 12;
        snprintf(line, sizeof(line), "PRG:     %u KB", h->nes->cart->prg_size / 1024);
        render_text(r, 10, y, line, 1, 220, 220, 230, 255); y += 12;
        snprintf(line, sizeof(line), "CHR:     %u KB %s", h->nes->cart->chr_size / 1024,
                 h->nes->cart->chr_ram ? "(RAM)" : "(ROM)");
        render_text(r, 10, y, line, 1, 220, 220, 230, 255); y += 12;
        snprintf(line, sizeof(line), "Mirror:  %d", (int)h->nes->cart->mirroring);
        render_text(r, 10, y, line, 1, 220, 220, 230, 255); y += 12;
        snprintf(line, sizeof(line), "Frames:  %llu", (unsigned long long)h->nes->frame_count);
        render_text(r, 10, y, line, 1, 220, 220, 230, 255); y += 12;
        snprintf(line, sizeof(line), "PC:  $%04X  A:%02X X:%02X Y:%02X S:%02X P:%02X",
                 h->nes->cpu->pc, h->nes->cpu->a, h->nes->cpu->x, h->nes->cpu->y,
                 h->nes->cpu->s, h->nes->cpu->p);
        render_text(r, 10, y, line, 1, 200, 200, 210, 255); y += 12;
    }
}

void hud_draw(Hud *h, Renderer *r, float fps, bool paused, float speed)
{
    char buf[96];

    if (h->toast_frames > 0) {
        int w = (int)strlen(h->toast) * 7 + 12;
        render_fill_rect(r, 128 - w / 2, 200, w, 14, 0x10, 0x10, 0x20, 220);
        render_text(r, 128 - w / 2 + 6, 203, h->toast, 1, 255, 230, 120, 255);
        h->toast_frames--;
    }

    if (h->settings->show_fps) {
        snprintf(buf, sizeof(buf), "%.1f fps", fps);
        render_text(r, 2, 2, buf, 1, 255, 255, 120, 255);
    }

    if (paused || speed != 1.0f) {
        if (paused) render_text(r, 2, 12, "PAUSED", 1, 255, 160, 160, 255);
        else {
            snprintf(buf, sizeof(buf), "SPEED %s", "MAX");
            render_text(r, 2, 12, buf, 1, 160, 255, 160, 255);
        }
    }

    if (h->screen != HUD_CLOSED) {
        /* Dim the game behind the menu. */
        render_fill_rect(r, 0, 0, 256, 240, 0, 0, 0, 120);
        draw_menu(h, r);
    }
}

/* ---------------------------------------------------------------- events -- */

static void remap_current(Hud *h, int player, int is_pad, int action,
                          SDL_Scancode sc, SDL_GamepadButton pb, bool is_key)
{
    if (is_key) {
        if (sc == SDL_SCANCODE_ESCAPE) { h->remapping = false; return; }
        h->settings->kb[player].key[action] = sc;
        h->settings->kb[player].has_key[action] = true;
    } else {
        if (pb == SDL_GAMEPAD_BUTTON_EAST && h->remapping) {
            /* B on the pad cancels the bind rather than binding itself. */
            h->remapping = false;
            return;
        }
        h->settings->pad[player].pad[action] = pb;
        h->settings->pad[player].has_pad[action] = true;
    }
    h->remapping = false;
    settings_save(h->settings);
    hud_toast(h, "Bound %s", ctl_action_name[action]);
}

bool hud_handle_event(Hud *h, const SDL_Event *e)
{
    if (h->screen == HUD_CLOSED && !h->remapping) return false;

    switch (e->type) {
    case SDL_EVENT_KEY_DOWN: {
        SDL_Scancode sc = e->key.scancode;
        if (h->remapping) {
            if (sc == SDL_SCANCODE_ESCAPE) { h->remapping = false; return true; }
            remap_current(h, h->remap_player, h->remap_pad, h->remap_action,
                          sc, SDL_GAMEPAD_BUTTON_INVALID, true);
            return true;
        }
        if (sc == SDL_SCANCODE_ESCAPE) {
            if (h->screen == HUD_MAIN) hud_close(h);
            else open_screen(h, h->screen == HUD_INFO ? HUD_MAIN : HUD_MAIN);
            return true;
        }
        if (sc == SDL_SCANCODE_UP || sc == SDL_SCANCODE_W) {
            do {
                h->cursor--;
                if (h->cursor < 0) h->cursor = item_count(h->screen) - 1;
            } while (item_at(h, h->cursor) && item_at(h, h->cursor)->type == MI_SEPARATOR);
            return true;
        }
        if (sc == SDL_SCANCODE_DOWN || sc == SDL_SCANCODE_S) {
            do {
                h->cursor++;
                if (h->cursor >= item_count(h->screen)) h->cursor = 0;
            } while (item_at(h, h->cursor) && item_at(h, h->cursor)->type == MI_SEPARATOR);
            return true;
        }
        if (sc == SDL_SCANCODE_LEFT || sc == SDL_SCANCODE_A) {
            MenuItem *it = item_at(h, h->cursor);
            if (it) {
                int *iv = int_field(h, h->screen, h->cursor);
                bool *bv = bool_field(h, h->screen, h->cursor);
                if (iv) { *iv -= it->step; if (*iv < it->lo) *iv = it->hi; }
                else if (bv) *bv = !*bv;
            }
            return true;
        }
        if (sc == SDL_SCANCODE_RIGHT || sc == SDL_SCANCODE_D) {
            MenuItem *it = item_at(h, h->cursor);
            if (it) {
                int *iv = int_field(h, h->screen, h->cursor);
                bool *bv = bool_field(h, h->screen, h->cursor);
                if (iv) { *iv += it->step; if (*iv > it->hi) *iv = it->lo; }
                else if (bv) *bv = !*bv;
            }
            return true;
        }
        if (sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_SPACE ||
            sc == SDL_SCANCODE_KP_ENTER) {
            MenuItem *it = item_at(h, h->cursor);
            if (!it) return true;
            switch (it->type) {
            case MI_ACTION:
                perform_action(h, it->action, h);
                if (it->action == ACT_QUIT) {
                    SDL_Event q;
                    q.type = SDL_EVENT_QUIT;
                    SDL_PushEvent(&q);
                }
                break;
            case MI_OPEN:
                open_screen(h, it->target);
                break;
            case MI_BACK:
                open_screen(h, HUD_MAIN);
                break;
            case MI_REMAP:
                h->remapping = true;
                h->remap_player = it->player;
                h->remap_action = it->remap_action;
                h->remap_pad = (it->remap_pad != 0);
                break;
            case MI_TOGGLE: {
                bool *bv = bool_field(h, h->screen, h->cursor);
                if (bv) *bv = !*bv;
                break;
            }
            case MI_ADJUST: {
                int *iv = int_field(h, h->screen, h->cursor);
                if (iv) { *iv += it->step; if (*iv > it->hi) *iv = it->lo; }
                break;
            }
            default:
                break;
            }
            return true;
        }
        if (sc == SDL_SCANCODE_F5) {
            if (state_save_slot(h->nes, h->slot)) hud_toast(h, "Saved slot %d", h->slot);
            return true;
        }
        if (sc == SDL_SCANCODE_F8) {
            if (state_load_slot(h->nes, h->slot)) hud_toast(h, "Loaded slot %d", h->slot);
            else hud_toast(h, "Slot %d empty", h->slot);
            return true;
        }
        return true;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
        SDL_GamepadButton b = e->gbutton.button;
        if (h->remapping && h->remap_pad) {
            remap_current(h, h->remap_player, 1, h->remap_action,
                          SDL_SCANCODE_UNKNOWN, b, false);
            return true;
        }
        if (!h->remapping && h->remap_pad == 0 && h->screen != HUD_CLOSED) {
            /* Pad navigation: south = confirm, east = back. */
            if (b == SDL_GAMEPAD_BUTTON_SOUTH) {
                MenuItem *it = item_at(h, h->cursor);
                if (it) {
                    if (it->type == MI_ACTION) perform_action(h, it->action, h);
                    else if (it->type == MI_OPEN) open_screen(h, it->target);
                    else if (it->type == MI_BACK) open_screen(h, HUD_MAIN);
                    else if (it->type == MI_REMAP) {
                        h->remapping = true;
                        h->remap_player = it->player;
                        h->remap_action = it->remap_action;
                        h->remap_pad = (it->remap_pad != 0);
                    } else if (it->type == MI_TOGGLE) {
                        bool *bv = bool_field(h, h->screen, h->cursor);
                        if (bv) *bv = !*bv;
                    } else if (it->type == MI_ADJUST) {
                        int *iv = int_field(h, h->screen, h->cursor);
                        if (iv) { *iv += it->step; if (*iv > it->hi) *iv = it->lo; }
                    }
                }
                return true;
            }
            if (b == SDL_GAMEPAD_BUTTON_EAST) {
                if (h->screen == HUD_MAIN) hud_close(h);
                else open_screen(h, HUD_MAIN);
                return true;
            }
            if (b == SDL_GAMEPAD_BUTTON_DPAD_UP) {
                do {
                    h->cursor--;
                    if (h->cursor < 0) h->cursor = item_count(h->screen) - 1;
                } while (item_at(h, h->cursor) && item_at(h, h->cursor)->type == MI_SEPARATOR);
                return true;
            }
            if (b == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
                do {
                    h->cursor++;
                    if (h->cursor >= item_count(h->screen)) h->cursor = 0;
                } while (item_at(h, h->cursor) && item_at(h, h->cursor)->type == MI_SEPARATOR);
                return true;
            }
        }
        return true;
    }

    default:
        break;
    }
    return false;
}

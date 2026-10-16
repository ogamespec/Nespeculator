/* input_map.c - default bindings, settings and persistence (INI-style text).
 *
 * The file lives next to the save states:
 *   Windows: %APPDATA%\Nespeculator\nespeculator.cfg
 *   other  : $HOME/.nespeculator/nespeculator.cfg
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input_map.h"

const char *ctl_action_name[CTL_COUNT] = {
    "A", "B", "Select", "Start",
    "Up", "Down", "Left", "Right",
    "SaveState", "LoadState", "Menu", "Reset",
    "Pause", "FastForward", "FrameAdvance", "Quit"
};

const char *region_names[3] = { "NTSC", "PAL", "Dendy" };

static void defaults_for(Bindings *b, int player)
{
    int i;
    memset(b, 0, sizeof(*b));
    for (i = 0; i < CTL_COUNT; i++) {
        b->key[i] = SDL_SCANCODE_UNKNOWN;
        b->has_key[i] = false;
        b->pad[i] = SDL_GAMEPAD_BUTTON_INVALID;
        b->has_pad[i] = false;
    }
    if (player == 0) {
        b->key[CTL_UP]     = SDL_SCANCODE_UP;      b->has_key[CTL_UP] = true;
        b->key[CTL_DOWN]   = SDL_SCANCODE_DOWN;    b->has_key[CTL_DOWN] = true;
        b->key[CTL_LEFT]   = SDL_SCANCODE_LEFT;    b->has_key[CTL_LEFT] = true;
        b->key[CTL_RIGHT]  = SDL_SCANCODE_RIGHT;   b->has_key[CTL_RIGHT] = true;
        b->key[CTL_A]      = SDL_SCANCODE_X;       b->has_key[CTL_A] = true;
        b->key[CTL_B]      = SDL_SCANCODE_Z;       b->has_key[CTL_B] = true;
        b->key[CTL_SELECT] = SDL_SCANCODE_RSHIFT;  b->has_key[CTL_SELECT] = true;
        b->key[CTL_START]  = SDL_SCANCODE_RETURN;  b->has_key[CTL_START] = true;
    } else {
        b->key[CTL_UP]     = SDL_SCANCODE_I;       b->has_key[CTL_UP] = true;
        b->key[CTL_DOWN]   = SDL_SCANCODE_K;       b->has_key[CTL_DOWN] = true;
        b->key[CTL_LEFT]   = SDL_SCANCODE_J;       b->has_key[CTL_LEFT] = true;
        b->key[CTL_RIGHT]  = SDL_SCANCODE_L;       b->has_key[CTL_RIGHT] = true;
        b->key[CTL_A]      = SDL_SCANCODE_G;       b->has_key[CTL_A] = true;
        b->key[CTL_B]      = SDL_SCANCODE_F;       b->has_key[CTL_B] = true;
        b->key[CTL_SELECT] = SDL_SCANCODE_T;       b->has_key[CTL_SELECT] = true;
        b->key[CTL_START]  = SDL_SCANCODE_Y;       b->has_key[CTL_START] = true;
    }
    b->key[CTL_SAVE_STATE]    = SDL_SCANCODE_F5;         b->has_key[CTL_SAVE_STATE] = true;
    b->key[CTL_LOAD_STATE]    = SDL_SCANCODE_F8;         b->has_key[CTL_LOAD_STATE] = true;
    b->key[CTL_MENU]          = SDL_SCANCODE_ESCAPE;     b->has_key[CTL_MENU] = true;
    b->key[CTL_RESET]         = SDL_SCANCODE_F2;         b->has_key[CTL_RESET] = true;
    b->key[CTL_PAUSE]         = SDL_SCANCODE_P;          b->has_key[CTL_PAUSE] = true;
    b->key[CTL_FAST_FORWARD]  = SDL_SCANCODE_TAB;        b->has_key[CTL_FAST_FORWARD] = true;
    b->key[CTL_FRAME_ADVANCE] = SDL_SCANCODE_BACKSPACE;  b->has_key[CTL_FRAME_ADVANCE] = true;
    b->key[CTL_QUIT]          = SDL_SCANCODE_F10;        b->has_key[CTL_QUIT] = true;

    b->pad[CTL_A]      = SDL_GAMEPAD_BUTTON_SOUTH; b->has_pad[CTL_A] = true;
    b->pad[CTL_B]      = SDL_GAMEPAD_BUTTON_EAST;  b->has_pad[CTL_B] = true;
    b->pad[CTL_SELECT] = SDL_GAMEPAD_BUTTON_BACK;  b->has_pad[CTL_SELECT] = true;
    b->pad[CTL_START]  = SDL_GAMEPAD_BUTTON_START; b->has_pad[CTL_START] = true;
    b->pad[CTL_UP]     = SDL_GAMEPAD_BUTTON_DPAD_UP;    b->has_pad[CTL_UP] = true;
    b->pad[CTL_DOWN]   = SDL_GAMEPAD_BUTTON_DPAD_DOWN;  b->has_pad[CTL_DOWN] = true;
    b->pad[CTL_LEFT]   = SDL_GAMEPAD_BUTTON_DPAD_LEFT;  b->has_pad[CTL_LEFT] = true;
    b->pad[CTL_RIGHT]  = SDL_GAMEPAD_BUTTON_DPAD_RIGHT; b->has_pad[CTL_RIGHT] = true;
}

void settings_defaults(Settings *s)
{
    memset(s, 0, sizeof(*s));
    s->region = 0;
    s->scale = 3;
    s->vsync = true;
    s->audio_enabled = true;
    s->volume = 70;
    s->show_fps = false;
    s->show_overlay = true;
    defaults_for(&s->kb[0], 0);
    defaults_for(&s->kb[1], 1);
    /* Player 2's pad defaults: only the buttons, on the second gamepad. */
    defaults_for(&s->pad[0], 0);
    defaults_for(&s->pad[1], 1);
}

static void config_dir(char *buf, size_t n)
{
#ifdef _WIN32
    const char *base = getenv("APPDATA");
    if (!base) base = ".";
    snprintf(buf, n, "%s\\Nespeculator", base);
#else
    const char *base = getenv("HOME");
    if (!base) base = ".";
    snprintf(buf, n, "%s/.nespeculator", base);
#endif
}

static void config_path(char *buf, size_t n)
{
    char dir[400];
    config_dir(dir, sizeof(dir));
#ifdef _WIN32
    snprintf(buf, n, "%s\\nespeculator.cfg", dir);
#else
    snprintf(buf, n, "%s/nespeculator.cfg", dir);
#endif
}

static void ensure_dir(void)
{
    char dir[400];
    config_dir(dir, sizeof(dir));
#ifdef _WIN32
    {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "if not exist \"%s\" mkdir \"%s\"", dir, dir);
        system(cmd);
    }
#else
    {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", dir);
        if (system(cmd) != 0) { /* best effort */ }
    }
#endif
}

bool settings_load(Settings *s)
{
    char path[512];
    char line[256];
    FILE *f;
    settings_defaults(s);
    config_path(path, sizeof(path));
    f = fopen(path, "r");
    if (!f) return false;
    while (fgets(line, sizeof(line), f)) {
        char key[128];
        int value;
        char *eq;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        value = atoi(eq + 1);
        strncpy(key, line, sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
        {
            char *nl = strchr(key, '\n');
            if (nl) *nl = '\0';
        }

        if (strcmp(key, "video.region") == 0) s->region = value;
        else if (strcmp(key, "video.scale") == 0) s->scale = value;
        else if (strcmp(key, "video.vsync") == 0) s->vsync = value != 0;
        else if (strcmp(key, "audio.enabled") == 0) s->audio_enabled = value != 0;
        else if (strcmp(key, "audio.volume") == 0) s->volume = value;
        else if (strcmp(key, "video.show_fps") == 0) s->show_fps = value != 0;
        else if (strcmp(key, "video.show_overlay") == 0) s->show_overlay = value != 0;
        else {
            int player = -1, idx;
            char kind[8];
            char action[64];
            if (sscanf(key, "input.p%d.%7[^.].%63s", &player, kind, action) == 3) {
                for (idx = 0; idx < CTL_COUNT; idx++)
                    if (strcmp(action, ctl_action_name[idx]) == 0) break;
                if (idx < CTL_COUNT && player >= 0 && player < 2) {
                    if (strcmp(kind, "pad") == 0) {
                        if (value < 0) {
                            s->pad[player].has_pad[idx] = false;
                        } else {
                            s->pad[player].pad[idx] = (SDL_GamepadButton)value;
                            s->pad[player].has_pad[idx] = true;
                        }
                    } else {
                        if (value < 0) {
                            s->kb[player].has_key[idx] = false;
                        } else {
                            s->kb[player].key[idx] = (SDL_Scancode)value;
                            s->kb[player].has_key[idx] = true;
                        }
                    }
                }
            }
        }
    }
    fclose(f);
    if (s->scale < 1 || s->scale > 8) s->scale = 3;
    if (s->region < 0 || s->region > 2) s->region = 0;
    if (s->volume < 0) s->volume = 0;
    if (s->volume > 100) s->volume = 100;
    return true;
}

bool settings_save(const Settings *s)
{
    char path[512];
    FILE *f;
    int player, i;
    ensure_dir();
    config_path(path, sizeof(path));
    f = fopen(path, "w");
    if (!f) return false;
    fprintf(f, "# Nespeculator configuration\n");
    fprintf(f, "video.region=%d\n", s->region);
    fprintf(f, "video.scale=%d\n", s->scale);
    fprintf(f, "video.vsync=%d\n", s->vsync ? 1 : 0);
    fprintf(f, "video.show_fps=%d\n", s->show_fps ? 1 : 0);
    fprintf(f, "video.show_overlay=%d\n", s->show_overlay ? 1 : 0);
    fprintf(f, "audio.enabled=%d\n", s->audio_enabled ? 1 : 0);
    fprintf(f, "audio.volume=%d\n", s->volume);
    for (player = 0; player < 2; player++) {
        for (i = 0; i < CTL_COUNT; i++) {
            fprintf(f, "input.p%d.key.%s=%d\n", player, ctl_action_name[i],
                    s->kb[player].has_key[i] ? (int)s->kb[player].key[i] : -1);
            fprintf(f, "input.p%d.pad.%s=%d\n", player, ctl_action_name[i],
                    s->pad[player].has_pad[i] ? (int)s->pad[player].pad[i] : -1);
        }
    }
    fclose(f);
    return true;
}

/* main.c - Nespeculator frontend: SDL3 video/audio, input, HUD, save states.
 *
 * Build: Visual Studio 2026 (MSVC 18) via CMake, or any C11 compiler with SDL3.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "cart.h"
#include "state.h"
#include "render.h"
#include "hud.h"
#include "input_map.h"

#define AUDIO_SAMPLE_RATE 48000

typedef struct App {
    Nes        *nes;
    Renderer    render;
    Settings    settings;
    Hud         hud;

    SDL_AudioStream *audio;
    /* Raw PCM dump (NES_WAV=<path>): 16-bit mono at AUDIO_SAMPLE_RATE.  Aided
     * facility for headless verification. */
    FILE       *wav;
    uint32_t    wav_bytes;

    SDL_Gamepad *pads[2];

    bool        running;
    bool        headless;      /* --headless: no window, no video driver */
    bool        paused;
    bool        fast_forward;
    bool        frame_advance;
    uint64_t    frame_counter;
    uint64_t    fps_timer;
    uint32_t    fps_frames;
    float       fps;

    /* Cartridge state: the emulator can also start with no ROM and wait for a
     * file to be dropped onto its window. */
    bool        has_rom;
    char        rom_path[512];
    char        rom_error[200];

    /* Command-line options used for automated verification. */
    const char *screenshot_path;
    int         screenshot_after;
    int         run_frames;
} App;

/* -------------------------------------------------------------- audio ----- */

static void audio_init(App *a)
{
    SDL_AudioSpec spec;
    /* Headless runs must not open an output stream: the dummy driver still
     * creates a (silent) WASAPI stream on Windows, which adds latency to
     * shutdown and can keep the process alive for a moment after the run. */
    if (a->headless)
        return;
    memset(&spec, 0, sizeof(spec));
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1;
    spec.freq = AUDIO_SAMPLE_RATE;
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("SDL_InitSubSystem(AUDIO) failed: %s", SDL_GetError());
        return;
    }
    /* A short device buffer keeps the emulator's audio latency (and the time
     * SDL_CloseAudioDevice spends stopping the WASAPI stream) small. */
    SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, "512");
    a->audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
                                         NULL, NULL);
    if (!a->audio) {
        SDL_Log("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
        return;
    }
    SDL_SetAudioStreamGain(a->audio, (float)a->settings.volume / 100.0f);
    SDL_ResumeAudioStreamDevice(a->audio);
}

static void audio_shutdown(App *a)
{
    if (a->audio) {
        /* Drop whatever is queued and stop the device before destroying the
         * stream: the queue can hold a lot of audio after a fast-forward
         * burst, and the WASAPI backend otherwise spends time in teardown. */
        SDL_PauseAudioStreamDevice(a->audio);
        SDL_ClearAudioStream(a->audio);
        SDL_DestroyAudioStream(a->audio);
        a->audio = NULL;
    }
    if (a->wav) {
        /* Fix up the RIFF/data sizes now that the length is known. */
        uint32_t riff = 36u + a->wav_bytes;
        fseek(a->wav, 4, SEEK_SET);
        fwrite(&riff, 4, 1, a->wav);
        fseek(a->wav, 40, SEEK_SET);
        fwrite(&a->wav_bytes, 4, 1, a->wav);
        fclose(a->wav);
        a->wav = NULL;
    }
}

/* Open the raw PCM dump requested through NES_WAV.  The header is written with
 * placeholder sizes and patched in audio_shutdown(). */
static void audio_dump_open(App *a)
{
    const char *path = SDL_getenv("NES_WAV");
    uint8_t hdr[44];
    uint32_t rate = AUDIO_SAMPLE_RATE;
    uint32_t byte_rate = AUDIO_SAMPLE_RATE * 2u;   /* mono 16-bit */
    uint32_t zero = 0;
    if (!path || !*path) return;
    a->wav = fopen(path, "wb");
    if (!a->wav) {
        fprintf(stderr, "NES_WAV: cannot open '%s'\n", path);
        return;
    }
    memset(hdr, 0, sizeof(hdr));
    memcpy(hdr +  0, "RIFF", 4);
    memcpy(hdr +  4, &zero, 4);            /* patched at close */
    memcpy(hdr +  8, "WAVE", 4);
    memcpy(hdr + 12, "fmt ", 4);
    hdr[16] = 16;                          /* fmt chunk size */
    hdr[20] = 1;                           /* PCM */
    hdr[22] = 1;                           /* channels */
    memcpy(hdr + 24, &rate, 4);
    memcpy(hdr + 28, &byte_rate, 4);
    hdr[32] = 2;                           /* block align */
    hdr[34] = 16;                          /* bits per sample */
    memcpy(hdr + 36, "data", 4);
    memcpy(hdr + 40, &zero, 4);            /* patched at close */
    fwrite(hdr, 1, sizeof(hdr), a->wav);
    a->wav_bytes = 0;
    fprintf(stderr, "NES_WAV: writing %u Hz 16-bit mono PCM to %s\n",
            rate, path);
}

/* The APU mixes every CPU cycle and decimates to AUDIO_SAMPLE_RATE (apu.c);
 * everything this frontend does is drain that queue to the SDL stream and/or
 * the raw dump.  Called after every emulated frame. */
static void audio_pump(App *a)
{
    float buf[1024];
    int n, i;
    bool to_sdl = a->audio && a->settings.audio_enabled;
    if (!to_sdl && !a->wav) return;
    while ((n = apu_audio_read(a->nes->apu, buf, (int)(sizeof(buf) / sizeof(buf[0])))) > 0) {
        if (to_sdl &&
            !SDL_PutAudioStreamData(a->audio, buf, n * (int)sizeof(float))) {
            /* Warn once: a persistent SDL failure would otherwise be silent. */
            static bool warned = false;
            if (!warned) {
                fprintf(stderr, "audio: SDL_PutAudioStreamData failed: %s\n",
                        SDL_GetError());
                warned = true;
            }
        }
        if (a->wav) {
            int16_t pcm[1024];
            for (i = 0; i < n; i++) {
                float v = buf[i];
                if (v > 1.0f) v = 1.0f;
                if (v < -1.0f) v = -1.0f;
                pcm[i] = (int16_t)(v * 32767.0f);
            }
            fwrite(pcm, sizeof(int16_t), (size_t)n, a->wav);
            a->wav_bytes += (uint32_t)n * 2u;
        }
    }
}

/* Turn on APU sample capture whenever someone is going to consume it.  Must be
 * called after nes_power_on(), which re-initialises the APU. */
static void audio_enable_capture(App *a)
{
    bool to_sdl = a->audio && a->settings.audio_enabled;
    apu_audio_enable(a->nes->apu,
                     (to_sdl || a->wav) ? (double)AUDIO_SAMPLE_RATE : 0.0);
}

/* --------------------------------------------------------------- input ---- */

static bool action_pressed(App *a, int player, int action)
{
    const Bindings *kb = &a->settings.kb[player];
    const Bindings *pd = &a->settings.pad[player];
    if (kb->has_key[action] && kb->key[action] != SDL_SCANCODE_UNKNOWN) {
        const bool *ks = SDL_GetKeyboardState(NULL);
        if (ks[kb->key[action]]) return true;
    }
    if (pd->has_pad[action] && pd->pad[action] != SDL_GAMEPAD_BUTTON_INVALID) {
        SDL_Gamepad *g = a->pads[player];
        if (g && SDL_GetGamepadButton(g, pd->pad[action])) return true;
    }
    return false;
}

static void update_nes_input(App *a)
{
    int player, btn;
    for (player = 0; player < 2; player++) {
        static const int map[8] = { CTL_A, CTL_B, CTL_SELECT, CTL_START,
                                    CTL_UP, CTL_DOWN, CTL_LEFT, CTL_RIGHT };
        for (btn = 0; btn < 8; btn++)
            input_set_button(&a->nes->input, player, btn,
                             action_pressed(a, player, map[btn]));
    }
}

static void handle_hotkeys(App *a)
{
    if (action_pressed(a, 0, CTL_MENU)) {
        if (!hud_is_open(&a->hud)) {
            hud_open(&a->hud);
            /* The menu key is consumed by opening the menu.  Opening the menu
             * must NOT touch `paused`: the frame gate below already stops the
             * machine while the HUD is up, and the "PAUSED" overlay is derived
             * from `paused || hud_is_open`.  Leaving `paused` alone is what
             * lets Resume/ESC actually resume instead of stranding the
             * emulator in a paused state. */
        }
    }
}

/* ---------------------------------------------------------------- setup --- */

static void app_shutdown(App *a)
{
    int i;
    for (i = 0; i < 2; i++) {
        if (a->pads[i]) SDL_CloseGamepad(a->pads[i]);
        a->pads[i] = NULL;
    }
    audio_shutdown(a);
    render_shutdown(&a->render);
    nes_destroy(a->nes);
    a->nes = NULL;
}

static void rom_load_failed(App *a, const char *path, int rc);

/* Load (or replace) the cartridge.  Used both at startup and when a ROM is
 * dropped onto the window. */
static bool app_load_rom(App *a, const char *path)
{
    int rc;
    cart_free(a->nes->cart);
    rc = cart_load(a->nes->cart, a->nes, path);
    if (rc != CART_OK) {
        rom_load_failed(a, path, rc);
        return false;
    }
    snprintf(a->rom_path, sizeof(a->rom_path), "%s", path);
    a->rom_error[0] = '\0';
    a->has_rom = true;
    nes_power_on(a->nes);
    a->frame_counter = 0;
    audio_enable_capture(a);
    hud_toast(&a->hud, "Loaded %s", path);
    printf("Loaded %s: mapper %d (%s), PRG %u KB, CHR %u KB %s, mirroring %d\n",
           path, a->nes->cart->mapper, cart_mapper_name(a->nes->cart->mapper),
           a->nes->cart->prg_size / 1024, a->nes->cart->chr_size / 1024,
           a->nes->cart->chr_ram ? "RAM" : "ROM", (int)a->nes->cart->mirroring);
    return true;
}

static void rom_load_failed(App *a, const char *path, int rc)
{
    const char *msg = "unknown error";
    switch (rc) {
    case CART_ERR_OPEN: msg = "cannot open the ROM file"; break;
    case CART_ERR_FORMAT: msg = "not a valid iNES/NES 2.0 image"; break;
    case CART_ERR_UNSUPPORTED:
        msg = "mapper not supported by this emulator";
        break;
    default: break;
    }
    /* Keep it short enough for the hint screen. */
    {
        const char *base = path;
        const char *slash = strrchr(path, '\\');
        const char *fwd = strrchr(path, '/');
        if (fwd && (!slash || fwd > slash)) slash = fwd;
        if (slash && slash[1]) base = slash + 1;
        snprintf(a->rom_error, sizeof(a->rom_error), "%s: %s", base, msg);
    }
    SDL_Log("Failed to load '%s': %s (mapper %d)", path, msg, a->nes->cart->mapper);
    fprintf(stderr, "Nespeculator: cannot load '%s': %s\n", path, msg);
}

static bool app_init(App *a, const char *rom_path)
{
    int i;
    memset(a, 0, sizeof(*a));
    a->running = true;
    a->fps = 60.0f;

    settings_load(&a->settings);

    /* --headless (and NES_HEADLESS=1) must decide the video driver before
     * SDL_Init: the dummy driver creates no window at all, so automation runs
     * never flash a GUI on the user's desktop. */
    if (SDL_getenv("NES_HEADLESS") && *SDL_getenv("NES_HEADLESS"))
        a->headless = true;
    if (a->headless) {
#ifdef _WIN32
        _putenv_s("SDL_VIDEODRIVER", "dummy");
#else
        setenv("SDL_VIDEODRIVER", "dummy", 1);
#endif
    }
    if (!SDL_Init(0)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    a->nes = nes_create((NesRegion)a->settings.region);
    if (!a->nes) return false;

    if (!a->headless) {
        if (!render_init(&a->render, "Nespeculator", a->settings.scale)) {
            return false;
        }
        SDL_SetRenderVSync(a->render.renderer, a->settings.vsync ? 1 : 0);
    }

    audio_init(a);
    audio_dump_open(a);

    for (i = 0; i < 2; i++) a->pads[i] = NULL;
    {
        int count = 0;
        SDL_JoystickID *ids = SDL_GetGamepads(&count);
        if (ids) {
            int slot = 0;
            for (i = 0; i < count && slot < 2; i++) {
                a->pads[slot] = SDL_OpenGamepad(ids[i]);
                if (a->pads[slot]) slot++;
            }
            SDL_free(ids);
        }
    }

    hud_init(&a->hud, &a->settings, a->nes);

    a->fps_timer = SDL_GetTicks();

    if (rom_path) {
        /* An explicit path on the command line: a failure is fatal so scripts
         * and the test harness see a non-zero exit instead of an empty window. */
        if (!app_load_rom(a, rom_path))
            return false;
    } else {
        /* No ROM: open the window and wait for the user to drop one in. */
        memset(a->nes->ppu->framebuffer, 0, sizeof(a->nes->ppu->framebuffer));
        a->has_rom = false;
    }
    return true;
}

/* --------------------------------------------------------------- events --- */

static void apply_region(App *a)
{
    nes_set_region(a->nes, (NesRegion)a->settings.region);
    /* nes_set_region() resets the APU: recompute the decimation ratio for the
     * new CPU clock. */
    audio_enable_capture(a);
}

static void handle_events(App *a)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            /* Shut the window down here rather than after the loop: the audio
             * device teardown below can take a moment, and the user must not
             * watch the window linger while that happens. */
            if (!a->headless)
                render_shutdown(&a->render);
            a->running = false;
            continue;
        }
        if (e.type == SDL_EVENT_DROP_FILE) {
            if (e.drop.data && *e.drop.data) {
                if (app_load_rom(a, e.drop.data))
                    hud_toast(&a->hud, "ROM loaded");
                else
                    hud_toast(&a->hud, "Could not load the dropped file");
            }
            continue;
        }
        if (e.type == SDL_EVENT_GAMEPAD_ADDED) {
            int i;
            for (i = 0; i < 2; i++) {
                if (!a->pads[i]) {
                    a->pads[i] = SDL_OpenGamepad(e.gdevice.which);
                    break;
                }
            }
            continue;
        }
        if (e.type == SDL_EVENT_GAMEPAD_REMOVED) {
            int i;
            for (i = 0; i < 2; i++) {
                if (a->pads[i] && SDL_GetGamepadID(a->pads[i]) == e.gdevice.which) {
                    SDL_CloseGamepad(a->pads[i]);
                    a->pads[i] = NULL;
                }
            }
            continue;
        }
        if (e.type == SDL_EVENT_KEY_DOWN && !hud_is_open(&a->hud)) {
            if (e.key.scancode == SDL_SCANCODE_P) { a->paused = !a->paused; continue; }
            if (e.key.scancode == SDL_SCANCODE_F2) { nes_reset(a->nes); continue; }
            if (e.key.scancode == SDL_SCANCODE_F5) {
                if (state_save_slot(a->nes, a->hud.slot))
                    hud_toast(&a->hud, "State saved (slot %d)", a->hud.slot);
                continue;
            }
            if (e.key.scancode == SDL_SCANCODE_F8) {
                if (state_load_slot(a->nes, a->hud.slot))
                    hud_toast(&a->hud, "State loaded (slot %d)", a->hud.slot);
                else
                    hud_toast(&a->hud, "Slot %d is empty", a->hud.slot);
                continue;
            }
            if (e.key.scancode == SDL_SCANCODE_F10) { a->running = false; continue; }
            if (e.key.scancode == SDL_SCANCODE_BACKSPACE) { a->frame_advance = true; continue; }
        }
        if (hud_handle_event(&a->hud, &e))
            continue;
    }
}

/* --------------------------------------------------------------- hint ----- */

static void hint_text(Renderer *r, int y, const char *text, int scale,
                      uint8_t cr, uint8_t cg, uint8_t cb)
{
    int w = render_text_width(text, scale);
    int x = (NES_SCREEN_W - w) / 2;
    if (x < 0) x = 0;
    render_text(r, x, y, text, scale, cr, cg, cb, 255);
}

/* Shown while no cartridge is loaded: explains how to get one in. */
static void draw_rom_hint(Renderer *r, const char *error)
{
    hint_text(r,  54, "Nespeculator", 2, 120, 190, 255);
    hint_text(r, 100, "Drop a .nes ROM file", 1, 235, 235, 235);
    hint_text(r, 114, "onto this window", 1, 235, 235, 235);
    hint_text(r, 146, "or start it as", 1, 150, 150, 150);
    hint_text(r, 160, "Nespeculator <rom.nes>", 1, 130, 190, 255);
    if (error && *error) {
        /* The logical screen is 256 px wide: 36 characters at scale 1 fit. */
        char line[40];
        snprintf(line, sizeof(line), "%.36s", error);
        hint_text(r, 192, line, 1, 255, 120, 120);
    }
}

/* ---------------------------------------------------------------- frame --- */

/* Optional per-frame PPU state dump (--palette FIRST [STEP] [LAST]).  Written to
 * stderr so a caller can diff the palette the game programs frame by frame
 * without guessing which frame holds which screen. */
static int pal_first = -1, pal_step = 1, pal_last = -1;

/* --ppudump DIR FRAME: write the whole PPU state at FRAME to DIR. */
static const char *ppudump_dir = NULL;
static int ppudump_frame = -1;

static void palette_probe(App *a)
{
    const Ppu *p = a->nes->ppu;
    int i;
    if (pal_first < 0) return;
    if ((long long)a->frame_counter < pal_first) return;
    if ((long long)a->frame_counter > pal_last) return;
    if (((long long)a->frame_counter - pal_first) % (pal_step > 0 ? pal_step : 1)) return;
    fprintf(stderr, "PAL %llu ctrl=%02X mask=%02X v=%04X t=%04X pal=",
            (unsigned long long)a->frame_counter, p->ctrl, p->mask, p->v, p->t);
    for (i = 0; i < 32; i++) fprintf(stderr, "%02X", p->pal[i]);
    fprintf(stderr, "\n");
    fflush(stderr);
}

static void run_one_frame(App *a)
{
    nes_run_frame(a->nes);
    audio_pump(a);
    a->frame_counter++;
    a->fps_frames++;
    palette_probe(a);
}

int main(int argc, char **argv)
{
    App app;
    const char *rom = NULL;
    int i;
    bool cli_mode = false;     /* an automation flag needs a ROM */
    bool headless = false;     /* --headless: never create a window */
    int screenshot_frame = -1;
    const char *screenshot_file = NULL;
    int trace_every = 0;
    int trace_ppu = 0;
    int trace_max_frames = 8;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Nespeculator - NES/Famicom emulator (spec-driven build)\n"
                   "Usage: Nespeculator <rom.nes> [options]\n"
                   "  --screenshot <file>   save a BMP after N frames and exit\n"
                   "  --screenshot-frame N  frame at which to save (default 600)\n"
                   "  --frames N            run N frames then exit\n"
                   "  --headless            run with no window/video driver\n"
                   "  --region ntsc|pal|dendy\n"
                   "  --scale N             window scale\n"
                   "  --keys                print the current bindings and exit\n");
            return 0;
        } else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            screenshot_file = argv[++i];
        } else if (strcmp(argv[i], "--screenshot-frame") == 0 && i + 1 < argc) {
            screenshot_frame = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--region") == 0 && i + 1 < argc) {
            i++;
            /* applied after settings load */
        } else if (strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
            i++;
        } else if (cli_mode = true, strcmp(argv[i], "--ramhex") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') i++;
            if (i + 1 < argc && argv[i + 1][0] != '-') i++;
        } else if (cli_mode = true, strcmp(argv[i], "--rombytes") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') i++;
            if (i + 1 < argc && argv[i + 1][0] != '-') i++;
        } else if (cli_mode = true, strcmp(argv[i], "--dumpstate") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') i++;   /* skip the frame count */
        } else if (cli_mode = true, strcmp(argv[i], "--dumpeach") == 0) {
            trace_every = -996;
        } else if (cli_mode = true, strcmp(argv[i], "--step") == 0) {
            trace_every = -997;
        } else if (cli_mode = true, strcmp(argv[i], "--ramtrace") == 0) {
            trace_every = -998;
        } else if (cli_mode = true, strcmp(argv[i], "--crashtrace") == 0) {
            trace_every = -999;
        } else if (cli_mode = true, strcmp(argv[i], "--trace") == 0 && i + 1 < argc) {
            trace_every = atoi(argv[++i]);
        } else if (cli_mode = true, strcmp(argv[i], "--traceppu") == 0) {
            trace_ppu = 1;
        } else if (strcmp(argv[i], "--headless") == 0) {
            headless = true;
        } else if (cli_mode = true, strcmp(argv[i], "--ppudump") == 0 && i + 2 < argc) {
            ppudump_dir = argv[++i];
            ppudump_frame = atoi(argv[++i]);
        } else if (cli_mode = true, strcmp(argv[i], "--palette") == 0) {
            pal_first = (i + 1 < argc) ? atoi(argv[++i]) : 0;
            pal_step = (i + 1 < argc && argv[i + 1][0] != '-') ? atoi(argv[++i]) : 1;
            pal_last = (i + 1 < argc && argv[i + 1][0] != '-') ? atoi(argv[++i]) : pal_first;
            if (pal_step < 1) pal_step = 1;
            if (pal_last < pal_first) pal_last = pal_first;
        } else if (strcmp(argv[i], "--keys") == 0) {
            Settings s;
            int p, k;
            settings_load(&s);
            for (p = 0; p < 2; p++) {
                printf("Player %d keyboard:\n", p + 1);
                for (k = 0; k < CTL_COUNT; k++)
                    printf("  %-14s %s\n", ctl_action_name[k],
                           s.kb[p].has_key[k] ? SDL_GetScancodeName(s.kb[p].key[k]) : "---");
            }
            return 0;
        } else if (argv[i][0] != '-') {
            rom = argv[i];
        }
    }

    if (!rom) {
        /* Also accept the ROM through the environment: Windows batch files
         * mishandle arguments that contain parentheses and spaces. */
        const char *envrom = SDL_getenv("NES_ROM");
        if (envrom && *envrom) rom = envrom;
    }
    if (!rom && (cli_mode || SDL_getenv("NES_AUTOINPUT") || SDL_getenv("NES_BUSLOG"))) {
        /* Automation always needs a ROM; only the plain interactive start may
         * come up empty and wait for a file to be dropped onto the window. */
        fprintf(stderr, "Usage: Nespeculator <rom.nes> [--screenshot out.bmp]\n"
                        "       set NES_ROM=<rom.nes> and run without arguments\n");
        return 2;
    }

    app.headless = headless;
    if (!app_init(&app, rom)) {
        app_shutdown(&app);
        return 1;
    }

    /* Re-apply command-line overrides after the settings were loaded. */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--region") == 0 && i + 1 < argc) {
            const char *r = argv[++i];
            if (strcmp(r, "pal") == 0) app.settings.region = 1;
            else if (strcmp(r, "dendy") == 0) app.settings.region = 2;
            else app.settings.region = 0;
            apply_region(&app);
        } else if (strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
            app.settings.scale = atoi(argv[++i]);
            render_set_scale(&app.render, app.settings.scale);
        }
    }

    {
        int k;
        for (k = 1; k < argc; k++) {
            if (strcmp(argv[k], "--dumpstate") == 0) {
                int f, fr;
                uint32_t frames = (k + 1 < argc) ? (uint32_t)strtoul(argv[k + 1], NULL, 10) : 300u;
                if (frames == 0) frames = 300;
                for (fr = 0; fr < (int)frames; fr++) { nes_run_frame(app.nes); audio_pump(&app); }
                printf("=== state after %u frames ===\n", frames);
                printf("CPU pc=%04X a=%02X x=%02X y=%02X s=%02X p=%02X cycles=%llu\n",
                       app.nes->cpu->pc, app.nes->cpu->a, app.nes->cpu->x,
                       app.nes->cpu->y, app.nes->cpu->s, app.nes->cpu->p,
                       (unsigned long long)app.nes->cpu_cycles);
                printf("PPU ctrl=%02X mask=%02X status=%02X oamaddr=%02X v=%04X t=%04X x=%d w=%d\n",
                       app.nes->ppu->ctrl, app.nes->ppu->mask, app.nes->ppu->status,
                       app.nes->ppu->oam_addr, app.nes->ppu->v, app.nes->ppu->t,
                       app.nes->ppu->fine_x, app.nes->ppu->w);
                {
                    int nonbg = 0, yy, xx;
                    for (yy = 0; yy < 240; yy++)
                        for (xx = 0; xx < 256; xx++)
                            if (app.nes->ppu->framebuffer[yy][xx] !=
                                app.nes->ppu->framebuffer[0][0]) nonbg++;
                    printf("pixels differing from [0][0]: %d  [0][0]=%06X\n", nonbg,
                           app.nes->ppu->framebuffer[0][0]);
                }
                printf("stack $01E0..$01FF:");
                for (f = 0; f < 32; f++) {
                    if ((f % 16) == 0) printf("\n  %04X:", 0x1E0 + f);
                    printf(" %02X", app.nes->ram[(0x1E0 + f) & 0x7FF]);
                }
                printf("\nram[$00..$0F]:");
                for (f = 0; f < 16; f++) printf(" %02X", app.nes->ram[f]);
                printf("\n");
                printf("nmi_line=%d nmi_pending=%d irq_line=%d frames=%llu\n",
                       app.nes->cpu->nmi_line, app.nes->cpu->nmi_pending,
                       app.nes->cpu->irq_line,
                       (unsigned long long)app.nes->frame_count);
                printf("palette:");
                for (f = 0; f < 32; f++) printf(" %02X", app.nes->ppu->pal[f]);
                printf("\nciram[0..63]:");
                for (f = 0; f < 64; f++) printf(" %02X", app.nes->ppu->ciram[f]);
                printf("\noam[0..31]:");
                for (f = 0; f < 32; f++) printf(" %02X", app.nes->ppu->oam[f]);
                printf("\nram[$00..$0F]:");
                for (f = 0; f < 16; f++) printf(" %02X", app.nes->ram[f]);
                printf("\nRAM page 6 ($0600..$06FF):");
                for (f = 0; f < 256; f++) {
                    if ((f % 32) == 0) printf("\n  %04X:", 0x600 + f);
                    printf(" %02X", app.nes->ram[(0x600 + f) & 0x7FF]);
                }
                printf("\n");
                app_shutdown(&app);
                SDL_Quit();
                return 0;
            }
            if (strcmp(argv[k], "--ramhex") == 0) {
                uint32_t a = (uint32_t)strtoul(argv[k + 1], NULL, 16);
                uint32_t n = (uint32_t)strtoul(argv[k + 2], NULL, 10);
                uint32_t j;
                printf("RAM at $%04X:", a);
                for (j = 0; j < n; j++) {
                    if ((j % 16) == 0) printf("\n  %04X:", a + j);
                    printf(" %02X", app.nes->ram[(a + j) & 0x7FF]);
                }
                printf("\n");
                app_shutdown(&app);
                SDL_Quit();
                return 0;
            }
            if (strcmp(argv[k], "--rombytes") == 0) {
                uint32_t a = (uint32_t)strtoul(argv[k + 1], NULL, 16);
                uint32_t n = (uint32_t)strtoul(argv[k + 2], NULL, 10);
                uint32_t j;
                printf("ROM bytes at $%04X:", a);
                for (j = 0; j < n; j++)
                    printf(" %02X", cart_cpu_read(app.nes->cart, (uint16_t)(a + j)));
                printf("\n");
                app_shutdown(&app);
                SDL_Quit();
                return 0;
            }
        }
    }

    if (trace_every < 0 && trace_every > -990 && trace_every != -996 && trace_every != -999 && trace_every != -998) {
        /* Instruction trace mode: run one instruction at a time and print it. */
        int n = -trace_every;
        int k;
        (void)n;
        for (k = 0; k < n; k++) {
            uint16_t pc0 = app.nes->cpu->pc;
            uint8_t op0 = nes_cpu_read(app.nes, pc0);
            uint8_t a0 = app.nes->cpu->a, x0 = app.nes->cpu->x, y0 = app.nes->cpu->y;
            uint8_t s0 = app.nes->cpu->s, p0 = app.nes->cpu->p;
            int guard = 0;
            do {
                nes_run_cpu_cycle(app.nes);
                guard++;
                if (trace_ppu) {
                    printf("    cyc=%llu pc=%04X addr=%04X %s data=%02X\n",
                           (unsigned long long)app.nes->cpu_cycles,
                           app.nes->cpu->pc, app.nes->cpu->bus_addr,
                           app.nes->cpu->bus_write ? "W" : "R",
                           app.nes->cpu->bus_data);
                    fflush(stdout);
                }
            } while (app.nes->cpu->steps != NULL && guard < 20);
            while (app.nes->cpu->steps != NULL && guard < 20);
            if (app.nes->cpu->steps != NULL) {
                printf("!! instruction at %04X did not finish in %d cycles\n", pc0, guard);
                fflush(stdout);
                break;
            }
            printf("%04X  %02X  A:%02X X:%02X Y:%02X S:%02X P:%02X -> A:%02X X:%02X Y:%02X S:%02X P:%02X  (%s)\n",
                   pc0, op0, a0, x0, y0, s0, p0,
                   app.nes->cpu->a, app.nes->cpu->x, app.nes->cpu->y,
                   app.nes->cpu->s, app.nes->cpu->p,
                   cpu_op_table[op0].name);
            fflush(stdout);
        }
        app_shutdown(&app);
        SDL_Quit();
        return 0;
    }

    if (trace_ppu) {
        fprintf(stderr, "[ppu] after init: line=%d dot=%d frames=%llu dots=%llu\n",
                app.nes->ppu->line, app.nes->ppu->dot,
                (unsigned long long)app.nes->frame_count,
                (unsigned long long)app.nes->ppu->dot_count);
    }

    if (SDL_getenv("NES_CYCLE_REPORT")) {
        /* Report the exact wall-clock cycles each instruction takes, by
         * measuring the CPU cycle counter at instruction boundaries. */
        long long i, lim = 200;
        const char *ls = SDL_getenv("NES_CYCLE_REPORT");
        lim = strtoll(ls, NULL, 10);
        for (i = 0; i < lim; i++) {
            uint16_t pc = app.nes->cpu->pc;
            uint64_t cyc0 = app.nes->cpu_cycles;
            uint64_t instr0 = app.nes->cpu->instructions;
            int guard = 0;
            uint8_t op = nes_cpu_read(app.nes, pc);
            do { nes_run_cpu_cycle(app.nes); guard++; }
            while (app.nes->cpu->instructions == instr0 && guard < 25);
            /* The first iteration of the do-while retires the *previous*
             * instruction's remainder, so it is not part of this one. */
            printf("%04X %02X ticks=%d real=%llu -> %04X\n", pc, op,
                   guard - 1 < 0 ? 0 : guard - 1,
                   (unsigned long long)(app.nes->cpu_cycles - cyc0),
                   app.nes->cpu->pc);
            if (op == 0xEA && pc >= 0x8010) break;  /* sentinel NOP */
        }
        app_shutdown(&app);
        SDL_Quit();
        return 0;
    }

    {
        const char *bl = SDL_getenv("NES_BUSLOG");
        if (bl) app.nes->buslog = fopen(bl, "w");
        /* NES_BUSLOG_FROM / NES_BUSLOG_TO limit the trace to a CPU-cycle window
         * so a 4 GB run does not have to be written and then grepped. */
        {
            const char *bf = SDL_getenv("NES_BUSLOG_FROM");
            const char *bt = SDL_getenv("NES_BUSLOG_TO");
            if (bf) app.nes->buslog_from = strtoull(bf, NULL, 10);
            if (bt) app.nes->buslog_to = strtoull(bt, NULL, 10);
        }
    }

    if (SDL_getenv("NES_AUTOINPUT")) {
        /* Scripted input for automated test-ROM runs.  The value is a list of
         * frame:button:duration triples, e.g. "100:start:4,200:a:4".  Frames
         * are absolute.  Buttons: a,b,select,start,up,down,left,right. */
        const char *spec = SDL_getenv("NES_AUTOINPUT");
        int frames = 3000;
        const char *fs = SDL_getenv("NES_AUTOFRAMES");
        if (fs) frames = SDL_atoi(fs);
        int shot = -1;
        const char *ss = SDL_getenv("NES_AUTOSHOT");
        if (ss) shot = SDL_atoi(ss);
        char buf[512];
        snprintf(buf, sizeof(buf), "%s", spec);
        /* Optional per-frame CPU-cycle budget (NES_AUTOCYCLES): when the
         * emulated machine is much faster than a real console an entire test
         * suite can finish before a single scripted input event arrives, which
         * makes frame-counted automation useless.  Running a scripted amount of
         * CPU time per "frame" restores the real console's ratio. */
        long long autocycles = 0;
        {
            const char *ac = SDL_getenv("NES_AUTOCYCLES");
            if (ac) autocycles = strtoll(ac, NULL, 10);
        }
        /* NES_AUTOTRACE=<n>: print one compact progress line every n scripted
         * frames so a long run can be watched instead of guessed at. */
        int autotrace = 0;
        {
            const char *at = SDL_getenv("NES_AUTOTRACE");
            if (at) autotrace = SDL_atoi(at);
        }
        int autostate = -1;
        {
            const char *ast = SDL_getenv("NES_AUTOSTATE");
            if (ast) autostate = SDL_atoi(ast);
        }
        /* NES_AUTOSNAP=<frame>[,...]: freeze the emulated machine for a few
         * scripted frames so the caller can read the ROM's result bytes before
         * a later test overwrites them. */
        int snap_first = -1, snap_every = 0, snap_max = 1, snap_len = 3;
        {
            const char *sp = SDL_getenv("NES_AUTOSNAP");
            if (sp) {
                sscanf(sp, "%d,%d,%d,%d", &snap_first, &snap_every, &snap_max, &snap_len);
                if (snap_len < 1) snap_len = 1;
            }
        }
        int snap_left = 0, snap_done = 0;
        /* Automatic result capture: the ROM's zero page $18..$1B holds the
         * pointer to the current page's result slots, the cursor at $16 selects
         * one, and a slot holding $01 means "in progress" (RunTest writes it
         * before PerformTest).  Watching for in-progress -> finished lets the
         * emulator grab each result the moment it exists, instead of racing a
         * later test that overwrites the same scratch byte. */
        int autores = SDL_getenv("NES_AUTORES") != NULL;
        int prev_inprog = 0;
        int prev_cursor = -1;
        int ntri = 0;
        struct { int frame, dur; char btn[16]; } tri[32];
        {
            char *tok = strtok(buf, ",");
            while (tok && ntri < 32) {
                int fr, du; char bt[16];
                if (sscanf(tok, "%d:%15[^:]:%d", &fr, bt, &du) == 3) {
                    tri[ntri].frame = fr; tri[ntri].dur = du;
                    snprintf(tri[ntri].btn, sizeof(tri[ntri].btn), "%s", bt);
                    ntri++;
                }
                tok = strtok(NULL, ",");
            }
        }
        {
            int f, k;
            static const struct { const char *n; int b; } map[] = {
                {"a",0},{"b",1},{"select",2},{"start",3},
                {"up",4},{"down",5},{"left",6},{"right",7}
            };
            fprintf(stderr, "\n");
            for (f = 0; f < frames; f++) {
                if (snap_first >= 0 && f >= snap_first && snap_done < snap_max &&
                    (snap_every <= 0 || ((f - snap_first) % snap_every) == 0)) {
                    char path[512];
                    const char *out = SDL_getenv("NES_AUTOSHOTFILE");
                    snprintf(path, sizeof(path), "%s.snap%d.ram", out ? out : "snap", snap_done);
                    {
                        FILE *sf = fopen(path, "wb");
                        if (sf) { fwrite(app.nes->ram, 1, 0x800, sf); fclose(sf); }
                    }
                    snap_done++;
                    snap_left = snap_len;
                }
                if (snap_left > 0) {
                    snap_left--;
                    for (k = 0; k < 8; k++)
                        input_set_button(&app.nes->input, 0, map[k].b, 0);
                    continue;
                }
                if (autores) {
                    int cursor = app.nes->ram[0x16];
                    int height = app.nes->ram[0x17];
                    int inprog = 0, slot = 0;
                    if (cursor >= 0 && cursor < height && height > 0 && height < 64) {
                        slot = ((app.nes->ram[0x18] | (app.nes->ram[0x19] << 8)) +
                                cursor * 2) & 0x7FF;
                        inprog = (app.nes->ram[slot] == 0x01);
                    }
                    if ((cursor != prev_cursor || inprog) && inprog) {
                        ; /* a test just started: remember nothing yet */
                    }
                    if (prev_inprog && !inprog && slot != 0) {
                        char path[512];
                        const char *out = SDL_getenv("NES_AUTOSHOTFILE");
                        snprintf(path, sizeof(path), "%s.res%02X.ram",
                                 out ? out : "res", app.nes->ram[slot]);
                        {
                            FILE *sf = fopen(path, "wb");
                            if (sf) { fwrite(app.nes->ram, 1, 0x800, sf); fclose(sf); }
                        }
                        fprintf(stderr, "R cursor=%02X slot=%04X val=%02X\n",
                                cursor, slot, app.nes->ram[slot]);
                        fflush(stderr);
                    }
                    prev_inprog = inprog;
                    prev_cursor = cursor;
                }
                if (autocycles > 0) {
                    long long n;
                    for (n = 0; n < autocycles; n++)
                        nes_run_cpu_cycle(app.nes);
                    app.nes->frame_count++;
                } else {
                    nes_run_frame(app.nes);
                }
                audio_pump(&app);
                /* --palette FIRST [STEP] [LAST]: per-frame PPU palette dump
                 * for scripted runs, which never reach the interactive loop. */
                if (pal_first >= 0 && f >= pal_first && f <= pal_last &&
                    ((f - pal_first) % (pal_step > 0 ? pal_step : 1)) == 0) {
                    int a;
                    fprintf(stderr, "PAL %d ctrl=%02X mask=%02X v=%04X t=%04X pal=",
                            f, app.nes->ppu->ctrl, app.nes->ppu->mask,
                            app.nes->ppu->v, app.nes->ppu->t);
                    for (a = 0; a < 32; a++) fprintf(stderr, "%02X", app.nes->ppu->pal[a]);
                    fprintf(stderr, "\n");
                    fflush(stderr);
                }
                if (shot >= 0 && f >= shot) {
                    const char *out = SDL_getenv("NES_AUTOSHOTFILE");
                    /* One-line CPU state at the captured frame: the quickest way
                     * to see where a test ROM is parked when a suite stalls. */
                    fprintf(stderr, "shot frame=%d pc=%04X a=%02X x=%02X y=%02X "
                            "s=%02X p=%02X cycles=%llu ctrl=%02X mask=%02X\n",
                            f, app.nes->cpu->pc, app.nes->cpu->a, app.nes->cpu->x,
                            app.nes->cpu->y, app.nes->cpu->s, app.nes->cpu->p,
                            (unsigned long long)app.nes->cpu_cycles,
                            app.nes->ppu->ctrl, app.nes->ppu->mask);
                    if (out) {
                        FILE *rf = fopen(out, "wb");
                        if (rf) { fwrite(app.nes->ppu->framebuffer, 4, 256u*240u, rf); fclose(rf); }
                        /* Work RAM next to the framebuffer: the only way to read
                         * the harness's own zero-page state from a scripted run. */
                        {
                            char ram[512];
                            snprintf(ram, sizeof(ram), "%s.ram", out);
                            rf = fopen(ram, "wb");
                            if (rf) { fwrite(app.nes->ram, 1, 0x800, rf); fclose(rf); }
                        }
                    }
                    shot = -1;
                }
                for (k = 0; k < 8; k++) {
                    /* OR across every triple for this button: a later triple for
                     * the same button must not clear an earlier press. */
                    int on = 0, t;
                    for (t = 0; t < ntri; t++)
                        if (strcmp(tri[t].btn, map[k].n) == 0 &&
                            f >= tri[t].frame && f < tri[t].frame + tri[t].dur)
                            on = 1;
                    input_set_button(&app.nes->input, 0, map[k].b, on != 0);
                }
                if (autostate >= 0 && f == autostate) {
                int a;
                fprintf(stderr, "STATE pc=%04X a=%02X x=%02X y=%02X s=%02X p=%02X cyc=%llu "
                        "ppu[line=%d dot=%d ctrl=%02X mask=%02X status=%02X v=%04X t=%04X w=%d fx=%d]\n",
                        app.nes->cpu->pc, app.nes->cpu->a, app.nes->cpu->x,
                        app.nes->cpu->y, app.nes->cpu->s, app.nes->cpu->p,
                        (unsigned long long)app.nes->cpu_cycles,
                        app.nes->ppu->line, app.nes->ppu->dot,
                        app.nes->ppu->ctrl, app.nes->ppu->mask, app.nes->ppu->status,
                        app.nes->ppu->v, app.nes->ppu->t, app.nes->ppu->w, app.nes->ppu->fine_x);
                fprintf(stderr, "  pal:");
                for (a = 0; a < 32; a++) fprintf(stderr, " %02X", app.nes->ppu->pal[a]);
                fprintf(stderr, "\n  ciram:");
                for (a = 0; a < 64; a++) fprintf(stderr, " %02X", app.nes->ppu->ciram[a]);
                fprintf(stderr, "\n  oam:");
                for (a = 0; a < 32; a++) fprintf(stderr, " %02X", app.nes->ppu->oam[a]);
                fprintf(stderr, "\n  ram0:");
                for (a = 0; a < 8; a++) fprintf(stderr, " %02X", app.nes->ram[a]);
                fprintf(stderr, "\n  spr: count=%d s0line=%d armed=",
                        app.nes->ppu->spr_count, app.nes->ppu->sprite0_on_line);
                for (a = 0; a < 8; a++) fprintf(stderr, "%d", app.nes->ppu->spr_lane_armed[a]);
                fprintf(stderr, " x=");
                for (a = 0; a < 8; a++) fprintf(stderr, " %02X", app.nes->ppu->spr_lane_x[a]);
                fprintf(stderr, " cnt=");
                for (a = 0; a < 8; a++) fprintf(stderr, " %02X", app.nes->ppu->spr_lane_cnt[a]);
                fprintf(stderr, " pat=");
                for (a = 0; a < 8; a++)
                    fprintf(stderr, " %02X%02X", app.nes->ppu->spr_lane_pat[a][0],
                            app.nes->ppu->spr_lane_pat[a][1]);
                fprintf(stderr, " is0=");
                for (a = 0; a < 8; a++) fprintf(stderr, "%d", app.nes->ppu->spr_lane_is0[a]);
                fprintf(stderr, " done=");
                for (a = 0; a < 8; a++) fprintf(stderr, "%d", app.nes->ppu->spr_lane_done[a]);
                fprintf(stderr, "\n  oam2:");
                for (a = 0; a < 32; a++) fprintf(stderr, " %02X", app.nes->ppu->oam2[a]);
                fprintf(stderr, "\n");
            }
                if (autotrace > 0 && (f % autotrace) == 0) {
                    int a;
                    fprintf(stderr, "T %d cur=%02X h=%02X pc=%04X cyc=%llu", f,
                            app.nes->ram[0x16], app.nes->ram[0x17],
                            app.nes->cpu->pc,
                            (unsigned long long)app.nes->cpu_cycles);
                    for (a = 0x400; a < 0x460; a += 2)
                        if (app.nes->ram[a])
                            fprintf(stderr, " %03X=%02X", a, app.nes->ram[a]);
                    fprintf(stderr, "\n");
                    fflush(stderr);
                }
            }
        }
        if (SDL_getenv("NES_AUTORUNTO")) {
            /* Diagnostic: run to an absolute CPU cycle, then report where the
             * machine is and the first instructions it executes. */
            long long target = strtoll(SDL_getenv("NES_AUTORUNTO"), NULL, 10);
            int i, count = 40;
            if (SDL_getenv("NES_AUTORUNCOUNT"))
                count = SDL_atoi(SDL_getenv("NES_AUTORUNCOUNT"));
            while ((long long)app.nes->cpu_cycles < target)
                nes_run_cpu_cycle(app.nes);
            fprintf(stderr, "RUNTO cycles=%llu pc=%04X a=%02X x=%02X y=%02X s=%02X p=%02X "
                    "ctrl=%02X mask=%02X line=%d dot=%d\n",
                    (unsigned long long)app.nes->cpu_cycles, app.nes->cpu->pc,
                    app.nes->cpu->a, app.nes->cpu->x, app.nes->cpu->y,
                    app.nes->cpu->s, app.nes->cpu->p,
                    app.nes->ppu->ctrl, app.nes->ppu->mask,
                    app.nes->ppu->line, app.nes->ppu->dot);
            for (i = 0; i < count; i++) {
                uint16_t pc = app.nes->cpu->pc;
                uint8_t op = nes_cpu_read(app.nes, pc);
                uint64_t c0 = app.nes->cpu_cycles;
                int guard = 0;
                nes_run_cpu_cycle(app.nes);
                while (app.nes->cpu->steps != NULL && guard++ < 12)
                    nes_run_cpu_cycle(app.nes);
                fprintf(stderr, "  %04X %02X -> %04X (%llu cyc)\n", pc, op,
                        app.nes->cpu->pc,
                        (unsigned long long)(app.nes->cpu_cycles - c0));
            }
        }
        if (SDL_getenv("NES_AUTODUMPPPU"))
            ppu_dump_ring();
        if (snap_done)
            fprintf(stderr, "snapshots written: %d (first frame %d, every %d)\n",
                    snap_done, snap_first, snap_every);
        if (SDL_getenv("NES_AUTODIAG")) {
            int a;
            fprintf(stderr, "auto end: frames=%llu cycles=%llu pc=%04X a=%02X x=%02X y=%02X "
                    "s=%02X p=%02X ctrl=%02X mask=%02X line=%d dot=%d\n",
                    (unsigned long long)app.nes->frame_count,
                    (unsigned long long)app.nes->cpu_cycles,
                    app.nes->cpu->pc, app.nes->cpu->a, app.nes->cpu->x,
                    app.nes->cpu->y, app.nes->cpu->s, app.nes->cpu->p,
                    app.nes->ppu->ctrl, app.nes->ppu->mask,
                    app.nes->ppu->line, app.nes->ppu->dot);
            for (a = 0x400; a < 0x410; a++) fprintf(stderr, " %02X", app.nes->ram[a]);
            fprintf(stderr, "\n44E=%02X menuCursorY=%02X menuCursorX=%02X menuHeight=%02X\n",
                    app.nes->ram[0x44E], app.nes->ram[0x16], app.nes->ram[0x15],
                    app.nes->ram[0x17]);
        }
        app_shutdown(&app);
        SDL_Quit();
        return 0;
    }





    if (trace_every == -996) {
        /* Dump the PPU framebuffer for the first N frames as PPM. */
        int n = -trace_every;
        (void)n;
        int frames = 40, k;
        const char *dir = SDL_getenv("NES_DUMPDIR");
        if (!dir) dir = "/tmp/frames";
        for (k = 0; k < frames; k++) {
            char path[400];
            FILE *f;
            int y, x;
            nes_run_frame(app.nes);
            audio_pump(&app);
            snprintf(path, sizeof(path), "%s/f%03d.ppm", dir, k + 1);
            f = fopen(path, "wb");
            if (!f) continue;
            fprintf(f, "P6\n256 240\n255\n");
            for (y = 0; y < 240; y++)
                for (x = 0; x < 256; x++) {
                    uint32_t c = app.nes->ppu->framebuffer[y][x];
                    unsigned char px[3] = { (unsigned char)((c >> 16) & 0xFF),
                                            (unsigned char)((c >> 8) & 0xFF),
                                            (unsigned char)(c & 0xFF) };
                    fwrite(px, 1, 3, f);
                }
            fclose(f);
        }
        printf("dumped %d frames to %s\n", frames, dir);
        printf("final pc=%04X ctrl=%02X mask=%02X\n", app.nes->cpu->pc,
               app.nes->ppu->ctrl, app.nes->ppu->mask);
        app_shutdown(&app);
        SDL_Quit();
        return 0;
    }

    if (trace_every == -997) {
        /* Single-step with full register write-back, starting at a given PC. */
        const char *start = SDL_getenv("NES_START_PC");
        uint16_t want = start ? (uint16_t)strtoul(start, NULL, 16) : 0xF0EC;
        long long i;
        for (i = 0; i < 30000; i++) {
            uint16_t pc0 = app.nes->cpu->pc;
            uint8_t op0 = nes_cpu_read(app.nes, pc0);
            uint8_t b1 = nes_cpu_read(app.nes, (uint16_t)(pc0+1));
            uint8_t b2 = nes_cpu_read(app.nes, (uint16_t)(pc0+2));
            int guard = 0;
            do { nes_run_cpu_cycle(app.nes); guard++; }
            while (app.nes->cpu->steps != NULL && guard < 25);
            if (pc0 >= want) {
                fprintf(stderr, "%04X: %02X %02X %02X | -> pc=%04X A=%02X X=%02X Y=%02X S=%02X P=%02X\n",
                        pc0, op0, b1, b2, app.nes->cpu->pc, app.nes->cpu->a,
                        app.nes->cpu->x, app.nes->cpu->y, app.nes->cpu->s, app.nes->cpu->p);
            }
        }
        app_shutdown(&app);
        SDL_Quit();
        return 0;
    }

    if (trace_every == -998) {
        /* Watch for the first execution below $8000 and dump the path there. */
        uint16_t ring_pc[48]; uint8_t ring_op[48], ring_s[48];
        int idx = 0; long long iter = 0;
        for (;;) {
            uint16_t pc0 = app.nes->cpu->pc;
            uint8_t op0 = nes_cpu_read(app.nes, pc0);
            ring_pc[idx % 48] = pc0; ring_op[idx % 48] = op0;
            ring_s[idx % 48] = app.nes->cpu->s;
            idx++; iter++;
            if (pc0 < 0x8000 && iter > 200) {
                int k;
                fprintf(stderr, "*** first RAM execution at %04X after %lld instructions (cyc=%llu)\n",
                        pc0, iter, (unsigned long long)app.nes->cpu_cycles);
                fprintf(stderr, "  ring (oldest..newest):\n");
                for (k = 47; k > 0; k--) {
                    int j = (idx - k) % 48; if (j < 0) j += 48;
                    fprintf(stderr, "  %04X %02X S=%02X\n", ring_pc[j], ring_op[j], ring_s[j]);
                }
                break;
            }
            {
                int guard = 0;
                do { nes_run_cpu_cycle(app.nes); guard++; }
                while (app.nes->cpu->steps != NULL && guard < 25);
                if (guard >= 25) { fprintf(stderr, "stuck at %04X\n", pc0); break; }
            }
        }
        app_shutdown(&app);
        SDL_Quit();
        return 0;
    }

    if (trace_every == -999) {
        /* Crash hunt: keep a ring buffer of the last 48 (pc, opcode, sp) and
         * print it when PC first reaches the given address. */
        static uint16_t ring_pc[64];
        static uint8_t ring_op[64];
        static uint8_t ring_sp[64];
        int idx = 0, hits = 0;
        for (i = 0; i < 4000000; i++) {
            uint16_t pc0 = app.nes->cpu->pc;
            uint8_t op0 = nes_cpu_read(app.nes, pc0);
            uint8_t sp0 = app.nes->cpu->s;
            int guard = 0;
            ring_pc[idx & 63] = pc0; ring_op[idx & 63] = op0; ring_sp[idx & 63] = sp0;
            idx++;
            if (idx == 60) {
                int k;
                fprintf(stderr, "*** dump at instruction 60\n");
                for (k = 40; k > 0; k--) {
                    int j = (idx - k) & 63;
                    fprintf(stderr, "  %04X %02X S=%02X\n", ring_pc[j], ring_op[j], ring_sp[j]);
                }
            }
            if (pc0 == 0x06F6) {
                int k;
                fprintf(stderr, "*** reached trap after %d instructions\n", idx);
                for (k = 48; k > 0; k--) {
                    int j = (idx - k) & 63;
                    fprintf(stderr, "  %04X %02X S=%02X\n", ring_pc[j], ring_op[j], ring_sp[j]);
                }
                break;
            }
            do { nes_run_cpu_cycle(app.nes); guard++; }
            while (app.nes->cpu->steps != NULL && guard < 25);
            if (guard >= 25) { fprintf(stderr, "stuck at %04X\n", pc0); break; }
        }
        app_shutdown(&app);
        SDL_Quit();
        return 0;
    }

    if (ppudump_frame >= 0) {
        /* --ppudump DIR FRAME: run to FRAME and write the complete PPU state
         * (framebuffer, palette, OAM, CIRAM pages, both pattern tables) so a
         * reference emulator's capture of the same screen can be diffed
         * pixel-by-pixel without guessing what the PPU held. */
        int f;
        char path[600];
        PpuDump d;
        for (f = 0; f <= ppudump_frame; f++) run_one_frame(&app);
        ppu_dump_state(app.nes->ppu, &d);
        snprintf(path, sizeof(path), "%s/fb.raw", ppudump_dir);
        {
            FILE *fp = fopen(path, "wb");
            if (fp) { fwrite(d.framebuffer, 4, 256u * 240u, fp); fclose(fp); }
        }
        snprintf(path, sizeof(path), "%s/pal.bin", ppudump_dir);
        { FILE *fp = fopen(path, "wb"); if (fp) { fwrite(d.palette, 1, 32, fp); fclose(fp); } }
        snprintf(path, sizeof(path), "%s/oam.bin", ppudump_dir);
        { FILE *fp = fopen(path, "wb"); if (fp) { fwrite(d.oam, 1, 256, fp); fclose(fp); } }
        snprintf(path, sizeof(path), "%s/ciram.bin", ppudump_dir);
        { FILE *fp = fopen(path, "wb"); if (fp) { fwrite(d.ciram, 1, 2048, fp); fclose(fp); } }
        snprintf(path, sizeof(path), "%s/chr.bin", ppudump_dir);
        {
            int a;
            FILE *fp = fopen(path, "wb");
            if (fp) {
                for (a = 0; a < 0x2000; a++)
                    fputc(ppu_chr_read(app.nes->ppu, (uint16_t)a), fp);
                fclose(fp);
            }
        }
        printf("ppudump frame=%d ctrl=%02X mask=%02X v=%04X t=%04X fx=%d pc=%04X\n",
               ppudump_frame, d.ctrl, d.mask, d.v, d.t, d.fine_x, app.nes->cpu->pc);
        app_shutdown(&app);
        SDL_Quit();
        return 0;
    }

    {
        /* Self-test hook: inject a file-drop event so the drag-and-drop path can
         * be exercised without a GUI (NES_DROP_TEST=<rom>). */
        const char *drop_test = SDL_getenv("NES_DROP_TEST");
        if (drop_test && *drop_test) {
            SDL_Event ev;
            SDL_zero(ev);
            ev.type = SDL_EVENT_DROP_FILE;
            ev.drop.data = drop_test;
            ev.drop.windowID = SDL_GetWindowID(app.render.window);
            SDL_PushEvent(&ev);
        }
    }

    while (app.running) {
        handle_events(&app);
        if (!app.running)
            break;   /* the quit event was handled: do not run another frame */
        /* An explicit "Resume" always wins, including over a P-key pause the
         * user made before opening the menu. */
        if (hud_take_resume(&app.hud))
            app.paused = false;

        if (!app.has_rom) {
            /* Nothing to emulate until a ROM is dropped onto the window; the
             * frame counter still ticks so --screenshot can capture the hint. */
            app.frame_counter++;
        } else if (!hud_is_open(&app.hud) && !app.paused) {
            bool ff = action_pressed(&app, 0, CTL_FAST_FORWARD);
            int iterations = ff ? 8 : 1;
            for (i = 0; i < iterations && app.running; i++)
                run_one_frame(&app);
        } else if (app.frame_advance) {
            run_one_frame(&app);
            app.frame_advance = false;
        }

        update_nes_input(&app);
        handle_hotkeys(&app);

        /* FPS window */
        {
            uint64_t now = SDL_GetTicks();
            if (now - app.fps_timer >= 500) {
                app.fps = (float)((app.fps_frames * 1000.0) / (double)(now - app.fps_timer));
                app.fps_timer = now;
                app.fps_frames = 0;
            }
        }

        if (trace_every > 0 && (app.frame_counter % trace_every) == 0 &&
            app.frame_counter <= (uint64_t)trace_every * trace_max_frames) {
            int nonzero = 0;
            int yy, xx;
            for (yy = 0; yy < 240; yy++)
                for (xx = 0; xx < 256; xx++)
                    if (app.nes->ppu->framebuffer[yy][xx] != 0) nonzero++;
            printf("[trace] frame %llu PC=%04X A=%02X X=%02X Y=%02X S=%02X P=%02X "
                   "cycles=%llu PPU line=%d dot=%d ctrl=%02X mask=%02X status=%02X "
                   "v=%04X vram[0..7]=%02X%02X%02X%02X%02X%02X%02X%02X "
                   "chr0=%02X%02X palette0=%02X nonsky=%d\n",
                   (unsigned long long)app.frame_counter, app.nes->cpu->pc,
                   app.nes->cpu->a, app.nes->cpu->x, app.nes->cpu->y,
                   app.nes->cpu->s, app.nes->cpu->p,
                   (unsigned long long)app.nes->cpu_cycles,
                   app.nes->ppu->line, app.nes->ppu->dot,
                   app.nes->ppu->ctrl, app.nes->ppu->mask, app.nes->ppu->status,
                   app.nes->ppu->v,
                   app.nes->ppu->ciram[0], app.nes->ppu->ciram[1],
                   app.nes->ppu->ciram[2], app.nes->ppu->ciram[3],
                   app.nes->ppu->ciram[4], app.nes->ppu->ciram[5],
                   app.nes->ppu->ciram[6], app.nes->ppu->ciram[7],
                   cart_chr_read(app.nes->cart, 0), cart_chr_read(app.nes->cart, 1),
                   app.nes->ppu->pal[0], nonzero);
        }

        if (!app.headless) {
            render_begin(&app.render);
            if (app.has_rom) {
                render_present(&app.render,
                               (const uint32_t *)app.nes->ppu->framebuffer,
                               NES_SCREEN_W, NES_SCREEN_H);
                /* The menu halts the machine without setting `paused`, so the
                 * overlay has to account for it too. */
                hud_draw(&app.hud, &app.render, app.fps,
                         app.paused || hud_is_open(&app.hud), 1.0f);
            } else {
                draw_rom_hint(&app.render, app.rom_error);
            }
        }

        /* Read the pixels back *before* presenting: after SDL_RenderPresent the
         * back buffer's contents are undefined (it is usually cleared), so
         * capturing there produced an all-black bitmap. */
        if (screenshot_file && screenshot_frame >= 0 &&
            app.frame_counter >= (uint64_t)screenshot_frame) {
            SDL_Surface *shot = SDL_RenderReadPixels(app.render.renderer, NULL);
            if (shot) {
                if (SDL_SaveBMP(shot, screenshot_file))
                    printf("Screenshot written to %s (frame %llu)\n", screenshot_file,
                           (unsigned long long)app.frame_counter);
                else
                    fprintf(stderr, "SDL_SaveBMP('%s') failed: %s\n",
                            screenshot_file, SDL_GetError());
                SDL_DestroySurface(shot);
            } else {
                fprintf(stderr, "SDL_RenderReadPixels failed: %s\n", SDL_GetError());
            }
            app.running = false;
        }
        if (!app.headless)
            render_end(&app.render);
        if (screenshot_file && screenshot_frame >= 0 &&
            app.frame_counter >= (uint64_t)screenshot_frame) {
            /* also dump the raw NES framebuffer for inspection */
            char raw[512];
            snprintf(raw, sizeof(raw), "%s.raw", screenshot_file);
            FILE *rf = fopen(raw, "wb");
            if (rf) {
                fwrite(app.nes->ppu->framebuffer, 4, 256u * 240u, rf);
                fclose(rf);
            }
        }
        if (trace_ppu && (app.frame_counter % 10) == 0) {
            fprintf(stderr, "[ppu] frame=%llu line=%d dot=%d frames=%llu dots=%llu pc=%04X\n",
                    (unsigned long long)app.frame_counter, app.nes->ppu->line,
                    app.nes->ppu->dot,
                    (unsigned long long)app.nes->frame_count,
                    (unsigned long long)app.nes->ppu->dot_count,
                    app.nes->cpu->pc);
        }
    }

    settings_save(&app.settings);
    hud_toast(&app.hud, "Bye");
    app_shutdown(&app);
    SDL_Quit();
    return 0;
}

/* render.c - SDL3 video: one streaming texture for the NES framebuffer plus a
 * bitmap-font overlay for the HUD.
 *
 * The renderer uses SDL's logical presentation so the overlay can be drawn in
 * NES pixel coordinates (0..255 horizontally) regardless of the window size. */
#include <string.h>
#include <stdio.h>

#include "render.h"
#include "hud_font.h"
#include "icon_32.h"

#define NES_W 256
#define NES_H 240

/* The window icon lives in icon_32.h as a hex string (see tools/make_icon.py).
 * SDL_CreateSurfaceFrom() keeps referencing the buffer, so the decoded pixels
 * are kept in a static array for the lifetime of the process. */
static int icon_hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static SDL_Surface *icon_surface(void)
{
    static uint8_t pixels[ICON_W * ICON_H * 4];
    const char *p = ICON_RGBA_HEX;
    size_t i;

    for (i = 0; i < sizeof(pixels) && p[0] && p[1]; i++, p += 2)
        pixels[i] = (uint8_t)((icon_hexval(p[0]) << 4) | icon_hexval(p[1]));
    return SDL_CreateSurfaceFrom(ICON_W, ICON_H, SDL_PIXELFORMAT_RGBA32, pixels,
                                 ICON_W * 4);
}

bool render_init(Renderer *r, const char *title, int scale)
{
    memset(r, 0, sizeof(*r));
    r->scale = scale > 0 ? scale : 3;

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_InitSubSystem(VIDEO) failed: %s", SDL_GetError());
        return false;
    }
    r->window = SDL_CreateWindow(title, NES_W * r->scale, NES_H * r->scale,
                                 SDL_WINDOW_RESIZABLE);
    if (!r->window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return false;
    }
    {
        SDL_Surface *icon = icon_surface();
        if (icon) {
            SDL_SetWindowIcon(r->window, icon);
            SDL_DestroySurface(icon);
        }
    }
    r->renderer = SDL_CreateRenderer(r->window, NULL);
    if (!r->renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderVSync(r->renderer, 1);
    r->vsync = true;

    r->texture = SDL_CreateTexture(r->renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, NES_W, NES_H);
    if (!r->texture) {
        SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
        return false;
    }
    SDL_SetTextureScaleMode(r->texture, SDL_SCALEMODE_NEAREST);
    /* The NES frame buffer carries no alpha (its top byte is always 0).  With
     * the default blend mode every pixel is fully transparent and the window
     * stays black, so the texture must be drawn opaquely. */
    SDL_SetTextureBlendMode(r->texture, SDL_BLENDMODE_NONE);
    SDL_SetRenderLogicalPresentation(r->renderer, NES_W, NES_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    r->tex_w = NES_W;
    r->tex_h = NES_H;
    return true;
}

void render_shutdown(Renderer *r)
{
    if (r->texture) SDL_DestroyTexture(r->texture);
    if (r->renderer) SDL_DestroyRenderer(r->renderer);
    if (r->window) SDL_DestroyWindow(r->window);
    memset(r, 0, sizeof(*r));
}

void render_set_scale(Renderer *r, int scale)
{
    if (scale < 1) scale = 1;
    if (scale > 8) scale = 8;
    r->scale = scale;
    if (r->window)
        SDL_SetWindowSize(r->window, NES_W * scale, NES_H * scale);
}

void render_begin(Renderer *r)
{
    SDL_SetRenderDrawColor(r->renderer, 0, 0, 0, 255);
    SDL_RenderClear(r->renderer);
}

void render_present(Renderer *r, const uint32_t *pixels, int w, int h)
{
    SDL_UpdateTexture(r->texture, NULL, pixels, w * (int)sizeof(uint32_t));
    SDL_RenderTexture(r->renderer, r->texture, NULL, NULL);
    (void)h;
}

void render_fill_rect(Renderer *r, int x, int y, int w, int h, uint8_t cr, uint8_t cg,
                      uint8_t cb, uint8_t ca)
{
    SDL_FRect rect;
    rect.x = (float)x; rect.y = (float)y;
    rect.w = (float)w; rect.h = (float)h;
    SDL_SetRenderDrawBlendMode(r->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->renderer, cr, cg, cb, ca);
    SDL_RenderFillRect(r->renderer, &rect);
}

void render_end(Renderer *r)
{
    SDL_RenderPresent(r->renderer);
}

int render_text_width(const char *text, int scale)
{
    int n = (int)strlen(text);
    if (scale < 1) scale = 1;
    if (n <= 0) return 0;
    return n * (HUD_FONT_W + 1) * scale - scale;
}

void render_text(Renderer *r, int x, int y, const char *text, int scale,
                 uint8_t cr, uint8_t cg, uint8_t cb, uint8_t ca)
{
    SDL_FRect px;
    SDL_SetRenderDrawBlendMode(r->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->renderer, cr, cg, cb, ca);
    if (scale < 1) scale = 1;
    for (; *text; text++) {
        unsigned char ch = (unsigned char)*text;
        int row, col;
        const unsigned char *glyph;
        if (ch < HUD_FONT_FIRST || ch > HUD_FONT_LAST) ch = '?';
        glyph = hud_font[ch - HUD_FONT_FIRST];
        for (row = 0; row < HUD_FONT_ROWS; row++) {
            unsigned char bits = glyph[row];
            for (col = 0; col < HUD_FONT_W; col++) {
                if (bits & (1u << (HUD_FONT_W - 1 - col))) {
                    px.x = (float)(x + col * scale);
                    px.y = (float)(y + row * scale);
                    px.w = (float)scale;
                    px.h = (float)scale;
                    SDL_RenderFillRect(r->renderer, &px);
                }
            }
        }
        x += (HUD_FONT_W + 1) * scale;
    }
}

/* render.h - SDL3 video output for the emulator framebuffer. */
#ifndef RENDER_H
#define RENDER_H

#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct Renderer {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *texture;
    int           scale;
    int           tex_w, tex_h;
    bool          vsync;
} Renderer;

bool render_init(Renderer *r, const char *title, int scale);
void render_shutdown(Renderer *r);
void render_present(Renderer *r, const uint32_t *pixels, int w, int h);
void render_set_scale(Renderer *r, int scale);
/* Draw a filled rectangle in window coordinates. */
void render_fill_rect(Renderer *r, int x, int y, int w, int h, uint8_t cr, uint8_t cg,
                      uint8_t cb, uint8_t ca);
void render_begin(Renderer *r);
void render_end(Renderer *r);
/* Text drawing with the embedded bitmap font.  scale = integer pixel scale. */
void render_text(Renderer *r, int x, int y, const char *text, int scale,
                 uint8_t cr, uint8_t cg, uint8_t cb, uint8_t ca);
/* Pixel width of `text` when drawn with render_text() at the same scale. */
int  render_text_width(const char *text, int scale);

#endif /* RENDER_H */

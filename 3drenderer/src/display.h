#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <SDL2/SDL.h>

extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;
extern uint32_t* g_color_buffer;
extern SDL_Texture * g_color_buffer_texture;
extern int window_width;
extern int window_height;

int initialize_window(void);
void draw_grid(void);
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void destroy_window(void);

#endif

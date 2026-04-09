#include "display.h"

#include <math.h>

SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;

uint32_t* g_color_buffer = NULL;
SDL_Texture * g_color_buffer_texture = NULL;

int window_width = 800;
int window_height = 600;

int initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return -1;
    }

    // Use SDL to query what the fullscreen max. width and height are
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    window_width = display_mode.w;
    window_height = display_mode.h;

    // Create an SDL Window
    g_window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED, // posX
        SDL_WINDOWPOS_CENTERED, // poxY
        window_width,           // width
        window_height,          // height
        SDL_WINDOW_BORDERLESS
    );

    if (!g_window) {
        fprintf(stderr, "Error creating the SDL window.\n");
        return -1;
    }
    
    // Create an SDL renderer
    g_renderer = SDL_CreateRenderer(g_window, -1, 0);

    if (!g_renderer) {
        fprintf(stderr, "Error creating the SDL Renderer.\n");
        return -1;
    }

    SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN);

    return 0;
    
}

void draw_grid(void) {
    for (int y = 0; y < window_height; y += 10) {
        for (int x = 0; x < window_width; x += 10) {
            g_color_buffer[(window_width * y) + x] = 0xFF333333;
        }
    }
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < window_width && y >= 0 && y < window_height) {
        g_color_buffer[(window_width * y) + x] = color;
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            draw_pixel(x + i, y + j, color);
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int delta_x = x1 - x0;
    int delta_y = y1 - y0;

    int side_length = abs(delta_x) > abs(delta_y) ? abs(delta_x) : abs(delta_y);

    float x_inc = delta_x / (float)side_length;
    float y_inc = delta_y / (float)side_length;

    float curr_x = x0;
    float curr_y = y0;

    for (int i = 0; i < side_length; i++) {
        draw_pixel(curr_x, curr_y, color); 
        curr_x += x_inc;
        curr_y += y_inc;
    }
}

void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            g_color_buffer[(window_width * y) + x] = color;
        }
    }
}

void render_color_buffer(void) {
    SDL_UpdateTexture(
        g_color_buffer_texture,
        NULL,
        g_color_buffer,
        (int)(window_width * sizeof(uint32_t))
    );
    SDL_RenderCopy(g_renderer, g_color_buffer_texture, NULL, NULL); 
}

void destroy_window(void) {
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}


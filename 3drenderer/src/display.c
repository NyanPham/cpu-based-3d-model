#include "display.h"
#include <math.h>

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

/**
 * The Color Buffer:
 * A 1D array representing every pixel on the screen. 
 * Since the screen is 2D, we use the formula `(y * width) + x` to find the correct 1D index.
 * It stores 32-bit integers representing colors (A R G B).
 */
static uint32_t* color_buffer = NULL;

/**
 * The Z-Buffer (Depth Buffer):
 * A 1D array running parallel to the color buffer. Instead of colors, it stores the 
 * Z-depth (distance from camera) of the pixel currently drawn at that location.
 * If we try to draw a new pixel, we check the Z-buffer to see if the new pixel is 
 * closer than what's already there.
 */
static float* z_buffer = NULL;

static SDL_Texture * color_buffer_texture = NULL;
static int window_width = 320;
static int window_height = 200;

static int render_method = 0;
static int cull_method = 0;

int get_window_width() {
    return window_width;
}

int get_window_height() {
    return window_height;
}

// ============================================================================
// Window & SDL Initialization
// ============================================================================

int initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return -1;
    }

    // Use SDL to query what the fullscreen max. width and height are
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    int fullscreen_width = display_mode.w;
    int fullscreen_height = display_mode.h;

    // Scale down the internal rendering resolution for a retro, pixelated look
    // and to save CPU processing power.
    window_width = fullscreen_width / 1.2;
    window_height = fullscreen_height / 1.2;

    // Create an SDL Window
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED, // posX
        SDL_WINDOWPOS_CENTERED, // poxY
        fullscreen_width,       // width
        fullscreen_height,      // height
        SDL_WINDOW_BORDERLESS
    );

    if (!window) {
        fprintf(stderr, "Error creating the SDL window.\n");
        return -1;
    }
    
    // Create an SDL renderer
    renderer = SDL_CreateRenderer(window, -1, 0);

    if (!renderer) {
        fprintf(stderr, "Error creating the SDL Renderer.\n");
        return -1;
    }

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    // Allocate required memory in bytes to hold the Color Buffer and Z-Buffer
    color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
    z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);
    
    // Create an SDL Texture that will act as the bridge between our CPU color_buffer 
    // and the GPU screen display.
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    return 0;
}

// ============================================================================
// Render State Getters/Setters
// ============================================================================

void set_render_method(int method) { render_method = method; }
void set_cull_method(int method) { cull_method = method; }
int is_cull_backface() { return cull_method == CULL_BACKFACE; }
int should_render_filled_triangle() { return render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE; }
int should_render_textured_triangle() { return render_method == RENDER_TEXTURE || render_method == RENDER_TEXTURE_WIRE; }
int should_render_wireframe() { return render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_TEXTURE_WIRE; }
int should_render_wire_vertex() { return render_method == RENDER_WIRE_VERTEX; }

// ============================================================================
// 2D Drawing Primitives
// ============================================================================

void draw_grid(void) {
    for (int y = 0; y < window_height; y += 10) {
        for (int x = 0; x < window_width; x += 10) {
            color_buffer[(window_width * y) + x] = 0xFF333333;
        }
    }
}

/**
 * The most fundamental drawing function.
 * Maps a 2D (x, y) coordinate to the 1D color_buffer array.
 * Formula: Index = (y * width) + x
 */
void draw_pixel(int x, int y, uint32_t color) {
    // Protect against out-of-bounds drawing (Segfault prevention)
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) return;
    
    color_buffer[(window_width * y) + x] = color;
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            draw_pixel(x + i, y + j, color);
        }
    }
}

/**
 * DDA (Digital Differential Analyzer) Line Drawing Algorithm
 * 
 * TEXTBOOK EXPLANATION: HOW TO DRAW A STRAIGHT LINE WITH DISCRETE PIXELS
 * =======================================================================
 * The challenge: We have two points (x0,y0) and (x1,y1), but the screen is 
 * made of discrete pixels. We need to figure out which pixels to light up!
 * 
 * WHY NOT y = mx + b?
 * ===================
 * 1. PERFORMANCE ISSUE: Computing y = mx + b requires a MULTIPLICATION (m*x)
 *    for every single pixel. Multiplication is computationally expensive.
 *    DDA optimizes this by converting multiplication into repeated ADDITION:
 *    curr_x += x_inc, curr_y += y_inc. Addition is much faster on hardware.
 * 
 * 2. THE "HOLES" PROBLEM: If we step along the X-axis for every pixel but the
 *    line is very steep (slope > 1), we'd get gaps! The Y-values would jump
 *    by more than 1 pixel per step, leaving holes in the line.
 * 
 * THE DDA SOLUTION:
 * =================
 * 1. Calculate delta_x = x1-x0 and delta_y = y1-y0
 * 2. Find the "longest axis" - which difference is bigger?
 *    This becomes our step count (side_length).
 * 3. Calculate increments: x_inc = delta_x / side_length, y_inc = delta_y / side_length
 * 4. Step along the longer axis, interpolating the other axis.
 * 
 * EXAMPLE:
 * ========
 * Drawing from (0,0) to (5,2):
 * - delta_x = 5, delta_y = 2
 * - side_length = 5 (X is longer)
 * - x_inc = 1.0, y_inc = 0.4
 * - Pixels: (0,0), (1,0), (2,1), (3,1), (4,2), (5,2) ✓
 * 
 * NOTE: This is NOT Bresenham's algorithm! Bresenham uses ONLY integer math
 * to completely avoid floating-point. DDA here still uses floats but is simpler
 * to understand and implement for educational purposes.
 */
void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int delta_x = x1 - x0;
    int delta_y = y1 - y0;

    // Find the longest side of the line to determine how many pixels to draw
    int side_length = abs(delta_x) > abs(delta_y) ? abs(delta_x) : abs(delta_y);

    // Calculate how much we should increment x and y each step
    float x_inc = delta_x / (float)side_length;
    float y_inc = delta_y / (float)side_length;

    float curr_x = x0;
    float curr_y = y0;

    // Loop through the longest side, placing pixels one by one
    for (int i = 0; i <= side_length; i++) {
        draw_pixel(round(curr_x), round(curr_y), color); 
        curr_x += x_inc;
        curr_y += y_inc;
    }
}

// ============================================================================
// Buffer Operations
// ============================================================================

void clear_color_buffer(uint32_t color) {
    for (int i = 0; i < window_width * window_height; i++) {
        color_buffer[i] = color;
    }
}

void clear_z_buffer() {
    for (int i = 0; i < window_width * window_height; i++) {
        z_buffer[i] = 1.0; // 1.0 is considered the "furthest" possible depth in NDC space
    }
}

float get_zbuffer_at(int x, int y) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) return 0;
    return z_buffer[(window_width * y) + x];
}

void update_zbuffer_at(int x, int y, float v) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) return;
    z_buffer[(window_width * y) + x] = v;
}

void render_color_buffer(void) {
    // 1. Copy the CPU color_buffer array into the SDL Texture
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (int)(window_width * sizeof(uint32_t))
    );
    
    // 2. Copy the texture to the SDL Renderer
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL); 
    
    // 3. Present the Renderer to the actual Screen
    SDL_RenderPresent(renderer);
}

void destroy_window(void) {
    free(color_buffer);
    free(z_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

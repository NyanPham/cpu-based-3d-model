#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <SDL2/SDL.h>

/**
 * @file display.h
 * @brief Handles the 2D screen initialization, the color/depth buffers, and basic 2D drawing.
 * 
 * Before we can render 3D objects, we need a 2D canvas to draw on.
 * This module uses the SDL (Simple DirectMedia Layer) library to create a window 
 * and manage a `color_buffer`. 
 * 
 * The `color_buffer` is just a massive 1D array of pixels. We use a formula 
 * (y * width + x) to find exactly where to place a color to draw 2D shapes.
 */

#define FPS 60 
#define FRAME_TARGET_TIME (1000 / FPS)

// ============================================================================
// Render States & Configuration
// ============================================================================

enum cull_method {
    CULL_NONE,
    CULL_BACKFACE,
};

enum render_method {
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURE,
    RENDER_TEXTURE_WIRE,
};

void set_render_method(int method);
void set_cull_method(int method);
int is_cull_backface(void);

int should_render_filled_triangle(void);
int should_render_textured_triangle(void);
int should_render_wireframe(void);
int should_render_wire_vertex(void);

// ============================================================================
// Window & Buffer Initialization
// ============================================================================

int initialize_window(void);
int get_window_width(void);
int get_window_height(void);
void destroy_window(void);

/**
 * Pushes the `color_buffer` array to the SDL Texture and renders it to the screen.
 * This is called once per frame at the very end of the Graphics Pipeline.
 */
void render_color_buffer(void);

/**
 * Wipes the screen clear with a background color before drawing the next frame.
 */
void clear_color_buffer(uint32_t color);

/**
 * Clears the Z-Buffer (Depth Buffer) to a default value (usually 1.0, representing the furthest depth).
 */
void clear_z_buffer(void);

// ============================================================================
// 2D Drawing Primitives
// ============================================================================

void draw_grid(void);

/**
 * Sets a specific pixel at (x, y) in the color_buffer to a 32-bit ARGB color.
 */
void draw_pixel(int x, int y, uint32_t color);

void draw_rect(int x, int y, int w, int h, uint32_t color);

/**
 * Draws a 2D line between two points using the DDA (Digital Differential Analyzer) algorithm.
 */
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);

// ============================================================================
// Z-Buffer (Depth Buffer)
// ============================================================================

/**
 * Gets the depth value stored at a specific screen pixel.
 * Used to determine if a new pixel is closer to the camera than the previously drawn pixel.
 */
float get_zbuffer_at(int x, int y);

/**
 * Updates the depth value at a specific screen pixel after drawing something closer to the camera.
 */
void update_zbuffer_at(int x, int y, float v);

#endif

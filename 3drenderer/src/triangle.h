#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdint.h>
#include "vector.h"
#include "texture.h" 
#include "upng.h"

/**
 * @file triangle.h
 * @brief The Core Geometric Primitives of the Graphics Pipeline.
 *
 * Textbook Explanation: MESH vs FACE vs TRIANGLE
 * It is completely normal to be confused by the distinction between a "Mesh", 
 * a "Face", and a "Triangle". Here is how data flows from the hard drive to the screen:
 *
 * 1. MESH (The Blueprint - `mesh_t`):
 *    Loaded from an .obj file. It holds a massive array of raw 3D Vertices (points in space) 
 *    and an array of Faces.
 *
 * 2. FACE (The Index Pointer - `face_t`):
 *    A Face does NOT store actual 3D (x,y,z) coordinates! It primarily stores INDICES (integers) 
 *    that point back to the Mesh's vertex array.
 *    Why? In a 3D model, multiple faces share the exact same vertex (e.g., the tip of a pyramid). 
 *    Storing indices instead of duplicating the (x,y,z) floats saves a massive amount of memory.
 *    - Example: `face.a = 5` means "My first corner is at vertex index 5 in the mesh".
 *    It DOES, however, store the 2D (u,v) texture coordinates for each of its corners, as UVs 
 *    are often unique to how a face is unwrapped on the texture map.
 *
 * 3. TRIANGLE (The Screen-Ready Object - `triangle_t`):
 *    A Triangle is created DURING the rendering pipeline. We take the indices from a Face, 
 *    look up their actual 3D coordinates, transform them (rotate, scale, translate), 
 *    apply perspective projection, and clip them. We also pass along the UVs and the texture pointer.
 *    By the time it becomes a `triangle_t`, it stores the actual projected 2D screen coordinates,
 *    the depth (w), and the UVs, all ready to be drawn by the Rasterizer.
 *
 * SUMMARY PIPELINE FLOW:
 * [Mesh (Raw Data)] -> [Face (Index Pointers & UVs)] -> [Pipeline Math] -> [Triangle (Screen Coords & UVs)]
 */

/**
 * Represents a face of a 3D mesh (usually a triangle).
 * It stores indices pointing to the mesh's vertex and UV coordinate arrays.
 * Using indices instead of actual coordinates saves memory, as many faces share the same vertices.
 */
typedef struct {
    int a;
    int b;
    int c;
    tex2_t a_uv; 
    tex2_t b_uv; 
    tex2_t c_uv; 
    uint32_t color;
} face_t;

/**
 * Represents a fully processed triangle ready to be drawn on the screen.
 * Unlike face_t, this stores the actual projected 2D coordinates (with depth info in 'w'),
 * the specific UV coordinates for its vertices, and the texture to be applied.
 */
typedef struct {
    vec4_t points[3];
    tex2_t texcoords[3];
    uint32_t color;
    upng_t* texture;
} triangle_t;

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p);
vec3_t get_triangle_normal(vec4_t vertices[3]);

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

void draw_filled_triangle(
    int x0, int y0, float z0, float w0, 
    int x1, int y1, float z1, float w1,
    int x2, int y2, float z2, float w2,
    uint32_t color
); 

void draw_triangle_texel(
    int x, int y, upng_t* texture, 
    vec4_t point_a, vec4_t point_b, vec4_t pointc,
    tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
);

void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0, 
    int x1, int y1, float z1, float w1, float u1, float v1, 
    int x2, int y2, float z2, float w2, float u2, float v2, 
    upng_t* texture
);

#endif

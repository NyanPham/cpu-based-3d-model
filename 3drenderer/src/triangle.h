#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdint.h>
#include "vector.h"
#include "texture.h"

typedef struct {
    int a;
    int b;
    int c;
    tex2_t a_uv; 
    tex2_t b_uv; 
    tex2_t c_uv; 
    uint32_t color;
} face_t;

typedef struct {
    vec2_t points[3];
    tex2_t texcoords[3];
    uint32_t color;
    float avg_depth;
} triangle_t;

void draw_triangle(triangle_t *triangle, uint32_t color);
void draw_filled_triangle(triangle_t* t, uint32_t color);
void draw_texel(
        int x, int y, uint32_t* texture, 
        vec2_t point_a, vec2_t point_b, vec2_t pointc,
        float u0, float v0, float u1, float v1, float u2, float v2
);
void draw_textured_triangle(triangle_t* t, uint32_t* texture);

#endif

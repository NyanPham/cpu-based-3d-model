/**
 * @file clipping.h
 * @brief Handles Frustum Culling and 3D Polygon Clipping.
 *
 * This module implements polygon clipping (often using concepts from the 
 * Sutherland-Hodgman algorithm). Before we project 3D triangles onto the 2D screen, 
 * we must ensure they are entirely inside the camera's viewing volume (the "Frustum").
 * 
 * - If a triangle is completely outside the frustum, it is discarded (Culling).
 * - If it is partially inside, it must be clipped against the 6 planes of the frustum 
 *   (Left, Right, Top, Bottom, Near, Far). 
 * 
 * Clipping a triangle against a plane can chop it into a quadrilateral or a more 
 * complex polygon. We store these intermediate shapes in a `polygon_t` (which can 
 * hold up to 10 vertices) and later break them back down into smaller triangles.
 */

#ifndef CLIPPING_H
#define CLIPPING_H

#include "vector.h"
#include "triangle.h"

#define MAX_NUM_POLY_VERTICES 10
#define MAX_NUM_POLY_TRIANGLES (MAX_NUM_POLY_VERTICES - 2)

enum {
    LEFT_FRUSTUM_PLANE,
    RIGHT_FRUSTUM_PLANE,
    TOP_FRUSTUM_PLANE,
    BOTTOM_FRUSTUM_PLANE,
    NEAR_FRUSTUM_PLANE,
    FAR_FRUSTUM_PLANE,
};

typedef struct {
    vec3_t point;
    vec3_t normal;
} plane_t;

typedef struct {
    vec3_t vertices[MAX_NUM_POLY_VERTICES];
    tex2_t texcoords[MAX_NUM_POLY_VERTICES];
    int num_vertices;
} polygon_t;

void init_frustum_planes(float fovx, float fovy, float z_near, float z_far);
polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2, tex2_t t0, tex2_t t1, tex2_t t2);
void triangles_from_polygon(polygon_t* polygon, triangle_t triangles[], int* num_triangles);
void clip_polygon(polygon_t* polygon);


#endif

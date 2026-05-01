#ifndef VECTOR_H
#define VECTOR_H

/**
 * @file vector.h
 * @brief Foundation, mathematical structures and operations for vectors and vector operations.
 * 
 * Vectors are to represent points and directions in space. 
 * This file provides the definitions for these structures and the declarations for 
 * basic vector mathematics operations such as addition, dot products, and cross products.
 */

/**
 * @struct vec2_t
 * @brief Represents a 2D vector or point (x, y).
 * Typically used for 2D screen coordinates, texture mapping (U, V), or 2D velocity.
 */
typedef struct {
    float x;
    float y;
} vec2_t;

/**
 * @struct vec3_t
 * @brief Represents a 3D vector or point (x, y, z).
 * The core mathematical unit for 3D space, used to define vertices, normals, and 
 * transformations in the world space.
 */
typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

/**
 * @struct vec4_t
 * @brief Represents a 4D homogeneous vector (x, y, z, w).
 * Normally, a 3x3 matrix can only rotate and resize a 3D object, but it cannot move it around. 
 * By adding a 4th coordinate 'w', we can use a 4x4 matrix to do all three: move, rotate, and resize. 
 * Later, this 'w' value helps us calculate how much smaller an object should look as 
 * it gets further away from the camera.
 */
typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4_t;

// ============================================================================
// Vector 2D Functions
// ============================================================================

vec2_t vec2_new(float x, float y);
float vec2_length(vec2_t v);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);
vec2_t vec2_mul(vec2_t v, float factor);
vec2_t vec2_div(vec2_t v, float factor);
float vec2_dot(vec2_t a, vec2_t b);
void vec2_normalize(vec2_t* v);

// ============================================================================
// Vector 3D Functions
// ============================================================================

vec3_t vec3_new(float x, float y, float z);
float vec3_length(vec3_t v);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_mul(vec3_t v, float factor);
vec3_t vec3_div(vec3_t v, float factor);

/**
 * Computes the Cross Product of two 3D vectors.
 * Returns a new vector that is perpendicular to both input vectors.
 * Crucial for finding the normal (facing direction) of a polygon/triangle.
 */
vec3_t vec3_cross(vec3_t a, vec3_t b);

/**
 * Computes the Dot Product of two 3D vectors.
 * Returns a scalar value representing the relationship between the two input vectors' directions.
 * Used heavily in lighting (angle between light ray and surface normal) and back-face culling.
 */
float vec3_dot(vec3_t a, vec3_t b);

/**
 * Normalizes a vector (modifies it in-place), making its length exactly 1.0.
 * A normalized vector is often called a "unit vector" and is purely used to represent direction.
 */
void vec3_normalize(vec3_t* v);

vec3_t vec3_rotate_x(vec3_t v, float angle);
vec3_t vec3_rotate_y(vec3_t v, float angle);
vec3_t vec3_rotate_z(vec3_t v, float angle);

vec3_t vec3_clone(vec3_t* v);

// ============================================================================
// Vector Conversion Functions
// ============================================================================

/**
 * Converts a 3D vector into a 4D homogeneous vector by appending a w-component of 1.0.
 * This is necessary before multiplying the vector by a 4x4 transformation matrix.
 */
vec4_t vec4_from_vec3(vec3_t v);
vec3_t vec3_from_vec4(vec4_t v);
vec2_t vec2_from_vec4(vec4_t v);

#endif

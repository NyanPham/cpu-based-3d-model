#include <math.h>
#include "vector.h"

// ============================================================================
// Implementations of Vector 2D Functions
// ============================================================================

vec2_t vec2_new(float x, float y) {
    vec2_t res = { x, y };
    return res;
}

/**
 * Calculates the magnitude of a 2D vector using the Pythagorean theorem.
 * Formula: ||v|| = sqrt(x^2 + y^2)
 */
float vec2_length(vec2_t v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

vec2_t vec2_add(vec2_t a, vec2_t b) {
    vec2_t res = {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
    return res;
}

vec2_t vec2_sub(vec2_t a, vec2_t b) {
    vec2_t res = {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
    return res;
}

vec2_t vec2_mul(vec2_t v, float factor) {
    vec2_t res = {
        .x = v.x * factor,
        .y = v.y * factor
    };
    return res;
}

vec2_t vec2_div(vec2_t v, float factor) {
    vec2_t res = {
        .x = v.x / factor,
        .y = v.y / factor
    };
    return res;
}

float vec2_dot(vec2_t a, vec2_t b) {
    return (a.x * b.x) + (a.y * b.y);
}

void vec2_normalize(vec2_t* v) {
    float len = vec2_length(*v);
    v->x /= len;
    v->y /= len;
}

// ============================================================================
// Implementations of Vector 3D Functions
// ============================================================================

vec3_t vec3_new(float x, float y, float z) {
    vec3_t res = { x, y, z };
    return res;
}

/**
 * Calculates the magnitude (length) of a 3D vector.
 * Formula: ||v|| = sqrt(x^2 + y^2 + z^2)
 */
float vec3_length(vec3_t v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_add(vec3_t a, vec3_t b) {
    vec3_t res = {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
    return res;
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
    vec3_t res = {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
    return res;
}

vec3_t vec3_mul(vec3_t v, float factor) {
    vec3_t res = {
        .x = v.x * factor,
        .y = v.y * factor,
        .z = v.z * factor
    };
    return res;
}

vec3_t vec3_div(vec3_t v, float factor) {
    vec3_t res = {
        .x = v.x / factor,
        .y = v.y / factor,
        .z = v.z / factor
    };
    return res;
}

/**
 * Cross Product of two vectors 'a' and 'b'.
 * Returns a new vector that is perpendicular to the plane containing 'a' and 'b'.
 * The direction is determined by the Right-Hand Rule.
 *
 * The formula is derived from the determinant of a 3x3 matrix:
 * 
 *   | i    j    k  |
 *   | a.x  a.y  a.z |
 *   | b.x  b.y  b.z |
 *
 * Expanding along the first row:
 *   i(a.y*b.z - a.z*b.y) - j(a.x*b.z - a.z*b.x) + k(a.x*b.y - a.y*b.x)
 *
 * Which gives:
 *   x = a.y * b.z - a.z * b.y
 *   y = a.z * b.x - a.x * b.z  (the minus sign is absorbed into j)
 *   z = a.x * b.y - a.y * b.x
 *
 * Intuitive understanding:
 * 1. Geometric Meaning: While Dot Product measures how "parallel" two vectors are,
 *    Cross Product measures how "perpendicular" they are. The length of the resulting
 *    vector equals the area of the parallelogram formed by 'a' and 'b'.
 * 2. Graphics Application: Its most vital use is finding the Surface Normal of a triangle.
 *    If you take two edge vectors of a triangle, their cross product is a vector pointing
 *    straight out of the surface. This tells the renderer exactly which way the polygon
 *    is facing (crucial for Lighting and Back-face Culling).
 */
vec3_t vec3_cross(vec3_t a, vec3_t b) {
    vec3_t res = {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x
    };
    return res;
}

/**
 * Dot Product of two vectors 'a' and 'b'.
 * Math: A · B = ||A|| * ||B|| * cos(theta).
 * If a and b are unit vectors, the dot product is exactly the cosine of the angle between them.
 * Useful for finding if two vectors are pointing in the same direction ( > 0 ) or opposite ( < 0 ).
 *
 * The component formula comes from projecting vectors onto coordinate axes.
 * Using the law of cosines: ||a - b||² = ||a||² + ||b||² - 2||a||||b||cos(θ)
 * Expanding the left side: (a-b)·(a-b) = a·a + b·b - 2a·b
 * Substituting magnitudes: (a.x-b.x)² + (a.y-b.y)² + (a.z-b.z)² = a.x²+a.y²+a.z² + b.x²+b.y²+b.z² - 2(a·b)
 * Simplifying: -2(a.x * b.x + a.y * b.y + a.z * b.z) = -2(a·b)
 * Therefore: a·b = a.x * b.x + a.y * b.y + a.z * b.z
 *
 * Intuitive understanding:
 * 1. Coordinate Formula: Breaking vectors into independent axes (X, Y, Z). 
 *    Orthogonal axes (e.g., X and Y) have a projection of 0 and cancel out.
 *    Only parallel components (X with X, Y with Y) affect each other, so they are multiplied and summed.
 * 2. Why MULTIPLY instead of ADD? The Dot Product measures "Combined Effect" or "Synergy".
 *    Like Work in Physics (Work = Force * Distance), the values are multiplied to represent amplification.
 *    If we used addition, a force of 0 could still produce work if distance > 0, which is a paradox.
 *    Multiplication ensures that if one contribution is 0, the combined synergy is 0.
 */
float vec3_dot(vec3_t a, vec3_t b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

void vec3_normalize(vec3_t* v) {
    float len = vec3_length(*v);
    v->x /= len;
    v->y /= len;
    v->z /= len;
}

// ============================================================================
// 3D Rotation Functions
// ============================================================================
/**
 * 3D Vector Rotation Intuition:
 * Rotating a 3D vector around an axis is essentially a 2D rotation taking place in 
 * the plane perpendicular to that axis. The coordinate along the axis of rotation 
 * remains unchanged.
 *
 * These operations are derived by multiplying the vector [x, y, z] by 3x3 rotation matrices:
 *
 * Rotation around X-axis (Rx):
 * | 1     0        0    | | x |   | x                   |
 * | 0   cos(θ)  -sin(θ) | | y | = | y*cos(θ) - z*sin(θ) |
 * | 0   sin(θ)   cos(θ) | | z |   | y*sin(θ) + z*cos(θ) |
 *
 * Rotation around Y-axis (Ry):
 * | cos(θ)   0  -sin(θ) | | x |   | x*cos(θ) - z*sin(θ) |
 * |   0      1     0    | | y | = | y                   |
 * | sin(θ)   0   cos(θ) | | z |   | x*sin(θ) + z*cos(θ) |
 *
 * Rotation around Z-axis (Rz):
 * | cos(θ)  -sin(θ)   0 | | x |   | x*cos(θ) - y*sin(θ) |
 * | sin(θ)   cos(θ)   0 | | y | = | x*sin(θ) + y*cos(θ) |
 * |   0        0      1 | | z |   | z                   |
 */

/**
 * Rotates a 3D vector around the X-axis by a given angle (in radians).
 */
vec3_t vec3_rotate_x(vec3_t v, float angle) {
    vec3_t rotated_vector = {
        .x = v.x,
        .y = v.y * cos(angle) - v.z * sin(angle),
        .z = v.y * sin(angle) + v.z * cos(angle)
    };

    return rotated_vector;
}

/**
 * Rotates a 3D vector around the Y-axis by a given angle (in radians).
 */
vec3_t vec3_rotate_y(vec3_t v, float angle) {
    vec3_t rotated_vector = {
        .x = v.x * cos(angle) - v.z * sin(angle),
        .y = v.y,
        .z = v.x * sin(angle) + v.z * cos(angle)
    };

    return rotated_vector;
}

/**
 * Rotates a 3D vector around the Z-axis by a given angle (in radians).
 */
vec3_t vec3_rotate_z(vec3_t v, float angle) {
    vec3_t rotated_vector = {
        .x = v.x * cos(angle) - v.y * sin(angle),
        .y = v.x * sin(angle) + v.y * cos(angle),
        .z = v.z
    };

    return rotated_vector;
}

vec3_t vec3_clone(vec3_t* v) {
    vec3_t res = {
        .x = v->x,
        .y = v->y,
        .z = v->z,
    };

    return res;
}

// ============================================================================
// Vector Conversion Functions
// ============================================================================

vec4_t vec4_from_vec3(vec3_t v) {
    vec4_t res = {
        .x = v.x,
        .y = v.y,
        .z = v.z,
        .w = 1.0,
    };
    return res;
}

vec3_t vec3_from_vec4(vec4_t v) {
    vec3_t res = {
        .x = v.x,
        .y = v.y,
        .z = v.z,
    };
    return res;
}

vec2_t vec2_from_vec4(vec4_t v) {
    vec2_t res = {
        .x = v.x,
        .y = v.y,
    };
    return res;
}

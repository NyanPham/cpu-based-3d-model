#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

/**
 * @file matrix.h
 * @brief Definitions and operations for 4x4 transformation matrices.
 *
 * In 3D graphics, transformations (Translation, Rotation, Scaling) and projections 
 * (Perspective) are applied to 3D models using 4x4 matrices. When multiplied together, 
 * multiple transformations can be combined into a single matrix, making the Graphics 
 * Pipeline incredibly fast and efficient.
 */

/**
 * @struct mat4_t
 * @brief Represents a 4x4 matrix, primarily used for transforming 3D coordinates.
 * We use 4x4 matrices so we can multiply them with 4D homogeneous vectors (vec4_t).
 * This allows us to perform Translation (movement) using matrix multiplication, 
 * which is otherwise impossible with just a 3x3 matrix.
 */
typedef struct {
    float m[4][4];
} mat4_t;

// ============================================================================
// Basic Matrix Transformations
// ============================================================================

/**
 * Creates an Identity Matrix.
 * Multiplying any vector/matrix by the identity matrix leaves it unchanged.
 * It serves as the starting point for building transformation matrices.
 */
mat4_t mat4_identity(void);

/**
 * Creates a Scale Matrix.
 * Multiplies the x, y, and z coordinates by sx, sy, and sz respectively,
 * causing the object to grow or shrink along the axes.
 */
mat4_t mat4_make_scale(float sx, float sy, float sz);

/**
 * Creates a Translation Matrix.
 * Moves an object in 3D space by adding tx, ty, and tz to its coordinates.
 * This is where the 4th column (homogeneous coordinates) does its magic.
 */
mat4_t mat4_make_translation(float tx, float ty, float tz);

/**
 * Creates Rotation Matrices around the X, Y, and Z axes.
 * These are built using the trigonometric formulas (sine and cosine).
 */
mat4_t mat4_make_rotation_x(float angle);
mat4_t mat4_make_rotation_y(float angle);
mat4_t mat4_make_rotation_z(float angle);

// ============================================================================
// Matrix Math Operations
// ============================================================================

/**
 * Multiplies a 4x4 matrix by a 4D vector.
 * This is the core operation for transforming a vertex in 3D space.
 * Example: `transformed_vector = World_Matrix * original_vector`
 */
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v);

/**
 * Multiplies two 4x4 matrices together.
 * Used to combine multiple transformations (e.g., Scale * Rotation * Translation)
 * into a single "World Matrix". Matrix multiplication is NOT commutative (A*B != B*A).
 */
mat4_t mat4_mul_mat4(mat4_t a, mat4_t b);

// ============================================================================
// Camera & Projection Matrices
// ============================================================================

/**
 * Creates a Perspective Projection Matrix.
 * This matrix transforms 3D View Space coordinates into 2D Screen Space by warping
 * the space to create a "frustum" (a pyramid with the top chopped off).
 * It calculates the 'w' component based on the Z-depth to prepare for Perspective Divide.
 */
mat4_t mat4_make_perspective(float fov, float aspect, float znear, float zfar);

/**
 * Multiplies a projection matrix by a vector, and immediately applies the Perspective Divide.
 * Perspective Divide (dividing x, y, z by w) creates the illusion of depth by making
 * objects further away appear smaller.
 */
vec4_t mat4_mul_vec4_project(mat4_t mat_proj, vec4_t v);

/**
 * Creates a "Look-At" Matrix (often called the View Matrix).
 * This matrix transforms vertices from World Space into Camera Space.
 * It simulates moving a camera by moving the entire world in the opposite direction.
 * It requires the camera's position (eye), what it's looking at (target), and which way is up.
 */
mat4_t mat4_look_at(vec3_t eye, vec3_t target, vec3_t up);

#endif

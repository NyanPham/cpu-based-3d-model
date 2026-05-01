#include <math.h>
#include "matrix.h"

// ============================================================================
// Basic Matrix Transformations
// ============================================================================

/**
 * Identity Matrix:
 * The diagonal is all 1s, and everything else is 0s.
 */
mat4_t mat4_identity() {
    // | 1  0  0  0 |
    // | 0  1  0  0 |
    // | 0  0  1  0 |
    // | 0  0  0  1 |
    mat4_t m = {{
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 },
    }};

    return m;
}

/**
 * Scale Matrix:
 * We replace the 1s on the diagonal with our scale factors (sx, sy, sz).
 * When multiplied by a vector [x, y, z, 1], the result is [x*sx, y*sy, z*sz, 1].
 */
mat4_t mat4_make_scale(float sx, float sy, float sz) {
    // | sx   0   0   0 |
    // |  0  sy   0   0 |
    // |  0   0  sz   0 |
    // |  0   0   0   1 |
    mat4_t m = mat4_identity();
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;

    return m;
}

/**
 * Translation Matrix:
 * We place our translation values (tx, ty, tz) in the 4th column.
 * When multiplied by [x, y, z, 1], the '1' at the end multiplies with the 4th column,
 * adding tx, ty, tz to the final result: [x + tx, y + ty, z + tz, 1].
 */
mat4_t mat4_make_translation(float tx, float ty, float tz) {
    // |  1   0   0  tx |
    // |  0   1   0  ty |
    // |  0   0   1  tz |
    // |  0   0   0   1 |
    mat4_t m = mat4_identity();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    
    return m;
}

/**
 * Rotation Matrix X:
 * Rotates points around the X axis.
 */
mat4_t mat4_make_rotation_x(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    // |  1   0   0   0 |
    // |  0   c  -s   0 |
    // |  0   s   c   0 |
    // |  0   0   0   1 |
    mat4_t m = mat4_identity();
    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;

    return m;
}   

/**
 * Rotation Matrix Y:
 * Rotates points around the Y axis.
 */
mat4_t mat4_make_rotation_y(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    // |  c   0   s   0 |
    // |  0   1   0   0 |
    // | -s   0   c   0 |
    // |  0   0   0   1 |
    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;

    return m;
}

/**
 * Rotation Matrix Z:
 * Rotates points around the Z axis.
 */
mat4_t mat4_make_rotation_z(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    // |  c  -s   0   0 |
    // |  s   c   0   0 |
    // |  0   0   1   0 |
    // |  0   0   0   1 |
    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;

    return m;
}

// ============================================================================
// Matrix Math Operations
// ============================================================================

/**
 * Matrix-Vector Multiplication.
 * Calculates the dot product of each matrix row with the column vector.
 */
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
    vec4_t res;
    res.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
    res.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
    res.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
    res.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;

    return res;
}

/**
 * Matrix-Matrix Multiplication.
 * Combines two transformations into one. Remember that A*B is not the same as B*A.
 * Standard transform order for objects is: Scale * Rotation * Translation.
 */
mat4_t mat4_mul_mat4(mat4_t a, mat4_t b) {
    mat4_t m;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];

        }
    }

    return m;
}

// ============================================================================
// Camera & Projection Matrices
// ============================================================================

/**
 * Perspective Projection Matrix:
 * Maps coordinates inside the viewing frustum (pyramid) to Normalized Device Coordinates (NDC).
 * 
 * - fov: Field of View (angle indicating how wide the camera sees).
 * - aspect: Aspect Ratio (Screen Width / Screen Height).
 * - znear: Distance to the near clipping plane.
 * - zfar: Distance to the far clipping plane.
 *
 * Notice the [3][2] element is 1.0. This copies the original Z-value into the W-component 
 * of the vector during multiplication, saving it so we can perform the Perspective Divide later.
 */
mat4_t mat4_make_perspective(float fov, float aspect, float znear, float zfar) {
    // | aspect*(1/tan(fov/2))             0               0                 0 |
    // |                     0  1/tan(fov/2)               0                 0 |
    // |                     0             0      zf/(zf-zn)  (-zf*zn)/(zf-zn) |
    // |                     0             0               1                 0 |
    mat4_t m = {{{ 0 }}};
    m.m[0][0] = aspect * (1 / tan(fov / 2));
    m.m[1][1] = 1 / tan(fov / 2);
    m.m[2][2] = zfar / (zfar - znear);
    m.m[2][3] = (-zfar * znear) / (zfar - znear);
    m.m[3][2] = 1.0;
    
    return m;
}

/**
 * Multiply Projection Matrix & Perspective Divide:
 * This function applies the projection matrix, and then immediately performs
 * the critical Perspective Divide.
 * 
 * By dividing X and Y by W (which stores the original Z depth), objects further 
 * away from the camera are mapped to smaller X,Y screen coordinates!
 */
vec4_t mat4_mul_vec4_project(mat4_t mat_proj, vec4_t v) {
    // multiply the projection matrix by our original vector
    vec4_t res = mat4_mul_vec4(mat_proj, v);

    // perform perspective divide with original z-value stored in w
    if (res.w != 0.0) {
        res.x /= res.w;
        res.y /= res.w;
        res.z /= res.w;
    }
    
    return res;
}

/**
 * Look-At Matrix (View Matrix):
 * Transforms world coordinates into coordinates relative to the camera.
 *
 * TEXTBOOK EXPLANATION: WHY DO WE MOVE THE WORLD?
 * In 3D graphics, a "Camera" doesn't actually exist. The screen is always fixed at 
 * the center of the universe (0,0,0) looking down the Z-axis. 
 * Therefore, to simulate a camera moving 5 units FORWARD and turning RIGHT, we must 
 * grab the ENTIRE WORLD and pull it 5 units BACKWARD and rotate it LEFT.
 *
 * HOW WE GOT THIS FORMULA (The Derivation):
 * The View Matrix (V) is the combination of two INVERSE transformations:
 * 1. Inverse Translation (T): If the camera is at 'eye', we must move the entire 
 *    world by '-eye' to bring the camera back to the origin (0,0,0).
 * 2. Inverse Rotation (R): We must rotate the world in the opposite direction 
 *    of the camera's orientation. For an orthogonal rotation matrix, its INVERSE 
 *    is simply its TRANSPOSE. This is why the camera's axes (Right, Up, Forward) 
 *    are placed as the ROWS of the matrix rather than the columns!
 * 
 * V = R_inverse * T_inverse
 * 
 * T_inverse = | 1  0  0 -eye.x |   R_inverse = | x.x  x.y  x.z  0 |
 *             | 0  1  0 -eye.y |               | y.x  y.y  y.z  0 |
 *             | 0  0  1 -eye.z |               | z.x  z.y  z.z  0 |
 *             | 0  0  0    1   |               |  0    0    0   1 |
 * 
 * If you multiply R_inverse * T_inverse, the 3x3 rotation block remains the same, 
 * but the 4th column becomes the dot product of the camera's axes (the rows of R) 
 * and the negative eye position (the 4th column of T)!
 * 
 * For example, the top right element becomes: (x.x * -eye.x) + (x.y * -eye.y) + (x.z * -eye.z)
 * This is exactly: -dot(x, eye).
 *
 * It is built using the Dot Product and Cross Product:
 * - Forward (z): The direction from the camera 'eye' to the 'target'.
 * - Right (x): Cross product of the 'up' vector and Forward.
 * - Up (y): Cross product of Forward and Right to ensure perfect orthogonality.
 */
mat4_t mat4_look_at(vec3_t eye, vec3_t target, vec3_t up) {
    // compute the forward (z), right (x) and up (y) vectors
    vec3_t z = vec3_sub(target, eye);
    vec3_normalize(&z);
    vec3_t x = vec3_cross(up, z);
    vec3_normalize(&x);
    vec3_t y = vec3_cross(z, x);

    // | x.x  x.y  x.z  -dot(x,eye) |
    // | y.x  y.y  y.z  -dot(y,eye) |
    // | z.x  z.y  z.z  -dot(z,eye) |
    // |   0    0    0            1 |
    mat4_t view_matrix = {{
        { x.x, x.y, x.z, -vec3_dot(x, eye) },
        { y.x, y.y, y.z, -vec3_dot(y, eye) },
        { z.x, z.y, z.z, -vec3_dot(z, eye) },
        {   0,   0,   0,                1  }
    }};

    return view_matrix;
}

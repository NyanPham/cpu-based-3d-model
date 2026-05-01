#include "camera.h"
#include "matrix.h"

static camera_t camera;

void init_camera(vec3_t position, vec3_t direction, vec3_t forward_velocity, float yaw, float pitch) {
    camera.position = position;
    camera.direction = direction;
    camera.forward_velocity = forward_velocity;
    camera.yaw = yaw;
    camera.pitch = pitch;
}

/**
 * Calculates the exact 3D point in world space that the camera is currently looking at.
 *
 * TEXTBOOK EXPLANATION: YAW vs PITCH ORDER
 * =========================================
 * In 3D graphics, rotation order matters! Yaw * Pitch ≠ Pitch * Yaw.
 * 
 * We apply Yaw (Y-axis rotation) FIRST, then Pitch (X-axis rotation) SECOND.
 * Why this specific order?
 * 
 * 1. Yaw represents looking LEFT/RIGHT (turning your head side to side).
 *    When you yaw, your local X (right) and Z (forward) axes rotate, but your 
 *    Y (up) axis stays pointing at the world's "up". This is the primary rotation.
 * 
 * 2. Pitch represents looking UP/DOWN (tilting your head up or down).
 *    After yawing, when you pitch, you're rotating around your NEW local X-axis.
 *    This mimics how a real camera or human head works - you turn to face a 
 *    direction first, then tilt up or down from that orientation.
 * 
 * 3. If we did Pitch FIRST, then Yaw, the result would be different because
 *    the Yaw rotation would be applied to an already-tilted coordinate system.
 *    This can cause unexpected "roll" effects (the camera twisting sideways).
 *
 * THE MATH:
 * =========
 * 1. Start with a default target looking straight down the positive Z-axis (0, 0, 1).
 *    This represents "forward" in our coordinate system.
 * 2. Create rotation matrices for the camera's Yaw (Y-axis rotation) and Pitch (X-axis rotation).
 * 3. Multiply these matrices together: camera_rotation = Yaw * Pitch
 *    (Matrix multiplication is NOT commutative - order matters!)
 * 4. Apply this rotation to the default target vector to get the camera's true direction vector.
 * 5. Finally, add the camera's current position to this direction vector to find the absolute
 *    coordinate in world space that the camera is focused on.
 *
 * NOTE ON EULER ANGLES & QUATERNIONS:
 * ====================================
 * This simple Yaw + Pitch system is called "Euler Angles". It's intuitive but has a flaw:
 * when pitch reaches ±90° (looking straight up or down), the yaw and pitch axes can become
 * aligned, causing "gimbal lock" where you lose one degree of freedom.
 * 
 * Modern game engines avoid this using Quaternions. Instead of rotating around 3 separate
 * axes sequentially, Quaternions rotate points on a 4D hypersphere all at once. This
 * mathematical trick avoids the sequential axis dependency that causes gimbal lock.
 * For this educational renderer, Euler angles are sufficient, but real engines use
 * quaternions for smooth, lock-free rotation in all directions.
 */
vec3_t get_camera_lookat_target() {
    // initialize the target
    vec3_t target = { 0, 0, 1 };
    mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
    mat4_t camera_pitch_rotation = mat4_make_rotation_x(camera.pitch);

    mat4_t camera_rotation = mat4_identity();
    camera_rotation = mat4_mul_mat4(camera_rotation, camera_yaw_rotation);
    camera_rotation = mat4_mul_mat4(camera_rotation, camera_pitch_rotation);
    
    camera.direction = vec3_from_vec4(mat4_mul_vec4(camera_rotation, vec4_from_vec3(target)));
    
    // offset the camera position in the direction 
    target = vec3_add(camera.position, camera.direction);
    return target;
}

vec3_t get_camera_position() {
    return camera.position;
}

void set_camera_position(vec3_t position) {
    camera.position = position; 
}


vec3_t get_camera_direction() {
    return camera.direction;
}

void set_camera_direction(vec3_t direction) {
    camera.direction = direction; 
}

vec3_t get_camera_forward_velocity() {
    return camera.forward_velocity; 
}

void set_camera_forward_velocity(vec3_t forward_velocity) {
    camera.forward_velocity = forward_velocity; 
}

float get_camera_yaw() {
    return camera.yaw;
}

void set_camera_yaw(float yaw) {
    camera.yaw = yaw;
}

float get_camera_pitch() {
    return camera.pitch;
}

void set_camera_pitch(float pitch) {
    camera.pitch = pitch;
}

void add_camera_position_x(float x) {
    camera.position.x += x;
}

void add_camera_position_y(float y) {
    camera.position.y += y;
}

void add_camera_position_z(float z) {
    camera.position.z += z;
}

void add_camera_yaw(float yaw) {
    camera.yaw += yaw;
}

void add_camera_pitch(float pitch) {
    camera.pitch += pitch;
}

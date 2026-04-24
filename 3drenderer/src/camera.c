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

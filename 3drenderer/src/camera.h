#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

/**
 * @file camera.h
 * @brief Represents the virtual camera (the "eye") in our 3D world.
 *
 * In 3D graphics, we don't actually move the camera around the world.
 * Instead, we mathematically move the entire world in the opposite direction
 * to simulate the perspective of a moving camera. This struct holds the
 * state necessary to construct the "View Matrix".
 */
typedef struct {
    vec3_t position;         // The (x, y, z) location of the camera in world space
    vec3_t direction;        // The normalized vector indicating where the camera is looking
    vec3_t forward_velocity; // How fast the camera is moving forward/backward
    float yaw;               // Left/Right rotation angle (like shaking your head "no")
    float pitch;             // Up/Down rotation angle (like nodding your head "yes")
} camera_t;

/**
 * Initializes the camera with default values.
 */
void init_camera(vec3_t position, vec3_t direction, vec3_t forward_velocity, float yaw, float pitch);

/**
 * Calculates the exact 3D point in world space that the camera is currently looking at.
 * This is computed by applying Yaw and Pitch rotations to a default forward vector (0,0,1),
 * and then adding the camera's current position to it.
 * This target point is essential for building the LookAt (View) matrix.
 */
vec3_t get_camera_lookat_target(void);

vec3_t get_camera_position(void);
void set_camera_position(vec3_t position);

vec3_t get_camera_direction(void);
void set_camera_direction(vec3_t direction);

vec3_t get_camera_forward_velocity(void);
void set_camera_forward_velocity(vec3_t forward_velocity);

float get_camera_yaw(void);
void set_camera_yaw(float yaw);

float get_camera_pitch(void);
void set_camera_pitch(float pitch);

void add_camera_position_x(float x);
void add_camera_position_y(float y);
void add_camera_position_z(float z);
void add_camera_yaw(float yaw);
void add_camera_pitch(float pitch);

#endif

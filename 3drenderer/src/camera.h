#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

typedef struct {
    vec3_t position;
    vec3_t direction;
    vec3_t forward_velocity;
    float yaw;
    float pitch;
} camera_t;

void init_camera(vec3_t position, vec3_t direction, vec3_t forward_velocity, float yaw, float pitch);
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

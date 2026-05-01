#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>
#include "vector.h"

/**
 * @file light.h
 * @brief Handles basic directional lighting and shading in the 3D scene.
 *
 * Defines a global directional light (like the sun) where all light 
 * rays are parallel and come from a single direction.
 * 
 * We use this direction to calculate Flat Shading. By comparing the angle 
 * (using the Dot Product) between the light's direction and a triangle's 
 * Surface Normal, we can determine how intensely the light hits that surface.
 */
typedef struct {
    vec3_t direction; // The direction the light is pointing TOWARDS
} light_t;

/**
 * Initializes the global directional light.
 */
void init_light(vec3_t direction);

/**
 * Returns the current direction of the global light.
 */
vec3_t get_light_direction(void);

/**
 * Applies a lighting intensity factor to a 32-bit ARGB color.
 * The intensity is a percentage (0.0 to 1.0) usually derived from the 
 * dot product of the surface normal and the inverted light direction.
 */
uint32_t light_apply_intensity(uint32_t org_color, float percent_factor);

#endif

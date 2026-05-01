#ifndef TEXTURE_H
#define TEXTURE_H 

#include <stdint.h>
#include "upng.h"

/**
 * Defines a 2D texture coordinate (U, V).
 * UV mapping is the 3D modeling process of projecting a 2D image to a 3D model's surface.
 * - U represents the horizontal axis (X in 2D).
 * - V represents the vertical axis (Y in 2D).
 * Coordinates usually range from 0.0 to 1.0.
 */
typedef struct {
    float u;
    float v;
} tex2_t;

tex2_t tex2_clone(tex2_t* t);

#endif 

#include "light.h"

static light_t light;

void init_light(vec3_t direction) {
    light.direction = direction;
}

vec3_t get_light_direction() {
    return light.direction; 
}

/**
 * Alters a 32-bit ARGB color by darkening its RGB components based on a percentage.
 * 
 * TEXTBOOK EXPLANATION: HOW FLAT SHADING WORKS
 * =============================================
 * Flat shading is the simplest lighting model in 3D graphics. Instead of calculating
 * light for every pixel, we calculate it once per triangle and apply it uniformly.
 * 
 * THE PROCESS:
 * ============
 * 1. Calculate the surface normal of the triangle (using cross product of two edges).
 * 2. Calculate the dot product between the normal and the light direction.
 *    - This gives us the cosine of the angle of incidence.
 * 3. Use this value (clamped 0.0 to 1.0) as a percentage to scale the color.
 * 
 * BITWISE COLOR MANIPULATION:
 * ==========================
 * A 32-bit ARGB color is stored as: AARRGGBB (in hexadecimal)
 * - 0xFF000000 = Alpha (fully opaque)
 * - 0x00FF0000 = Red
 * - 0x0000FF00 = Green
 * - 0x000000FF = Blue
 * 
 * We use bitwise AND (&) to extract each channel, multiply by the intensity factor,
 * then use bitwise OR (|) to recombine them into a single color value.
 * 
 * WHY THIS IS FAST:
 * =================
 * - No per-pixel calculations needed
 * - Integer math only (no floating point in the shading step)
 * - Perfectly adequate for low-poly models and retro aesthetics
 * 
 * LIMITATIONS:
 * ============
 * - Looks "faceted" on smooth models (you can see each triangle's flat color)
 * - For smooth shading, you'd need Gouraud or Phong shading (per-vertex or per-pixel)
 */
uint32_t light_apply_intensity(uint32_t org_color, float percent_factor) {
    if (percent_factor < 0) percent_factor = 0;
    if (percent_factor > 1) percent_factor = 1;

    uint32_t a = (org_color & 0xFF000000);
    uint32_t r = (org_color & 0x00FF0000) * percent_factor;
    uint32_t g = (org_color & 0x0000FF00) * percent_factor;
    uint32_t b = (org_color & 0x000000FF) * percent_factor;
    
    uint32_t new_color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);

    return new_color;
}


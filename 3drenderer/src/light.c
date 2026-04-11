#include "light.h"

light_t light = {
    .direction = { 0, 0, 1 }
};

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


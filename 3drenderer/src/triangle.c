#include "triangle.h" 
#include "display.h"
#include "swap.h"

void draw_pixel_depth(
    int x, int y, uint32_t color, 
    vec4_t point_a, vec4_t point_b, vec4_t point_c
) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) return;

    vec2_t point_p = { x, y };
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    vec3_t weights = barycentric_weights(a, b, c, point_p); 
    
    float alpha = weights.x;
    float beta = weights.y;
    float gama = weights.z;

    float interpolated_reciprocal_w;

    interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gama;

    // adjust 1/w so that pixels that are closer to the camera have smaller values
    interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;
        
    // only draw the pixel of the depth value is less than the one previously stored in the z buff
    if (interpolated_reciprocal_w < z_buffer[(window_width * y) + x]) {
        draw_pixel(x, y, color);

        // update the z buffer with 1/2 of the current pixel
        z_buffer[(window_width * y) + x] = interpolated_reciprocal_w;
    }
}

void draw_filled_triangle(
    int x0, int y0, float z0, float w0, 
    int x1, int y1, float z1, float w1,
    int x2, int y2, float z2, float w2,
    uint32_t color
) {
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
    }

    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
    }

    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
    }


    vec4_t point_a = { x0, y0, z0, w0 };
    vec4_t point_b = { x1, y1, z1, w1 };
    vec4_t point_c = { x2, y2, z2, w2 };
    
    // render the upper part of the triangle (flat-bottom)
    float inv_slope1 = 0;
    float inv_slope2 = 0;
   
    if (y1 - y0 != 0) {
        inv_slope1 = (float)(x1 - x0) / abs(y1 - y0);
    }
    
    if (y2 - y0 != 0) {
        inv_slope2 = (float)(x2 - x0) / abs(y2 - y0);
    }

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; y++) {
            int x_start = x1 + (y - y1) * inv_slope1; 
            int x_end = x0 + (y - y0) * inv_slope2;
            
            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                draw_pixel_depth(x, y, color, point_a, point_b, point_c);                  
            }
        }
    }

    // render the lower part of the triangle (flat-top)
    inv_slope1 = 0;
    inv_slope2 = 0;
   
    if (y2 - y1 != 0) {
        inv_slope1 = (float)(x2 - x1) / abs(y2 - y1);
    }
    
    if (y2 - y0 != 0) {
        inv_slope2 = (float)(x2 - x0) / abs(y2 - y0);
    }

    if (y2 - y1 != 0) {
        for (int y = y1; y <= y2; y++) {
            int x_start = x1 + (y - y1) * inv_slope1; 
            int x_end = x0 + (y - y0) * inv_slope2;
            
            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                draw_pixel_depth(x, y, color, point_a, point_b, point_c);                  
            }
        }
    }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
    vec2_t ab = vec2_sub(b, a);
    vec2_t ac = vec2_sub(c, a);
    vec2_t ap = vec2_sub(p, a);
    vec2_t pc = vec2_sub(c, p);
    vec2_t pb = vec2_sub(b, p);
    
    float area_parallelogram_abc = (ac.x * ab.y) - (ac.y * ab.x);
    float alpha = ((pc.x * pb.y) - (pc.y * pb.x)) / area_parallelogram_abc;
    float beta = ((ac.x * ap.y - ac.y * ap.x)) / area_parallelogram_abc;
    float gama = 1 - alpha - beta;
    
    vec3_t weights = { alpha, beta, gama };
 
    return weights;
}

void draw_texel(
    int x, int y, uint32_t* texture, 
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) return;

    vec2_t point_p = { x, y };
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    vec3_t weights = barycentric_weights(a, b, c, point_p); 
    
    float alpha = weights.x;
    float beta = weights.y;
    float gama = weights.z;

    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    interpolated_u = (a_uv.u / point_a.w) * alpha + (b_uv.u / point_b.w) * beta + (c_uv.u / point_c.w) * gama;
    interpolated_v = (a_uv.v / point_a.w) * alpha + (b_uv.v / point_b.w) * beta + (c_uv.v / point_c.w) * gama;
    interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gama;
    
    interpolated_u /= interpolated_reciprocal_w;
    interpolated_v /= interpolated_reciprocal_w;

    int tex_x = abs((int)(interpolated_u * texture_width)) % texture_width;
    int tex_y = abs((int)(interpolated_v * texture_height)) % texture_height;

    // adjust 1/w so that pixels that are closer to the camera have smaller values
    interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;
        
    // only draw the pixel of the depth value is less than the one previously stored in the z buff
    if (interpolated_reciprocal_w < z_buffer[(window_width * y) + x]) {
        draw_pixel(x, y, texture[tex_y * texture_width + tex_x]);

        // update the z buffer with 1/2 of the current pixel
        z_buffer[(window_width * y) + x] = interpolated_reciprocal_w;
    }
}

void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0, 
    int x1, int y1, float z1, float w1, float u1, float v1, 
    int x2, int y2, float z2, float w2, float u2, float v2, 
    uint32_t* texture
) {
    // sort the vertices by the y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }

    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // flip the v component to account for the inverted UV-coords where v grows downwards
    v0 = 1.0 - v0; 
    v1 = 1.0 - v1;
    v2 = 1.0 - v2;

    // create vector points and texture coords after sorting the vertices
    vec4_t point_a = { x0, y0, z0, w0 };
    vec4_t point_b = { x1, y1, x1, w1 };
    vec4_t point_c = { x2, y2, z2, w2 };
    tex2_t a_uv = { u0, v0 };
    tex2_t b_uv = { u1, v1 };
    tex2_t c_uv = { u2, v2 };

    // render the upper part of the triangle (flat-bottom)
    float inv_slope1 = 0;
    float inv_slope2 = 0;
   
    if (y1 - y0 != 0) {
        inv_slope1 = (float)(x1 - x0) / abs(y1 - y0);
    }
    
    if (y2 - y0 != 0) {
        inv_slope2 = (float)(x2 - x0) / abs(y2 - y0);
    }

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; y++) {
            int x_start = x1 + (y - y1) * inv_slope1; 
            int x_end = x0 + (y - y0) * inv_slope2;
            
            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);                  
            }
        }
    }

    // render the lower part of the triangle (flat-top)
    inv_slope1 = 0;
    inv_slope2 = 0;
   
    if (y2 - y1 != 0) {
        inv_slope1 = (float)(x2 - x1) / abs(y2 - y1);
    }
    
    if (y2 - y0 != 0) {
        inv_slope2 = (float)(x2 - x0) / abs(y2 - y0);
    }

    if (y2 - y1 != 0) {
        for (int y = y1; y <= y2; y++) {
            int x_start = x1 + (y - y1) * inv_slope1; 
            int x_end = x0 + (y - y0) * inv_slope2;
            
            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);                  
            }
        }
    }
}

#include "triangle.h" 
#include "display.h"
#include "swap.h"

void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // find the 2 slopes (2 triangle edges)
    float inv_slope1 = (x1 - x0) / (float)(y1 - y0);
    float inv_slope2 = (x2 - x0) / (float)(y2 - y0);

    float x_start = x0;
    float x_end = x0;

    for (int y = y0; y <= y2; y++) {
        draw_line(x_start, y, x_end, y, color);
        x_start += inv_slope1;
        x_end += inv_slope2;
    }
}

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    float inv_slope1 = (x2 - x0) / (float)(y2 - y0);
    float inv_slope2 = (x2 - x1) / (float)(y2 - y1);

    float x_start = x2;
    float x_end = x2;

    for (int y = y2; y >= y0; y--) {
        draw_line(x_start, y, x_end, y, color);
        x_start -= inv_slope1;
        x_end -= inv_slope2;
    }
}

void draw_filled_triangle(triangle_t* t, uint32_t color) {
    // we need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    int x0 = t->points[0].x;
    int y0 = t->points[0].y;
    int x1 = t->points[1].x;
    int y1 = t->points[1].y;
    int x2 = t->points[2].x;
    int y2 = t->points[2].y;


    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    
    if (y1 == y2) {
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
        return;
    }

    if (y0 == y1) {
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
        return; 
    }

    // compute the new vertex (Mx, My) using triangle similarity
    int My = y1;
    int Mx = (((x2 - x0) * (y1 - y0)) / (y2 - y0)) + x0; 

    // draw flat-bottom triangle 
    fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

    // draw flat-top triangle
    fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
}

void draw_triangle(triangle_t *triangle, uint32_t color) {
    draw_line(triangle->points[0].x, triangle->points[0].y, triangle->points[1].x, triangle->points[1].y, color);
    draw_line(triangle->points[1].x, triangle->points[1].y, triangle->points[2].x, triangle->points[2].y, color);
    draw_line(triangle->points[2].x, triangle->points[2].y, triangle->points[0].x, triangle->points[0].y, color);
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
        vec2_t point_a, vec2_t point_b, vec2_t point_c,
        float u0, float v0, float u1, float v1, float u2, float v2
) {
    vec2_t point_p = { x, y };
    vec3_t weights = barycentric_weights(point_a, point_b, point_c, point_p); 
    
    float alpha = weights.x;
    float beta = weights.y;
    float gama = weights.z;

    float interpolated_u = u0 * alpha + u1 * beta + u2 * gama;
    float interpolated_v = v0 * alpha + v1 * beta + v2 * gama;

    int tex_x = abs((int)(interpolated_u * texture_width));
    int tex_y = abs((int)(interpolated_v * texture_height));
    
    if (tex_y >= texture_height || tex_x >= texture_width) return;
    draw_pixel(x, y, texture[tex_y * texture_width + tex_x]);
}

void draw_textured_triangle(triangle_t* t, uint32_t* texture) {
    int x0 = t->points[0].x;
    int y0 = t->points[0].y;
    float u0 = t->texcoords[0].u;
    float v0 = t->texcoords[0].v;

    int x1 = t->points[1].x;
    int y1 = t->points[1].y;
    float u1 = t->texcoords[1].u;
    float v1 = t->texcoords[1].v;
    
    int x2 = t->points[2].x;
    int y2 = t->points[2].y;
    float u2 = t->texcoords[2].u;
    float v2 = t->texcoords[2].v;

    // sort the vertices by the y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }

    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    
    // create vector points after sorting the vertices
    vec2_t point_a = { x0, y0 };
    vec2_t point_b = { x1, y1 };
    vec2_t point_c = { x2, y2 };

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
                draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);                  
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
                draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);                  
            }
        }
    }
}

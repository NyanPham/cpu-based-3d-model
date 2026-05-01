#include "triangle.h" 
#include "display.h"
#include "swap.h"

/**
 * Calculates the surface normal of a triangle.
 * The normal is a 3D vector perpendicular to the triangle's surface.
 * 
 * It's calculated by taking two edge vectors (AB and AC) and finding 
 * their Cross Product. The resulting vector's direction follows the 
 * Right-Hand Rule and indicates which way the polygon is facing.
 * This is crucial for Back-face Culling and Lighting.
 */
vec3_t get_triangle_normal(vec4_t vertices[3]) {
    vec3_t vertex_a = vec3_from_vec4(vertices[0]);
    vec3_t vertex_b = vec3_from_vec4(vertices[1]);
    vec3_t vertex_c = vec3_from_vec4(vertices[2]);

    vec3_t vector_ab = vec3_sub(vertex_b, vertex_a);
    vec3_t vector_ac = vec3_sub(vertex_c, vertex_a);
    vec3_normalize(&vector_ab);
    vec3_normalize(&vector_ac);

    vec3_t normal = vec3_cross(vector_ab, vector_ac);
    vec3_normalize(&normal);

    return normal;
}

/**
 * Draws a solid color pixel and updates the Z-Buffer for depth testing.
 * 
 * TEXTBOOK EXPLANATION: WHY 1/w AND THE INVERSION TRICK
 * =====================================================
 * PART 1: Why interpolate 1/w instead of w?
 * -----------------------------------------
 * When 3D points are projected onto a 2D screen (Perspective Divide), the depth (w)
 * gets distorted. Depth is no longer linear across the 2D surface of the triangle.
 * If we try to linearly interpolate 'w' directly using 2D screen coordinates, it will be incorrect.
 * However, the reciprocal of depth (1/w) IS linear in 2D screen space!
 * Therefore, we must linearly interpolate 1/w using Barycentric coordinates to get accurate depth.
 * 
 * PART 2: Why do we invert the value with (1.0 - 1/w)?
 * ----------------------------------------------------
 * Standard Z-Buffer convention: SMALLER value = CLOSER to camera (draw first)
 * Our clear_z_buffer() sets default to 1.0 (furthest possible in NDC)
 * 
 * The math:
 * - Objects CLOSE to camera have SMALL w → LARGE 1/w
 * - Objects FAR from camera have LARGE w → SMALL 1/w
 * 
 * Problem: 1/w is LARGE for close objects, but we want SMALL values for close objects!
 * Solution: Invert! 1.0 - 1/w gives us:
 *   - Close object: 1.0 - large = SMALL ✓
 *   - Far object: 1.0 - small = LARGE ✓
 * 
 * Now we can use the intuitive comparison: if (new_depth < existing_depth) draw it!
 */
void draw_pixel_depth(
    int x, int y, uint32_t color, 
    vec4_t point_a, vec4_t point_b, vec4_t point_c
) {
    if (x < 0 || x >= get_window_width() || y < 0 || y >= get_window_height()) return;

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
    if (interpolated_reciprocal_w < get_zbuffer_at(x, y)) {
        draw_pixel(x, y, color);

        // update the z buffer with 1/2 of the current pixel
        update_zbuffer_at(x, y, interpolated_reciprocal_w);
    }
}

/**
 * Rasterizes a filled solid-color triangle using the scanline algorithm.
 * 
 * TEXTBOOK EXPLANATION: WHY SCANLINE?
 * ====================================
 * Drawing an arbitrary triangle is tricky because none of its edges are guaranteed 
 * to be perfectly horizontal or vertical. The scanline algorithm solves this by 
 * exploiting a geometric insight: ANY triangle can be split into TWO right triangles:
 * 
 * 1. Flat-Bottom Triangle: The bottom edge IS horizontal (y1 == y0)
 * 2. Flat-Top Triangle: The top edge IS horizontal (y2 == y1)
 * 
 * THE ALGORITHM:
 * =============
 * Step 1: Sort vertices by Y-coordinate (y0 ≤ y1 ≤ y2)
 * Step 2: Split into two parts at the middle vertex (y1):
 *         - Top half: Flat-Bottom triangle (vertices at y0, y1, y2_top)
 *         - Bottom half: Flat-Top triangle (vertices at y0_bottom, y1, y2)
 * Step 3: For each horizontal row (scanline), calculate the X-coordinates 
 *         where the triangle's edges intersect that row.
 * Step 4: Draw horizontal lines from left_edge X to right_edge X.
 * 
 * WHY THIS WORKS:
 * ===============
 * For a flat-bottom triangle, the left and right edges are simple linear functions.
 * As we move down each row, we can calculate the edge positions using the slope:
 *     x = x_start + (y - y_start) * slope
 *     where slope = (x_end - x_start) / (y_end - y_start)
 * 
 * This is much simpler than handling arbitrary triangles directly!
 */
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

/**
 * Draws a wireframe triangle by drawing 3 lines connecting the vertices.
 * This is used for wireframe rendering mode and for drawing triangle edges
 * when rendering filled triangles with wireframe overlay.
 */
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

/**
 * Calculates the Barycentric Coordinates (alpha, beta, gamma) for a point 'p' inside a triangle (a, b, c).
 * 
 * TEXTBOOK EXPLANATION: WHAT ARE BARYCENTRIC COORDINATES?
 * =======================================================
 * Barycentric coordinates are a coordinate system for triangles where any point inside 
 * the triangle can be expressed as a weighted combination of the three vertices.
 * 
 * THE WEIGHTS (alpha, beta, gamma):
 * =================================
 * - alpha: Weight for vertex A (how much of point P is "owned" by A)
 * - beta:  Weight for vertex B
 * - gamma: Weight for vertex C
 * - alpha + beta + gamma = 1 (they always sum to 1)
 * 
 * THE 2D CROSS PRODUCT METHOD:
 * ============================
 * We use the 2D cross product (a.x * b.y - a.y * b.x) to calculate areas.
 * The cross product of two vectors gives the area of the parallelogram they form.
 * Since triangle area = parallelogram area / 2, we can use this for proportional weights.
 * 
 * THE FORMULAS:
 * ============
 * area_parallelogram_abc = (c-a) × (b-a)     [cross product in 2D]
 * alpha = area(PBC) / area(ABC) = (c-p) × (b-p) / area_parallelogram_abc
 * beta  = area(APC) / area(ABC) = (c-a) × (p-a) / area_parallelogram_abc
 * gamma = 1 - alpha - beta
 * 
 * WHY THIS MATTERS:
 * =================
 * Once we have these weights, we can interpolate ANY property across the triangle:
 * - Depth (z) values for the Z-buffer
 * - Texture coordinates (u, v) for texturing
 * - Colors for Gouraud shading
 * 
 * The weights tell us "how close" the pixel is to each vertex, proportionally.
 */
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

/**
 * Draws a single textured pixel (texel) and updates the Z-Buffer.
 * 
 * TEXTBOOK EXPLANATION: PERSPECTIVE-CORRECT TEXTURE MAPPING
 * ==========================================================
 * This is one of the most subtle but important concepts in 3D graphics!
 * 
 * THE PROBLEM:
 * ============
 * When we project a 3D triangle onto a 2D screen using perspective projection,
 * the screen coordinates are NOT linearly related to the 3D coordinates.
 * This means texture coordinates (U, V) are also NOT linear across the triangle.
 * 
 * IF WE DON'T CORRECT FOR THIS:
 * =============================
 * Textures will appear "warped" or "stretched" - especially near the edges
 * of triangles that are at an angle to the camera. This is called "perspective
 * distortion" and looks very wrong to the human eye.
 * 
 * THE SOLUTION: DIVIDE BY W
 * =========================
 * 1. At each vertex, store (u/w) and (v/w) instead of raw u, v.
 * 2. Interpolate these values linearly across the triangle using barycentric weights.
 * 3. At the pixel, divide the interpolated values by the interpolated 1/w.
 * 
 * WHY THIS WORKS:
 * ===============
 * The math proves that (u/w) and (v/w) ARE linear in screen space!
 * So we can safely interpolate them, then recover the true u and v at the end.
 * 
 * THE FORMULA:
 * ============
 * interpolated_u = (u0/w0)*α + (u1/w1)*β + (u2/w2)*γ
 * interpolated_v = (v0/w0)*α + (v1/w1)*β + (v2/w2)*γ
 * interpolated_1/w = (1/w0)*α + (1/w1)*β + (1/w2)*γ
 * 
 * final_u = interpolated_u / interpolated_1/w
 * final_v = interpolated_v / interpolated_1/w
 * 
 * This is exactly what the GPU does in modern 3D games!
 */
void draw_triangle_texel(
    int x, int y, upng_t* texture, 
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
) {
    if (x < 0 || x >= get_window_width() || y < 0 || y >= get_window_height()) return;

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

    // get the mesh texture width and height
    int texture_width = upng_get_width(texture);
    int texture_height = upng_get_height(texture);

    int tex_x = abs((int)(interpolated_u * texture_width)) % texture_width;
    int tex_y = abs((int)(interpolated_v * texture_height)) % texture_height;

    /**
     * Z-BUFFER INVERSION FOR TEXTURES:
     * ================================
     * Same trick as draw_pixel_depth: we invert 1/w so that closer pixels
     * have smaller depth values, allowing simple < comparison with the Z-buffer.
     * This ensures textured triangles are depth-tested correctly.
     */
    interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;

    // only draw the pixel of the depth value is less than the one previously stored in the z buff
    if (interpolated_reciprocal_w < get_zbuffer_at(x, y)) {
        // get the buffer of colors from the texture
        uint32_t* texture_buffer = (uint32_t*)upng_get_buffer(texture);
        
        // draw a pixel at position (x, y) with the color that comes from the mapped texture
        draw_pixel(x, y, texture_buffer[tex_y * texture_width + tex_x]);

        // update the z buffer with 1/2 of the current pixel
        update_zbuffer_at(x, y, interpolated_reciprocal_w);
    }
}

/**
 * Rasterizes a textured triangle using the scanline algorithm.
 * We split the triangle horizontally into two smaller triangles:
 * 1. A Flat-Bottom triangle (top half)
 * 2. A Flat-Top triangle (bottom half)
 * 
 * We then draw horizontal lines (scanlines) from the left edge to the right edge,
 * calculating the exact UV texture coordinate for each pixel along the way.
 */
void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0, 
    int x1, int y1, float z1, float w1, float u1, float v1, 
    int x2, int y2, float z2, float w2, float u2, float v2, 
    upng_t* texture
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
                draw_triangle_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);                  
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
                draw_triangle_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);                  
            }
        }
    }
}

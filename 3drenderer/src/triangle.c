#include "triangle.h" 
#include "display.h"

void int_swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

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

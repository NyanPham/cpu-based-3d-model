#include "clipping.h"
#include "vector.h"

#include <math.h>

#define NUM_PLANES 6

plane_t frustum_planes[NUM_PLANES];

void init_frustum_planes(float fovx, float fovy, float z_near, float z_far) {
    float cos_half_fovx = cos(fovx / 2);
    float sin_half_fovx = sin(fovx / 2);
    float cos_half_fovy = cos(fovy / 2);
    float sin_half_fovy = sin(fovy / 2);

    frustum_planes[LEFT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fovx;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fovx;
     
    frustum_planes[RIGHT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fovx;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fovx;

    frustum_planes[TOP_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fovy;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fovy;
    
    frustum_planes[BOTTOM_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fovy;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fovy;

    frustum_planes[NEAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_near);
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

    frustum_planes[FAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_far);
    frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}

polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2, tex2_t t0, tex2_t t1, tex2_t t2) {
    polygon_t polygon = {
        .vertices = { v0, v1, v2 },
        .texcoords = { t0, t1, t2 },
        .num_vertices = 3,
    };

    return polygon;
}

float float_lerp(float a, float b, float t){
    return a + t * (b - a);
}

void clip_polygon_against_plane(polygon_t* polygon, int plane) {
    vec3_t plane_point = frustum_planes[plane].point;
    vec3_t plane_normal = frustum_planes[plane].normal;

    // declare a static of inside vertices that will be part of the final polygon returned via parameter
    vec3_t inside_vertices[MAX_NUM_POLY_VERTICES];
    tex2_t inside_texcoords[MAX_NUM_POLY_VERTICES];
    int num_inside_vertices = 0;

    // start the curr vertex with the first polygon vertex and texture coord
    vec3_t* curr_vertex = &polygon->vertices[0];
    tex2_t* curr_texcoord = &polygon->texcoords[0];
    
    // start the prev vertex with the last polygon vertex and texture coord
    vec3_t* prev_vertex = &polygon->vertices[polygon->num_vertices - 1];
    tex2_t* prev_texcoord = &polygon->texcoords[polygon->num_vertices - 1];

    // calculate the dot product of the curr and prev vertices
    float curr_dot = vec3_dot(vec3_sub(*curr_vertex, plane_point), plane_normal);
    float prev_dot = vec3_dot(vec3_sub(*prev_vertex, plane_point), plane_normal);

    // loop all the polygon vertices while the current is different than the last one 
    while (curr_vertex != &polygon->vertices[polygon->num_vertices]) {
        curr_dot = vec3_dot(vec3_sub(*curr_vertex, plane_point), plane_normal);

        // if we changed from inside to outside or vice versa
        if (curr_dot * prev_dot < 0) {
            // find the interpolation factor t
            float t = prev_dot / (prev_dot - curr_dot);

            // calculate the intersection point I = Q1 + t(Q2-Q1)
            vec3_t intersection_point = {
                .x = float_lerp(prev_vertex->x, curr_vertex->x, t),
                .y = float_lerp(prev_vertex->y, curr_vertex->y, t),
                .z = float_lerp(prev_vertex->z, curr_vertex->z, t)
            };

            // use the lerp formula to get the interpolated u and v
            tex2_t interpolated_texcoord = { 
                .u = float_lerp(prev_texcoord->u, curr_texcoord->u, t),
                .v = float_lerp(prev_texcoord->v, curr_texcoord->v, t)
            };

            // insert the intersection point to the list of the inside vertices
            inside_vertices[num_inside_vertices] = vec3_clone(&intersection_point);
            inside_texcoords[num_inside_vertices] = tex2_clone(&interpolated_texcoord);
            num_inside_vertices++;
        }
    
        // curr_vertex is inside the plane
        if (curr_dot > 0) {
            // insert the curr_vertex to the list of inside vertices
            inside_vertices[num_inside_vertices] = vec3_clone(curr_vertex);
            inside_texcoords[num_inside_vertices] = tex2_clone(curr_texcoord);
            num_inside_vertices++;
        }

        prev_dot = curr_dot;
        prev_vertex = curr_vertex;
        prev_texcoord = curr_texcoord;
        curr_vertex++;
        curr_texcoord++;
    }

    // copy the list of inside vertices into the destination polygon (in, and out parameter)
    for (int i = 0; i < num_inside_vertices; i++) {
        polygon->vertices[i] = vec3_clone(&inside_vertices[i]);
        polygon->texcoords[i] = tex2_clone(&inside_texcoords[i]);
    }
    polygon->num_vertices = num_inside_vertices;
}

void triangles_from_polygon(polygon_t* polygon, triangle_t triangles[], int* num_triangles) {
   for (int i = 0; i < polygon->num_vertices - 2; i++) {
       int idx0 = 0;
       int idx1 = i + 1;
       int idx2 = i + 2;

       triangles[i].points[0] = vec4_from_vec3(polygon->vertices[idx0]);
       triangles[i].points[1] = vec4_from_vec3(polygon->vertices[idx1]);
       triangles[i].points[2] = vec4_from_vec3(polygon->vertices[idx2]);

       triangles[i].texcoords[0] = polygon->texcoords[idx0];
       triangles[i].texcoords[1] = polygon->texcoords[idx1];
       triangles[i].texcoords[2] = polygon->texcoords[idx2];
   }
   *num_triangles = polygon->num_vertices - 2;
}

void clip_polygon(polygon_t* polygon) {
    clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE); 
    clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE); 
    clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE); 
    clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE); 
    clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE); 
    clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE); 
}

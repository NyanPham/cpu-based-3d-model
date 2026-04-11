#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "triangle.h"
#include "matrix.h"
#include "light.h"

triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { 0, 0, 0 };
mat4_t proj_matrix;

uint8_t is_running = 0;
int previous_frame_time = 0;

enum cull_method {
    CULL_NONE,
    CULL_BACKFACE,
} cull_method;

enum render_method {
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
} render_method;

void setup(void) {
    // init render mode and triangle culling mode
    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

    // allocate required mem in bytes to hold the color buffer
    g_color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
    
    g_color_buffer_texture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    // init the perspective projection matrix
    float fov = 1.047197;
    float aspect = (float)window_height / (float)window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

    // load_cube_mesh_data();
    load_obj_file_data("./assets/f22.obj");
}

void process_input(void) {
    SDL_Event event;

    SDL_PollEvent(&event);
    
    switch(event.type) {
        case SDL_QUIT:
            is_running = 0;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                is_running = 0;
            } 
            if (event.key.keysym.sym == SDLK_1) {
                render_method = RENDER_WIRE_VERTEX;
            } 
            if (event.key.keysym.sym == SDLK_2) {
                render_method = RENDER_WIRE;
            }
            if (event.key.keysym.sym == SDLK_3) {
                render_method = RENDER_FILL_TRIANGLE;
            }
            if (event.key.keysym.sym == SDLK_4) {
                render_method = RENDER_FILL_TRIANGLE_WIRE; 
            }
            if (event.key.keysym.sym == SDLK_c) {
                cull_method = CULL_BACKFACE;
            } 
            if (event.key.keysym.sym == SDLK_d) {
                cull_method = CULL_NONE;
            }
            break;
        default:
            break;
    }
}

/*
// Receives a 3d vector and returns a projected 2d point
vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z,
    };

    return projected_point;
}
*/

void update(void) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    } 

    previous_frame_time = SDL_GetTicks();
    
    // initialize the array of triangles to render
    triangles_to_render = NULL;

    mesh.rotation.x += 0.005;
    mesh.rotation.y += 0.0;
    mesh.rotation.z += 0.0;
    //mesh.scale.x += 0.002;
    //mesh.scale.y += 0.001;
    //mesh.translation.x += 0.01;
    mesh.translation.z = 5;

    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t trans_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    int num_faces = array_length(mesh.faces);
     
    // loop all triangle faces of our mesh
    // TODO: need to transform the vertices outside of the loop
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];
        
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];
       
        vec4_t transformed_vertices[3];
            
        // loop all three vertices of the current face and do transformations
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            mat4_t world_matrix = mat4_identity();

            // order of transformation: scale -> rotate -> translate. [T]*[R]*[S]*v
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(trans_matrix, world_matrix);

            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            transformed_vertices[j] = transformed_vertex;
        }
   
        vec3_t vertex_a = vec3_from_vec4(transformed_vertices[0]);
        vec3_t vertex_b = vec3_from_vec4(transformed_vertices[1]);
        vec3_t vertex_c = vec3_from_vec4(transformed_vertices[2]);

        vec3_t vector_ab = vec3_sub(vertex_b, vertex_a);
        vec3_t vector_ac = vec3_sub(vertex_c, vertex_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);
    
        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalize(&normal);

        // check backface culling
        if (cull_method == CULL_BACKFACE) {
            vec3_t camera_ray = vec3_sub(camera_position, vertex_a);
            float dot_normal_camera = vec3_dot(camera_ray, normal);

            if (dot_normal_camera < 0) {
                continue;
            }
        }   

        // flat shading
        float dot_normal_light = vec3_dot(normal, light.direction); 
        uint32_t shaded_color = light_apply_intensity(mesh_face.color, -dot_normal_light);
        
        vec4_t projected_points[3];

        // loop all 3 vertices to perform projection
        for (int j = 0; j < 3; j++) {
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]); 
            
            // scale the points into the view
            projected_points[j].x *= window_width / 2.0;
            projected_points[j].y *= window_height / 2.0;
           
            // invert the y value for flipped screen y coordinate
            projected_points[j].y *= -1;

            // translate the projected points to the middle of the screen
            projected_points[j].x += (window_width / 2.0);
            projected_points[j].y += (window_height / 2.0);
        }
        
        // compute the avg_depth for each face based on vertices after transformation
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

        triangle_t projected_triangle = {
            .points = {
                { projected_points[0].x, projected_points[0].y },
                { projected_points[1].x, projected_points[1].y },
                { projected_points[2].x, projected_points[2].y },
            },
            .color = shaded_color,
            .avg_depth = avg_depth,
        };

        array_push(triangles_to_render, projected_triangle);
    }
    
    // sort the triangles to render by avg_depth
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        uint8_t swapped = 0;
        for (int j = i; j < num_triangles; j++) {
            if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth) {
                triangle_t tmp = triangles_to_render[i];
                triangles_to_render[i] = triangles_to_render[j];
                triangles_to_render[j] = tmp;
                swapped = 1;
            }
        }
        if (swapped == 0) {
            break;
        }
    }
}

void render(void) {
    SDL_RenderClear(g_renderer);

    draw_grid();
    
    // loop all projected triangles and render them
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];
        
        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_filled_triangle(&triangle, triangle.color);
        }

        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_triangle(&triangle, 0xFFFFFFFF);
        }

        if (render_method == RENDER_WIRE_VERTEX) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);        
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);        
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }

    }

    array_free(triangles_to_render);

    render_color_buffer();
    clear_color_buffer(0xFF000000);
    SDL_RenderPresent(g_renderer);
}

void free_resources(void) {
    free(g_color_buffer);
    array_free(mesh.vertices);
    array_free(mesh.faces);
}

int main(void) {
    is_running = initialize_window() == 0;
    
    setup();
        
    while (is_running) {
        process_input();
        update();
        render();
    }
    
    destroy_window();
    free_resources();
    
    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "triangle.h"

triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { .x = 0, .y = 0, .z = -5 };
float fov_factor = 640;

uint8_t is_running = 0;
int previous_frame_time = 0;

void setup(void) {
    // allocate required mem in bytes to hold the color buffer
    g_color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
    
    g_color_buffer_texture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    load_cube_mesh_data();
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
            break;
        default:
            break;
    }
}

// Receives a 3d vector and returns a projected 2d point
vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z,
    };

    return projected_point;
}

void update(void) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    } 

    previous_frame_time = SDL_GetTicks();
    
    // initialize the array of triangles to render
    triangles_to_render = NULL;

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;
    
    int num_faces = array_length(mesh.faces);
     
    // loop all triangle faces of our mesh
    // TODO: need to transform the vertices outside of the loop
    for (int i = 0; i < num_faces; i++) {
        face_t cube_face = mesh.faces[i];
        
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[cube_face.a - 1];
        face_vertices[1] = mesh.vertices[cube_face.b - 1];
        face_vertices[2] = mesh.vertices[cube_face.c - 1];
       
        triangle_t projected_triangle;

        // loop all three vertices of the current face and do transformations
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];
        
            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // trnanslate the vertex away from the camera
            transformed_vertex.z -= camera_position.z;

            vec2_t projected_point = project(transformed_vertex);
            
            // scale and translate the projected points to the middle of the screen
            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);

            projected_triangle.points[j] = projected_point;
        }
         
        array_push(triangles_to_render, projected_triangle);
    }

  }

void render(void) {
    draw_grid();
    
    // loop all projected triangles and render them
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];
        draw_triangle(&triangle, 0xFFFFFF00);
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

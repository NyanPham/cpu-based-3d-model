#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "upng.h"
#include <math.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "triangle.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "camera.h"
#include "clipping.h"

#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

uint8_t is_running = 0;
int previous_frame_time = 0;
float delta_time = 0.0;

void setup(void) {
    // init render mode and triangle culling mode
    set_render_method(RENDER_WIRE);
    set_cull_method(CULL_BACKFACE);

    // init the scene light direction
    init_light(vec3_new(0, 0, 1));

    // init the camera
    init_camera(
        vec3_new(0, 0, 0), // position
        vec3_new(0, 0, 1), // direction
        vec3_new(0, 0, 0), // forward_velocity
        0.0,               // yaw
        0.0                // pitch
    );

    // init the perspective projection matrix
    float aspecty = (float)get_window_height() / (float)get_window_width();
    float aspectx = (float)get_window_width() / (float)get_window_height();
    float fovy = 3.141592 / 3.0;
    float fovx = atan(tan(fovy / 2) * aspectx) * 2.0;
    float z_near = 1.0;
    float z_far = 50.0;
    proj_matrix = mat4_make_perspective(fovy, aspecty, z_near, z_far);

    // init frustume planes with a point and a normal
    init_frustum_planes(fovx, fovy, z_near, z_far);
    
    load_mesh(
        "./assets/runway.obj", 
        "./assets/runway.png", 
        vec3_new(1, 1, 1), 
        vec3_new(0, -1.5, 23), 
        vec3_new(0, 0, 0)
    );
    load_mesh(
        "./assets/f22.obj", 
        "./assets/f22.png", 
        vec3_new(1, 1, 1), 
        vec3_new(0, -1.3, 5), 
        vec3_new(0, -M_PI/2, 0)
    ); 
    load_mesh(
        "./assets/efa.obj", 
        "./assets/efa.png", 
        vec3_new(1, 1, 1), 
        vec3_new(-2, -1.3, 9), 
        vec3_new(0, -M_PI/2, 0)
    );
    load_mesh(
        "./assets/f117.obj", 
        "./assets/f117.png", 
        vec3_new(1, 1, 1), 
        vec3_new(2, -1.3, 9), 
        vec3_new(0, -M_PI/2, 0)
    ); ; 
}

/**
 * ============================================================================
 * STAGE 2: PROCESS INPUT
 * ============================================================================
 * Handles user interactions (keyboard/mouse) via SDL Events.
 * Updates camera position, rotation, and toggles rendering modes.
 */
void process_input(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT: {
                is_running = 0;
                break;
            }
            case SDL_KEYDOWN: {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    is_running = 0;
                    break;
                } 
                if (event.key.keysym.sym == SDLK_1) {
                    set_render_method(RENDER_WIRE_VERTEX);
                    break;
                } 
                if (event.key.keysym.sym == SDLK_2) {
                    set_render_method(RENDER_WIRE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_3) {
                    set_render_method(RENDER_FILL_TRIANGLE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_4) {
                    set_render_method(RENDER_FILL_TRIANGLE_WIRE);
                    break; 
                }
                if (event.key.keysym.sym == SDLK_5) {
                    set_render_method(RENDER_TEXTURE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_6) {
                    set_render_method(RENDER_TEXTURE_WIRE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_c) {
                    set_cull_method(CULL_BACKFACE);
                    break;
                } 
                if (event.key.keysym.sym == SDLK_x) {
                    set_cull_method(CULL_NONE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    add_camera_yaw(1.0 * delta_time);
                    break;
                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    add_camera_yaw(-1.0 * delta_time);
                    break;
                } 
                if (event.key.keysym.sym == SDLK_w) {
                    add_camera_pitch(3.0 * delta_time);
                    break;
                }
                if (event.key.keysym.sym == SDLK_s) {
                    add_camera_pitch(-3.0 * delta_time);
                    break;
                } 
                if (event.key.keysym.sym == SDLK_UP) {
                    set_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                    set_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
                    break;
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                    set_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                    set_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
                    break;
                }
                break;
            }
            default: 
                break;
        }
    }
}

/**
 * ============================================================================
 * STAGE 3: THE GRAPHICS PIPELINE
 * ============================================================================
 * This is the heart of the 3D Engine. It transforms raw 3D data into 2D pixels.
 * 
 * Pipeline execution order per triangle:
 * 1. TRANSFORM (World Matrix): Scale -> Rotate -> Translate to position objects in the world.
 * 2. CAMERA VIEW (View Matrix): Move the world opposite to the camera to simulate looking through it.
 * 3. BACK-FACE CULLING: Discard triangles facing away from the camera (saves processing power).
 * 4. LIGHTING: Apply flat shading based on the angle to the light source.
 * 5. CLIPPING: Cut triangles against the 6 frustum planes (creates new polygons if partially inside).
 * 6. PERSPECTIVE DIVIDE: Divide x,y,z by 'w' to create depth distortion (objects further away look smaller).
 * 7. SCREEN MAPPING: Scale the normalized coordinates to fit the actual pixel width/height of the window.
 * 8. ASSEMBLY: Store the finalized 2D triangles in an array to be drawn in the render stage.
 */
void process_graphics_pipeline_stages(mesh_t* mesh) { 
    // initialize the target
    vec3_t target = get_camera_lookat_target();
    vec3_t up_dir = { 0, 1, 0 };
    
    // create a view matrix
    view_matrix = mat4_look_at(get_camera_position(), target, up_dir);
    // create a scale, rotation, and translation matrix to multiply the vertices
    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t trans_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);

    int num_faces = array_length(mesh->faces);
     
    // loop all triangle faces of our mesh
    // TODO: need to transform the vertices outside of the loop
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh->faces[i];
        
        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a];
        face_vertices[1] = mesh->vertices[mesh_face.b];
        face_vertices[2] = mesh->vertices[mesh_face.c];
       
        vec4_t transformed_vertices[3];
            
        // loop all three vertices of the current face and do transformations
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            world_matrix = mat4_identity();

            // order of transformation: scale -> rotate -> translate. [T]*[R]*[S]*v
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(trans_matrix, world_matrix);
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
            
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

            transformed_vertices[j] = transformed_vertex;
        }
        
        // calculate the triangle face normal
        vec3_t face_normal = get_triangle_normal(transformed_vertices);
       
        /**
         * BACK-FACE CULLING INTUITION:
         * ===========================
         * We use the Dot Product to determine if the triangle is facing the camera or away from it.
         * 
         * THE TEST:
         * - Compute the surface normal (N) of the triangle using cross product.
         * - Compute a vector from the camera to the triangle (Camera Ray).
         * - Calculate: dot(N, Camera_Ray)
         * 
         * WHY THIS WORKS:
         * - If the triangle faces the camera, the normal points TOWARD the camera.
         *   The dot product will be POSITIVE (angle < 90°).
         * - If the triangle faces AWAY from the camera, the normal points AWAY from the camera.
         *   The dot product will be NEGATIVE (angle > 90°).
         * 
         * ANALOGY: Think of holding a mirror toward a light source.
         * If the mirror faces you, you see the reflection (positive).
         * If the mirror faces away, you don't see the reflection (negative).
         * 
         * PERFORMANCE: This simple math test saves us from processing ~50% of triangles!
         */
        if (is_cull_backface()) {                  
            vec3_t camera_ray = vec3_sub(vec3_new(0, 0, 0), vec3_from_vec4(transformed_vertices[0]));
            float dot_normal_camera = vec3_dot(camera_ray, face_normal);

            if (dot_normal_camera < 0) {
                continue;
            }
        }   

        // flat shading
        /**
         * FLAT SHADING INTUITION:
         * =======================
         * We use the Dot Product to calculate how much light hits this triangle.
         * 
         * THE TEST:
         * - dot(Surface_Normal, Light_Direction)
         * - This gives us the cosine of the angle between the light and the surface.
         * 
         * WHY DOT PRODUCT?
         * - When light hits a surface head-on (perpendicular), it's brightest.
         *   The normal and light direction are parallel (angle = 0°), cos(0) = 1.
         * - When light hits at a glancing angle, it's dimmer.
         *   Angle = 90°, cos(90) = 0 (no light).
         * - When light hits from behind, it's negative (backwards).
         * 
         * THE SHADING FORMULA:
         * ====================
         * intensity = dot(normal, light_direction)
         * color = original_color * intensity
         * 
         * Note: We use -dot because our light direction points TOWARD the light source,
         * but the normal points OUT of the surface. We need to flip one to measure the angle.
         */
        float dot_normal_light = vec3_dot(face_normal, get_light_direction()); 
        uint32_t shaded_color = light_apply_intensity(mesh_face.color, -dot_normal_light);
          
        // create a polygon from original transformed triangle to be clipped
        polygon_t polygon = create_polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]), 
            vec3_from_vec4(transformed_vertices[1]), 
            vec3_from_vec4(transformed_vertices[2]),
            mesh_face.a_uv,
            mesh_face.b_uv,
            mesh_face.c_uv
        );
       
        // clip the polygon and returns a new polygon with potential new vertices
        clip_polygon(&polygon);
        
        // after clipping, break the polygon into triangles
        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;
        
        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        // loops all the assembled triangles after clipping
        for (int t = 0; t < num_triangles_after_clipping; t++) {
            triangle_t triangle_after_clipping = triangles_after_clipping[t];
                
            vec4_t projected_points[3];

            // loop all 3 vertices to perform projection
            for (int j = 0; j < 3; j++) {
                projected_points[j] = mat4_mul_vec4(proj_matrix, triangle_after_clipping.points[j]); 
                
                /**
                 * PERSPECTIVE DIVIDE INTUITION:
                 * =============================
                 * This is where the magic of 3D depth happens!
                 * 
                 * WHAT HAPPENS:
                 * - The projection matrix stores the original Z-depth in the W component.
                 * - We divide x, y, and z by w.
                 * 
                 * WHY THIS CREATES DEPTH:
                 * - Objects CLOSER to camera have SMALLER w (less distance to divide by).
                 *   Result: x and y get DIVIDED by a small number → LARGER screen coordinates.
                 * - Objects FARTHER from camera have LARGER w.
                 *   Result: x and y get DIVIDED by a large number → SMALLER screen coordinates.
                 * 
                 * VISUAL ANALOGY:
                 * ===============
                 * Imagine looking down a long hallway. The doors get progressively smaller
                 * as they get further away. That's perspective divide doing its job!
                 * 
                 * MATHEMATICAL INSIGHT:
                 * =====================
                 * This is NOT linear scaling. It's a non-linear transformation that
                 * mimics how human eyes perceive distance (things appear smaller as they get further).
                 */
                if (projected_points[j].w != 0) {
                    projected_points[j].x /= projected_points[j].w;
                    projected_points[j].y /= projected_points[j].w;
                    projected_points[j].z /= projected_points[j].w;
                }
                
                // invert the y value for flipped screen y coordinate
                projected_points[j].y *= -1;
                
                /**
                 * SCREEN MAPPING: From Normalized Device Coordinates to Screen Pixels
                 * =========================================================================
                 * After perspective divide, all coordinates are in the range [-1, +1].
                 * This is called "Normalized Device Coordinates" (NDC).
                 * 
                 * THE MAPPING PROCESS:
                 * =====================
                 * 1. NDC range: x ∈ [-1, 1], y ∈ [-1, 1]
                 * 2. Scale to half window size: x * (width/2), y * (height/2)
                 *    - Now x ∈ [-width/2, +width/2], y ∈ [-height/2, +height/2]
                 * 3. Translate to center: x + (width/2), y + (height/2)
                 *    - Now x ∈ [0, width], y ∈ [0, height] - actual screen pixels!
                 * 
                 * WHY THE Y-AXIS IS INVERTED:
                 * ============================
                 * In 3D graphics (OpenGL/DirectX convention):
                 * - Positive Y points UP in world space
                 * - Positive Z points TOWARD the viewer (out of the screen)
                 * 
                 * In 2D screen space:
                 * - Positive Y points DOWN (row index increases going down)
                 * 
                 * So we flip Y (* -1) to convert from 3D convention to screen convention.
                 */
                projected_points[j].x *= get_window_width() / 2.0;
                projected_points[j].y *= get_window_height() / 2.0;

                // translate the projected points to the middle of the screen
                projected_points[j].x += (get_window_width() / 2.0);
                projected_points[j].y += (get_window_height() / 2.0);
            }
            
            triangle_t triangle_to_render = {
                .points = {
                    { projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
                    { projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
                    { projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w },
                },
                .texcoords = {
                    { triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
                    { triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
                    { triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v },
                },
                .color = shaded_color,
                .texture = mesh->texture,
            };
           
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
                triangles_to_render[num_triangles_to_render++] = triangle_to_render;
            }
        }
    }
}

void update(void) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    } 
    
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0f;
    previous_frame_time = SDL_GetTicks();
    
    // initialize the counter of triangles to render for the current frame
    num_triangles_to_render = 0; 

    // loop all the meshes of our scene
    for (int m_idx = 0; m_idx < get_num_meshes(); m_idx++) {
        mesh_t* mesh = get_mesh(m_idx);
        
        // process the graphics pipeline stages for every mesh of uor 3D scene
        process_graphics_pipeline_stages(mesh);
    }
}

/**
 * ============================================================================
 * STAGE 5: RENDER
 * ============================================================================
 * The final stage. Translates our mathematical arrays into actual colored pixels.
 * 
 * 1. Clears the previous frame's color buffer and depth buffer.
 * 2. Draws the background grid.
 * 3. Loops over `triangles_to_render` array generated by the Graphics Pipeline.
 * 4. Calls the appropriate drawing functions (filled, textured, or wireframe) 
 *    based on the current render mode.
 * 5. Pushes the final 1D array of pixels to the SDL window hardware.
 */
void render(void) {
    clear_color_buffer(0xFF000000);
    clear_z_buffer();
    draw_grid();
    
    // loop all projected triangles and render them
    for (int i = 0; i < num_triangles_to_render; i++) {
        triangle_t triangle = triangles_to_render[i];
        
        if (should_render_filled_triangle()) {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
                triangle.color);
        }
        
        if (should_render_textured_triangle()) {
           draw_textured_triangle(
               triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, 
               triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, 
               triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, 
               triangle.texture);
        }

        if (should_render_wireframe()) {
            draw_triangle(
                    triangle.points[0].x, triangle.points[0].y,
                    triangle.points[1].x, triangle.points[1].y,
                    triangle.points[2].x, triangle.points[2].y,
                    0xFFFFFFFF);
        }
                    
        if (should_render_wire_vertex()) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);        
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);        
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }
    }

    render_color_buffer();
}

void free_resources(void) {
    free_meshes();
    destroy_window();
}

int main(void) {
    is_running = initialize_window() == 0;
    
    setup();
        
    while (is_running) {
        process_input();
        update();
        render();
    }
    
    free_resources();
    
    return 0;
}

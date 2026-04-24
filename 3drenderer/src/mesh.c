#include <stdio.h>
#include "mesh.h"
#include "array.h"
#include "vector.h"

#define MAX_LINES 1024
#define MAX_NUM_MESHES 10

static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;

void load_mesh(const char* obj_filename, const char* png_filename, vec3_t scale, vec3_t translation, vec3_t rotation) {
    load_mesh_obj_data(&meshes[mesh_count], obj_filename);
    load_mesh_png_data(&meshes[mesh_count], png_filename);
    
    meshes[mesh_count].scale = scale;
    meshes[mesh_count].translation = translation;
    meshes[mesh_count].rotation = rotation;

    mesh_count++;
}

void load_mesh_obj_data(mesh_t* mesh, const char* obj_filename) {
    array_free(mesh->vertices);
    array_free(mesh->faces);
    mesh->vertices = NULL;
    mesh->faces = NULL;

    FILE* file = fopen(obj_filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file!\n");
        return;
    }

    char line_buf[MAX_LINES];

    tex2_t* texcoords = NULL;

    while (fgets(line_buf, sizeof(line_buf), file) != NULL) {
        switch (line_buf[0]) {
            case 'v':
                if (line_buf[1] == 't') {
                    tex2_t texcoord;
                    if(sscanf(line_buf, "vt %f %f", &texcoord.u, &texcoord.v) == 2) {
                        array_push(texcoords, texcoord);
                    } else {
                        fprintf(stderr, "Not valid vt line\n");
                        return;
                    }
                    break;
                }

                if (line_buf[1] != ' ') break;
                vec3_t vertex;
                if (sscanf(line_buf, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3) {
                    array_push(mesh->vertices, vertex);
                } else {
                    fprintf(stderr, "Not valid v line\n");
                    return;
                }
                break;
            case 'f':
                int vertex_indices[3];
                int texture_indices[3];
                int normal_indices[3];
                if (sscanf(line_buf, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
                    &vertex_indices[0], &texture_indices[0], &normal_indices[0], 
                    &vertex_indices[1], &texture_indices[1], &normal_indices[1], 
                    &vertex_indices[2], &texture_indices[2], &normal_indices[2]
                ) == 9) {
                    face_t face = {
                        .a = vertex_indices[0] - 1,
                        .b = vertex_indices[1] - 1,
                        .c = vertex_indices[2] - 1,
                        .a_uv = texcoords[texture_indices[0] - 1],
                        .b_uv = texcoords[texture_indices[1] - 1],
                        .c_uv = texcoords[texture_indices[2] - 1],
                        .color = 0xFFFFFFFF,
                    };
                    array_push(mesh->faces, face);
                } else {    
                    fprintf(stderr, "Not valid f line\n");
                    return;
                }
                break;
            default:
                break;

        }
    }

    array_free(texcoords);
    fclose(file);
}

void load_mesh_png_data(mesh_t* mesh, const char* dng_filename) {
   upng_t* png_image= upng_new_from_file(dng_filename);
   if (png_image != NULL) {
       upng_decode(png_image);
       if (upng_get_error(png_image) == UPNG_EOK) {
            mesh->texture = png_image;
       }
   }
}

mesh_t* get_mesh(int idx) {
    return &meshes[idx];
}

int get_num_meshes() {
    return mesh_count;
}

void free_meshes() {
    for (int i = 0; i < mesh_count; i++) {
        mesh_t mesh = meshes[i];

        upng_free(mesh.texture);
        array_free(mesh.faces);
        array_free(mesh.vertices);
    }
}

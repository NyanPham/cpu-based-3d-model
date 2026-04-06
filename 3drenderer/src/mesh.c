#include <stdio.h>
#include "mesh.h"
#include "array.h"
#include "vector.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = { 0, 0, 0 }
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    { .x = -1, .y = -1, .z = -1 }, // 1
    { .x = -1, .y =  1, .z = -1 }, // 2
    { .x =  1, .y =  1, .z = -1 }, // 3
    { .x =  1, .y = -1, .z = -1 }, // 4
    { .x =  1, .y =  1, .z =  1 }, // 5
    { .x =  1, .y = -1, .z =  1 }, // 6 
    { .x = -1, .y =  1, .z =  1 }, // 7
    { .x = -1, .y = -1, .z =  1 }, // 8
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    { .a = 1, .b = 2, .c = 3 },
    { .a = 1, .b = 3, .c = 4 },
    // right
    { .a = 4, .b = 3, .c = 5 },
    { .a = 4, .b = 5, .c = 6 },
    // back
    { .a = 6, .b = 5, .c = 7 },
    { .a = 6, .b = 7, .c = 8 },
    // left
    { .a = 8, .b = 7, .c = 2 },
    { .a = 8, .b = 2, .c = 1 },
    // top
    { .a = 2, .b = 7, .c = 5 },
    { .a = 2, .b = 5, .c = 3 },
    // bottom
    { .a = 6, .b = 8, .c = 1 },
    { .a = 6, .b = 1, .c = 4 },
};

void load_cube_mesh_data(void) {
    for (int i = 0; i < N_CUBE_VERTICES; i++) {
        array_push(mesh.vertices, cube_vertices[i]);
    }

    for (int i = 0; i < N_CUBE_FACES; i++) {
        array_push(mesh.faces, cube_faces[i]);
    }
}

#define MAX_LINES 1024
void load_obj_file_data(const char* filename) {
    // read the contents of the .obj file
    // and load the vertices and faces in
    // our mesh.vertices and mesh.faces 
   
    array_free(mesh.vertices);
    array_free(mesh.faces);
    mesh.vertices = NULL;
    mesh.faces = NULL;

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file!\n");
        return;
    }

    char line_buf[MAX_LINES];

    while (fgets(line_buf, sizeof(line_buf), file) != NULL) {
        switch (line_buf[0]) {
            case 'v':
                if (line_buf[1] != ' ') break;
                vec3_t vertex;
                if (sscanf(line_buf, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3) {
                    array_push(mesh.vertices, vertex);
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
                    &vertex_indices[0], 
                    &texture_indices[0], 
                    &normal_indices[0], 
                    &vertex_indices[1], 
                    &texture_indices[1], 
                    &normal_indices[1], 
                    &vertex_indices[2], 
                    &texture_indices[2], 
                    &normal_indices[2]) == 9) {
                    face_t face = {
                        .a = vertex_indices[0],
                        .b = vertex_indices[1],
                        .c = vertex_indices[2]
                    };
                    array_push(mesh.faces, face);
                } else {    
                    fprintf(stderr, "Not valid f line\n");
                    return;
                }
                break;
            default:
                break;

        }
    }
}

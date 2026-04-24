#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"
#include "upng.h"

typedef struct {
    vec3_t* vertices;       // mesh dynamic array of vertices
    face_t* faces;          // mesh dynamic array of faces
    upng_t* texture;        // mesh PNG texture pointer
    vec3_t rotation;        // mesh rotation with x, y, and z values
    vec3_t scale;           // mesh scale with x, y, and z values
    vec3_t translation;     // mesh translation with x, y, and z values
} mesh_t;

void load_mesh(const char* obj_filename, const char* png_filename, vec3_t scale, vec3_t translation, vec3_t rotation);
void load_mesh_obj_data(mesh_t* mesh, const char*);
void load_mesh_png_data(mesh_t* mesh, const char* dng_filename);

mesh_t* get_mesh(int idx); 
int get_num_meshes(void);

void free_meshes(void);

#endif


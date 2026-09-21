#ifndef RAXTER_SCENE_H
#define RAXTER_SCENE_H

#include "../core/raxter_types.h"
#include "../math/vec.h"
#include "../math/mat.h"
#include "../math/spectrum.h"
#include "../math/wave.h"

#define RAXTER_MAX_SCENE_OBJECTS 256
#define RAXTER_MAX_SCENE_LIGHTS 64
#define RAXTER_MAX_SCENE_MATERIALS 256

typedef enum {
    SCENE_OBJECT_SPHERE = 0,
    SCENE_OBJECT_PLANE = 1,
    SCENE_OBJECT_MESH = 2,
} scene_object_type;

typedef struct {
    vec3 center;
    f32 radius;
    u32 material_id;
} scene_sphere;

typedef struct {
    vec3 point;
    vec3 normal;
    u32 material_id;
} scene_plane;

typedef struct {
    scene_object_type type;
    u32 material_id;
    mat4 transform;
    mat4 inverse_transform;
    union {
        scene_sphere sphere;
        scene_plane plane;
    };
} scene_object;

typedef struct {
    vec3 position;
    vec3 direction;
    spectral_power emission;
    f32 intensity;
    bool is_directional;
    bool is_coherent;
    f32 coherence_length;
} scene_light;

typedef struct {
    vec3 albedo;
    f32 roughness;
    f32 metallic;
    f32 ior;
    f32 emission_strength;
    spectral_power emission_spectrum;
    material_optical optical;
} scene_material;

typedef struct {
    scene_object objects[RAXTER_MAX_SCENE_OBJECTS];
    u32 object_count;
    scene_light lights[RAXTER_MAX_SCENE_LIGHTS];
    u32 light_count;
    scene_material materials[RAXTER_MAX_SCENE_MATERIALS];
    u32 material_count;
    vec3 background_color;
    f32 background_intensity;
} scene;

void scene_init(scene* s);
void scene_add_sphere(scene* s, vec3 center, f32 radius, u32 material_id);
void scene_add_plane(scene* s, vec3 point, vec3 normal, u32 material_id);
void scene_add_light(scene* s, vec3 position, vec3 direction, spectral_power emission, f32 intensity, bool directional);
u32 scene_add_material(scene* s, const scene_material* mat);
bool scene_intersect(const scene* s, vec3 origin, vec3 direction, f32* t, u32* object_id, vec3* hit_pos, vec3* normal);

#endif
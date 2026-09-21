#include "scene.h"
#include "math/vec.h"
#include "math/mat.h"
#include <math.h>
#include <string.h>

void scene_init(scene* s) {
    memset(s, 0, sizeof(scene));
    s->background_color = (vec3){0.0f, 0.0f, 0.0f};
    s->background_intensity = 0.0f;
}

void scene_add_sphere(scene* s, vec3 center, f32 radius, u32 material_id) {
    if (s->object_count >= RAXTER_MAX_SCENE_OBJECTS) return;
    
    scene_object* obj = &s->objects[s->object_count++];
    obj->type = SCENE_OBJECT_SPHERE;
    obj->material_id = material_id;
    obj->sphere.center = center;
    obj->sphere.radius = radius;
    obj->transform = mat4_identity();
    obj->inverse_transform = mat4_identity();
}

void scene_add_plane(scene* s, vec3 point, vec3 normal, u32 material_id) {
    if (s->object_count >= RAXTER_MAX_SCENE_OBJECTS) return;
    
    scene_object* obj = &s->objects[s->object_count++];
    obj->type = SCENE_OBJECT_PLANE;
    obj->material_id = material_id;
    obj->plane.point = point;
    obj->plane.normal = vec3_normalize(normal);
    obj->transform = mat4_identity();
    obj->inverse_transform = mat4_identity();
}

void scene_add_light(scene* s, vec3 position, vec3 direction, spectral_power emission, f32 intensity, bool directional) {
    if (s->light_count >= RAXTER_MAX_SCENE_LIGHTS) return;
    
    scene_light* light = &s->lights[s->light_count++];
    light->position = position;
    light->direction = vec3_normalize(direction);
    light->emission = emission;
    light->intensity = intensity;
    light->is_directional = directional;
    light->is_coherent = false;
    light->coherence_length = 1.0f;
}

u32 scene_add_material(scene* s, const scene_material* mat) {
    if (s->material_count >= RAXTER_MAX_SCENE_MATERIALS) return UINT32_MAX;
    
    s->materials[s->material_count] = *mat;
    return s->material_count++;
}

bool scene_intersect(const scene* s, vec3 origin, vec3 direction, f32* t, u32* object_id, vec3* hit_pos, vec3* normal) {
    f32 closest_t = 1e30f;
    u32 closest_id = UINT32_MAX;
    vec3 closest_pos = {0, 0, 0};
    vec3 closest_normal = {0, 0, 0};
    
    for (u32 i = 0; i < s->object_count; i++) {
        const scene_object* obj = &s->objects[i];
        f32 obj_t = 1e30f;
        vec3 obj_pos = {0, 0, 0};
        vec3 obj_normal = {0, 0, 0};
        bool hit = false;
        
        if (obj->type == SCENE_OBJECT_SPHERE) {
            vec3 oc = vec3_sub(origin, obj->sphere.center);
            f32 b = vec3_dot(oc, direction);
            f32 c = vec3_dot(oc, oc) - obj->sphere.radius * obj->sphere.radius;
            f32 disc = b * b - c;
            
            if (disc >= 0.0f) {
                f32 sqrt_disc = sqrtf(disc);
                f32 t1 = -b - sqrt_disc;
                f32 t2 = -b + sqrt_disc;
                
                if (t1 > 1e-4f) {
                    obj_t = t1;
                    hit = true;
                } else if (t2 > 1e-4f) {
                    obj_t = t2;
                    hit = true;
                }
                
                if (hit) {
                    obj_pos = vec3_add(origin, vec3_scale(direction, obj_t));
                    obj_normal = vec3_normalize(vec3_sub(obj_pos, obj->sphere.center));
                }
            }
        } else if (obj->type == SCENE_OBJECT_PLANE) {
            f32 denom = vec3_dot(direction, obj->plane.normal);
            if (fabsf(denom) > 1e-6f) {
                vec3 diff = vec3_sub(obj->plane.point, origin);
                obj_t = vec3_dot(diff, obj->plane.normal) / denom;
                
                if (obj_t > 1e-4f) {
                    hit = true;
                    obj_pos = vec3_add(origin, vec3_scale(direction, obj_t));
                    obj_normal = obj->plane.normal;
                }
            }
        }
        
        if (hit && obj_t < closest_t) {
            closest_t = obj_t;
            closest_id = i;
            closest_pos = obj_pos;
            closest_normal = obj_normal;
        }
    }
    
    if (closest_id != UINT32_MAX) {
        *t = closest_t;
        *object_id = closest_id;
        *hit_pos = closest_pos;
        *normal = closest_normal;
        return true;
    }
    
    return false;
}
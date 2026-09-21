#ifndef RAXTER_VEC_H
#define RAXTER_VEC_H

#include "../core/raxter_types.h"
#include <math.h>

static inline vec2 vec2_make(f32 x, f32 y) {
    return (vec2){x, y};
}

static inline vec3 vec3_make(f32 x, f32 y, f32 z) {
    return (vec3){x, y, z};
}

static inline vec4 vec4_make(f32 x, f32 y, f32 z, f32 w) {
    return (vec4){x, y, z, w};
}

static inline vec3 vec3_add(vec3 a, vec3 b) {
    return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline vec3 vec3_sub(vec3 a, vec3 b) {
    return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline vec3 vec3_mul(vec3 a, vec3 b) {
    return (vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

static inline vec3 vec3_scale(vec3 a, f32 s) {
    return (vec3){a.x * s, a.y * s, a.z * s};
}

static inline f32 vec3_dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec3 vec3_cross(vec3 a, vec3 b) {
    return (vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static inline f32 vec3_length(vec3 a) {
    return sqrtf(vec3_dot(a, a));
}

static inline f32 vec3_length_sq(vec3 a) {
    return vec3_dot(a, a);
}

static inline vec3 vec3_normalize(vec3 a) {
    f32 len = vec3_length(a);
    return len > RAXTER_EPSILON ? vec3_scale(a, 1.0f / len) : (vec3){0, 0, 0};
}

static inline vec3 vec3_reflect(vec3 v, vec3 n) {
    return vec3_sub(v, vec3_scale(n, 2.0f * vec3_dot(v, n)));
}

static inline vec3 vec3_refract(vec3 v, vec3 n, f32 eta) {
    f32 cosi = RAXTER_CLAMP(vec3_dot(v, n), -1.0f, 1.0f);
    f32 etai = 1.0f, etat = eta;
    vec3 n_out = n;
    
    if (cosi < 0) {
        cosi = -cosi;
    } else {
        f32 tmp = etai;
        etai = etat;
        etat = tmp;
        n_out = vec3_scale(n, -1.0f);
    }
    
    f32 eta_ratio = etai / etat;
    f32 k = 1.0f - eta_ratio * eta_ratio * (1.0f - cosi * cosi);
    
    return k < 0 ? (vec3){0, 0, 0} : 
        vec3_add(vec3_scale(v, eta_ratio), vec3_scale(n_out, eta_ratio * cosi - sqrtf(k)));
}

static inline vec4 vec4_add(vec4 a, vec4 b) {
    return (vec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

static inline vec4 vec4_scale(vec4 a, f32 s) {
    return (vec4){a.x * s, a.y * s, a.z * s, a.w * s};
}

static inline f32 vec4_dot(vec4 a, vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

#endif
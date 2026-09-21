#ifndef RAXTER_MAT_H
#define RAXTER_MAT_H

#include "../core/raxter_types.h"
#include "vec.h"
#include <math.h>

static inline mat4 mat4_identity() {
    mat4 m = {{{0}}};
    m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f;
    return m;
}

static inline mat4 mat4_zero() {
    return (mat4){{{0}}};
}

static inline mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 result = mat4_zero();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = a.m[i][0] * b.m[0][j] +
                             a.m[i][1] * b.m[1][j] +
                             a.m[i][2] * b.m[2][j] +
                             a.m[i][3] * b.m[3][j];
        }
    }
    return result;
}

static inline vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    return (vec4){
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w,
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w,
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w,
        m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w,
    };
}

static inline vec3 mat4_mul_vec3(mat4 m, vec3 v) {
    vec4 result = mat4_mul_vec4(m, vec4_make(v.x, v.y, v.z, 1.0f));
    if (result.w != 0.0f && result.w != 1.0f) {
        return (vec3){result.x / result.w, result.y / result.w, result.z / result.w};
    }
    return (vec3){result.x, result.y, result.z};
}

static inline mat4 mat4_transpose(mat4 m) {
    mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = m.m[j][i];
        }
    }
    return result;
}

static inline mat4 mat4_translate(vec3 t) {
    mat4 m = mat4_identity();
    m.m[0][3] = t.x;
    m.m[1][3] = t.y;
    m.m[2][3] = t.z;
    return m;
}

static inline mat4 mat4_scale(vec3 s) {
    mat4 m = mat4_zero();
    m.m[0][0] = s.x;
    m.m[1][1] = s.y;
    m.m[2][2] = s.z;
    m.m[3][3] = 1.0f;
    return m;
}

static inline mat4 mat4_rotate_x(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    mat4 m = mat4_identity();
    m.m[1][1] = c; m.m[1][2] = -s;
    m.m[2][1] = s; m.m[2][2] = c;
    return m;
}

static inline mat4 mat4_rotate_y(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    mat4 m = mat4_identity();
    m.m[0][0] = c; m.m[0][2] = s;
    m.m[2][0] = -s; m.m[2][2] = c;
    return m;
}

static inline mat4 mat4_rotate_z(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    mat4 m = mat4_identity();
    m.m[0][0] = c; m.m[0][1] = -s;
    m.m[1][0] = s; m.m[1][1] = c;
    return m;
}

static inline mat4 mat4_perspective(f32 fov, f32 aspect, f32 near, f32 far) {
    mat4 m = mat4_zero();
    f32 f = 1.0f / tanf(fov * 0.5f);
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (far + near) / (near - far);
    m.m[2][3] = (2.0f * far * near) / (near - far);
    m.m[3][2] = -1.0f;
    return m;
}

static inline mat4 mat4_look_at(vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec3_normalize(vec3_sub(center, eye));
    vec3 s = vec3_normalize(vec3_cross(f, up));
    vec3 u = vec3_cross(s, f);
    
    mat4 m = mat4_identity();
    m.m[0][0] = s.x; m.m[0][1] = s.y; m.m[0][2] = s.z;
    m.m[1][0] = u.x; m.m[1][1] = u.y; m.m[1][2] = u.z;
    m.m[2][0] = -f.x; m.m[2][1] = -f.y; m.m[2][2] = -f.z;
    m.m[0][3] = -vec3_dot(s, eye);
    m.m[1][3] = -vec3_dot(u, eye);
    m.m[2][3] = vec3_dot(f, eye);
    return m;
}

#endif
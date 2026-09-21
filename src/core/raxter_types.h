#ifndef RAXTER_TYPES_H
#define RAXTER_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef size_t   usize;

#define RAXTER_PI 3.14159265358979323846f
#define RAXTER_TAU (2.0f * RAXTER_PI)
#define RAXTER_EPSILON 1e-6f

#define RAXTER_MIN(a, b) ((a) < (b) ? (a) : (b))
#define RAXTER_MAX(a, b) ((a) > (b) ? (a) : (b))
#define RAXTER_CLAMP(x, min, max) RAXTER_MAX(min, RAXTER_MIN(max, x))

#define RAXTER_ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct {
    f32 x, y;
} vec2;

typedef struct {
    f32 x, y, z;
} vec3;

typedef struct {
    f32 x, y, z, w;
} vec4;

typedef struct {
    f32 m[4][4];
} mat4;

typedef struct {
    u32 width;
    u32 height;
} extent2d;

typedef struct {
    u32 x, y, z;
} extent3d;

#endif
#include "math/vec.h"
#include "math/mat.h"
#include "math/wave.h"
#include "math/spectrum.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_FLOAT_EQ(a, b, eps, msg) \
    do { \
        if (fabsf((a) - (b)) > (eps)) { \
            printf("FAIL: %s (expected %f, got %f) (%s:%d)\n", msg, (double)(b), (double)(a), __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

bool test_vec3_basic() {
    vec3 a = vec3_make(1.0f, 2.0f, 3.0f);
    vec3 b = vec3_make(4.0f, 5.0f, 6.0f);
    
    vec3 sum = vec3_add(a, b);
    TEST_ASSERT_FLOAT_EQ(sum.x, 5.0f, 1e-6, "vec3_add x");
    TEST_ASSERT_FLOAT_EQ(sum.y, 7.0f, 1e-6, "vec3_add y");
    TEST_ASSERT_FLOAT_EQ(sum.z, 9.0f, 1e-6, "vec3_add z");
    
    vec3 diff = vec3_sub(b, a);
    TEST_ASSERT_FLOAT_EQ(diff.x, 3.0f, 1e-6, "vec3_sub x");
    
    vec3 scaled = vec3_scale(a, 2.0f);
    TEST_ASSERT_FLOAT_EQ(scaled.x, 2.0f, 1e-6, "vec3_scale x");
    
    f32 dot = vec3_dot(a, b);
    TEST_ASSERT_FLOAT_EQ(dot, 32.0f, 1e-6, "vec3_dot");
    
    vec3 cross = vec3_cross(a, b);
    TEST_ASSERT_FLOAT_EQ(cross.x, -3.0f, 1e-6, "vec3_cross x");
    TEST_ASSERT_FLOAT_EQ(cross.y, 6.0f, 1e-6, "vec3_cross y");
    TEST_ASSERT_FLOAT_EQ(cross.z, -3.0f, 1e-6, "vec3_cross z");
    
    f32 len = vec3_length(a);
    TEST_ASSERT_FLOAT_EQ(len, sqrtf(14.0f), 1e-6, "vec3_length");
    
    vec3 norm = vec3_normalize(a);
    TEST_ASSERT_FLOAT_EQ(vec3_length(norm), 1.0f, 1e-6, "vec3_normalize");
    
    vec3 reflect = vec3_reflect(vec3_make(1, 0, 0), vec3_make(0, 1, 0));
    TEST_ASSERT_FLOAT_EQ(reflect.x, 1.0f, 1e-6, "vec3_reflect x");
    TEST_ASSERT_FLOAT_EQ(reflect.y, 0.0f, 1e-6, "vec3_reflect y");
    
    printf("PASS: test_vec3_basic\n");
    return true;
}

bool test_mat4_basic() {
    mat4 m = mat4_identity();
    TEST_ASSERT_FLOAT_EQ(m.m[0][0], 1.0f, 1e-6, "mat4_identity");
    
    mat4 t = mat4_translate(vec3_make(1, 2, 3));
    TEST_ASSERT_FLOAT_EQ(t.m[0][3], 1.0f, 1e-6, "mat4_translate x");
    TEST_ASSERT_FLOAT_EQ(t.m[1][3], 2.0f, 1e-6, "mat4_translate y");
    TEST_ASSERT_FLOAT_EQ(t.m[2][3], 3.0f, 1e-6, "mat4_translate z");
    
    mat4 s = mat4_scale(vec3_make(2, 3, 4));
    TEST_ASSERT_FLOAT_EQ(s.m[0][0], 2.0f, 1e-6, "mat4_scale x");
    TEST_ASSERT_FLOAT_EQ(s.m[1][1], 3.0f, 1e-6, "mat4_scale y");
    TEST_ASSERT_FLOAT_EQ(s.m[2][2], 4.0f, 1e-6, "mat4_scale z");
    
    mat4 rx = mat4_rotate_x(RAXTER_PI / 2.0f);
    TEST_ASSERT_FLOAT_EQ(rx.m[1][1], 0.0f, 1e-5, "mat4_rotate_x");
    TEST_ASSERT_FLOAT_EQ(rx.m[1][2], -1.0f, 1e-5, "mat4_rotate_x");
    
    mat4 p = mat4_perspective(RAXTER_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    TEST_ASSERT(p.m[0][0] > 0 && p.m[1][1] > 0, "mat4_perspective");
    
    mat4 look = mat4_look_at(vec3_make(0, 0, 5), vec3_make(0, 0, 0), vec3_make(0, 1, 0));
    vec3 transformed = mat4_mul_vec3(look, vec3_make(0, 0, 0));
    TEST_ASSERT_FLOAT_EQ(transformed.z, -5.0f, 1e-5, "mat4_look_at");
    
    printf("PASS: test_mat4_basic\n");
    return true;
}

bool test_wave_spectrum() {
    wave_spectrum s1 = wave_spectrum_single(550.0f, 1.0f, vec3_make(1, 0, 0));
    TEST_ASSERT(s1.count == 1, "wave_spectrum_single count");
    TEST_ASSERT_FLOAT_EQ(s1.components[0].wavelength_nm, 550.0f, 1e-6, "wave_spectrum_single wavelength");
    
    wave_spectrum s2 = wave_spectrum_single(650.0f, 0.5f, vec3_make(0, 0, 1));
    wave_spectrum combined = wave_spectrum_add(s1, s2);
    TEST_ASSERT(combined.count == 2, "wave_spectrum_add count");
    
    f32 intensity = wave_spectrum_intensity(s1);
    TEST_ASSERT_FLOAT_EQ(intensity, 1.0f, 1e-6, "wave_spectrum_intensity");
    
    vec3 color = wave_spectrum_color(s1);
    TEST_ASSERT(color.x > 0 || color.y > 0 || color.z > 0, "wave_spectrum_color");
    
    printf("PASS: test_wave_spectrum\n");
    return true;
}

bool test_fresnel() {
    f32 f0 = fresnel_schlick(1.0f, 0.04f);
    TEST_ASSERT_FLOAT_EQ(f0, 0.04f, 1e-6, "fresnel_schlick normal");
    
    f32 f90 = fresnel_schlick(0.0f, 0.04f);
    TEST_ASSERT_FLOAT_EQ(f90, 1.0f, 1e-6, "fresnel_schlick grazing");
    
    f32 exact = fresnel_exact(1.0f, 1.0f, 1.5f);
    TEST_ASSERT_FLOAT_EQ(exact, 0.04f, 1e-4, "fresnel_exact normal");
    
    f32 exact_grazing = fresnel_exact(0.0f, 1.0f, 1.5f);
    TEST_ASSERT_FLOAT_EQ(exact_grazing, 1.0f, 1e-4, "fresnel_exact grazing");
    
    printf("PASS: test_fresnel\n");
    return true;
}

bool test_spectrum() {
    spectral_power s = spectrum_make_blackbody(6500.0f);
    TEST_ASSERT(s.lambda_min == 380.0f && s.lambda_max == 780.0f, "spectrum_make_blackbody range");
    
    f32 max_val = 0.0f;
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        if (s.samples[i] > max_val) max_val = s.samples[i];
    }
    TEST_ASSERT_FLOAT_EQ(max_val, 1.0f, 1e-4, "spectrum_make_blackbody normalized");
    
    cie_xyz xyz = spectrum_to_xyz(s);
    TEST_ASSERT(xyz.x > 0 && xyz.y > 0 && xyz.z > 0, "spectrum_to_xyz positive");
    
    vec3 rgb = xyz_to_srgb(xyz);
    TEST_ASSERT(rgb.x >= 0 && rgb.x <= 1 && rgb.y >= 0 && rgb.y <= 1 && rgb.z >= 0 && rgb.z <= 1, "xyz_to_srgb clamped");
    
    printf("PASS: test_spectrum\n");
    return true;
}

int main() {
    int passed = 0;
    int total = 0;
    
    total++; if (test_vec3_basic()) passed++;
    total++; if (test_mat4_basic()) passed++;
    total++; if (test_wave_spectrum()) passed++;
    total++; if (test_fresnel()) passed++;
    total++; if (test_spectrum()) passed++;
    
    printf("\n=== Results: %d/%d tests passed ===\n", passed, total);
    return passed == total ? 0 : 1;
}
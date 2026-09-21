#include "math/wave.h"
#include "math/spectrum.h"
#include <stdio.h>
#include <math.h>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

bool test_interference_pattern() {
    light_source src1 = {0};
    src1.position = vec3_make(-1.0f, 0.0f, 0.0f);
    src1.direction = vec3_make(1.0f, 0.0f, 0.0f);
    src1.spectrum = wave_spectrum_single(550.0f, 1.0f, vec3_make(1, 0, 0));
    src1.intensity = 1.0f;
    src1.coherence_length = 10.0f;
    src1.is_coherent = true;
    
    light_source src2 = src1;
    src2.position = vec3_make(1.0f, 0.0f, 0.0f);
    
    float intensity_center = interference_pattern(vec3_make(0, 0, 10), src1, src2, 550.0f);
    float intensity_side = interference_pattern(vec3_make(2, 0, 10), src1, src2, 550.0f);
    
    printf("Interference center: %f, side: %f\n", intensity_center, intensity_side);
    TEST_ASSERT(intensity_center > intensity_side, "interference pattern center brighter");
    
    printf("PASS: test_interference_pattern\n");
    return true;
}

bool test_polarization_reflection() {
    vec3 incident_pol = vec3_make(1.0f, 0.0f, 0.0f);
    vec3 normal = vec3_make(0.0f, 0.0f, 1.0f);
    
    vec3 refl_pol = compute_polarization_reflection(incident_pol, normal, 0.0f, 1.0f, 1.5f);
    
    float phase = phase_shift_reflection(0.0f, 1.0f, 1.5f);
    TEST_ASSERT(phase == RAXTER_PI || phase == 0.0f, "phase shift is 0 or pi");
    
    printf("PASS: test_polarization_reflection\n");
    return true;
}

bool test_wave_propagation() {
    light_source src = {0};
    src.position = vec3_make(0.0f, 0.0f, -5.0f);
    src.direction = vec3_make(0.0f, 0.0f, 1.0f);
    src.spectrum = wave_spectrum_single(550.0f, 1.0f, vec3_make(1, 0, 0));
    src.intensity = 1.0f;
    src.coherence_length = 100.0f;
    src.is_coherent = true;
    
    material_optical mat = {0};
    mat.refractive_index_real = 1.5f;
    mat.absorption_coefficient = 0.0f;
    mat.scattering_coefficient = 0.0f;
    
    wave_spectrum incident = src.spectrum;
    vec3 k = vec3_make(0, 0, 1);
    vec3 normal = vec3_make(0, 0, -1);
    
    wave_spectrum reflected = propagate_wave(incident, k, normal, mat, 0);
    TEST_ASSERT(reflected.count > 0, "propagate_wave produces reflection");
    
    printf("PASS: test_wave_propagation\n");
    return true;
}

int main() {
    int passed = 0;
    int total = 0;
    
    total++; if (test_interference_pattern()) passed++;
    total++; if (test_polarization_reflection()) passed++;
    total++; if (test_wave_propagation()) passed++;
    
    printf("\n=== Wave Tests: %d/%d passed ===\n", passed, total);
    return passed == total ? 0 : 1;
}
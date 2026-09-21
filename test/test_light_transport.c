#include "renderer/light_transport.h"
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

bool test_light_transport_create() {
    light_transport_renderer renderer;
    bool result = light_transport_create(&renderer, 256, 256, 16, 4);
    TEST_ASSERT(result, "light_transport_create");
    TEST_ASSERT(renderer.framebuffer != NULL, "framebuffer allocated");
    TEST_ASSERT(renderer.width == 256 && renderer.height == 256, "dimensions");
    TEST_ASSERT(renderer.spectral_samples == 16, "spectral samples");
    
    light_transport_destroy(&renderer);
    printf("PASS: test_light_transport_create\n");
    return true;
}

bool test_light_transport_config() {
    light_transport_renderer renderer;
    light_transport_create(&renderer, 128, 128, 8, 4);
    
    light_transport_config config = {0};
    config.max_bounces = 8;
    config.wavelength_min = 380.0f;
    config.wavelength_max = 780.0f;
    config.spectral_samples = 16;
    config.enable_interference = true;
    config.enable_diffraction = true;
    config.enable_polarization = true;
    config.enable_volume_scattering = false;
    
    light_transport_set_config(&renderer, &config);
    
    light_transport_destroy(&renderer);
    printf("PASS: test_light_transport_config\n");
    return true;
}

bool test_spectral_power_operations() {
    spectral_power s1 = spectrum_make_uniform(1.0f, 380.0f, 780.0f);
    spectral_power s2 = spectrum_make_uniform(0.5f, 380.0f, 780.0f);
    
    spectral_power sum = spectrum_add(s1, s2);
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        TEST_ASSERT_FLOAT_EQ(sum.samples[i], 1.5f, 1e-5, "spectrum_add");
    }
    
    spectral_power scaled = spectrum_scale(s1, 0.25f);
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        TEST_ASSERT_FLOAT_EQ(scaled.samples[i], 0.25f, 1e-5, "spectrum_scale");
    }
    
    spectral_power mul = spectrum_mul(s1, s2);
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        TEST_ASSERT_FLOAT_EQ(mul.samples[i], 0.5f, 1e-5, "spectrum_mul");
    }
    
    printf("PASS: test_spectral_power_operations\n");
    return true;
}

bool test_tonemapping() {
    spectral_power s = spectrum_make_blackbody(6500.0f);
    vec3 rgb = spectrum_to_srgb(s);
    
    float reinhard = reinhard_tonemap(rgb.x);
    TEST_ASSERT(reinhard >= 0.0f && reinhard <= 1.0f, "reinhard_tonemap range");
    
    vec3 aces = aces_filmic(rgb);
    TEST_ASSERT(aces.x >= 0.0f && aces.x <= 1.0f, "aces_filmic range");
    
    printf("PASS: test_tonemapping\n");
    return true;
}

int main() {
    int passed = 0;
    int total = 0;
    
    total++; if (test_light_transport_create()) passed++;
    total++; if (test_light_transport_config()) passed++;
    total++; if (test_spectral_power_operations()) passed++;
    total++; if (test_tonemapping()) passed++;
    
    printf("\n=== Light Transport Tests: %d/%d passed ===\n", passed, total);
    return passed == total ? 0 : 1;
}
#ifndef RAXTER_WAVE_OPTICS_H
#define RAXTER_WAVE_OPTICS_H

#include "core/vk_core.h"
#include "core/vk_pipeline.h"
#include "core/vk_memory.h"
#include "core/raxter_types.h"
#include "math/wave.h"

#define RAXTER_MAX_WAVELENGTHS 32

typedef struct {
    vk_compute_pipeline pipeline;
    vk_buffer source_buffer;
    vk_buffer material_buffer;
    vk_image output_image;
    VkDescriptorSetLayout set_layout;
    VkDescriptorPool desc_pool;
    VkDescriptorSet desc_sets[RAXTER_MAX_FRAMES_IN_FLIGHT];
    u32 width, height;
} wave_optics_renderer;

typedef struct {
    light_source sources[RAXTER_MAX_SOURCES];
    u32 source_count;
    u32 max_bounces;
    f32 wavelength_min;
    f32 wavelength_max;
    f32 time;
    u32 frame_index;
    int test_mode;
} wave_optics_params;

bool wave_optics_create(vk_context* ctx, wave_optics_renderer* renderer, u32 width, u32 height, u32 max_bounces);
void wave_optics_destroy(vk_context* ctx, wave_optics_renderer* renderer);
void wave_optics_set_sources(wave_optics_renderer* renderer, const light_source* sources, u32 count);
void wave_optics_set_params(wave_optics_renderer* renderer, const wave_optics_params* params);
void wave_optics_render(vk_context* ctx, wave_optics_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);
void wave_optics_resize(wave_optics_renderer* renderer, u32 width, u32 height);

#endif
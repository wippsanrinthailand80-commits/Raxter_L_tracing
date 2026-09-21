#ifndef RAXTER_FOUR_WAVE_MIXING_H
#define RAXTER_FOUR_WAVE_MIXING_H

#include "core/vk_core.h"
#include "core/vk_pipeline.h"
#include "core/vk_memory.h"
#include "core/raxter_types.h"

typedef struct {
    vk_compute_pipeline pipeline;
    vk_image output_image;
    VkDescriptorSetLayout set_layout;
    VkDescriptorPool desc_pool;
    VkDescriptorSet desc_sets[RAXTER_MAX_FRAMES_IN_FLIGHT];
    u32 width, height;
} four_wave_mixing_renderer;

typedef struct {
    u32 num_pumps;
    f32 time;
    f32 interaction_length;
    f32 nonlinear_coeff;
    f32 dispersion;
    int process_type;
    f32 phase_matching;
} four_wave_mixing_params;

bool four_wave_mixing_create(vk_context* ctx, four_wave_mixing_renderer* renderer, u32 width, u32 height);
void four_wave_mixing_destroy(vk_context* ctx, four_wave_mixing_renderer* renderer);
void four_wave_mixing_set_params(four_wave_mixing_renderer* renderer, const four_wave_mixing_params* params);
void four_wave_mixing_render(vk_context* ctx, four_wave_mixing_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif
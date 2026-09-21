#ifndef RAXTER_OAM_BEAMS_H
#define RAXTER_OAM_BEAMS_H

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
} oam_beams_renderer;

typedef struct {
    u32 num_sources;
    f32 time;
    f32 propagation_distance;
    f32 screen_distance;
    int visualization_mode;
} oam_beams_params;

bool oam_beams_create(vk_context* ctx, oam_beams_renderer* renderer, u32 width, u32 height);
void oam_beams_destroy(vk_context* ctx, oam_beams_renderer* renderer);
void oam_beams_set_params(oam_beams_renderer* renderer, const oam_beams_params* params);
void oam_beams_render(vk_context* ctx, oam_beams_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif
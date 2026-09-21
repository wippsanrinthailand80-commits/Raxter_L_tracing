#ifndef RAXTER_QUANTUM_COHERENCE_H
#define RAXTER_QUANTUM_COHERENCE_H

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
} quantum_coherence_renderer;

typedef struct {
    u32 num_sources;
    f32 time;
    f32 detector_separation;
    int experiment_type;
    f32 bunching_parameter;
} quantum_coherence_params;

bool quantum_coherence_create(vk_context* ctx, quantum_coherence_renderer* renderer, u32 width, u32 height);
void quantum_coherence_destroy(vk_context* ctx, quantum_coherence_renderer* renderer);
void quantum_coherence_set_params(quantum_coherence_renderer* renderer, const quantum_coherence_params* params);
void quantum_coherence_render(vk_context* ctx, quantum_coherence_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif
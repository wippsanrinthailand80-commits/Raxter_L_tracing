#ifndef RAXTER_VK_COMMAND_POOL_H
#define RAXTER_VK_COMMAND_POOL_H

#include "vk_core.h"

bool vk_command_pool_create(vk_context* ctx, vk_command_pool* pool, u32 queue_family, VkCommandPoolCreateFlags flags);
void vk_command_pool_destroy(vk_context* ctx, vk_command_pool* pool);

#endif
#ifndef RAXTER_VK_DEBUG_H
#define RAXTER_VK_DEBUG_H

#include "vk_core.h"

void vk_set_object_name(vk_context* ctx, VkObjectType type, u64 handle, const char* name);
void vk_cmd_begin_label(vk_context* ctx, VkCommandBuffer cmd, const char* name, float color[4]);
void vk_cmd_end_label(vk_context* ctx, VkCommandBuffer cmd);
void vk_cmd_insert_label(vk_context* ctx, VkCommandBuffer cmd, const char* name, float color[4]);

#endif
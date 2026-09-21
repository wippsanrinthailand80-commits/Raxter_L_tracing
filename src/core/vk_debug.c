#include "vk_debug.h"
#include <stdio.h>

static PFN_vkSetDebugUtilsObjectNameEXT g_vkSetDebugUtilsObjectNameEXT = NULL;
static PFN_vkCmdBeginDebugUtilsLabelEXT g_vkCmdBeginDebugUtilsLabelEXT = NULL;
static PFN_vkCmdEndDebugUtilsLabelEXT g_vkCmdEndDebugUtilsLabelEXT = NULL;
static PFN_vkCmdInsertDebugUtilsLabelEXT g_vkCmdInsertDebugUtilsLabelEXT = NULL;
static bool debug_loaded = false;

static void load_debug_functions(vk_context* ctx) {
    if (debug_loaded) return;
    g_vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(ctx->device, "vkSetDebugUtilsObjectNameEXT");
    g_vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(ctx->device, "vkCmdBeginDebugUtilsLabelEXT");
    g_vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(ctx->device, "vkCmdEndDebugUtilsLabelEXT");
    g_vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetDeviceProcAddr(ctx->device, "vkCmdInsertDebugUtilsLabelEXT");
    debug_loaded = true;
}

void vk_set_object_name(vk_context* ctx, VkObjectType type, u64 handle, const char* name) {
    load_debug_functions(ctx);
    if (!g_vkSetDebugUtilsObjectNameEXT) return;
    
    VkDebugUtilsObjectNameInfoEXT info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = name,
    };
    g_vkSetDebugUtilsObjectNameEXT(ctx->device, &info);
}

void vk_cmd_begin_label(vk_context* ctx, VkCommandBuffer cmd, const char* name, float color[4]) {
    load_debug_functions(ctx);
    if (!g_vkCmdBeginDebugUtilsLabelEXT) return;
    
    VkDebugUtilsLabelEXT label = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        .pLabelName = name,
        .color = {color[0], color[1], color[2], color[3]},
    };
    g_vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
}

void vk_cmd_end_label(vk_context* ctx, VkCommandBuffer cmd) {
    load_debug_functions(ctx);
    if (g_vkCmdEndDebugUtilsLabelEXT) g_vkCmdEndDebugUtilsLabelEXT(cmd);
}

void vk_cmd_insert_label(vk_context* ctx, VkCommandBuffer cmd, const char* name, float color[4]) {
    load_debug_functions(ctx);
    if (!g_vkCmdInsertDebugUtilsLabelEXT) return;
    
    VkDebugUtilsLabelEXT label = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        .pLabelName = name,
        .color = {color[0], color[1], color[2], color[3]},
    };
    g_vkCmdInsertDebugUtilsLabelEXT(cmd, &label);
}
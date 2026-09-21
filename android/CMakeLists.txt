cmake_minimum_required(VERSION 3.18)
project(madel_android LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(ANDROID_ABI arm64-v8a)
set(ANDROID_PLATFORM android-24)
set(ANDROID_STL c++_shared)

find_package(Vulkan REQUIRED)

add_library(madel SHARED
    ../src/core/vk_core.c
    ../src/core/vk_debug.c
    ../src/core/vk_memory.c
    ../src/core/vk_pipeline.c
    ../src/core/vk_descriptor.c
    ../src/core/vk_sync.c
    ../src/math/vec.c
    ../src/math/mat.c
    ../src/math/wave.c
    ../src/math/spectrum.c
    ../src/compute/wave_optics.c
    ../src/compute/interference.c
    ../src/compute/diffraction.c
    ../src/compute/polarization.c
    ../src/compute/light_propagation.c
    ../src/renderer/renderer.c
    ../src/renderer/swapchain.c
    ../src/renderer/light_transport.c
    ../src/renderer/scene.c
    ../src/platform/window.c
    ../src/platform/input.c
    ../src/platform/platform.c
)

target_include_directories(madel PRIVATE
    ../src
    ../src/core
    ../src/math
    ../src/compute
    ../src/renderer
    ../src/platform
)

target_link_libraries(madel PRIVATE
    ${VULKAN_LIBRARIES}
    android
    log
)

set_target_properties(madel PROPERTIES
    ANDROID_LIBRARY_TYPE SHARED
)
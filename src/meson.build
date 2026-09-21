core_sources = files(
  'core/vk_core.c',
  'core/vk_debug.c',
  'core/vk_memory.c',
  'core/vk_pipeline.c',
  'core/vk_descriptor.c',
  'core/vk_sync.c',
  'core/gpu_detect.c',
)

math_sources = files(
  'math/vec.c',
  'math/mat.c',
  'math/wave.c',
  'math/spectrum.c',
)

platform_sources = files(
  'platform/window.c',
  'platform/input.c',
  'platform/platform.c',
)

renderer_sources = files(
  'renderer/renderer.c',
  'renderer/swapchain.c',
  'renderer/light_transport.c',
  'renderer/scene.c',
)

compute_sources = files(
  'compute/wave_optics.c',
  'compute/interference.c',
  'compute/diffraction.c',
  'compute/polarization.c',
  'compute/light_propagation.c',
  'compute/quantum_coherence.c',
  'compute/oam_beams.c',
  'compute/spatiotemporal_packets.c',
  'compute/four_wave_mixing.c',
  'compute/casimir_effect.c',
  'compute/ghost_imaging.c',
)

all_sources = core_sources + math_sources + platform_sources + renderer_sources + compute_sources

inc_root = include_directories('..')
inc_src = include_directories('.')

raxter_lib = static_library('raxter', all_sources,
  include_directories: [inc_root, inc_src],
  dependencies: [vulkan_dep],
  c_args: [
    '-DRAXTER_MAX_BOUNCES=@0@'.format(get_option('max-light-bounces')),
    get_option('enable-validation') ? '-DRAXTER_VALIDATION=1' : '-DRAXTER_VALIDATION=0',
  ],
  install: true,
)

raxter_dep = declare_dependency(
  link_with: raxter_lib,
  include_directories: [inc_root, inc_src],
  dependencies: [vulkan_dep],
)

executable('raxter-demo', 'demo/main.c',
  dependencies: [raxter_dep],
  install: true,
)
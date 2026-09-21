shaderc = find_program('glslc', required: false)

shader_sources = [
  'wave_optics.comp',
  'interference.comp',
  'diffraction.comp',
  'polarization.comp',
  'light_propagation.comp',
  'spectral_render.comp',
  'quantum_coherence.comp',
  'oam_beams.comp',
  'spatiotemporal_packets.comp',
  'four_wave_mixing.comp',
  'casimir_effect.comp',
  'ghost_imaging.comp',
]

if shaderc.found()
  shader_spvs = []
  foreach shader : shader_sources
    spv = custom_target(
      shader + '.spv',
      input: shader,
      output: shader + '.spv',
      command: [shaderc, '-V', '@INPUT@', '-o', '@OUTPUT@'],
      install: true,
      install_dir: get_option('datadir') / 'raxter/shaders',
    )
    shader_spvs += spv
  endforeach
else
  shader_spvs = []
  foreach shader : shader_sources
    spv_file = files(shader + '.spv')
    if not spv_file[0].exists()
      warning('Precompiled shader @0@.spv not found'.format(shader))
    endif
    shader_spvs += spv_file
  endforeach
endif

shader_dep = declare_dependency(
  sources: shader_spvs,
)
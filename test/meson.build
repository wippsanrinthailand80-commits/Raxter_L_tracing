test_sources = [
  'test_math.c',
  'test_gpu_detect.c',
]

test_deps = [raxter_dep]
# raxter_dep is defined in parent meson.build

if cc.has_function('pthread_create')
  test_deps += dependency('threads')
endif

foreach test_src : test_sources
  test_name = test_src.split('.')[0]
  test_exe = executable(test_name, test_src,
    dependencies: test_deps,
    install: false,
  )
  test(test_name, test_exe)
endforeach

# benchmark_sources = [
#   'bench_wave_optics.c',
# ]
# 
# foreach bench_src : benchmark_sources
#   bench_name = bench_src.split('.')[0]
#   executable(bench_name, bench_src,
#     dependencies: test_deps,
#     install: false,
#   )
# endforeach
project('raxter-l-tracing', 'c',
  version: '0.1.0',
  default_options: [
    'c_std=c99',
    'warning_level=3',
    'werror=false',
    'buildtype=debugoptimized',
  ])

cc = meson.get_compiler('c')

vulkan_dep = dependency('vulkan', required: true)

platform = host_machine.system()
is_android = platform == 'android'
is_linux = platform == 'linux'

if is_android
  android_ndk = get_option('android-ndk')
  android_api = get_option('android-api')
  ndk_path = android_ndk != '' ? android_ndk : environment.get('ANDROID_NDK_HOME')
  if ndk_path == ''
    error('Android NDK not found. Set --android-ndk or ANDROID_NDK_HOME')
  endif
  
  api_level = android_api != '' ? android_api : '24'
  
  c_args_android = [
    '-DANDROID',
    '-DVK_USE_PLATFORM_ANDROID_KHR',
    '-I@0@/sysroot/usr/include'.format(ndk_path),
    '-I@0@/sysroot/usr/include/@1@'.format(ndk_path, 'android-' + api_level),
    '-I@0@/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include'.format(ndk_path),
  ]
  
  c_link_args_android = [
    '-landroid',
    '-llog',
    '-lvulkan',
  ]
  
  vulkan_dep = declare_dependency(
    include_directories: include_directories('@0@/sysroot/usr/include'.format(ndk_path)),
    link_args: c_link_args_android,
  )
  
  add_project_arguments(c_args_android, language: 'c')
  add_project_link_arguments(c_link_args_android, language: 'c')
endif

if is_linux
  x11_dep = dependency('x11', required: false)
  xcb_dep = dependency('xcb', required: false)
  x11_xcb_dep = dependency('x11-xcb', required: false)
  wayland_dep = dependency('wayland-client', required: false)
  m_dep = cc.find_library('m', required: false)
  
  platform_deps = []
  if x11_dep.found()
    platform_deps += x11_dep
    add_project_arguments('-DVK_USE_PLATFORM_XLIB_KHR', language: 'c')
  endif
  if xcb_dep.found()
    platform_deps += xcb_dep
    add_project_arguments('-DVK_USE_PLATFORM_XCB_KHR', language: 'c')
  endif
  if x11_xcb_dep.found()
    platform_deps += x11_xcb_dep
  endif
  if wayland_dep.found()
    platform_deps += wayland_dep
    add_project_arguments('-DVK_USE_PLATFORM_WAYLAND_KHR', language: 'c')
  endif
  if m_dep.found()
    platform_deps += m_dep
  endif
  
  vulkan_dep = declare_dependency(
    dependencies: [vulkan_dep] + platform_deps,
  )
endif

shaderc = find_program('glslc', required: false)
if not shaderc.found()
  warning('glslc not found - shaders must be precompiled')
endif

subdir('src')
subdir('shaders')
subdir('test')
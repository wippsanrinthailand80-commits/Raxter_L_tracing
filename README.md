# Raxter L Tracing - Wave Optics Game Engine

A physically-based rendering engine using compute-based wave optics simulation with Vulkan and C99.

## Features

- **Wave Optics Simulation**: Full wave-based light transport with interference, diffraction, and polarization
- **Physically Accurate**: Spectral rendering with CIE color matching functions, Fresnel equations, Mueller calculus
- **Configurable Light Bounces**: 1-8 bounces (mobile: 1-3, desktop: 1-8)
- **Cross-Platform**: Linux (X11/Wayland) and Android (Vulkan 1.1+)
- **Compute-Based**: All rendering via Vulkan compute shaders
- **Validation Layers**: Full Vulkan validation layer support for correctness

## Architecture

```
src/
├── core/           # Vulkan core (instance, device, memory, pipelines, sync)
├── math/           # Vector, matrix, wave, spectral math
├── compute/        # Compute shader interfaces (wave_optics, interference, diffraction, polarization, light_propagation)
├── renderer/       # High-level renderer, light transport, scene
├── platform/       # Window, input, platform abstraction
└── demo/           # Demo application

shaders/
├── wave_optics.comp       # Main wave optics simulation
├── interference.comp      # Double/multi-slit interference
├── diffraction.comp       # Fresnel/Fraunhofer diffraction
├── polarization.comp      # Polarization (Malus, waveplates, Mueller)
├── light_propagation.comp # Volumetric light transport
└── spectral_render.comp   # Spectral to sRGB conversion

test/
├── test_math.c           # Vector/matrix/wave/spectrum tests
├── test_wave.c           # Wave optics tests
├── test_vk_core.c        # Vulkan core tests
├── test_light_transport.c # Light transport tests
└── bench_wave_optics.c   # Performance benchmarks
```

## Building

### Linux (Meson)

```bash
# Install dependencies
# Ubuntu/Debian: sudo apt install meson ninja-build libvulkan-dev libx11-dev libxcb1-dev libwayland-dev glslang-tools
# Arch: sudo pacman -S meson ninja vulkan-headers vulkan-icd-loader xorg-server xcb-util wayland glslang

# Build
./build.sh

# Build with options
./build.sh --type release --max-bounces 8

# Run demo
./build_linux/raxter-demo

# Run tests
meson test -C build_linux
```

### Android (NDK)

```bash
# Set NDK path
export ANDROID_NDK_HOME=/path/to/android-ndk

# Build
cd android
./build.sh --api 24 --abi arm64-v8a

# Output: build_android_arm64-v8a_Release/libraxter.so
```

## Rendering Modes

1. **Wave Optics** - Full spectral wave simulation with sources and materials
2. **Interference** - Double-slit and multi-slit interference patterns
3. **Diffraction** - Fresnel and Fraunhofer diffraction (circular/rectangular apertures)
4. **Polarization** - Linear/circular polarization, waveplates, Malus law
5. **Light Propagation** - 3D volumetric light transport with scattering

## Controls (Demo)

- `1-5` - Switch render modes
- `Up/Down` - Adjust max light bounces (1-8)
- `ESC` - Quit

## Light Model

- **Spectral Power Distributions**: 32 wavelength samples (380-780nm)
- **CIE 1931 Color Matching**: XYZ to sRGB conversion
- **Fresnel Equations**: Exact dielectric and Schlick approximation
- **Polarization**: Jones and Mueller calculus for polarizers, retarders
- **Interference**: Coherent superposition with configurable coherence length
- **Diffraction**: Fresnel integrals, Airy patterns, Fraunhofer approximation
- **Volume Scattering**: Absorption and scattering coefficients per material

## Light Bounce Configuration

| Platform | Range | Default |
|----------|-------|---------|
| Mobile   | 1-3   | 3       |
| Desktop  | 1-8   | 8       |

Set at build time: `-Dmax-light-bounces=N`

## Validation & Testing

```bash
# Run all tests
meson test -C build_linux --verbose

# Run benchmarks
./build_linux/bench_wave_optics

# Enable validation layers (default)
./build_linux/raxter-demo
```

## Shader Compilation

Shaders are compiled with `glslc` (from glslang/shaderc):

```bash
# Compile all shaders
for f in shaders/*.comp; do
    glslc "$f" -o "${f%.comp}.comp.spv"
done
```

## License

MIT License - See LICENSE file for details.
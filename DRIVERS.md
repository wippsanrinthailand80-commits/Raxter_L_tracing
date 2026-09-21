# Raxter L Tracing Engine - Vulkan Driver Setup Guide

## Problem
The engine requires **native Vulkan ICD (Installable Client Drivers)** for hardware acceleration. Without proper drivers, it falls back to `llvmpipe` (CPU software rendering) which is 10-100x slower.

## Required Drivers by Platform

### Linux (Desktop/Server)

#### AMD (RDNA/RDNA2/RDNA3)
```bash
# Ubuntu/Debian
sudo apt install mesa-vulkan-drivers libvulkan1 vulkan-tools

# Arch
sudo pacman -S vulkan-radeon lib32-vulkan-radeon vulkan-tools

# Fedora
sudo dnf install mesa-vulkan-drivers vulkan-tools

# Verify
vulkaninfo | grep -E "(deviceName|driverVersion|Vulkan)"
```

#### NVIDIA
```bash
# Ubuntu (proprietary)
sudo apt install nvidia-driver-550 vulkan-tools

# Or open-source (Nouveau - limited Vulkan)
sudo apt install mesa-vulkan-drivers
```

#### Intel (Integrated + Arc)
```bash
# Ubuntu 22.04+
sudo apt install intel-gpu-tools mesa-vulkan-drivers vulkan-tools

# Arc discrete
sudo apt install intel-arc-gpu-tools
```

#### ARM Mali (Midgard/Valhall)
```bash
# Typically provided by SoC vendor
# Rockchip: sudo apt install libmali-valhall-g610-g13p0-gbm
# Generic: check vendor BSP
```

### Android (Adreno / Mali / Xclipse)

#### Adreno (Qualcomm Snapdragon)
```bash
# Device-side: usually pre-installed
# Verify: adb shell dumpsys | grep -i vulkan

# For development: Android NDK includes validation layers
# $ANDROID_NDK/sources/third_party/vulkan/src/build/android/
```

#### Samsung Xclipse (mRDNA - Exynos 2200+)
```bash
# Galaxy S22/S23/S24 series
# Driver: com.samsung.android.vulkan (system app)
# Verify: adb shell pm list packages | grep vulkan
```

#### Mali (Valhall - G78/G715/G720)
```bash
# Usually in vendor partition
# Check: adb shell ls /vendor/lib64/hw/vulkan.mali.so
```

### Driver Discovery Order (Linux)
Vulkan loader searches in order:
1. `$VK_ICD_FILENAMES` (explicit)
2. `/usr/share/vulkan/icd.d/*.json` (system ICDs)
2. `/etc/vulkan/icd.d/*.json` (local ICDs)
3. `$HOME/.local/share/vulkan/icd.d/*.json`

### Verify Installation
```bash
# List all ICDs
vulkaninfo --summary

# Test minimal Vulkan app
vkcube  # from vulkan-tools

# Check for software fallback (BAD)
vulkaninfo | grep "deviceName.*llvmpipe"

# Check for hardware (GOOD)
vulkaninfo | grep -E "(deviceName|driverVersion|API Version)"
```

### Common Issues

| Symptom | Cause | Fix |
|---------|-------|-----|
| `llvmpipe` shown | No ICD found | Install `mesa-vulkan-drivers` |
| `vkCreateInstance` fails | Loader can't find ICD | Check `/usr/share/vulkan/icd.d/` |
| `VK_ERROR_INCOMPATIBLE_DRIVER` | Driver too old | Update GPU driver |
| `VK_ERROR_EXTENSION_NOT_PRESENT` | Missing extension | Update driver / check `vulkaninfo` |

### Android NDK Build - Include Validation Layers
```bash
# In build.gradle or CMakeLists.txt
# Add validation layers for debug builds
# $ANDROID_NDK/sources/third_party/vulkan/src/build/android/
```

### Raxter L Tracing Engine - Runtime Driver Validation
The engine includes runtime validation in `gpu_detect.c`:

```c
// Checks at startup:
gpu_tier tier = gpu_detect_tier(&renderer->vk);
// Warns if llvmpipe detected
if (strstr(props.deviceName, "llvmpipe")) {
    fprintf(stderr, "WARNING: Running on software renderer (llvmpipe)!\n");
    fprintf(stderr, "Install GPU Vulkan drivers for hardware acceleration.\n");
}
```

### CI/CD - Testing on Multiple GPUs
```yaml
# GitHub Actions example
jobs:
  test:
    runs-on: ubuntu-latest
    container:
      image: ubuntu:24.04
    steps:
      - apt update && apt install -y mesa-vulkan-drivers vulkan-tools
      - vulkaninfo --summary
      - ./build_linux/test/test_gpu_detect
```

### Minimum Driver Versions
| GPU Family | Minimum Version | Vulkan Version |
|------------|-----------------|----------------|
| AMD RDNA1  | 20.1+ | 1.2 |
| AMD RDNA2  | 21.1+ | 1.2 |
| AMD RDNA3  | 22.3+ | 1.3 |
| NVIDIA Turing | 450+ | 1.2 |
| NVIDIA Ampere | 460+ | 1.2 |
| NVIDIA Ada | 525+ | 1.3 |
| Intel Arc | 22.3+ | 1.3 |
| Mali Valhall | r30p0+ | 1.1 |
| Adreno 6xx | Q3 2019+ | 1.1 |
| Adreno 7xx | Q1 2022+ | 1.2 |
| Xclipse 920 | 2022+ | 1.1 |

### Troubleshooting Checklist
- [ ] `vulkaninfo` runs without error
- [ ] GPU name is **not** `llvmpipe`
- [ ] `vkcube` runs at >60 FPS
- [ ] `VK_KHR_swapchain` extension present
- [ ] `VK_KHR_storage_buffer_storage_class` supported
- [ ] 16-bit float storage supported (for spectral data)
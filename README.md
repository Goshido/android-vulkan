# android-vulkan

Welcome to _android-vulkan_ source core repository. This project was started as personal hobby. Main purpose of this project is learning and implementing the most recent programming techniques for robust 3D game engines on the _Android_ mobile devices. Two years later the project goals were extended to _3D_ physics engine development.

## Introduction

_android-vulkan_ is 3D engine framework. _android-vulkan_ is dedicated to _Vulkan API_ learning and _3D_ physics engine development.

## Documentation

Usefull documentation is located [here](docs/documentation.md).

## Quick start instructions

Requirements:

* _Windows Vista_+ or _Monjaro KDE_
* _Android Studio Chipmunk | 2021.2.1 Patch 1_
* _Android Studio Gradle Plugin 7.2.1_
* _Android NDK 24.0.8215888 (side by side)_
* _Android SDK 11.0 (API level 30)_
* _Android SDK Build-Tools 33.0.0_
* _Android SDK Platform-Tools 33.0.2_
* _CMake 3.18.1_
* _DirectX Shader Compiler 1.6.2112.10238_
* _Kotlin 1.7.0_
* _Kotlin Gradle plugin 1.7.0_
* _Vulkan Validation Layers_ `6e8fb40b3b364391095e40f894cd3922854a0317`
* _Lua_ `8426d9b4d4df1da3c5b2d759e509ae1c50a86667`
* _Gradle 7.4.2-bin_
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

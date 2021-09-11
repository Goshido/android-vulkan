# android-vulkan

Welcome to _android-vulkan_ source core repository. This project was started as personal hobby. Main purpose of this project is learning and implementing the most recent programming techniques for robust 3D game engines on the _Android_ mobile devices. Two years later the project goals were extended to _3D_ physics engine development.

## Introduction

_android-vulkan_ is 3D engine framework. _android-vulkan_ is dedicated to _Vulkan API_ learning and _3D_ physics engine development.

## Documentation

Usefull documentation is located [here](docs/documentation.md).

## Quick start instructions

Requirements:

* _Windows Vista_+ or _Monjaro KDE_
* _Android Studio Arctic Fox | 2020.3.1 Patch 2_
* _Android Studio Gradle Plugin 7.0.2_
* _Android NDK 23.0.7599858 (side by side)_
* _Android SDK 11.0 (API level 30)_
* _Android SDK Build-Tools 31.0.0_
* _Android SDK Tools 26.1.1_
* _Android SDK Platform-Tools 31.0.3_
* _CMake 3.18.1_
* _DirectX Shader Compiler 1.6.2109.10005_
* _Vulkan Validation Layers_ `63f3cb0aed87278665da8d7d4b966379b442b3fe`
* _Gradle 7.2_
* _Google USB Driver 13_
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon_ _A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

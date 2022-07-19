# android-vulkan

Welcome to _android-vulkan_ source code repository. This project was started as personal hobby. Main purpose of the project is learning and implementing the most recent programming techniques for robust _3D_ game engines on the _Android_ mobile devices. Two years later the project goals were extended to _3D_ physics engine development.

## Introduction

_android-vulkan_ is _3D_ engine framework. _android-vulkan_ is dedicated to _Vulkan API_ learning and _3D_ physics engine development.

## Documentation

Usefull documentation is located [here](docs/documentation.md).

## Quick start instructions

Requirements:

* _Windows Vista_+ or _Monjaro KDE_
* _Android Studio Chipmunk | 2021.2.1 Patch 1_
* _Android Studio Gradle Plugin 7.2.1_
* _Android NDK 25.0.8775105 (side by side)_
* _Android SDK 11.0 (API level 30)_
* _Android SDK Build-Tools 33.0.0_
* _Android SDK Platform-Tools 33.0.2_
* _CMake 3.18.1_
* _DirectX Shader Compiler 1.7.2207.10002_
* _Kotlin 1.7.10_
* _Kotlin Gradle plugin 1.7.10_
* _Vulkan Validation Layers_ `58f83806b0be820246cffa9a31550b7ab856e5d4`
* _Lua_ `8426d9b4d4df1da3c5b2d759e509ae1c50a86667`
* _Gradle 7.5-bin_
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

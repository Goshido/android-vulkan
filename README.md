# android-vulkan

Welcome to _android-vulkan_ source core repository. This project was started as personal hobby. Main purpose of this project is learning and implementing the most recent programming techniques for robust 3D game engines on the _Android_ mobile devices.

## Introduction

_android-vulkan_ is 3D engine framework. _android-vulkan_ is dedicated to _Vulkan API_ learning.

## Documentation

Usefull documentation is located [here](docs/documentation.md).

## Quick start instructions

Requirements:

* _Windows Vista_+
* _Android Studio 4.1.1_
* _Android Studio Gradle Plugin 4.1.1_
* _Android NDK 22.0.6917172 (side by side)_
* _Android SDK 10.0 (API level 29)_
* _Android SDK Build-Tools 30.0.3_
* _Android SDK Tools 26.1.1_
* _Android SDK Platform-Tools 30.0.5_
* _CMake 3.10.2.4988404_
* _DirectX Shader Compiler 1.6.2012.10013_
* _Gradle 6.7_
* _Google USB Driver 13_
* _LLDB 3.1.4508709_
* Real _Android 10_ device with _Vulkan 1.1.108_ support
* [_ARM Neon_ _A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

To begin, clone this repository onto your local drive.

Compile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

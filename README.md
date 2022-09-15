# android-vulkan

Welcome to _android-vulkan_ source code repository. This project was started as personal hobby. Main purpose of the project is learning and implementing the most recent programming techniques for robust _3D_ game engines on the _Android_ mobile devices. Two years later the project goals were extended to _3D_ physics engine development and [_Lua_](https://en.wikipedia.org/wiki/Lua_(programming_language)) embedded scripting language integration.

<img src="./docs/images/preview.png"/>

## Introduction

_android-vulkan_ is _3D_ engine framework. _android-vulkan_ is dedicated to _Vulkan API_ learning, _3D_ physics engine development and [_Lua_](https://en.wikipedia.org/wiki/Lua_(programming_language)) embedded scripting language integration.

## Documentation

Useful documentation is located [here](docs/documentation.md).

## Quick start instructions

### Requirements

* _Windows Vista_+ or _Monjaro KDE_
* _Android Studio Chipmunk | 2021.2.1 Patch 2_
* _Android Studio Gradle Plugin 7.2.2_
* _Android NDK 25.1.8937393 (side by side)_
* _Android SDK 11.0 (API level 30)_
* _Android SDK Build-Tools 33.0.0_
* _Android SDK Platform-Tools 33.0.3_
* _Kotlin 1.7.10_
* _Kotlin Gradle plugin 1.7.10_
* _CMake 3.22.1_
* _Gradle 7.5.1-bin_
* _DirectX Shader Compiler 1.7.2207.10036_
* _Vulkan Validation Layers_ `92ed21f6a188b02dfc62a88c0ac40d5153429704`
* _Lua 5.4.5_ `71bc69c2afaf49ab5f54f3443af9ae70dd1fed06`
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

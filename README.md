# android-vulkan

Welcome to _android-vulkan_ source code repository. This project was started as personal hobby. Main purpose of the project is learning and implementing the most recent programming techniques for robust _3D_ game engines on the _Android_ mobile devices. Two years later the project goals were extended to _3D_ physics engine development, [_Lua_](https://en.wikipedia.org/wiki/Lua_(programming_language)) embedded scripting language integration, spatial sound rendering and _HTML5 + CSS_ rendering system for _UI_.

<img src="./docs/images/preview.png"/>

---

<img src="./docs/images/preview-002.png"/>


## Introduction

_android-vulkan_ is _3D_ engine framework. _android-vulkan_ is dedicated to _Vulkan API_ learning, _3D_ physics engine development, [_Lua_](https://en.wikipedia.org/wiki/Lua_(programming_language)) embedded scripting language integration, spatial sound rendering and _HTML5 + CSS_ rendering system for _UI_.

## Documentation

Useful documentation is located [here](docs/documentation.md).

## Quick start instructions

### Requirements

* _Windows Vista_+ or _Monjaro KDE_
* [_Android Studio Jellyfish | 2023.3.1 Patch 2_](https://developer.android.com/studio)
* _Android Studio Gradle Plugin 8.4.2_
* _Android NDK 26.3.11579264 (side by side)_
* _Minimum _Android SDK_ version: Android 11 (API level 30)_
* _Compile _Android SDK_ version: Android 13 (API level 34)_
* _Android SDK Build-Tools 34.0.0_
* _Android SDK Platform-Tools 35.0.1_
* _Kotlin 2.0.0_
* _Kotlin Gradle plugin 2.0.0_
* _CMake 3.22.1_
* [_Gradle 8.8-bin_](https://services.gradle.org/distributions/)
* [_DirectX Shader Compiler 1.8.2405.10047_](https://github.com/microsoft/DirectXShaderCompiler) `80f6e46bf8cd0e193f90040168e4e48f4e0576aa`
* [_libfreetype 2.13.2_](https://gitlab.freedesktop.org/freetype/freetype) `d41a855aab6d8ce131c465ce01f0f536e0fb2269`
* [_libogg 1.3.5_](https://gitlab.xiph.org/xiph/ogg) `db5c7a49ce7ebda47b15b78471e78fb7f2483e22`
* [_libvorbis 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `84c023699cdf023a32fa4ded32019f194afcdad0`
* [_libvorbisfile 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `84c023699cdf023a32fa4ded32019f194afcdad0`
* [_stb_image 2.30_](https://github.com/nothings/stb) `013ac3beddff3dbffafd5177e7972067cd2b5083`
* [_Vulkan Validation Layers 1.3.288_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `6eaf97e9366755822923147c94e0ee1760a393d5`
* [_Lua 5.5.0_](https://github.com/lua/lua) `bdc85357aa41a9610498232c2cffe7aa191e5cf6`
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

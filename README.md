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
* [_Android Studio Giraffe | 2022.3.1 Patch 2_](https://developer.android.com/studio)
* _Android Studio Gradle Plugin 8.1.2_
* _Android NDK 26.1.10909125 (side by side)_
* _Minimum _Android SDK_ version: Android 11 (API level 30)_
* _Compile _Android SDK_ version: Android 13 (API level 33)_
* _Android SDK Build-Tools 34.0.0_
* _Android SDK Platform-Tools 34.0.5_
* _Kotlin 1.9.10_
* _Kotlin Gradle plugin 1.9.10_
* _CMake 3.22.1_
* [_Gradle 8.4-bin_](https://services.gradle.org/distributions/)
* [_DirectX Shader Compiler 1.7.2308.10173_](https://github.com/microsoft/DirectXShaderCompiler) `c8de4c36111bdb5e9b00a2ab7c61e5edec27c1ec`
* [_libfreetype 2.13.2_](https://gitlab.freedesktop.org/freetype/freetype) `4e61303a3b0e6656c38dd9da9f70d18d7e30585b`
* [_libogg 1.3.5_](https://gitlab.xiph.org/xiph/ogg) `db5c7a49ce7ebda47b15b78471e78fb7f2483e22`
* [_libvorbis 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `84c023699cdf023a32fa4ded32019f194afcdad0`
* [_libvorbisfile 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `84c023699cdf023a32fa4ded32019f194afcdad0`
* [_stb_image 2.28_](https://github.com/nothings/stb) `beebb24b945efdea3b9bba23affb8eb3ba8982e7`
* [_Vulkan Validation Layers_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `6aff75b8564393e3fb52f592f65d16f32517637a`
* [_Lua 5.4.6_](https://github.com/lua/lua) `6baee9ef9d5657ab582c8a4b9f885ec58ed502d0`
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

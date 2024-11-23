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
* [_Android Studio Ladybug | 2024.2.1 Patch 2_](https://developer.android.com/studio)
* _Android Studio Gradle Plugin 8.7.2_
* _Android NDK 27.2.12479018 (side by side)_
* _Minimum _Android SDK_ version: Android 11 (API level 30)_
* _Compile _Android SDK_ version: Android 15 (API level 35)_
* _Android SDK Build-Tools 35.0.0_
* _Android SDK Platform-Tools 35.0.2_
* _Kotlin 2.0.21_
* _Kotlin Gradle plugin 2.0.21_
* _CMake 3.31.0_
* [_Gradle 8.11.1-bin_](https://services.gradle.org/distributions/)
* [_DirectX Shader Compiler 1.8.2407.10117_](https://github.com/microsoft/DirectXShaderCompiler) `848b7c42bd8da13693273513412c0a554c7918a1`
* [_libfreetype 2.13.3_](https://gitlab.freedesktop.org/freetype/freetype) `3f3e3de34ee3613d621b643c58a40b93148e0932`
* [_libogg 1.3.5_](https://gitlab.xiph.org/xiph/ogg) `7cf42ea17aef7bc1b7b21af70724840a96c2e7d0`
* [_libvorbis 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `bb4047de4c05712bf1fd49b9584c360b8e4e0adf`
* [_libvorbisfile 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `bb4047de4c05712bf1fd49b9584c360b8e4e0adf`
* [_stb_image 2.30_](https://github.com/nothings/stb) `5c205738c191bcb0abc65c4febfa9bd25ff35234`
* [_Vulkan Validation Layers 1.3.302_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `736e979cbb2579384ee420dc349de4611bdee3ee`
* [_Lua 5.5.0_](https://github.com/lua/lua) `50c7c915ee2fa239043d5456237f5145d064089b`
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

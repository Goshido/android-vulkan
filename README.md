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

Note desktop operating system requirements apply for builder machine only. In other words you can build, deploy and debug the project out of the box as soon as you are able to install proper [_Android Studio_](https://developer.android.com/studio).

The canonical way is to use real _Android_ device via _USB_ connection. _Android_ emulator is never tested and there are no plans to support it.

Pay attention that all 3<sup>rd</sup> party libraries already prebuild and project has all needed header files. You **_do not need_** to build them by yourself. Same applies to _SPIR-V_ shader blobs and game assets.

* _Windows 10+_ or _Monjaro KDE_
* [_Android Studio Ladybug | 2024.2.1 Patch 3_](https://developer.android.com/studio)
* _Android Studio Gradle Plugin 8.7.3_
* _Android NDK 27.2.12479018 (side by side)_
* _Minimum _Android SDK_ version: Android 11 (API level 30)_
* _Compile _Android SDK_ version: Android 15 (API level 35)_
* _Android SDK Build-Tools 35.0.0_
* _Android SDK Platform-Tools 35.0.2_
* _Kotlin 2.1.0_
* _Kotlin Gradle plugin 2.1.0_
* _CMake 3.31.1_
* [_PowerShell 7.5.2_](https://github.com/PowerShell/PowerShell/releases/tag/v7.5.2)
* [_Gradle 8.12-bin_](https://services.gradle.org/distributions/)
* [_DirectX Shader Compiler 1.8.2407.10120_](https://github.com/microsoft/DirectXShaderCompiler) `d39324e0635130e834a68e33b0c603cf5fc9fb4f`
* [_libfreetype 2.13.3_](https://gitlab.freedesktop.org/freetype/freetype) `59320b2d3c2584ac01914ed0deff64bcc8fb23b2`
* [_libogg 1.3.5_](https://gitlab.xiph.org/xiph/ogg) `7cf42ea17aef7bc1b7b21af70724840a96c2e7d0`
* [_libvorbis 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `bb4047de4c05712bf1fd49b9584c360b8e4e0adf`
* [_libvorbisfile 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `bb4047de4c05712bf1fd49b9584c360b8e4e0adf`
* [_stb_image 2.30_](https://github.com/nothings/stb) `5c205738c191bcb0abc65c4febfa9bd25ff35234`
* [_Vulkan Validation Layers 1.4.303_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `e1b6492a0b1491f82f3f6c5a2054b7bdc711ad5f`
* [_Lua 5.5.0_](https://github.com/lua/lua) `2fccd3c4b9043a77b0be2da74c8c7ffe31ce28c7`
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

### Building manual

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

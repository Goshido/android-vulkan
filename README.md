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

Pay attention that all 3<sup>rd</sup> party libraries are prebuilt already and project has all needed header files. You **_do not need_** to build them by yourself. Same applies to _SPIR-V_ shader blobs and game assets.

* _Windows 10+_
* [_Android Studio Narwhal 4 Feature Drop | 2025.1.4_](https://developer.android.com/studio)
* _Android Studio Gradle Plugin 8.13.0_
* _Android NDK 29.0.14206865 (side by side)_
* _Minimum _Android SDK_ version: Android 11 (API level 30)_
* _Compile _Android SDK_ version: Android 16 (API level 36.1)_
* _Android SDK Build-Tools 36.1.0_
* _Android SDK Platform-Tools 36.0.0_
* _Kotlin 2.2.20_
* _Kotlin Gradle plugin 2.2.20_
* _CMake 4.1.2_
* [_PowerShell 7.5.4_](https://github.com/PowerShell/PowerShell/releases/tag/v7.5.4)
* [_Gradle 9.1.0-bin_](https://services.gradle.org/distributions/)
* [_DirectX Shader Compiler 1.8.2505.10149_](https://github.com/microsoft/DirectXShaderCompiler) `0bf8434bc3b57a0b99477162dfe54673d9b5153b`
* [_libfreetype 2.14.1_](https://gitlab.freedesktop.org/freetype/freetype) `4334f009e7d20789cc7ee1224290ea1e22a17b5b`
* [_libogg 1.3.6_](https://gitlab.xiph.org/xiph/ogg) `0288fadac3ac62d453409dfc83e9c4ab617d2472`
* [_libvorbis 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `851cce991da34adf5e1f3132588683758a6369ec`
* [_libvorbisfile 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `851cce991da34adf5e1f3132588683758a6369ec`
* [_stb_image 2.30_](https://github.com/nothings/stb) `fede005abaf93d9d7f3a679d1999b2db341b360f`
* [_Vulkan Validation Layers 1.4.329_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `62d79257ac9b93ba3f6fa7507fb172cb9cf8e7ff`
* [_Lua 5.5.0_](https://github.com/lua/lua) `9ea06e61f20ae34974226074fc6123dbb54a07c2`
* Real _Android 11_ device with _Vulkan 1.1_ support

### Building manual

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

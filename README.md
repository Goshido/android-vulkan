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

* _Windows 10+_ or _Monjaro KDE_
* [_Android Studio Narwhal | 2025.1.1_](https://developer.android.com/studio)
* _Android Studio Gradle Plugin 8.11.0_
* _Android NDK 28.1.13356709 (side by side)_
* _Minimum _Android SDK_ version: Android 11 (API level 30)_
* _Compile _Android SDK_ version: Android 16 (API level 36)_
* _Android SDK Build-Tools 36.0.0_
* _Android SDK Platform-Tools 36.0.0_
* _Kotlin 2.2.0_
* _Kotlin Gradle plugin 2.2.0_
* _CMake 4.0.2_
* [_PowerShell 7.5.2_](https://github.com/PowerShell/PowerShell/releases/tag/v7.5.2)
* [_Gradle 8.14.2-bin_](https://services.gradle.org/distributions/)
* [_DirectX Shader Compiler 1.8.2505.10062_](https://github.com/microsoft/DirectXShaderCompiler) `94abfe972ad839185965f670329bcf33cd7bccbd`
* [_libfreetype 2.13.3_](https://gitlab.freedesktop.org/freetype/freetype) `58be4879c5d3840315f037dca44e92384113f8f9`
* [_libogg 1.3.6_](https://gitlab.xiph.org/xiph/ogg) `fe20a3ed04b9e4de8d2b4c753077d9a7c2a7e588`
* [_libvorbis 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `43bbff0141028e58d476c1d5fd45dd5573db576d`
* [_libvorbisfile 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `43bbff0141028e58d476c1d5fd45dd5573db576d`
* [_stb_image 2.30_](https://github.com/nothings/stb) `f58f558c120e9b32c217290b80bad1a0729fbb2c`
* [_Vulkan Validation Layers 1.4.320_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `7d29258f5e5bb765057929b217d9a9662315e610`
* [_Lua 5.5.0_](https://github.com/lua/lua) `cfce6f4b20afe85ede2182b3df3ab2bfcdb0e692`
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

### Building manual

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

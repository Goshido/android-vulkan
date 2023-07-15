# android-vulkan

Welcome to _android-vulkan_ source code repository. This project was started as personal hobby. Main purpose of the project is learning and implementing the most recent programming techniques for robust _3D_ game engines on the _Android_ mobile devices. Two years later the project goals were extended to _3D_ physics engine development, [_Lua_](https://en.wikipedia.org/wiki/Lua_(programming_language)) embedded scripting language integration and spatial sound rendering.

<img src="./docs/images/preview.png"/>

---

<img src="./docs/images/preview-002.png"/>


## Introduction

_android-vulkan_ is _3D_ engine framework. _android-vulkan_ is dedicated to _Vulkan API_ learning, _3D_ physics engine development, [_Lua_](https://en.wikipedia.org/wiki/Lua_(programming_language)) embedded scripting language integration and spatial sound rendering.

## Documentation

Useful documentation is located [here](docs/documentation.md).

## Quick start instructions

### Requirements

* _Windows Vista_+ or _Monjaro KDE_
* [_Android Studio Flamingo | 2022.2.1 Patch 2_](https://developer.android.com/studio)
* _Android Studio Gradle Plugin 8.0.2_
* _Android NDK 25.2.9519653 (side by side)_
* _Android SDK 11.0 (API level 30)_
* _Android SDK Build-Tools 34.0.0_
* _Android SDK Platform-Tools 34.0.4_
* _Kotlin 1.8.20_
* _Kotlin Gradle plugin 1.8.20_
* _CMake 3.22.1_
* [_Gradle 8.0.2-bin_](https://services.gradle.org/distributions/)
* [_DirectX Shader Compiler 1.7.2212.10216_](https://github.com/microsoft/DirectXShaderCompiler) `2096d1b8054d1de864924ef0c5009f533597b9d4`
* [_libfreetype 2.13.1_](https://gitlab.freedesktop.org/freetype/freetype) `e4586d960f339cf75e2e0b34aee30a0ed8353c0d`
* [_libogg 1.3.5_](https://gitlab.xiph.org/xiph/ogg) `db5c7a49ce7ebda47b15b78471e78fb7f2483e22`
* [_libvorbis 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `84c023699cdf023a32fa4ded32019f194afcdad0`
* [_libvorbisfile 1.3.7_](https://gitlab.xiph.org/xiph/vorbis) `84c023699cdf023a32fa4ded32019f194afcdad0`
* [_stb_image 2.28_](https://github.com/nothings/stb) `5736b15f7ea0ffb08dd38af21067c314d6a3aae9`
* [_Vulkan Validation Layers_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `766e34d398895f430377701b6e9354e4774007c4`
* [_Lua 5.4.6_](https://github.com/lua/lua) `ea39042e13645f63713425c05cc9ee4cfdcf0a40`
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

To begin, clone this repository onto your local drive.

_Optional_: Recompile project shaders to _SPIR-V_ representation via _DirectX Shader Compiler_. See manual [here](docs/shader-compilation.md).

Create and setup _Android_ certificate. See manual [here](docs/release-build.md).

Next step is to compile project via _Android Studio IDE_ as usual.

## Controller support

_XBOX ONE S_ controller is supported via _Bluetooth_ connection. Other controllers are not tested.

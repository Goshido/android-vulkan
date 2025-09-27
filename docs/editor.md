# Editor

## <a id="table-of-content">Table of content</a>

- [_Quick start instructions_](#quck-start)
  - [_Requirements_](#requirements)
  - [_Building_](#building)
- [_Configurations_](#configurations)
  - [_RenderDoc integration_](#renderdoc-integration)
  - [_NVIDIA Nsight Graphics integration_](#nsight-integration)
  - [_NVIDIA Aftermath integration_](#aftermath-integration)
  - [_PIX integration_](#pix-integration)
  - [_ASAN integration_](#asan-integration)
  - [_Debug build_](#debug-build)
  - [_Release build_](#release-build)

## <a id="quck-start">Quick start instructions</a>

### <a id="requirements">Requirements</a>

* _Windows 11_+
* _Visual Studio 2022 Community 17.12.1_
  - Workloads: Desktop development with C++
  - Individual components
    - MSBuild
    - _MSVC v143 - VS 2022 C++ x64/x86 build tools (v14.42-17.12)_
    - Windows Universal CRT SDK
    - C++ core features
    - Windows 11 SDK (10.0.26100.0)
    - Windows Universal C Runtime
* [_RenderDoc v1.40_](https://renderdoc.org/)
* [_NVIDIA Nsight Graphics 2025.4.1.0 (build 36508989) (public-release)_](https://developer.nvidia.com/nsight-graphics)
* [_PIX 2409.23 / WinPixEventRuntime.\(dll|lib\) 1.0.240308001_](https://devblogs.microsoft.com/pix/download/)
* [_DirectX Shader Compiler 1.8.2505.10062_](https://github.com/microsoft/DirectXShaderCompiler) `94abfe972ad839185965f670329bcf33cd7bccbd`
* [_libfreetype 2.13.3_](https://gitlab.freedesktop.org/freetype/freetype) `58be4879c5d3840315f037dca44e92384113f8f9`
* [_stb_image 2.30_](https://github.com/nothings/stb) `f58f558c120e9b32c217290b80bad1a0729fbb2c`
* [_Vulkan Validation Layers 1.4.320_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `7d29258f5e5bb765057929b217d9a9662315e610`

[↬ table of content ⇧](#table-of-content)

### <a id="building">Building</a>

To begin you have to run script

`<repo>tools\editor\scripts\prepare-environment.ps1`

This script gonna create symbolic links for:

- asset directories
- [_PIX_](https://devblogs.microsoft.com/pix/) tooling library for _CPU_ tracing
- 3rd-party libraries

Next step is to compile project via _Visual Studio_ as usual.

[↬ table of content ⇧](#table-of-content)

## <a id="configurations">Configurations</a>

### <a id="renderdoc-integration">_RenderDoc_ integration</a>

First step is to select _RenderDoc_ build configuration:

<img src="./images/editor-renderdoc-config.png">

Next step is to [compile _HLSL_ code into _SPIR-V_ with shader sources](./shader-compilation.md#spirv-sources).

Under the hood it's used [_RenderDoc_ compatibility compile options](./renderdoc-integration.md).

The build artifacts will be located in directory:

`<repo>\tools\editor\bin\RenderDoc`

Now you will be able to use [_RenderDoc_](https://renderdoc.org/) for debugging:

<img src="./images/editor-renderdoc.png">

**Note:** It's expected that application will crash in case of running without _RenderDoc_. For performance reasons internal implementation does not check that the following debug functions are not `nullptr`.

- `vkCmdBeginDebugUtilsLabelEXT`
- `vkCmdEndDebugUtilsLabelEXT`
- `vkSetDebugUtilsObjectNameEXT`

The _RenderDoc Vulkan_ layer provides those functions.

[↬ table of content ⇧](#table-of-content)

### <a id="nsight-integration">_NVIDIA Nsight Graphics_ integration</a>

First step is to select _Nsight_ build configuration:

<img src="./images/editor-nsight-config.png">

Next step is to [compile _HLSL_ code into _SPIR-V_ with shader sources](./shader-compilation.md#spirv-sources).

The build artifacts will be located in directory:

`<repo>\tools\editor\bin\Nsight`

Now you will be able to use [_NVIDIA Nsight Graphics_](https://developer.nvidia.com/nsight-graphics) for debugging:

Graphics debugger:

<img src="./images/editor-nsight-debugger.png">

_GPU_ profiler:

<img src="./images/editor-nsight-profiler.png">

**Note:** It's expected that application could crash in case of running outside _NVIDIA Nsight Graphics_. For performance reasons internal implementation does not check that the following debug functions are not `nullptr`.

- `vkCmdBeginDebugUtilsLabelEXT`
- `vkCmdEndDebugUtilsLabelEXT`
- `vkSetDebugUtilsObjectNameEXT`

The _NVIDIA Nsight Graphics Vulkan_ layer provides those functions.

[↬ table of content ⇧](#table-of-content)

### <a id="aftermath-integration">_NVIDIA Aftermath_ integration</a>

First step is to select _Aftermath_ build configuration:

<img src="./images/editor-aftermath-config.png">

Next step is to [compile _HLSL_ code into _SPIR-V_ with shader sources](./shader-compilation.md#spirv-sources).

The build artifacts will be located in directory:

`<repo>\tools\editor\bin\Aftermath`

Now you will be able to use [_NVIDIA Aftermath_](https://developer.nvidia.com/nsight-aftermath) for debugging:

<img src="./images/editor-aftermath.png">

ℹ️ Pay attention that [_NVIDIA Nsight Graphics_](https://developer.nvidia.com/nsight-graphics) already contains [_NVIDIA Aftermath_](https://developer.nvidia.com/nsight-aftermath) components. So all you need is to activate [_NVIDIA Aftermath_](https://developer.nvidia.com/nsight-aftermath) tool and wait for application crash.

[↬ table of content ⇧](#table-of-content)

### <a id="pix-integration">_PIX_ integration</a>

First step is to select _Profile_ build configuration:

<img src="./images/editor-profile-config.png">

The build artifacts will be located in directory:

`<repo>\tools\editor\bin\Profile`

Now you will able to use _PIX_ for _CPU_ tracing.

<img src="./images/pix.png">

[↬ table of content ⇧](#table-of-content)

### <a id="asan-integration">_ASAN_ integration</a>

First step is to select _Release + ASAN_ build configuration:

<img src="./images/editor-asan-config.png">

The build artifacts will be located in directory:

`<repo>\tools\editor\bin\Release + ASAN`

Now you will able to use [_ASAN_ features](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170).

[↬ table of content ⇧](#table-of-content)

### <a id="debug-build">Debug build</a>

First step is to select _Debug_ build configuration:

<img src="./images/editor-debug-config.png">

The build artifacts will be located in directory:

`<repo>\tools\editor\bin\Debug`

Build activates: _VVL_ with core, best practices and sync validation features.

[↬ table of content ⇧](#table-of-content)

### <a id="release-build">Release build</a>

First step is to select _Release_ build configuration:

<img src="./images/editor-release-config.png">

The build artifacts will be located in directory:

`<repo>\tools\editor\bin\Release`

Build features:

- all _CPU_ optimization are enabled
- _VVL_ are disabled

[↬ table of content ⇧](#table-of-content)

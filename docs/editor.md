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
* _Visual Studio 2026 Community 18.0.0_
  - Workloads: Desktop development with C++
  - Individual components
    - MSBuild
    - _MSVC Build Tools v14.50 for x64/x86_
    - Windows Universal CRT SDK
    - C++ core features
    - Windows 11 SDK (10.0.26100.6901)
    - Windows Universal C Runtime
* [_PowerShell 7.5.4_](https://github.com/PowerShell/PowerShell/releases/tag/v7.5.4)
* [_RenderDoc v1.41_](https://renderdoc.org/)
* [_NVIDIA Nsight Graphics 2025.4.1.0 (build 36508989) (public-release)_](https://developer.nvidia.com/nsight-graphics)
* [_PIX 2509.25 / WinPixEventRuntime.\(dll|lib\) 1.0.240308001_](https://devblogs.microsoft.com/pix/download/)
* [_DirectX Shader Compiler 1.8.2505.10178_](https://github.com/microsoft/DirectXShaderCompiler) `b1cf2cad8f19f2ce733bd108e63485b33fbd4774`
* [_libfreetype 2.14.1_](https://gitlab.freedesktop.org/freetype/freetype) `fc9cc5038e05edceec3d0f605415540ac76163e9`
* [_stb_image 2.30_](https://github.com/nothings/stb) `f1c79c02822848a9bed4315b12c8c8f3761e1296`
* [_Vulkan SDK 1.4.328.1_](https://vulkan.lunarg.com/sdk/home)
* [_Vulkan Validation Layers 1.4.332_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `fc24b1981d8e11ed35ee2af0d9f43b92285e38a8`

[↬ table of content ⇧](#table-of-content)

### <a id="building">Building</a>

To begin you have to run script

`<repo>tools\editor\scripts\prepare-environment.ps1`

This script gonna create symbolic links for:

- asset directories
- [_PIX_](https://devblogs.microsoft.com/pix/) tooling library for _CPU_ tracing
- 3rd-party libraries

Open and compile `<repo>\tools\editor\editor.sln` solution via _Visual Studio_ as usual.

Last step is to edit _Visual Studio_ active configuration and specify working directory for `editor.exe`:

**_`editor` project properties_** → **_Debugging_** → **_Working Directory_** → **_Set to_** `$(OutputPath)`

ℹ️ Note: You need to do this once for every configuration.

That's it.

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

- all _CPU_ optimizations are enabled
- _VVL_ are disabled

[↬ table of content ⇧](#table-of-content)

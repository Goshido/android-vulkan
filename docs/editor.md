# Editor

## <a id="table-of-content">Table of content</a>

- [_Quick start instructions_](#quck-start)
  - [_Requirements_](#requirements)
  - [_Building_](#building)
- [_Configurations_](#configurations)
  - [_RendeDoc integration_](#renderdoc-integration)
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
* [_PIX 2409.23 / WinPixEventRuntime.\(dll|lib\) 1.0.240308001_](https://devblogs.microsoft.com/pix/download/)
* [_stb_image 2.30_](https://github.com/nothings/stb) `f7f20f39fe4f206c6f19e26ebfef7b261ee59ee4`
* [_libfreetype 2.13.3_](https://gitlab.freedesktop.org/freetype/freetype) `0ae7e607370cc66218ccfacf5de4db8a35424c2f`
* [_Vulkan Validation Layers 1.3.299_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) `edb909e193061350f0274345463e7b9747e109ba`

[↬ table of content ⇧](#table-of-content)

### <a id="building">Building</a>

To begin you have to run script

`<repo>tools\editor\scripts\prepare-environment.ps1`

This scripts gonna create symbolic links for:

- asset directories
- [_PIX_](https://devblogs.microsoft.com/pix/) tooling library for _CPU_ tracing
- 3rd-party libraries

Next step is to compile project via _Visual Studio_ as usual.

[↬ table of content ⇧](#table-of-content)

## <a id="configurations">Configurations</a>

### <a id="renderdoc-integration">_RendeDoc_ integration</a>

First step is to select _RenderDoc_ build configuration:

<img src="./images/editor-renderdoc-config.png">

Next step is to [compile _HLSL_ code into _SPIR-V_ with shader sources](./shader-compilation.md##spirv-sources).

Under the hood it's used _RenderDoc_ compatibility is achieved using [the following](./renderdoc-integration.md).

The build artefacts will be located in directory:

`<repo>\tools\editor\bin\RenderDoc`

Now you will be able to use [_RenderDoc_](https://renderdoc.org/) for debugging:

<img src="./images/editor-renderdoc.png">

**Note:** It's expected that application will crash in case of running without _RenderDoc_. For performance reasons internal implementation does not check that the following debug functions are not `nullptr`.

- `vkCmdBeginDebugUtilsLabelEXT`
- `vkCmdEndDebugUtilsLabelEXT`
- `vkSetDebugUtilsObjectNameEXT`

The _RenderDoc Vulkan_ layer provides those functions.

[↬ table of content ⇧](#table-of-content)

### <a id="pix-integration">_PIX_ integration</a>

First step is to select _Profile_ build configuration:

<img src="./images/editor-profile-config.png">

The build artefacts will be located in directory:

`<repo>\tools\editor\bin\Profile`

Now you will able to use _PIX_ for _CPU_ tracing.

<img src="./images/pix.png">

[↬ table of content ⇧](#table-of-content)

### <a id="asan-integration">_ASAN_ integration</a>

First step is to select _Release + ASAN_ build configuration:

<img src="./images/editor-asan-config.png">

The build artefacts will be located in directory:

`<repo>\tools\editor\bin\Release + ASAN`

Now you will able to use [_ASAN_ features](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170).

[↬ table of content ⇧](#table-of-content)

### <a id="debug-build">Debug build</a>

First step is to select _Debug_ build configuration:

<img src="./images/editor-debug-config.png">

The build artefacts will be located in directory:

`<repo>\tools\editor\bin\Debug`

Build activates: _VVL_ with core, best practices and sync validation features.

[↬ table of content ⇧](#table-of-content)

### <a id="release-build">Release build</a>

First step is to select _Release_ build configuration:

<img src="./images/editor-release-config.png">

The build artefacts will be located in directory:

`<repo>\tools\editor\bin\Release`

Build features:

- all _CPU_ optimization are enabled
- _VVL_ are disabled

[↬ table of content ⇧](#table-of-content)

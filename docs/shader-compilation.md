
# Shader compilation

## <a id="table-of-content">Table of content</a>

- [_Compile tool_](#compile-tool)
- [_Compile and deploy vertex shader module_](#compile-vs)
- [_Compile and deploy fragment shader module_](#compile-fs)
- [_Compile and deploy compute shader module_](#compile-cs)
- [_SPIR-V disassembler via DXC_](#spirv-disassm)
- [_SPIR-V with shader sources_](#spirv-sources)
- [_Compilation automation \(Windows OS only\)_](#automation)
- [_Reported issues_](#issues)

## <a id="compile-tool">Compile tool</a>

Current project is using [_DXC_ compiler](https://github.com/microsoft/DirectXShaderCompiler) to produce _SPIR-V_ binary representations. The manual is valid against **_DXC v1.8.2505.10062_**.

The _android-vulkan_ project is using _HLSL_ shader language as high level programming language. All shader sources are located in the following directory:

`<android-vulkan directory>/app/src/main/hlsl`

**Note:** the resulting shaders must be placed in the following directories:

⁘ Core:

`<android-vulkan directory>/app/src/main/assets/shaders`

⁘ Editor:

`<android-vulkan directory>/tools/editor/editor-assets/shaders`

**Note:** the project is using _DirectX_ alignment layout rules for _SPIR-V_ packing. It's achieved via [_DXC_'s flag](https://github.com/microsoft/DirectXShaderCompiler/blob/master/docs/SPIR-V.rst#memory-layout-rules):

```txt
-fvk-use-dx-layout
```

**Note:** the project is using `float16` type support because it is critical type for performance on mobile devices. To enable this feature you have to specify the following _DXC_ flag:

```txt
-enable-16bit-types
```

**Note:** the project is using `Vulkan 1.1` as target platform. To enable this feature you have to specify the following _DXC_ flag:

```txt
-fspv-target-env=vulkan1.1
```

[↬ table of content ⇧](#table-of-content)

## <a id="compile-vs">Compile and deploy vertex shader module</a>

```txt
dxc.exe                                                         ^
    -HV 2021                                                    ^
    -spirv                                                      ^
    -fvk-use-dx-layout                                          ^
    -fspv-reduce-load-size                                      ^
    -fspv-target-env=vulkan1.1                                  ^
    -ffinite-math-only                                          ^
    -enable-16bit-types                                         ^
    -WX                                                         ^
    -O3                                                         ^
    -T vs_6_9                                                   ^
    -E VS                                                       ^
    -I <android-vulkan directory>\tools\editor\include          ^
    -I <android-vulkan directory>\app\src\main\cpp\include      ^
    -Fo <core or editor shader directory>\<file name>.vs.spv    ^
    <file name>.vs
```

[↬ table of content ⇧](#table-of-content)

## <a id="compile-fs">Compile and deploy fragment shader module</a>

```txt
dxc.exe                                                         ^
    -HV 2021                                                    ^
    -spirv                                                      ^
    -fvk-use-dx-layout                                          ^
    -fspv-reduce-load-size                                      ^
    -fspv-target-env=vulkan1.1                                  ^
    -ffinite-math-only                                          ^
    -enable-16bit-types                                         ^
    -WX                                                         ^
    -O3                                                         ^
    -T ps_6_9                                                   ^
    -E PS                                                       ^
    -I <android-vulkan directory>\tools\editor\include          ^
    -I <android-vulkan directory>\app\src\main\cpp\include      ^
    -Fo <core or editor shader directory>\<file name>.ps.spv    ^
    <file name>.ps
```

[↬ table of content ⇧](#table-of-content)

## <a id="compile-cs">Compile and deploy compute shader module</a>

```txt
dxc.exe                                                         ^
    -HV 2021                                                    ^
    -spirv                                                      ^
    -fvk-use-dx-layout                                          ^
    -fspv-reduce-load-size                                      ^
    -fspv-target-env=vulkan1.1                                  ^
    -ffinite-math-only                                          ^
    -enable-16bit-types                                         ^
    -WX                                                         ^
    -O3                                                         ^
    -T cs_6_9                                                   ^
    -E CS                                                       ^
    -I <android-vulkan directory>\tools\editor\include          ^
    -I <android-vulkan directory>\app\src\main\cpp\include      ^
    -Fo <core or editor shader directory>\<file name>.cs.spv    ^
    <file name>.cs
```

[↬ table of content ⇧](#table-of-content)

## <a id="spirv-disassm">_SPIR-V_ disassembler via _DXC_</a>

The [_DXC_](https://github.com/microsoft/DirectXShaderCompiler) has special flag to print out _SPIR-V_ disassembler code of the binary representation. Use the following command:

```txt
-Fc <file name>
```

[↬ table of content ⇧](#table-of-content)

## <a id="spirv-sources">_SPIR-V_ with shader sources</a>

The [_DXC_](https://github.com/microsoft/DirectXShaderCompiler) has special flags to embed shader sources into _SPIR-V_. Use the following command:

```txt
-Od
-fspv-debug=vulkan-with-source
```

[↬ table of content ⇧](#table-of-content)

## <a id="automation">Compilation automation (_Windows OS only_)</a>

To compile all shaders to _SPIR-V_ representation you can invoke special `make-all.ps1` script which is located here:

```txt
<android-vulkan directory>/scripts/windows
```

But **before** that you have to specify root directory where _dxc.exe_ is located on you system via environment variable `ANDROID_VULKAN_DXC_ROOT`

For example:

Variable name | Value
--- | ---
`ANDROID_VULKAN_DXC_ROOT` | `D:\Development\DXC-builds\Release-2025-06-28\bin`

[↬ table of content ⇧](#table-of-content)

## <a id="issues">Reported issues</a>

Name | Link | Status
--- | --- | ---
SPIR-V degradation 1.7.2207.10069 compare to 1.7.2207.10036 | [#4714](https://github.com/microsoft/DirectXShaderCompiler/issues/4714) | ✔️ Fixed
SPIR-V degradation 1.7.2212.10204 compare to 1.7.2212.10142 | [#5342](https://github.com/microsoft/DirectXShaderCompiler/issues/5342) | ✔️ Fixed
`-fspv-debug=vulkan-with-source` crash issue | [#5441](https://github.com/microsoft/DirectXShaderCompiler/issues/5441) | ✔️ Fixed
Problem with `mad` intrinsic | [#5608](https://github.com/microsoft/DirectXShaderCompiler/issues/5608) | ✔️ Fixed
[SPIR-V] Define-only include files are missing in `-fspv-debug=vulkan-with-source` | [#6907](https://github.com/microsoft/DirectXShaderCompiler/issues/6907) | ✔️ Fixed
[SPIR-V] Non semantic shader information issue (-fspv-debug=vulkan-with-source) | [#6939](https://github.com/microsoft/DirectXShaderCompiler/issues/6939) | ✔️ Fixed
[SPIR-V] Compute shader output into float16_t RWTexture2D | [#7595](https://github.com/microsoft/DirectXShaderCompiler/issues/7595) | 🛡️ not an issue

[↬ table of content ⇧](#table-of-content)

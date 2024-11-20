
# Shader compilation

## Compile tool

Current project is using [_DXC_ compiler](https://github.com/microsoft/DirectXShaderCompiler) to produce _SPIR-V_ binary representations. The manual is valid against **_DXC v1.8.2407.10115_**.

The _android-vulkan_ project is using _HLSL_ shader language as high level programming language. All shader sources are located in the following directory:

`<android-vulkan directory>/app/src/main/hlsl`

**Note:** the resulting shaders must be placed in the following directory:

`<android-vulkan directory>/app/src/main/assets/shaders/`

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

## Compile and deploy vertex shader module

```txt
dxc.exe                                                                                 ^
    -HV 2021                                                                            ^
    -spirv                                                                              ^
    -fvk-use-dx-layout                                                                  ^
    -fspv-reduce-load-size                                                              ^
    -fspv-target-env=vulkan1.1                                                          ^
    -enable-16bit-types                                                                 ^
    -WX                                                                                 ^
    -O3                                                                                 ^
    -T vs_6_8                                                                           ^
    -E VS                                                                               ^
    -I <android-vulkan directory>\tools\editor\hlsl                                     ^
    -I <android-vulkan directory>\tools\editor\include                                  ^
    -I <android-vulkan directory>\app\src\main\hlsl                                     ^
    -I <android-vulkan directory>\app\src\main\cpp\include                              ^
    -Fo <android-vulkan directory>\app\src\main\assets\shaders\<file name>.vs.spv       ^
    <file name>.vs
```

## Compile and deploy fragment shader module

```txt
dxc.exe                                                                                 ^
    -HV 2021                                                                            ^
    -spirv                                                                              ^
    -fvk-use-dx-layout                                                                  ^
    -fspv-reduce-load-size                                                              ^
    -fspv-target-env=vulkan1.1                                                          ^
    -enable-16bit-types                                                                 ^
    -WX                                                                                 ^
    -O3                                                                                 ^
    -T ps_6_8                                                                           ^
    -E PS                                                                               ^
    -I <android-vulkan directory>\tools\editor\hlsl                                     ^
    -I <android-vulkan directory>\tools\editor\include                                  ^
    -I <android-vulkan directory>\app\src\main\hlsl                                     ^
    -I <android-vulkan directory>\app\src\main\cpp\include                              ^
    -Fo <android-vulkan directory>\app\src\main\assets\shaders\<file name>.ps.spv       ^
    <file name>.ps
```

## Compile and deploy compute shader module

```txt
dxc.exe                                                                                 ^
    -HV 2021                                                                            ^
    -spirv                                                                              ^
    -fvk-use-dx-layout                                                                  ^
    -fspv-reduce-load-size                                                              ^
    -fspv-target-env=vulkan1.1                                                          ^
    -enable-16bit-types                                                                 ^
    -WX                                                                                 ^
    -O3                                                                                 ^
    -T cs_6_8                                                                           ^
    -E CS                                                                               ^
    -I <android-vulkan directory>\tools\editor\hlsl                                     ^
    -I <android-vulkan directory>\tools\editor\include                                  ^
    -I <android-vulkan directory>\app\src\main\hlsl                                     ^
    -I <android-vulkan directory>\app\src\main\cpp\include                              ^
    -Fo <android-vulkan directory>\app\src\main\assets\shaders\<file name>.cs.spv       ^
    <file name>.cs
```

## _SPIR-V_ disassembler via _DXC_

The [_DXC_](https://github.com/microsoft/DirectXShaderCompiler) has special flag to print out _SPIR-V_ disassembler code of the binary representation. Use the following command:

```txt
-Fc <file name>
```

## Compilation automation (_Windows OS only_)

To compile all shaders to _SPIR-V_ representation you can invoke special `make-all.ps1` script which is located here:

```txt
<android-vulkan directory>/scripts/windows
```

But **before** that you have to specify root directory where _dxc.exe_ is located on you system via environment variable `ANDROID_VULKAN_DXC_ROOT`

For example:

Variable name | Value
--- | ---
`ANDROID_VULKAN_DXC_ROOT` | `D:\Development\DXC-builds\Release-2024-09-28\bin`

### Reported issues

Name | Link | Status
--- | --- | ---
SPIR-V degradation 1.7.2207.10069 compare to 1.7.2207.10036 | [#4714](https://github.com/microsoft/DirectXShaderCompiler/issues/4714) | ✔️ Fixed
SPIR-V degradation 1.7.2212.10204 compare to 1.7.2212.10142 | [#5342](https://github.com/microsoft/DirectXShaderCompiler/issues/5342) | ✔️ Fixed
`-fspv-debug=vulkan-with-source` crash issue | [#5441](https://github.com/microsoft/DirectXShaderCompiler/issues/5441) | ✔️ Fixed
Problem with `mad` intrinsic | [#5608](https://github.com/microsoft/DirectXShaderCompiler/issues/5608) | ✔️ Fixed
[SPIR-V] Define-only include files are missing in `-fspv-debug=vulkan-with-source` | [#6907](https://github.com/microsoft/DirectXShaderCompiler/issues/6907) | ✔️ Fixed
[SPIR-V] Non semantic shader information issue (-fspv-debug=vulkan-with-source) | [#6939](https://github.com/microsoft/DirectXShaderCompiler/issues/6939) | ⚠️ Submitted

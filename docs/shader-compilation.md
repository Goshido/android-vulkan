# Shader compilation

## Compile tool

Current project is using [_DXC_ compiler](https://github.com/microsoft/DirectXShaderCompiler) to produce _SPIR-V_ binary representations.

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

## Compile and deploy vertex shader module

```txt
dxc.exe -spirv -WX -O3 -fvk-use-dx-layout -enable-16bit-types -T vs_6_6 -E VS -I <android-vulkan directory>\app\src\main\hlsl -Fo <android-vulkan directory>\app\src\main\assets\shaders\<file name>-vs.spv <file name>.vs
```

## Compile and deploy fragment shader module

```txt
dxc.exe -spirv -WX -O3 -fvk-use-dx-layout -enable-16bit-types -T ps_6_6 -E PS -I <android-vulkan directory>\app\src\main\hlsl -Fo <android-vulkan directory>\app\src\main\assets\shaders\<file name>-ps.spv <file name>.ps
```

## _SPIR-V_ disassembler via _DXC_

The [_DXC_](https://github.com/microsoft/DirectXShaderCompiler) has special flag to print out _SPIR-V_ disassembler code of the binary representation. Use the following command:

```txt
-Fc <file name>
```

## Compilation automation (_Windows OS only_)

To compile all shaders to _SPIR-V_ representation you can invoke special `make-all.bat` script which is located here:

```txt
<android-vulkan directory>/app/src/main/hlsl/build-system/windows
```

But **before** that you have to specify root directory where _dxc.exe_ is located on you system via environment varible `ANDROID_VULKAN_DXC_ROOT`

For example:

Variable name | Value
--- | ---
`ANDROID_VULKAN_DXC_ROOT` | `D:\Development\DXC-builds\Release-2020-05-23\bin`

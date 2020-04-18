# Shader compilation

## Compile tool

Current project is using [_DXC_ compiler](https://github.com/microsoft/DirectXShaderCompiler) to produce _SPIR-V_ binary representations.

The _android-vulkan_ project is using _HLSL_ shader language as high level programming language. All shader sources are located in the following directory:

`<android-vulkan directory>/app/src/main/hlsl`

Note the resulting shaders must be placed in the following directory:

`<android-vulkan directory>/app/src/main/assets/shaders/`

## Compile and deploy vertex shader module

```txt
dxc.exe -spirv -WX -O3 -T vs_6_6 -E VS -Fo <android-vulkan directory>\app\src\main\assets\shaders\<file name>-vs.spv <file name>.vs
```

## Compile and deploy fragment shader module

```txt
dxc.exe -spirv -WX -O3 -T ps_6_6 -E PS -Fo <android-vulkan directory>\app\src\main\assets\shaders\<file name>-ps.spv <file name>.ps
```
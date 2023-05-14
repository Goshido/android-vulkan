$global:PIVOT_DIRECTORY = ".\..\..\.."
$global:DXC = "${Env:ANDROID_VULKAN_DXC_ROOT}\dxc.exe"

$global:BASE_SRC = "${PIVOT_DIRECTORY}\hlsl"
$global:BASE_DST = "${PIVOT_DIRECTORY}\assets\shaders"

$global:FLAGS =
    "-HV", "2021",
    "-spirv",
    "-fvk-use-dx-layout",
    "-fspv-reduce-load-size",
    "-fspv-target-env=vulkan1.1",
    "-enable-16bit-types",
    "-WX",
    "-O3",
    "-I", "${PIVOT_DIRECTORY}\hlsl",
    "-I", "${PIVOT_DIRECTORY}\cpp\include\pbr"

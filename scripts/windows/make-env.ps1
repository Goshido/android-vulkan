$embedSources = $args[ 0 ]

[string] $editorDirectory = "tools\editor"
[string] $EDITOR_HLSL_DIRECTORY = "$editorDirectory\hlsl"
[string] $EDITOR_SHADER_DIRECTORY = "$editorDirectory\editor-assets\shaders"

[string] $mobileDirectory = "app\src\main"
[string] $MOBILE_HLSL_DIRECTORY = "$mobileDirectory\hlsl"
[string] $MOBILE_SHADER_DIRECTORY = "$mobileDirectory\assets\shaders"

[string] $DXC = "$Env:ANDROID_VULKAN_DXC_ROOT\dxc.exe"
[string] $HLSL_PROFILE = "6_8"

$global:FLAGS =
    "-HV", "2021",
    "-spirv",
    "-fvk-use-dx-layout",
    "-fspv-reduce-load-size",
    "-fspv-target-env=vulkan1.1",
    "-enable-16bit-types",
    "-WX",
    "-I", $EDITOR_HLSL_DIRECTORY,
    "-I", "$editorDirectory\include",
    "-I", $MOBILE_HLSL_DIRECTORY,
    "-I", "$mobileDirectory\cpp\include"

if ( $embedSources )
{
    $FLAGS +=
        "-Od",
        "-fspv-debug=vulkan-with-source"

    return
}

$FLAGS += "-O3"

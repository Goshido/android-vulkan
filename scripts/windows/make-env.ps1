$embedSources = $args[ 0 ]

[string] $editorDirectory = "tools\editor"
[string] $global:EDITOR_HLSL_DIRECTORY = "$editorDirectory\hlsl"
[string] $global:EDITOR_SHADER_DIRECTORY = "$editorDirectory\editor-assets\shaders"

[string] $mobileDirectory = "app\src\main"
[string] $global:MOBILE_HLSL_DIRECTORY = "$mobileDirectory\hlsl"
[string] $global:MOBILE_SHADER_DIRECTORY = "$mobileDirectory\assets\shaders"

[string] $global:DXC = "$Env:ANDROID_VULKAN_DXC_ROOT\dxc.exe"
[string] $global:HLSL_PROFILE = "6_8"

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
    "-I", "$mobileDirectory\cpp\include\pbr"

if ( $embedSources )
{
    $global:FLAGS +=
        "-Od",
        "-fspv-debug=vulkan-with-source"

    return
}

$global:FLAGS += "-O3"

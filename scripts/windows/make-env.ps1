$embedSources = $args[ 0 ]

[string] $editorDirectory = "tools\editor"
[string] $EDITOR_HLSL_DIRECTORY = "$editorDirectory\hlsl"
[string] $EDITOR_SHADER_DIRECTORY = "$editorDirectory\editor-assets\shaders"

[string] $mobileDirectory = "app\src\main"
[string] $MOBILE_HLSL_DIRECTORY = "$mobileDirectory\hlsl"
[string] $MOBILE_SHADER_DIRECTORY = "$mobileDirectory\assets\shaders"

[string] $DXC = "$Env:ANDROID_VULKAN_DXC_ROOT\dxc.exe"
[string] $HLSL_PROFILE = "6_9"

$global:FLAGS =
    "-HV", "2021",
    "-spirv",
    "-fvk-use-dx-layout",
    "-fspv-reduce-load-size",
    "-fspv-target-env=vulkan1.1",
    "-ffinite-math-only",
    "-enable-16bit-types",
    "-WX",
    "-I", "$editorDirectory\include",
    "-I", "$mobileDirectory\cpp\include"

function Resolve-Type-HLSL
{
    param
    (
        [Parameter(Mandatory)]
        [string] $Src
    )

    if ( $Src.EndsWith("ps.hlsl") )
    {
        return [PSCustomObject]@{
            _entryPoint = "PS"
            _profile = "ps_$HLSL_PROFILE"
        }
    }

    if ( $Src.EndsWith("vs.hlsl") )
    {
        return [PSCustomObject]@{
            _entryPoint = "VS"
            _profile = "vs_$HLSL_PROFILE"
        }
    }

    return [PSCustomObject]@{
        _entryPoint = "CS"
        _profile = "cs_$HLSL_PROFILE"
    }
}

if ( $embedSources )
{
    $FLAGS +=
        "-Od",
        "-fspv-debug=vulkan-with-source"

    return
}

$FLAGS += "-O3"

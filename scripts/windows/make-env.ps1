$embedSources = $args[ 0 ]

[string] $editorDirectory = "tools\editor"
[string] $EDITOR_HLSL_DIRECTORY = "$editorDirectory\hlsl"
[string] $EDITOR_SHADER_DIRECTORY = "$editorDirectory\editor-assets\shaders"

[string] $coreDirectory = "app\src\main"
[string] $CORE_HLSL_DIRECTORY = "$coreDirectory\hlsl"
[string] $ANDROID_HLSL_DIRECTORY = "$CORE_HLSL_DIRECTORY\android"
[string] $WINDOWS_HLSL_DIRECTORY = "$CORE_HLSL_DIRECTORY\windows"

[string] $CORE_SHADER_DIRECTORY = "$coreDirectory\assets\shaders"
[string] $ANDROID_SHADER_DIRECTORY = "$CORE_SHADER_DIRECTORY\android"
[string] $WINDOWS_SHADER_DIRECTORY = "$CORE_SHADER_DIRECTORY\windows"

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
    "-I", "$CORE_HLSL_DIRECTORY",
    "-I", "$editorDirectory\include",
    "-I", "$coreDirectory\cpp\include"

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
        "-Zi",
        "-fspv-debug=vulkan-with-source"

    return
}

$FLAGS += "-O3"

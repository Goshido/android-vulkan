[string] $src = $args[ 0 ]
[string] $dst = $args[ 1 ]

[PSCustomObject] $type = Resolve-Type-HLSL                                                                             `
    -Src $src

if ($FLAGS -ccontains "-fspv-debug=vulkan-with-source")
{
    # Workaround for https://github.com/microsoft/DirectXShaderCompiler/issues/8543
    [string] $workaround = "$CORE_HLSL_DIRECTORY\preprocess\__8543-workaround.hlsl"

    & $DXC $FLAGS @(
        "-E", $type._entryPoint,
        "-T", $type._profile,
        "/P",
        "/Fi", $workaround,
        $src
    )

    $src = $workaround
    (Get-Content -LiteralPath $src) -cnotlike "#line *" | Set-Content -LiteralPath $src
}

[string[]] $params = @(
    "-E", $type._entryPoint,
    "-T", $type._profile,
    "-Fo", $dst
)

if (Test-Windows-Platform -SPV $src)
{
    $params += @(
        "-fvk-bind-resource-heap",
        "0",
        "0",
        "-fvk-bind-sampler-heap",
        "1",
        "0"
    )
}

$params += @(
    $src
)

Write-Host "Compiling:" $DXC $FLAGS $params
& $DXC $FLAGS $params
Write-Host "Done"

Write-Host ""

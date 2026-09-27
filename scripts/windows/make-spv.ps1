[string] $src = $args[ 0 ]
[string] $dst = $args[ 1 ]

[PSCustomObject] $type = Resolve-Type-HLSL                                                                             `
    -Src $src

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

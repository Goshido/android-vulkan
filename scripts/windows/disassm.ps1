Clear-Host
. scripts\windows\make-env.ps1 $false

[string] $src = $args[ 0 ]

[PSCustomObject] $type = Resolve-Type-HLSL                                                                             `
    -Src $src

[string[]] $params = @(
    "-E", $type._entryPoint,
    "-T", $type._profile,
    "-Fc", "$CORE_HLSL_DIRECTORY\disassm\blob.spvasm"
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

Write-Host "SPIR-V Disassemble:" $DXC $FLAGS $params
& $DXC $FLAGS $params
Write-Host "Done"

Write-Host ""

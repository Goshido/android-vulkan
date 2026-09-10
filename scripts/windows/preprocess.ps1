Clear-Host
. scripts\windows\make-env.ps1 $false

[string] $src = $args[ 0 ]

[PSCustomObject] $type = Resolve-Type-HLSL                                                                             `
    -Src $src

[string] $targetSource = "$CORE_HLSL_DIRECTORY\preprocess\source.hlsl"

$params = @(
    "-E", $type._entryPoint,
    "-T", $type._profile,
    "/P",
    "/Fi", $targetSource,
    $src
)

Write-Host "Preprocessing:" $DXC $FLAGS $params
& $DXC $FLAGS $params

Write-Host ""
Write-Host "Done"

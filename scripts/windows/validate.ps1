[string] $src = $args[ 0 ]

#-----------------------------------------------------------------------------------------------------------------------

Clear-Host
. scripts\windows\make-env.ps1 $false

[PSCustomObject] $type = Resolve-Type-HLSL                  `
    -Src $src

[string] $targetBlob = "$CORE_HLSL_DIRECTORY\validation\blob.spv"

$params =
    "-E", $type._entryPoint,
    "-T", $type._profile,
    "-Fo", $targetBlob,
    $src

Write-Host "Validating:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Validating: spirv-val $targetBlob"

spirv-val $targetBlob

Write-Host "Done"
Write-Host

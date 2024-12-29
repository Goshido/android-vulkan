[string] $src = $args[ 0 ]

#-----------------------------------------------------------------------------------------------------------------------

Clear-Host
. scripts\windows\make-env.ps1 $false

[PSCustomObject] $type = Resolve-Type-HLSL      `
    -Src $src

$params =
    "-E", $type._entryPoint,
    "-T", $type._profile,
    "-Fo", "$MOBILE_HLSL_DIRECTORY\validation\blob.spv",
    $src

Write-Host "Validating:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

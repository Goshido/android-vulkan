[string] $src = $args[ 0 ]

#-----------------------------------------------------------------------------------------------------------------------

Clear-Host
. scripts\windows\make-env.ps1 $false

[PSCustomObject] $type = Resolve-Type-HLSL      `
    -Src $src

$params =
    "-E", $type._entryPoint,
    "-T", $type._profile,
    "-Fc", "$MOBILE_HLSL_DIRECTORY\disassm\blob.spvasm",
    $src

Write-Host "SPIR-V Disassemble:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

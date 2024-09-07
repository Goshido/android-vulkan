[string] $src = $args[ 0 ]
[string] $entryPoint = $args[ 1 ]
[string] $profilePrefix = $args[ 2 ]

Clear-Host
scripts\windows\make-env.ps1 $false

$params =
    "-E", $entryPoint,
    "-T", "${profilePrefix}_$HLSL_PROFILE",
    "-Fc", "$MOBILE_HLSL_DIRECTORY\disassm\blob.spvasm",
    $src

Write-Host "SPIR-V Disassemble:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

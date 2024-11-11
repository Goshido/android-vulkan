[string] $src = $args[ 0 ]
[string] $entryPoint = $args[ 1 ]
[string] $profilePrefix = $args[ 2 ]

Clear-Host
. scripts\windows\make-env.ps1 $false

$params =
    "-E", $entryPoint,
    "-T", "${profilePrefix}_$HLSL_PROFILE",
    "-Fo", "$MOBILE_HLSL_DIRECTORY\validation\blob.spv",
    $src

Write-Host "Validating:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

[string] $entryPoint = $args[ 0 ]
[string] $profilePrefix = $args[ 1 ]
[string] $src = $args[ 2 ]
[string] $dst = $args[ 3 ]

$params =
    "-E", $entryPoint,
    "-T", "${profilePrefix}_$HLSL_PROFILE",
    "-Fo", $dst,
    $src

Write-Host "Compiling:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

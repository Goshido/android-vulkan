[string] $src = $args[ 0 ]
[string] $dst = $args[ 1 ]

[PSCustomObject] $type = Resolve-Type-HLSL      `
    -Src $src

$params =
    "-E", $type._entryPoint,
    "-T", $type._profile,
    "-Fo", $dst,
    $src

Write-Host "Compiling:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

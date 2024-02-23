$base = $args[ 0 ]

$params =
    "-E", "VS",
    "-T", "vs_6_8",
    "-Fo", "${BASE_DST}\${base}.vs.spv",
    "${BASE_SRC}\${base}.vs"

Write-Host "Compiling:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

$base = $args[ 0 ]

$params =
    "-E", "PS",
    "-T", "ps_6_7",
    "-Fo", "${BASE_DST}\${base}-ps.spv",
    "${BASE_SRC}\${base}.ps"

Write-Host "Compiling:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

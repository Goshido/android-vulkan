$base = $args[ 0 ]

$params =
    "-E", "CS",
    "-T", "cs_6_8",
    "-Fo", "${BASE_DST}\${base}.cs.spv",
    "${BASE_SRC}\${base}.cs"

Write-Host "Compiling:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

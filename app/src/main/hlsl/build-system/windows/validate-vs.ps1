$source = $args[ 0 ]

Clear-Host
.\make-env.ps1 $false

$params =
    "-E", "VS",
    "-T", "vs_6_8",
    "-Fo", "${BASE_SRC}\validation\vs.spv",
    "${source}"

Write-Host "Validating:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

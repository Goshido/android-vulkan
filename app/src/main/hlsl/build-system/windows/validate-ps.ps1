$source = $args[ 0 ]

Clear-Host
.\make-env.ps1 $false

$params =
    "-E", "PS",
    "-T", "ps_6_8",
    "-Fo", "${BASE_SRC}\validation\ps.spv",
    "${source}"

Write-Host "Validating:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

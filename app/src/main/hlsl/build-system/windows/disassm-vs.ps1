$source = $args[ 0 ]

Clear-Host
.\make-env.ps1 $false

$params =
    "-E", "VS",
    "-T", "vs_6_8",
    "-Fc", "${BASE_SRC}\disassm\blob.txt",
    "${source}"

Write-Host "SPIR-V Disassemble:" $DXC $FLAGS $params

& $DXC $FLAGS $params

Write-Host "Done"
Write-Host

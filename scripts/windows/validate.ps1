[string[]] $SPIRV_VAL_FLAGS = @(
    "--scalar-block-layout",
    "--target-env", "vulkan1.1spv1.4"
)

#-----------------------------------------------------------------------------------------------------------------------

Clear-Host
. scripts\windows\make-env.ps1 $false

[string] $src = $args[ 0 ]

[PSCustomObject] $type = Resolve-Type-HLSL                                                                             `
    -Src $src

[string] $targetBlob = "$CORE_HLSL_DIRECTORY\validation\blob.spv"

$params = @(
    "-E", $type._entryPoint,
    "-T", $type._profile,
    "-Fo", $targetBlob,
    $src
)

Write-Host "Compiling:" $DXC $FLAGS $params
& $DXC $FLAGS $params
Write-Host "Done"

Write-Host ""

Write-Host "Validating: spirv-val" $SPIRV_VAL_FLAGS $targetBlob
spirv-val $SPIRV_VAL_FLAGS $targetBlob
Write-Host "Done"

Write-Host ""

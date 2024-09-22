function AV-Create-Directory
{
    param
    (
        [Parameter ( Mandatory )]
        [string] $Path
    )

    if ( !( Test-Path $Path ) )
    {
        New-Item                                        `
            -ItemType Directory                         `
            -Path $Path
    }
}

function AV-Create-Symlink
{
    param
    (
        [Parameter ( Mandatory )]
        [string] $Path,

        [Parameter ( Mandatory )]
        [string] $Target
    )

    if ( !( Test-Path $Path ) )
    {
        New-Item                                        `
            -ItemType SymbolicLink                      `
            -Path $Path                                 `
            -Target $Target
    }
}

#-----------------------------------------------------------------------------------------------------------------------

[string] $projectDir = Get-Item ( "$PSScriptRoot\..\..\.." )
[string] $editorDir = "$projectDir\tools\editor"

AV-Create-Symlink                                       `
    -Path "$editorDir\assets"                           `
    -Target "$projectDir\app\src\main\assets"

$configs = @(
    "Debug",
    "Debug - GPU select",
    "Profile",
    "Release",
    "Release + ASAN",
    "RenderDoc"
)

$binDir = "$editorDir\bin"
$libDir = "$editorDir\libs\Release"

foreach ( $config in $configs )
{
    $dstDir = "$binDir\$config"

    AV-Create-Directory                                 `
        -Path $dstDir

    AV-Create-Symlink                                   `
        -Path "$dstDir\freetype.dll"                    `
        -Target "$libDir\freetype.dll"
}

AV-Create-Symlink                                       `
    -Path "$binDir\Profile\WinPixEventRuntime.dll"      `
    -Target "$libDir\WinPixEventRuntime.dll"

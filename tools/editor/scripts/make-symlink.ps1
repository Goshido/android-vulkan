[string] $projectDir = Get-Item ( "$PSScriptRoot\..\..\.." )
[string] $dstDir = "$projectDir\tools\editor\assets"

if ( !( Test-Path $dstDir ) )
{
    New-Item                                            `
        -ItemType SymbolicLink                          `
        -Path $dstDir                                   `
        -Target "$projectDir\app\src\main\assets"
}

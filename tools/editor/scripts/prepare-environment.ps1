Clear-Host

$invokeArgs = @(
    "-Command",
    "&",
    "`"$PSScriptRoot\make-symlink.ps1`""
)

Start-Process                       `
    -Verb runAs                     `
    -ArgumentList $invokeArgs       `
    powershell

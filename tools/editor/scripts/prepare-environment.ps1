Clear-Host

$invokeArgs = @(
    "-Command",
    "&",
    "$PSScriptRoot\make-symlinks.ps1"
)

Start-Process                       `
    -Verb runAs                     `
    -ArgumentList $invokeArgs       `
    powershell

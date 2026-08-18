param(
    [string]$File = "",
    [switch]$Interactive,
    [switch]$Check,
    [switch]$Tokens,
    [switch]$Help
)

$workspaceRoot = $PSScriptRoot
$gotoExe = Join-Path $workspaceRoot "bin\goto.exe"

if (-not (Test-Path $gotoExe)) {
    Write-Host "Binário não encontrado. Compilando SDK primeiro..." -ForegroundColor Yellow
    & "$workspaceRoot\build.ps1"
}

if (-not (Test-Path $gotoExe)) {
    Write-Error "Executável não encontrado em $gotoExe"
    exit 1
}

$argsList = @()
if ($Help) {
    & "$gotoExe" --help
    exit 0
}
if ($Check) {
    $argsList += "--check"
}
if ($Tokens) {
    $argsList += "--tokens"
}

if ($Interactive -or (-not $File -and $argsList.Count -eq 0)) {
    & "$gotoExe" --repl
} else {
    if ($File) {
        $argsList += $File
    }
    & "$gotoExe" $argsList
}

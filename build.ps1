param(
    [string]$Config = "Debug",
    [switch]$Rebuild,
    [switch]$RunTests,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$workspaceRoot = $PSScriptRoot
$binDir = Join-Path $workspaceRoot "bin"
$objDir = Join-Path $workspaceRoot "build\obj"

Write-Host "=== GoTo SDK Build System (C++20 MSVC) ===" -ForegroundColor Cyan

if ($Clean -or $Rebuild) {
    if (Test-Path $binDir) { Remove-Item -Recurse -Force $binDir }
    if (Test-Path "$workspaceRoot\build") { Remove-Item -Recurse -Force "$workspaceRoot\build" }
    Get-ChildItem $workspaceRoot -Include *.obj, *.ilk, *.pdb, *.exe -Recurse -ErrorAction SilentlyContinue | Remove-Item -Force
    if ($Clean) {
        Write-Host "Ambiente limpo com sucesso!" -ForegroundColor Green
        exit 0
    }
}

if (-not (Test-Path $binDir)) { New-Item -ItemType Directory -Path $binDir | Out-Null }
if (-not (Test-Path $objDir)) { New-Item -ItemType Directory -Path $objDir | Out-Null }

# Locate vcvarsall.bat
$vcvarsPaths = @(
    "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvarsall.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
)

$vcvarsBat = $null
foreach ($path in $vcvarsPaths) {
    if (Test-Path $path) {
        $vcvarsBat = $path
        break
    }
}

if (-not $vcvarsBat) {
    Write-Error "vcvarsall.bat não encontrado! Verifique a instalação do Visual Studio C++."
    exit 1
}

Write-Host "[1/3] Usando compilador MSVC via: $vcvarsBat" -ForegroundColor Green

$coreSources = "src\value.cpp src\token.cpp src\lexer.cpp src\ast.cpp src\parser.cpp src\environment.cpp src\interpreter.cpp src\sdk.cpp"
$cliSources = "src\main.cpp $coreSources"
$testSources = "tests\test_main.cpp tests\test_lexer.cpp tests\test_parser.cpp tests\test_values.cpp tests\test_control_flow.cpp tests\test_narrative.cpp $coreSources"

$optFlags = if ($Config -eq "Release") { "/O2 /DNDEBUG" } else { "/Od /Z7" }

Write-Host "[2/3] Compilando CLI goto.exe..." -ForegroundColor Green
$cmdCli = "call `"$vcvarsBat`" x64 >nul && cl.exe /nologo /EHsc /std:c++20 /utf-8 /W4 $optFlags /I include /Fe:`"$binDir\goto.exe`" /Fo:`"$objDir/`" $cliSources"
cmd /c $cmdCli
if ($LASTEXITCODE -ne 0) {
    Write-Error "Falha ao compilar goto.exe!"
    exit $LASTEXITCODE
}

Write-Host "[3/3] Compilando suite de testes goto_tests.exe..." -ForegroundColor Green
$cmdTests = "call `"$vcvarsBat`" x64 >nul && cl.exe /nologo /EHsc /std:c++20 /utf-8 /W4 $optFlags /I include /Fe:`"$binDir\goto_tests.exe`" /Fo:`"$objDir/`" $testSources"
cmd /c $cmdTests
if ($LASTEXITCODE -ne 0) {
    Write-Error "Falha ao compilar goto_tests.exe!"
    exit $LASTEXITCODE
}

Write-Host "`n[SUCESSO] Compilação concluída com sucesso em: $binDir" -ForegroundColor Green

if ($RunTests) {
    Write-Host "`nExecutando Testes Unitários..." -ForegroundColor Cyan
    & "$binDir\goto_tests.exe"
}

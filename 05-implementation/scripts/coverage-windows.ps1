#!/usr/bin/env powershell
# Windows Coverage Generation Script using OpenCppCoverage
# Requires: OpenCppCoverage installed (choco install opencppcoverage)
#
# Usage: .\scripts\coverage-windows.ps1
# Output: coverage.xml (Cobertura format compatible with CI)

param(
    [string]$BuildDir = "build\coverage",
    [string]$TestExecutable = "Debug\*.exe",
    [string]$SourceDir = "src",
    [string]$OutputXml = "coverage.xml",
    [switch]$Html = $false
)

# Ensure we're in the implementation directory
Push-Location $PSScriptRoot\..

Write-Host "=== Windows Coverage Generation with OpenCppCoverage ===" -ForegroundColor Green

# Check if OpenCppCoverage is available
try {
    $opencppVersion = & "OpenCppCoverage" "--help" 2>$null | Select-String "OpenCppCoverage"
    Write-Host "✅ Found: $opencppVersion" -ForegroundColor Green
} catch {
    Write-Host "❌ OpenCppCoverage not found. Install with:" -ForegroundColor Red
    Write-Host "   choco install opencppcoverage" -ForegroundColor Yellow  
    Write-Host "   or download from: https://github.com/OpenCppCoverage/OpenCppCoverage/releases" -ForegroundColor Yellow
    Pop-Location
    exit 1
}

# Build with coverage flags
Write-Host "📦 Building with MSVC coverage instrumentation..." -ForegroundColor Blue
if (!(Test-Path $BuildDir)) {
    cmake -B $BuildDir -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DENABLE_TESTING=ON -DENABLE_SANITIZERS=OFF
}
cmake --build $BuildDir --config Debug

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Build failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Find test executables
$testExes = Get-ChildItem -Path "$BuildDir\Debug" -Name "*.exe" | Where-Object { $_ -like "*test*" -or $_ -like "*smoke*" }
if ($testExes.Count -eq 0) {
    Write-Host "❌ No test executables found in $BuildDir\Debug" -ForegroundColor Red
    Pop-Location
    exit 1
}

Write-Host "🔍 Found test executables: $($testExes -join ', ')" -ForegroundColor Blue

# Generate coverage for each test executable
$coverageArgs = @(
    "--sources", $(Resolve-Path $SourceDir).Path,
    "--export_type", "cobertura:$OutputXml",
    "--modules", "$BuildDir\Debug"
)

if ($Html) {
    $coverageArgs += "--export_type", "html:coverage_html"
}

foreach ($testExe in $testExes) {
    $testPath = Join-Path "$BuildDir\Debug" $testExe
    Write-Host "🧪 Running coverage for: $testExe" -ForegroundColor Blue
    
    $cmd = @("OpenCppCoverage") + $coverageArgs + @("--", $testPath)
    Write-Host "Command: $($cmd -join ' ')" -ForegroundColor Gray
    
    try {
        & $cmd[0] $cmd[1..($cmd.Length-1)]
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ Coverage generated successfully" -ForegroundColor Green
            break
        } else {
            Write-Host "⚠️  Coverage command returned exit code $LASTEXITCODE" -ForegroundColor Yellow
        }
    } catch {
        Write-Host "❌ Coverage generation failed: $($_.Exception.Message)" -ForegroundColor Red
    }
}

# Verify output
if (Test-Path $OutputXml) {
    $xmlContent = Get-Content $OutputXml -Raw
    if ($xmlContent -match 'line-rate="([0-9.]+)"') {
        $coverage = [math]::Round([double]$matches[1] * 100, 2)
        Write-Host "📊 Line Coverage: $coverage%" -ForegroundColor $(if ($coverage -gt 0) { "Green" } else { "Yellow" })
    }
    Write-Host "📁 Coverage report: $(Resolve-Path $OutputXml)" -ForegroundColor Blue
    
    if ($Html -and (Test-Path "coverage_html")) {
        Write-Host "🌐 HTML report: $(Resolve-Path "coverage_html\index.html")" -ForegroundColor Blue
    }
} else {
    Write-Host "❌ Coverage XML not generated" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location
Write-Host "✅ Windows coverage generation complete!" -ForegroundColor Green
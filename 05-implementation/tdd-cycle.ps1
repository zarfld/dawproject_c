#!/usr/bin/env pwsh
# TDD Red-Green-Refactor Cycle Script for Windows PowerShell
# This script demonstrates the TDD workflow for Phase 05 implementation

param(
    [Parameter()]
    [string]$Phase = "red",
    [Parameter()]
    [switch]$Clean = $false
)

Write-Host "=== DAW Project C++ Library - TDD Cycle ===" -ForegroundColor Cyan
Write-Host "Phase 05 Implementation using Test-Driven Development" -ForegroundColor Green

# Set error handling
$ErrorActionPreference = "Stop"

# Get script location
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RootDir = $ScriptDir
$BuildDir = Join-Path $RootDir "build_tdd"
$TestDir = Join-Path $RootDir "tests\unit"

Write-Host "Root Directory: $RootDir" -ForegroundColor Yellow
Write-Host "Build Directory: $BuildDir" -ForegroundColor Yellow

# Clean build directory if requested
if ($Clean -or $Phase -eq "red") {
    Write-Host "`n=== CLEANING BUILD DIRECTORY ===" -ForegroundColor Magenta
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
        Write-Host "Cleaned build directory" -ForegroundColor Green
    }
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
    Write-Host "Created build directory" -ForegroundColor Green
}

# Change to build directory  
Set-Location $BuildDir

try {
    switch ($Phase) {
        "red" {
            Write-Host "`n=== TDD RED PHASE: Running Tests That Should Fail ===" -ForegroundColor Red
            Write-Host "Goal: Write failing tests that specify desired behavior" -ForegroundColor Yellow
            
            # Configure with CMake
            Write-Host "`nConfiguring CMake..." -ForegroundColor Cyan
            cmake -G "Visual Studio 17 2022" -A x64 `
                  -DCMAKE_BUILD_TYPE=Debug `
                  -DENABLE_TESTING=ON `
                  $TestDir
                  
            if ($LASTEXITCODE -ne 0) {
                throw "CMake configuration failed"
            }
            
            # Build the test
            Write-Host "`nBuilding test executable..." -ForegroundColor Cyan
            cmake --build . --config Debug
            
            if ($LASTEXITCODE -ne 0) {
                throw "Build failed - this is expected in RED phase if interfaces aren't implemented"
            }
            
            # Run tests (expect failures)
            Write-Host "`nRunning tests (expecting failures)..." -ForegroundColor Red
            ctest --output-on-failure --build-config Debug
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host "`nWARNING: All tests passed in RED phase!" -ForegroundColor Yellow
                Write-Host "This may indicate missing tests or incomplete test scenarios." -ForegroundColor Yellow
            } else {
                Write-Host "`nEXPECTED: Tests failed in RED phase" -ForegroundColor Green
                Write-Host "This confirms our tests specify behavior not yet implemented." -ForegroundColor Green
            }
            
            Write-Host "`n=== RED PHASE COMPLETE ===" -ForegroundColor Red
            Write-Host "Next: Run with -Phase green to implement minimal code to pass tests" -ForegroundColor Yellow
        }
        
        "green" {
            Write-Host "`n=== TDD GREEN PHASE: Making Tests Pass ===" -ForegroundColor Green
            Write-Host "Goal: Write minimal code to make failing tests pass" -ForegroundColor Yellow
            
            Write-Host "`nGREEN phase implementation not yet available" -ForegroundColor Yellow
            Write-Host "This phase requires implementing the actual file system interface" -ForegroundColor Yellow
            Write-Host "Following XP Simple Design principles:" -ForegroundColor Cyan
            Write-Host "- Write only enough code to make tests pass" -ForegroundColor White
            Write-Host "- No speculative features" -ForegroundColor White  
            Write-Host "- Keep it simple and focused" -ForegroundColor White
        }
        
        "refactor" {
            Write-Host "`n=== TDD REFACTOR PHASE: Improving Design ===" -ForegroundColor Blue
            Write-Host "Goal: Improve code design while keeping all tests green" -ForegroundColor Yellow
            
            Write-Host "`nREFACTOR phase not yet available" -ForegroundColor Yellow
            Write-Host "This phase requires existing working implementation" -ForegroundColor Yellow
            Write-Host "Refactoring activities:" -ForegroundColor Cyan
            Write-Host "- Remove code duplication" -ForegroundColor White
            Write-Host "- Improve naming and clarity" -ForegroundColor White
            Write-Host "- Optimize performance if needed" -ForegroundColor White
            Write-Host "- Ensure SOLID principles compliance" -ForegroundColor White
        }
        
        default {
            Write-Host "Unknown phase: $Phase" -ForegroundColor Red
            Write-Host "Valid phases: red, green, refactor" -ForegroundColor Yellow
            exit 1
        }
    }
    
    Write-Host "`n=== TDD CYCLE STATUS ===" -ForegroundColor Cyan
    Write-Host "Current Phase: $Phase" -ForegroundColor White
    Write-Host "Build Directory: $BuildDir" -ForegroundColor White
    Write-Host "Test Files: Created and ready" -ForegroundColor Green
    Write-Host "Interface Definitions: Complete" -ForegroundColor Green
    
    if ($Phase -eq "red") {
        Write-Host "`nNext Steps:" -ForegroundColor Yellow
        Write-Host "1. Review failing test output to understand requirements" -ForegroundColor White
        Write-Host "2. Implement minimal IFileSystem concrete class" -ForegroundColor White
        Write-Host "3. Run: .\tdd-cycle.ps1 -Phase green" -ForegroundColor White
    }
    
} catch {
    Write-Host "`nERROR: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Stack Trace: $($_.ScriptStackTrace)" -ForegroundColor Red
    exit 1
} finally {
    # Return to original directory
    Set-Location $ScriptDir
}

Write-Host "`n=== TDD CYCLE COMPLETE ===" -ForegroundColor Cyan
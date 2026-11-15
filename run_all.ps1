# Automated Run Script for Generic Compiler Phase 1
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "   COMPILER PROJECT PHASE 1 - AUTOMATED RUN SCRIPT      " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan

$rootDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $rootDir

# 1. CMake Build
Write-Host "`n[1/5] Building project with CMake..." -ForegroundColor Yellow
cmake -B build -G "MinGW Makefiles"
cmake --build build

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

# 2. Run Test Suites
Write-Host "`n[2/5] Running automated test suites..." -ForegroundColor Yellow
$tests = @("test_step0", "test_step1", "test_step2", "test_step3", "test_step4")
foreach ($t in $tests) {
    Write-Host "Running $t..." -ForegroundColor DarkCyan
    & ".\build\$t.exe"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Test $t failed!" -ForegroundColor Red
        exit 1
    }
}

# 3. Run Main Lexer Generator CLI
Write-Host "`n[3/5] Running Main Lexer Generator CLI on sample_program.java..." -ForegroundColor Yellow
& ".\build\lexer_generator.exe"

# Copy generated outputs to docs folder as well
Copy-Item ".\build\minimal_dfa_table.txt" ".\docs\" -Force
Copy-Item ".\build\minimal_dfa_table.csv" ".\docs\" -Force

# 4. Run Flex Bonus Tool (flex-2.5.4a-1-bin)
Write-Host "`n[4/5] Running Flex Bonus Tool (flex-2.5.4a)..." -ForegroundColor Yellow
Push-Location ".\flex"
& ".\flex-2.5.4a-1-bin\bin\flex.exe" "lexer.l"
gcc -O2 "lex.yy.c" -o "flex_lexer.exe"
& ".\flex_lexer.exe" "..\tests\sample_program.java"
Pop-Location

# 5. Generate PDF Report
Write-Host "`n[5/5] Generating PDF report..." -ForegroundColor Yellow
if (Test-Path ".\scripts\generate_pdf.py") {
    python ".\scripts\generate_pdf.py"
}

Write-Host "`n========================================================" -ForegroundColor Green
Write-Host "   ALL PIPELINES EXECUTED SUCCESSFULLY!                 " -ForegroundColor Green
Write-Host "========================================================" -ForegroundColor Green
Write-Host "Outputs generated:"
Write-Host "  - build/output_tokens.txt"
Write-Host "  - docs/minimal_dfa_table.txt"
Write-Host "  - docs/minimal_dfa_table.csv"
Write-Host "  - docs/Project_Report.pdf"

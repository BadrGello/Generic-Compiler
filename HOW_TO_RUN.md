# How to Run the Lexical Analyzer Generator Project

This guide provides simple, step-by-step instructions to build, test, and run the entire project on Windows (PowerShell or Command Prompt).

---

## ⚡ Option 1: Quick Run (One Command)

From the project root directory (`D:\Github\Generic-Compiler`), run:

```powershell
.\run_all.ps1
```

This automated script will:
1. Configure and build the project using CMake.
2. Run all 5 automated test suites (`test_step0` through `test_step4`).
3. Run the main `lexer_generator` CLI on `tests/sample_program.java`.
4. Run the Flex bonus tool and compare outputs.
5. Generate `docs/Project_Report.pdf`.

---

## 🛠️ Option 2: Step-by-Step Manual Commands

### Step 1: Build the Project with CMake
```powershell
cd D:\Github\Generic-Compiler
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

This compiles:
* `build/lexer_generator.exe`: The main Lexical Analyzer Generator CLI.
* `build/test_step0.exe`: Tests for rules parsing and regex to postfix.
* `build/test_step1.exe`: Tests for Thompson's NFA and Combined NFA.
* `build/test_step2.exe`: Tests for Subset Construction (NFA $\to$ DFA).
* `build/test_step3.exe`: Tests for DFA Minimization.
* `build/test_step4.exe`: Tests for the Runtime Lexer, Symbol Table, and Error Recovery.

---

### Step 2: Run the Unit & Integration Tests
To verify all stages of the pipeline:
```powershell
cd D:\Github\Generic-Compiler\build
.\test_step0.exe
.\test_step1.exe
.\test_step2.exe
.\test_step3.exe
.\test_step4.exe
```
All 5 test suites will output `ALL STEP X TESTS PASSED SUCCESSFULLY!`.

---

### Step 3: Run the Main Lexical Analyzer Generator
To run the full end-to-end generator on the sample program:
```powershell
cd D:\Github\Generic-Compiler\build
.\lexer_generator.exe
```

Or provide custom input paths:
```powershell
.\lexer_generator.exe [rules_file] [input_program] [output_tokens] [output_table]
```

**Outputs generated:**
* `build/output_tokens.txt`: The stream of recognized token classes (one per line).
* `build/minimal_dfa_table.txt`: The text matrix of the minimal DFA transition table.
* `build/minimal_dfa_table.csv`: The CSV spreadsheet of the minimal DFA transition table.

---

### Step 4: Run the Bonus Tool (Flex 2.5.4a)
Flex 2.5.4 from GnuWin32 expects to be run from inside the `flex/` directory:

```powershell
cd D:\Github\Generic-Compiler\flex

# 1. Generate C scanner from Flex specification (creates lex.yy.c)
.\flex-2.5.4a-1-bin\bin\flex.exe lexer.l

# 2. Compile generated C scanner with GCC
gcc -O2 lex.yy.c -o flex_lexer.exe

# 3. Run scanner on sample program
.\flex_lexer.exe ..\tests\sample_program.java
```

---

## 📁 Key File Locations

| File | Purpose |
|---|---|
| [`docs/Project_Report.pdf`](docs/Project_Report.pdf) | Formal project report in PDF format. |
| [`docs/Project_Report.md`](docs/Project_Report.md) | Markdown source of the project report. |
| [`docs/minimal_dfa_table.txt`](docs/minimal_dfa_table.txt) | Aligned ASCII transition table of the 44-state minimal DFA. |
| [`docs/minimal_dfa_table.csv`](docs/minimal_dfa_table.csv) | CSV spreadsheet of the minimal DFA transition table. |
| [`build/output_tokens.txt`](build/output_tokens.txt) | Token classes output for the sample program (23 tokens, 1 per line). |
| [`tests/lexical_rules.txt`](tests/lexical_rules.txt) | Language rules specification file. |
| [`tests/sample_program.java`](tests/sample_program.java) | The test program from Page 3 of the PDF. |
| [`flex/lexer.l`](flex/lexer.l) | Flex bonus specification file. |
| [`docs/Bonus_Flex_Report.md`](docs/Bonus_Flex_Report.md) | Dedicated Flex report with screenshots and comparative analysis. |

import os
import zipfile

def create_submission_zip():
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    zip_path = os.path.join(root, "Phase1_Lexical_Analyzer_Submission.zip")

    includes = [
        ("CMakeLists.txt", "CMakeLists.txt"),
        ("HOW_TO_RUN.md", "HOW_TO_RUN.md"),
        ("run_all.ps1", "run_all.ps1"),
        (os.path.join("docs", "Project_Report.pdf"), "Project_Report.pdf"),
        ("include", "include"),
        ("src", "src"),
        ("tests", "tests"),
        ("docs", "docs"),
        ("flex", "flex"),
    ]

    bin_executables = [
        os.path.join("build", "lexer_generator.exe"),
        os.path.join("build", "test_step0.exe"),
        os.path.join("build", "test_step1.exe"),
        os.path.join("build", "test_step2.exe"),
        os.path.join("build", "test_step3.exe"),
        os.path.join("build", "test_step4.exe"),
    ]

    print(f"Creating submission zip: {zip_path}...")
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for src_rel, dest_rel in includes:
            full_src = os.path.join(root, src_rel)
            if os.path.isfile(full_src):
                zf.write(full_src, dest_rel)
            elif os.path.isdir(full_src):
                for dirpath, dirnames, filenames in os.walk(full_src):
                    for f in filenames:
                        # Skip leftover executables in tests folder
                        if f.endswith(".exe") and src_rel == "tests":
                            continue
                        file_full = os.path.join(dirpath, f)
                        rel_in_dir = os.path.relpath(file_full, full_src)
                        zf.write(file_full, os.path.join(dest_rel, rel_in_dir))

        for exe_rel in bin_executables:
            exe_full = os.path.join(root, exe_rel)
            if os.path.exists(exe_full):
                zf.write(exe_full, os.path.join("bin", os.path.basename(exe_full)))

    size_mb = os.path.getsize(zip_path) / (1024 * 1024)
    print(f"SUCCESS: Created {zip_path} ({size_mb:.2f} MB)")

if __name__ == "__main__":
    create_submission_zip()

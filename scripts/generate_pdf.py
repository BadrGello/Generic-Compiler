import os
import subprocess
import markdown

def main():
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    md_path = os.path.join(root_dir, "docs", "Project_Report.md")
    html_path = os.path.join(root_dir, "docs", "Project_Report.html")
    pdf_path = os.path.join(root_dir, "docs", "Project_Report.pdf")

    print(f"Reading {md_path}...")
    with open(md_path, "r", encoding="utf-8") as f:
        md_content = f.read()

    # Convert markdown to HTML
    html_body = markdown.markdown(
        md_content,
        extensions=["tables", "fenced_code", "toc"]
    )

    # Clean academic / professional report CSS
    styled_html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Phase 1: Lexical Analyzer Generator — Project Report</title>
    <style>
        @page {{
            size: A4;
            margin: 20mm 15mm 20mm 15mm;
            @bottom-right {{
                content: counter(page);
            }}
        }}
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            color: #24292e;
            line-height: 1.5;
            font-size: 11pt;
            background-color: #ffffff;
            margin: 0;
            padding: 10px;
        }}
        h1 {{
            color: #1a365d;
            font-size: 20pt;
            border-bottom: 2px solid #2b6cb0;
            padding-bottom: 8px;
            margin-top: 0;
        }}
        h2 {{
            color: #2b6cb0;
            font-size: 15pt;
            border-bottom: 1px solid #e2e8f0;
            padding-bottom: 5px;
            margin-top: 25px;
            page-break-after: avoid;
        }}
        h3 {{
            color: #2d3748;
            font-size: 12pt;
            margin-top: 18px;
            page-break-after: avoid;
        }}
        h4 {{
            color: #4a5568;
            font-size: 11pt;
            margin-top: 14px;
            page-break-after: avoid;
        }}
        p, li {{
            text-align: justify;
        }}
        pre, code {{
            font-family: Consolas, "Courier New", Courier, monospace;
            font-size: 9pt;
        }}
        p code, li code {{
            background-color: #edf2f7;
            padding: 2px 5px;
            border-radius: 4px;
            color: #c53030;
        }}
        pre {{
            background-color: #f7fafc;
            border: 1px solid #e2e8f0;
            border-radius: 6px;
            padding: 10px;
            overflow-x: auto;
            white-space: pre-wrap;
            word-wrap: break-word;
            page-break-inside: avoid;
        }}
        table {{
            border-collapse: collapse;
            width: 100%;
            margin: 15px 0;
            font-size: 9pt;
            page-break-inside: auto;
        }}
        tr {{
            page-break-inside: avoid;
            page-break-after: auto;
        }}
        th, td {{
            border: 1px solid #cbd5e0;
            padding: 6px 10px;
            text-align: left;
            vertical-align: top;
        }}
        th {{
            background-color: #ebf8ff;
            color: #2b6cb0;
            font-weight: 600;
        }}
        tr:nth-child(even) {{
            background-color: #f7fafc;
        }}
        blockquote {{
            border-left: 4px solid #4299e1;
            padding: 6px 15px;
            color: #4a5568;
            background-color: #ebf8ff;
            margin: 10px 0;
            border-radius: 0 4px 4px 0;
        }}
        hr {{
            border: 0;
            height: 1px;
            background: #e2e8f0;
            margin: 25px 0;
        }}
        ul, ol {{
            padding-left: 25px;
        }}
        img {{
            max-width: 100%;
            height: auto;
            border: 1px solid #cbd5e0;
            border-radius: 6px;
            margin: 15px auto;
            display: block;
            box-shadow: 0 2px 6px rgba(0,0,0,0.1);
        }}
    </style>
</head>
<body>
{html_body}
</body>
</html>
"""

    with open(html_path, "w", encoding="utf-8") as f:
        f.write(styled_html)
    print(f"Wrote styled HTML to {html_path}...")

    # Look for Edge executable
    edge_paths = [
        r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
        r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
        r"C:\Program Files\Google\Chrome\Application\chrome.exe"
    ]
    browser_exe = None
    for p in edge_paths:
        if os.path.exists(p):
            browser_exe = p
            break

    if not browser_exe:
        print("Warning: Edge/Chrome executable not found for PDF export.")
        return

    print(f"Generating PDF using {browser_exe}...")
    cmd = [
        browser_exe,
        "--headless",
        "--disable-gpu",
        "--no-pdf-header-footer",
        f"--print-to-pdf={pdf_path}",
        html_path
    ]

    res = subprocess.run(cmd, capture_output=True)
    if os.path.exists(pdf_path):
        size_kb = os.path.getsize(pdf_path) / 1024
        print(f"SUCCESS: Generated {pdf_path} ({size_kb:.1f} KB)")
    else:
        print(f"Error generating PDF: {res.stderr.decode()}")

if __name__ == "__main__":
    main()

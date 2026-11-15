import subprocess
import os

def generate():
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    out_svg = os.path.join(root, "docs", "pipeline_diagram.svg")
    out_png = os.path.join(root, "docs", "pipeline_diagram.png")
    dot_exe = r"C:\Program Files\Graphviz\bin\dot.exe"

    dot_content = """digraph LexerPipeline {
    rankdir=TB;
    bgcolor="transparent";
    fontname="Helvetica,Arial,sans-serif";
    node [fontname="Helvetica,Arial,sans-serif", fontsize=10, penwidth=1.5];
    edge [fontname="Helvetica,Arial,sans-serif", fontsize=9, penwidth=1.2, color="#4A5568"];

    subgraph cluster_gen {
        label="Lexical Analyzer Generator Pipeline";
        fontsize=12;
        fontcolor="#1A365D";
        style="rounded,filled";
        fillcolor="#F7FAFC";
        color="#CBD5E0";
        margin=15;

        rules [shape=box, style="filled,rounded", fillcolor="#EBF8FF", color="#3182CE", label="Lexical Rules Specification\\n(tests/lexical_rules.txt)"];
        step0 [shape=box, style="filled,rounded", fillcolor="#E6FFFA", color="#319795", label="Step 0: RuleParser\\n(Macro & Range Expansion, Infix to Postfix)"];
        step1 [shape=box, style="filled,rounded", fillcolor="#FEFCBF", color="#D69E2E", label="Step 1: NFA Engine\\n(Thompson's Construction -> Combined Master NFA)"];
        step2 [shape=box, style="filled,rounded", fillcolor="#FEEBC8", color="#DD6B20", label="Step 2: DFA Engine\\n(Subset Construction & Priority Resolution)"];
        step3 [shape=box, style="filled,rounded", fillcolor="#E9D8FD", color="#805AD5", label="Step 3: DFA Minimizer\\n(Partition Refinement / Hopcroft's Algorithm)"];
        table [shape=box, style="filled,rounded", fillcolor="#EDF2F7", color="#718096", label="Transition Table Exporter\\n(minimal_dfa_table.txt & .csv)"];

        rules -> step0 -> step1 -> step2 -> step3 -> table;
    }

    subgraph cluster_sim {
        label="Runtime Simulation & Outputs";
        fontsize=12;
        fontcolor="#1A365D";
        style="rounded,filled";
        fillcolor="#F7FAFC";
        color="#CBD5E0";
        margin=15;

        source [shape=box, style="filled,rounded", fillcolor="#EBF8FF", color="#3182CE", label="Input Source Code Program\\n(sample_program.java)"];
        step4 [shape=box, style="filled,rounded", fillcolor="#FED7D7", color="#E53E3E", label="Step 4: Lexical Analyzer Simulator\\n(Maximal Munch, Symbol Table, Error Recovery)"];
        
        tokens [shape=box, style="filled,rounded", fillcolor="#C6F6D5", color="#38A169", label="Token Stream Output\\n(output_tokens.txt)"];
        symtab [shape=box, style="filled,rounded", fillcolor="#C6F6D5", color="#38A169", label="Symbol Table\\n(Identifiers & Counts)"];
        parser [shape=box, style="filled,rounded,dashed", fillcolor="#FFFFFF", color="#A0AEC0", label="Phase 2 Parser\\n(getNextToken() API)"];

        source -> step4;
        step4 -> tokens;
        step4 -> symtab;
        step4 -> parser [style=dashed, label=" getNextToken()"];
    }

    table -> step4 [label=" minimal transition table"];
}
"""

    if os.path.exists(dot_exe):
        subprocess.run([dot_exe, "-Tsvg", "-o", out_svg], input=dot_content.encode("utf-8"), check=True)
        print(f"Generated {out_svg}")
        subprocess.run([dot_exe, "-Tpng", "-Gdpi=200", "-o", out_png], input=dot_content.encode("utf-8"), check=True)
        print(f"Generated {out_png}")
    else:
        print("Graphviz dot not found!")

if __name__ == "__main__":
    generate()

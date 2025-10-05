#!/usr/bin/env python3
"""
Script to compile all standalone experiments as individual PDFs.
"""

import subprocess
import sys
from pathlib import Path


def compile_experiment(exp_num: int) -> bool:
    """
    Compile a single experiment.
    
    Args:
        exp_num: Experiment number (1-9)
        
    Returns:
        True if successful, False otherwise
    """
    print(f"\nCompiling Experiment {exp_num}...")
    
    # LaTeX command with experiment number
    tex_command = rf'\def\EXPNUM{{{exp_num}}}\input{{exp_standalone.tex}}'
    
    # Compile twice for proper TOC and references
    for _ in range(2):
        result = subprocess.run(
            ['pdflatex', '-interaction=nonstopmode', '-shell-escape', tex_command],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        if result.returncode != 0 and result.returncode != 1:
            # Exit code 1 is often just warnings, 0 is success
            print(f"  ✗ Error: Failed to compile Experiment {exp_num}")
            return False
    
    # Rename the output PDF
    source = Path("exp_standalone.pdf")
    if source.exists():
        exp_two_digit = f"{exp_num:02d}"
        target = Path(f"encs4110_{exp_two_digit}.pdf")
        # Replace if target already exists
        if target.exists():
            target.unlink()
        source.rename(target)
        print(f"Created: {target.name}")
        return True
    else:
        print(f"Error: Failed to create PDF for Experiment {exp_num}")
        return False


def cleanup_auxiliary_files():
    """Remove auxiliary LaTeX files."""
    print("\nCleaning up auxiliary files...")
    
    aux_extensions = ['.aux', '.log', '.toc', '.out', '.synctex.gz']
    base_name = "exp_standalone"
    
    for ext in aux_extensions:
        aux_file = Path(f"{base_name}{ext}")
        if aux_file.exists():
            aux_file.unlink()


def main():
    """Main function to compile all experiments."""
    print("Compiling all experiments as standalone PDFs...")
    print("=" * 60)
    
    # Compile experiments 1-9
    success_count = 0
    failed: list[int] = []
    
    for exp_num in range(1, 10):
        if compile_experiment(exp_num):
            success_count += 1
        else:
            failed.append(exp_num)
    
    # Clean up
    cleanup_auxiliary_files()
    
    # Summary
    print("\n" + "=" * 60)
    print(f"Successfully compiled: {success_count}/9 experiments")
    
    if failed:
        print(f"Failed experiments: {', '.join(map(str, failed))}")
        sys.exit(1)
    else:
        print("All experiments compiled successfully!")
        print("\nOutput files: encs4110_01.pdf through encs4110_09.pdf")
        sys.exit(0)


if __name__ == "__main__":
    main()

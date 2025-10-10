#!/usr/bin/env python3
"""Cross-platform coverage generation script for dawproject_c.

Automatically detects platform and uses appropriate coverage tools:
- Windows: OpenCppCoverage (MSVC)
- Linux/macOS: gcovr (GCC/Clang)

Usage:
    python scripts/generate_coverage.py [--build-dir BUILD_DIR] [--html]
    
Requirements:
    Windows: OpenCppCoverage (choco install opencppcoverage)
    Linux: gcovr (apt-get install gcovr)
    macOS: gcovr (brew install gcovr)
"""

import argparse
import os
import platform
import subprocess
import sys
from pathlib import Path


def run_command(cmd, cwd=None, check=True):
    """Run shell command and return result."""
    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, cwd=cwd, check=check, capture_output=True, text=True)
        if result.stdout:
            print(result.stdout)
        return result
    except subprocess.CalledProcessError as e:
        print(f"❌ Command failed: {e}")
        if e.stderr:
            print(f"Error: {e.stderr}")
        if check:
            sys.exit(1)
        return e


def find_test_executables(build_dir):
    """Find test executables in build directory."""
    if platform.system() == "Windows":
        # Windows MSVC build structure
        test_patterns = ["*test*.exe", "*smoke*.exe"]
        search_dirs = [build_dir / "Debug", build_dir / "Release"]
    else:
        # Unix-like build structure  
        test_patterns = ["*test*", "*smoke*"]
        search_dirs = [build_dir]
    
    executables = []
    for search_dir in search_dirs:
        if search_dir.exists():
            for pattern in test_patterns:
                executables.extend(search_dir.glob(pattern))
    
    return [exe for exe in executables if exe.is_file() and os.access(exe, os.X_OK)]


def generate_coverage_windows(build_dir, source_dir, output_xml, html_output):
    """Generate coverage on Windows using OpenCppCoverage."""
    print("🪟 Windows coverage generation using OpenCppCoverage")
    
    # Check for OpenCppCoverage
    try:
        subprocess.run(["OpenCppCoverage", "--help"], 
                      capture_output=True, check=True)
        print("✅ OpenCppCoverage found")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ OpenCppCoverage not found. Install with:")
        print("   choco install opencppcoverage")
        print("   or download from: https://github.com/OpenCppCoverage/OpenCppCoverage/releases")
        return False
    
    # Find test executables
    test_exes = find_test_executables(build_dir)
    if not test_exes:
        print(f"❌ No test executables found in {build_dir}")
        return False
    
    print(f"🔍 Found test executables: {[exe.name for exe in test_exes]}")
    
    # Generate coverage
    coverage_args = [
        "OpenCppCoverage",
        "--sources", str(source_dir.resolve()),
        "--export_type", f"cobertura:{output_xml}",
        "--modules", str((build_dir / "Debug").resolve())
    ]
    
    if html_output:
        coverage_args.extend(["--export_type", "html:coverage_html"])
    
    for test_exe in test_exes:
        print(f"🧪 Running coverage for: {test_exe.name}")
        cmd = coverage_args + ["--", str(test_exe)]
        
        result = run_command(cmd, check=False)
        if result.returncode == 0:
            print("✅ Coverage generated successfully")
            return True
        else:
            print(f"⚠️  Coverage command returned exit code {result.returncode}")
    
    return False


def generate_coverage_unix(build_dir, source_dir, output_xml, html_output):
    """Generate coverage on Unix-like systems using gcovr."""
    print("🐧 Unix coverage generation using gcovr")
    
    # Check for gcovr
    try:
        subprocess.run(["gcovr", "--version"], capture_output=True, check=True)
        print("✅ gcovr found")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ gcovr not found. Install with:")
        if platform.system() == "Darwin":  # macOS
            print("   brew install gcovr")
        else:  # Linux
            print("   sudo apt-get install gcovr")
        return False
    
    # Check for .gcda files (execution data)
    gcda_files = list(build_dir.rglob("*.gcda"))
    if not gcda_files:
        print("❌ No .gcda files found. Tests may not have run or coverage not enabled.")
        
        # Try to run tests first
        test_exes = find_test_executables(build_dir)
        if test_exes:
            print("🧪 Running tests to generate coverage data...")
            for test_exe in test_exes:
                run_command([str(test_exe)], cwd=build_dir, check=False)
            
            # Check again for .gcda files
            gcda_files = list(build_dir.rglob("*.gcda"))
    
    if not gcda_files:
        print("❌ Still no .gcda files found after running tests.")
        return False
    
    print(f"✅ Found {len(gcda_files)} .gcda files")
    
    # Generate coverage report
    root_dir = source_dir.parent  # Assuming source_dir is src/, we want parent
    gcovr_args = [
        "gcovr",
        "--root", str(root_dir),
        "--filter", str(source_dir),
        "--xml", output_xml,
        "--exclude", str(root_dir / "_deps"),  
        "--exclude", str(root_dir / "external"),
        "--exclude", str(root_dir / "tests"),
        str(build_dir)
    ]
    
    if html_output:
        gcovr_args.extend(["--html", "coverage.html"])
    
    result = run_command(gcovr_args, check=False)
    return result.returncode == 0


def main():
    parser = argparse.ArgumentParser(description="Cross-platform coverage generation")
    parser.add_argument("--build-dir", default="build/coverage", 
                       help="Build directory (default: build/coverage)")
    parser.add_argument("--source-dir", default="src",
                       help="Source directory (default: src)")
    parser.add_argument("--output", default="coverage.xml",
                       help="Output XML file (default: coverage.xml)")
    parser.add_argument("--html", action="store_true",
                       help="Also generate HTML report")
    
    args = parser.parse_args()
    
    # Convert to Path objects
    build_dir = Path(args.build_dir)
    source_dir = Path(args.source_dir)
    output_xml = Path(args.output)
    
    print(f"=== Cross-Platform Coverage Generation ===")
    print(f"Platform: {platform.system()}")
    print(f"Build dir: {build_dir}")
    print(f"Source dir: {source_dir}")
    print(f"Output: {output_xml}")
    
    if not build_dir.exists():
        print(f"❌ Build directory {build_dir} does not exist")
        sys.exit(1)
    
    if not source_dir.exists():
        print(f"❌ Source directory {source_dir} does not exist")
        sys.exit(1)
    
    # Platform-specific coverage generation
    if platform.system() == "Windows":
        success = generate_coverage_windows(build_dir, source_dir, output_xml, args.html)
    else:
        success = generate_coverage_unix(build_dir, source_dir, output_xml, args.html)
    
    if success and output_xml.exists():
        # Parse and display coverage percentage
        try:
            import xml.etree.ElementTree as ET
            tree = ET.parse(output_xml)
            root = tree.getroot()
            if 'line-rate' in root.attrib:
                coverage = float(root.attrib['line-rate']) * 100
                print(f"📊 Line Coverage: {coverage:.2f}%")
        except Exception as e:
            print(f"⚠️  Could not parse coverage percentage: {e}")
        
        print(f"✅ Coverage report generated: {output_xml.resolve()}")
        if args.html:
            html_file = Path("coverage.html") if platform.system() != "Windows" else Path("coverage_html/index.html")
            if html_file.exists():
                print(f"🌐 HTML report: {html_file.resolve()}")
    else:
        print("❌ Coverage generation failed")
        sys.exit(1)


if __name__ == "__main__":
    main()
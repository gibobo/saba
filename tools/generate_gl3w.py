#!/usr/bin/env python3
"""
GL3W Auto Generation Script
This script automatically executes gl3w_gen.py during CMake configuration phase
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path

def check_python():
    """Check Python environment"""
    print(f"Using Python: {sys.executable}")
    print(f"Python version: {sys.version}")
    return True

def generate_gl3w(gl3w_dir, output_dir):
    """Generate gl3w files"""
    gl3w_script = gl3w_dir / "gl3w_gen.py"
    
    if not gl3w_script.exists():
        print(f"Error: Cannot find {gl3w_script}")
        return False
    
    # Ensure output directory exists
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"Executing gl3w_gen.py...")
    print(f"Script path: {gl3w_script}")
    print(f"Working directory: {output_dir}")
    
    try:
        result = subprocess.run(
            [sys.executable, str(gl3w_script)],
            cwd=str(output_dir),
            capture_output=True,
            text=True
        )
        
        if result.returncode == 0:
            print("✓ GL3W files generated successfully")
            if result.stdout:
                print(result.stdout)
            return True
        else:
            print("✗ GL3W generation failed")
            if result.stderr:
                print(f"Error output: {result.stderr}")
            if result.stdout:
                print(f"Standard output: {result.stdout}")
            return False
            
    except Exception as e:
        print(f"✗ Execution error: {e}")
        return False

def check_generated_files(output_dir):
    """Check if generated files exist"""
    required_files = [
        "src/gl3w.c",
        "include/GL/gl3w.h",
        "include/GL/glcorearb.h",
        "include/KHR/khrplatform.h"
    ]
    
    missing_files = []
    for file_path in required_files:
        full_path = output_dir / file_path
        if not full_path.exists():
            missing_files.append(file_path)
    
    if missing_files:
        print(f"Missing files: {missing_files}")
        return False
    else:
        print("✓ All required files have been generated")
        return True

def main():
    parser = argparse.ArgumentParser(description="GL3W Auto Generation Tool")
    parser.add_argument("--gl3w-dir", help="gl3w source directory (auto-detected if not specified)")
    parser.add_argument("--output-dir", help="Output directory (auto-detected if not specified)")
    parser.add_argument("--force", action="store_true", help="Force regeneration")
    
    args = parser.parse_args()
    
    # Auto-detect paths if not provided
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    if args.gl3w_dir:
        gl3w_dir = Path(args.gl3w_dir).resolve()
    else:
        gl3w_dir = (project_root / "external" / "gl3w").resolve()
    
    if args.output_dir:
        output_dir = Path(args.output_dir).resolve()
    else:
        output_dir = (project_root / "build" / "external" / "gl3w").resolve()
    
    print("=== GL3W Auto Generation Tool ===")
    print(f"GL3W source directory: {gl3w_dir}")
    print(f"Output directory: {output_dir}")
    
    if not gl3w_dir.exists():
        print(f"Error: gl3w directory does not exist: {gl3w_dir}")
        if not args.gl3w_dir and not args.output_dir:
            print("Hint: Run this script from the project root, or specify paths explicitly:")
            print("  python tools/generate_gl3w.py --gl3w-dir external/gl3w --output-dir build/external/gl3w")
        return 1
    
    # Check if generation is needed
    if not args.force and check_generated_files(output_dir):
        print("Files already exist, skipping generation (use --force to regenerate)")
        return 0
    
    # Check Python environment
    if not check_python():
        return 1
    
    # Generate files
    if not generate_gl3w(gl3w_dir, output_dir):
        return 1
    
    # Verify generation result
    if not check_generated_files(output_dir):
        return 1
    
    print("=== Generation Complete ===")
    return 0

if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
Script to generate Python gRPC code from protobuf definitions.
"""

import os
import subprocess
import sys
from pathlib import Path

def generate_grpc_code():
    """Generate Python gRPC code from proto files."""
    
    # Get the script directory
    script_dir = Path(__file__).parent
    proto_dir = script_dir.parent / "protos"
    output_dir = script_dir / "generated"
    
    # Create output directory if it doesn't exist
    output_dir.mkdir(exist_ok=True)
    
    # Create __init__.py file in generated directory
    (output_dir / "__init__.py").touch()
    
    # Proto file to process
    proto_file = proto_dir / "test_agent_service.proto"
    
    if not proto_file.exists():
        print(f"Error: Proto file not found: {proto_file}")
        return False
    
    # Generate Python protobuf and gRPC code
    cmd = [
        sys.executable, "-m", "grpc_tools.protoc",
        f"--proto_path={proto_dir}",
        f"--python_out={output_dir}",
        f"--grpc_python_out={output_dir}",
        str(proto_file)
    ]
    
    print(f"Running command: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("✅ Successfully generated Python gRPC code")
        print(f"Output files created in: {output_dir}")
        
        # List generated files
        for file in output_dir.glob("*.py"):
            print(f"  - {file.name}")
            
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"❌ Error generating gRPC code: {e}")
        print(f"stdout: {e.stdout}")
        print(f"stderr: {e.stderr}")
        return False

if __name__ == "__main__":
    success = generate_grpc_code()
    sys.exit(0 if success else 1) 
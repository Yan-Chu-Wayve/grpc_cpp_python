#!/bin/bash

# Setup script for gRPC C++ Python project
set -e

echo "🚀 Setting up gRPC C++ Python Project"
echo "====================================="

# Check if we're in the right directory
if [ ! -f "protos/test_agent_service.proto" ]; then
    echo "❌ Error: Please run this script from the grpc_cpp_python directory"
    exit 1
fi

# Setup Python client
echo ""
echo "📦 Setting up Python client..."
cd python_client

# Create virtual environment
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3.10 -m venv venv
fi

# Activate virtual environment
echo "Activating virtual environment..."
source venv/bin/activate

# Install dependencies
echo "Installing Python dependencies..."
pip install -r requirements.txt

# Generate protobuf files
echo "Generating protobuf files..."
python generate_protos.py

echo "✅ Python client setup complete!"

# Go back to project root
cd ..

echo ""
echo "🔨 C++ Server Setup Instructions:"
echo "================================="
echo "To build the C++ server, run:"
echo ""
echo "  cd cpp_server"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make"
echo ""
echo "Then run the server with:"
echo "  ./test_agent_server"
echo ""

echo "🎯 Quick Start Guide:"
echo "===================="
echo "1. Build and run the C++ server (see instructions above)"
echo "2. In another terminal, run the Python client:"
echo "   cd python_client"
echo "   source venv/bin/activate"
echo "   python src/test_agent_client.py --demo"
echo ""
echo "✅ Setup complete! Check README.md for detailed instructions." 

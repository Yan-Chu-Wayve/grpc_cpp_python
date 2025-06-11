# gRPC C++ Python Project

A comprehensive gRPC-based TestAgent service with a C++ server implementation and Python client, derived from the Wayve Driver SDK.

## 🚀 Project Overview

This project demonstrates a complete gRPC service architecture featuring:

- **C++ Server**: High-performance server implementation with thread-safe operations
- **Python Client**: Comprehensive client with full API coverage and demo functionality
- **Rich Service Interface**: 10 different RPC methods including streaming
- **Production Ready**: Proper error handling, logging, and graceful shutdown

## 📁 Project Structure

```
grpc_cpp_python/
├── protos/
│   └── test_agent_service.proto    # Protocol buffer definitions
├── cpp_server/
│   ├── src/
│   │   ├── test_agent_service_impl.h    # Service implementation header
│   │   ├── test_agent_service_impl.cpp  # Service implementation
│   │   └── server_main.cpp              # Server entry point
│   └── CMakeLists.txt                   # C++ build configuration
├── python_client/
│   ├── src/
│   │   └── test_agent_client.py         # Python client implementation
│   ├── generate_protos.py               # Proto generation script
│   └── requirements.txt                 # Python dependencies
└── README.md                            # This file
```

## 🔧 Prerequisites

### For C++ Server
- **CMake** (3.10 or higher)
- **gRPC** and **Protocol Buffers**
- **C++17** compatible compiler
- **pkg-config**

### For Python Client
- **Python** 3.7 or higher
- **pip** package manager

## 🏗️ Building and Running

### 1. Build the C++ Server

```bash
cd cpp_server
mkdir build && cd build
cmake ..
make

# Run the server
./test_agent_server
```

**Server Options:**
```bash
./test_agent_server -h              # Show help
./test_agent_server -p 8080         # Custom port
./test_agent_server -a 0.0.0.0      # Bind to all interfaces
```

### 2. Set up the Python Client

```bash
cd python_client

# Create virtual environment (recommended)
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Generate gRPC code
python generate_protos.py

# Run the client
python src/test_agent_client.py
```

**Client Options:**
```bash
python src/test_agent_client.py --help          # Show help
python src/test_agent_client.py --demo          # Run full demo
python src/test_agent_client.py --host server.com --port 8080  # Custom server
```

## 🌟 Service Features

### Core Information Services
- **IsWayveDriverMock**: Check if running in mock mode
- **GetWayveDriverVersion**: Retrieve driver version
- **GetIntegrationStatus**: Get current integration state
- **GetModelId**: Get the active model identifier

### Service Management
- **GetServiceStatus**: Query status of specific services
- **StartService**: Start trajectory, navigation, or inference services
- **StopService**: Stop running services

### Driver Control
- **EngageWayveDriver**: Engage autonomous driving mode
- **DisengageWayveDriver**: Switch to manual mode

### Real-time Monitoring
- **StreamTrace**: Stream live trace events with filtering

## 🎯 Service Types

The system manages three core service types:
- **Trajectory**: Path planning and trajectory generation
- **Navigation**: Route planning and navigation logic
- **Inference**: AI model inference and decision making

## 📊 Integration States

The system supports multiple integration states:
- `UNKNOWN` / `INITIALIZING` - System startup
- `IDLE` - Ready but not active
- `AV_LIMITED` / `AV` - Autonomous driving modes
- `MANUAL` - Human control
- `REMOTE_CONTROL` - Remote operation
- `ERROR` / `SHUTDOWN` - Error or shutdown states

## 🔍 Example Usage

### Basic Client Usage
```python
from test_agent_client import TestAgentClient

client = TestAgentClient()

# Check system status
client.is_wayve_driver_mock()
client.get_wayve_driver_version()

# Manage services
client.start_service(pb2.SERVICE_TYPE_TRAJECTORY)
client.get_service_status(pb2.SERVICE_TYPE_TRAJECTORY)

# Stream trace events
client.stream_trace_events(max_events=5)

client.close()
```

### Running Full Demo
```bash
python src/test_agent_client.py --demo
```

This runs a comprehensive test suite covering all service features.

## 🧪 Testing

### Manual Testing with grpcurl
```bash
# List available services
grpcurl -plaintext localhost:50051 list

# Test specific methods
grpcurl -plaintext localhost:50051 wayve.driver.tests.protobuf.TestAgentService/GetWayveDriverVersion
grpcurl -plaintext localhost:50051 wayve.driver.tests.protobuf.TestAgentService/IsWayveDriverMock
```

### Python Client Testing
```bash
# Basic connectivity test
python src/test_agent_client.py

# Full feature demonstration
python src/test_agent_client.py --demo
```

## 🔒 Thread Safety

The C++ server implementation includes:
- **Mutex Protection**: All shared state protected by mutexes
- **Thread-Safe Operations**: Safe concurrent access to service states
- **Graceful Shutdown**: Proper cleanup on termination signals

## 🚨 Error Handling

- **gRPC Status Codes**: Proper error propagation
- **Connection Handling**: Automatic retry and timeout handling
- **Graceful Degradation**: Continues operation on non-critical errors

## 🛠️ Development

### Adding New Services
1. Update the `.proto` file with new RPC methods
2. Regenerate code using `generate_protos.py`
3. Implement methods in C++ server
4. Add client methods in Python

### Debugging
- Enable gRPC logging: `export GRPC_VERBOSITY=DEBUG`
- Use server reflection for introspection
- Monitor with standard debugging tools

## 📚 Dependencies

### C++ Dependencies
- gRPC (1.50+)
- Protocol Buffers (3.20+)
- CMake (3.10+)

### Python Dependencies
- grpcio (1.60.0)
- grpcio-tools (1.60.0)
- protobuf (4.25.1)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is part of the Wayve Driver SDK and follows the same licensing terms.

## 🆘 Troubleshooting

### Common Issues

**"ImportError: No module named 'test_agent_service_pb2'"**
- Run `python generate_protos.py` to generate protobuf files

**"Connection refused"**
- Ensure the C++ server is running
- Check the port (default: 50051)

**"CMake Error: Could not find gRPC"**
- Install gRPC development packages
- Set CMAKE_PREFIX_PATH if needed

**Build Errors**
- Ensure C++17 compiler support
- Check all dependencies are installed

For more help, check the individual component READMEs or open an issue. 
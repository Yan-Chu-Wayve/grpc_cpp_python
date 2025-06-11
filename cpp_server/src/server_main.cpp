#include <iostream>
#include <memory>
#include <string>
#include <signal.h>
#include <thread>
#include <chrono>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

// Try to include reflection, but make it optional
#ifdef GRPC_HAS_REFLECTION
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#endif

#include "test_agent_service_impl.h"

std::unique_ptr<grpc::Server> g_server;

void SignalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down server..." << std::endl;
    if (g_server) {
        g_server->Shutdown();
    }
}

void RunServer(const std::string& server_address) {
    wayve::driver::tests::TestAgentServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    
    // Enable reflection if available
    #ifdef GRPC_HAS_REFLECTION
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    std::cout << "gRPC reflection enabled" << std::endl;
    #else
    std::cout << "gRPC reflection not available" << std::endl;
    #endif

    grpc::ServerBuilder builder;

    // Listen on the given address without any authentication mechanism
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    // Register the service
    builder.RegisterService(&service);

    // Set server options
    builder.SetMaxReceiveMessageSize(4 * 1024 * 1024); // 4MB
    builder.SetMaxSendMessageSize(4 * 1024 * 1024);    // 4MB

    // Build and start the server
    g_server = builder.BuildAndStart();

    if (!g_server) {
        std::cerr << "Failed to start server on " << server_address << std::endl;
        return;
    }

    std::cout << "TestAgentService server listening on " << server_address << std::endl;
    std::cout << "\nPress Ctrl+C to stop the server" << std::endl;

    // Demonstrate service configuration
    std::cout << "\nConfiguring service for demo..." << std::endl;
    service.SetIntegrationState(wayve::driver::tests::protobuf::INTEGRATION_STATE_IDLE);

    // Wait for the server to shutdown
    g_server->Wait();
    std::cout << "Server shutdown complete." << std::endl;
}

void PrintUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -p PORT     Server port (default: 50051)" << std::endl;
    std::cout << "  -a ADDR     Server address (default: localhost)" << std::endl;
    std::cout << "  -h          Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << "                    # Run on localhost:50051" << std::endl;
    std::cout << "  " << program_name << " -p 8080            # Run on localhost:8080" << std::endl;
    std::cout << "  " << program_name << " -a 0.0.0.0 -p 9090  # Run on 0.0.0.0:9090" << std::endl;
}

int main(int argc, char** argv) {
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    std::string address = "localhost";
    int port = 50051;

    // Simple command line argument parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            PrintUsage(argv[0]);
            return 0;
        } else if (arg == "-p" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
            if (port <= 0 || port > 65535) {
                std::cerr << "Error: Port must be between 1 and 65535" << std::endl;
                return 1;
            }
        } else if (arg == "-a" && i + 1 < argc) {
            address = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            PrintUsage(argv[0]);
            return 1;
        }
    }

    std::string server_address = address + ":" + std::to_string(port);

    std::cout << "Starting Wayve TestAgent gRPC Server" << std::endl;
    std::cout << "=====================================" << std::endl;

    try {
        RunServer(server_address);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 
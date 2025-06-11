cmake_minimum_required(VERSION 3.10)
project(TestAgentServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages with better error handling
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# Use pkg-config to find protobuf and gRPC on macOS
pkg_check_modules(PROTOBUF REQUIRED protobuf)
pkg_check_modules(GRPC REQUIRED grpc++)

# Find protoc compiler
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc not found")
endif()

# Find grpc_cpp_plugin
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
if(NOT GRPC_CPP_PLUGIN)
    message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()

# Generate protobuf and gRPC files
set(PROTO_FILES
    ../protos/test_agent_service.proto
)

# Set the output directory for generated files
set(GENERATED_PROTO_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_PROTO_DIR})

# Generate protobuf and gRPC files
set(PROTO_SRCS)
set(PROTO_HDRS)
set(GRPC_SRCS)
set(GRPC_HDRS)

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    get_filename_component(PROTO_PATH ${PROTO_FILE} DIRECTORY)
    
    set(PROTO_SRC ${GENERATED_PROTO_DIR}/${PROTO_NAME}.pb.cc)
    set(PROTO_HDR ${GENERATED_PROTO_DIR}/${PROTO_NAME}.pb.h)
    set(GRPC_SRC ${GENERATED_PROTO_DIR}/${PROTO_NAME}.grpc.pb.cc)
    set(GRPC_HDR ${GENERATED_PROTO_DIR}/${PROTO_NAME}.grpc.pb.h)
    
    list(APPEND PROTO_SRCS ${PROTO_SRC})
    list(APPEND PROTO_HDRS ${PROTO_HDR})
    list(APPEND GRPC_SRCS ${GRPC_SRC})
    list(APPEND GRPC_HDRS ${GRPC_HDR})
    
    # Generate protobuf files
    add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR}
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS --cpp_out=${GENERATED_PROTO_DIR}
             --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_PATH}
             ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating protobuf C++ files for ${PROTO_FILE}"
    )
    
    # Generate gRPC files
    add_custom_command(
        OUTPUT ${GRPC_SRC} ${GRPC_HDR}
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS --grpc_out=${GENERATED_PROTO_DIR}
             --cpp_out=${GENERATED_PROTO_DIR}
             --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_PATH}
             --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
             ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating gRPC C++ files for ${PROTO_FILE}"
    )
endforeach()

# Source files
set(SOURCES
    src/test_agent_service_impl.cpp
    src/server_main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

# Create executable
add_executable(test_agent_server ${SOURCES})

# Include directories
target_include_directories(test_agent_server PRIVATE
    src
    ${GENERATED_PROTO_DIR}
    ${PROTOBUF_INCLUDE_DIRS}
    ${GRPC_INCLUDE_DIRS}
)

# Compiler flags
target_compile_options(test_agent_server PRIVATE
    ${PROTOBUF_CFLAGS_OTHER}
    ${GRPC_CFLAGS_OTHER}
)

# Link directories
target_link_directories(test_agent_server PRIVATE
    ${PROTOBUF_LIBRARY_DIRS}
    ${GRPC_LIBRARY_DIRS}
)

# Find the reflection library directly
find_library(GRPC_REFLECTION_LIBRARY 
    NAMES grpc++_reflection
    PATHS /opt/homebrew/lib /usr/local/lib
    NO_DEFAULT_PATH
)

# Check if reflection library was found and enable it
if(GRPC_REFLECTION_LIBRARY)
    message(STATUS "Found gRPC reflection library: ${GRPC_REFLECTION_LIBRARY}")
    target_compile_definitions(test_agent_server PRIVATE GRPC_HAS_REFLECTION)
    set(REFLECTION_LIBRARIES ${GRPC_REFLECTION_LIBRARY})
else()
    message(STATUS "gRPC reflection library not found, will build without it")
    set(REFLECTION_LIBRARIES)
endif()

# Link libraries
target_link_libraries(test_agent_server
    ${PROTOBUF_LIBRARIES}
    ${GRPC_LIBRARIES}
    ${REFLECTION_LIBRARIES}
    Threads::Threads
)

# Custom target to generate proto files
add_custom_target(generate_proto_files DEPENDS ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS})
add_dependencies(test_agent_server generate_proto_files)

# Print debug information
message(STATUS "Protobuf libraries: ${PROTOBUF_LIBRARIES}")
message(STATUS "gRPC libraries: ${GRPC_LIBRARIES}")
message(STATUS "gRPC reflection library: ${GRPC_REFLECTION_LIBRARY}")
message(STATUS "protoc executable: ${PROTOC_EXECUTABLE}")
message(STATUS "grpc_cpp_plugin: ${GRPC_CPP_PLUGIN}") 
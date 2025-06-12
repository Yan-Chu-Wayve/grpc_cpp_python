#!/usr/bin/env python3
"""
TestAgent gRPC Client

A comprehensive client for the TestAgentService gRPC interface.
"""

import grpc
import sys
import os
import time
import threading
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import queue

# Add the generated directory to Python path
sys.path.append(str(Path(__file__).parent.parent / "generated"))

try:
    import test_agent_service_pb2 as pb2
    import test_agent_service_pb2_grpc as pb2_grpc
except ImportError as e:
    print(f"Error importing generated protobuf files: {e}")
    print("Please run 'python generate_protos.py' first to generate the required files.")
    sys.exit(1)

class TestAgentClient:
    """Client for TestAgentService gRPC interface."""
    
    def __init__(self, host='localhost', port=50051):
        """Initialize the client.
        
        Args:
            host: Server hostname
            port: Server port
        """
        self.address = f'{host}:{port}'
        self.channel = grpc.insecure_channel(self.address)
        self.stub = pb2_grpc.TestAgentServiceStub(self.channel)
        
    def close(self):
        """Close the gRPC channel."""
        if self.channel:
            self.channel.close()
    
    def is_wayve_driver_mock(self):
        """Check if WayveDriver is in mock mode."""
        try:
            response = self.stub.IsWayveDriverMock(pb2.Empty())
            print(f"Mock mode: {'enabled' if response.value else 'disabled'}")
            return response.value
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return None
    
    def get_wayve_driver_version(self):
        """Get the WayveDriver version."""
        try:
            response = self.stub.GetWayveDriverVersion(pb2.Empty())
            print(f"WayveDriver version: {response.version}")
            return response.version
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return None
    
    def get_integration_status(self):
        """Get the integration status."""
        try:
            response = self.stub.GetIntegrationStatus(pb2.Empty())
            state_name = pb2.IntegrationState.Name(response.state)
            print(f"Integration status: {state_name}")
            return response.state
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return None
    
    def get_model_id(self):
        """Get the model ID."""
        try:
            response = self.stub.GetModelId(pb2.Empty())
            print(f"Model ID: {response.model_id}")
            return response.model_id
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return None
    
    def get_service_status(self, service_type):
        """Get the status of a specific service.
        
        Args:
            service_type: ServiceType enum value
        """
        try:
            request = pb2.ServiceTypeRequest(service_type=service_type)
            response = self.stub.GetServiceStatus(request)
            service_name = pb2.ServiceType.Name(service_type)
            state_name = pb2.ServiceState.Name(response.state)
            print(f"Service {service_name} status: {state_name}")
            return response.state
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return None
    
    def start_service(self, service_type):
        """Start a service.
        
        Args:
            service_type: ServiceType enum value
        """
        try:
            request = pb2.ServiceTypeRequest(service_type=service_type)
            self.stub.StartService(request)
            service_name = pb2.ServiceType.Name(service_type)
            print(f"Service {service_name} started")
            return True
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return False
    
    def stop_service(self, service_type):
        """Stop a service.
        
        Args:
            service_type: ServiceType enum value
        """
        try:
            request = pb2.ServiceTypeRequest(service_type=service_type)
            self.stub.StopService(request)
            service_name = pb2.ServiceType.Name(service_type)
            print(f"Service {service_name} stopped")
            return True
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return False
    
    def engage_wayve_driver(self):
        """Engage the WayveDriver."""
        try:
            self.stub.EngageWayveDriver(pb2.Empty())
            print("WayveDriver engaged")
            return True
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return False
    
    def disengage_wayve_driver(self):
        """Disengage the WayveDriver."""
        try:
            self.stub.DisengageWayveDriver(pb2.Empty())
            print("WayveDriver disengaged")
            return True
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return False
    
    def stream_trace_events(self, max_events=10, timeout=30, log_queue=None):
        """Stream trace events from the server.
        
        Args:
            max_events: Maximum number of events to receive
            timeout: Timeout in seconds
            log_queue: Optional queue to put log events for concurrent processing
        """
        try:
            print(f"Streaming trace events (max: {max_events}, timeout: {timeout}s)...")
            
            event_count = 0
            start_time = time.time()
            
            for trace_event in self.stub.StreamTrace(pb2.Empty()):
                if event_count >= max_events:
                    print(f"Reached maximum events limit ({max_events})")
                    break
                    
                if time.time() - start_time > timeout:
                    print(f"Timeout reached ({timeout}s)")
                    break
                
                event_count += 1
                
                # Parse the trace event
                timestamp = trace_event.timestamp_ns
                groups_mask = trace_event.groups_mask
                severity = pb2.TraceSeverity.Name(trace_event.severity)
                event_type = pb2.TraceEventType.Name(trace_event.event_type)
                message = trace_event.message.decode('utf-8') if trace_event.message else "No message"
                
                log_entry = {
                    'event_number': event_count,
                    'timestamp': timestamp,
                    'groups_mask': groups_mask,
                    'severity': severity,
                    'event_type': event_type,
                    'message': message
                }
                
                # If queue provided, put event there for concurrent processing
                if log_queue:
                    log_queue.put(log_entry)
                else:
                    # Print directly if no queue
                    print(f"  Event {event_count}:")
                    print(f"    Timestamp: {timestamp} ns")
                    print(f"    Groups: {groups_mask}")
                    print(f"    Severity: {severity}")
                    print(f"    Type: {event_type}")
                    print(f"    Message: {message}")
                    print()
            
            print(f"Received {event_count} trace events")
            return event_count
            
        except grpc.RpcError as e:
            print(f"RPC failed: {e.code()} - {e.details()}")
            return 0

    def get_all_server_info(self):
        """Get all server information in one call."""
        print("Fetching server information...")
        info = {}
        
        info['mock_mode'] = self.is_wayve_driver_mock()
        info['version'] = self.get_wayve_driver_version()
        info['integration_status'] = self.get_integration_status()
        info['model_id'] = self.get_model_id()
        
        return info
    
    def stream_logs_while_polling_server_info(self, stream_duration=15, poll_interval=3):
        """Stream logs while periodically polling server information.
        
        Args:
            stream_duration: How long to stream logs (seconds)
            poll_interval: How often to poll server info (seconds)
        """
        print("Starting concurrent log streaming and server info polling...")
        print("=" * 60)
        
        # Queue for log events
        log_queue = queue.Queue()
        stop_streaming = threading.Event()
        
        def log_processor():
            """Process log events from the queue."""
            while not stop_streaming.is_set() or not log_queue.empty():
                try:
                    log_entry = log_queue.get(timeout=1)
                    print(f"[LOG] Event {log_entry['event_number']}: "
                          f"{log_entry['severity']} - {log_entry['message']}")
                except queue.Empty:
                    continue
        
        def server_info_poller():
            """Periodically poll server information."""
            poll_count = 0
            while not stop_streaming.is_set():
                poll_count += 1
                print(f"\n[INFO POLL #{poll_count}] Checking server status...")
                
                # Get server info concurrently
                with ThreadPoolExecutor(max_workers=4) as executor:
                    futures = {
                        'mock': executor.submit(self.is_wayve_driver_mock),
                        'version': executor.submit(self.get_wayve_driver_version),
                        'status': executor.submit(self.get_integration_status),
                        'model': executor.submit(self.get_model_id)
                    }
                    
                    # Wait for all to complete
                    for name, future in futures.items():
                        try:
                            result = future.result(timeout=2)
                        except Exception as e:
                            print(f"Error getting {name}: {e}")
                
                print(f"[INFO POLL #{poll_count}] Complete\n")
                
                # Wait for next poll or stop signal
                if stop_streaming.wait(poll_interval):
                    break
        
        def log_streamer():
            """Stream logs in the background."""
            try:
                # Stream for the specified duration or until stopped
                max_events = stream_duration * 2  # Roughly 2 events per second
                self.stream_trace_events(
                    max_events=max_events, 
                    timeout=stream_duration + 5,  # Add buffer
                    log_queue=log_queue
                )
            except Exception as e:
                print(f"Log streaming error: {e}")
            finally:
                stop_streaming.set()
        
        # Start all threads
        threads = [
            threading.Thread(target=log_processor, name="LogProcessor"),
            threading.Thread(target=server_info_poller, name="InfoPoller"),
            threading.Thread(target=log_streamer, name="LogStreamer")
        ]
        
        for thread in threads:
            thread.daemon = True
            thread.start()
        
        # Wait for streaming to complete
        try:
            print(f"Running for {stream_duration} seconds...")
            time.sleep(stream_duration)
        except KeyboardInterrupt:
            print("\nInterrupted by user")
        finally:
            stop_streaming.set()
            
            # Wait for threads to finish
            for thread in threads:
                thread.join(timeout=2)
        
        print("\nConcurrent streaming and polling completed!")


def run_demo_suite(client):
    """Run a comprehensive demo of all client features."""
    
    print("Starting TestAgent Client Demo")
    print("=" * 50)
    
    # Test basic information
    print("\n1. Testing basic information...")
    client.is_wayve_driver_mock()
    client.get_wayve_driver_version()
    client.get_integration_status()
    client.get_model_id()
    
    # Test service management
    print("\n2. Testing service management...")
    service_types = [
        pb2.SERVICE_TYPE_TRAJECTORY,
        pb2.SERVICE_TYPE_NAVIGATION,
        pb2.SERVICE_TYPE_INFERENCE
    ]
    
    for service_type in service_types:
        service_name = pb2.ServiceType.Name(service_type)
        print(f"\nTesting {service_name} service:")
        
        # Check initial status
        client.get_service_status(service_type)
        
        # Start service
        client.start_service(service_type)
        client.get_service_status(service_type)
        
        # Stop service
        client.stop_service(service_type)
        client.get_service_status(service_type)
    
    # Test driver engagement
    print("\n3. Testing driver engagement...")
    client.get_integration_status()
    client.engage_wayve_driver()
    client.get_integration_status()
    client.disengage_wayve_driver()
    client.get_integration_status()
    
    # Test trace streaming
    print("\n4. Testing trace streaming...")
    client.stream_trace_events(max_events=5, timeout=10)
    
    # Test concurrent operations
    print("\n5. Testing concurrent log streaming and server info polling...")
    client.stream_logs_while_polling_server_info(stream_duration=10, poll_interval=2)
    
    print("\nDemo completed successfully!")


def main():
    """Main function to run the client."""
    
    # Parse command line arguments
    import argparse
    parser = argparse.ArgumentParser(description='TestAgent gRPC Client')
    parser.add_argument('--host', default='localhost', help='Server host')
    parser.add_argument('--port', type=int, default=50051, help='Server port')
    parser.add_argument('--demo', action='store_true', help='Run demo suite')
    parser.add_argument('--concurrent', action='store_true', help='Test concurrent log streaming and server info')
    parser.add_argument('--stream-duration', type=int, default=15, help='Duration for streaming (seconds)')
    parser.add_argument('--poll-interval', type=int, default=3, help='Server info polling interval (seconds)')
    
    args = parser.parse_args()
    
    # Create client
    client = TestAgentClient(host=args.host, port=args.port)
    
    try:
        print(f"Connecting to TestAgent server at {client.address}")
        
        if args.demo:
            run_demo_suite(client)
        elif args.concurrent:
            client.stream_logs_while_polling_server_info(
                stream_duration=args.stream_duration,
                poll_interval=args.poll_interval
            )
        else:
            # Run basic tests
            client.is_wayve_driver_mock()
            client.get_wayve_driver_version()
            client.get_integration_status()
            
    except KeyboardInterrupt:
        print("\nInterrupted by user")
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        client.close()
        print("Client closed")


if __name__ == "__main__":
    main() 

#include "test_agent_service_impl.h"
#include <thread>
#include <chrono>
#include <random>
#include <iostream>
#include <grpcpp/grpcpp.h>

using grpc::Status;
using grpc::StatusCode;

namespace wayve::driver::tests {

TestAgentServiceImpl::TestAgentServiceImpl()
    : integration_state_(protobuf::INTEGRATION_STATE_IDLE),
      is_mock_mode_(true),
      wayve_driver_version_("0.1.0-mock"),
      model_id_("test-model-123")
{
    InitializeServiceStates();
}

void TestAgentServiceImpl::InitializeServiceStates()
{
    service_states_[protobuf::SERVICE_TYPE_TRAJECTORY] = protobuf::SERVICE_STATE_UNKNOWN;
    service_states_[protobuf::SERVICE_TYPE_NAVIGATION] = protobuf::SERVICE_STATE_UNKNOWN;
    service_states_[protobuf::SERVICE_TYPE_INFERENCE] = protobuf::SERVICE_STATE_UNKNOWN;
}

grpc::Status TestAgentServiceImpl::IsWayveDriverMock(
    grpc::ServerContext* /*context*/,
    const protobuf::Empty* /*request*/,
    protobuf::Boolean* response) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    response->set_value(is_mock_mode_);
    return Status::OK;
}

grpc::Status TestAgentServiceImpl::GetWayveDriverVersion(
    grpc::ServerContext* /*context*/,
    const protobuf::Empty* /*request*/,
    protobuf::WayveDriverVersionResponse* response) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    response->set_version(wayve_driver_version_);
    return Status::OK;
}

grpc::Status TestAgentServiceImpl::GetIntegrationStatus(
    grpc::ServerContext* /*context*/,
    const protobuf::Empty* /*request*/,
    protobuf::IntegrationStatusResponse* response) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    response->set_state(integration_state_);
    return Status::OK;
}

grpc::Status TestAgentServiceImpl::GetModelId(
    grpc::ServerContext* /*context*/,
    const protobuf::Empty* /*request*/,
    protobuf::ModelIdResponse* response) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    response->set_model_id(model_id_);
    return Status::OK;
}

grpc::Status TestAgentServiceImpl::GetServiceStatus(
    grpc::ServerContext* /*context*/,
    const protobuf::ServiceTypeRequest* request,
    protobuf::ServiceStatusResponse* response) {
    auto service_type = request->service_type();
    std::lock_guard<std::mutex> lock(state_mutex_);
    auto state = GetServiceStateSafe(service_type);
    response->set_state(state);
    return Status::OK;
}

grpc::Status TestAgentServiceImpl::StartService(
    grpc::ServerContext* /*context*/,
    const protobuf::ServiceTypeRequest* request,
    protobuf::Empty* /*response*/) {
    auto service_type = request->service_type();
    std::lock_guard<std::mutex> lock(state_mutex_);
    SetServiceStateSafe(service_type, protobuf::SERVICE_STATE_RUNNING);
    return Status::OK;
}

grpc::Status TestAgentServiceImpl::StopService(
    grpc::ServerContext* /*context*/,
    const protobuf::ServiceTypeRequest* request,
    protobuf::Empty* /*response*/) {
    auto service_type = request->service_type();
    std::lock_guard<std::mutex> lock(state_mutex_);
    SetServiceStateSafe(service_type, protobuf::SERVICE_STATE_STOPPED);
    return Status::OK;
}

grpc::Status TestAgentServiceImpl::EngageWayveDriver(
    grpc::ServerContext* /*context*/,
    const protobuf::Empty* /*request*/,
    protobuf::Empty* /*response*/) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    integration_state_ = protobuf::INTEGRATION_STATE_AV;
    return Status::OK;
}

grpc::Status TestAgentServiceImpl::DisengageWayveDriver(
    grpc::ServerContext* /*context*/,
    const protobuf::Empty* /*request*/,
    protobuf::Empty* /*response*/) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    integration_state_ = protobuf::INTEGRATION_STATE_IDLE;
    return Status::OK;
}

grpc::Status TestAgentServiceImpl::StreamTrace(
    grpc::ServerContext* context,
    const protobuf::Empty* /*request*/,
    grpc::ServerWriter<protobuf::TraceEvent>* writer) {
    int event_count = 0;
    const int max_events = 10;
    while (!context->IsCancelled() && event_count < max_events) {
        auto trace_event = GenerateMockTraceEvent();
        if (!writer->Write(trace_event)) {
            std::cout << "Failed to write trace event to client" << std::endl;
            break;
        }
        event_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return Status::OK;
}

void TestAgentServiceImpl::SetIntegrationState(protobuf::IntegrationState state) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    integration_state_ = state;
}

protobuf::ServiceState TestAgentServiceImpl::GetServiceStateSafe(
    protobuf::ServiceType service_type) const {
    if (service_states_.count(service_type) > 0) {
        return service_states_.at(service_type);
    }
    return protobuf::SERVICE_STATE_UNKNOWN;
}

void TestAgentServiceImpl::SetServiceStateSafe(
    protobuf::ServiceType service_type,
    protobuf::ServiceState state) {
    service_states_[service_type] = state;
}

protobuf::TraceEvent TestAgentServiceImpl::GenerateMockTraceEvent() const {
    protobuf::TraceEvent event;

    // Set timestamp in nanoseconds
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    event.set_timestamp_ns(nanos);

    // Randomly select a trace group
    protobuf::TraceGroup groups[] = {
        protobuf::TRACE_GROUP_TRAJECTORY,
        protobuf::TRACE_GROUP_NAVIGATION,
        protobuf::TRACE_GROUP_INFERENCE,
        protobuf::TRACE_GROUP_SAFETY_CRITICAL
    };
    event.set_groups_mask(static_cast<uint32_t>(groups[rand() % 4]));

    // Randomly select a severity
    protobuf::TraceSeverity severities[] = {
        protobuf::TRACE_SEVERITY_DEBUG,
        protobuf::TRACE_SEVERITY_INFO,
        protobuf::TRACE_SEVERITY_ERROR
    };
    event.set_severity(severities[rand() % 3]);

    // Randomly select an event type
    protobuf::TraceEventType event_types[] = {
        protobuf::TRACE_EVENT_TYPE_FUNCTION_CALL,
        protobuf::TRACE_EVENT_TYPE_LOG_MESSAGE
    };
    event.set_event_type(event_types[rand() % 2]);

    // Set a mock message
    event.set_message("Mock trace event from TestAgentService");

    return event;
}

} // namespace wayve::driver::tests 
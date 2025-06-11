#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>

#include "test_agent_service.grpc.pb.h"

namespace wayve::driver::tests {

/**
 * @brief Implementation of the TestAgentService gRPC interface.
 */
class TestAgentServiceImpl final : public wayve::driver::tests::protobuf::TestAgentService::Service {
public:
    TestAgentServiceImpl();

    ~TestAgentServiceImpl() override = default;

    /**
     * @brief Check if WayveDriver is in mock mode.
     */
    grpc::Status IsWayveDriverMock(
        grpc::ServerContext* context,
        const protobuf::Empty* request,
        protobuf::Boolean* response) override;

    /**
     * @brief Get the WayveDriver version.
     */
    grpc::Status GetWayveDriverVersion(
        grpc::ServerContext* context,
        const protobuf::Empty* request,
        protobuf::WayveDriverVersionResponse* response) override;

    /**
     * @brief Get the integration status.
     */
    grpc::Status GetIntegrationStatus(
        grpc::ServerContext* context,
        const protobuf::Empty* request,
        protobuf::IntegrationStatusResponse* response) override;

    /**
     * @brief Get the model ID.
     */
    grpc::Status GetModelId(
        grpc::ServerContext* context,
        const protobuf::Empty* request,
        protobuf::ModelIdResponse* response) override;

    /**
     * @brief Get the status of a service.
     */
    grpc::Status GetServiceStatus(
        grpc::ServerContext* context,
        const protobuf::ServiceTypeRequest* request,
        protobuf::ServiceStatusResponse* response) override;

    /**
     * @brief Start a service.
     */
    grpc::Status StartService(
        grpc::ServerContext* context,
        const protobuf::ServiceTypeRequest* request,
        protobuf::Empty* response) override;

    /**
     * @brief Stop a service.
     */
    grpc::Status StopService(
        grpc::ServerContext* context,
        const protobuf::ServiceTypeRequest* request,
        protobuf::Empty* response) override;

    /**
     * @brief Engage the WayveDriver.
     */
    grpc::Status EngageWayveDriver(
        grpc::ServerContext* context,
        const protobuf::Empty* request,
        protobuf::Empty* response) override;

    /**
     * @brief Disengage the WayveDriver.
     */
    grpc::Status DisengageWayveDriver(
        grpc::ServerContext* context,
        const protobuf::Empty* request,
        protobuf::Empty* response) override;

    /**
     * @brief Stream trace events.
     */
    grpc::Status StreamTrace(
        grpc::ServerContext* context,
        const protobuf::Empty* request,
        grpc::ServerWriter<protobuf::TraceEvent>* writer) override;

    /**
     * @brief Set the integration state (for demo/testing).
     */
    void SetIntegrationState(protobuf::IntegrationState state);

private:
    mutable std::mutex state_mutex_;

    std::unordered_map<protobuf::ServiceType,
                      protobuf::ServiceState> service_states_;

    protobuf::IntegrationState integration_state_;

    bool is_mock_mode_;
    std::string wayve_driver_version_;
    std::string model_id_;

    void InitializeServiceStates();

    protobuf::ServiceState GetServiceStateSafe(
        protobuf::ServiceType service_type) const;

    void SetServiceStateSafe(
        protobuf::ServiceType service_type,
        protobuf::ServiceState state);

    protobuf::TraceEvent GenerateMockTraceEvent() const;
};

} // namespace wayve::driver::tests 
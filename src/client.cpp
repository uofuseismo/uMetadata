#include <thread>
#include <mutex>
#include <condition_variable>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <spdlog/spdlog.h>
#include "uMetadata/client.hpp"
#include "uMetadata/station.hpp"
#include "proto/station.grpc.pb.h"

using namespace UMetadata;

class Client::ClientImpl
{
public:
    // Creates an insecure client
    explicit ClientImpl(std::string endPoint)
    {
        if (endPoint.empty())
        {
            throw std::invalid_argument("End point is empty");
        }
        std::shared_ptr<grpc::Channel> channel
            = grpc::CreateChannel(endPoint,
                                  grpc::InsecureChannelCredentials());
        mStub = std::make_unique<UMetadata::GRPC::Information::Stub> (channel); 
    }
    // Get all the stations
    [[nodiscard]] std::vector<UMetadata::Station> getAllActiveStations() const
    {
        spdlog::debug("Querying for all active stations");
        std::vector<UMetadata::Station> result;
        UMetadata::GRPC::AllActiveStationsRequest request;
        grpc::ClientContext context;
        UMetadata::GRPC::StationsResponse response;

        std::mutex mutex;
        std::condition_variable cv;
        grpc::Status status; 
        bool done{false};
        mStub->async()->GetAllActiveStations(
            &context, &request, &response,
            [&mutex, &cv, &done, &status](grpc::Status returnedStatus)
        {
            status = std::move(returnedStatus);
            std::lock_guard<std::mutex> lock(mutex);
            done = true;
            cv.notify_one();
        });

        std::unique_lock<std::mutex> lock(mutex);
        while (!done)
        {
            cv.wait(lock);
        }

        // Take action
        if (status.ok())
        {
            spdlog::debug("Successfully queried " 
                        + std::to_string(response.stations().size())
                        + " stations");
            for (const auto &station : response.stations())
            {
                result.push_back( UMetadata::Station {station} );
            }
            return result;
        }
        else
        {
            auto error = "Active stations request failed with "
                        + std::string {status.error_code()}
                        + ": " 
                        + status.error_message();
            throw std::runtime_error(error);
        }
    }
    std::unique_ptr<UMetadata::GRPC::Information::Stub> mStub{nullptr};
};

/// Constructor
Client::Client(const std::string &endPoint) :
    pImpl(std::make_unique<ClientImpl> (endPoint))
{
}

/// Gets all the current stations
std::vector<UMetadata::Station> Client::getAllActiveStations() const
{
    return pImpl->getAllActiveStations();
}

/// Destructor
Client::~Client() = default;

#include <cmath>
#include <limits>
#include <thread>
#include <mutex>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include "uMetadata/station.hpp"
#include "uMetadata/database.hpp"
#include "proto/station.pb.h"
#include "proto/station.grpc.pb.h"

struct ProgramOptions
{
    std::filesystem::path mSqlite3Database{"metdata.sqlite3"};
};

class StationInformationServiceImpl final :
    public UMetadata::GRPC::Information::CallbackService
{
public:
    explicit StationInformationServiceImpl(const ::ProgramOptions &options)
    {

    }
//    ~StationMetadataServiceImpl() = default;
    grpc::ServerUnaryReactor*
        GetAllActiveStations(grpc::CallbackServerContext *context,
                             const UMetadata::GRPC::AllActiveStationsRequest *request,
                             UMetadata::GRPC::StationsResponse *response) override
    {
        class Reactor : public grpc::ServerUnaryReactor 
        {
        public:
            Reactor(const UMetadata::Database &mDatabaseHandle,
                    const UMetadata::GRPC::AllActiveStationsRequest &request,
                    UMetadata::GRPC::StationsResponse *response)
            {
                try
                {
                    auto allStations = mDatabaseHandle.getAllActiveStations();
                    response->clear_stations();
                    for (const auto &station : allStations)
                    {
                        *response->add_stations() = station.toProtobuf();
                    }
                }
                catch (const std::exception &e)
                {
                    spdlog::error(
                        "GetAllActiveStations request query failed with "
                       + std::string{e.what()});
                    grpc::Status status{grpc::StatusCode::UNKNOWN,
                                        "Server-side query failed"};
                    Finish(status);
                }
                Finish(grpc::Status::OK);
            }
        private:
            void OnDone() override
            {
                spdlog::debug("GetAllActiveStations RPC completed");
                delete this;
            }
            void OnCancel() override
            {
                spdlog::debug("GetAllActiveStations RPC canceled");
            }
        };
        return new Reactor(*mDatabase, *request, response);
/*
        // Get the active stations
        auto reactor = context->DefaultReactor();
        reactor->Finish(grpc::Status::OK);
        return reactor;
*/
    }

    mutable std::mutex mMutex;
    std::unique_ptr<UMetadata::Database> mDatabase{nullptr};
};
/*

[[nodiscard]] std::string transformString(const std::string_view &input)
{
    std::string result{input.data(), input.size()};
    result.erase(std::remove_if(result.begin(), result.end(), isspace),
                 result.end());  
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

[[nodiscard]] double lonTo180(const double lonIn)
{
    auto lon = lonIn;
    if (lon < -180)
    {   
        for (int k = 0; k < std::numeric_limits<int>::max(); ++k)
        {
            auto tempLon = lon + k*360;
            if (tempLon >= -180)
            {
                lon = tempLon;
                break;
            }
        }
    }   
    if (lon >= 180)
    {   
        for (int k = 0; k < std::numeric_limits<int>::max(); ++k)
        {
            auto tempLon = lon - k*360;
            if (tempLon < 180)
            {
                lon = tempLon;
                break;
            }
        }
    }   
#ifndef NDEBUG
    assert(lon >= -180 && lon < 180);
#endif
    return lon;
}
*/

int main()
{
    return EXIT_SUCCESS;
}

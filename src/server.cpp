#include <cmath>
#include <fstream>
#include <sstream>
#include <limits>
#include <thread>
#include <mutex>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
//#include <grpcpp/ext/otel_plugin.h>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "uMetadata/station.hpp"
#include "uMetadata/database.hpp"
#include "proto/v1/station.pb.h"
#include "proto/v1/station.grpc.pb.h"

#include "data/utah.hpp"
#include "data/ynp.hpp"


#define APPLICATION_NAME "uMetadataServer"

namespace
{

struct ProgramOptions
{
    std::string applicationName{APPLICATION_NAME};
    std::filesystem::path sqlite3Database{"metadata.sqlite3"};
    std::filesystem::path grpcServerKey; // e.g., localhost.key
    std::filesystem::path grpcServerCertificate; // e.g., localhost.crt
    std::string grpcHost{"0.0.0.0"};
    uint16_t grpcPort{50000};
    int verbosity{3};
    bool grpcEnableReflection{false};
bool isUtah{true};
};

std::string loadStringFromFile(const std::filesystem::path &path);
std::pair<std::string, bool> parseCommandLineOptions(int argc, char *argv[]);
::ProgramOptions parseIniFile(const std::filesystem::path &iniFile);
void setVerbosityForSPDLOG(int verbosity);

}

class StationInformationServiceImpl final :
    public UMetadata::GRPC::V1::Information::CallbackService
{
public:
    explicit StationInformationServiceImpl(const ::ProgramOptions &options)
    {
        constexpr bool openReadOnly{true};
        try
        {
            mDatabase
                = std::make_unique<UMetadata::Database>
                  (options.sqlite3Database, openReadOnly); 
        }
        catch (const std::exception &e)
        {
            spdlog::critical("Failed to open read-only connection because "
                           + std::string {e.what()});
            throw std::runtime_error("Failed to open database connection");
        }
    }
    grpc::ServerUnaryReactor*
        GetAllActiveStations(grpc::CallbackServerContext *context,
                             const UMetadata::GRPC::V1::AllActiveStationsRequest *request,
                             UMetadata::GRPC::V1::StationsResponse *response) override
    {
        class Reactor : public grpc::ServerUnaryReactor 
        {
        public:
            Reactor(const UMetadata::Database &mDatabaseHandle,
                    const UMetadata::GRPC::V1::AllActiveStationsRequest &request,
                    UMetadata::GRPC::V1::StationsResponse *response)
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
    }
    grpc::ServerUnaryReactor*
        GetActiveStation(grpc::CallbackServerContext *context,
                         const UMetadata::GRPC::V1::ActiveStationRequest *request,
                         UMetadata::GRPC::V1::Station *response) override
    {   
        // Get the active stations
        auto network = request->network();
        auto name = request->name();
        grpc::Status status{grpc::Status::OK};
        try
        {
            auto result = mDatabase->getActiveStationInformation(network, name);
            if (!result)
            {
                status = grpc::Status{grpc::StatusCode::NOT_FOUND,
                                      "Could not find "
                                      + network + "." + name};
            }
            else
            {
                *response = result->toProtobuf();
            }
        }
        catch (const std::invalid_argument &e)
        {
            status = grpc::Status{grpc::StatusCode::INVALID_ARGUMENT,
                                  std::string {e.what()}};
        }
        catch (const std::exception &e)
        {
            spdlog::warn(e.what());
            status = grpc::Status{grpc::StatusCode::UNKNOWN,
                                  "Server-side query failed"};
        }
        auto reactor = context->DefaultReactor();
        reactor->Finish(status);
        return reactor;
    }

    void setHealthCheckService(
        grpc::HealthCheckServiceInterface *healthCheckService)
    {
        mHealthCheckService = healthCheckService;
    }

    mutable std::mutex mMutex;
    std::unique_ptr<UMetadata::Database> mDatabase{nullptr};
    grpc::HealthCheckServiceInterface *mHealthCheckService{nullptr};
};

void runServer(const ::ProgramOptions &options)
{
    auto serverAddress = options.grpcHost + ":"
                        + std::to_string(options.grpcPort);

    StationInformationServiceImpl service{options};

    grpc::EnableDefaultHealthCheckService(true);
    if (options.grpcEnableReflection)
    {
        spdlog::info("Enabling reflection");
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    } 
 
    grpc::ServerBuilder builder;
    if (options.grpcServerKey.empty() || options.grpcServerCertificate.empty())
    {
        spdlog::info("Initiating non-secured server");
        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    }
    else
    {
        grpc::SslServerCredentialsOptions::PemKeyCertPair keyCertPair
        {
            ::loadStringFromFile(options.grpcServerKey),
            ::loadStringFromFile(options.grpcServerCertificate) 
        };
        grpc::SslServerCredentialsOptions sslOptions; 
        sslOptions.pem_key_cert_pairs.emplace_back(keyCertPair);
        builder.AddListeningPort(serverAddress,
                                 grpc::SslServerCredentials(sslOptions));
    }
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); 
    service.setHealthCheckService(server->GetHealthCheckService());

    spdlog::info("Listening on " + serverAddress); 
    server->Wait(); 
}

int main(int argc, char *argv[])
{
    // Get the ini file from the command line
    std::filesystem::path iniFile;
    try 
    {
        auto [iniFileName, isHelp] = ::parseCommandLineOptions(argc, argv);
        if (isHelp){return EXIT_SUCCESS;}
        iniFile = iniFileName;
    }
    catch (const std::exception &e)
    {
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }

    // Read the program properties
    ::ProgramOptions programOptions;
    try
    {
        programOptions = ::parseIniFile(iniFile);
    }
    catch (const std::exception &e)
    {
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }
    ::setVerbosityForSPDLOG(programOptions.verbosity);

// kludge so i can test grpc in k8s
try 
{   
    constexpr bool readOnly{false};
    UMetadata::Database database{programOptions.sqlite3Database, readOnly};

    if (programOptions.isUtah)
    {
        spdlog::info("Making utah stations");
        auto stations = ::createStationsUtah();
        database.insert(stations);
    }
    else
    {
        spdlog::info("Making ynp stations");
        auto stations = ::createStationsYNP();
        database.insert(stations);
    }
    database.close();
}   
catch (const std::exception &e)
{
    spdlog::critical(e.what());
    return EXIT_FAILURE;
}


    try
    {
        runServer(programOptions);
    }
    catch (const std::exception &e)
    {
        spdlog::error("Server failed with " + std::string {e.what()});
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


///--------------------------------------------------------------------------///
///                            Utility Functions                             ///
///--------------------------------------------------------------------------///
namespace
{

/// Read the program options from the command line
std::pair<std::string, bool> parseCommandLineOptions(int argc, char *argv[])
{
    std::string iniFile;
    boost::program_options::options_description desc(R"""(
The uMetadataServer is a service that provides station metadata.

    uMetadataServer --ini=uMetadataServer.ini

Allowed options)""");
    desc.add_options()
        ("help", "Produces this help message")
        ("ini",  boost::program_options::value<std::string> (), 
                 "The initialization file for this executable");
    boost::program_options::variables_map vm; 
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm); 
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {    
        std::cout << desc << std::endl;
        return {iniFile, true};
    }   
    if (vm.count("ini"))
    {    
        iniFile = vm["ini"].as<std::string>();
        if (!std::filesystem::exists(iniFile))
        {
            throw std::runtime_error("Initialization file: " + iniFile
                                   + " does not exist");
        }
    }   
    return {iniFile, false};
}

std::string loadStringFromFile(const std::filesystem::path &path)
{
    std::string result;
    if (std::filesystem::exists(path)){return result;}
    std::ifstream file(path);
    if (!file.is_open())
    {   
        throw std::runtime_error("Failed to open " + path.string());
    }
    std::stringstream sstr;
    sstr << file.rdbuf();
    file.close(); 
    result = sstr.str();
    return result;
}

void setVerbosityForSPDLOG(const int verbosity)
{
    if (verbosity <= 1)
    {
        spdlog::set_level(spdlog::level::critical);
    }
    if (verbosity == 2){spdlog::set_level(spdlog::level::warn);}
    if (verbosity == 3){spdlog::set_level(spdlog::level::info);}
    if (verbosity >= 4){spdlog::set_level(spdlog::level::debug);}
}

::ProgramOptions parseIniFile(const std::filesystem::path &iniFile)
{   
    ::ProgramOptions options;
    if (!std::filesystem::exists(iniFile)){return options;}
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);

    // Application name
    options.applicationName
        = propertyTree.get<std::string> ("General.applicationName",
                                         options.applicationName);
    if (options.applicationName.empty())
    {   
        options.applicationName = APPLICATION_NAME;
    }   
    options.verbosity
        = propertyTree.get<int> ("General.verbosity", options.verbosity);

options.isUtah = propertyTree.get<bool> ("General.isUtah", options.isUtah);

    options.sqlite3Database
        = propertyTree.get<std::string> ("SQLite3.databaseFile",
                                         options.sqlite3Database.string());
/*
    if (!std::filesystem::exists(options.sqlite3Database))
    {
         throw std::invalid_argument("SQLite3 database " 
                                   + options.sqlite3Database.string()
                                   + " does not exist");
    }
*/

    options.grpcHost
        = propertyTree.get<std::string> ("gRPC.host", options.grpcHost);
    if (options.grpcHost.empty())
    {
        throw std::invalid_argument("gRPC host must be specified");
    }
    options.grpcPort 
        = propertyTree.get<uint16_t> ("gRPC.port", options.grpcPort);

    options.grpcEnableReflection
        = propertyTree.get<bool> ("gRPC.enableReflection",
                                  options.grpcEnableReflection);

    std::string grpcServerKey = "";
    grpcServerKey 
        = propertyTree.get<std::string> ("gRPC.serverKey",
                                         grpcServerKey);
    std::string grpcServerCertificate = "";
    grpcServerCertificate
        = propertyTree.get<std::string> ("gRPC.serverCertificate",
                                         grpcServerCertificate);
    if (!grpcServerKey.empty() && !grpcServerCertificate.empty())
    {
        if (!std::filesystem::exists(grpcServerKey))
        {
            throw std::invalid_argument("gRPC server key file "
                                      + grpcServerKey + " does not exist");
        }
        if (!std::filesystem::exists(grpcServerCertificate))
        {
            throw std::invalid_argument("gRPC server certificate file "
                                      + grpcServerCertificate
                                      + " does not exist");
        }
        options.grpcServerKey = grpcServerKey;
        options.grpcServerCertificate = grpcServerCertificate;
    }
    return options;
}

}

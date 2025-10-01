#ifndef UMETADATA_CLIENT_HPP
#define UMETADATA_CLIENT_HPP
#include <string>
#include <vector>
#include <memory>
namespace UMetadata
{
  class Station;
}

namespace UMetadata
{

class Client
{
    /// @brief Initializes a metadata client at the given end point.
    /// @note This will be a non-secured connection.
    explicit Client(const std::string &endPoint);

    [[nodiscard]] std::vector<UMetadata::Station> getAllActiveStations() const;

    ~Client();

    Client() = delete;
    Client(const Client &) = delete;
    Client& operator=(const Client &) = delete;
private:
    class ClientImpl;
    std::unique_ptr<ClientImpl> pImpl;
};

}
#endif 

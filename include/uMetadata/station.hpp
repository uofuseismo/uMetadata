#ifndef UMETADATA_STATION_HPP
#define UMETADATA_STATION_HPP
#include <string>
#include <chrono>
#include <optional>

namespace UMetadata::GRPC
{
  class Station;
}

namespace UMetadata
{

class Station
{
public:
    Station();
    Station(const Station &station);
    Station(Station &&station) noexcept;
    // Constructs from a protobuf.
    explicit Station(const UMetadata::GRPC::Station &station);

    void setNetwork(const std::string &network);
    [[nodiscard]] std::string getNetwork() const;
    [[nodiscard]] bool hasNetwork() const noexcept;

    void setName(const std::string &name);
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] bool hasName() const noexcept;

    void setLatitude(double latitude);
    [[nodiscard]] double getLatitude() const;
    [[nodiscard]] bool hasLatitude() const noexcept;

    void setLongitude(double longitude) noexcept;
    [[nodiscard]] double getLongitude() const;
    [[nodiscard]] bool hasLongitude() const noexcept;

    void setElevation(double elevation);
    [[nodiscard]] double getElevation() const;
    [[nodiscard]] bool hasElevation() const noexcept;

    void setStartAndEndTime(const std::pair<std::chrono::seconds, std::chrono::seconds> &startAndEndTime);
    [[nodiscard]] std::pair<std::chrono::seconds, std::chrono::seconds> getStartAndEndTime() const;
    [[nodiscard]] bool hasStartAndEndTime() const noexcept;

    void setDescription(const std::string &description) noexcept;
    [[nodiscard]] std::optional<std::string> getDescription() const noexcept;

    void setLastModified(const std::chrono::microseconds &lastModfied) noexcept;
    [[nodiscard]] std::optional<std::chrono::microseconds> getLastModified() const noexcept;

    Station& operator=(const Station &station);
    Station& operator=(Station &&station) noexcept;

    [[nodiscard]] UMetadata::GRPC::Station toProtobuf() const;
    ~Station();
private:
    class StationImpl;
    std::unique_ptr<StationImpl> pImpl;
};

}
#endif

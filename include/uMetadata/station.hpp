#ifndef UMETADATA_STATION_HPP
#define UMETADATA_STATION_HPP
#include <string>
#include <chrono>
#include <optional>

namespace UMetadataAPI::V1
{
  class Station;
}

namespace UMetadata
{

/// @class Station "station.hpp" "uMetadata/station.hpp"
/// @brief Defines a seismic station.
/// @copyright Ben Baker (UUSS) distributed under the NO AI MIT license.
class Station
{
public:
    /// @brief Constructor.
    Station();
    /// @brief Copy constructor.
    /// @param[in] station  The station from which to construct this class.
    Station(const Station &station);
    /// @brief Move constructor.
    /// @param[in,out] station  The station from which to construct this class.
    ///                         On exit, station's behavior is undefined.
    Station(Station &&station) noexcept;
    /// @brief Constructs from a protobuf.
    /// @param[in] station  The station representation as a protobuf.
    explicit Station(const UMetadataAPI::V1::Station &station);

    /// @name Required Properties
    /// @{

    /// @brief Sets the network code.
    /// @param[in] network  The network code - e.g., UU.
    void setNetwork(const std::string &network);
    /// @result The network code.
    /// @throws std::runtime_error if \c hasNetwork() is false.
    [[nodiscard]] std::string getNetwork() const;
    /// @result True indicates the network was set.
    [[nodiscard]] bool hasNetwork() const noexcept;

    /// @brief Sets the station's name.
    /// @param[in] name  The station name - e.g., CWU.
    void setName(const std::string &name);
    /// @result The station name.
    /// @throws std::runtime_error if \c hasName() is false.
    [[nodiscard]] std::string getName() const;
    /// @result True indicates the station name was set.
    [[nodiscard]] bool hasName() const noexcept;

    /// @brief Sets the latitude of the station.
    /// @param[in] latitude  The latitude of the station in degrees.
    /// @throws std::invalid_argument if the latitude is not in the range of
    ///         [-90,90].
    void setLatitude(double latitude);
    /// @result The station's latitude in degrees.
    /// @throws std::runtime_error if \c hasLatitude() is false.
    [[nodiscard]] double getLatitude() const;
    /// @result True indicates the latitude was set.
    [[nodiscard]] bool hasLatitude() const noexcept;

    /// @brief Sets the longitude of the station.
    /// @param[in] longitude  The longitude of the station in degrees.  This is
    ///                       measured positive east of 0.
    void setLongitude(double longitude) noexcept;
    /// @result The station's longitude in degrees.  This will be
    ///         the range of [-180, 180).
    /// @throws std::runtime_error if \c hasLongitude() is false.
    [[nodiscard]] double getLongitude() const;
    /// @result True indicates the longitude was set.
    [[nodiscard]] bool hasLongitude() const noexcept;

    /// @brief Sets the elevation of the station.
    /// @param[in] elevation  The elevation of the station in meters.  This is 
    ///                       measured positive from sea-level.
    void setElevation(double elevation);
    /// @result The elevation of the station in meters.  
    /// @throws std::runtime_error if \c hasElevation() is false.
    [[nodiscard]] double getElevation() const;
    /// @result True indicates the elevation was set.
    [[nodiscard]] bool hasElevation() const noexcept;

    /// @brief Sets the start and end time of the station.
    /// @param[in] startAndEndTime  The start and end UTC time of the station 
    ///                             measured in seconds since the epoch
    ///                             (Jan 1 1970).
    /// @throws std::invalid_argument if
    ///         startAndEndTime.first >= startAndEndTime.second.
    void setStartAndEndTime(const std::pair<std::chrono::seconds, std::chrono::seconds> &startAndEndTime);
    /// @result The start and end UTC time in seconds since the epoch
    [[nodiscard]] std::pair<std::chrono::seconds, std::chrono::seconds> getStartAndEndTime() const;
    /// @result True indicates the station's start and end time was set.
    [[nodiscard]] bool hasStartAndEndTime() const noexcept;
    /// @}

    /// @name Optional Parameters
    /// @{
 
    /// @brief Sets a brief station description.
    /// @param[in] description   A description of the station.
    void setDescription(const std::string &description) noexcept;
    /// @result A description of the staiton.
    [[nodiscard]] std::optional<std::string> getDescription() const noexcept;

    /// @param[in] lastModified  The last UTC time this station's information
    ///                          was modified in microseconds since the epoch.
    void setLastModified(const std::chrono::microseconds &lastModified) noexcept;
    /// @result The time was the station's information was modified.
    /// @note By default this will be the class creation time.
    [[nodiscard]] std::chrono::microseconds getLastModified() const noexcept;
    /// @}

    /// @result The station expressed as a protobuf for gRPC communication.
    /// @throws std::runtime_error if any of the required values are not set.
    [[nodiscard]] UMetadataAPI::V1::Station toProtobuf() const;

    /// @brief Copy assignment.
    /// @param[in] station  The station to copy to this.
    /// @result A deep copy of the station.
    Station& operator=(const Station &station);
    /// @brief Move assignment.
    /// @param[in,out] station  The station whose memory will be moved to this.
    ///                         On exit, station's behavior is undefined.
    /// @result The memory from station moved to this. 
    Station& operator=(Station &&station) noexcept;

    /// @brief Destructor.
    ~Station();
private:
    class StationImpl;
    std::unique_ptr<StationImpl> pImpl;
};

}
#endif

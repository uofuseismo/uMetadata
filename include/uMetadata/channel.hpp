#ifndef UMETADATA_CHANNEL_HPP
#define UMETADATA_CHANNEL_HPP
#include <string>
#include <chrono>
#include <optional>

namespace UMetadata::V1
{
  class Channel;
}

namespace UMetadata
{

/// @class Channel "channel.hpp" "uMetadata/channel.hpp"
/// @brief Defines a channel in a seismic sensor.
/// @copyright Ben Baker (UUSS) distributed under the NO AI MIT license.
class Channel
{
public:
    /// @brief Constructor.
    Channel();
    /// @brief Copy constructor.
    /// @param[in] channel  The channel from which to construct this class.
    Channel(const Channel &channel);
    /// @brief Move constructor.
    /// @param[in,out] channel  The channel from which to construct this class.
    ///                         On exit, channel's behavior is undefined.
    Channel(Channel &&channel) noexcept;
    /// @brief Constructs from a protobuf.
    /// @param[in] channel  The channel representation as a protobuf.
    explicit Channel(const UMetadata::V1::Channel &channel);

    /// @name Required Properties
    /// @{

    /// @brief Sets the network code.
    /// @param[in] network  The network code - e.g., UU.
    void setNetwork(const std::string &network);
    /// @result The network code.
    /// @throws std::runtime_error if \c hasNetwork() is false.
    [[nodiscard]] std::string getNetwork() const;
    // @result True indicates the network code was set.
    [[nodiscard]] bool hasNetwork() const noexcept;

    /// @brief Sets the station name to which this channel belongs.
    /// @param[in] station  The station code - e.g., CWU.
    void setStation(const std::string &station);
    /// @result The staiton name.
    /// @throws std::runtime_error if \c hasStation() is false.
    [[nodiscard]] std::string getStation() const;
    // @result True indicates the station name was set.
    [[nodiscard]] bool hasStation() const noexcept;

    /// @brief Sets the channel's name.
    /// @param[in] name  The channel name - e.g., HHZ.
    void setName(const std::string &name);
    /// @result The channel name.
    /// @throws std::runtime_error if \c hasName() is false.
    [[nodiscard]] std::string getName() const;
    /// @result True indicates the channel name was set.
    [[nodiscard]] bool hasName() const noexcept;

    /// @brief Sets the location code.
    /// @param[in] locationCode  The location code - e.g., 01.
    /// @note This can be blank.
    void setLocationCode(const std::string &locationCode);
    /// @result The location code.
    /// @throws std::runtime_error if \c hasLocationCode() is false.
    [[nodiscard]] std::string getLocationCode() const;
    /// @result True indicates the location code was set.
    [[nodiscard]] bool hasLocationCode() const noexcept;

    /// @brief Sets the latitude of the channel.
    /// @param[in] latitude  The latitude of the channel in degrees.
    /// @throws std::invalid_argument if the latitude is not in the range of
    ///         [-90,90].
    void setLatitude(double latitude);
    /// @result The channel's latitude in degrees.
    /// @throws std::runtime_error if \c hasLatitude() is false.
    [[nodiscard]] double getLatitude() const;
    /// @result True indicates the latitude was set.
    [[nodiscard]] bool hasLatitude() const noexcept;

    /// @brief Sets the longitude of the channel.
    /// @param[in] longitude  The longitude of the channel in degrees.  This is
    ///                       measured positive east of 0.
    void setLongitude(double longitude) noexcept;
    /// @result The channel's longitude in degrees.  This will be
    ///         the range of [-180, 180).
    /// @throws std::runtime_error if \c hasLongitude() is false.
    [[nodiscard]] double getLongitude() const;
    /// @result True indicates the longitude was set.
    [[nodiscard]] bool hasLongitude() const noexcept;

    /// @brief Sets the elevation of the channel.
    /// @param[in] elevation  The elevation of the channel in meters.  This is 
    ///                       measured positive from sea-level.
    void setElevation(double elevation);
    /// @result The elevation of the channel in meters.  
    /// @throws std::runtime_error if \c hasElevation() is false.
    [[nodiscard]] double getElevation() const;
    /// @result True indicates the elevation was set.
    [[nodiscard]] bool hasElevation() const noexcept;

    /// @brief Sets the channel's nominal sampling rate.
    /// @param[in] samplingRate  The nominal sampling rate in Hz.
    void setSamplingRate(double samplingRate);
    /// @result The sampling rate in Hz.
    /// @throws std::runtime_error if \hasSamplingRate() is false.
    [[nodiscard]] double getSamplingRate() const;
    /// @result True indicates the sampling rate was set.
    [[nodiscard]] bool hasSamplingRate() const noexcept;

    /// @brief Sets the azimuth of the channel.
    /// @param[in] azimuth  The channel's azimuth measured in degrees positive
    ///                     east of north - i.e., 0 is north and 90 is east.
    /// @throws std::invalid_argument if azimuth is not in the range [0, 360).
    void setAzimuth(double azimuth);
    /// @result The azimuth of the channel in degrees.
    /// @throws std::runtime_error if \c hasAzimuth() is false.
    [[nodiscard]] double getAzimuth() const;
    /// @result True indicates the azimuth was set.
    [[nodiscard]] bool hasAzimuth() const noexcept;

    /// @brief Sets the dip of the channel.
    /// @param[in] dip  The channel's dip measured in degrees positive down from
    ///                 horizontal - i.e., -90 is positive up and 90 is positive
    ///                 down.
    /// @throws std::invalid_argument if azimuth is not in the range [-90, 90].
    void setDip(double dip);
    /// @result The dip of the channel in degrees.
    /// @throws std::runtime_error if \c hasDip() is false.
    [[nodiscard]] double getDip() const;
    /// @result True indicates the dip was set.
    [[nodiscard]] bool hasDip() const noexcept;

    /// @brief Sets the start and end time of the channel.
    /// @param[in] startAndEndTime  The start and end UTC time of the channel 
    ///                             measured in seconds since the epoch
    ///                             (Jan 1 1970).
    /// @throws std::invalid_argument if
    ///         startAndEndTime.first >= startAndEndTime.second.
    void setStartAndEndTime(const std::pair<std::chrono::seconds, std::chrono::seconds> &startAndEndTime);
    /// @result The start and end UTC time in seconds since the epoch
    [[nodiscard]] std::pair<std::chrono::seconds, std::chrono::seconds> getStartAndEndTime() const;
    /// @result True indicates the channel's start and end time was set.
    [[nodiscard]] bool hasStartAndEndTime() const noexcept;
    /// @}

    /// @name Optional Parameters
    /// @{
 
    /// @brief Sets when the channel information was last modified.
    /// @param[in] lastModified  The UTC time in microseconds since the epoch
    ///                          when the channel information was last modified.
    void setLastModified(const std::chrono::microseconds &lastModfied) noexcept;
    /// @result The time was the channel's information was modified.
    /// @note By default this is the time of class creation.
    [[nodiscard]] std::chrono::microseconds getLastModified() const noexcept;
    /// @}

    /// @result The channel expressed as a protobuf for gRPC communication.
    /// @throws std::runtime_error if any of the required values are not set.
    [[nodiscard]] UMetadata::V1::Channel toProtobuf() const;

    /// @brief Copy assignment.
    /// @param[in] channel  The channel to copy to this.
    /// @result A deep copy of the channel.
    Channel& operator=(const Channel &channel);
    /// @brief Move assignment.
    /// @param[in,out] channel  The channel whose memory will be moved to this.
    ///                         On exit, channel's behavior is undefined.
    /// @result The memory from channel moved to this. 
    Channel& operator=(Channel &&channel) noexcept;

    /// @brief Destructor.
    ~Channel();
private:
    class ChannelImpl;
    std::unique_ptr<ChannelImpl> pImpl;
};

}
#endif

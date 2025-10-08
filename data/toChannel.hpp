#ifndef TO_CHANNEL_HPP
#define TO_CHANNEL_HPP
#include <string>
#include <chrono>
#include <uMetadata/channel.hpp>
namespace 
{
[[nodiscard]] UMetadata::Channel toChannel(
     const std::string &network, const std::string &station,
     const std::string &name, const std::string &locationCode,
     const double latitude, const double longitude, const double elevation,
     const double samplingRate, const double azimuth, const double dip,
     const int64_t startTime, const int64_t endTime, const int64_t lastModified)
{
    UMetadata::Channel result;
    result.setNetwork(network);
    result.setStation(station);
    result.setName(name);
    result.setLocationCode(locationCode);
    result.setLatitude(latitude);
    result.setLongitude(longitude);
    result.setElevation(elevation);
    result.setSamplingRate(samplingRate);
    result.setAzimuth(azimuth);
    result.setDip(dip);
    result.setStartAndEndTime( std::pair {std::chrono::seconds {startTime}, std::chrono::seconds {endTime}} );
    result.setLastModified( std::chrono::seconds {lastModified} );
    return result;
}
}
#endif

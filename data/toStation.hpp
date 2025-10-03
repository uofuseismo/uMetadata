#ifndef TO_STATION_HPP
#define TO_STATION_HPP
#include <string>
#include <chrono>
#include <uMetadata/station.hpp>
namespace 
{
[[nodiscard]] UMetadata::Station toStation(const std::string &network,
     const std::string &name, const std::string &description,
     const double latitude, const double longitude, const double elevation,
     const int64_t startTime, const int64_t endTime, const int64_t lastModified)
{
    UMetadata::Station result;
    result.setNetwork(network);
    result.setName(name);
    result.setDescription(description);
    result.setLatitude(latitude);
    result.setLongitude(longitude);
    result.setElevation(elevation);
    result.setStartAndEndTime( std::pair {std::chrono::seconds {startTime}, std::chrono::seconds {endTime}} );
    result.setLastModified( std::chrono::seconds {lastModified} );
    return result;
}
}
#endif

#include <string>
#include <chrono>
#ifndef NDEBUG
#include <cassert>
#endif
#include "uMetadata/station.hpp"
#include "proto/station.pb.h"

using namespace UMetadata;

namespace
{
[[nodiscard]] std::chrono::microseconds getNow() 
{                   
     auto now    
        = std::chrono::duration_cast<std::chrono::microseconds>
          ((std::chrono::high_resolution_clock::now()).time_since_epoch());
     return now;    
}                

[[nodiscard]] std::chrono::seconds getYear3000() noexcept
{
    return std::chrono::seconds {32503680000};

}

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

}

class Station::StationImpl
{
public:
    std::string mNetwork;
    std::string mName;
    std::string mDescription;
    std::chrono::seconds mStartTime{0};
    std::chrono::seconds mEndTime{::getYear3000()};
    std::chrono::microseconds mLastModified{::getNow()};
    double mLatitude{0};
    double mLongitude{0};
    double mElevation{0};
    bool mHasLatitude{false};
    bool mHasLongitude{false};
    bool mHasElevation{false};
    bool mHasStartAndEndTime{false};
    bool mHasDescription{false};
    bool mHasLastModified{false};
};

/// Constructor
Station::Station() :
    pImpl(std::make_unique<StationImpl> ())
{
}

/// Copy constructor
Station::Station(const Station &station)
{
    *this = station;
}

/// Move constructor.
Station::Station(Station &&station) noexcept
{
    *this = std::move(station);
}

/// Create from a protobuf
Station::Station(const UMetadata::GRPC::Station &station) :
    pImpl(std::make_unique<StationImpl> ())
{
    Station work;
    work.setNetwork(station.network());
    work.setName(station.name());
    auto startTime = std::chrono::seconds {station.start_time()};
    auto endTime = std::chrono::seconds {station.end_time()};
    work.setStartAndEndTime(std::pair {startTime, endTime});
    work.setLongitude(station.longitude());
    work.setLatitude(station.latitude()); 
    work.setElevation(station.elevation());
    auto lastModified = station.last_modified_mus();
    if (lastModified)
    {
        work.setLastModified(std::chrono::microseconds {lastModified});
    }
    else
    {
        work.setLastModified(::getNow());
    }
    if (station.has_description())
    {
        work.setDescription(station.description());
    }
    *this = std::move(work);
}

/// Copy assignment
Station& Station::operator=(const Station &station)
{
    if (&station == this){return *this;}
    pImpl = std::make_unique<StationImpl> (*station.pImpl);
    return *this;
}

/// Move assignment
Station& Station::operator=(Station &&station) noexcept
{
    if (&station == this){return *this;}
    pImpl = std::move(station.pImpl);
    return *this;
}

/// Destructor
Station::~Station() = default;

/// Network
void Station::setNetwork(const std::string &networkIn)
{    
    auto network = ::transformString(networkIn);
    if (network.empty()){throw std::invalid_argument("Network is empty");}
    pImpl->mNetwork = network;
}

std::string Station::getNetwork() const
{
    if (!hasNetwork()){throw std::runtime_error("Network not set");}
    return pImpl->mNetwork;
}

bool Station::hasNetwork() const noexcept
{
    return !pImpl->mNetwork.empty();
}

/// Name
void Station::setName(const std::string &nameIn)
{
    auto name = ::transformString(nameIn);
    if (name.empty()){throw std::invalid_argument("Name is empty");}
    pImpl->mName = name;
}

std::string Station::getName() const
{
    if (!hasName()){throw std::runtime_error("Name not set");}
    return pImpl->mName;
}

bool Station::hasName() const noexcept
{
    return !pImpl->mName.empty();
}

/// Latitude
void Station::setLatitude(const double latitude)
{
    if (latitude < -90 || latitude > 90) 
    {
        throw std::invalid_argument("Latitude " + std::to_string(latitude)
                                  + " must be in range [-90,90]");
    }
    pImpl->mLatitude = latitude;
    pImpl->mHasLatitude = true;
}

double Station::getLatitude() const
{
    if (!hasLatitude()){throw std::runtime_error("Latitude not set");}
    return pImpl->mLatitude;
}

bool Station::hasLatitude() const noexcept
{
    return pImpl->mHasLatitude;
}

/// Longitude
void Station::setLongitude(const double longitude) noexcept
{
    pImpl->mLongitude = ::lonTo180(longitude);
    pImpl->mHasLongitude = true;
}

double Station::getLongitude() const
{
    if (!hasLongitude()){throw std::runtime_error("Longitude not set");}
    return pImpl->mLongitude;
}

bool Station::hasLongitude() const noexcept
{
    return pImpl->mHasLongitude;
}

/// Elevation
void Station::setElevation(const double elevation)
{
    if (elevation > 8600 || elevation < -10000)
    {
        throw std::invalid_argument("Elevation " + std::to_string(elevation)
                                  + " must be in range [-10000, 8600]");
    }
    pImpl->mElevation = elevation;
    pImpl->mHasElevation = true;
}

double Station::getElevation() const
{
    if (!hasElevation()){throw std::runtime_error("Elevation not set");}
    return pImpl->mElevation;
}

bool Station::hasElevation() const noexcept
{
    return pImpl->mHasElevation;
}

/// Start and end time
void Station::setStartAndEndTime(
    const std::pair<std::chrono::seconds, std::chrono::seconds> &startAndEndTime)
{
    if (startAndEndTime.first >= startAndEndTime.second)
    {
        throw std::invalid_argument("start time must be less than end time");
    }
    pImpl->mStartTime = startAndEndTime.first;
    pImpl->mEndTime = startAndEndTime.second;
    pImpl->mHasStartAndEndTime = true;
}

std::pair<std::chrono::seconds, std::chrono::seconds>
Station::getStartAndEndTime() const
{
    if (!hasStartAndEndTime())
    {
        throw std::runtime_error("start and end time not set");
    }
    return std::pair{pImpl->mStartTime, pImpl->mEndTime};
}

bool Station::hasStartAndEndTime() const noexcept
{
    return pImpl->mHasStartAndEndTime;
}

/// Description
void Station::setDescription(const std::string &description) noexcept
{
    pImpl->mDescription = description;
    pImpl->mHasDescription = true;
}

std::optional<std::string> Station::getDescription() const noexcept
{
    return pImpl->mHasDescription ?
           std::optional<std::string> (pImpl->mDescription) : std::nullopt;
}

/// Last modified
void Station::setLastModified(
    const std::chrono::microseconds &lastModified) noexcept
{
    pImpl->mLastModified = lastModified;
    pImpl->mHasLastModified = true;
}

std::optional<std::chrono::microseconds> 
Station::getLastModified() const noexcept
{
    return pImpl->mHasLastModified ?
           std::optional<std::chrono::microseconds> (pImpl->mLastModified) :
           std::nullopt;
}

[[nodiscard]] UMetadata::GRPC::Station Station::toProtobuf() const
{
    UMetadata::GRPC::Station result;
    *result.mutable_network() = getNetwork();
    *result.mutable_name() = getName();
    result.set_latitude(getLatitude());
    result.set_longitude(getLongitude());
    result.set_elevation(getElevation());
    auto [startTime, endTime] = getStartAndEndTime();
    result.set_start_time(startTime.count());
    result.set_end_time(endTime.count());
    auto lastModified = getLastModified();
    if (lastModified)
    {
        result.set_last_modified_mus(lastModified->count());
    }
    else
    {
        result.set_last_modified_mus(::getNow().count());
    }
    auto description = getDescription();
    if (description){*result.mutable_description() = *description;}
    return result;
}


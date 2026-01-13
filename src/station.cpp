#include <string>
#include <chrono>
#ifndef NDEBUG
#include <cassert>
#endif
#include <google/protobuf/util/time_util.h>
#include "uMetadata/station.hpp"
#include "utilities.hpp"
#include "proto/v1/station.pb.h"

using namespace UMetadata;

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
Station::Station(const UMetadata::V1::Station &station) :
    pImpl(std::make_unique<StationImpl> ())
{
    Station work;
    work.setNetwork(station.network());
    work.setName(station.name());
    
    auto startTime = std::chrono::seconds {station.start_time().seconds()};
    auto endTime = std::chrono::seconds {station.end_time().seconds()};
    work.setStartAndEndTime(std::pair {startTime, endTime});
    work.setLongitude(station.longitude());
    work.setLatitude(station.latitude()); 
    work.setElevation(station.elevation());
    auto lastModifiedMuS
         = static_cast<int64_t> (
              std::round(
                  station.last_modified().seconds()
                + station.last_modified().nanos()*1.e-9)
           )*1000000;
    work.setLastModified(std::chrono::microseconds {lastModifiedMuS});
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
}

std::chrono::microseconds Station::getLastModified() const noexcept
{
    return pImpl->mLastModified;
}

[[nodiscard]] UMetadata::V1::Station Station::toProtobuf() const
{
    UMetadata::V1::Station result;
    *result.mutable_network() = getNetwork();
    *result.mutable_name() = getName();
    result.set_latitude(getLatitude());
    result.set_longitude(getLongitude());
    result.set_elevation(getElevation());
    auto [startTime, endTime] = getStartAndEndTime();
    auto startTimeProtobuf
        = google::protobuf::util::TimeUtil::SecondsToTimestamp(
             startTime.count());
    auto endTimeProtobuf
         = google::protobuf::util::TimeUtil::SecondsToTimestamp(
             endTime.count());
    *result.mutable_start_time() = std::move(startTimeProtobuf);
    *result.mutable_end_time() = std::move(endTimeProtobuf);
    auto lastModifiedProtobuf
        = google::protobuf::util::TimeUtil::MicrosecondsToTimestamp(
            getLastModified().count());
    *result.mutable_last_modified() = std::move(lastModifiedProtobuf);
    auto description = getDescription();
    if (description){*result.mutable_description() = *description;}
    return result;
}


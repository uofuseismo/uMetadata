#include <string>
#include <chrono>
#include <google/protobuf/util/time_util.h>
#include "uMetadata/channel.hpp"
#include "proto/v1/channel.pb.h"
#include "utilities.hpp"

using namespace UMetadata;

class Channel::ChannelImpl
{
public:
    std::string mNetwork;
    std::string mStation;
    std::string mName;
    std::string mLocationCode;
    std::chrono::seconds mStartTime{0};
    std::chrono::seconds mEndTime{::getYear3000()};
    std::chrono::microseconds mLastModified{::getNow()};
    double mLatitude{0};
    double mLongitude{0};
    double mElevation{0};
    double mSamplingRate{0};
    double mDip{0};
    double mAzimuth{0};
    bool mHasLatitude{false};
    bool mHasLongitude{false};
    bool mHasElevation{false};
    bool mHasDip{false};
    bool mHasAzimuth{false};
    bool mHaveLocationCode{false};
    bool mHasStartAndEndTime{false};
};

/// Constructor
Channel::Channel() :
    pImpl(std::make_unique<ChannelImpl> ())
{
}

/// Copy constructor
Channel::Channel(const Channel &channel)
{
    *this = channel;
}

/// Move constructor
Channel::Channel(Channel &&channel) noexcept
{
    *this = std::move(channel);
}

/// Create from a protobuf
Channel::Channel(const UMetadata::V1::Channel &channel) :
    pImpl(std::make_unique<ChannelImpl> ())
{
    Channel work;
    work.setNetwork(channel.network());
    work.setStation(channel.station());
    work.setName(channel.name());
    work.setLocationCode(channel.location_code());

    auto startTime = std::chrono::seconds {channel.start_time().seconds()};
    auto endTime = std::chrono::seconds {channel.end_time().seconds()};
    work.setStartAndEndTime(std::pair {startTime, endTime});
    work.setLongitude(channel.longitude());
    work.setLatitude(channel.latitude()); 
    work.setElevation(channel.elevation());
    work.setSamplingRate(channel.sampling_rate());
    work.setAzimuth(channel.azimuth());
    work.setDip(channel.dip());
    auto lastModifiedMuS
         = static_cast<int64_t> (
              std::round(
                  channel.last_modified().seconds()
                + channel.last_modified().nanos()*1.e-9)
           )*1000000;
    work.setLastModified(std::chrono::microseconds {lastModifiedMuS});
    *this = std::move(work);
}

/// Copy assignment
Channel& Channel::operator=(const Channel &channel)
{
    if (&channel == this){return *this;}
    pImpl = std::make_unique<ChannelImpl> (*channel.pImpl);
    return *this;
}

/// Move assignment
Channel& Channel::operator=(Channel &&channel) noexcept
{
    if (&channel == this){return *this;}
    pImpl = std::move(channel.pImpl);
    return *this;
}

/// Network
void Channel::setNetwork(const std::string &networkIn)
{    
    auto network = ::transformString(networkIn);
    if (network.empty()){throw std::invalid_argument("Network is empty");}
    pImpl->mNetwork = network;
}

std::string Channel::getNetwork() const
{
    if (!hasNetwork()){throw std::runtime_error("Network not set");}
    return pImpl->mNetwork;
}

bool Channel::hasNetwork() const noexcept
{
    return !pImpl->mNetwork.empty();
}

/// Station
void Channel::setStation(const std::string &stationIn)
{    
    auto station= ::transformString(stationIn);
    if (station.empty()){throw std::invalid_argument("Station is empty");}
    pImpl->mStation = station;
}

std::string Channel::getStation() const
{
    if (!hasStation()){throw std::runtime_error("Station not set");}
    return pImpl->mStation;
}

bool Channel::hasStation() const noexcept
{
    return !pImpl->mStation.empty();
}

/// Name
void Channel::setName(const std::string &nameIn)
{
    auto name = ::transformString(nameIn);
    if (name.empty()){throw std::invalid_argument("Name is empty");}
    pImpl->mName = name;
}

std::string Channel::getName() const
{
    if (!hasName()){throw std::runtime_error("Name not set");}
    return pImpl->mName;
}

bool Channel::hasName() const noexcept
{
    return !pImpl->mName.empty();
}

/// Location code
void Channel::setLocationCode(const std::string &locationCodeIn)
{
    auto locationCode = ::transformString(locationCodeIn);
    if (locationCode.empty())
    {
        locationCode = "--";
    }
    else
    {
        pImpl->mLocationCode = locationCode;
    }
}

std::string Channel::getLocationCode() const
{
    if (!hasLocationCode())
    {
        throw std::runtime_error("Location code not set");
    }
    return pImpl->mLocationCode;
}

bool Channel::hasLocationCode() const noexcept
{
    return !pImpl->mLocationCode.empty();
}

/// Latitude
void Channel::setLatitude(const double latitude)
{
    if (latitude < -90 || latitude > 90) 
    {   
        throw std::invalid_argument("Latitude " + std::to_string(latitude)
                                  + " must be in range [-90,90]");
    }   
    pImpl->mLatitude = latitude;
    pImpl->mHasLatitude = true;
}

double Channel::getLatitude() const
{
    if (!hasLatitude()){throw std::runtime_error("Latitude not set");}
    return pImpl->mLatitude;
}

bool Channel::hasLatitude() const noexcept
{
    return pImpl->mHasLatitude;
}

/// Longitude
void Channel::setLongitude(const double longitude) noexcept
{
    pImpl->mLongitude = ::lonTo180(longitude);
    pImpl->mHasLongitude = true;
}

double Channel::getLongitude() const
{
    if (!hasLongitude()){throw std::runtime_error("Longitude not set");}
    return pImpl->mLongitude;
}

bool Channel::hasLongitude() const noexcept
{
    return pImpl->mHasLongitude;
}

/// Elevation
void Channel::setElevation(const double elevation)
{
    if (elevation > 8600 || elevation < -10000)
    {   
        throw std::invalid_argument("Elevation " + std::to_string(elevation)
                                  + " must be in range [-10000, 8600]");
    }   
    pImpl->mElevation = elevation;
    pImpl->mHasElevation = true;
}

double Channel::getElevation() const
{
    if (!hasElevation()){throw std::runtime_error("Elevation not set");}
    return pImpl->mElevation;
}

bool Channel::hasElevation() const noexcept
{
    return pImpl->mHasElevation;
}

/// Sampling rate
void Channel::setSamplingRate(const double samplingRate)
{
    if (samplingRate <= 0)
    {
        throw std::invalid_argument("samplingRate = "
                                   + std::to_string(samplingRate)
                                   + " must be positive");
    }
    pImpl->mSamplingRate = samplingRate;
}

double Channel::getSamplingRate() const
{
    if (!hasSamplingRate())
    {
        throw std::runtime_error("Sampling rate not set");
    }
    return pImpl->mSamplingRate;
}

bool Channel::hasSamplingRate() const noexcept
{
    return pImpl->mSamplingRate > 0;
}

/// Dip
void Channel::setDip(const double dip)
{
    if (dip < -90 || dip > 90)
    {
        throw std::invalid_argument("dip = " 
                                  + std::to_string(dip)
                                  + " must be in range [-90,90]");
    }
    pImpl->mDip = dip;
    pImpl->mHasDip = true;
}

double Channel::getDip() const
{
    if (!hasDip()){throw std::runtime_error("Dip not set");}
    return pImpl->mDip;
}

bool Channel::hasDip() const noexcept
{
    return pImpl->mHasDip;
}

/// Azimuth
void Channel::setAzimuth(const double azimuth)
{
    if (azimuth < 0 || azimuth >= 360)
    {
        throw std::invalid_argument("azimuth = "
                                  + std::to_string(azimuth)
                                  + " must be in range [0, 360)");
    }
    pImpl->mAzimuth = azimuth;
    pImpl->mHasAzimuth = true;
}

double Channel::getAzimuth() const
{
    if (!hasAzimuth()){throw std::runtime_error("Azimuth not set");}
    return pImpl->mAzimuth;
}

bool Channel::hasAzimuth() const noexcept
{
    return pImpl->mHasAzimuth;
}

/// Start and end time
void Channel::setStartAndEndTime(
    const std::pair<std::chrono::seconds, std::chrono::seconds>
    &startAndEndTime)
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
Channel::getStartAndEndTime() const
{
    if (!hasStartAndEndTime())
    {   
        throw std::runtime_error("start and end time not set");
    }   
    return std::pair{pImpl->mStartTime, pImpl->mEndTime};
}

bool Channel::hasStartAndEndTime() const noexcept
{
    return pImpl->mHasStartAndEndTime;
}

/// Last modified
void Channel::setLastModified(
    const std::chrono::microseconds &lastModified) noexcept
{
    pImpl->mLastModified = lastModified;
}

std::chrono::microseconds Channel::getLastModified() const noexcept
{
    return pImpl->mLastModified;
}

/// Destructor
Channel::~Channel() = default;

[[nodiscard]] UMetadata::V1::Channel Channel::toProtobuf() const
{
    UMetadata::V1::Channel result;
    *result.mutable_network() = getNetwork();
    *result.mutable_station() = getStation();
    *result.mutable_name() = getName();
    *result.mutable_location_code() = getLocationCode();
    result.set_latitude(getLatitude());
    result.set_longitude(getLongitude());
    result.set_elevation(getElevation());
    result.set_sampling_rate(getSamplingRate());
    result.set_azimuth(getAzimuth());
    result.set_dip(getDip());
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
    return result;
}


#include <vector>
#include <string>
#include <chrono>
#include <limits>
#include <google/protobuf/util/time_util.h>
#include "uMetadata/channel.hpp"
#include "proto/v1/channel.pb.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("UMetadata::Channel", "[channel]")
{
    std::string network{"UU"};
    std::string station{"CTU"};
    std::string name{"HHZ"};
    std::string locationCode{"01"};
    constexpr double latitude{40.6925};
    constexpr double longitude{-111.75034};
    constexpr double elevation{1731.0};
    constexpr double samplingRate{100};
    constexpr double dip{-90};
    constexpr double azimuth{0};
    const std::chrono::seconds startTime{1590624000};
    const std::chrono::seconds endTime{32503680000};
    const std::chrono::seconds lastModified{1727969872};

    UMetadata::Channel channel;
    REQUIRE_NOTHROW(channel.setNetwork(network));
    REQUIRE_NOTHROW(channel.setStation(station));
    REQUIRE_NOTHROW(channel.setName(name));
    REQUIRE_NOTHROW(channel.setLocationCode(locationCode));
    REQUIRE_NOTHROW(channel.setLatitude(latitude));
    channel.setLongitude(longitude);
    REQUIRE_NOTHROW(channel.setElevation(elevation));
    REQUIRE_NOTHROW(channel.setSamplingRate(samplingRate));
    REQUIRE_NOTHROW(channel.setAzimuth(azimuth));
    REQUIRE_NOTHROW(channel.setDip(dip));
    REQUIRE_NOTHROW(channel.setStartAndEndTime(std::pair {startTime, endTime}));
    REQUIRE_NOTHROW(channel.setLastModified(lastModified));

    REQUIRE(channel.getNetwork() == network);
    REQUIRE(channel.getStation() == station);
    REQUIRE(channel.getName() == name);
    REQUIRE(channel.getLocationCode() == locationCode);
    REQUIRE_THAT(channel.getLatitude(),
                 Catch::Matchers::WithinAbs(latitude, 1.e-10));
    REQUIRE_THAT(channel.getLongitude(),
                 Catch::Matchers::WithinAbs(longitude, 1.e-10));
    REQUIRE_THAT(channel.getElevation(),
                 Catch::Matchers::WithinAbs(elevation, 1.e-10));
    REQUIRE_THAT(channel.getSamplingRate(),
                 Catch::Matchers::WithinAbs(samplingRate, 1.e-10));
    REQUIRE_THAT(channel.getDip(),
                 Catch::Matchers::WithinAbs(dip, 1.e-10));
    REQUIRE_THAT(channel.getAzimuth(),
                 Catch::Matchers::WithinAbs(azimuth, 1.e-10));
    REQUIRE(channel.getStartAndEndTime().first == startTime);
    REQUIRE(channel.getStartAndEndTime().second == endTime);
    REQUIRE(channel.getLastModified() ==
            std::chrono::microseconds {lastModified});

    SECTION("To Protobuf")
    {   
        auto proto = channel.toProtobuf();
        REQUIRE(proto.network() == network); 
        REQUIRE(proto.station() == station);
        REQUIRE(proto.name() == name);
        REQUIRE(proto.location_code() == locationCode);
        REQUIRE_THAT(proto.latitude(),
                     Catch::Matchers::WithinAbs(latitude, 1.e-10));
        REQUIRE_THAT(proto.longitude(),
                     Catch::Matchers::WithinAbs(longitude, 1.e-10));
        REQUIRE_THAT(proto.elevation(),
                     Catch::Matchers::WithinAbs(elevation, 1.e-10));
        REQUIRE_THAT(proto.sampling_rate(),
                     Catch::Matchers::WithinAbs(samplingRate, 1.e-10));
        REQUIRE_THAT(proto.azimuth(),
                     Catch::Matchers::WithinAbs(azimuth, 1.e-10));
        REQUIRE_THAT(proto.dip(),
                     Catch::Matchers::WithinAbs(dip, 1.e-10));
        REQUIRE(proto.start_time().seconds() == startTime.count());
        REQUIRE(proto.end_time().seconds() == endTime.count());
        auto protoLastModified
            = proto.last_modified().seconds()*1000000000
            + proto.last_modified().nanos();
        REQUIRE(protoLastModified ==
                std::chrono::nanoseconds {lastModified}.count());

    }   

    SECTION("From Protobuf")
    {   
        UMetadata::V1::Channel proto;
        proto.set_network("uu");
        proto.set_station(station);
        proto.set_name(name);
        proto.set_location_code(" 01 ");
        proto.set_latitude(latitude);
        proto.set_longitude(longitude + 360);
        proto.set_elevation(elevation);
        proto.set_sampling_rate(samplingRate);
        proto.set_azimuth(azimuth);
        proto.set_dip(dip);
        auto startTimeProtobuf
            = google::protobuf::util::TimeUtil::SecondsToTimestamp(
                 startTime.count());
        auto endTimeProtobuf
            = google::protobuf::util::TimeUtil::SecondsToTimestamp(
                 endTime.count());

        *proto.mutable_start_time() = std::move(startTimeProtobuf);
        *proto.mutable_end_time() = std::move(endTimeProtobuf);
        auto lastModifiedProtobuf
            = google::protobuf::util::TimeUtil::MicrosecondsToTimestamp(
                 lastModified.count()*1000000);
        *proto.mutable_last_modified() = std::move(lastModifiedProtobuf);

        UMetadata::Channel cproto{proto};
        REQUIRE(cproto.getNetwork() == network);
        REQUIRE(cproto.getStation() == station);
        REQUIRE(cproto.getName() == name);
        REQUIRE(cproto.getLocationCode() == locationCode);
        REQUIRE_THAT(cproto.getLatitude(),
                     Catch::Matchers::WithinAbs(latitude, 1.e-10));
        REQUIRE_THAT(cproto.getLongitude(),
                     Catch::Matchers::WithinAbs(longitude, 1.e-10));
        REQUIRE_THAT(cproto.getElevation(),
                     Catch::Matchers::WithinAbs(elevation, 1.e-10));
        REQUIRE_THAT(cproto.getSamplingRate(),
                     Catch::Matchers::WithinAbs(samplingRate, 1.e-10));
        REQUIRE_THAT(cproto.getAzimuth(),
                     Catch::Matchers::WithinAbs(azimuth, 1.e-10));
        REQUIRE_THAT(cproto.getDip(),
                     Catch::Matchers::WithinAbs(dip, 1.e-10));
        REQUIRE(cproto.getStartAndEndTime().first == startTime);
        REQUIRE(cproto.getStartAndEndTime().second == endTime);
        REQUIRE(cproto.getLastModified() ==
                std::chrono::microseconds {lastModified});
    }



}

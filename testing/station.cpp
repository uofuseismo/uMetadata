#include <vector>
#include <string>
#include <chrono>
#include <limits>
#include <google/protobuf/util/time_util.h>
#include "uMetadata/station.hpp"
#include "proto/v1/station.pb.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("UMetadata::Station", "[station]")
{
    std::string network{"UU"};
    std::string name{"CTU"};
    std::string description{"Camp Tracy"};
    constexpr double latitude{40.6925};
    constexpr double longitude{-111.75034};
    constexpr double elevation{1731.0};
    const std::chrono::seconds startTime{1590624000};
    const std::chrono::seconds endTime{32503680000};
    const std::chrono::seconds lastModified{1727969872};
 
    UMetadata::Station station;
    REQUIRE_NOTHROW(station.setNetwork(network));
    REQUIRE_NOTHROW(station.setName(name));
    REQUIRE_NOTHROW(station.setDescription(description));    
    REQUIRE_NOTHROW(station.setLatitude(latitude));
    station.setLongitude(longitude);
    REQUIRE_NOTHROW(station.setLongitude(longitude));
    REQUIRE_NOTHROW(station.setElevation(elevation));
    REQUIRE_NOTHROW(station.setStartAndEndTime(std::pair {startTime, endTime}));
    REQUIRE_NOTHROW(station.setLastModified(lastModified));

    REQUIRE(station.getNetwork() == network);
    REQUIRE(station.getName() == name);
    REQUIRE(*station.getDescription() == description);
    REQUIRE_THAT(station.getLatitude(),
                 Catch::Matchers::WithinAbs(latitude, 1.e-10));
    REQUIRE_THAT(station.getLongitude(),
                 Catch::Matchers::WithinAbs(longitude, 1.e-10));
    REQUIRE_THAT(station.getElevation(),
                 Catch::Matchers::WithinAbs(elevation, 1.e-10));
    REQUIRE(station.getStartAndEndTime().first == startTime);
    REQUIRE(station.getStartAndEndTime().second == endTime);
    REQUIRE(station.getLastModified() ==
            std::chrono::microseconds {lastModified});

    SECTION("To Protobuf")
    {
        auto proto = station.toProtobuf();
        REQUIRE(proto.network() == network); 
        REQUIRE(proto.name() == name);
        REQUIRE(proto.description() == description);
        REQUIRE_THAT(proto.latitude(),
                     Catch::Matchers::WithinAbs(latitude, 1.e-10));
        REQUIRE_THAT(proto.longitude(),
                     Catch::Matchers::WithinAbs(longitude, 1.e-10));
        REQUIRE_THAT(proto.elevation(),
                     Catch::Matchers::WithinAbs(elevation, 1.e-10));
        REQUIRE(proto.start_time().seconds() == startTime.count());
        REQUIRE(proto.end_time().seconds() == endTime.count());
        auto protoLastModified
            = proto.last_modified().seconds()*1000000000
            + proto.last_modified().nanos();
        REQUIRE(protoLastModified
                == std::chrono::nanoseconds {lastModified}.count());

    }

    SECTION("From Protobuf")
    {
        UMetadata::V1::Station proto;
        proto.set_network(network);
        proto.set_name(name);
        proto.set_description(description);
        proto.set_latitude(latitude);
        proto.set_longitude(longitude + 360);
        proto.set_elevation(elevation);

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

        UMetadata::Station sproto{proto};
        REQUIRE(sproto.getNetwork() == network);
        REQUIRE(sproto.getName() == name);
        REQUIRE(*sproto.getDescription() == description);
        REQUIRE_THAT(sproto.getLatitude(),
                     Catch::Matchers::WithinAbs(latitude, 1.e-10));
        REQUIRE_THAT(sproto.getLongitude(),
                     Catch::Matchers::WithinAbs(longitude, 1.e-10));
        REQUIRE_THAT(sproto.getElevation(),
                     Catch::Matchers::WithinAbs(elevation, 1.e-10));
        REQUIRE(sproto.getStartAndEndTime().first == startTime);
        REQUIRE(sproto.getStartAndEndTime().second == endTime);
        REQUIRE(sproto.getLastModified() ==
                std::chrono::microseconds {lastModified});
    }
}


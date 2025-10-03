#include <iostream>
#include <filesystem>
#include <string>
#include "uMetadata/database.hpp"
#include "uMetadata/station.hpp"
#include "data/utah.hpp"
#include "data/ynp.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

bool operator==(const UMetadata::Station &lhs,
                const UMetadata::Station &rhs)
{
    if (lhs.getNetwork() != rhs.getNetwork()){return false;}
    if (lhs.getName() != rhs.getName()){return false;}
    if (std::abs(lhs.getLatitude()
              - rhs.getLatitude()) > 1.e-8){return false;}
    if (std::abs(lhs.getLongitude()
               - rhs.getLongitude()) > 1.e-8){return false;}
    if (std::abs(lhs.getElevation()
               - rhs.getElevation()) > 1.e-6){return false;}
    auto [start, end] = lhs.getStartAndEndTime();
    auto [startRef, endRef] = rhs.getStartAndEndTime();
    if (start != startRef){return false;}
    if (end != endRef){return false;}
    auto lastModified = *lhs.getLastModified();
    auto lastModifiedRef = *rhs.getLastModified();
    if (lastModified != lastModifiedRef){return false;}
    //std::cout << lastModified.count() << " " << lastModifiedRef.count() << std::endl;
    if (lhs.getDescription() && rhs.getDescription())
    {
        if (*lhs.getDescription() != *rhs.getDescription())
        {
            return false;
        }
    }
    else
    {
        if (lhs.getDescription() || rhs.getDescription()) 
        {
            return false;
        }
    }
    return true;
}

TEST_CASE("UMetadata::Database", "[sqlite3]")
{
    std::filesystem::path databaseFile{"utah.sqlite3"};
    if (std::filesystem::exists(databaseFile))
    {
        std::filesystem::remove(databaseFile);
    }

    UMetadata::Database database{databaseFile, false}; 
    auto activeStationsRef = ::createStationsUtah();
    database.insert(activeStationsRef);

    // Fail adding
    REQUIRE_NOTHROW(database.insert(activeStationsRef.at(0)));
    database.close();
    

    SECTION("Query")
    {
        constexpr bool readOnly{true};
        UMetadata::Database database{databaseFile, readOnly}; 

        auto firstStationRef = activeStationsRef[0];
        auto firstStation
            = database.getActiveStationInformation(firstStationRef.getNetwork(),
                                                   firstStationRef.getName());
        bool match = (firstStation && (*firstStation == firstStationRef));
        CHECK(match);

        auto activeStations = database.getAllActiveStations();
        REQUIRE(activeStations.size() == activeStationsRef.size());
        // Find them all
        for (const auto &station : activeStations)
        {
            bool match{false};
            for (const auto &refStation : activeStationsRef)
            {
                if (station == refStation)
                {
                    match = true;
                    break;
                }
                /*
                if (station.getNetwork() != refStation.getNetwork()){continue;}
                if (station.getName() != refStation.getName()){continue;}
                if (std::abs(station.getLatitude()
                           - refStation.getLatitude()) > 1.e-8){continue;}
                if (std::abs(station.getLongitude()
                           - refStation.getLongitude()) > 1.e-8){continue;}
                if (std::abs(station.getElevation()
                           - refStation.getElevation()) > 1.e-6){continue;}
                auto [start, end] = station.getStartAndEndTime();
                auto [startRef, endRef] = refStation.getStartAndEndTime();
                if (start != startRef){continue;}
                if (end != endRef){continue;}
                auto lastModified = *station.getLastModified();
                auto lastModifiedRef = *refStation.getLastModified();
                if (lastModified != lastModifiedRef){continue;}
                //std::cout << lastModified.count() << " " << lastModifiedRef.count() << std::endl;
                if (station.getDescription() && refStation.getDescription())
                {
                    if (*station.getDescription() !=
                        *refStation.getDescription())
                    {
                        continue;
                    }
                }
                else
                {
                    if (station.getDescription() || 
                        refStation.getDescription()) 
                    { 
                        continue;
                    }
                }
                match = true;
                break;
                */
            }
            CHECK(match);
        }
    }

}


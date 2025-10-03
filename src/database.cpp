#include <iostream>
#include <chrono>
#include <string>
#include <cmath>
#include <filesystem>
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include "uMetadata/database.hpp"
#include "uMetadata/station.hpp"
#include "utilities.hpp"
#define STATION_TABLE "station"
#define CHANNEL_TABLE "channel"
#define POLE_ZERO_TABLE "poles_and_zeros"

#define SQLITE_CHECK_BIND(returnCode, statement) \
{ \
    if (returnCode != SQLITE_OK) \
    { \
        sqlite3_finalize(statement); \
        throw std::runtime_error("Failed to bind statement with " \
                               + std::to_string(returnCode)); \
   } \
}

#define SQLITE_CHECK_PREPARE(returnCode, statement) \
{ \
    if (returnCode != SQLITE_OK) \
    { \
        sqlite3_finalize(statement); \
        throw std::runtime_error("Failed to prepare statement with " \
                               + std::to_string(returnCode)); \
   } \
}

#define SQLITE_CHECK_FINALIZE(returnCode) \
{ \
    if (returnCode != SQLITE_OK) \
    { \
        spdlog::warn("Failed to finalize create station table stmt"); \
    } \
}



namespace
{
/*
[[nodiscard]] std::chrono::microseconds getNow() 
{    
     auto now    
        = std::chrono::duration_cast<std::chrono::microseconds>
          ((std::chrono::high_resolution_clock::now()).time_since_epoch());
     return now;        
}                    
*/
[[nodiscard]] UMetadata::Station unpackStationRow(sqlite3_stmt *statement)
{
     UMetadata::Station result;
     std::string network{
         reinterpret_cast<const char *> (sqlite3_column_text(statement, 0))};
     result.setNetwork(network);

     std::string name{
         reinterpret_cast<const char *> (sqlite3_column_text(statement, 1))};
     result.setName(name);

     std::string description;
     auto descriptionResult = sqlite3_column_text(statement, 2);
     if (descriptionResult)
     {
         std::string description{
             reinterpret_cast<const char *> (descriptionResult)};
         result.setDescription(description);
     }

     double latitude = sqlite3_column_double(statement, 3);
     result.setLatitude(latitude);

     double longitude = sqlite3_column_double(statement, 4);
     result.setLongitude(longitude);

     double elevation = sqlite3_column_double(statement, 5); 
     result.setElevation(elevation);
 
     int64_t startTime = sqlite3_column_int64(statement, 6);
     int64_t endTime = sqlite3_column_int64(statement, 7);
     result.setStartAndEndTime(
        std::pair { std::chrono::seconds {startTime}, 
                    std::chrono::seconds {endTime}    } );

     auto lastModified
         = static_cast<int64_t> (
              std::floor(sqlite3_column_double(statement, 8)*1.e6));
     result.setLastModified(
         std::chrono::microseconds {lastModified} );

     return result;
}
}


using namespace UMetadata;

class Database::DatabaseImpl
{
public:
    //DatabaseImpl() = default;
    DatabaseImpl(const std::filesystem::path &fileName, const bool readOnly)
    {
        if (readOnly)
        {
            openReadOnly(fileName);
            if (!tableExists(STATION_TABLE))
            {
                close();
                throw std::runtime_error("Station table does not exist");
            }
        }
        else
        {
            if (!std::filesystem::exists(fileName))
            {
                openCreateReadWrite(fileName);
                createStationTable();
            }
            else
            {
                openReadWrite(fileName);
            }
        }
    }
    void openReadOnly(const std::filesystem::path &fileName)
    {
        if (!std::filesystem::exists(fileName))
        {
            throw std::invalid_argument(fileName.string() + " does not exist");
        }
        //mURI = "file:/" + fileName.string();
        mURI = fileName.string();
        spdlog::debug("Opening sqlite3 " + mURI + " in read-only mode");
        auto returnCode = sqlite3_open_v2(mURI.c_str(),
                                          &mDatabaseHandle,
                                          SQLITE_OPEN_READONLY |
                                          SQLITE_OPEN_FULLMUTEX,
                                          nullptr);
        if (returnCode != SQLITE_OK)
        {
            sqlite3_close(mDatabaseHandle);
            throw std::runtime_error("Failed to open sqlite3 database "
                                   + fileName.string());
        }
        mHaveReadOnlyDatabase = true;
        mHaveReadWriteDatabase = false;
    }
    void openReadWrite(const std::filesystem::path &fileName)
    {
        //mURI = "file:/" + fileName.string();
        mURI = fileName.string();
        spdlog::debug("Opening read-write database");
        auto returnCode = sqlite3_open_v2(mURI.c_str(),
                                          &mDatabaseHandle,
                                          SQLITE_OPEN_READWRITE |
                                          SQLITE_OPEN_FULLMUTEX,
                                          nullptr);
        if (returnCode != SQLITE_OK)
        {
            sqlite3_close(mDatabaseHandle);
            throw std::runtime_error(
                "Failed to open sqlite3 read-write database "
              + fileName.string());
        }
        mHaveReadOnlyDatabase = false;
        mHaveReadWriteDatabase = true;
    }
    void openCreateReadWrite(const std::filesystem::path &fileName)
    {
        auto parentPath = fileName.parent_path();
        if (!parentPath.empty())
        {
            if (!std::filesystem::exists(parentPath))
            {
                spdlog::info(parentPath.string()
                           + " does not exist; trying to make path");
                if (!std::filesystem::create_directories(parentPath))
                {
                    throw std::runtime_error("Could not create path "
                                           + parentPath.string());
                }
            }
        }
        mURI = fileName.string();
        spdlog::debug("Creating read-write database");
        auto returnCode = sqlite3_open_v2(mURI.c_str(),
                                          &mDatabaseHandle,
                                          SQLITE_OPEN_READWRITE |
                                          SQLITE_OPEN_CREATE |
                                          SQLITE_OPEN_FULLMUTEX,
                                          nullptr);
        if (returnCode != SQLITE_OK)
        {
            sqlite3_close(mDatabaseHandle);
            throw std::runtime_error(
                "Failed to create sqlite3 read-write database "
              + fileName.string());
        }
        mHaveReadOnlyDatabase = false;
        mHaveReadWriteDatabase = true;
    }
    [[nodiscard]] bool exists(const Station &station) const
    {
        bool result{false};
        if (!mDatabaseHandle)
        {
            throw std::runtime_error("database not initialized");
        }
        // These statements throw
        auto network = station.getNetwork();
        auto name = station.getName();
        auto startAndEndTime = station.getStartAndEndTime();
        auto startTime = static_cast<sqlite3_int64> (startAndEndTime.first.count());
        auto endTime = static_cast<sqlite3_int64> (startAndEndTime.second.count());

        const std::string_view sql{
R"""(
SELECT COUNT(*) FROM station WHERE
  network = ?1 AND
  name = ?2 AND
  ((start_time >= ?3 AND ?4 <= end_time) OR (start_time >= ?5 AND ?6 <= end_time))
)"""};
        sqlite3_stmt *statement{nullptr};
        auto returnCode = sqlite3_prepare_v2(mDatabaseHandle,
                                             sql.data(),
                                             -1,
                                             &statement,
                                             nullptr);
        SQLITE_CHECK_PREPARE(returnCode, statement);

        returnCode = sqlite3_bind_text(statement,
                                       1,
                                       network.data(),
                                       network.size(),
                                       SQLITE_STATIC);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_text(statement,
                                       2,
                                       name.data(),
                                       name.size(),
                                       SQLITE_STATIC);
        returnCode = sqlite3_bind_int64(statement, 3, startTime);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_int64(statement, 4, startTime);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_int64(statement, 5, endTime);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_int64(statement, 6, endTime);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_step(statement);
        if (returnCode == SQLITE_ROW)
        {
            auto exists = sqlite3_column_int(statement, 0);
            if (exists >= 1){result = true;}
            result = exists == 0 ? false : true;
        }
        returnCode = sqlite3_finalize(statement);
        SQLITE_CHECK_FINALIZE(returnCode);
        return result;
    }
    [[nodiscard]] bool tableExists(const std::string_view &table) const
    {
        bool result{false};
        if (!mDatabaseHandle)
        {
            throw std::runtime_error("database not initialized");
        }
        sqlite3_stmt *statement{nullptr};
        const std::string_view sql{
            "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = ?1"};
        auto returnCode = sqlite3_prepare_v2(mDatabaseHandle, 
                                             sql.data(),
                                             -1,
                                             &statement,
                                             nullptr);
        SQLITE_CHECK_PREPARE(returnCode, statement);
        constexpr int index{1};
        returnCode = sqlite3_bind_text(statement,
                                       index,
                                       table.data(),
                                       table.size(), 
                                       SQLITE_STATIC);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_step(statement);
        if (returnCode == SQLITE_ROW)
        {
            auto exists = sqlite3_column_int(statement, 0);
            result = exists == 0 ? false : true;
        }
        returnCode = sqlite3_finalize(statement);
        if (returnCode != SQLITE_OK)
        {
            spdlog::warn("Failed to finalize table exists statement");
        }
        return result;
    } 
    void createTable(const std::string_view &schema)
    {
        bool result{false};
        if (!mHaveReadWriteDatabase)
        {            
            throw std::runtime_error("database not initialized");
        }
        sqlite3_stmt *statement{nullptr};
        auto returnCode = sqlite3_prepare_v2(mDatabaseHandle,
                                             schema.data(),
                                             -1,
                                             &statement,
                                             nullptr);
        SQLITE_CHECK_PREPARE(returnCode, statement);
        returnCode = sqlite3_step(statement);
        if (returnCode == SQLITE_DONE)
        {
            spdlog::debug("Succesfully created table");
        }
        else
        {
            sqlite3_finalize(statement);
            throw std::runtime_error("Failed to create table");
        }
        returnCode = sqlite3_finalize(statement);
        if (returnCode != SQLITE_OK)
        {
            spdlog::warn("Failed to finalize create station table stmt");
        }
    }
    void createStationTable()
    {
        const std::string_view stationTable{STATION_TABLE};
        try
        {
            if (tableExists(stationTable))
            {
                spdlog::info(std::string {stationTable}
                            + " already exists; will not create");
                return;
            }
            const std::string_view schema{
R"""(
CREATE TABLE station (
  network TEXT,
  name TEXT,
  description TEXT DEFAULT NULL,
  latitude DOUBLE NOT NULL CHECK( latitude >= -90 AND latitude <= 90),
  longitude DOUBLE NOT NULL,
  elevation DOUBLE NOT NULL CHECK( elevation >= -10000 AND elevation <= 8600 ),
  start_time BIGINT NOT NULL,
  end_time BIGINT DEFAULT 32503680000 CHECK (end_time > start_time),
  last_modified DOUBLE DEFAULT CURRENT_TIMESTAMP,
  UNIQUE(network, name, start_time)
)
)"""};
             createTable(schema);
             spdlog::info("Successfully created station table");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to create station table because " 
                        + std::string {e.what()});
            return;
        }
    }
    std::optional<UMetadata::Station>
        getActiveStationInformation(const std::string &network,
                                    const std::string &name) const
    {
        if (!mDatabaseHandle)
        {                  
            throw std::runtime_error("database not initialized");
        }
        const std::string_view sql{
R"""(
SELECT network, name, description, latitude, longitude, elevation, start_time, end_time, last_modified FROM station WHERE
  network = ?1 AND name = ?2 AND
  unixepoch(CURRENT_TIMESTAMP) >= start_time AND unixepoch(CURRENT_TIMESTAMP) <= end_time LIMIT 1
)"""};
        sqlite3_stmt *statement{nullptr};
        auto returnCode = sqlite3_prepare_v2(mDatabaseHandle,
                                             sql.data(),
                                             -1, 
                                             &statement,
                                             nullptr);
        SQLITE_CHECK_PREPARE(returnCode, statement);
        returnCode = sqlite3_bind_text(statement,
                                       1,
                                       network.data(),
                                       network.size(),
                                       SQLITE_STATIC);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_text(statement,
                                       2,
                                       name.data(),
                                       name.size(),
                                       SQLITE_STATIC);
        SQLITE_CHECK_BIND(returnCode, statement);
        UMetadata::Station station;
        bool found{false};
        returnCode = sqlite3_step(statement);
        if (returnCode == SQLITE_ROW)
        {
            try
            {
                station = ::unpackStationRow(statement);
                found = true;
            }
            catch (const std::exception &e)
            {
                spdlog::warn("Failed to unpack row for "
                           + network + "." + name);
            }
        }
        returnCode = sqlite3_finalize(statement);
        SQLITE_CHECK_FINALIZE(returnCode);
        return found ? std::optional<UMetadata::Station> {std::move(station)}
                     : std::nullopt;
    }
    std::vector<UMetadata::Station> getAllActiveStations() const
    {
        if (!mDatabaseHandle)
        {              
            throw std::runtime_error("database not initialized");
        }
        const std::string_view sql{
R"""(
SELECT network, name, description, latitude, longitude, elevation, start_time, end_time, last_modified FROM station WHERE
  unixepoch(CURRENT_TIMESTAMP) >= start_time AND unixepoch(CURRENT_TIMESTAMP) <= end_time
)"""};
        sqlite3_stmt *statement{nullptr};
        auto returnCode = sqlite3_prepare_v2(mDatabaseHandle,
                                             sql.data(),
                                             -1,
                                             &statement,
                                             nullptr);
        SQLITE_CHECK_PREPARE(returnCode, statement);

        std::vector<UMetadata::Station> result;
        result.reserve(1024);
        returnCode = SQLITE_ROW;
        while (returnCode == SQLITE_ROW)
        {
            returnCode = sqlite3_step(statement);
            if (returnCode == SQLITE_ROW)
            {
                try
                {
                    result.push_back(std::move(::unpackStationRow(statement)));
                }
                catch (const std::exception &e) 
                {
                    spdlog::warn("Failed to unpack row");
                }
            }
            else
            {
                if (returnCode != SQLITE_DONE)
                {
                    spdlog::warn(
                       "Current station query did not finish with SQLITE_DONE");
                }
            }
        }
        returnCode = sqlite3_finalize(statement);
        SQLITE_CHECK_FINALIZE(returnCode);
        return result;
    }
    void insertStation(const UMetadata::Station &station)
    {
        if (!mDatabaseHandle)
        {          
            throw std::runtime_error("database not initialized");
        }   
        if (!mHaveReadWriteDatabase)
        {
            throw std::runtime_error("database not must be read-write");
        }
        if (exists(station))
        {
            spdlog::warn(station.getNetwork() + "." + station.getName()
                      +  " already exists; skipping");
            return;
        }
        // These statements throw
        auto network = station.getNetwork();
        auto name = station.getName();
        auto description = station.getDescription();
        double latitude = station.getLatitude();
        double longitude = station.getLongitude();
        double elevation = station.getElevation();
        auto startAndEndTime = station.getStartAndEndTime();
        auto startTime = static_cast<sqlite3_int64> (startAndEndTime.first.count());
        auto endTime = static_cast<sqlite3_int64> (startAndEndTime.second.count());
        double lastModified = ::getNow().count()*1.e-6;
        if (station.getLastModified())
        {
            lastModified = station.getLastModified()->count()*1.e-6;
        }

        const std::string_view sql{
R"""(
INSERT INTO station (network, name, latitude, longitude, elevation, start_time, end_time, last_modified, description)
  VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)
)"""
};
        sqlite3_stmt *statement{nullptr};
        auto returnCode = sqlite3_prepare_v2(mDatabaseHandle,
                                             sql.data(),
                                             -1,
                                             &statement,
                                             nullptr);
        SQLITE_CHECK_PREPARE(returnCode, statement);
        returnCode = sqlite3_bind_text(statement,
                                       1,
                                       network.data(),
                                       network.size(),
                                       SQLITE_STATIC);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_text(statement,
                                       2,
                                       name.data(),
                                       name.size(),
                                       SQLITE_STATIC);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_double(statement, 3, latitude);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_double(statement, 4, longitude);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_double(statement, 5, elevation);
        SQLITE_CHECK_BIND(returnCode, statement); 
        returnCode = sqlite3_bind_int64(statement, 6, startTime);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_int64(statement, 7, endTime);
        SQLITE_CHECK_BIND(returnCode, statement);
        returnCode = sqlite3_bind_double(statement, 8, lastModified);
        SQLITE_CHECK_BIND(returnCode, statement);
        if (description)
        {
            returnCode = sqlite3_bind_text(statement,
                                           9,
                                           description->data(),
                                           description->size(),
                                           SQLITE_STATIC);
            SQLITE_CHECK_BIND(returnCode, statement);
        }
        else
        {
            returnCode = sqlite3_bind_null(statement, 9); 
            SQLITE_CHECK_BIND(returnCode, statement);
        }
        // Insert it
        returnCode = sqlite3_step(statement);
        if (returnCode == SQLITE_DONE)
        {
            spdlog::debug("Succesfully inserted " + network + "." + name
                        + " into station table");
        }
        else
        {
            spdlog::warn("Failed to insert " + network + "." + name 
                       + std::to_string(returnCode));
        }
        returnCode = sqlite3_finalize(statement);
        if (returnCode != SQLITE_OK)
        {
            spdlog::warn("Failed to finalize insert station table stmt");
        }
    }
    void close()
    {
        auto returnCode = SQLITE_OK;
        if (mHaveReadOnlyDatabase)
        {
            spdlog::debug("Closing read-only database " + mURI);
            returnCode = sqlite3_close(mDatabaseHandle);
            if (returnCode != SQLITE_OK)
            {
                spdlog::warn("Failed to close read-only sqlite3 database "
                           + mURI);
            }
            mHaveReadOnlyDatabase = false;
        }
        if (mHaveReadWriteDatabase)
        {
            spdlog::debug("Closing read-writedatabase " + mURI);
            returnCode = sqlite3_close(mDatabaseHandle);
            if (returnCode != SQLITE_OK)
            {
                spdlog::warn("Failed to close read-write sqlite3 database " 
                           + mURI);
            }
            mHaveReadWriteDatabase = false;
        }
    }
    ~DatabaseImpl()
    {
        close();
    }
//private:
    mutable sqlite3 *mDatabaseHandle{nullptr};
    std::string mURI;
    bool mHaveReadOnlyDatabase{false};
    bool mHaveReadWriteDatabase{false};
};

/// Constructor
/*
Database::Database() :
    pImpl(std::make_unique<DatabaseImpl> ())
{
}
*/

/// Constructor
Database::Database(const std::filesystem::path &fileName,
                   const bool readOnly) :
    pImpl(std::make_unique<DatabaseImpl> (fileName, readOnly))
{
}

void Database::insert(const std::vector<Station> &stations)
{
    for (const auto &station : stations)
    {
        insert(station);
    }
}

void Database::insert(const Station &station)
{
    pImpl->insertStation(station);
}


std::vector<UMetadata::Station> Database::getAllActiveStations() const
{
    return pImpl->getAllActiveStations();
}

std::optional<UMetadata::Station> Database::getActiveStationInformation(
    const std::string &networkIn, const std::string &nameIn) const
{
    auto network = ::transformString(networkIn);
    auto name = ::transformString(nameIn);
    if (network.empty()){throw std::invalid_argument("Network is empty");}
    if (name.empty()){throw std::invalid_argument("Name is empty");}
    return pImpl->getActiveStationInformation(network, name);
}

/// Destructor
Database::~Database() = default;

/// Close the database
void Database::close()
{
    pImpl->close();
}

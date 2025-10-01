#include <string>
#include <filesystem>
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include "uMetadata/database.hpp"

#define STATION_TABLE "station"
#define CHANNEL_TABLE "channel"
#define POLE_ZERO_TABLE "poles_and_zeros"

using namespace UMetadata;

class Database::DatabaseImpl
{
public:
    void openReadOnly(const std::filesystem::path &fileName)
    {
        if (!std::filesystem::exists(fileName))
        {
            throw std::invalid_argument(fileName.string() + " does not exist");
        }
        mURI = "file:/" + fileName.string();
        spdlog::debug("Opening sqlite3 " + mURI + " in read-only mode");
        auto returnCode = sqlite3_open_v2(mURI.c_str(),
                                          &mDatabaseHandle,
                                          SQLITE_OPEN_READONLY |
                                          SQLITE_OPEN_FULLMUTEX,
                                          nullptr);
        if (returnCode != SQLITE_OK)
        {
            throw std::runtime_error("Failed to open sqlite3 database "
                                   + fileName.string());
        }
        mHaveReadOnlyDatabase = true;
    }
    [[nodiscard]] bool tableExists(const std::string_view &table) const
    {
        bool result{false};
        if (mDatabaseHandle)
        {
            throw std::runtime_error("Database not initialized");
        }
        sqlite3_stmt *statement{nullptr};
        const std::string_view sql{
            "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = 1"};
        auto returnCode = sqlite3_prepare(mDatabaseHandle, 
                                          sql.data(),
                                          sql.size(),
                                          &statement,
                                          nullptr);
        if (returnCode != SQLITE_OK)
        {
            sqlite3_finalize(statement);
            throw std::runtime_error("Failed to prepare table check name");
        }
        constexpr int index{1};
        returnCode = sqlite3_bind_text(statement,
                                       index,
                                       table.data(),
                                       table.size(), 
                                       SQLITE_STATIC);
        if (returnCode != SQLITE_OK)
        {
            sqlite3_finalize(statement);
            throw std::runtime_error("Failed to bind statement");
        }
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
    void createStationTable()
    {
        const std::string_view stationTable{STATION_TABLE};
        try
        {
            if (tableExists(stationTable))
            {
                spdlog::debug(std::string {stationTable}
                            + " already exists; will not create");
                return;
            }
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to create station table because " 
                        + std::string {e.what()});
            return;
        }
        // Create it
 
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
Database::Database() :
    pImpl(std::make_unique<DatabaseImpl> ())
{
}

/// Destructor
Database::~Database() = default;

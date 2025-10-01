#ifndef UMETADATA_DATABASE_HPP
#define UMETADATA_DATABASE_HPP
#include <filesystem>
#include <memory>
#include <vector>
namespace UMetadata
{

class Station;

class Database
{
public:
    Database();
    Database(const std::filesystem::path &fileName,
             bool openReadOnly);

    [[nodiscard]] std::vector<Station> getAllActiveStations() const;

    ~Database();
private:
    class DatabaseImpl;
    std::unique_ptr<DatabaseImpl> pImpl;
};

}
#endif

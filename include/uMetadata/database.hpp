#ifndef UMETADATA_DATABASE_HPP
#define UMETADATA_DATABASE_HPP
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>
namespace UMetadata
{

class Station;

class Database
{
public:
    Database() = delete;
    Database(const std::filesystem::path &fileName,
             bool openReadOnly);

    [[nodiscard]] std::vector<Station> getAllActiveStations() const;
    [[nodiscard]] std::optional<Station> getActiveStationInformation(const std::string &network, const std::string &name) const;
    void insert(const std::vector<Station> &stations);
    void insert(const Station &station);

    void close();

    ~Database();

    Database& operator=(const Database &) = delete;
    Database& operator=(Database &&) noexcept = delete;
private:
    class DatabaseImpl;
    std::unique_ptr<DatabaseImpl> pImpl;
};

}
#endif

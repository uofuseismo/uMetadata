#include <iostream>
#include <string>
#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "uMetadata/database.hpp"
#include "data/utah.hpp"
#include "data/ynp.hpp"

#define APPLICATION_NAME "uMetadataUpdater"

namespace
{
struct ProgramOptions
{
    std::string applicationName{APPLICATION_NAME};
    std::filesystem::path sqlite3Database{"utah.db"};
    int verbosity{3};
bool isUtah{true};
};

std::pair<std::string, bool> parseCommandLineOptions(int argc, char *argv[]);
::ProgramOptions parseIniFile(const std::filesystem::path &iniFile);

}

int main(int argc, char *argv[])
{
    // Get the ini file from the command line
    std::filesystem::path iniFile;
    try
    {   
        auto [iniFileName, isHelp] = ::parseCommandLineOptions(argc, argv);
        if (isHelp){return EXIT_SUCCESS;}
        iniFile = iniFileName;
    }   
    catch (const std::exception &e) 
    {   
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }

    // Read the program properties
    ::ProgramOptions programOptions;
    try
    {
        programOptions = ::parseIniFile(iniFile);
    }
    catch (const std::exception &e)
    {
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }
    if (programOptions.verbosity <= 1){spdlog::set_level(spdlog::level::critical);}
    if (programOptions.verbosity == 2){spdlog::set_level(spdlog::level::warn);}
    if (programOptions.verbosity == 3){spdlog::set_level(spdlog::level::info);}
    if (programOptions.verbosity >= 4){spdlog::set_level(spdlog::level::debug);}

    try
    {
        constexpr bool readOnly{false};
        UMetadata::Database database{programOptions.sqlite3Database,
                                     readOnly};

        if (programOptions.isUtah)
        {
            auto stations = ::createStationsUtah();
            database.insert(stations);
        }
        else
        {
            auto stations = ::createStationsYNP();
            database.insert(stations);
        }
        database.close();
    }
    catch (const std::exception &e)
    {
        spdlog::critical(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

///--------------------------------------------------------------------------///
///                            Utility Functions                             ///
///--------------------------------------------------------------------------///
namespace
{

/// Read the program options from the command line
std::pair<std::string, bool> parseCommandLineOptions(int argc, char *argv[])
{
    std::string iniFile;
    boost::program_options::options_description desc(R"""(
The uMetadataUpdater is a utility that updates the UUSS SQLite3 metadata database.

    uMetadataUpdater --ini=uMetadataUpdater.ini

Allowed options)""");
    desc.add_options()
        ("help", "Produces this help message")
        ("ini",  boost::program_options::value<std::string> (),
                 "The initialization file for this executable");
    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return {iniFile, true};
    }
    if (vm.count("ini"))
    {
        iniFile = vm["ini"].as<std::string>();
        if (!std::filesystem::exists(iniFile))
        {
            throw std::runtime_error("Initialization file: " + iniFile
                                   + " does not exist");
        }
    }
    return {iniFile, false};
}

::ProgramOptions parseIniFile(const std::filesystem::path &iniFile)
{   
    ::ProgramOptions options;
    if (!std::filesystem::exists(iniFile)){return options;}
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);

    // Application name
    options.applicationName
        = propertyTree.get<std::string> ("General.applicationName",
                                         options.applicationName);
    if (options.applicationName.empty())
    {   
        options.applicationName = APPLICATION_NAME;
    }   
    options.verbosity
        = propertyTree.get<int> ("General.verbosity", options.verbosity);

options.isUtah = propertyTree.get<bool> ("General.isUtah", options.isUtah);

    options.sqlite3Database
        = propertyTree.get<std::string> ("SQLite3.databaseFile",
                                         options.sqlite3Database.string());
    auto parentPath = options.sqlite3Database.parent_path();
    if (!parentPath.empty())
    {
        if (!std::filesystem::exists(parentPath))
        {
            spdlog::info(parentPath.string()
                       + " does not exist; trying to make path");
            if (!std::filesystem::create_directories(parentPath))
            {
                throw std::invalid_argument("Could not create path "
                                          + parentPath.string());
            }
        }
    }

    return options;
}

}

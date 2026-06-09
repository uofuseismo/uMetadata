#include <string>
#include "uMetadata/version.hpp"

using namespace UMetadata;

int Version::getMajor() noexcept
{
    return UMetadata_MAJOR;
}

int Version::getMinor() noexcept
{
    return UMetadata_MINOR;
}

int Version::getPatch() noexcept
{
    return UMetadata_PATCH;
}

//NOLINTBEGIN(bugprone-easily-swappable-parameters)
bool Version::isAtLeast(const int major, const int minor,
                        const int patch) noexcept
//NOLINTEND(bugprone-easily-swappable-parameters)
{
    if (UMetadata_MAJOR < major){return false;}
    if (UMetadata_MAJOR > major){return true;}
    if (UMetadata_MINOR < minor){return false;}
    if (UMetadata_MINOR > minor){return true;}
    if (UMetadata_PATCH < patch){return false;}
    return true;
}

std::string Version::getVersion() noexcept
{
    std::string version{UMetadata_VERSION};
    return version;
}

std::string Version::getTag() noexcept
{
    std::string tag{UMetadata_GITTAG};
    return tag;
}

std::string Version::getVersionWithTag() noexcept
{
    auto tag = Version::getTag();
    if (tag.empty())
    {
        return Version::getVersion();
    }
    else
    {
        return Version::getVersion() + "-" + tag;
    }
}


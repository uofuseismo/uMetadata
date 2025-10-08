#ifndef UTILITIES_HPP
#define UTILITIES_HPP
#include <chrono>
#include <string>
#include <limits>
#ifndef NDEBUG
#include <cassert>
#endif
namespace
{

[[nodiscard]] std::chrono::microseconds getNow() 
{    
     auto now    
        = std::chrono::duration_cast<std::chrono::microseconds>
          ((std::chrono::high_resolution_clock::now()).time_since_epoch());
     return now;    
}    

[[nodiscard]] std::string transformString(const std::string_view &input)
{
    std::string result{input.data(), input.size()};
    result.erase(std::remove_if(result.begin(), result.end(), isspace),
                 result.end());
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

[[maybe_unused]]
[[nodiscard]] std::chrono::seconds getYear3000() noexcept
{
    return std::chrono::seconds {32503680000};

}

[[maybe_unused]]
[[nodiscard]] double lonTo180(const double lonIn)
{
    auto lon = lonIn;
    if (lon < -180)
    {   
        for (int k = 0; k < std::numeric_limits<int>::max(); ++k)
        {
            auto tempLon = lon + k*360;
            if (tempLon >= -180)
            {
                lon = tempLon;
                break;
            }
        }
    }   
    if (lon >= 180)
    {   
        for (int k = 0; k < std::numeric_limits<int>::max(); ++k)
        {
            auto tempLon = lon - k*360;
            if (tempLon < 180)
            {
                lon = tempLon;
                break;
            }
        }
    }   
#ifndef NDEBUG
    assert(lon >= -180 && lon < 180);
#endif
    return lon;
}


}
#endif

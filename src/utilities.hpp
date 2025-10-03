#ifndef UTILITIES_HPP
#define UTILITIES_HPP
#include <chrono>
#include <string>
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

}
#endif

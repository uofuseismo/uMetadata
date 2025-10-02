#ifndef YNP_DEFAULT
#define YNP_DEFAULT
#include <uMetadata/station.hpp>
namespace {
[[nodiscard]] UMetadata::Station toStation(const std::string &network,
     const std::string &name, const std::string &description,
     const double latitude, const double longitude, const double elevation,
     const int64_t startTime, const int64_t endTime, const int64_t lastModified)
{
    UMetadata::Station result;
    result.setNetwork(network);
    result.setName(name);
    result.setDescription(description);
    result.setLatitude(latitude);
    result.setLongitude(longitude);
    result.setElevation(elevation);
    result.setStartAndEndTime( std::pair {std::chrono::seconds {startTime}, std::chrono::seconds {endTime}} );
    result.setLastModified( std::chrono::seconds {lastModified} );
    return result;
}
[[nodiscard]] std::vector<UMetadata::Station> createStations()
{
    std::vector<UMetadata::Station> stations;
    stations.push_back(::toStation("IW", "FLWY", "Flagg Ranch, Wyoming, USA", 44.083002, -110.699888, 2078.0, 1124236800, 32503680000, 1727969750));
    stations.push_back(::toStation("IW", "IMW", "Indian Meadow, Wyoming, USA", 43.89303, -110.94343, 2552.0, 1093046400, 32503680000, 1727969754));
    stations.push_back(::toStation("IW", "LOHW", "Long Hollow, Wyoming USA", 43.6124, -110.6038, 2121.0, 1092096000, 32503680000, 1727969757));
    stations.push_back(::toStation("IW", "MOOW", "Moose Ponds, Wyoming, USA", 43.7486, -110.7448, 2128.0, 1088640000, 32503680000, 1727969759));
    stations.push_back(::toStation("IW", "PLID", "Pearl Lake, Idaho, USA", 45.08757, -116.00025, 2164.4, 1476293400, 4070908800, 1678134434));
    stations.push_back(::toStation("IW", "SNOW", "Snow King Mountain, Wyoming, USA", 43.4599, -110.75408, 2413.0, 1088640000, 32503680000, 1753805144));
    stations.push_back(::toStation("IW", "TPAW", "Teton Pass, Wyoming, USA", 43.48965, -110.95675, 2680.0, 1088640000, 32503680000, 1727969769));
    stations.push_back(::toStation("MB", "BGMT", "Barton Gulch, Montana", 45.2333, -112.0405, 2172.0, 997833600, 4070908800, 1678134434));
    stations.push_back(::toStation("MB", "BZMT", "Bozeman Pass, Montana", 45.64817, -110.7967, 1905.0, 997833600, 4070908800, 1678134434));
    stations.push_back(::toStation("MB", "CRMT", "Chrome Mountain, Montana", 45.45583, -110.1402, 2941.0, 997833600, 4070908800, 1678134434));
    stations.push_back(::toStation("MB", "MOMT", "Monida, Montana", 44.59333, -112.3943, 2220.0, 997833600, 4070908800, 1678134434));
    stations.push_back(::toStation("MB", "QLMT", "Earthquake Lake, Montana", 44.83067, -111.43, 2064.0, 992044800, 4070908800, 1678134434));
    stations.push_back(::toStation("MB", "TPMT", "Tepee Creek, Montana", 44.72983, -111.6657, 2518.0, 992044800, 4070908800, 1678134434));
    stations.push_back(::toStation("PB", "B206", "canyon206bwy2008, Yellowstone, WY, USA", 44.7177, -110.5117, 2399.5, 1214611200, 4070908800, 1678134434));
    stations.push_back(::toStation("PB", "B207", "madisn207bwy2007, Yellowstone, Wy, USA", 44.619, -110.8485, 2182.0, 1192838400, 4070908800, 1678134434));
    stations.push_back(::toStation("PB", "B208", "lakejn208bwy2007, Yellowstone, Wy, USA", 44.5602, -110.4015, 2405.8, 1193875200, 4070908800, 1678134434));
    stations.push_back(::toStation("PB", "B944", "grantt944bwy2008, Yellowstone, WY, USA", 44.3897, -110.5438, 2364.6, 1218067200, 4070908800, 1678134434));
    stations.push_back(::toStation("PB", "B945", "panthr945swy2008, Yellowstone, WY, USA", 44.894, -110.7441, 2241.4, 1220486400, 4070908800, 1678134434));
    stations.push_back(::toStation("PB", "B950", "norris950bwy2013, Yellowstone, WY, USA", 44.7128, -110.6785, 2328.4, 1380672000, 4070908800, 1678134434));
    stations.push_back(::toStation("US", "BOZ", "Bozeman, Montana, USA", 45.59697, -111.62967, 1589.0, 942278400, 32503680000, 1727969775));
    stations.push_back(::toStation("US", "HLID", "Hailey, Idaho, USA", 43.5625, -114.4138, 1772.0, 902707200, 32503680000, 1727969787));
    stations.push_back(::toStation("US", "LKWY", "Lake (Yellowstone--Lake), Yellowstone National Park, Wyoming", 44.5652, -110.4, 2424.0, 813369600, 32503680000, 1753456640));
    stations.push_back(::toStation("US", "RLMT", "Red Lodge, Montana, USA", 45.122101, -109.267303, 2086.0, 1241548800, 4070908800, 1678134434));
    stations.push_back(::toStation("WY", "MCID", "Moose Creek, ID", 44.19033, -111.18266, 2134.49, 820022400, 32503680000, 1727970226));
    stations.push_back(::toStation("WY", "YBB", "Biscuit Basin, Yellowstone, WY", 44.485811, -110.855123, 2221.33, 1750118400, 32503680000, 1753990652));
    stations.push_back(::toStation("WY", "YDC", "Denny Creek, YNP, MT, USA", 44.7095, -111.23967, 2025.95, 440726400, 32503680000, 1727970228));
    stations.push_back(::toStation("WY", "YDD", "Grant Village, YNP, WY, USA", 44.3951, -110.5762, 2390.54, 1536105600, 32503680000, 1727970230));
    stations.push_back(::toStation("WY", "YEE", "East Entrance, YNP, WY, USA", 44.4853, -109.8969, 2281.23, 1473206400, 32503680000, 1727970232));
    stations.push_back(::toStation("WY", "YFT", "Old Faithful, YNP, WY, USA", 44.45133, -110.83583, 2298.53, 751593600, 32503680000, 1727970234));
    stations.push_back(::toStation("WY", "YGC", "Grayling Creek, YNP, MT, USA", 44.79617, -111.1065, 2086.12, 440726400, 32503680000, 1727970236));
    stations.push_back(::toStation("WY", "YHB", "Horse Butte, YNP, MT, USA", 44.7508, -111.1962, 2148.59, 814060800, 32503680000, 1727970238));
    stations.push_back(::toStation("WY", "YHH", "Holmes Hill, YNP, WY, USA", 44.78833, -110.8505, 2683.37, 751593600, 32503680000, 1727970240));
    stations.push_back(::toStation("WY", "YHL", "Hebgen Lake, YNP, WY, USA", 44.8509, -111.183, 2695.81, 1314921600, 32503680000, 1754414039));
    stations.push_back(::toStation("WY", "YHR", "Hawk's Rest, WY, USA", 44.106, -110.0816, 2978.77, 1409184000, 32503680000, 1727970245));
    stations.push_back(::toStation("WY", "YJC", "Joseph's Coat, YNP, WY, USA", 44.7555, -110.34917, 2688.58, 751593600, 32503680000, 1727970246));
    stations.push_back(::toStation("WY", "YLA", "Lake Butte, YNP, WY, USA", 44.51266, -110.26867, 2582.21, 625968000, 32503680000, 1753904140));
    stations.push_back(::toStation("WY", "YLT", "Little Thumb Creek, YNP, WY, USA", 44.437, -110.588, 2388.93, 935971200, 32503680000, 1756830203));
    stations.push_back(::toStation("WY", "YMC", "Maple Creek, YNP, WY, USA", 44.75933, -111.00616, 2075.31, 440726400, 32503680000, 1727970252));
    stations.push_back(::toStation("WY", "YML", "Mark Lake, YNP, WY, USA", 44.60533, -110.64317, 2557.12, 751593600, 32503680000, 1727970254));
    stations.push_back(::toStation("WY", "YMP", "Mirror Lake Plateau, YNP, WY, USA", 44.74017, -110.156, 2775.62, 1030924800, 32503680000, 1727970256));
    stations.push_back(::toStation("WY", "YMR", "Madison River, YNP, WY, USA", 44.66867, -110.965, 2164.45, 810950400, 32503680000, 1727970258));
    stations.push_back(::toStation("WY", "YMS", "Mount Sheridan, YNP, WY, USA", 44.264, -110.52783, 3089.03, 810950400, 32503680000, 1727970260));
    stations.push_back(::toStation("WY", "YMV", "Mammoth Vault, YNP, WY, USA", 44.97367, -110.68884, 1830.9, 698544000, 32503680000, 1736784640));
    stations.push_back(::toStation("WY", "YNB", "Norris Geyser Basin", 44.72741, -110.71121, 2307.0, 1693958400, 32503680000, 1727970264));
    stations.push_back(::toStation("WY", "YNE", "Northeast Entrance, YNP, WY, USA", 45.0076, -110.008, 2342.14, 1313884800, 32503680000, 1727970266));
    stations.push_back(::toStation("WY", "YNM", "Yellowstone Norris Museum, YNP, WY, USA", 44.7265, -110.7037, 2308.31, 1327449600, 32503680000, 1727970268));
    stations.push_back(::toStation("WY", "YNR", "Norris Junction, YNP, WY, USA", 44.7155, -110.67917, 2337.84, 751593600, 32503680000, 1756486458));
    stations.push_back(::toStation("WY", "YPC", "Pelican Cone, YNP, WY, USA", 44.648, -110.1925, 2928.92, 440726400, 32503680000, 1756486452));
    stations.push_back(::toStation("WY", "YPK", "Parker Peak, YNP, WY, USA", 44.7318, -109.921933, 2888.66, 1127174400, 32503680000, 1727970274));
    stations.push_back(::toStation("WY", "YPM", "Purple Mountain, YNP,  WY, USA", 44.65717, -110.86867, 2570.05, 751593600, 32503680000, 1727970276));
    stations.push_back(::toStation("WY", "YPP", "Pitchstone Plateau, YNP, WY, USA", 44.271, -110.8045, 2708.67, 827884800, 32503680000, 1727970278));
    stations.push_back(::toStation("WY", "YSC", "Slough Creek, Yellowstone, WY", 44.942945, -110.306596, 1930.25, 1750118400, 32503680000, 1754070456));
    stations.push_back(::toStation("WY", "YTP", "The Promontory, YNP, WY, USA", 44.39183, -110.285, 2373.26, 776044800, 32503680000, 1727970282));
    stations.push_back(::toStation("WY", "YUF", "Upper Falls, YNP, WY, USA", 44.7126, -110.5117, 2392.97, 1125619200, 32503680000, 1727970285));
    stations.push_back(::toStation("WY", "YWB", "West Boundary, YNP, WY, USA", 44.60583, -111.10083, 2307.43, 440726400, 32503680000, 1727970287));
    return stations;
}}
#endif
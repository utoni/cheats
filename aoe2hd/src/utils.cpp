#include "utils.h"

#include <sstream>
#include <iomanip>


namespace utils
{
std::string convertBinToHexstr(const std::vector<unsigned char>& bin)
{
    std::stringstream buffer;
    for (auto byte : bin)
    {
        buffer << std::hex << std::setfill('0');
        buffer << std::setw(2) << static_cast<unsigned>(byte);
    }
    return buffer.str();
}
}

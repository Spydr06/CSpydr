#include "StringUtils.hpp"

std::string getFilenameFromPath(std::string path) 
{

#if defined(_WIN32)
    const char* delims = "\\";
#elif defined(__linux__)
    const char* delims = "/";
#else
    #error "Only Linux and Windows are currently supported!"
#endif

    // Remove directory if present.
    // Do this before extension removal incase directory has a period character.
    const size_t last_slash_idx = path.find_last_of(delims);
    if (std::string::npos != last_slash_idx)
    {
        path.erase(0, last_slash_idx + 1);
    }

    // Remove extension if present.
    const size_t period_idx = path.rfind('.');
    if (std::string::npos != period_idx)
    {
        path.erase(period_idx);
    }

    return path;
}
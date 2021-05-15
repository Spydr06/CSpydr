#include "stringutils.hpp"
#include "../../../platform/platform_bindings.h"

std::string getFilenameFromPath(std::string path) 
{
    // Remove directory if present.
    // Do this before extension removal incase directory has a period character.
    const size_t last_slash_idx = path.find_last_of(DIRECTORY_DELIMS);
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
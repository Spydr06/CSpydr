#include "version.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char* get_cspydr_version()
{
    return "v" CSPYDR_VERSION_X "." CSPYDR_VERSION_Y "." CSPYDR_VERSION_Z CSPYDR_VERSION_W;
}

const char* get_cspydr_build()
{
    const char* buildTmp = "%s";
    const char* buildType = strcmp(CSPYDR_VERSION_W, "d") == 0 ? "debug" : "release";

    char* build = (char*) calloc(strlen(buildTmp) + strlen(buildType) + 1, sizeof(char));
    sprintf(build, buildTmp, buildType);
    return build;
}
#include "version.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char* getCSpydrVersion()
{
    return "v" CSPYDR_VERSION_X "." CSPYDR_VERSION_Y "." CSPYDR_VERSION_Z CSPYDR_VERSION_W;
}

const char* getCSpydrBuild()
{
    const char* buildTmp = "%ld-%s";
    const char* buildType = strcmp(CSPYDR_VERSION_W, "d") == 0 ? "debug" : "release";

    char* build = (char*) calloc(strlen(buildTmp) + strlen(buildType) + 1, sizeof(char));
    sprintf(build, buildTmp, BUILD_NUMBER, buildType);
    return build;
}
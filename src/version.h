#ifndef CSPYDR_VERSION_H
#define CSPYDR_VERION_H

#include "buildnumber.h"

#define CSPYDR_VERSION_X "0"
#define CSPYDR_VERSION_Y "0"
#define CSPYDR_VERSION_Z "3"

#ifdef DEBUG
    #define CSPYDR_VERSION_W "d"
#else
    #define CSPYDR_VERSION_W "r"
#endif

const char* getCSpydrBuild();
const char* getCSpydrVersion();

#endif